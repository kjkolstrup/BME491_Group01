#ifndef FLEX_SENSOR_H
#define FLEX_SENSOR_H

#include <Arduino.h>

// Pin definitions for flex sensors
#define FLEX1A A0
#define FLEX1B A1
#define FLEX2A A2
#define FLEX2B A3
#define FLEX3A A4
#define FLEX3B A5
#define FLEX4A A6
#define FLEX4B A7

class FlexSensor {
  public:
    // Constructor
    FlexSensor();
    
    // Initialize the flex sensors
    void begin();
    
    // Reading functions
    int readSensor(int pinA, int pinB);
    int getIndex();
    int getMiddle();
    int getRing();
    int getPinky();
    int getAverage();
    
    // Conversion functions
    int toAngle(int reading);
    
    // Track maximum angles
    void updateMaxValues();
    int getMaxIndex();
    int getMaxMiddle();
    int getMaxRing();
    int getMaxPinky();
    int getMaxAverage();
    
    // Reset maximum values
    void resetMaxValues();
    
  private:
    // Current sensor readings
    int indexFinger;
    int middleFinger;
    int ringFinger;
    int pinkyFinger;
    int average;
    
    // Maximum values recorded
    int maxIndex;
    int maxMiddle;
    int maxRing;
    int maxPinky;
    int maxAverage;
};

#endif