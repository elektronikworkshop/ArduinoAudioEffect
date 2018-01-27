
const uint8_t AudioInPin = A0;
const int TargetValue = 1023 / 2;
int previousValue;

void setup()
{
  Serial.begin(9600);

  previousValue = analogRead(AudioInPin);
}

void loop()
{
  int v = analogRead(AudioInPin);
  int value = (v + previousValue) / 2;
  previousValue = v;
    
  Serial.print(value);
  Serial.print(" (target ");
  Serial.print(TargetValue);
  Serial.print("): ");
  unsigned int offset = abs(TargetValue - value);
  if (offset <= 2) {
    Serial.println("good!");
  } else if (value < TargetValue) {
    Serial.println(" turn right >>>");
  } else {
    Serial.println(" <<< turn left");
  }
  delay (100);
}
