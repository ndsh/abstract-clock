/*
replace modulos: http://embeddedgurus.com/stack-overflow/2011/02/efficient-c-tip-13-use-the-modulus-operator-with-caution/
*/

/*
 * ok wichtig. PULLUPS benutzen für SDA/SCL
 * und: entweder einen ersatz für ds1307 lib finden oder komplett mit TinyWire
 * den stream auslesen
 */
 /*
  * 1st button: under 800
  * 2nd button: >= 800 && < 1000
  * 3rd button: > 1000
  */

  /* buttons:
     http://damienclarke.me/code/analog-multi-button
   */
 /*

                               +-----+
         +----[PWR]-------------------| USB |--+
         |                            +-----+  |
         |         GND/RST2  [ ][ ]            |
         |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] |   C5 
         |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   C4 
         |                             AREF[ ] |
         |                              GND[ ] |
         | [ ]N/C                    SCK/13[ ] |   B5
         | [ ]IOREF                 MISO/12[ ] |   .
         | [ ]RST                   MOSI/11[ ]~|   .
         | [ ]3V3    +---+               10[ ]~|   .
         | [ ]5v    -| A |-               9[ ]~|   .
         | [ ]GND   -| R |-               8[ ] |   B0
         | [X]GND   -| D |-                    |
         | [X]Vin   -| U |-               7[ ] |   D7
         |          -| I |-               6[X]~|   .
         | [/]A0    -| N |-               5[ ]~|   .
         | [ ]A1    -| O |-               4[X] |   .
         | [ ]A2     +---+           INT1/3[X]~|   .
         | [ ]A3                     INT0/2[X] |   .
         | [X]A4/SDA  RST SCK MISO     TX>1[ ] |   .
         | [X]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0
         |            [ ] [ ] [ ]              |
         |  UNO_R3    GND MOSI 5V  ____________/
          \_______________________/
*/

 /*
 * attiny85:
 * d1: leds
 * d0 : sda
 * d2 : scl
 * d4 : button3
 * d3 : button2
 * d5 : button1
 * d? : photo resistor
 */

 /* todo list: 
  [ ] photo resistor
  [/] start up sequence
  [ ] secret button codes
  [ ] replace modulo operations
  [ ] better setup mode sequence (fade?)
  [ ] shift register for buttons
*/

/* photores
  analogRead(lightPin)/4);  //you have  to divide the value. for example, 
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

//READ ONLY
const uint8_t mLeds= 6;
const uint8_t mButtons = A0;
const uint8_t NUMPIXELS = 10;
#define DS1307_ADRESSE 0x68 // I2C Addresse
const boolean debug = false;
const int BUTTONS_PIN = A0;
const int BUTTONS_TOTAL = 3;
const int BUTTONS_VALUES[BUTTONS_TOTAL] = {0, 317, 486};
const int BUTTON_SELECT = 0;
const int BUTTON_DOWN = 1;
const int BUTTON_UP = 2;
const int startup = false;

// GLOBAL
bool setupMode;

unsigned long pixelsInterval = 200; // the time we need to wait
unsigned long colorWipePreviousMillis = 0;
uint16_t currentPixel = 0; // what pixel are we operating on
float correction = 2.0; // gamma correction for night time

uint8_t mSecond, mMinute, mDay, mWeekday, mMonth, mYear;
int mHour;


// OBJECTS
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, mLeds, NEO_GRB + NEO_KHZ800);
AnalogMultiButton buttons(BUTTONS_PIN, BUTTONS_TOTAL, BUTTONS_VALUES);

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
    Wire.begin();
    RTCoutput();
    Serial.print(mHour);
    Serial.print(":");
    Serial.println(mMinute);
  #endif
  

  // initialize sub-routines
  pixels.begin();

  setupMode = false;
  currentPixel = 0;

  // START UP SEQUENCE
  if(startup) {
    // fade in "white"
    for(int j = 0; j<255; j++) {
      for(int i = 0; i<NUMPIXELS; i++) pixels.setPixelColor(i, pixels.Color(j,j,j));
      pixels.show();
      delay(20);
    }
    
    // flash through all of the colors. BAM! BAM! BAM!
    for(int j = 0; j<12; j++) {
      for(int i = 0; i<NUMPIXELS; i++) setPixelColorWrapper(i, hours2color[j][0], hours2color[j][2], hours2color[j][2], 12); // just set currentHour to 12, so we get full brightness here
      pixels.show();
      delay(100);
    }
  
    // maybe fade out
    // and then fade to the currentHours array?
  }
}

void loop() {
  buttons.update();
  
  uint8_t nextHour = 0;
  uint8_t currentHour = 0;
  uint8_t mCurrentSeconds = 0;
  uint8_t deltaNoon = 0;
  uint8_t theOdd = 0;
  boolean beforeNoon = false;
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  
  if(buttons.onRelease(BUTTON_SELECT)) {
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
    int mPressed = 0;
    
    if(buttons.onRelease(BUTTON_DOWN)) { // down button
      mPressed = -1;
      Serial.println("down");
    }
    if(buttons.onRelease(BUTTON_UP)) { // up button
      mPressed = 1;
      Serial.println("up");
    }
    #if defined (__AVR_ATtiny85__)
    #else
      if(mPressed != 0) {
      Serial.print("##### \t");
      Serial.println(mPressed);
      }
    #endif
    if(mPressed != 0) {
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
      RTCoutput(); // korrekt hier? oder außerbalb der schleife
      Serial.print(mHour);
      Serial.print(":");
      Serial.println(mMinute);
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
  // read/write is normal!!
  #if defined (__AVR_ATtiny85__)
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
// Helpers
byte decToBcd(byte val) {
  return ((val/10*16) + (val%10));
}
byte bcdToDec(byte val) {
  return ((val/16*10) + (val%16));
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
