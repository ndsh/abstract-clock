#include <Button.h>

Button button1(2); // Connect your button between pin 2 and GND
Button button2(3); // Connect your button between pin 3 and GND
Button button3(4); // Connect your button between pin 4 and GND
bool setupMode;
int setupOption = 0;

void setup() {
  button1.begin();
  button2.begin();
  button3.begin();

  setupMode = false;
  
  while (!Serial) { }; // for Leos
  Serial.begin(9600);
}

void loop() {
  if (button1.pressed()) {
    Serial.println("Button 1 pressed");
    if(setupMode == false) {
      Serial.println("Entering Setup Mode");
      setupMode = true;
    } else if(setupMode == true) {
      Serial.print("Current setupOption is: ");
      Serial.println(setupOption);
      if(setupOption == 0) {
        Serial.println("- Setting hours");
      } else if(setupOption == 1) {
        Serial.println("- Setting minutes");
      }
      setupOption++;
      if(setupOption == 2) {
        Serial.println("Leaving Setup Mode");
        setupMode = false;
        setupOption = 0;
      }
    }
  }
/*
  Serial.println(setupMode);
  Serial.println(setupOption);
  */
  
  if (button2.released())
    Serial.println("Button 2 released");
  
  if (button3.toggled()) {
    if (button3.read() == Button::PRESSED)
      Serial.println("Button 3 has been pressed");
    else
      Serial.println("Button 3 has been released");
  }
}