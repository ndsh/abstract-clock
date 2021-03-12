void keyPressed() {
  if (key == CODED) {
    if (keyCode == DOWN) {
    } else if (keyCode == UP) {
    }
  }
}

int getMinute() {
  return (hour()*60+minute() + offsetFromMidnight) % 1440;
}

int getMinute(int h, int m) {
  return (h*60+m + offsetFromMidnight) % 1440;
}

int getIndex(int i) {
  return i/4;
}

void displayColors() {
  int x = 0;
  float y = 1.65;
  String e[] = null;
  color c;
  for(int i = 0; i<colors.length; i++) {
    e = split(colors[i], ",");
    c = color(int(e[0]), int(e[1]), int(e[2]));
    fill(c);
    rect(x, i*y, 10, y);
    if(i == index) rect(x+10, i*y, 10, y);
  }
}
