//We always have to include the library
#include "LedControl.h"
char __mathHelperBuffer[17];
/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */


 #define LCD_CS 10
 #define LCD_DIN 11
#define LCD_CLK 13

LedControl lc=LedControl(LCD_DIN,LCD_CLK,LCD_CS,0);

/* we always wait a bit between updates of the display */
unsigned long delaytime=5000;

void setup() {
  Serial.begin(9600);
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
   
  /*
  pinMode(LCD_DIN, OUTPUT);
  pinMode(LCD_CLK, OUTPUT);
  */
  pinMode(LCD_CS, OUTPUT);
  
  lc.shutdown(0,false);
  // Set the brightness to a medium values */
  lc.setIntensity(0,8);
  // and clear the display 
  lc.clearDisplay(0);
  /*
  displayDigit(123456.7,1,0);
  delay(20000);
  lc.clearDisplay(0);
  displayDigit(223.6,1,0);
  delay(2000000);
  */
 /*
 float n=12345;
 
 int d=50;
  delay(d);
 char c=char(s[0]);
 Serial.print(c);
 Serial.println();

  lc.setChar(0, "0" , c, false);
  Serial.print("1 ");
  delay(d);
  lc.setDigit(0, 1 , 2, true);
  Serial.print("2 ");
  delay(d);
  lc.setDigit(0, 2 , 3, false);
  Serial.print("3 ");
  delay(d);
  lc.setDigit(0, 3 , 4, false);
  Serial.print("4 ");
  delay(d);
  lc.setDigit(0, 4 , 5, true);
  Serial.print("5 ");
  delay(d);
  lc.setDigit(0, 5 , 6, false);
  Serial.print("6 ");
  delay(d);
  lc.setDigit(0, 6, 7, false);
  Serial.print("7 ");
  delay(d);
*/
double x=12345.3;
Serial.print(x);
Serial.println();
 displayDigit(x, 1, 0) ;
}


void loop() { 

}
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
  d = exponent - d * 10;
  __mathHelperBuffer[pos++] = d + '0';
  __mathHelperBuffer[pos] = '\0';
  return __mathHelperBuffer;
}
void displayDigit(double _inVal_, int _DecPoints_, int dispPos) {
  String c; //Single character variable to send to the display module
  int  i,val;
  bool dpf;
  String pt = ".";
  String s = String(_inVal_, _DecPoints_);
 
  if (s.length() > 8) { // if the number more than 8 digits convert it to scientific notation.
    s = sci(_inVal_, 2);
  }
  
  int len = int(s.length());
  Serial.println(s);
  for (i = len - 1; i >= 0; --i) { // start from the back end of the string
    c = s.substring(i, i+1);
    Serial.print(c);
    Serial.println();
    if ( c == pt) { /*if the character is a decimal point set the decimal point flag to true and read the character to the left of the decimal point*/
      --i;      //Skip to the next character to the left
      c = s[i]; // and read it
      dpf = true;
    } else {
      dpf = false;
    }
    val=c.toInt();
    Serial.println(val);
   lc.setDigit(0, dispPos, val, dpf);
    Serial.print(c);
    Serial.println();
    //Serial.print(int(c));
    ++dispPos;
  }
}
