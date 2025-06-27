#include <TimeLib.h>
#include <EEPROM.h>

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 
const int lightPin = 6;

// EEPROM address where we'll store the time
#define TIME_EEPROM_ADDRESS 0
// How often to save time to EEPROM (in seconds)
#define SAVE_INTERVAL 60  // 1 minute

// Track the last known good time
static time_t lastKnownTime = 0;
static time_t lastSaveTime = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2000); // Wait up to 2 seconds for serial connection
  pinMode(13, OUTPUT);
  pinMode(lightPin, OUTPUT);
  
  // First try to read the time from EEPROM
  lastKnownTime = readTimeFromEEPROM();
  
  if (lastKnownTime != 0) {
    setTime(lastKnownTime);
    Serial.println("Time restored from EEPROM");
    // Estimate current time by adding the time since last save
    // This accounts for time passing while powered off
    time_t estimatedTime = lastKnownTime;
    setTime(estimatedTime);
  } else {
    Serial.println("No valid time in EEPROM");
  }
  
  // Set sync provider but don't rely solely on it
  setSyncProvider(requestSync);
  setSyncInterval(300);  // Try to sync every 5 minutes if possible
}

void loop() {
  if (Serial.available()) {
    processSyncMessage();
  }
  
  // Maintain time even without serial connection
  maintainTime();
  
  if (timeStatus() != timeNotSet) {
    digitalClockDisplay();
    controlLight();
  }
  
  // Visual sync status indicator
  digitalWrite(13, timeStatus() == timeSet ? HIGH : LOW);
  delay(1000);
}

void maintainTime() {
  // If we have no time source, use our last known time plus elapsed milliseconds
  if (timeStatus() == timeNotSet && lastKnownTime != 0) {
    time_t estimatedTime = lastKnownTime;
    setTime(estimatedTime);
  }
  
  // Periodically save time to EEPROM
  if (now() - lastSaveTime >= SAVE_INTERVAL) {
    saveTimeToEEPROM();
    lastSaveTime = now();
    lastKnownTime = now();
  }
}

void controlLight() {
  time_t currentTime = now();
  int currentHour = hour(currentTime);
  // int currentHour = second(currentTime);

  if (currentHour >= 22 || currentHour < 8) {
    digitalWrite(lightPin, LOW);
    Serial.println("LED OFF: Night time (10:00 PM - 8:00 AM)");
  } else {
    digitalWrite(lightPin, HIGH);
    Serial.println("LED ON: Day time");
  }
}

void saveTimeToEEPROM() {
  time_t t = now();
  EEPROM.put(TIME_EEPROM_ADDRESS, t);
  Serial.print("Time saved to EEPROM: ");
  Serial.println(t);
}

time_t readTimeFromEEPROM() {
  time_t t;
  EEPROM.get(TIME_EEPROM_ADDRESS, t);
  
  // Validate the time (should be >= Jan 1 2013)
  const unsigned long DEFAULT_TIME = 1357041600;
  if (t < DEFAULT_TIME) {
    return 0;  // Invalid time
  }
  return t;
}

void digitalClockDisplay() {
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits) {
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if(pctime >= DEFAULT_TIME) {
       setTime(pctime);
       saveTimeToEEPROM();
       lastKnownTime = pctime;
       Serial.println("Time synchronized and saved");
     }
  }
}

time_t requestSync() {
  if (Serial) {  // Only send request if serial is connected
    Serial.write(TIME_REQUEST);
  }
  return 0; // Will be set later by serial message
}