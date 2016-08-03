int mHour;
int mMinute;

void setup() {
  mHour = 0;
  mMinute = 0;
  
  while (!Serial) { }; // for Leos
  Serial.begin(9600);
}

void loop() {
  /* counting up: hours + minutes
  mHour++;
  mHour%=24;
  Serial.println(mHour);
  
  */




  
    /* up up
  mMinute++;
  if(mMinute%60 == 0 && mMinute != 0) {
    mMinute%=60;
    mHour++;
    mHour%=24;
  }
  */
/*
  mMinute--;
  if(mMinute == -1) {
    mMinute = 59;
    //mHour--;
    //if(mHour == -1) mHour = 23;
    mHour = getHour(mHour, false);
  }
  */
  mMinute = getMinute(mMinute, mHour, false);

  Serial.print(mHour);
  Serial.print(":");
  Serial.println(mMinute);

  delay(100);
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