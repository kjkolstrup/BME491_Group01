#include "RehabMode.h"

// Color definitions
#define BLACK   0x0000
#define BLUE    0x001F
#define LIGHTBLUE 0x039F
#define DARKBLUE  0x0013
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

RehabMode::RehabMode() {
  currentExercise = 0;
  exerciseStartTime = 0;
  exerciseComplete = false;
  totalExercises = 4;  // Total number of exercises in the sequence
  successCount = 0;
  exerciseTracker = 0;
}

void RehabMode::begin() {
  clearScreen();
  drawBoxedString(10, 10, "Mobility Rehabilitation", LIGHTBLUE, WHITE);
  drawBoxedString(10, 45, "Follow the prompts to", YELLOW, BLACK);
  drawBoxedString(10, 70, "perform finger exercises", YELLOW, BLACK);
  drawBoxedString(10, 105, "Press thumb sensor to start", GREEN, BLACK);
}

void RehabMode::update(int flex1, int flex2, int flex3, int flex4, int thumbForce) {
  // Display progress
  displayProgress();
  
  // Check for timeout if exerciseTracker is too high
  if ((exerciseTracker > 7) && !exerciseComplete) {
    // Move to next exercise if timeout reached
    currentExercise++;
    exerciseTracker = 0;
    exerciseComplete = 0;
    
    if (currentExercise > totalExercises) {
      // End of all exercises
      finishMode();
      return;
    }
    
    clearScreen();
  }
  
  // Handle different exercises
  switch (currentExercise) {
    case 1:
      // Index finger to thumb
      drawExercise("Touch INDEX finger to thumb", 
                  "Hold for 2 seconds",
                  1);
      
      // Check if exercise is completed
      if (thumbForce > COMPLETE_THRESHOLD && flex1 < 400 && !exerciseComplete && exerciseTracker > 3) {
        // Exercise completed
        exerciseComplete = true;
        successCount++;
        drawBoxedString(10, 110, "Great job!", GREEN, BLACK);
        delay(2000);
        
        // Move to next exercise
        currentExercise++;
        exerciseTracker = 0;
        exerciseComplete = false;
        clearScreen();
      }
      break;
      
    case 2:
      // Middle finger to thumb
      drawExercise("Touch MIDDLE finger to thumb", 
                  "Hold for 2 seconds",
                  2);
      
      // Check if exercise is completed
      if (thumbForce > COMPLETE_THRESHOLD && flex2 < 400 && !exerciseComplete && exerciseTracker > 3) {
        // Exercise completed
        exerciseComplete = true;
        successCount++;
        drawBoxedString(10, 110, "Excellent work!", GREEN, BLACK);
        delay(2000);
        
        // Move to next exercise
        currentExercise++;
        exerciseTracker = 0;
        exerciseComplete = false;
        clearScreen();
      }
      break;
      
    case 3:
      // Ring finger to thumb
      drawExercise("Touch RING finger to thumb", 
                  "Hold for 2 seconds",
                  3);
      
      // Check if exercise is completed
      if (thumbForce > COMPLETE_THRESHOLD && flex3 < 400 && !exerciseComplete && exerciseTracker > 3) {
        // Exercise completed
        exerciseComplete = true;
        successCount++;
        drawBoxedString(10, 110, "Well done!", GREEN, BLACK);
        delay(2000);
        
        // Move to next exercise
        currentExercise++;
        exerciseTracker = 0;
        exerciseComplete = false;
        clearScreen();
      }
      break;
      
    case 4:
      // Pinky finger to thumb
      drawExercise("Touch PINKY finger to thumb", 
                  "Hold for 2 seconds",
                  4);
      
      // Check if exercise is completed
      if (thumbForce > COMPLETE_THRESHOLD && flex4 < 400 && !exerciseComplete && exerciseTracker > 3) {
        // Exercise completed
        exerciseComplete = true;
        successCount++;
        drawBoxedString(10, 110, "Perfect!", GREEN, BLACK);
        delay(2000);
        
        // All exercises complete
        finishMode();
      }
      break;
  }
}

void RehabMode::reset() {
  currentExercise = 0;
  exerciseStartTime = 0;
  exerciseComplete = false;
  successCount = 0;
  exerciseTracker = 0;
}

bool RehabMode::isComplete() {
  return (currentExercise > totalExercises) || (successCount >= totalExercises);
}

void RehabMode::startExercise(int exerciseNumber) {
  currentExercise = exerciseNumber;
  exerciseStartTime = millis();
  exerciseComplete = false;
  exerciseTracker = 0;
  clearScreen();
}

void RehabMode::completeExercise() {
  exerciseComplete = true;
  successCount++;
}

void RehabMode::finishMode() {
  clearScreen();
  drawBoxedString(10, 20, "Rehabilitation Done!", BLUE, WHITE);
  
  char resultText[30];
  sprintf(resultText, "Exercises completed:  %d/%d", successCount, totalExercises);
  drawBoxedString(10, 45, resultText, GREEN, BLACK);
  
  // Display message based on performance
  if (successCount == totalExercises) {
    drawBoxedString(10, 85, "Perfect score! Great job!", YELLOW, BLACK);
  } else if (successCount >= totalExercises/2) {
    drawBoxedString(10, 85, "Good work! Keep practicing!", YELLOW, BLACK);
  } else {
    drawBoxedString(10, 85, "Keep trying! You'll improve!", YELLOW, BLACK);
  }
  
  drawBoxedString(10, 130, "Press thumb to continue", GREEN, BLACK);
}

void RehabMode::drawExercise(const char* instructionText, const char* holdText, int fingerNum) {
  drawBoxedString(10, 10, "Finger Mobility", BLUE, WHITE);
  drawBoxedString(10, 35, instructionText, YELLOW, BLACK);
  drawBoxedString(10, 125, holdText, YELLOW, BLACK);  
  // Draw finger diagram
  drawFingerDiagram(fingerNum);
}

void RehabMode::drawFingerDiagram(int fingerNum) {
  // Draw hand outline
  Paint_DrawRectangle(200, 60, 260, 120, WHITE, 1, 0);
  
  // Draw fingers (vertical lines)
  for (int i = 0; i < 4; i++) {
    int x = 210 + (i * 15);
    Paint_DrawLine(x, 60, x, 90, (i == fingerNum-1) ? YELLOW : WHITE, 2, 1);
  }
  
  // Draw thumb (horizontal)
  Paint_DrawLine(200, 100, 230, 100, YELLOW, 2, 1);
  
  // Highlight the target finger
  if (fingerNum > 0 && fingerNum <= 4) {
    int x = 210 + ((fingerNum-1) * 15);
    Paint_DrawLine(x, 90, 230, 100, RED, 3, 1);
  }
}

void RehabMode::displayProgress() {
  // Draw progress text
  char progressText[20];
  sprintf(progressText, "Progress: %d/%d", successCount, totalExercises);
  drawBoxedString(10, 80, progressText, GREEN, BLACK);
  
  // Draw progress bar
  int barWidth = 100;
  int progress = (barWidth * successCount) / totalExercises;
  
  Paint_DrawRectangle(10, 105, 10 + barWidth, 120, WHITE, 1, 0);
  if (progress > 0) {
    Paint_DrawRectangle(10, 105, 10 + progress, 120, GREEN, 1, 1);
  }
}

void RehabMode::showCompletionMessage() {
  // Called when all exercises are done
  drawBoxedString(10, 40, "All exercises complete!", GREEN, BLACK);
  drawBoxedString(10, 70, "Press thumb to return", GREEN, BLACK);
  drawBoxedString(10, 100, "to the main menu", GREEN, BLACK);
}

void RehabMode::drawBoxedString(const uint16_t x, const uint16_t y, const char* string, 
                               const uint16_t foreColor, const uint16_t backgroundColor) {
  Paint_DrawString_EN(x, y, string, &Font20, foreColor, backgroundColor);
}

void RehabMode::clearScreen() {
  Paint_Clear(BLACK);
}