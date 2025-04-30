#include "SingleTact.h"

SingleTact::SingleTact() {
  baseline1 = 0;
  baseline2 = 0;
}

void SingleTact::begin() {
  // Initialize I2C communication
  Wire.begin();
  TWBR = 12; // Set I2C clock speed
  
  // Scan for devices on the I2C bus
  scanI2C();
  
  // Allow system to stabilize
  delay(2000);
  
  // Perform initial calibration
  Serial.println("Performing initial calibration, make sure no pressure is applied to sensors..."); // CAN DELETE FOR DEBUGGING MOSTLY
  calibrateAll();
  Serial.println("Calibration complete!");// CAN DELETE FOR DEBUGGING MOSTLY
}
 //we can delete the address scanner since we know the addresses
void SingleTact::scanI2C() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("Scanning I2C bus for devices...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      
      // Note if this is one of our expected sensor addresses
      if (address == SENSOR1_ADDRESS) {
        Serial.print(" (Sensor 1)");
      } else if (address == SENSOR2_ADDRESS) {
        Serial.print(" (Sensor 2)");
      } else if (address == 0x04) {
        Serial.print(" (Default address - should not be used)");
      }
      
      Serial.println();
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found! Check connections.");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" device(s)");
  }
  
  
  boolean sensor1Found = false;
  boolean sensor2Found = false;
  // Scanner end DELETE if we want to
  Wire.beginTransmission(SENSOR1_ADDRESS);
  if (Wire.endTransmission() == 0) sensor1Found = true;
  
  Wire.beginTransmission(SENSOR2_ADDRESS);
  if (Wire.endTransmission() == 0) sensor2Found = true;
  
  if (!sensor1Found) {
    Serial.println("WARNING: Sensor 1 (0x06) not found on I2C bus!"); // CAN DELETE FOR DEBUGGING MOSTLY
  }
  
  if (!sensor2Found) {
    Serial.println("WARNING: Sensor 2 (0x08) not found on I2C bus!"); // CAN DELETE FOR DEBUGGING/TROUBLESHOOT 
  }
  
  Serial.println("----------------------------------------");
}

void SingleTact::calibrateAll() {
  // Take multiple readings and average them for more stable baselines
  const int numSamples = 10;
  int32_t total1 = 0, total2 = 0;
  int validSamples1 = 0, validSamples2 = 0;
  
  // Collect samples
  for (int sample = 0; sample < numSamples; sample++) {
    // First sensor
    short rawValue1 = readDataFromSensor(SENSOR1_ADDRESS);
    if (rawValue1 != -1) { // If reading is valid
      total1 += rawValue1;
      validSamples1++;
    }
    
    // Second sensor
    short rawValue2 = readDataFromSensor(SENSOR2_ADDRESS);
    if (rawValue2 != -1) { // If reading is valid
      total2 += rawValue2;
      validSamples2++;
    }
    
    delay(50); // Wait between samples
  }
  
  // Calculate average for sensor 1 if we have valid samples
  if (validSamples1 > 0) {
    baseline1 = total1 / validSamples1;
    Serial.print("Sensor 1 baseline: ");
    Serial.println(baseline1);
  } else {
    Serial.println("Warning: Could not get valid readings for sensor 1 calibration");
  }
  
  if (validSamples2 > 0) {
    baseline2 = total2 / validSamples2;
    Serial.print("Sensor 2 baseline: ");
    Serial.println(baseline2);
  } else {
    Serial.println("Warning: Could not get valid readings for sensor 2 calibration");
  }
}

void SingleTact::calibrateSensor(byte sensorAddress, int sensorNumber) {
  // Take multiple readings and average them for more stable baseline
  const int numSamples = 10;
  int32_t total = 0;
  int validSamples = 0;
  
  // Collect samples
  for (int sample = 0; sample < numSamples; sample++) {
    short rawValue = readDataFromSensor(sensorAddress);
    if (rawValue != -1) { // If reading is valid
      total += rawValue;
      validSamples++;
    }
    delay(50); // Wait between samples
  }
  
  // Calculate average if we have valid samples
  if (validSamples > 0) {
    switch(sensorNumber) {
      case 1:
        baseline1 = total / validSamples;
        Serial.print("Sensor 1 baseline: "); // CAN DELETE FOR DEBUGGING MOSTLY
        Serial.println(baseline1); // CAN DELETE FOR DEBUGGING MOSTLY
        break;
      case 2:
        baseline2 = total / validSamples;
        Serial.print("Sensor 2 baseline: "); // CAN DELETE FOR DEBUGGING MOSTLY
        Serial.println(baseline2); // CAN DELETE FOR DEBUGGING MOSTLY
        break;
    }
  } else {
    Serial.println("Warning: Could not get valid readings for calibration");
  }
}

int SingleTact::readSensor(const byte sensorAddress) {
  short data = readDataFromSensor(sensorAddress);
  if (data != -1) {
    // Apply baseline correction based on sensor address
    if (sensorAddress == SENSOR1_ADDRESS) {
      return data - baseline1;
    } else if (sensorAddress == SENSOR2_ADDRESS) {
      return data - baseline2;
    }
    return data; 
  }
  return 0; // Error reading sensor
}

float SingleTact::readSensorNewtons(const byte sensorAddress) {
  int correctedData = readSensor(sensorAddress);
  // Convert to force units in Newtons
  return max(0.0, correctedData * FORCE_SCALE);
}

float SingleTact::readSensorPounds(const byte sensorAddress) {
  // Convert Newtons to pounds (1 N = 0.224809 lbs)
  return readSensorNewtons(sensorAddress) * 0.224809;
}

void SingleTact::displayData() {
  // Read and display data from all sensors
  
  // First sensor
  short data1 = readDataFromSensor(SENSOR1_ADDRESS);
  if (data1 != -1) {
    // Apply baseline correction
    short correctedData1 = data1 - baseline1;
    
    // Convert to force units
    float forceNewtons1 = max(0.0, correctedData1 * FORCE_SCALE);
    float forcePounds1 = forceNewtons1 * 0.224809;
    //////////////////////// CAN DELETE USER WON'T NEED TO SEE THIS
    Serial.print("I2C Sensor 1 Data: ");
    Serial.print(data1);
    Serial.print(" Corrected: ");
    Serial.print(correctedData1);
    Serial.print(" Force (N): ");
    Serial.print(forceNewtons1, 2);  
    Serial.print(" Force (lbs): ");
    Serial.println(forcePounds1, 2);
  } else {
    Serial.println("Sensor 1 not detected or error in communication");
  }
  
  // Second sensor
  short data2 = readDataFromSensor(SENSOR2_ADDRESS);
  if (data2 != -1) {
    // Apply baseline correction
    short correctedData2 = data2 - baseline2;
    
    // Convert to force units
    float forceNewtons2 = max(0.0, correctedData2 * FORCE_SCALE);
    float forcePounds2 = forceNewtons2 * 0.224809;
    //////////////////////// CAN DELETE USER WON'T NEED TO SEE THIS
    Serial.print("I2C Sensor 2 Data: ");
    Serial.print(data2);
    Serial.print(" Corrected: ");
    Serial.print(correctedData2);
    Serial.print(" Force (N): ");
    Serial.print(forceNewtons2, 2);  
    Serial.print(" Force (lbs): ");
    Serial.println(forcePounds2, 2);
  } else {
    Serial.println("Sensor 2 not detected or error in communication");
  }
  
  Serial.println("----------------------------------------");
} // This closing brace was missing

short SingleTact::readDataFromSensor(byte address) {
  byte i2cPacketLength = 6; 
  byte outgoingI2CBuffer[3]; // outgoing array buffer
  byte incomingI2CBuffer[6]; // incoming array buffer
  
  outgoingI2CBuffer[0] = 0x01; // I2c read command
  outgoingI2CBuffer[1] = 128;  // Slave data offset
  outgoingI2CBuffer[2] = i2cPacketLength; // require 6 bytes
  
  Wire.beginTransmission(address); // transmit to device 
  Wire.write(outgoingI2CBuffer, 3); // send out command
  byte error = Wire.endTransmission(); // stop transmitting and check slave status
  if (error != 0) return -1; // if slave not exists or has error, return -1
  
  Wire.requestFrom(address, i2cPacketLength); // require 6 bytes from slave
  
  byte incomeCount = 0;
  while (incomeCount < i2cPacketLength) { // slave may send less than requested
    if (Wire.available()) {
      incomingI2CBuffer[incomeCount] = Wire.read(); // receive a byte as character
      incomeCount++;
    } else {
      delayMicroseconds(10); // Wait 10us 
    }
  }
  
  short rawData = (incomingI2CBuffer[4] << 8) + incomingI2CBuffer[5]; // get the raw data
  return rawData;
}