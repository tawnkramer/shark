

long o;

void setup()
{
  dir_init ();
  throttle_init();
  Serial.begin (9600);
  pinMode (5, INPUT);
  o = 0;
}

void loop()
{
  if ((o = getSensorCm()) > 100)
  {
    throttle_forwardPct(17);
  }
  else {
    throttle_stop();
  }
  
}

long getSensorCm()
{
  long pulse = 0;
  long cm = 0;
  pulse = pulseIn (5, HIGH);
  cm = int((double(pulse) / 147) * 2.54);
  Serial.print ("distance: ");
  Serial.print (cm);
  Serial.println (" cm");
  return cm;
}

void test_sensor ()
{
  char in;
  long pulse = 0;
  long cm = 0;
  
  if (Serial.available() > 0) {
  
    in = Serial.read();
    if (in == 32) {
      pulse = pulseIn (5, HIGH);
      cm = int((double(pulse) / 147) * 2.54);
      Serial.print ("distance: ");
      Serial.print (cm);
      Serial.println (" cm");
    }  
  
  }
}

