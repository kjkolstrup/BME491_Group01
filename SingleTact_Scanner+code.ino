#include <Wire.h> //For I2C/SMBus

// Constants for the four pressure sensors
const float FORCE_SCALE = 0.01; 
const byte SENSOR1_ADDRESS = 0x06; 
const byte SENSOR2_ADDRESS = 0x08; 
const byte SENSOR3_ADDRESS = 0x0A; 
const byte SENSOR4_ADDRESS = 0x0C; 
int16_t baseline1 = 0; // Store baseline value for taring sensors
int16_t baseline2 = 0; 
int16_t baseline3 = 0;  
int16_t baseline4 = 0; 

void setup()
{
  Wire.begin();
  TWBR=12;
  Serial.begin(57600);  // start serial for output
  Serial.flush();
  
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  // Run I2C scanner to check for connected devices
  scanI2C();
  
  Serial.println("SingleTact multiple sensor value in N and lbs.");
  Serial.println("----------------------------------------");
  
  // Allow system to stabilize before calibration
  delay(2000);
  
  // Perform initial taring of all pressure sensors
  Serial.println("Performing initial calibration, make sure no pressure is applied to sensors...");
  calibratePressureSensors();
  Serial.println("Calibration complete!");
}

void loop()
{
  // Read and display pressure sensor data for all sensors
  readAndDisplayPressureData();
  
  // Command handling
  if (Serial.available() > 0) {
    char command = Serial.read();
    handleCommand(command);
  }
  
  delay(2000);
}

void scanI2C() {
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
      } else if (address == SENSOR3_ADDRESS) {
        Serial.print(" (Sensor 3)");
      } else if (address == SENSOR4_ADDRESS) {
        Serial.print(" (Sensor 4)");
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
  
  // Specific warnings for our expected sensors
  boolean sensor1Found = false;
  boolean sensor2Found = false;
  boolean sensor3Found = false;
  boolean sensor4Found = false;
  
  Wire.beginTransmission(SENSOR1_ADDRESS);
  if (Wire.endTransmission() == 0) sensor1Found = true;
  
  Wire.beginTransmission(SENSOR2_ADDRESS);
  if (Wire.endTransmission() == 0) sensor2Found = true;
  
  Wire.beginTransmission(SENSOR3_ADDRESS);
  if (Wire.endTransmission() == 0) sensor3Found = true;
  
  Wire.beginTransmission(SENSOR4_ADDRESS);
  if (Wire.endTransmission() == 0) sensor4Found = true;
  
  if (!sensor1Found) {
    Serial.println("WARNING: Sensor 1 (0x06) not found on I2C bus!");
  }
  
  if (!sensor2Found) {
    Serial.println("WARNING: Sensor 2 (0x08) not found on I2C bus!");
  }
  
  if (!sensor3Found) {
    Serial.println("WARNING: Sensor 3 (0x0A) not found on I2C bus!");
  }
  
  if (!sensor4Found) {
    Serial.println("WARNING: Sensor 4 (0x0C) not found on I2C bus!");
  }
  
  Serial.println("----------------------------------------");
}

void calibratePressureSensors() {
  // Take multiple readings and average them for more stable baselines
  const int numSamples = 10;
  int32_t total1 = 0, total2 = 0, total3 = 0, total4 = 0;
  int validSamples1 = 0, validSamples2 = 0, validSamples3 = 0, validSamples4 = 0;
  
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
    
    // Third sensor
    short rawValue3 = readDataFromSensor(SENSOR3_ADDRESS);
    if (rawValue3 != -1) { // If reading is valid
      total3 += rawValue3;
      validSamples3++;
    }
    
    // Fourth sensor
    short rawValue4 = readDataFromSensor(SENSOR4_ADDRESS);
    if (rawValue4 != -1) { // If reading is valid
      total4 += rawValue4;
      validSamples4++;
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
  
  // Calculate average for sensor 2 if we have valid samples
  if (validSamples2 > 0) {
    baseline2 = total2 / validSamples2;
    Serial.print("Sensor 2 baseline: ");
    Serial.println(baseline2);
  } else {
    Serial.println("Warning: Could not get valid readings for sensor 2 calibration");
  }
  
  // Calculate average for sensor 3 if we have valid samples
  if (validSamples3 > 0) {
    baseline3 = total3 / validSamples3;
    Serial.print("Sensor 3 baseline: ");
    Serial.println(baseline3);
  } else {
    Serial.println("Warning: Could not get valid readings for sensor 3 calibration");
  }
  
  // Calculate average for sensor 4 if we have valid samples
  if (validSamples4 > 0) {
    baseline4 = total4 / validSamples4;
    Serial.print("Sensor 4 baseline: ");
    Serial.println(baseline4);
  } else {
    Serial.println("Warning: Could not get valid readings for sensor 4 calibration");
  }
}

void readAndDisplayPressureData() {
  // Read first sensor
  short data1 = readDataFromSensor(SENSOR1_ADDRESS);
  if (data1 != -1) { // If sensor reading was successful
    // Apply baseline correction
    short correctedData1 = data1 - baseline1;
    
    // Convert to force units
    float forceNewtons1 = max(0.0, correctedData1 * FORCE_SCALE); // Prevent negative values
    float forcePounds1 = forceNewtons1 * 0.224809; // Convert to pounds
    
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
  
  delay(100); // Brief delay between sensor reads
  
  // Read second sensor
  short data2 = readDataFromSensor(SENSOR2_ADDRESS);
  if (data2 != -1) { // If sensor reading was successful
    // Apply baseline correction
    short correctedData2 = data2 - baseline2;
    
    // Convert to force units
    float forceNewtons2 = max(0.0, correctedData2 * FORCE_SCALE); // Prevent negative values
    float forcePounds2 = forceNewtons2 * 0.224809; // Convert to pounds
    
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
  
  delay(100); // Brief delay between sensor reads
  
  // Read third sensor
  short data3 = readDataFromSensor(SENSOR3_ADDRESS);
  if (data3 != -1) { // If sensor reading was successful
    // Apply baseline correction
    short correctedData3 = data3 - baseline3;
    
    // Convert to force units
    float forceNewtons3 = max(0.0, correctedData3 * FORCE_SCALE); // Prevent negative values
    float forcePounds3 = forceNewtons3 * 0.224809; // Convert to pounds
    
    Serial.print("I2C Sensor 3 Data: ");
    Serial.print(data3);
    Serial.print(" Corrected: ");
    Serial.print(correctedData3);
    Serial.print(" Force (N): ");
    Serial.print(forceNewtons3, 2);  
    Serial.print(" Force (lbs): ");
    Serial.println(forcePounds3, 2);
  } else {
    Serial.println("Sensor 3 not detected or error in communication");
  }
  
  delay(100); // Brief delay between sensor reads
  
  // Read fourth sensor
  short data4 = readDataFromSensor(SENSOR4_ADDRESS);
  if (data4 != -1) { // If sensor reading was successful
    // Apply baseline correction
    short correctedData4 = data4 - baseline4;
    
    // Convert to force units
    float forceNewtons4 = max(0.0, correctedData4 * FORCE_SCALE); // Prevent negative values
    float forcePounds4 = forceNewtons4 * 0.224809; // Convert to pounds
    
    Serial.print("I2C Sensor 4 Data: ");
    Serial.print(data4);
    Serial.print(" Corrected: ");
    Serial.print(correctedData4);
    Serial.print(" Force (N): ");
    Serial.print(forceNewtons4, 2);  
    Serial.print(" Force (lbs): ");
    Serial.println(forcePounds4, 2);
  } else {
    Serial.println("Sensor 4 not detected or error in communication");
  }
  
  Serial.println("----------------------------------------");
}

void handleCommand(char command) {
  switch(command) {
    case 'c': // Calibrate/tare all pressure sensors
    case 'C':
      Serial.println("Calibrating all pressure sensors...");
      calibratePressureSensors();
      Serial.println("Calibration complete!");
      break;
      
    case '1': // Calibrate/tare only sensor 1
      Serial.println("Calibrating sensor 1...");
      calibrateSingleSensor(SENSOR1_ADDRESS, 1);
      Serial.println("Sensor 1 calibration complete!");
      break;
      
    case '2': // Calibrate/tare only sensor 2
      Serial.println("Calibrating sensor 2...");
      calibrateSingleSensor(SENSOR2_ADDRESS, 2);
      Serial.println("Sensor 2 calibration complete!");
      break;
      
    case '3': // Calibrate/tare only sensor 3
      Serial.println("Calibrating sensor 3...");
      calibrateSingleSensor(SENSOR3_ADDRESS, 3);
      Serial.println("Sensor 3 calibration complete!");
      break;
      
    case '4': // Calibrate/tare only sensor 4
      Serial.println("Calibrating sensor 4...");
      calibrateSingleSensor(SENSOR4_ADDRESS, 4);
      Serial.println("Sensor 4 calibration complete!");
      break;
      
    case 's': // Scan I2C bus
    case 'S':
      scanI2C();
      break;
      
    case 'h': // Display help
    case 'H': 
      displayHelp();
      break;
      
    default:
      Serial.println("Unknown command. Type 'h' for help.");
      break;
  }
}

void calibrateSingleSensor(byte sensorAddress, int sensorNumber) {
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
        Serial.print("Sensor 1 baseline: ");
        Serial.println(baseline1);
        break;
      case 2:
        baseline2 = total / validSamples;
        Serial.print("Sensor 2 baseline: ");
        Serial.println(baseline2);
        break;
      case 3:
        baseline3 = total / validSamples;
        Serial.print("Sensor 3 baseline: ");
        Serial.println(baseline3);
        break;
      case 4:
        baseline4 = total / validSamples;
        Serial.print("Sensor 4 baseline: ");
        Serial.println(baseline4);
        break;
    }
  } else {
    Serial.println("Warning: Could not get valid readings for calibration");
  }
}

void displayHelp() {
  Serial.println("--- Available Commands ---");
  Serial.println("c: Calibrate all pressure sensors (tare to zero)");
  Serial.println("1: Calibrate only sensor 1 (tare to zero)");
  Serial.println("2: Calibrate only sensor 2 (tare to zero)");
  Serial.println("3: Calibrate only sensor 3 (tare to zero)");
  Serial.println("4: Calibrate only sensor 4 (tare to zero)");
  Serial.println("s: Scan I2C bus for devices");
  Serial.println("h: Display this help message");
}

short readDataFromSensor(byte address)
{
  // Using exactly the same function as the official demo code
  byte i2cPacketLength = 6; //i2c packet length. Just need 6 bytes from each slave
  byte outgoingI2CBuffer[3]; //outgoing array buffer
  byte incomingI2CBuffer[6]; //incoming array buffer
  
  outgoingI2CBuffer[0] = 0x01; //I2c read command
  outgoingI2CBuffer[1] = 128;  //Slave data offset
  outgoingI2CBuffer[2] = i2cPacketLength; //require 6 bytes
  
  Wire.beginTransmission(address); // transmit to device 
  Wire.write(outgoingI2CBuffer, 3); // send out command
  byte error = Wire.endTransmission(); // stop transmitting and check slave status
  if (error != 0) return -1; //if slave not exists or has error, return -1
  
  Wire.requestFrom(address, i2cPacketLength); //require 6 bytes from slave
  
  byte incomeCount = 0;
  while (incomeCount < i2cPacketLength)    // slave may send less than requested
  {
    if (Wire.available())
    {
      incomingI2CBuffer[incomeCount] = Wire.read(); // receive a byte as character
      incomeCount++;
    }
    else
    {
      delayMicroseconds(10); //Wait 10us 
    }
  }
  
  short rawData = (incomingI2CBuffer[4] << 8) + incomingI2CBuffer[5]; //get the raw data
  return rawData;
}