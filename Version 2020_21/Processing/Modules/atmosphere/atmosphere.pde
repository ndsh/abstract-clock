/*
todo:
- gradient map die die dichte der atmosphäre wiederspiegelt

- sonne geht auf und hat immer 100% helligkeit
- helligkeit wird nur winkel zum horizont und dichte der atmosphäre beieinflusst
1 tag = 1440 minuten / 360° = 4minuten
*/

LED led;

int horizonAngle = 0;

void setup() {
  size(600, 600);
  surface.setLocation(0, 0);
  colorMode(HSB, 360, 100, 100);
  
  led = new LED();
}

void draw() {
  background(160);
  led.display();
}
