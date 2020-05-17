#define SETUP 0
#define INTRO 1
#define RUN 2
#define CHECKINPUT 3
#define OPTIONS 4 // ehemals SETUPMODE

int state = 0;

void stateMachine() {
  switch (state) {

    case SETUP:
      Serial.begin(9600);
      Wire.begin();

      Serial.println("");
      Serial.println("→ Setup");
      Serial.println("Ocolorum is starting");
      RTCoutput();
      Serial.print("The time is ");
      Serial.print(mHour);
      Serial.print(":");
      Serial.println(mMinute);


      // initialize sub-routines
      pixels.begin();
      setupMode = false;
      Serial.println();
      state = INTRO;
    break;

    case INTRO:
      Serial.println("→ Intro");
      for (int j = 0; j < 125; j++) {
        for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, pixels.Color(j, j, j));
        pixels.show();
        delay(20);
      }
      pixels.clear();
      pixels.show();
      state = RUN;
    break;

    case RUN:
      //Serial.println("→ Run");
      deltaNoon = 0;
      theOdd = 0;
      beforeNoon = false;
      red = 0;
      green = 0;
      blue = 0;
      
      if(mPreviousSecond != mSecond) {
        calculateNewTime();
        red = getNewValue(mCurrentSeconds, (int)(pgm_read_byte(&hours2color[currentHour][0])), (int)(pgm_read_byte(&hours2color[nextHour][0])));
        green = getNewValue(mCurrentSeconds, (int)(pgm_read_byte(&hours2color[currentHour][1])), (int)(pgm_read_byte(&hours2color[nextHour][1])));
        blue = getNewValue(mCurrentSeconds, (int)(pgm_read_byte(&hours2color[currentHour][2])), (int)(pgm_read_byte(&hours2color[nextHour][2])));
        
        for(int i = 0; i<NUMPIXELS; i++) setPixelColorWrapper(i, red, green, blue, mHour);
        pixels.show();
        mPreviousSecond = mSecond;
      }
    break;

    case CHECKINPUT:
      Serial.println("→ Check for Input");
      buttons.update();
      if (buttons.onRelease(BUTTON_SELECT)) {
        setupMode = !setupMode;
        if (setupMode == true) {
          for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, 0, 0, 0);
          pixels.show();
          setupDir = true;
          setupCorrection = 0.0;
        }
      }
    break;

    case OPTIONS:
      red = 0;
      green = 0;
      blue = 0;
      int mPressed = 0;

      if (buttons.onRelease(BUTTON_DOWN)) { // down button
        mPressed = -1;
        //SerialPrint("down: ");
      }
      if (buttons.onRelease(BUTTON_UP)) { // up button
        mPressed = 1;
        //SerialPrint("up: ");
      }
      if (mPressed != 0) {
        RTCinput(mPressed);
        RTCoutput(); // korrekt hier? oder außerbalb der schleife
        /*
          SerialPrint("new time: ");
          SerialPrint(mHour);
          SerialPrint(":");
          SerialPrintln(mMinute);
        */
      }
      calculateNewTime();

      red =   (int)( pgm_read_byte(&hours2color[currentHour][0] ) * setupCorrection);
      green = (int)( pgm_read_byte(&hours2color[currentHour][1] ) * setupCorrection);
      blue =  (int)( pgm_read_byte(&hours2color[currentHour][2] ) * setupCorrection);
      for (int i = 0; i < NUMPIXELS; i++) setPixelColorWrapper(i, red, green, blue, mHour);
      pixels.show();
      if (setupDir) {
        setupCorrection += setupStepLength;
        if (setupCorrection >= setupCorrectionMax) {
          setupDir = false;
          setupCorrection = setupCorrectionMax;
        }
      }
      else {
        setupCorrection -= setupStepLength;
        if (setupCorrection < setupCorrectionMin) {
          setupDir = true;
          setupCorrection = setupCorrectionMin;
        }
      }
      break;

  }
}
