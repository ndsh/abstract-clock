class LED {
  int brightness = 0;
  color c = 0;
  float density = 1.0;
  PVector pos;
  
  public LED() {
    pos = new PVector(width/2, height/2);
  }
  
  public LED(int x, int y) {
    pos = new PVector(x, y);
  }
  
  void update() {
  }
  
  void display() {
    noStroke();
    fill(c);
    ellipse(pos.x, pos.y, 40, 40);
  }
  
  void setColor(int r, int g, int b) {
    c = color(r, g, b);
  }
  
  void setColor(int r, int g, int b, int a) {
    c = color(r, g, b, a);
  }
  
  void setColor(String s) {
    String e[] = split(s, ",");
    
    c = color(int(e[0]), int(e[1]), int(e[2]));
  }
  
  void setColor(color _c) {
    c = _c;
  }
  
  void setBrightness(int i) {
    brightness = i;
    //int map = int(map(i, 0, 100, 0, 360));
    //c = color(map, 100, 100);
  }
  
  void calculate(int angle) {
  }
  
}
