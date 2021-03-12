void keyPressed() {
  if (key == CODED) {
    if (keyCode == DOWN) {
      horizonAngle--;
      if(horizonAngle < 0) horizonAngle = 360;
      println(horizonAngle);
      led.calculate(horizonAngle);
    } else if (keyCode == UP) {
      horizonAngle++;
      horizonAngle %= 360;
      println(horizonAngle);
      led.calculate(horizonAngle);
    }
  }
}
