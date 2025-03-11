const int pumpPin1 = 12;
const int ledPin = 13;

void setup() {
  // put your setup code here, to run once:
  pinMode(pumpPin1, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pumpPin1, LOW); // Output PWM signal
  digitalWrite(ledPin, LOW); // Output PWM signal
  delay(5000);
  digitalWrite(pumpPin1, LOW); // Output PWM signal
  digitalWrite(ledPin, LOW); // Output PWM signal
  delay(5000);

}
