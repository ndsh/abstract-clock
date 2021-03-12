class RGBLED {
  color c = 0;
  public RGBLED() {
  }
  
  void update() {
  }
  
  void display() {
    noStroke();
    fill(c);
    ellipse(width/2, height/2, 40, 40);
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
  
  
}
