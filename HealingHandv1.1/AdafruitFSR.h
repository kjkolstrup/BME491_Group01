#ifndef ADAFRUIT_FSR_H
#define ADAFRUIT_FSR_H

#include <Arduino.h>

// Pin definitions for FSR sensors
#define FORCE1 A8
#define FORCE2 A11
#define FORCE3 A9
#define FORCE4 A10

class AdafruitFSR {
  public:
    // Constructor
    AdafruitFSR();
    
    // Initialize the FSR sensors
    void begin();
    
    // Reading functions
    int readRaw(int pin);
    int getThumb();
    int getIndex();
    int getMiddle();
    int getRing();
    int getAverage();
    
    // Conversion functions
    float toNewtons(int reading);
    
    // Track maximum values
    void updateMaxValues();
    int getMaxValue();
    
    // Reset maximum values
    void resetMaxValues();
    
    // Utility function - detect thumb press
    bool isThumbPressed(int threshold = 500);
    
  private:
    // Current sensor readings
    int thumbForce;
    int indexForce;
    int middleForce;
    int ringForce;
    int average;
    
    // Maximum value recorded
    int maxForce;
};

#endif
