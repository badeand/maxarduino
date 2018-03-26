void transmit(int prefix, int value)
{
  // Serial.write(00);
  Serial.write(prefix);
  Serial.write(value);
  Serial.write(99);
}

class Potentiometer
{
  int min1;
  int min2;
  int min3;
  int min4;
  int min5;
  int min6;
  int prefix;
  int pin;
  int value = 0;
  int lastTransmittedValue = 0;
  int sensorValue = 0;
  public:
    Potentiometer(int _pin, int _prefix, int _min1, int _min2,int _min3, int _min4,int _min5, int _min6)
    {
      prefix = _prefix;
      pin = _pin;
      pinMode(pin, INPUT);
      min1 = _min1;
      min2 = _min2;
      min3 = _min3;
      min4 = _min4;
      min5 = _min5;
      min6 = _min6;
    }
    void Update()
    {
      sensorValue = analogRead(pin);
      // Serial.println(value);
    }
    void Transmit()
    {
      Update();
      if (lastTransmittedValue > sensorValue+10 || lastTransmittedValue < sensorValue-10)
      {
      if(sensorValue > min1 && sensorValue < min2) value = 2;
      else if(sensorValue > min2 && sensorValue < min3) value = 1;
      else if(sensorValue > min3 && sensorValue < min4) value = 4;
      else if(sensorValue > min4 && sensorValue < min5) value = 3;
      else if(sensorValue > min5 && sensorValue < min6) value = 5;
      else value = 0;
        transmit(prefix, value);
        lastTransmittedValue = sensorValue;
      }
    }
    
};

class GeneralEncoder
{
  int pinA = 0;
  int pinB = 0;
  int pos = 0;
  unsigned char encoder_A;
  unsigned char encoder_B;
  unsigned char encoder_A_prev = 0;
  int prefix;
  public:
    GeneralEncoder(int pA,
    int pB, int pf)
    {
      pinA = pA;
      pinB = pB;
      pinMode(pinA, INPUT);
      pinMode(pinB, INPUT);
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      prefix = pf;
    }
    void Update()
    {
      encoder_A = digitalRead(pinA);
      encoder_B = digitalRead(pinB);
      if ((!encoder_A) && (encoder_A_prev))
      {
        // A has gone from high to low
        if (encoder_B)
        {
          pos++;
        }
        else
        {
          pos--;
        }
      }
      encoder_A_prev = encoder_A; // Store value of A for next time
    }
    void Transmit()
    {
      if (pos != 0)
      {
        transmit(prefix, 127 + GetChangeAndReset());
      }
    }
  public:
    int
    GetChangeAndReset()
    {
      int r = pos;
      pos = 0;
      return r;
    }
  public:
    int
    GetChange()
    {
      return pos;
    }
};

class DimmableLED
{
  int brightness;
  int pin;
  int prefix;
  public:
    DimmableLED(int _pin, int _prefix, int _brightness)
    {
      pin = _pin;
      prefix = _prefix;
      brightness = _brightness;
      pinMode(pin, OUTPUT);
    }
    void SetBrightness(int _brightness)
    {
      if (_brightness < 0)
      {
        _brightness = 0;
      }
      if (_brightness > 1024)
      {
        _brightness = 1024;
      }
      brightness = _brightness;
      analogWrite(pin, brightness);
    }
    int GetBrightness()
    {
      return brightness;
    }
    int GetPrefix()
    {
      return prefix;
    }
};

class TouchButton
{
  int prefix;
  int state;
  int pin;
  public:
    TouchButton(int _pin, int _prefix)
    {
      pin = _pin;
      prefix = _prefix;
      state = 999;
      pinMode(pin, INPUT);
      digitalWrite(pin, HIGH);
    }
    void Update()
    {
      int newState = !digitalRead(pin);
      if (newState != state)
      {
        state = newState;
        Transmit();
      }
    }
    void Transmit()
    {
      transmit(prefix, 127 + state);
    }
};

DimmableLED leds[] =
{
  // Effect1
  DimmableLED(2, 2, 512),
  // Effect2
  DimmableLED(3, 3, 512),
  // Effect3
  DimmableLED(4, 4, 512),
  // Effect4
  DimmableLED(5, 5, 512)
};

GeneralEncoder encoders[] =
{
  // Effect1a
  GeneralEncoder(22, 23, 22),
  // Effect1b
  GeneralEncoder(24, 25, 24),
  // Effect2a
  GeneralEncoder(28, 29, 26),
  // Effect2b
  GeneralEncoder(30, 31, 28),
  // Effect3a
  GeneralEncoder(34, 35, 30),
  // Effect3b
  GeneralEncoder(36, 37, 32),
  // Effect4a
  GeneralEncoder(40, 41, 34),
  // Effect4b
  GeneralEncoder(42, 43, 36),
  // Effect4c
  GeneralEncoder(44, 45, 44)
};

TouchButton touchbuttons[] =
{
  // Effect1
  TouchButton(54, 57),
  // Effect2
  TouchButton(55, 56),
  // Effect3
  TouchButton(56, 55),
  // Effect4
  TouchButton(57, 54)
};

Potentiometer potentiometers[] =
{
  // Guitar Toggle switch pin,prefix,min1,min2,min3,min4,min5,min6
  Potentiometer(A5, 59, 141,199,260,350,460,563),

};



int numEncoders = 0;
int numTouchButtons = 0;
int numLeds = 0;
int numPotentiometers = 0;
void setup()
{
  Serial.begin(9600);
  // numEncoders =sizeof(encoders);
  numEncoders = sizeof(encoders) /sizeof(encoders[0]) + 1;
  numLeds = sizeof(leds) /sizeof(leds[0]) + 1;
  numTouchButtons = sizeof(touchbuttons) /sizeof(touchbuttons[0]) + 1;
  numPotentiometers = sizeof(potentiometers) / sizeof(potentiometers[0]) + 1;
}

unsigned long millisSinceTransmit;
unsigned long midMillisSinceAction;
unsigned long lowMillisSinceAction;
void loop()
{
  // high update frequency
  // update all encoders
  for (int i = 0; i < numEncoders - 1; i++)
  {
    encoders[i].Update();
  }
  // mid update frequency
  unsigned long midMillis = millis();
  if (midMillis - midMillisSinceAction >= 20)
  {
    // transmit all encoders
    for (int i = 0; i < numEncoders - 1; i++)
    {
      encoders[i].Transmit();
    }
    // update and transmit all touchbuttons
    for (int i = 0; i < numTouchButtons - 1; i++)
    {
      touchbuttons[i].Update();
      touchbuttons[i].Transmit();
    }
    for (int i = 0; i < numPotentiometers - 1; i++)
    {
      potentiometers[i].Transmit();
    }
    midMillisSinceAction = midMillis;
  }
  // low update frequency
  unsigned long lowMillis = millis();
  if (lowMillis - lowMillisSinceAction >= 1000)
  {
    transmit(1, 10);
    lowMillisSinceAction = lowMillis;
  }
  if (Serial.available() >= 3)
  {
    if (Serial.read() == 255)
    // Oppdater LED- lamper (DimmableLEDs)
    {
      int b = Serial.read();
      int v = Serial.read();
      for (int i = 0; i < numLeds - 1; i++)
      {
        if (leds[i].GetPrefix() == b)
        {
          leds[i].SetBrightness(v * 2);
        }
      }
    }
  }
  delay(2);
}
