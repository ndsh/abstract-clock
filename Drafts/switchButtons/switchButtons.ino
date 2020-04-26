const int sensorPin = A0;
int sensorValue = 0;
const int sensorMin = 600;      // sensor minimum, discovered through experiment
const int sensorMax = 1200;

long lastDebounceTime = 0;
long debounceDelay = 20;
int lastButtonState = 0;

 void setup() {
 // turn on LED to signal the start of the calibration period:
   Serial.begin(9600);
    pinMode(sensorPin, INPUT);
 }
 void loop() {
   sensorValue = analogRead(sensorPin);
   //Serial.println(sensorValue);
   int range = map(sensorValue, sensorMin, sensorMax, 0, 9);
   if (range != lastButtonState) {
    lastDebounceTime = millis();
   }
   if ((millis() - lastDebounceTime) > debounceDelay) {
    
    if(range >= 2) Serial.println(range);
   }
    lastButtonState = range;
   delay(200);
   /*
   switch (range) {
      default:
    Serial.print(sensorValue);
       Serial.print(" - ");
     case 0:    //
       //Serial.println("0"); //0-102
       break;
     case 1:    // 54k ohm - 175
       Serial.println("r");
       break;
     case 2:    // 205-306
       Serial.println("0");
       break;
     case 3:    // 307-409 18.5k ohm 378
       Serial.println("c");
       break;
     case 4:    // 410-511
       break;
     case 5:    // 512-613 8.5k ohm 579
       Serial.println("e");
       break;
     case 6:    // 614-716 5.5k ohm 682
       Serial.println("s");
       break;
     case 7:    // 717-818 3k ohm 807
       Serial.println("f");
       break;
     case 8:    // 819-920 1.2k ohm 913
       Serial.println("ph adjust");
       break;
     case 9:    // 921-1023 0 ohm
       Serial.println("auto");
       break;
     case 10:    // 921-1023 0 ohm
       Serial.println("?=??=");
       break;
       
   }
   */
   }
