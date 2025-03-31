// Pin Definitions
const int outputPin1 = 9;
const int outputPin2 = A8;
const int inputPin1 = A3;
const int pumpPin1 = 12;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Set pin modes
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin2, OUTPUT);
  pinMode(pumpPin1, OUTPUT);
  
  pinMode(inputPin1, INPUT);
  

  analogWriteResolution(16); // Set PWM resolution to 16 bits (0-65535)
  analogReadResolution(16); // Set PWM resolution to 16 bits (0-65535)
  digitalWrite(outputPin1, HIGH); // Output PWM signal
  analogWrite(outputPin2, 0);
  digitalWrite(pumpPin1, LOW);

}

void loop() {
  // Read analog inputs
  float sensorValue1 = analogRead(inputPin1); 

  // Convert analog readings to voltage
  float v1 = sensorValue1 * (3.3 / 65535); // Assuming 3.3V reference
  float c1 = sensorValue1/(1*(65535 - sensorValue1));

  Serial.print("Voltage 1: ");
  Serial.println(v1);

  Serial.print("Conductance 1: ");
  Serial.println(c1);

  Serial.print("sensorValue1 ");
  Serial.println(sensorValue1);

  if (c1 < 1) {
    digitalWrite(pumpPin1, HIGH); // Output PWM signal
    while (c1 < 1.3) {
      sensorValue1 = analogRead(inputPin1); 
      v1 = sensorValue1 * (3.3 / 65535); // Assuming 3.3V reference
      c1 = sensorValue1/(1*(65535 - sensorValue1));

      Serial.print("Voltage 1: ");
      Serial.println(v1);

      Serial.print("Conductance 1: ");
      Serial.println(c1);

      Serial.print("sensorValue1 ");
      Serial.println(sensorValue1);

      delay(1000);
    }
  } else {
    digitalWrite(pumpPin1, LOW); // Output PWM signal
  }


  delay(1000);
}