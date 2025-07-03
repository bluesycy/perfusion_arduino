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
  // Variables for averaging
  long sumSensorValue1 = 0;
  const int numReads = 10000;

  // Read analog inputs multiple times and sum them
  for (int i = 0; i < numReads; i++) {                                                                                                                                                                                                                                  
    sumSensorValue1 += analogRead(inputPin1);
  }

  // Calculate the average sensor value
  float sensorValue1 = (float)sumSensorValue1 / numReads;

  // Convert analog readings to voltage
  float v1 = sensorValue1 * (3.3 / 65535); // Assuming 3.3V reference
  float c1 = sensorValue1 / (1 * (65535 - sensorValue1));

  Serial.print("V_loop:");
  Serial.print(v1, 4);
  Serial.print("\tC_loop:");
  Serial.print(c1, 4);
  Serial.print("\tS_loop:");
  Serial.print(sensorValue1, 0);
  Serial.println(); // Newline for next reading

  if (c1 < 1) {
    digitalWrite(pumpPin1, HIGH); // Output PWM signal
    while (c1 < 1.15) {
      // Re-average inside the while loop as well
      sumSensorValue1 = 0;
      for (int i = 0; i < numReads; i++) {
        sumSensorValue1 += analogRead(inputPin1);
      }
      sensorValue1 = (float)sumSensorValue1 / numReads;

      v1 = sensorValue1 * (3.3 / 65535); // Assuming 3.3V reference
      c1 = sensorValue1 / (1 * (65535 - sensorValue1));

      Serial.print("Voltage 1: ");
      Serial.println(v1);

      Serial.print("Conductance 1: ");
      Serial.println(c1);

      Serial.print("sensorValue1 (averaged): ");
      Serial.println(sensorValue1);

      // delay(1000);
    }
  } else {
    digitalWrite(pumpPin1, LOW); // Output PWM signal
    // delay(1000);
  }

  // delay(1000);
}