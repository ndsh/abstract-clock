/*
  replace modulos: http://embeddedgurus.com/stack-overflow/2011/02/efficient-c-tip-13-use-the-modulus-operator-with-caution/
*/

/*
  ok wichtig. PULLUPS benutzen für SDA/SCL
  und: entweder einen ersatz für ds1307 lib finden oder komplett mit TinyWire
  den stream auslesen
*/
 
/* buttons:
  http://damienclarke.me/code/analog-multi-button
 */

 /* todo list: 
  [ ] photo resistor
  [x] start up sequence
  [x] replace modulo operations
  [x] better setup mode sequence (fade?)
  [x] analog input buttons
  [/] serial println wrapper
  [-] in debug: show whenever color changes to the next value
  [ ] replace floats
  [x] loop only every second (millis timer)
  
  [ ] secret button codes
  [ ] put functions into a proper library
*/

/*
  give your modulo operations a boost:
  C = A % B is equivalent to C = A - B * (A / B)
 */

/*
  photores:
  analogRead(mSensorPin)/4);  //you have  to divide the value. for example, 
                            //with a 10k resistor divide the value by 2, for 100k resistor divide by 4.
*/

// IMPORTS
#include <Adafruit_NeoPixel.h>
#include <AnalogMultiButton.h>
#if defined (__AVR_ATtiny85__)
  #include <TinyWireM.h>
#else
  #include <Wire.h>
#endif

// PINS
#if defined (__AVR_ATtiny85__)
const int mButtonsPin = A3;
const uint8_t mSensorPin = A2;
const uint8_t mLedsPin = 1;
#else
const int mButtonsPin = A0;
const uint8_t mSensorPin = A1;
const uint8_t mLedsPin = 6;
#endif

// READ ONLY
const uint8_t NUMPIXELS = 10;
#define DS1307_ADRESSE 0x68 // I2C Addresse
#define DEPLOY false
const bool debug = false;
const int mButtonsTotal = 3;
const int mButtonsValues[mButtonsTotal] = {0, 317, 486};
const int BUTTON_SELECT = 0;
const int BUTTON_DOWN = 1;
const int BUTTON_UP = 2;
const bool startup = true;



// GLOBAL
bool setupMode;
float correction = 2.0; // gamma correction for night time
int mSecond, mPreviousSecond, mMinute, mHour, mDay, mWeekday, mMonth, mYear;
int nextHour = 0;
int currentHour = 0;
int mCurrentSeconds = 0;

// variables for fading the lights in setup mode
float setupCorrection = 0.0;
bool setupDir = true;
float setupCorrectionMin = 0.4;
float setupCorrectionMax = 0.8;
float setupStepLength = 0.001; // ohne serial print is ganz geil

// OBJECTS
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, mLedsPin, NEO_GRB + NEO_KHZ800);
AnalogMultiButton buttons(mButtonsPin, mButtonsTotal, mButtonsValues);

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
  #if defined (__AVR_ATtiny85__)
    TinyWireM.begin();
  #else
    Serial.begin(9600);
    SerialPrintln("### welcome to ocolorum");
    Wire.begin();
    RTCoutput();
    SerialPrint(mHour);
    SerialPrint(":");
    SerialPrintln(mMinute);
    SerialPrint("---------");
  #endif

  // initialize sub-routines
  pixels.begin();

  setupMode = false;

  // START UP SEQUENCE
  if(startup) {
    // fade in "white"
    for(int j = 0; j<125; j++) {
      for(int i = 0; i<NUMPIXELS; i++) pixels.setPixelColor(i, pixels.Color(j,j,j));
      pixels.show();
      delay(20);
    }    
  }
}

void loop() {
  buttons.update();
  
  int deltaNoon = 0;
  int theOdd = 0;
  boolean beforeNoon = false;
  int red = 0;
  int green = 0;
  int blue = 0;
  
  if(buttons.onRelease(BUTTON_SELECT)) {
    setupMode = !setupMode;
    if(setupMode == true) {
      for(int i = 0; i<NUMPIXELS; i++) pixels.setPixelColor(i,0,0,0);
      pixels.show();
      setupDir = true;
      setupCorrection = 0.0;
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
    if(mHour >= 24) mHour = 0;
  }
  /* ++++++++++++++++++++++++++++++++++++
  //
  //            SETUP MODE
  //
  // +++++++++++++++++++++++++++++++++ */
  if(setupMode) {
    int mPressed = 0;
    
    if(buttons.onRelease(BUTTON_DOWN)) { // down button
      mPressed = -1;
      SerialPrint("down: ");
    }
    if(buttons.onRelease(BUTTON_UP)) { // up button
      mPressed = 1;
      SerialPrint("up: ");
    }
    if(mPressed != 0) {
      RTCinput(mPressed);
      RTCoutput(); // korrekt hier? oder außerbalb der schleife
      SerialPrint("new time: ");
      SerialPrint(mHour);
      SerialPrint(":");
      SerialPrintln(mMinute);
    }
    calculateNewTime();
    red = hours2color[currentHour][0]*setupCorrection;
    green = hours2color[currentHour][1]*setupCorrection;
    blue = hours2color[currentHour][2]*setupCorrection;
    for(int i = 0; i<NUMPIXELS; i++) setPixelColorWrapper(i, red, green, blue, mHour);
    pixels.show();
    if(setupDir) {
      setupCorrection += setupStepLength;
      if(setupCorrection >= setupCorrectionMax) {
        setupDir = false;
        setupCorrection = setupCorrectionMax;
      }
    }
    else {
      setupCorrection -= setupStepLength;
      if(setupCorrection < setupCorrectionMin) {
        setupDir = true;
        setupCorrection = setupCorrectionMin;
      }
    }
  /* ++++++++++++++++++++++++++++++++++++
  //
  //            NORMAL MODE
  //
  // +++++++++++++++++++++++++++++++++ */
  } else {
    if(mPreviousSecond != mSecond) {
      calculateNewTime();
      red = getNewValue(mCurrentSeconds, hours2color[currentHour][0], hours2color[nextHour][0]);
      green = getNewValue(mCurrentSeconds, hours2color[currentHour][1], hours2color[nextHour][1]);
      blue = getNewValue(mCurrentSeconds, hours2color[currentHour][2], hours2color[nextHour][2]);
      
      for(int i = 0; i<NUMPIXELS; i++) setPixelColorWrapper(i, red, green, blue, mHour);
      pixels.show();
      mPreviousSecond = mSecond;
    }
  }
}

/* ############################## */
/* ######### FUNCTIONS ########## */
/* ############################## */
void calculateNewTime() {
  mCurrentSeconds = (mMinute*60)+mSecond;
  int beforeNoon = (mHour<=11?true:false);
  currentHour = mHour;
  nextHour = (currentHour+1)-12*((currentHour+1)/12);
  if(nextHour==0) nextHour = currentHour;
  
  if(!beforeNoon) {
    int deltaNoon = (mHour-12)+1;
    int theOdd = (deltaNoon*2)-1;
    currentHour = mHour-theOdd;
    nextHour = (currentHour-1)-24*((currentHour-1)/24);    
    if(nextHour == -1) nextHour = 0;    
  }
}
int getNewValue(int step, int a, int b) {
  if(getPositive(a, b)) {
    return (a+getNewPosition(step, a, b));
  } else {
    return (a-getNewPosition(step, a, b));
  }
}
int getNewPosition(int step, int a, int b) {
  float mBucket = 0.0;
  float mCalculatedStep = ((float)abs(b-a))/3600.0;
  int mNewPosition = 0;
  for(int i = 0; i<step; i++) {
    mBucket += mCalculatedStep;
    if(mBucket >= 1.0) {
      mBucket -= 1.0;
      mNewPosition += 1;
    }
  }
  return mNewPosition;
}
boolean getPositive(int a, int b) {
  return (b-a>0?true:false);
}
int getHour(int _mHour, bool dir) {
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
int getMinute(int _mMinute, int _mHour, bool dir) {
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
void setPixelColorWrapper(uint8_t pixel, int r, int g, int b, int currentHour) {
  r =  pgm_read_byte(&gamma[r]);
  g =  pgm_read_byte(&gamma[g]);
  b =  pgm_read_byte(&gamma[b]);
  if( (currentHour >= 22 && currentHour <= 23) || (currentHour >= 0 && currentHour <= 2)) {
    int sensorValue = analogRead(mSensorPin);
    if(sensorValue < 100) {
      //SerialPrint("sensing mode : ");
      //SerialPrintln(sensorValue);
      float gammaRange = mapfloat((float)sensorValue, 0.0,1023.0,0.0,1.0);
      r *= gammaRange;
      g *= gammaRange;
      b *= gammaRange;
    } else {
      //SerialPrintln("normal mode");
      r *= correction;
      g *= correction;
      b *= correction;
    }
  }
  pixels.setPixelColor(pixel, r, g, b);
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void RTCinput(int mPressed) {
  #if defined (__AVR_ATtiny85__)
      TinyWireM.beginTransmission(DS1307_ADRESSE);
      TinyWireM.send(0x00);
      TinyWireM.send(decToBcd(mSecond));
      TinyWireM.send(decToBcd(mMinute));
      if(mPressed == -1) TinyWireM.send(decToBcd(getHour(mHour,false)));
      else if(mPressed == 1) TinyWireM.send(decToBcd(getHour(mHour,true)));
      TinyWireM.send(decToBcd(mWeekday));
      TinyWireM.send(decToBcd(mDay));
      TinyWireM.send(decToBcd(mMonth));
      TinyWireM.send(decToBcd(mYear));
      TinyWireM.send(0x00);
      TinyWireM.endTransmission();
    #else
      Wire.beginTransmission(DS1307_ADRESSE);
      Wire.write(0x00);
      Wire.write(decToBcd(mSecond));
      Wire.write(decToBcd(mMinute));
      if(mPressed == -1) Wire.write(decToBcd(getHour(mHour,false)));
      else if(mPressed == 1) Wire.write(decToBcd(getHour(mHour,true)));
      Wire.write(decToBcd(mWeekday));
      Wire.write(decToBcd(mDay));
      Wire.write(decToBcd(mMonth));
      Wire.write(decToBcd(mYear));
      Wire.write(0x00);
      Wire.endTransmission();
    #endif
}
void RTCoutput(){
  // initialize and point at head
  // read/write is normal since Wire 1.0
  #if defined (__AVR_ATtiny85__)
    TinyWireM.beginTransmission(DS1307_ADRESSE);
    TinyWireM.write(0x00);
    TinyWireM.endTransmission();
   
    TinyWireM.requestFrom(DS1307_ADRESSE, 7);
   
    mSecond = bcdToDec(TinyWireM.read());
    mMinute = bcdToDec(TinyWireM.read());
    mHour = bcdToDec(TinyWireM.read() & 0b111111);
    mWeekday = bcdToDec(TinyWireM.read());
    mDay = bcdToDec(TinyWireM.read());
    mMonth = bcdToDec(TinyWireM.read());
    mYear = bcdToDec(TinyWireM.read());
  #else
    Wire.beginTransmission(DS1307_ADRESSE);
    Wire.write(0x00);
    Wire.endTransmission();
   
    Wire.requestFrom(DS1307_ADRESSE, 7);
   
    mSecond = bcdToDec(Wire.read());
    mMinute = bcdToDec(Wire.read());
    mHour = bcdToDec(Wire.read() & 0b111111);
    mWeekday = bcdToDec(Wire.read());
    mDay = bcdToDec(Wire.read());
    mMonth = bcdToDec(Wire.read());
    mYear = bcdToDec(Wire.read());
  #endif
}
byte decToBcd(byte val) {
  return ((val/10*16) + (val - 10 * (val / 10)));
}
byte bcdToDec(byte val) {
  return ((val/16*10) + (val - 16 * (val / 16)));
}
byte wireRead() {
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
void SerialPrint(int i) {
  #if defined (__AVR_ATtiny85__)
  #else
    Serial.print(i);
  #endif
}
void SerialPrint(byte b) {
  #if defined (__AVR_ATtiny85__)
  #else
    Serial.print(b);
  #endif
}
void SerialPrint(String s) {
  #if defined (__AVR_ATtiny85__)
  #else
    Serial.print(s);
  #endif
}
void SerialPrintln(String s) {
  #if defined (__AVR_ATtiny85__)
  #else
    Serial.println(s);
  #endif
}
void SerialPrintln(int i) {
  #if defined (__AVR_ATtiny85__)
  #else
    Serial.println(i);
  #endif
}
void SerialPrintln(float f) {
  #if defined (__AVR_ATtiny85__)
  #else
    Serial.println(f);
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
