
/////////////////////////////////////////////////////
// Buzzer
//////////////////////////////////////////////////////////
#define BUZERPIN 5
#define TOGGLEPIN 6
//////////////////////////////////////////////////////
// Rotary Encoder Header
/////////////////////////////////////////////////////

// https://lastminuteengineers.com/rotary-encoder-arduino-tutorial/
#define CLK 4 // Rotary Encoder
#define DT 3//Pin2 Rotary Encoder
#define SW 2
int lastStateCLK;

///////////////////////////////////////////////////////////
// LCD Header
//////////////////////////////////////////////////////////
/**************** LED Pin Definitions ********
  pin 12 is connected to the DIN DataIn
  pin 13 is connected to the CLK
  pin 10 is connected to CS (LOAD)
*/
char __mathHelperBuffer[17];
float stage[4];
#include "LedControl.h"
LedControl lc = LedControl(12, 13, 10, 1);


///////////////////////////////////////
//
//////////////////////////////////////
unsigned long agitate_period = 30000;

void setup() {
  // put your setup code here, to run once:
  // Setup Serial Monitor
  Serial.begin(9600);

  /////////////////////////////////////////////////////
  // Set Up Rotary Encoder
  ////////////////////////////////////////////////////

  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  /////////////////////////////////////////////////////
  // LCD Display
  ////////////////////////////////////////////////////

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  /// Start of block code
  //
  // Get and update delay durations
  get_delays(); // update the inputs
  //////////////////////////////////////////////////
  // Hang until the button pressed
  /////////////////////////////////////////////////
  int was = digitalRead(TOGGLEPIN);
  do {
  } while (digitalRead(TOGGLEPIN) == was);
  //Alert that buzzer has been pressed before jumping to timer.
  //wait 30 seconds with 10 second countdown
  wait(millis() + 30000, 10, 1000);
  int this_event = 0;
  unsigned long run_start = millis();
  while (stage[this_event] > 0) {
    unsigned long event_end = run_start + 1000 * stage[this_event];
    unsigned long next_Agitate = millis() + agitate_period;
    while (event_end > next_Agitate + 15000) {
      wait(next_Agitate, 3, 1000); // three pulses just prior to next agitate
      next_Agitate = millis() + agitate_period;
    };
    wait(event_end, 10, 1000);
    run_start = event_end;
    ++this_event;
  }
  pulse(100);
}

void loop() {

}

/////////////////////////////////////////////
//get data
//////////////////////////////////////////////

void get_delays() {
  int i;
  stage[0] = get_enc_val(500, 1, SW);
  Serial.println(stage[0]);
  stage[1] = get_enc_val(60, 1, SW);
  Serial.println(stage[1]);
  stage[2] = get_enc_val(300, 1, SW);
  Serial.println(stage[2]);
  Serial.println("Shutting led down");
  stage[3]=0;
  for (i = 0; i < 4; ++i) {
    lc.clearDisplay(0);
    delay(100);
    displayDigit_clr(stage[i], 1, 0);
    delay(1000);
  }

  lc.shutdown(0, true);
}






void wait(unsigned long thisend, int pulses, int dur) {
  thisend = thisend - pulses * dur;
  while (millis() < thisend) {}
  triple(pulses, dur);
}

/////////////////////////////////////////////////
// Buzzer utils
//////////////////////////////////////////////////
void pulse(unsigned long dur) {

  digitalWrite(BUZERPIN, HIGH);
  delay(dur);
  digitalWrite(BUZERPIN, LOW);
}
///////////////////////////////////////////////////
// Repeated pulse pulse times for a duration of dur
// interspursed with a 100ms pause
//////////////////////////////////////////////////
void triple(int times, unsigned long dur) {
  for (int i = 1; i <= times; ++i) {
    pulse(100);
    delay(dur - 100);
  }
}




//////////////////////////////////////////////////////////
// Rotary Encoder utility routines
/////////////////////////////////////////////////////////
float get_enc_val(float value, float factor, int button) {
  int digs = 2;
  displayDigit_clr(value, digs, 0);
  while (check_button_press(button) == false) {
    float old_value = value;
    value = read_enc_change(value, factor);
    if (old_value != value) {
      Serial.print(" value: ");
      Serial.println(value);
      displayDigit_clr(value, digs, 0);
    }
  }
  Serial.println("Button pressed");
  delay(500);
  return (value);
}

float read_enc_change(float value, float factor) {
  int currentStateCLK = digitalRead(CLK);
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
    if (digitalRead(DT) != currentStateCLK) {
      value = value - factor;
    } else {
      value = value + factor;
    }
  }
  lastStateCLK = currentStateCLK;
  return (value);
}
bool check_button_press(int button) {
  bool ret = true;
  unsigned long enter = millis();
  int btnState = digitalRead(button);
  while (btnState == LOW && millis() < enter + 50) {
    btnState = digitalRead(button);
  }
  if (millis() < enter + 50) {
    ret = false;
  }
  return (ret);
}
/////////////////////////////////////////////////////////////////
// LCD Utilities
/////////////////////////////////////////////////////////////////
/*
   Chunk of code for converting any number into scientific notation.
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
  d = exponent - d * 10;
  __mathHelperBuffer[pos++] = d + '0';
  __mathHelperBuffer[pos] = '\0';
  return __mathHelperBuffer;
}
/*
  displayDigit displays a number to the display unit
  you can set the number of digits after the decimal point
  but if the resulting number is >8 then the number will
  be converted to scientific notation.
*/
void displayDigit(double _inVal_, int _DecPoints_, int dispPos) {
  char c; //Single character variable to send to the display module
  int  i;
  bool dpf;

  String s = String(_inVal_, _DecPoints_);

  if (s.length() > 8) { // if the number more than 8 digits convert it to scientific notation.
    s = sci(_inVal_, 2);
  }
  int len = int(s.length());
  for (i = len - 1; i >= 0; --i) { // start from the back end of the string
    c = s[i];
    if ( c == '.') { /*if the character is a decimal point set the decimal point flag to true and read the character to the left of the decimal point*/
      --i;      //Skip to the next character to the left
      c = s[i]; // and read it
      dpf = true;
    } else {
      dpf = false;
    }
    lc.setChar(0, dispPos, c, dpf);
    ++dispPos;
  }
  //delay(delaytime);
}

void writetemp(double _inVal1_, double _inVal2_) {
  lc.clearDisplay(0);
  displayDigit( _inVal1_, 1, 0);
  displayDigit( _inVal2_, 2, 4);
  delay(1000);
}

void displayDigit_clr(double _inVal_, int _DecPoints_, int dispPos) {
  lc.clearDisplay(0);
  displayDigit( _inVal_, _DecPoints_, dispPos);
}
