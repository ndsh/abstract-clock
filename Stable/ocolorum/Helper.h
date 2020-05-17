// Include libraries
#include <Adafruit_NeoPixel.h>
#include <AnalogMultiButton.h>
#include <Wire.h>

// Pin definitions
#define BUTTONSPIN A0
#define SENSORPIN A1
#define LEDPIN 6

// Variables
#define NUMPIXELS 10
#define DS1307_ADRESSE 0x68 // I2C Addresse
#define DEPLOY false

// Read-only
const bool debug = false;
const int buttonsTotal = 3;
const int buttonsValues[buttonsTotal] = {0, 317, 486};
const int BUTTON_SELECT = 0;
const int BUTTON_DOWN = 1;
const int BUTTON_UP = 2;

// Globals
bool setupMode;
float correction = 2.0; // gamma correction for night time
int mSecond, mPreviousSecond, mMinute, mHour, mDay, mWeekday, mMonth, mYear;
int nextHour = 0;
int currentHour = 0;
int mCurrentSeconds = 0;

int deltaNoon = 0;
int theOdd = 0;
boolean beforeNoon = false;
int red = 0;
int green = 0;
int blue = 0;

// variables for fading the lights in setup mode
float setupCorrection = 0.0;
bool setupDir = true;
float setupCorrectionMin = 0.4;
float setupCorrectionMax = 0.8;
float setupStepLength = 0.001; // ohne serial print is ganz geil

// Objects
Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
AnalogMultiButton buttons(BUTTONSPIN, buttonsTotal, buttonsValues);

// Arrays
extern const uint8_t gamma[];
const uint8_t PROGMEM hours2color[12][3] = {
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

// Functions
byte decToBcd(byte val) {
  return ((val/10*16) + (val - 10 * (val / 10)));
}
byte bcdToDec(byte val) {
  return ((val/16*10) + (val - 16 * (val / 16)));
}

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

boolean getPositive(int a, int b) {
  return (b-a>0?true:false);
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

int getNewValue(int step, int a, int b) {
  if(getPositive(a, b)) {
    return (a+getNewPosition(step, a, b));
  } else {
    return (a-getNewPosition(step, a, b));
  }
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

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setPixelColorWrapper(uint8_t pixel, int r, int g, int b, int currentHour) {
  r =  pgm_read_byte(&gamma[r]);
  g =  pgm_read_byte(&gamma[g]);
  b =  pgm_read_byte(&gamma[b]);
  if( (currentHour >= 22 && currentHour <= 23) || (currentHour >= 0 && currentHour <= 2)) {
    int sensorValue = analogRead(SENSORPIN);
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

void RTCinput(int mPressed) {
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
}

void RTCoutput(){
  // initialize and point at head
  // read/write is normal since Wire 1.0

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
}
