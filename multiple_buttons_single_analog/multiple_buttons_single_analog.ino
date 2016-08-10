int a=0;
 
void setup()
{
  Serial.begin(9600);
  pinMode(A3, INPUT);
}
 
void loop()
{
  a = analogRead(3);
  Serial.print("  analogRead() ");
  Serial.print("  value is :");
  Serial.println(a);
  delay(250);
}
