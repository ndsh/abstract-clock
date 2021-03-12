/*int getMinute() {
  return (hour()*60+minute() + offsetFromMidnight) % 1440;
}*/

int getMinute(int h, int m) {
  return (h*60+m + offsetFromMidnight) % 1440;
}

int getIndex(int i) {
  return i/4;
}
