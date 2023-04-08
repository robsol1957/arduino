 
#include <OneWire.h> 
#include <DallasTemperature.h> //Thermocpouples
#include <Arduino.h>
#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>
#define INFO // Deactivate this to save program memory and suppress info output from the LG-AC driver.
#include "ac_LG.hpp"
//
//
#define LED1 8
#define LED2 9
#define ONE_WIRE_BUS 2 //For Thermocouples and one wire devices
#define IR_TX_PIN 3 //IR LED - This has to be the value
#define TEMPERATURE_PRECISION 12

#define DISABLE_CODE_FOR_RECEIVER // Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not used.






OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
Aircondition_LG MyLG_Aircondition;


DeviceAddress tc[5]; // arrays to hold device addresses

float   tempoffset = -2;
int Nsensors,airconState;
float   smoothTemp;
float   alpha=0.5;
float   mintemp = 22;
float   sleeptemp = 23.5;
float   maxtemp = 24.5;


unsigned long thistime,lastsleep;
unsigned long maxsleeptime=3500000;
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

float getSmoothTemp()
{
  //float temp;
  float temp=getTemp();
  smoothTemp=smoothTemp*alpha+(1-alpha)*temp;
//  Serial.println("    Smoothtemp ," + String(smoothTemp));
  return(smoothTemp);
}

void writetext(String text){
  Serial.println(",data,"+String(thistime)+","+text);
}
float getTemp()
{
  int i;
  float result, tempC;
  result = 0;
  sensors.requestTemperatures();
  for (i = 0; i < Nsensors; ++i) {
    tempC = sensors.getTempC(tc[i]);
//   Serial.print("Currenttemp ," + String(tempC));
   result = result + tempC;
  }
  result = result / float(Nsensors);
  return (result);
}


void setup() {
  thistime = millis();
  // Basic setup
  //
  Serial.begin(115200);
  initTCs();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  #if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
  delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
  #endif
  // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    /*
     * The IR library setup. That's all!
     */
  #if defined(IR_SEND_PIN)
    IrSender.begin(); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
    Serial.println(F("Ready to send IR signals at pin " STR(IR_SEND_PIN)));
  #else
    IrSender.begin(3, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
    Serial.println(F("Ready to send IR signals at pin 3"));
  #endif

    Serial.println();
    MyLG_Aircondition.setType(LG_IS_WALL_TYPE);
    MyLG_Aircondition.printMenu(&Serial);
// Start running
// Initially test to see if temp is outside of deadband and set aircon state appropriately.
// If in side deadband then just leave alone

mintemp += tempoffset;
sleeptemp += tempoffset;
maxtemp += tempoffset;
 airconState=0;
 smoothTemp=getTemp();
 accontrol(int(2));
}



void loop() {
  // put your main code here, to run repeatedly:
   
  smoothTemp=getSmoothTemp();
  thistime = millis();
  if(airconState == 0 ){
    if( smoothTemp > maxtemp){
      writetext("Off-Full,"+String(smoothTemp));
      accontrol(int(2));
    } else if( smoothTemp > sleeptemp) {
        writetext("Sleep-off,"+String(smoothTemp));
        accontrol(int(1));
      }
      } else if(airconState == 1) {// on sleep
        unsigned long lag=millis()-lastsleep;
        writetext("time since last sleep on "+String(lag));
        writetext("maxsleep time "+ String(maxsleeptime));
        if (smoothTemp > maxtemp){
          writetext("Sleep-Full,"+String(smoothTemp));
          accontrol(int(2)); 
        } else if (smoothTemp <mintemp){
          accontrol(int(0));
          writetext("Sleep-Off,"+String(smoothTemp));
        } else if (lag >maxsleeptime){// still in sleep range but stable for 1 hr therefore reset
          writetext("resume sleep"+String(lag));
          accontrol(int(1));
        }
      } else if(airconState == 2){
        if(smoothTemp<mintemp){
          accontrol(int(0));
          writetext("Full-Sleep,"+String(smoothTemp));
        } else if(smoothTemp < sleeptemp){
          accontrol(int(1));
          writetext("Full-Sleep,"+String(smoothTemp));
          
        }
        
      } 
 writetext("aircon status ,"+String(airconState));
 writetext("Current_temp ,"+String(smoothTemp));
 delay(5000); 
}

void accontrol(int Action) {
//  Serial.println("In control loop "+String(Action));
if(Action==0) {
    Serial.println("Turning off");
    MyLG_Aircondition.sendCommandAndParameter('0', 0);
    airconState = 0;
    digitalWrite(LED1, LOW);
    digitalWrite(LED2,LOW);
} else  if(Action==1){
    Serial.println("Turning sleep");
    MyLG_Aircondition.sendCommandAndParameter('1', 0);
    delay(100);
    MyLG_Aircondition.sendCommandAndParameter('t', mintemp);
    delay(100);
    MyLG_Aircondition.sendCommandAndParameter('S', 420);
    airconState=1;
    lastsleep=millis();
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2,LOW);
  } else   if(Action==2){
    Serial.println("Turning full");
    MyLG_Aircondition.sendCommandAndParameter('C',0);
    delay(100);
    MyLG_Aircondition.sendCommandAndParameter('1', 0);
    delay(100);
    MyLG_Aircondition.sendCommandAndParameter('t', mintemp);
    delay(100);
    MyLG_Aircondition.sendCommandAndParameter('f', 1);
    airconState=2;
    digitalWrite(LED1, LOW);
    digitalWrite(LED2,HIGH);
  }
 }
