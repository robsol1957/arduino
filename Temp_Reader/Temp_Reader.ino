
#include <LedControl.h>
#include <OneWire.h> //one wire devices // For Thermocouples and other one wire devices
#include <DallasTemperature.h> //Thermocpouples



/**************** LED Pin Definitions ********
#define LCD_DIN 12  //miso Master in Slave out
#define LCD_CLK 13 //sck
#define LCD_CS 10 //ss - slave select
*/
LedControl lc=LedControl(12,13,10,1);

#define ONE_WIRE_BUS 2 //For Thermocouples and one wire devices

#define TEMPERATURE_PRECISION 12

OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress tc[5]; // arrays to hold device addresses
int Nsensors;
char __mathHelperBuffer[17];
float Calib[][2] ={{0.0,1.0},{0.0,1.0}};

void setup() {
  Serial.begin(9600);
  initTCs();
 lc.shutdown(0,false);
 //and clear the display 
  lc.clearDisplay(0);
}

void loop() {
  float AvgTemp = getTemp();
  lc.clearDisplay(0);
  displayDigit( AvgTemp,2,0);
  Serial.println(AvgTemp);
  delay(1000);
}

float getTemp()
{
  int i;
  float result, tempC;
  result = 0;
  sensors.requestTemperatures();
  for (i = 0; i < Nsensors; ++i) {
    tempC = sensors.getTempC(tc[i]);
   // printtofile(filename, "TCRaw" + String(i) + "," + String(tempC, 4));
    tempC=(tempC-Calib[i][0])/Calib[i][1];
   // printtofile(filename, "TC" + String(i) + "," + String(tempC, 4));
    //Serial.println("TC" + String(i) + "," + String(tempC, 4));
    result = result + tempC;
  }
  result = result / float(Nsensors);
  return (result);
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


 // Chunk of code for converting any number into scientific notation.
 
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
