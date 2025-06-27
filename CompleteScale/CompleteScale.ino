/*
  Use the Qwiic Scale to read load cells and scales
  By: Nathan Seidle @ SparkFun Electronics (adapted for Teensy & calibration)
  Date: June 4, 2025
  License: Public Domain (Beerware)

  This example shows how to setup a scale complete with zero offset (tare),
  and linear calibration. It also saves these values to EEPROM for persistence.

  Hardware Connections:
  Plug a Qwiic cable into the Qwiic Scale and your Teensy.
  (Teensy pins: GND, 3.3V, SDA (pin 9), SCL (pin 8))
  Open the serial monitor at 115200 baud to see the output and interact.
*/

#include <Wire.h>
#include <EEPROM.h> // Needed to store user settings in non-volatile memory

#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"

NAU7802 myScale; // Create instance of the NAU7802 class

// EEPROM locations to store 4-byte variables (float and int32_t)
#define LOCATION_CALIBRATION_FACTOR 0 // Float, requires 4 bytes of EEPROM
#define LOCATION_ZERO_OFFSET 10       // int32_t, requires 4 bytes of EEPROM. Must be at least 4 bytes away from previous.

bool settingsDetected = false; // Used to prompt the user to calibrate their scale

// Create an array to average weights. This helps smooth out jitter.
#define AVG_SIZE 100 // Increased average size for more stable readings
float avgWeights[AVG_SIZE];
byte avgWeightSpot = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Qwiic Scale Calibration & Reading Example");
  Serial.println("----------------------------------------");

  // Wait for the serial port to connect (especially useful for Teensy)
  while (!Serial && millis() < 5000);

  Wire.begin();
  Wire.setClock(400000); // Qwiic Scale is capable of running at 400kHz I2C

  if (myScale.begin() == false) {
    Serial.println("Scale not detected. Please check wiring. Freezing...");
    while (1); // Halt if scale isn't found
  }
  Serial.println("Scale detected and initialized!");

  // Load zeroOffset and calibrationFactor from EEPROM
  // (Teensy's EEPROM library does not use EEPROM.begin() or EEPROM.commit())
  readSystemSettings();

  // Set sample rate and calibrate Analog Front End (AFE) for optimal performance
  myScale.setSampleRate(NAU7802_SPS_320); // Max sample rate for faster readings
  myScale.calibrateAFE();                 // Recalibrate AFE after changing sample rate

  Serial.print("Current Zero offset: ");
  Serial.println(myScale.getZeroOffset());
  Serial.print("Current Calibration factor: ");
  Serial.println(myScale.getCalibrationFactor(), 4); // Print with more decimal places

  Serial.println("\r\n--- Instructions ---");
  Serial.println("Press 't' to Tare (zero) the scale.");
  Serial.println("Press 'c' to Calibrate the scale with a known weight.");
  Serial.println("--------------------");

  if (settingsDetected == false) {
    Serial.println("\n*** Scale not yet calibrated! Please press 'c' to start calibration. ***");
  }
}

void loop() {
  // Read scale data only when a new reading is available
  if (myScale.available() == true) {
    int32_t currentReading = myScale.getReading(); // Raw ADC reading
    float currentWeight = myScale.getWeight();     // Calibrated weight

    // Add current weight to the averaging array
    avgWeights[avgWeightSpot++] = currentWeight;
    if (avgWeightSpot == AVG_SIZE) avgWeightSpot = 0; // Wrap around

    // Calculate the average weight
    float avgWeight = 0;
    for (int x = 0; x < AVG_SIZE; x++)
      avgWeight += avgWeights[x];
    avgWeight /= AVG_SIZE;

    // Output all relevant data to Serial Monitor
    Serial.print("Reading: ");
    Serial.print(currentReading);
    Serial.print("\tWeight: ");
    Serial.print(currentWeight, 2); // Print 2 decimal places for current weight
    Serial.print("\tAvgWeight: ");
    Serial.print(avgWeight, 2);     // Print 2 decimal places for average weight

    if (settingsDetected == false) {
      Serial.print("\t(Calibrate Me!)"); // Reminder if not calibrated
    }
    Serial.println(); // Newline for next reading
  }

  // Check for incoming serial commands for tare or calibrate
  if (Serial.available()) {
    char incoming = Serial.read(); // Read single character

    if (incoming == 't') { // Tare the scale
      Serial.println("\n--- Taring Scale ---");
      myScale.calculateZeroOffset(64); // Average over 64 readings for stability
      Serial.print("New Zero Offset: ");
      Serial.println(myScale.getZeroOffset());
      recordSystemSettings(); // Save the new zero offset
      Serial.println("Scale tared and offset saved.");
    } else if (incoming == 'c') { // Calibrate the scale
      calibrateScale();
    }
  }
}

// --- Calibration and EEPROM Functions ---

// Guides the user through setting a known weight and calculating a calibration factor
void calibrateScale(void) {
  Serial.println("\n\n--- SCALE CALIBRATION PROCESS ---");

  // Step 1: Tare with no weight
  Serial.println("1. Ensure there is NO weight on the scale.");
  Serial.println("   Press any key in the Serial Monitor when ready to tare.");
  while (Serial.available()) Serial.read(); // Clear any existing input
  while (Serial.available() == 0) delay(10); // Wait for user input
  myScale.calculateZeroOffset(64); // Tare (zero) the scale, averaging 64 readings
  Serial.print("   Zero offset calculated: ");
  Serial.println(myScale.getZeroOffset());
  Serial.println("   Scale is now tared. Remove any weight if you placed it to press key.");

  // Step 2: Place known weight
  Serial.println("\n2. Place a KNOWN weight on the scale.");
  Serial.println("   For example, if you have a 100g weight, place it now.");
  Serial.println("   Press any key in the Serial Monitor when the weight is stable.");
  while (Serial.available()) Serial.read(); // Clear any existing input
  while (Serial.available() == 0) delay(10); // Wait for user input

  // Step 3: Get known weight value from user
  Serial.print("\n3. Enter the value of the known weight you just placed (e.g., 100 for 100g, 1.5 for 1.5kg): ");
  // Wait for user to type and press Enter
  while (Serial.available()) Serial.read(); // Clear any existing input
  while (Serial.available() == 0) delay(10); // Wait for user to type
  float weightOnScale = Serial.parseFloat(); // Read the floating-point number
  Serial.println(); // Consume the newline after parseFloat

  if (weightOnScale <= 0) { // Basic validation
    Serial.println("Invalid weight entered. Calibration cancelled.");
    return;
  }

  // Step 4: Calculate calibration factor
  myScale.calculateCalibrationFactor(weightOnScale, 64); // Calculate factor, averaging 64 readings
  Serial.print("   New Calibration Factor: ");
  Serial.println(myScale.getCalibrationFactor(), 4); // Print with more decimal places

  Serial.print("   Current Reading with New Calibration: ");
  Serial.println(myScale.getWeight(), 2); // Show the current weight with new calibration

  // Step 5: Save settings
  recordSystemSettings(); // Save the new calibration and zero offset to EEPROM
  settingsDetected = true; // Mark as calibrated
  Serial.println("\n--- CALIBRATION COMPLETE! Settings saved to EEPROM. ---");
  Serial.println("You can now remove the known weight.");
}

// Record the current system settings to EEPROM
void recordSystemSettings(void) {
  // Use EEPROM.put() for float and int32_t directly.
  // Teensy's EEPROM library handles writing without a separate commit() call.
  EEPROM.put(LOCATION_CALIBRATION_FACTOR, myScale.getCalibrationFactor());
  EEPROM.put(LOCATION_ZERO_OFFSET, myScale.getZeroOffset());

  Serial.println("Settings saved to EEPROM.");
}

// Reads the current system settings from EEPROM
// If anything looks weird (uninitialized EEPROM), resets settings to default values
void readSystemSettings(void) {
  float settingCalibrationFactor; // Value used to convert the load cell reading to lbs or kg
  int32_t settingZeroOffset;      // Zero value that is found when scale is tared

  // Look up the calibration factor
  EEPROM.get(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor);
  // If the EEPROM location was never written, it might contain all 0xFF bytes.
  // For a float, 0xFFFFFFFF can be an invalid number (NaN) or a very large negative value.
  // A robust check for uninitialized EEPROM for floats is tricky.
  // Here, we check for a value of 1.0 (the default set below) or if it's very close to 0,
  // which might indicate an uninitialized state or an invalid calibration.
  if (settingCalibrationFactor == 1.0f || (settingCalibrationFactor < 0.0001f && settingCalibrationFactor > -0.0001f)) {
    Serial.println("No valid calibration factor found in EEPROM. Setting to default 1.0.");
    settingCalibrationFactor = 1.0; // Default to 1.0 (raw reading output)
    EEPROM.put(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor); // Store default
  }

  // Look up the zero tare point
  EEPROM.get(LOCATION_ZERO_OFFSET, settingZeroOffset);
  // For int32_t, 0xFFFFFFFF is -1. If EEPROM was never written, this might be its content.
  if (settingZeroOffset == -1 || settingZeroOffset == 0) { // Check for -1 or 0 (default set below)
    Serial.println("No valid zero offset found in EEPROM. Setting to default 0.");
    settingZeroOffset = 0; // Default to 0 (no offset)
    EEPROM.put(LOCATION_ZERO_OFFSET, settingZeroOffset); // Store default
  }

  // Pass these values to the library
  myScale.setCalibrationFactor(settingCalibrationFactor);
  myScale.setZeroOffset(settingZeroOffset);

  // Mark as calibrated if valid settings were retrieved
  settingsDetected = true;
  if (settingCalibrationFactor == 1.0f || settingZeroOffset == 0) {
    settingsDetected = false; // If defaults were detected, prompt user to calibrate
  }
}