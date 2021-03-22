// https://www.youtube.com/watch?v=LKnqECcg6Gw&ab_channel=minutephysics
// https://sighack.com/post/averaging-rgb-colors-the-right-way


color c1 = color(255, 0, 0);
color c2 = color(0, 255, 255);
float mix = 0.5;
float increment = 0.01;

void setup() {
  size(600, 600);
  surface.setLocation(0, 0);
  rectMode(CENTER);
}

void draw() {
  
  fill(c1);
  rect(width/2 - 40, height/2, 20, 20);
  
  fill(c2);
  rect(width/2     , height/2, 20, 20);
  
  fill(mixColors1(c1, c2, mix));
  rect(width/2 + 40, height/2, 20, 20);
  
  fill(mixColors2(c1, c2, mix));
  rect(width/2 + 40, height/2 + 40, 20, 20);
}


color mixColors1(color a, color b, float f) {
  //if(f == 1) return a;
  //else if(f == 0) return b;
  //else {
    color c = 0;
    float i = 1.0-f;
    int r1 = 0;
    int g1 = 0;
    int b1 = 0;
    
    int r2 = 0;
    int g2 = 0;
    int b2 = 0;
    
    // extract a
    r1 = a>>16&0xFF;
    g1 = a>>8&0xFF;
    b1 = a&0xFF;
    
    // extract b
    r2 = b>>16&0xFF;
    g2 = b>>8&0xFF;
    b2 = b&0xFF;
    
    r1 *= f;
    g1 *= f;
    b1 *= f;
    
    r2 *= i;
    g2 *= i;
    b2 *= i;
    
    //c = color((r1+r2)/2, (g1+g2)/2, (b1+b2)/2);
    //c = color(min(r1+r2, 255), min(g1+g2, 255), min(b1+b2, 255));
    //c = color(r1-(r1-r2)/2, g1-(g1-g2)/2, b1-(b1-b2)/2);
    c = color( sqrt( ((r1*r1) + (r2*r2))/2 ), sqrt( ((g1*g1) + (g2*g2))/2 ), sqrt( ((b1*b1) + (b2*b2))/2 ));
    
    return c;
 // }
}

color mixColors2(color a, color b, float f) {
  //if(f == 1) return a;
  //else if(f == 0) return b;
  //else {
    color c = 0;
    float i = 1.0-f;
    int r1 = 0;
    int g1 = 0;
    int b1 = 0;
    
    int r2 = 0;
    int g2 = 0;
    int b2 = 0;
    
    // extract a
    r1 = a>>16&0xFF;
    g1 = a>>8&0xFF;
    b1 = a&0xFF;
    
    // extract b
    r2 = b>>16&0xFF;
    g2 = b>>8&0xFF;
    b2 = b&0xFF;
    
    r1 *= f;
    g1 *= f;
    b1 *= f;
    
    r2 *= i;
    g2 *= i;
    b2 *= i;
    
    c = color((r1+r2)/2, (g1+g2)/2, (b1+b2)/2);
    //c = color(min(r1+r2, 255), min(g1+g2, 255), min(b1+b2, 255));
    //c = color(r1-(r1-r2)/2, g1-(g1-g2)/2, b1-(b1-b2)/2);
    //c = color( sqrt( ((r1*r1) + (r2*r2))/2 ), sqrt( ((g1*g1) + (g2*g2))/2 ), sqrt( ((b1*b1) + (b2*b2))/2 ));
    
    return c;
 // }
}



void keyPressed() {
  if (key == CODED) {
    if (keyCode == UP) {
      mix += increment;
      if(mix >= 1.0) mix = 1.0;
      println("mix up=" + mix);
    } else if (keyCode == DOWN) {
      mix -= increment;
      if(mix <= 0.0) mix = 0.0;
      println("mix down=" + mix);
    }
  }
}
