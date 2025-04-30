#include "FlexSensor.h"

FlexSensor::FlexSensor() {
  // Initialize all values to zero
  indexFinger = 0;
  middleFinger = 0;
  ringFinger = 0;
  pinkyFinger = 0;
  average = 0;
  
  maxIndex = 0;
  maxMiddle = 0;
  maxRing = 0;
  maxPinky = 0;
  maxAverage = 0;
}

void FlexSensor::begin() {
  // Set analog pins as inputs
  pinMode(FLEX1A, INPUT);
  pinMode(FLEX1B, INPUT);
  pinMode(FLEX2A, INPUT);
  pinMode(FLEX2B, INPUT);
  pinMode(FLEX3A, INPUT);
  pinMode(FLEX3B, INPUT);
  pinMode(FLEX4A, INPUT);
  pinMode(FLEX4B, INPUT);
}

int FlexSensor::readSensor(int pinA, int pinB) {
  int readA = analogRead(pinA);
  int readB = analogRead(pinB);
  int reading = readB - readA;
  return reading;
}

int FlexSensor::getIndex() {
  // Read sensor and update value
  indexFinger = readSensor(FLEX1A, FLEX1B);
  return indexFinger;
}

int FlexSensor::getMiddle() {
  // Read sensor and update value
  middleFinger = readSensor(FLEX2A, FLEX2B);
  return middleFinger;
}

int FlexSensor::getRing() {
  // Read sensor and update value
  ringFinger = readSensor(FLEX3A, FLEX3B);
  return ringFinger;
}

int FlexSensor::getPinky() {
  // Read sensor and update value
  pinkyFinger = readSensor(FLEX4A, FLEX4B);
  return pinkyFinger;
}

int FlexSensor::getAverage() {
  // Get all finger readings and calculate average
  getIndex();
  getMiddle();
  getRing();
  getPinky();
  
  average = (indexFinger + middleFinger + ringFinger + pinkyFinger) / 4;
  return average;
}

int FlexSensor::toAngle(int reading) {
  // Convert raw sensor reading to degrees
  // Calibrated using tracker/matlab method
  int deg = (-0.0013 * reading * reading) + (1.0877 * reading) - 59.9341;
  return deg;
}

void FlexSensor::updateMaxValues() {
  // Get current readings
  getIndex();
  getMiddle();
  getRing();
  getPinky();
  getAverage();
  
  // Update max values if current values are higher
  int indexAngle = toAngle(indexFinger);
  int middleAngle = toAngle(middleFinger);
  int ringAngle = toAngle(ringFinger);
  int pinkyAngle = toAngle(pinkyFinger);
  int avgAngle = toAngle(average);
  
  if (indexAngle > maxIndex) {
    maxIndex = indexAngle;
  }
  
  if (middleAngle > maxMiddle) {
    maxMiddle = middleAngle;
  }
  
  if (ringAngle > maxRing) {
    maxRing = ringAngle;
  }
  
  if (pinkyAngle > maxPinky) {
    maxPinky = pinkyAngle;
  }
  
  if (avgAngle > maxAverage) {
    maxAverage = avgAngle;
  }
}

int FlexSensor::getMaxIndex() {
  return maxIndex;
}

int FlexSensor::getMaxMiddle() {
  return maxMiddle;
}

int FlexSensor::getMaxRing() {
  return maxRing;
}

int FlexSensor::getMaxPinky() {
  return maxPinky;
}

int FlexSensor::getMaxAverage() {
  return maxAverage;
}

void FlexSensor::resetMaxValues() {
  maxIndex = 0;
  maxMiddle = 0;
  maxRing = 0;
  maxPinky = 0;
  maxAverage = 0;
}