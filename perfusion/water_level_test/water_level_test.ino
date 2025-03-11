// Pin Definitions
const int outputPin1 = 9;       // Digital output pin
const int outputPin2 = A8;       // Digital output pin
const int inputPin1 = A3;     // Analog input pin 2


// const int inputPin_test = A4;     // Analog input pin 2
// const int outputPin_test = 18;     // Analog input pin 2

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Set pin modes
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin2, OUTPUT);
  
  pinMode(inputPin1, INPUT);

  // pinMode(inputPin_test, INPUT);
  // pinMode(outputPin_test, OUTPUT);


  analogWriteResolution(16); // Set PWM resolution to 16 bits (0-65535)
  analogReadResolution(16); // Set PWM resolution to 16 bits (0-65535)
  digitalWrite(outputPin1, HIGH); // Output PWM signal
  analogWrite(outputPin2, 0);

  // digitalWrite(outputPin_test, HIGH); // Output PWM signal
}

void loop() {
  // Read analog inputs
  float sensorValue1 = analogRead(inputPin1); // Read from pin 22

  // Convert analog readings to voltage
  float v1 = sensorValue1 * (3.3 / 65535); // Assuming 3.3V reference
  float c1 = sensorValue1/(1*(65535 - sensorValue1));
  // Print results to serial monitor
  Serial.print("Voltage 1: ");
  Serial.println(v1);

  Serial.print("Conductance 1: ");
  Serial.println(c1);
  // // Print results to serial monitor
  Serial.print("sensorValue1 ");
  Serial.println(sensorValue1);


  // int sensorValue_test = analogRead(inputPin_test); // Read from pin 22
  // Serial.print("sensorValue_test ");
  // Serial.println(sensorValue_test);
  // Delay for stability
  delay(1000);
}