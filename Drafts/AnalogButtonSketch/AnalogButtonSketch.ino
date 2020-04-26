// analog button test

#define ANALOG_BTN A0

byte btnVal = 0;
byte lastButton = 0;
unsigned long lastBtnPress;
int out = 0;

// changes the sensorValue into the number of the button pressed
byte getButton(int sensorValue) {
  // change these sensorValue numbers to suit your own button readings
  if (sensorValue > 300) {
    return 1;
  }
  if (sensorValue > 100) {
    return 3;
  }
  if (sensorValue > 80) {
    return 2;
  }
  
  return 0;
}

// which analog select button has been pressed?
//   byte imp   analog input 
byte whichSelectBtn(byte imp) {

  const int sensorMax = 1023;  // 671 for 3.3 volts
  // return when no buttons are being pressed
  if (analogRead(imp) >= sensorMax-1) {
    return 0;
  }

  byte buttonOut = 0;
  const int samples = 150;
  const int debounce  = 30;    // no of milisecs to wait before accepting a button change  
  
  // take an average reading
  unsigned int sensorValue = 0;
  for (int i = 0; i < samples; i++) { 
     sensorValue = sensorValue + analogRead(imp); 
  } 
  sensorValue = sensorValue / samples; 
  buttonOut = getButton(sensorValue); 

  // if buttonOut changes too rapidly, then ignore the change - debounce 
  if (buttonOut != lastButton && lastBtnPress > millis()) {
    out = 0;
  }  
  lastBtnPress = millis() + debounce; 
  lastButton = buttonOut; 
  return buttonOut;
}
  
void setup() {  
  Serial.begin(9600);  
  delay(1000);
  Serial.println("ready");
}

void loop() {

  btnVal = whichSelectBtn(ANALOG_BTN);
  if (btnVal > 0) {
    Serial.print("button ");
    Serial.println(btnVal);

    if (btnVal == 3) {
      Serial.println("select button");
    }
    // etc...   or use a switch statement    
  }
  
}
