int old_button = 0;

int getButton()
{
  int i, z, sum;
  int button;

  sum = 0;
  for (i=0; i < 4; i++)
  {
     sum += analogRead(0);
  }
  z = sum / 4;
  Serial.println(z);
  if (z > 1000) button = 3;                                           
  else if (z > 740 && z < 760) button = 1;                     
  else if (z > 960 && z < 980) button = 2;                

  return button;
}

void setup () {
  Serial.begin(9600);
  pinMode(A0, INPUT);
}

void loop ()
{
  int button, button2, pressed_button;  
  button = getButton();
  if (button != old_button)
  {
      delay(50);        // debounce
      button2 = getButton();

      if (button == button2)
      {
         old_button = button;
         pressed_button = button;
         Serial.println(pressed_button);
      }
   }
}
