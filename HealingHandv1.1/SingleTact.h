#ifndef SINGLE_TACT_H
#define SINGLE_TACT_H

#include <Arduino.h>
#include <Wire.h>

// Constants for pressure sensors
#define SENSOR1_ADDRESS 0x06
#define SENSOR2_ADDRESS 0x08
#define FORCE_SCALE 0.01

class SingleTact {
  public:
    // Constructor
    SingleTact();
    
    // Initialize and scan for connected devices
    void begin();
    
    // Calibration functions
    void calibrateAll();
    void calibrateSensor(byte sensorAddress, int sensorNumber);
    
    // Reading functions
    int readSensor(const byte sensorAddress);
    float readSensorNewtons(const byte sensorAddress);
    float readSensorPounds(const byte sensorAddress);
    
    // Utility functions. Commented in the .cpp as well we can adjst the functionality scanner can be deleted
    // display can be used to just gather the data and just the delete the serial print or leave as is 
    void scanI2C(); 
    void displayData();
    
  private:
    int16_t baseline1; // Store baseline value for taring sensor 1
    int16_t baseline2; // Store baseline value for taring sensor 2
    
    // Internal function for direct sensor communication
    short readDataFromSensor(byte address);
};

#endif