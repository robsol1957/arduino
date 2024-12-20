/*
 * General Constant temperature controller
 * Controller Powered by DC voltage source between 5-20 volts
 * Includes 4 Relays rated up to 240vAC- Take care that the controlled devices operate at the appropriate 
 * inpur Voltage
 * 
 * Setpoint can be changed by pressing the Button an waiting up to 10 seconds then using the Rotary encoder or
 * via serial port Command S
 * Other parameters that can be changed are P,I and D Constants
 * 
 * Heater Relay is R0
 * Fan Relay is R1
 * 
 * R Solomon 07/03/2021
 * 
 *
 */
 
#include "LedControl.h"
#include "ESPRotary.h";
//#include "LedControl.h"
#include <OneWire.h> //one wire devices // For Thermocouples and other one wire devices
#include <DallasTemperature.h> //Thermocpouples
#include <DS1302.h>
// Init the DS1302


/**************** LED Pin Definitions ********
 pin 12 is connected to the DIN DataIn 
 pin 13 is connected to the CLK 
pin 10 is connected to CS (LOAD)
*/
LedControl lc=LedControl(12,13,10,1);
//DS1302(uint8_t rst_pin, uint8_t data_pin, uint8_t sclk_pin)
DS1302 rtc(A1,A2, A3);

#define ONE_WIRE_BUS 2 //For Thermocouples and one wire devices
#define ROTARY_PIN1 3 //Pin1 Rotary Encoder
#define ROTARY_PIN2 4 //Pin2 Rotary Encoder
#define Button  5     //Press button to adjust setpoint
#define LED_Pin 11    //Switch to turn LED Display on

#define TEMPERATURE_PRECISION 12
#define HOURS 3600000  //milliseconds in an hour
#define MINUTES 60000 //milliseconds in a minute
#define SECONDS 1000  //milliseconds in a minute
/* ***********  Relay Pins 
 *  R0-3:= 6-9
 *  In this instance Heater =Relay0
 *  Fan = Relay 1
 */
int Relay[]= {6,7,8,9};
#define HEATER 0 // relay 0 for heater
#define COOLER 2 // relay 0 for heater
#define FAN 1  // for fan

OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress tc[5]; // arrays to hold device addresses
ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2);



char __mathHelperBuffer[17];
String filename = "Dummy"; // not used in this code

int Nsensors;
float  SetPoint=20.0;  //= 39.3;// target 38.9 possible calibration error of -0.8
float kP = 5000;
//float kI = 0.006;
float kI = 0.01;
float MaxI = 10000;
float kD = 1000;
float FlashDur = 10000;
float AvgTemp,  Error;
float p, I, d, pid, off, lastError, dt;
float Calib[][2] ={{0.0,1.0},{0.0,1.0}};
unsigned long PulseStart, lasttime, thistime;
String CycleStart;
boolean HeaterState;
byte LEDStatus;




void setup() {
  Serial.begin(9600);
  for(int i=0;i<=3;i++){
    pinMode(Relay[i],OUTPUT);
    }
  setupClck();
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  pinMode (Button,INPUT_PULLUP);
  pinMode(LED_Pin,INPUT_PULLUP);
  
  initTCs();
  
  //for(int i=0;i<=7;i++){
  //  pinMode(DecPins[i],INPUT_PULLUP);
  //  }
  if(!digitalRead(LED_Pin)){
      LEDStatus=true;
      lc.shutdown(0,false);
      lc.setIntensity(0,8);
  } else{
    LEDStatus=false;
  }

  /* and clear the display */
  lc.clearDisplay(0);
  CycleStart = datestring();
  printtofile(filename, "kP," + String(kP,6));
  printtofile(filename, "kI," + String(kI,6));
  printtofile(filename, "kD," + String(kD,6));
  digitalWrite(Relay[FAN], HIGH);
}

void loop() {
  while(!digitalRead(Button)){// press and hold the button to modify set point
      getsp();
      }
  CheckConstants();
  printtofile(filename, "Setpoint," + String(SetPoint));
  CycleStart = datestring();
  AvgTemp = getTemp();
    if(!digitalRead(LED_Pin)){//LED Switch on
      if(!LEDStatus){// but LED OFF
        LEDStatus=true; // Switch LED On
        lc.shutdown(0,false);
        lc.setIntensity(0,8);
      }
    writetemp(SetPoint,AvgTemp);
  } else {// switch off
    if(LEDStatus){
      lc.shutdown(0,true);
      LEDStatus=false;
    }
  }
  
  thistime = millis();
  dt = float(thistime - lasttime);
  lasttime = thistime;
  lastError = Error;
  Error = SetPoint - AvgTemp;
  printtofile(filename, "Error," + String(Error, 4));
  p = kP * Error;
  I = (I + kI * Error * dt);
  if(abs(p) >= MaxI){
    I=0;
    //I = MaxI*abs(I)/I;
    printtofile(filename, "Trimmed I," + String(I));
  }
  printtofile(filename, "i," + String(I));
  printtofile(filename, "p," + String(p));
  d = kD * (Error - lastError) / FlashDur;
  //printtofile(filename, "d," + String(d));
  pid = (I + p + d);
  printtofile(filename, "pid," + String(pid));
  if (pid <= 0) {
    pid = 0;
  }
  //pid=0; debug
  off = FlashDur - pid;
  if (pid > FlashDur) {
    pid = FlashDur;
    off = 0;
  }
  printtofile(filename, "pidTrim," + String(pid));
  if (pid > 0) {
    printtofile(filename, "HeaterStatus,1");
    digitalWrite(Relay[HEATER], HIGH);
    delay(long(pid));
  }

  if (off > 0) {
    printtofile(filename, "HeaterStatus,0");
    digitalWrite(Relay[HEATER], LOW);
    delay(long(off));
  }
   Serial.println(" ");
  // delay(1000000);// debug
}

void setupClck()
{
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  // The following lines can be commented out to use the values already stored in the DS1302
/*
  rtc.setDOW(THURSDAY);        // Set Day-of-Week to FRIDAY
  rtc.setTime(16, 11, 0);     // Set the time to 12:00:00 (24hr format)
  rtc.setDate(11, 3, 2021);   // Set the date to August 6th, 2010
*/
}



float getsp() {
   if(LEDStatus==false){
   lc.shutdown(0,false);
   lc.setIntensity(0,8);
   LEDStatus=true;
   }
  lc.clearDisplay(0);
  displayDigit( SetPoint,2,4);
  long et=millis()+1000;
  while(!digitalRead(Button)){// press and hold the button to modify set point
    r.loop();
    if(et<millis()){
      displayDigit(SetPoint,2,4);
      et=millis()+250;
    }
  }
  
}
// on change
void rotate(ESPRotary& r) {
 //Serial.println(r.getPosition());
   SetPoint=38-float(r.getPosition())/40;
 //  Serial.println(SetPoint);
}

// on left or right rotattion
void showDirection(ESPRotary& r) {
 // Serial.println(r.directionToString(r.getDirection()));
}
float getTemp()
{
  int i;
  float result, tempC;
  result = 0;
  sensors.requestTemperatures();
  for (i = 0; i < Nsensors; ++i) {
    tempC = sensors.getTempC(tc[i]);
   // printtofile(filename, "TCRaw " + String(i) + "," + String(tempC, 4));
    //tempC=(tempC-Calib[i][0])/Calib[i][1];
    printtofile(filename, "TC " + String(i) + "," + String(tempC, 4));
    //Serial.println("TC" + String(i) + "," + String(tempC, 4));
      if(tempC<10){
        tempC=SetPoint+1;
        printtofile(filename,"Sensor has blown, cap temp******************");
        }
    result = result + tempC;
  }
  
  result = result / float(Nsensors);
  // check blown sensor

  printtofile(filename, "AvgTemp," + String(result, 4));
  return (result);
}
void printtofile(String _filename, String _val) {
  Serial.print(CycleStart);
  Serial.print(",");
  Serial.println(String(_val));
}
String fixed(int val,int digits) {
  String r;
  r="0000000"+String(val);
  r=r.substring(r.length()-digits);
  return (r);
}

String datestring() {
  /*
  unsigned long millisecfromStart=millis();
  int hoursfromStart=millisecfromStart/HOURS;
  millisecfromStart=millisecfromStart-hoursfromStart*HOURS;
  int minutesfromstart=millisecfromStart/MINUTES;
  millisecfromStart=millisecfromStart-minutesfromstart*MINUTES;
  int secondsfromstart = millisecfromStart/SECONDS;
  int msfromstart=millisecfromStart-secondsfromstart*SECONDS;
  String s=fixed(hoursfromStart,2)+":" + fixed(minutesfromstart,2) + ":"+ fixed(secondsfromstart,2) + "." + fixed(msfromstart, 3);
  */
  Time t = rtc.getTime();
  String s=String(t.year)+"-"+fixed(t.mon,2)+"-"+fixed(t.date,2) +" "+fixed(t.hour,2)+":"+fixed(t.min,2)+":"+fixed(t.sec,2);
  
  return (s);
}

void initTCs() {
  //////////////////////////////////////////
  // Connect Thermocouples
  /////////////////////////////////////////
  // Start up the library
  Serial.println("initialising Thermocouples.");
  sensors.begin();
  Nsensors = sensors.getDeviceCount();
  Serial.print(Nsensors);
  Serial.println(" Found");
  // locate devices on the bus
  oneWire.reset_search();
  int i = 0;
  while (oneWire.search(tc[i]) && i < Nsensors) {
    sensors.setResolution(tc[i], TEMPERATURE_PRECISION);
    ++i;
  }
}
void CheckConstants(){
  int i,n;
  String text="";
  char c;
  float Flt;
  if (Serial.available() > 0){
     n=Serial.available();
     for(i=1;i<=n;++i){
      text=text+char(Serial.read());
      }
      c=text[0];
      text=text.substring(1);
      Flt=text.toFloat();
      if(Flt >0){
     
        switch (c){
          case 'P':
            printtofile(filename, "kP was," + String(kP,6));
            kP = Flt;
            printtofile(filename, "kP is," + String(kP,6));
            break;
          case 'I':
            printtofile(filename, "kI was," + String(kI,6));
            kI = Flt;
            printtofile(filename, "kI is," + String(kI,6));
            break;
          case 'D':
            printtofile(filename, "kD was," + String(kD,6));
            kD = Flt;
            printtofile(filename, "kD is," + String(kD,6));
            break;
          case 'S':
            printtofile(filename, "SetPoint was," + String(SetPoint,2));
            SetPoint = Flt;
            printtofile(filename, "SetPoint is," + String(SetPoint,2));
            break;
          default:
            Serial.println("Undefined code only upper case P , I, D or S for Setpoint");
            break;
        }
       } else {
          Serial.println("Invalid constant "+text+" needs to be valid numeric");
      }  
  }  
}
/*
 * Chunk of code for converting any number into scientific notation.
 */
char * sci(double number, int digits)
{
  int exponent = 0;
  int pos = 0;
  // Handling these costs 13 bytes RAM
  // shorten them with N, I, -I ?
  if (isnan(number)) 
  {
    strcpy(__mathHelperBuffer, "nan");
    return __mathHelperBuffer;
  }
  if (isinf(number))
  {
    if (number < 0) strcpy(__mathHelperBuffer, "-inf");
    strcpy(__mathHelperBuffer, "inf");
    return __mathHelperBuffer;
  }
  // Handle negative numbers
  bool neg = (number < 0.0);
  if (neg)
  {
    __mathHelperBuffer[pos++] = '-';
    number = -number;
  }
  while (number >= 10.0)
  {
    number /= 10;
    exponent++;
  }
  while (number < 1 && number != 0.0)
  {
    number *= 10;
    exponent--;
  }
  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i = 0; i < digits; ++i)
  {
    rounding *= 0.1;
  }
  number += rounding;
  if (number >= 10)
  {
    exponent++;
    number /= 10;
  }
  // Extract the integer part of the number and print it
  uint8_t d = (uint8_t)number;
  double remainder = number - d;
  __mathHelperBuffer[pos++] = d + '0';   // 1 digit before decimal point
  if (digits > 0)
  {
    __mathHelperBuffer[pos++] = '.';  // decimal point TODO:rvdt CONFIG?
  }
  // Extract digits from the remainder one at a time to prevent missing leading zero's
  while (digits-- > 0)
  {
    remainder *= 10.0;
    d = (uint8_t)remainder;
    __mathHelperBuffer[pos++] = d + '0';
    remainder -= d;
  }
  // print exponent
  __mathHelperBuffer[pos++] = 'E';
  neg = exponent < 0;
  if (neg)
  {
    __mathHelperBuffer[pos++] = '-';
    exponent = -exponent;
  }
  else __mathHelperBuffer[pos++] = '+';
  // 3 digits for exponent;           // needed for double
  // d = exponent / 100;
  // __mathHelperBuffer[pos++] = d + '0';
  // exponent -= d * 100;
  // 2 digits for exponent
  d = exponent / 10;
  __mathHelperBuffer[pos++] = d + '0';
  d = exponent - d*10;
  __mathHelperBuffer[pos++] = d + '0';
  __mathHelperBuffer[pos] = '\0';
  return __mathHelperBuffer;
}
 /* 
 * displayDigit displays a number to the display unit
 * you can set the number of digits after the decimal point
 * but if the resulting number is >8 then the number will
 * be converted to scientific notation.
 */
void displayDigit(double _inVal_,int _DecPoints_,int dispPos){
  
  char c; //Single character variable to send to the display module
  int  i;
  bool dpf;
  
  String s=String(_inVal_,_DecPoints_);

    if(s.length()>8){// if the number more than 8 digits convert it to scientific notation.
    s=sci(_inVal_, 2); 
    }
  int len=int(s.length());
  for(i=len-1;i>=0;--i){// start from the back end of the string
    c=s[i];
    if( c == '.'){/*if the character is a decimal point set the decimal point flag to true and read the character to the left of the decimal point*/
      --i;      //Skip to the next character to the left
      c=s[i];   // and read it
      dpf=true;
    } else {
      dpf=false;
    }
    lc.setChar(0,dispPos,c,dpf);
    ++dispPos;
  }
 //delay(delaytime);
}

void writetemp(double _inVal1_,double _inVal2_){
  lc.clearDisplay(0);
  displayDigit( _inVal1_,1,0);
  displayDigit( _inVal2_,2,4);
  delay(1000);
}
