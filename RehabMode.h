#ifndef REHAB_MODE_H
#define REHAB_MODE_H

#include <Arduino.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"

// Constants
#define REHAB_TIMEOUT 10000  // Timeout for each exercise in milliseconds
#define COMPLETE_THRESHOLD 800  // Threshold to consider exercise complete

class RehabMode {
  public:
    // Constructor
    RehabMode();
    
    // Initialize rehab mode
    void begin();
    
    // Main rehab mode functions
    void update(int flex1, int flex2, int flex3, int flex4, int thumbForce);
    void reset();
    bool isComplete();
    
    // Exercise handling
    void startExercise(int exerciseNumber);
    void completeExercise();
    void finishMode();
    
    // Drawing functions
    void drawExercise(const char* instructionText, const char* holdText, int fingerNum);
    void drawFingerDiagram(int fingerNum);
    void displayProgress();
    void showCompletionMessage();
    
    // Getters
    int getCurrentExercise() { return currentExercise; }
    int getSuccessCount() { return successCount; }
    int getTotalExercises() { return totalExercises; }
    
  private:
    int currentExercise;            // Current exercise in rehab mode
    unsigned long exerciseStartTime; // Time when current exercise started
    bool exerciseComplete;          // Flag to track if current exercise is complete
    int totalExercises;             // Total number of exercises in the sequence
    int successCount;               // Counter for successful exercises
    
    // Helper functions for drawing UI elements
    void drawBoxedString(const uint16_t x, const uint16_t y, const char* string, 
                         const uint16_t foreColor, const uint16_t backgroundColor);
    void clearScreen();
};

#endif