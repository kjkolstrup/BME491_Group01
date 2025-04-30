#include "AdafruitFSR.h"

AdafruitFSR::AdafruitFSR() {
  // Initialize all values to zero
  thumbForce = 0;
  indexForce = 0;
  middleForce = 0;
  ringForce = 0;
  average = 0;
  maxForce = 0;
}

void AdafruitFSR::begin() {
  // Set analog pins as inputs
  pinMode(FORCE1, INPUT);
  pinMode(FORCE2, INPUT);
  pinMode(FORCE3, INPUT);
  pinMode(FORCE4, INPUT);
}

int AdafruitFSR::readRaw(int pin) {
  // Read raw analog value from FSR
  return analogRead(pin);
}

int AdafruitFSR::getThumb() {
  // Read sensor and update value
  thumbForce = readRaw(FORCE1);
  return thumbForce;
}

int AdafruitFSR::getIndex() {
  // Read sensor and update value
  indexForce = readRaw(FORCE2);
  return indexForce;
}

int AdafruitFSR::getMiddle() {
  // Read sensor and update value
  middleForce = readRaw(FORCE3);
  return middleForce;
}

int AdafruitFSR::getRing() {
  // Read sensor and update value
  ringForce = readRaw(FORCE4);
  return ringForce;
}

int AdafruitFSR::getAverage() {
  // Get all finger readings and calculate average
  getThumb();
  getIndex();
  getMiddle();
  getRing();
  
  average = (thumbForce + indexForce + middleForce + ringForce) / 4;
  return average;
}

float AdafruitFSR::toNewtons(int reading) {
  // Convert raw FSR reading to Newtons
  // This is an approximate conversion based on the FSR characteristics
  float fsrForce = 0;
  
  // Calculate FSR resistance
  int fsrResistance = 5000 - reading;     // fsrVoltage is in millivolts so 5V = 5000mV
  fsrResistance *= 10000;                 // 10K resistor
  fsrResistance /= reading;
  
  // Calculate FSR conductance
  int fsrConductance = 1000000;           // we measure in micromhos
  fsrConductance /= fsrResistance;
  
  // Convert to force
  if (fsrConductance <= 1000) {
    fsrForce = fsrConductance / 80;
  } else {
    fsrForce = fsrConductance - 1000;
    fsrForce /= 30;           
  }
  
  return fsrForce;
}

void AdafruitFSR::updateMaxValues() {
  // Get current readings
  getAverage();
  
  // Update max value if current value is higher
  if (average > maxForce) {
    maxForce = average;
  }
}

int AdafruitFSR::getMaxValue() {
  return maxForce;
}

void AdafruitFSR::resetMaxValues() {
  maxForce = 0;
}

bool AdafruitFSR::isThumbPressed(int threshold) {
  // Read current thumb force
  getThumb();
  
  // Return true if force is above threshold
  return (thumbForce > threshold);
}