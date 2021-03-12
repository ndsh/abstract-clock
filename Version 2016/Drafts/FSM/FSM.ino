#define INTRO 0
#define RUN 1

int state = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("→ setup");
}

void loop() {
  stateMachine();
}

void stateMachine() {
  switch(state) {
    case INTRO:
      Serial.println("→ intro");
      state = RUN;
    break;

    case RUN:
      Serial.println("→ run");
    break;
  }
}
