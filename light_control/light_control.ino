const int lightPin = 6;
const int ledPin = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(lightPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(lightPin, HIGH); // Output PWM signal
  digitalWrite(ledPin, HIGH); // Output PWM signal
  delay(5000);
  digitalWrite(lightPin, LOW); // Output PWM signal
  digitalWrite(ledPin, LOW); // Output PWM signal
  delay(5000);

}
