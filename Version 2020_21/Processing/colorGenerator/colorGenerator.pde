/*
1 tag = 1440 minuten / 360° = 4minuten

exportiert ein array mit 1440 eintraegen?
exportiert ein array mit 360 eintraegen?
*/
PImage p;
float r;
int theta;
int stepSize = 1;
int maxSteps = 360;
String[] output = new String[360];
int index = 0;

void setup() {
  size(800, 800);
  surface.setLocation(0, 0);
  //colorMode(HSB, 360, 100, 100);
  p = loadImage("ring4.jpg");
  r = height * 0.45;
  theta = 0;
  //imageMode(CENTER);
}

void draw() {
  background(0);
  
  //theta %= 360;
  int m = (int)map(theta, 0, 360, 360, 0);
  int x = int(r * cos(radians(m)))+400;
  int y = int(r * sin(radians(m)))+400;
  
  image(p, 0, 0, width, height);
  loadPixels();
  //print(y*width+x + " = " + theta + " ");
  color c = pixels[y*width+x];
  //println("x=" + x + " | y=" + y);
  ellipseMode(CENTER);
  noStroke();
  fill(200);
  ellipse(x,y,32,32);
  
  fill(c);
  rectMode(CENTER);
  ellipse(width/2,height/2,400,400);
  
  if(index < 360) output[index] = "" + int(red(c)) +","+ int(green(c)) +","+ int(blue(c));
  index++;
  
  theta += stepSize;
  if(theta > 360) {
    export();
    exit();
  } 
}

void keyPressed() {
  if (key == CODED) {
    if (keyCode == DOWN) {
    } else if (keyCode == UP) {
    }
  } else if (key == 'e') { // Export
  }
}

void export() {
  saveStrings("export.txt", output);
}
