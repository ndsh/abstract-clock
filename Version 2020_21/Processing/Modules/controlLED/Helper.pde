void keyPressed() {
  if (key == CODED) {
    if (keyCode == DOWN) {
      colorIndex--;
      if(colorIndex < 0) colorIndex = colors.length-1;
      println(colorIndex);
      led.setColor(colors[colorIndex]);
    } else if (keyCode == UP) {
      colorIndex++;
      colorIndex %= colors.length;
      println(colorIndex);
      led.setColor(colors[colorIndex]);
    }
  }
}
