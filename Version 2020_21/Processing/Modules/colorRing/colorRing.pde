/*
todo:
- 1 tag = 1440 minuten / 360° = 4minuten
*/

LED[] leds = new LED[8];
String[] colors;
PFont font;

int index = 0;
int offsetFromMidnight = 0*60;

int h;
int m;
int t = 0;

void setup() {
  size(600, 600);
  frameRate(30);
  surface.setLocation(0, 0);
  //colorMode(HSB, 360, 100, 100);
  colors = loadStrings("export.txt");
  for(int i = 0; i<8; i++) {
    leds[i] = new LED(width/2, (height/2+200) - i*60);
  }
  h = 0;
  m = 0;
  font = loadFont("InputMono-Medium-11.vlw");
  textFont(font);
  textAlign(CENTER);
}

void draw() {
  //println(getMinute());
  background(160);
  m++;
  if(m >= 60) {
    m = 0;
    h++;
  }
  if(h >= 24) h = 0;
  
  displayColors();
  //index = getIndex(getMinute(0,0));
  index = getIndex(getMinute(h, m));
  //println(index);
  for(int i = 0; i<leds.length; i++) {
    t = index-i;
    if(t < 0) t = colors.length-i;
    leds[i].setColor(colors[(t)]);
    leds[i].display();
  }
  fill(0);
  text(nf(h, 2)+":"+nf(m,2), width/2-100, height/2);
}
