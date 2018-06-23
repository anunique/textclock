#include <Wire.h>
#include "RTClib.h"
#include <FastLED.h>
#include "math.h"

#define  NUM_LEDS               11*11+6       // total number of Neo-Pixels
#define  LED_STRIP            4             // led strip output       - pin 4
#define  BUT_UP               7             // mode up   button       - pin 7
#define  BUT_DN               8             // mode down button       - pin 8
#define  RTC_SQW              2             // RTC_SQW pin            - pin 2
#define  LDR_PIN              A0            // Analog Pin of LDR      - pin A0
#define  MODE_LIMIT           7             // upper mode limit

CRGB leds[NUM_LEDS];

RTC_DS3231    rtc;

byte          secondOld   = 255;
byte          hour        = 0;              // hours in 24 hour clock
byte          minute      = 0;              // minutes
byte          second      = 0;              // seconds
byte          day         = 0;              // day of the month
byte          month       = 0;              // month
byte          year        = 0;              // year in 2 digit format
byte          mode        = 0;              // display modes (clock (default), date, audio1, audio1 rotating, audio2, audio2 rotating, PacMan)
byte          modeOld     = mode;           // previous mode
byte          buttonState = 0;              // holds the status of the buttons after being read

//array pointers
byte by, by_len;
byte cw, cw_len;
byte it, it_len;
byte is, is_len;
byte half, half_len;
byte past, past_len;
byte to, to_len;
byte oclock, oclock_len;
byte und, und_len;
byte twentyh, twentyh_len;
byte hours[13], hours_len[13];
byte dots[4];
byte five, five_len;
byte ten, ten_len;
byte quarter, quarter_len;
byte twenty, twenty_len;

byte matrix[11];
// =====================================================================================================
// ADC IMPROVEMENTS
// to increase frequency of ADC readings
// defines for setting and clearing register bits
// =====================================================================================================
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
void initFastADC() {
    // set ADC prescaler to 16 to speedup ADC readings
    sbi(ADCSRA,ADPS2);
    cbi(ADCSRA,ADPS1);
    cbi(ADCSRA,ADPS0);
}
// =====================================================================================================
// setup()
// =====================================================================================================
void setup() {
  Serial.begin(115200);
  initFastADC();
  while (!Serial);
  Serial.println("starting");
  if (!rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }  
  if (rtc.lostPower()) 
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));     //set the RTC to the date & time this sketch was compiled
  }
  randomSeed(analogRead(0));
  // start up the 1Hz interrupt from RTC
  //TODO why doesnt this one work???
  rtc.writeSqwPinMode (DS3231_SquareWave1Hz);
  attachInterrupt (digitalPinToInterrupt (RTC_SQW), registerOneSecond, FALLING);  
  resetArrayIndexes();                           // initialises the reference index arrays 
  LEDS.addLeds<WS2812,LED_STRIP,GRB>(leds,NUM_LEDS);
  LEDS.setBrightness(255); 
  // initialise buttons
  pinMode        (BUT_UP,         INPUT);      // up   button  pin set as digital input
  pinMode        (BUT_DN,         INPUT);      // down button  pin set as digital input
  pinMode        (LDR_PIN,        INPUT);      // LDR          pin set as analog  input
  digitalWrite   (BUT_UP,          HIGH);      // up   button  pin pull up resistor enabled 
  digitalWrite   (BUT_DN,          HIGH);      // down button  pin pull up resistor enabled 
  pinMode        (RTC_SQW, INPUT_PULLUP);      // pullup resistor for RTC interrupt
  ReadTimeDate();                              // read date and time
  for (int i = 0; i<11; i++)
  {
    matrix[i]=14;
  }
  clearLeds(CRGB(120, 0, 0));
  showLeds();
  delay(1000);
  clearLeds(CRGB(0, 120, 0));
  showLeds();
  delay(1000);
  clearLeds(CRGB(0, 0, 120));
  showLeds();
  delay(1000);
  clearLeds(CRGB(0, 120, 120));
  showLeds();
  delay(1000);
  clearLeds(CRGB(120, 0, 120));
  showLeds();
  delay(1000);
  clearLeds(CRGB(120, 120, 0));
  showLeds();
  delay(1000);
  clearLeds(CRGB(120, 120, 120));
  showLeds();
  delay(1000);
}
// =====================================================================================================
// loop()
// =====================================================================================================
int ctr = 0;
void loop() {
  if (ctr > 10000)
  {
    ctr=0;
    secondOld=255;
    ReadTimeDate();                              // read date and time
  }
  ctr++;
  buttonState = getButtons();
  if (buttonState == 1)                                                                   // mode up   detected
  { 
    mode++; 
    if (mode > MODE_LIMIT)    
    {
      mode = 0; 
    }
  }
  if (buttonState == 2)                                                                   // mode down detected
  { 
    mode--; 
    if (mode == 255)    
    {
      mode = MODE_LIMIT; 
    }
  }  
  if (mode != modeOld)                                                                   // mode changed do some resets
  { 
    secondOld = 255; 
    modeOld = mode;       
  }
  if (buttonState == 3)                                                                   // both mode buttons pressed
  { 
    setTime();
  }
  if (second != secondOld)                          // 1Hz interrupt occured or first show after mode changed to Time mode
  {
    clearLeds(0);
    secondOld = second;                          // reset interrupt flag 
    if (mode == 0)
    {
      showMatrix();
      displayTime(hour, minute, second, 0);
    }
    if (mode == 1)
    {
      displayTime(hour, minute, second, 0);
    }
    if (mode == 2)
    {
      displaySeconds(hour, minute, second, 0);
      displayTime(hour, minute, second, 0);
    }
    if (mode == 3)
    {
      showMatrix();
    }
    if (mode == 4)
    {
      showPLASMA();
      displayTime(hour, minute, second, 0);
    }
    if (mode == 5)
    {
      showRainbow();
      displayTime(hour, minute, second, 0);
    }
    if (mode == 6)
    {
      showNoise();
      displayTime(hour, minute, second, 0);
    }
    if (mode == 7)
    {
      showSinusoids();
      displayTime(hour, minute, second, 0);
    }
    showLeds();
  }
  //TODO: cookie();                                                    
  //TODO: pacman();
}
// =====================================================================================================
// INTURRUPT ROUTINE
//   - registers 1 second elapsed
// =====================================================================================================
void registerOneSecond() 
{
  secondOld = 255;
}
// =====================================================================================================
// GET BUTTION STATE
// =====================================================================================================
byte getButtons() 
{
  boolean butRight = 0;                                  // right    button status
  boolean butLeft = 0;                                   // left     button status
  byte butState = 0;                                     // combined button status
  butRight = !digitalRead(BUT_UP);                       // read right button state
  butLeft = !digitalRead(BUT_DN);                        // read left  button state
  while (butRight || butLeft)                            // if either button input is activated loop until non are pressed
  {
    butRight = !digitalRead(BUT_UP);                     // read right button state
    butLeft = !digitalRead(BUT_DN);                      // read left  button state
    if (butRight)                                        // if right button has been pressed record it in combined button status 
    { 
      butState |= 1; 
    }
    if (butLeft )                                        // if left  button has been pressed record it in combined button status 
    { 
      butState |= 2; 
    }
  }
  return (butState);                                     // return combined button status
}
// =====================================================================================================
// BUTTON STATE INCRUMENT/DECROMENT 
//   - accepts an initial value, low & high limit, buttons inc/dec then retrun value within the limits
// =====================================================================================================
byte buttonIncDec(int value, byte lowLimit, byte highLimit) 
{
  buttonState = getButtons();
  switch (buttonState) 
  {
    case 1:
      value++;
      if (value > highLimit) 
      { 
        value = lowLimit; 
      }
      break;
    case 2:
      value--; 
      if (value < lowLimit) 
      { 
        value = highLimit; 
      }
      break;
    default:
      break;
  }
  return (value);
}  
// =====================================================================================================
// SET DATE AND TIME FUNCTION
//   - function taken directly from the RTC example code on the Sparkfun web site
// =====================================================================================================
int SetTimeDate(int d, int mo, int y, int h, int mi, int s)
{ 
  rtc.adjust(DateTime(y, mo, d, h, mi, s));
  hour = h;
  minute = mi;
  second = s;
  year = y;
  month = mo;
  day = d;
}
// =====================================================================================================
// READ DATE AND TIME
//   - function taken directly from the RTC example code on the Sparkfun web site
// =====================================================================================================
void ReadTimeDate()
{
  String temp;
  DateTime now = rtc.now();
  day    = now.day();
  month  = now.month();
  year   = now.year();
  minute = now.minute(); 
  second = now.second();
  hour = now.hour();
  temp.concat(year);
  temp.concat("-") ;
  temp.concat(month);
  temp.concat("-") ;
  temp.concat(day);
  temp.concat("     ") ;
  temp.concat(hour);
  temp.concat(":") ;
  temp.concat(minute);
  temp.concat(":") ;
  temp.concat(second);
  Serial.println(temp);
}
// =====================================================================================================
// RESET ARRAY POINTERS TO DEFAULT
// =====================================================================================================
void resetArrayIndexes() 
{
  dots[0] = 5;
  dots[1] = 3;
  dots[2] = 2;
  dots[3] = 0;
  oclock = 6+8;
  oclock_len = 3;
  hours[12] = 6+3;
  hours_len[12] = 5;
  hours[11] = 6;
  hours_len[11] = 3;
  hours[10] = 6+1*11+7;
  hours_len[10] = 4;
  hours[9] = 6+2*11+4;
  hours_len[9] = 4;
  hours[8] = 6+2*11;
  hours_len[8] = 4;
  hours[7] = 6+3*11+5;
  hours_len[7] = 6;
  hours[6] = 6+3*11;
  hours_len[6] = 5;
  hours[5] = 6+4*11+7;
  hours_len[5] = 4;
  hours[4] = 6+4*11;
  hours_len[4] = 4;
  hours[3] = 6+5*11;
  hours_len[3] = 4;
  hours[2] = 6+5*11+7;
  hours_len[2] = 4;
  hours[1] = 6+6*11+8;
  hours_len[1] = 3;
  hours[0] = 6+6*11;
  hours_len[0] = 4;
  twentyh = 6+1*11;
  twentyh_len = 7;
  und = 6+2*11+8;             //cant use 'and' here w00t
  und_len = 3;
  half = 6+7*11;
  half_len = 4;
  to = 6+7*11+8;
  to_len = 3;
  past = 6+7*11+4;
  past_len = 4;
  by = 6+8*11;
  by_len = 2;
  cw = 6+8*11 + 9;
  cw_len = 2;

  it = 6+10*11;
  it_len = 2;
  is = 6+10*11+3;
  is_len = 3;

  five = 6+10*11+7;
  five_len = 4;
  ten = 6+9*11+7;
  ten_len = 4;
  quarter = 6+9*11;
  quarter_len = 7;
  twenty = 6+8*11+2;
  twenty_len = 7;
}
// =====================================================================================================
// SET THE TIME
//   - when time mode is being displayed, 2 button press will enter time setting mode.
//   - each 2 button press skips between - hour, minute, second, apply new time. Date is unaltered.
//   - single button press will either inc/dec each value within certain limits.
//   - temp disables interrupts during time setting.
// =====================================================================================================
void setTime()
{
  byte newHour = hour;                                                // temporarily store current 24 hour as new 24 hour  
  byte newMinute = minute;                                            // temporarily store current minute  as new minute  
  byte newSecond = second;                                            // temporarily store current second  as new second
  noInterrupts();                                                     // temp disable the 1Hz interrupt
  // set hour
  // --------
  buttonState = getButtons();
  while (buttonState != 3) 
  { 
    newHour = buttonIncDec(newHour, 0, 23);                           // inc/dec the new hour value making sure it stays within 0 - 23 boundary
    clearLeds(0);
    displayTime(newHour, newMinute, newSecond, 1);                 // display new time with hours brighter than minutes & seconds
    showLeds();
  }
  // set minute
  // ----------
  buttonState = getButtons();
  while (buttonState != 3) 
  { 
    newMinute = buttonIncDec(newMinute, 0, 59);                      // inc/dec the new minute value making sure it stays within 0 - 59 boundary
    clearLeds(0);
    displayTime(newHour, newMinute, newSecond, 2);                // display new time with minutes brighter than hours & seconds
    showLeds();
  }
  // set second
  // ----------
  buttonState = getButtons();
  while (buttonState != 3) 
  { 
    newSecond = buttonIncDec(newSecond, 0, 59);                      // inc/dec the new second value making sure it stays within 0 - 59 boundary
    clearLeds(0);
    displaySeconds(newHour, newMinute, newSecond, 3);                // display new time with seconds brighter than hours & minutes
    showLeds();
  }  
  buttonState = 0; 
  ReadTimeDate();  
  SetTimeDate(day, month, year, newHour, newMinute, newSecond);      // apply current date and new time 
  interrupts();                                                      // re-enable the interrupts
}
// =====================================================================================================
// DISPLAY THE SECONDS
//   - accepts focus, which applies a highlight to either hours, minutes, seconds 
// ===================================================================================================== 
void displaySeconds(byte hour, byte minute, byte second, byte focus)
{
  CRGB secondBright = CRGB(120, 0, 0);
  if (second / 10 == 0)
  {
    byte coords[] = {
      6+1*11+7,
      6+1*11+8,
      6+1*11+9,
      6+2*11,
      6+2*11+4,
      6+3*11+6,
      6+3*11+10,
      6+4*11,
      6+4*11+1,
      6+4*11+4,
      6+5*11+6,
      6+5*11+8,
      6+5*11+10,
      6+6*11,
      6+6*11+3,
      6+6*11+4,
      6+7*11+6,
      6+7*11+10,
      6+8*11,
      6+8*11+4,
      6+9*11+7,
      6+9*11+8,
      6+9*11+9
    };
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second / 10 == 1)
  {
    byte coords[] = {
      6+1*11+6,
      6+1*11+7,
      6+1*11+8,
      6+1*11+9,
      6+1*11+10,
      6+2*11+2,
      6+3*11+8,
      6+4*11+2,
      6+5*11+8,
      6+6*11+2,
      6+7*11+8,
      6+7*11+10,
      6+8*11+1,
      6+8*11+2,
      6+9*11+8
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second / 10 == 2)
  {
    byte coords[] = {
      6+1*11+6,
      6+1*11+7,
      6+1*11+8,
      6+1*11+9,
      6+1*11+10,
      6+2*11,
      6+3*11+9,
      6+4*11+2,
      6+5*11+7,
      6+6*11+4,
      6+7*11+6,
      6+8*11,
      6+8*11+4,
      6+9*11+7,
      6+9*11+8,
      6+9*11+9
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second / 10 == 3)
  {
    byte coords[] = {
      6+1*11+7,
      6+1*11+8,
      6+1*11+9,
      6+2*11,
      6+2*11+4,
      6+3*11+6,
      6+4*11+4,
      6+5*11+7,
      6+6*11+2,
      6+7*11+7,
      6+8*11+4,
      6+9*11+6,
      6+9*11+7,
      6+9*11+8,
      6+9*11+9,
      6+9*11+10
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second / 10 == 4)
  {
    byte coords[] = {
      6+1*11+7,
      6+2*11+3,
      6+3*11+7,
      6+4*11+3,
      6+5*11+6,
      6+5*11+7,
      6+5*11+8,
      6+5*11+9,
      6+5*11+10,
      6+6*11,
      6+6*11+3,
      6+7*11+7,
      6+7*11+9,
      6+8*11+2,
      6+8*11+3,
      6+9*11+7
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second / 10 == 5)
  {
    byte coords[] = {
      6+1*11+9,
      6+1*11+8,
      6+1*11+7,
      6+2*11,
      6+2*11+4,
      6+3*11+6,
      6+4*11+4,
      6+5*11+6,
      6+6*11,
      6+6*11+1,
      6+6*11+2,
      6+6*11+3,
      6+7*11+10,
      6+8*11,
      6+9*11+6,
      6+9*11+7,
      6+9*11+8,
      6+9*11+9,
      6+9*11+10
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  if (second % 10 == 0)
  {
    byte coords[] = {
      6+1*11+1,
      6+1*11+2,
      6+1*11+3,
      6+9*11+1,
      6+9*11+2,
      6+9*11+3,
      6+3*11,
      6+5*11,
      6+7*11,
      6+3*11+4,
      6+5*11+4,
      6+7*11+4,
      6+2*11+6,
      6+4*11+6,
      6+6*11+6,
      6+8*11+6,
      6+2*11+10,
      6+4*11+10,
      6+6*11+10,
      6+8*11+10,
      6+4*11+7,
      6+5*11+2,
      6+6*11+9        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 1)
  {
    byte coords[] = {
      6+1*11,
      6+1*11+1,
      6+1*11+2,
      6+1*11+3,
      6+1*11+4,
      6+1*11+2,
      6+3*11+2,
      6+5*11+2,
      6+7*11+2,
      6+9*11+2,
      6+2*11+8,
      6+4*11+8,
      6+6*11+8,
      6+8*11+8,
      6+8*11+7,
      6+7*11+4        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 2)
  {
    byte coords[] = {
      6+1*11,
      6+1*11+1,
      6+1*11+2,
      6+1*11+3,
      6+1*11+4,
      6+2*11+6,
      6+3*11+3,
      6+4*11+8,
      6+5*11+1,
      6+6*11+10,
      6+7*11,
      6+8*11+6,
      6+8*11+10,
      6+9*11+1,
      6+9*11+2,
      6+9*11+3        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 3)
  {
    byte coords[] = {
      6+1*11+1,
      6+1*11+2,
      6+1*11+3,
      6+2*11+6,
      6+2*11+10,
      6+3*11,
      6+4*11+10,
      6+5*11+1,
      6+6*11+8,
      6+7*11+1,
      6+8*11+10,
      6+9*11,
      6+9*11+1,
      6+9*11+2,
      6+9*11+3,
      6+9*11+4        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 4)
  {
    byte coords[] = {
      6+1*11+1,
      6+3*11+1,
      6+5*11+1,
      6+7*11+1,
      6+9*11+1,
      6+2*11+9,
      6+4*11+9,
      6+6*11+9,
      6+8*11+9,
      6+5*11,
      6+5*11+1,
      6+5*11+2,
      6+5*11+3,
      6+5*11+4,
      6+6*11+6,
      6+7*11+3,
      6+8*11+8        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 5)
  {
    byte coords[] = {
      6+1*11+3,
      6+1*11+2,
      6+1*11+1,
      6+2*11+6,
      6+2*11+10,
      6+3*11,
      6+4*11+10,
      6+5*11,
      6+6*11+6,
      6+6*11+7,
      6+6*11+8,
      6+6*11+9,
      6+7*11+4,
      6+8*11+6,
      6+9*11,
      6+9*11+1,
      6+9*11+2,
      6+9*11+3,
      6+9*11+4        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 6)
  {
    byte coords[] = {
      6+1*11+3,
      6+1*11+2,
      6+1*11+1,
      6+2*11+6,
      6+2*11+10,
      6+3*11,
      6+3*11+4,
      6+4*11+6,
      6+4*11+10,
      6+5*11,
      6+5*11+4,
      6+6*11+6,
      6+6*11+7,
      6+6*11+8,
      6+6*11+9,
      6+7*11+4,
      6+8*11+6,
      6+9*11+3,
      6+9*11+2,
      6+9*11+1        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 7)
  {
    byte coords[] = {
      6+1*11+3,
      6+2*11+7,
      6+3*11+3,
      6+4*11+7,
      6+5*11+2,
      6+6*11+9,
      6+7*11,
      6+8*11+10,
      6+9*11,
      6+9*11+1,
      6+9*11+2,
      6+9*11+3,
      6+9*11+4        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 8)
  {
    byte coords[] = {
      6+1*11+3,
      6+1*11+2,
      6+1*11+1,
      6+2*11+6,
      6+2*11+10,
      6+3*11,
      6+3*11+4,
      6+4*11+6,
      6+4*11+10,
      6+5*11,
      6+5*11+4,
      6+6*11+7,
      6+6*11+8,
      6+6*11+9,
      6+7*11+4,
      6+7*11,
      6+8*11+6,
      6+8*11+10,
      6+9*11+3,
      6+9*11+2,
      6+9*11+1        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
  else if (second % 10 == 9)
  {
    byte coords[] = {
      6+1*11+3,
      6+1*11+2,
      6+2*11+9,
      6+3*11,
      6+4*11+10,
      6+5*11,
      6+6*11+7,
      6+6*11+8,
      6+6*11+9,
      6+6*11+10,
      6+7*11+4,
      6+7*11,
      6+8*11+6,
      6+8*11+10,
      6+9*11+3,
      6+9*11+2,
      6+9*11+1        
    };    
    for (int i = 0; i < sizeof(coords); i++)
    {
      PixelAdd(coords[i], secondBright);
    }    
  }
}
// =====================================================================================================
// DISPLAY THE TIME
//   - accepts focus, which applies a highlight to either hours, minutes, seconds 
// ===================================================================================================== 
void displayTime(byte hour, byte minute, byte second, byte focus) 
{
  CRGB hourBright = CRGB(000, 000, 040);                       // set hours   colour faint yellow
  CRGB minuteBright = CRGB(000, 000, 040);                     // set minutes colour faint orange

  switch (focus)                                                      // depending on FOCUS value
  {
    case 1:
      hourBright = CRGB(0, 0, 240);                        // set hours   colour bright yellow
      break;
    case 2:
      minuteBright = CRGB(0, 0, 240);                      // set minutes colour bright orange
      break;
    case 0:
      hourBright = CRGB(0, 0, 240);                        // set hours   colour bright yellow
      minuteBright = CRGB(0, 0, 240);                      // set minutes colour bright orange
      break;
  }
  byte thishour = hour;
  if ((minute % 5) > 0)
  {
    PixelAdd(dots[minute % 5 - 1], minuteBright);  
  }
  if (minute > 24)
  {
    thishour++;
    if (thishour == 24)
      thishour = 0;
  }
  if (minute < 5)
  {
    //DO NOTHING!!
  }
  else if (minute <10)
  {
    for (int i=0; i<five_len;i++)
    {
       PixelAdd(five+i, minuteBright);
    }    
    for (int i=0; i<past_len;i++)
    {
       PixelAdd(past+i, minuteBright);
    }    
  }
  else if (minute <15)
  {
    for (int i=0; i<ten_len;i++)
    {
       PixelAdd(ten+i, minuteBright);
    }    
    for (int i=0; i<past_len;i++)
    {
       PixelAdd(past+i, minuteBright);
    }    
  }
  else if (minute <20)
  {
    for (int i=0; i<quarter_len;i++)
    {
       PixelAdd(quarter+i, minuteBright);
    }    
    for (int i=0; i<past_len;i++)
    {
       PixelAdd(past+i, minuteBright);
    }    
  }
  else if (minute <25)
  {
    for (int i=0; i<twenty_len;i++)
    {
       PixelAdd(twenty+i, minuteBright);
    }    
    for (int i=0; i<past_len;i++)
    {
       PixelAdd(past+i, minuteBright);
    }    
  }
  else if (minute <30)
  {
    for (int i=0; i<five_len;i++)
    {
       PixelAdd(five+i, minuteBright);
    }    
    for (int i=0; i<to_len;i++)
    {
       PixelAdd(to+i, minuteBright);
    }    
    for (int i=0; i<half_len;i++)
    {
       PixelAdd(half+i, minuteBright);
    }    
  }
  else if (minute <35)
  {
    for (int i=0; i<half_len;i++)
    {
       PixelAdd(half+i, minuteBright);
    }    
  }
  else if (minute <40)
  {
    for (int i=0; i<five_len;i++)
    {
       PixelAdd(five+i, minuteBright);
    }    
    for (int i=0; i<past_len;i++)
    {
       PixelAdd(past+i, minuteBright);
    }    
    for (int i=0; i<half_len;i++)
    {
       PixelAdd(half+i, minuteBright);
    }      
  }
  else if (minute <45)
  {
    for (int i=0; i<twenty_len;i++)
    {
       PixelAdd(twenty+i, minuteBright);
    }    
    for (int i=0; i<to_len;i++)
    {
       PixelAdd(to+i, minuteBright);
    }        
  }
  else if (minute <50)
  {
    for (int i=0; i<quarter_len;i++)
    {
       PixelAdd(quarter+i, minuteBright);
    }    
    for (int i=0; i<to_len;i++)
    {
       PixelAdd(to+i, minuteBright);
    }        
  }
  else if (minute <55)
  {
    for (int i=0; i<ten_len;i++)
    {
       PixelAdd(ten+i, minuteBright);
    }    
    for (int i=0; i<to_len;i++)
    {
       PixelAdd(to+i, minuteBright);
    }        
  }
  else 
  {
    for (int i=0; i<five_len;i++)
    {
       PixelAdd(five+i, minuteBright);
    }    
    for (int i=0; i<to_len;i++)
    {
       PixelAdd(to+i, minuteBright);
    }        
  }

  //hour displaying
  if (thishour <= 12)
  {
    for (int i=0; i<hours_len[thishour];i++)
    {
       PixelAdd(hours[thishour]+i, hourBright);
    }
  }
  else if (thishour > 19)
  {
    for (int i=0; i<twentyh_len;i++)
    {
      PixelAdd(twentyh+i, hourBright);
    }
    if (thishour > 20)
    {
      for (int i=0; i<und_len;i++)
      {
        PixelAdd(und+i, hourBright);
      }
      for (int i=0; i<hours_len[thishour%10];i++)
      {
        PixelAdd(hours[thishour%10]+i, hourBright);
      }
    }
  }
  else
  {
    for (int i=0; i<hours_len[10];i++)
    {
      PixelAdd(hours[10]+i, hourBright);
    }
    for (int i=0; i<hours_len[thishour%10];i++)
    {
      PixelAdd(hours[thishour%10]+i, hourBright);
    }
  }

  for (int i=0;i<it_len;i++)
  {
    PixelAdd(it+i, hourBright);
  }

  for (int i=0;i<is_len;i++)
  {
    PixelAdd(is+i, hourBright);
  }
 
  for (int i=0;i<oclock_len;i++)
  {
    PixelAdd(oclock+i, hourBright);
  }
}

// =====================================================================================================
// show lsd effect
// =====================================================================================================
double r, g, b;
int time = 0;
#define TODISPLAY(x) ((x) + 1) * 120
void showPLASMA() {
  for(int y = 0; y < 11; y++)
    for(int x = 0; x < 11; x++)
    {
      double cx = x / (double) 22 + 0.5 * sin(time / 10.);
      double cy = y / (double) 22 + 0.5 * cos(time / 6.);
      double v = sin(sqrt(100 * (cx*cx+cy*cy) + 1) + time / 2.);

      switch(time / 100)
      {
        case 0:
          r = 1;
          g = cos(v * PI);
          b = sin(v * PI);
          break;
          
        case 1:
          r = sin(v * PI);
          g = cos(v * PI);
          b = 0;
          break;
          
        case 2:
          r = sin(v * PI);
          b = cos(v * PI);
          g = 0;
          //g = sin(v * PI + 2 * PI / 3);
          //b = sin(v * PI + 4 * PI / 3);
          break;
          
        case 4:
          time = 0;  // note no break

        case 3:
          r = g = b = sin(v * .75 * PI);
          break;
      }

      r = TODISPLAY(r);
      g = TODISPLAY(g);
      b = TODISPLAY(b);
      //setPixel(x, y, leds.Color(r, g, b));
      setPixel(x, y, CRGB(r, g, b));
    }
    time++;
}
// =====================================================================================================
// show matrix effect
// =====================================================================================================
void showMatrix()
{
  int rnd = random(11);
  if (matrix[rnd] == 14)
  {
    matrix[rnd] = 0;
  }
  for (int i = 0; i < 11; i++)
  {
    if (matrix[i] < 14)
    {
      for (byte ii = 0; ii < 4; ii++)
      {
        byte y = matrix[i] - ii;
        if (y < 11) 
        {
          //setPixel(i,10-y, leds.Color(0, 160-ii*50, 0));
          setPixel(i,10-y, CRGB(0, 160-ii*50, 0));
        }
      }
      matrix[i]++;
    }
  }
}

byte gHue = 0;
// =====================================================================================================
// show rainbow effect
// =====================================================================================================
void showRainbow()
{
  for (int i=0;i<6;i++)
  {
    for (int x=5-i;x<6+i;x++)
    {
      setPixel(x,5-i, Wheel(gHue-i*20));
      setPixel(x,5+i, Wheel(gHue-i*20));
    }
    for (int y=5-i;y<6+i;y++)
    {
      setPixel(5-i,y, Wheel(gHue-i*20));
      setPixel(5+i,y, Wheel(gHue-i*20));
    }
  }
  gHue+=5;
}


// =====================================================================================================
// showNoise
// =====================================================================================================
uint16_t scale = 311;

// This is the array that we keep our computed noise values in
uint8_t noise[11][11];

static uint16_t x= random16();;
static uint16_t y= random16();;
static uint16_t z= random16();;
  
void fillnoise8() {
  for(int i = 0; i < 11; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < 11; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset,y + joffset,z);
    }
  }
  z += 20;
}

void showNoise()
{
  static uint8_t ihue=0;
  fillnoise8();
  for(int i = 0; i < 11; i++) {
    for(int j = 0; j < 11; j++) {
      setPixel(i,j, CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]));
    }
  }
  ihue+=1;  
}


// =====================================================================================================
// showNoise
// =====================================================================================================


float speed = .1; // speed of the movement along the Lissajous curves
float size = 3;    // amplitude of the curves

void showSinusoids()
{
  for (uint8_t y = 0; y < 11; y++) {
    for (uint8_t x = 0; x < 11; x++) {

      float cx = y + float(size * (sinf (float(speed * 0.003 * float(millis() ))) ) ) - 8;  // the 8 centers the middle on a 16x16
      float cy = x + float(size * (cosf (float(speed * 0.0022 * float(millis()))) ) ) - 8;
      float v = 127 * (1 + sinf ( sqrtf ( ((cx * cx) + (cy * cy)) ) ));
      uint8_t data = v;
      CRGB pixel;
      pixel.r = data;

      cx = x + float(size * (sinf (speed * float(0.0021 * float(millis()))) ) ) - 8;
      cy = y + float(size * (cosf (speed * float(0.002 * float(millis() ))) ) ) - 8;
      v = 127 * (1 + sinf ( sqrtf ( ((cx * cx) + (cy * cy)) ) ));
      data = v;
      pixel.b = data;

      cx = x + float(size * (sinf (speed * float(0.0041 * float(millis() ))) ) ) - 8;
      cy = y + float(size * (cosf (speed * float(0.0052 * float(millis() ))) ) ) - 8;
      v = 127 * (1 + sinf ( sqrtf ( ((cx * cx) + (cy * cy)) ) ));
      data = v;
      pixel.g = data;
      setPixel(x,y,pixel);
    }
  }
}

// =====================================================================================================
// clear all leds
// =====================================================================================================
void clearLeds(CRGB color)
{
  for (int i=0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}
// =====================================================================================================
// calculate Pixel Values
// =====================================================================================================
void PixelAdd(byte index, CRGB color)
{
  //TODO maybe better algo
  leds[index] |= color;
}
// =====================================================================================================
// GEt Pixel Index
// =====================================================================================================
byte XY(byte x, byte y)
{
  if (y % 2 == 1)
  {
    return 6+y*11+x;
  }
  return 6+y*11+10-x;
}
// =====================================================================================================
// Set Pixel Values
// =====================================================================================================
void setPixel(byte x, byte y, CRGB color)
{
  byte index;
  if (y % 2 == 1)
  {
    index = 6+y*11+x;
  }
  else
  {
    index = 6+y*11+10-x;
  }
  leds[index] = color;
}
// =====================================================================================================
// Update the rings
// =====================================================================================================
void showLeds()
{
//TODO: calculate average of 15 seconds using modulo and change ldr_val by 1 if not equal to current ldr_val
  int ldr_val = 0;
  for (int i=0;i<10;i++)
    ldr_val += analogRead(LDR_PIN);
  ldr_val /= 10;
  if (ldr_val < 600)
  {
    ldr_val = 600;
  }
  ldr_val -= 600;
  ldr_val /= 4;
  if (ldr_val > 100)
    ldr_val = 100;
  if (ldr_val < 1)
    ldr_val = 1;
  
  //range: 0 - 100
//  String temp;
// temp.concat("LDR ") ;
//  temp.concat(ldr_val);
//  Serial.println(temp);
  LEDS.setBrightness(255 * ldr_val / 100); 
  FastLED.show(); 
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
CRGB Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
