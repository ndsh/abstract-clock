/*
replace modulos: http://embeddedgurus.com/stack-overflow/2011/02/efficient-c-tip-13-use-the-modulus-operator-with-caution/
*/

/*
 * ok wichtig. PULLUPS benutzen für SDA/SCL
 * und: entweder einen ersatz für ds1307 lib finden oder komplett mit TinyWire
 * den stream auslesen
 */

 /*
 * attiny85:
 * [x] d1: leds
 * d0: sda
 * d2: scl
 * d4: button3
 * d3: button2
 * d5: button1
 */

 /* todo list: 
  [] photo resistor
  [] start up sequence
  [] secret button codes
  [] replace modulo operations
  [] better setup mode sequence (fade?)
*/

// IMPORTS

#include <Adafruit_NeoPixel.h>
#include <Button.h>
#if defined (__AVR_ATtiny85__)
  #include <TinyWireM.h>
#else
  #include <Wire.h>
#endif

//READ ONLY
const uint8_t mLeds= 6;
const uint8_t NUMPIXELS = 10;
#define DS1307_ADRESSE 0x68 // I2C Addresse
const boolean debug = false;

// GLOBAL
bool setupMode;

unsigned long pixelsInterval = 200; // the time we need to wait
unsigned long colorWipePreviousMillis = 0;
uint16_t currentPixel = 0; // what pixel are we operating on
float correction = 2.0; // gamma correction for night time

uint8_t mSecond, mMinute, mHour, mDay, mWeekday, mMonth, mYear;


// OBJECTS
Button button1(2);
Button button2(3);
Button button3(4);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, mLeds, NEO_GRB + NEO_KHZ800);

// ARRAYS
extern const uint8_t gamma[];
uint8_t hours2color[12][3] = {
  { 20, 24, 39 },
  { 6, 30, 64 },
  { 8, 41, 92 },
  { 80, 68, 118 },
  { 170, 134, 162 },
  { 216, 189, 194 },
  { 223, 203, 202 },
  { 215, 200, 203 },
  { 188, 195, 203 },
  { 134, 181, 201 },
  { 20, 163, 193 },
  { 2, 154, 195 }
};




void setup() { 
//  Serial.begin(9600);

  // initialize sub-routines
  pixels.begin();
  button1.begin();
  button2.begin();
  button3.begin();

  setupMode = false;
  currentPixel = 0;

  // START UP SEQUENCE
  
  // fade in "white"
  for(int j = 0; j<255; j++) {
    for(int i = 0; i<NUMPIXELS; i++) pixels.setPixelColor(i, pixels.Color(j,j,j));
    pixels.show();
  }
  
  // flash through all of the colors. BAM! BAM! BAM!
  for(int j = 0; j<12; j++) {
    for(int i = 0; i<NUMPIXELS; i++) setPixelColorWrapper(i, hours2color[j][0], hours2color[j][2], hours2color[j][2], 12); // just set currentHour to 12, so we get full brightness here
    pixels.show();
  }
  // and one more for the reverse colors
  for(int j = 12; j>=0; j--) {
    for(int i = 0; i<NUMPIXELS; i++) setPixelColorWrapper(i, hours2color[j][0], hours2color[j][2], hours2color[j][2], 12); // just set currentHour to 12, so we get full brightness here
    pixels.show();
  }

  // maybe fade out
  // and then fade to the currentHours array?
  
}

void loop() {
  uint8_t nextHour = 0;
  uint8_t currentHour = 0;
  uint8_t mCurrentSeconds = 0;
  uint8_t deltaNoon = 0;
  uint8_t theOdd = 0;
  boolean beforeNoon = false;
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  
  if (button1.pressed()) {
    setupMode = !setupMode;
    if(setupMode == true) {
      for(int i = 0; i<NUMPIXELS; i++) pixels.setPixelColor(i,0,0,0);
      pixels.show();
    }
  }
  if(!debug) {
    RTCoutput();
  } else {
    mSecond += 10;
    if(mSecond >= 60) {
      mSecond = 0;
      mMinute++;
    }
    if(mMinute >= 60) {
      mMinute = 0;
      mHour++;
    }
    if(mHour >= 24) {
      mHour = 0;
    }
  }

  /* ++++++++++++++++++++++++++++++++++++
  //
  //            SETUP MODE
  //
  // +++++++++++++++++++++++++++++++++ */
  if(setupMode) {
    uint8_t mPressed = 0;
    if (button2.pressed()) {
      mPressed = -1;
    }
    if (button3.pressed()) {
      mPressed = 1;
    }
    if(mPressed != 0) {
      #if defined (__AVR_ATtiny85__)
      TinyWireM.beginTransmission(DS1307_ADRESSE);
      #else
      Wire.beginTransmission(DS1307_ADRESSE);
      #endif
      
      wireWrite(0x00);
      wireWrite(decToBcd(mSecond));
      wireWrite(decToBcd(mMinute));
      if(mPressed == -1) wireWrite(decToBcd(getHour(mHour,false)));
      else if(mPressed == 1) wireWrite(decToBcd(getHour(mHour,true)));
      wireWrite(decToBcd(mWeekday));
      wireWrite(decToBcd(mDay));
      wireWrite(decToBcd(mMonth));
      wireWrite(decToBcd(mYear));
      wireWrite(0x00);
      #if defined (__AVR_ATtiny85__)
      TinyWireM.endTransmission();
      #else
      Wire.endTransmission();
      #endif
      RTCoutput(); // korrekt hier? oder außerbalb der schleife
    }

    beforeNoon = (mHour<=11?true:false);
    currentHour = mHour;
    nextHour = (currentHour+1)%12;
    if(nextHour == 0) nextHour = currentHour;
    
    if(!beforeNoon) {
      deltaNoon = (mHour-12)+1;
      theOdd = (deltaNoon*2)-1;
      currentHour = mHour-theOdd;
      nextHour = (currentHour-1)%24;
      if(nextHour == -1) nextHour = 0;    
    }  
    
    if ((unsigned long)(millis() - colorWipePreviousMillis) >= pixelsInterval) {
      colorWipePreviousMillis = millis();
      red = hours2color[currentHour][0];
      green = hours2color[currentHour][1];
      blue = hours2color[currentHour][2];
      setPixelColorWrapper(currentPixel, red, green, blue, currentHour);
      pixels.show();
      int j = currentPixel;
      j--;
      if(j==-1) j = 9;
      pixels.setPixelColor(j,0,0,0);
      pixels.show();
      currentPixel++;
      if(currentPixel == NUMPIXELS) currentPixel = 0;
    }
  /* ++++++++++++++++++++++++++++++++++++
  //
  //            NORMAL MODE
  //
  // +++++++++++++++++++++++++++++++++ */
  } else {
    mCurrentSeconds = (mMinute*60)+mSecond;
    beforeNoon = (mHour<=11?true:false);
    currentHour = mHour;
    nextHour = (currentHour+1)%12;
    if(nextHour==0) nextHour = currentHour;
    
    if(!beforeNoon) {
      deltaNoon = (mHour-12)+1;
      theOdd = (deltaNoon*2)-1;
      currentHour = mHour-theOdd;
      nextHour = (currentHour-1)%24;
      if(nextHour == -1) nextHour = 0;    
    }
    red = getNewValue(mCurrentSeconds, hours2color[currentHour][0], hours2color[nextHour][0]);
    green = getNewValue(mCurrentSeconds, hours2color[currentHour][1], hours2color[nextHour][1]);
    blue = getNewValue(mCurrentSeconds, hours2color[currentHour][2], hours2color[nextHour][2]);
    
    for(int i = 0; i<NUMPIXELS; i++) setPixelColorWrapper(i, red, green, blue, currentHour);
    pixels.show();
  }
}

void setPixelColorWrapper(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t currentHour) {
  r =  pgm_read_byte(&gamma[r]);
  g =  pgm_read_byte(&gamma[g]);
  b =  pgm_read_byte(&gamma[b]);
  if( (currentHour >= 22 && currentHour <= 23) || (currentHour >= 0 && currentHour <= 2)) {
    r *= correction;
    g *= correction;
    b *= correction;
  }
  pixels.setPixelColor(pixel, r, g, b);
}

boolean getPositive(uint8_t a, uint8_t b) {
  return (b-a>0?true:false);
}

uint8_t getNewPosition(uint8_t step, uint8_t a, uint8_t b) {
  float mBucket = 0;
  float mCalculatedStep = (abs(b-a))/3600.0;
  int mNewPosition = 0;
  for(int i = 0; i<step;i++) {
    mBucket += mCalculatedStep;
    if(mBucket >= 1) {
      mBucket -= 1.0;
      mNewPosition += 1;
    }
  }
  return mNewPosition;
}

uint8_t getNewValue(int step, int a, int b) {
  if(getPositive(a, b)) {
    return (a+getNewPosition(step, a, b));
  } else {
    return (a-getNewPosition(step, a, b));
  }
}

uint8_t getHour(uint8_t _mHour, bool dir) {
  if(dir) {
    _mHour++;
    _mHour%=24;
  } else {
    _mHour--;
    if(_mHour == -1) _mHour = 23;
  }
  return _mHour;
}

void setHour(int _mHour) {
  mHour = _mHour;
}

uint8_t getMinute(uint8_t _mMinute, uint8_t _mHour, bool dir) {
  if(dir) {
    _mMinute++;
    if(_mMinute%60 == 0) {
      _mMinute%=60;
      setHour(getHour(_mHour, true));
    }
  } else {
    _mMinute--;
    if(_mMinute == -1) {
        _mMinute = 59;
        setHour(getHour(_mHour, false));
      }
  }
  return _mMinute;
}

void RTCoutput(){
  // initialize and point at head
  #if defined (__AVR_ATtiny85__)
  TinyWireM.beginTransmission(DS1307_ADRESSE);
  #else
  Wire.beginTransmission(DS1307_ADRESSE);
  #endif
  wireWrite(0x00);
  #if defined (__AVR_ATtiny85__)
  TinyWireM.endTransmission();
  #else
  Wire.endTransmission();
  #endif

  #if defined (__AVR_ATtiny85__)
  TinyWireM.requestFrom(DS1307_ADRESSE, 7);
  #else
  Wire.requestFrom(DS1307_ADRESSE, 7);
  #endif
 
  mSecond = bcdToDec(wireRead());
  mMinute = bcdToDec(wireRead());
  mHour = bcdToDec(wireRead() & 0b111111);
  mWeekday = bcdToDec(wireRead());
  mDay = bcdToDec(wireRead());
  mMonth = bcdToDec(wireRead());
  mYear = bcdToDec(wireRead());
}
// Helpers
byte decToBcd(byte val) {
  return ((val/10*16) + (val%10));
}
byte bcdToDec(byte val) {
  return ((val/16*10) + (val%16));
}

int wireRead() {
  #if defined (__AVR_ATtiny85__)
  TinyWireM.receive();
  #else
  Wire.read();
  #endif
  
}

void wireWrite(byte b) {
  #if defined (__AVR_ATtiny85__)
  TinyWireM.send(b);
  #else
  Wire.write(b);
  #endif
  
}

const uint8_t PROGMEM gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
