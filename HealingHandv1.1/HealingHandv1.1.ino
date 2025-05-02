/*
  HealingHand Device - Arduino Rehabilitation System
  Developed by Anjalee Gitthens, Godwin Igbekoyi, Dillon Hughes, Kemma Kolstrup, and Erica Nichols
*/
/*
  Features:
  - BLOCK BREAKOUT game for motor training
  - Finger mobility rehabilitation mode
  - User profile management
  - Grip strength and finger bend angle tracking

  Hardware:
  - Arduino Mega2560
  - WaveShare 1.9in LED screen
  - Flex sensors for finger position
  - Force sensors for grip strength measurement
*/

// Standard Arduino libraries
#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Wire.h>

// LCD related libraries
#include "LCD_Driver.h"
#include "GUI_Paint.h"

// Custom component libraries
#include "SingleTact.h"
#include "FlexSensor.h"
#include "AdafruitFSR.h"
#include "RehabMode.h"
#include "Breakout.h"

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
#define PRIMARY_COLOR 0x4A11
#define PRIMARY_LIGHT_COLOR 0x7A17
#define PRIMARY_DARK_COLOR 0x4016
#define PRIMARY_TEXT_COLOR 0x7FFF

// Button pins (if still used)
#define buttonA 2
#define buttonB 3

// Create instances of our component classes
SingleTact singleTact;
FlexSensor flexSensor;
AdafruitFSR fsrSensor;
RehabMode rehabMode;
Breakout breakout;

// Global variables
int HHState = 0;        // Overall device state-tracker
int selection = -1;     // Controller for game interface
int userID = 0;         // Current user ID

// User statistics variables
int userFlex1, userFlex2, userFlex3, userFlex4, userFlexAvg = 0;
int userForceAvg, userLvl, userHiScore = 0;

// Current sensor reading variables
int flex1, flex2, flex3, flex4, flexAvg = 0;
int force1, force2, force3, force4, force5, force6, forceAvg = 0;

// Function prototypes
void initLCD();
void clearDialog();
void drawBoxedString(const uint16_t x, const uint16_t y, const char* string, 
                    const uint16_t foreColor, const uint16_t backgroundColor);
void displayUsers();
int getSelection();
int waitForThumbPress();
void newUser();
void loadUserData(int UID);
void storeUserData(int UID);
void switchToGame(int level);
void switchToMenu();
void switchToRehabMode();
void shutDown(int UID);
void updateSensorValues();

//////////////////////////////////////////////////////////////
// ARDUINO SETUP
//////////////////////////////////////////////////////////////
void setup() {
  // Initialize LCD
  initLCD();
  
  // Initialize serial communication
  Serial.begin(57600);
  Serial.flush();
  
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  // Initialize sensor systems
  singleTact.begin();
  flexSensor.begin();
  fsrSensor.begin();
  
  // Display startup screen
  drawBoxedString(80, 10, "HealingHand", LIGHTBLUE, WHITE);
  drawBoxedString(10, 35, "Developed by BME Group01: AG, DH, GI, KK, EN", BLUE, WHITE);
  drawBoxedString(10, 110, "UNR CoE Capstone 2025", DARKBLUE, WHITE);
  drawBoxedString(10, 150, "Press thumb to start", GREEN, BLACK);
  
  while (waitForThumbPress() < 0) {}
  clearDialog();
  
  // Display user selection screen
  drawBoxedString(10, 10, "Select User", LIGHTBLUE, WHITE);
  displayUsers();
  
  // Get user selection
  userID = -1;
  while (userID == -1) {
    userID = getSelection();
  }
  clearDialog();
  
  // Handle new user and load user data
  if (userID == 0) {
    newUser();
  } else {
    loadUserData(userID);
  }
}

//////////////////////////////////////////////////////////////
// ARDUINO LOOP
//////////////////////////////////////////////////////////////
void loop() {
  updateSensorValues();
  
  switch((int)HHState) {
    case 0: {//main menu
      
      // Display main menu with options
      clearDialog();
      drawBoxedString(10, 10, "MAIN MENU", LIGHTBLUE, WHITE);
      drawBoxedString(10, 35, "1: View Stats", YELLOW, BLACK);
      drawBoxedString(10, 60, "2: Play BLOCK BREAKOUT", YELLOW, BLACK);
      drawBoxedString(10, 85, "3: Mobility Rehab", YELLOW, BLACK);
      drawBoxedString(10, 110, "4: Shut Down", YELLOW, BLACK);
      drawBoxedString(10, 135, "1-4: Bend fingers 1-4", GREEN, BLACK);
      
      // Wait for menu selection
      int menuChoice = -1;
      while (menuChoice <= 0) {
        // Check finger flexion for menu choice
        menuChoice = getSelection();
        delay(100);
      }
      
      // Process menu choice
      switch (menuChoice) {
        case 1: {// View Stats
          clearDialog();
          char buff[3];
          char userStr[15] = "USER ";
          strcat(userStr, itoa(userID, buff, 10));
          strcat(userStr, " STATS");
          drawBoxedString(90, 10, userStr, LIGHTBLUE, WHITE);
  
          drawBoxedString(10, 32, "Flex(deg)|1 |2 |3 |4  ", YELLOW, RED);
          char flexStr[20] = "avg:" ;
          strcat(flexStr, itoa(userFlexAvg, buff, 10));
          strcat(flexStr, " |");
          strcat(flexStr, itoa(userFlex1, buff, 10));
          strcat(flexStr, "|");
          strcat(flexStr, itoa(userFlex2, buff, 10));
          strcat(flexStr, "|");
          strcat(flexStr, itoa(userFlex3, buff, 10));
          strcat(flexStr, "|");
          strcat(flexStr, itoa(userFlex4, buff, 10));
          drawBoxedString(10, 54, flexStr, YELLOW, RED);
  
          char forceString[10] = "Force: ";
          strcat(forceString, itoa(userForceAvg, buff, 10));
          strcat(forceString, "N");
          drawBoxedString(10, 76, forceString, MAGENTA, BLACK);
  
          char lvlString[15] = "High Score: ";
          strcat(lvlString, itoa(userHiScore, buff, 10)); 
          drawBoxedString(10, 98, lvlString, BLUE, WHITE);
  
          char lvlString2[20] = "Current Level: ";
          strcat(lvlString2, itoa(userLvl, buff, 10));
          drawBoxedString(10, 120, lvlString2, BLUE, WHITE);
  
          drawBoxedString(10, 142, "Press thumb to continue", GREEN, BLACK);
          while (waitForThumbPress() < 0) {}
          clearDialog();
        }
        break;
          
        case 2: { // Play Breakout
          switchToGame(userLvl);   
        }
        break;
          
        case 3: { // Mobility Rehab
          switchToRehabMode();
        }
        break;
          
        case 4: { // Shut Down
          shutDown(userID);
        }
        break;
      }
      break;
    }
    
    case 1: { // Play game (Breakout)
      // Update sensor values
      updateSensorValues();
      
      // Update game with current flex sensor values
      breakout.update(flex1, flex2);
      
      // Check for power-up activation
      if (breakout.hasPowerUp()) {
        bool thumbPressed = (force1 > 500);
        breakout.checkAndActivatePowerUp(thumbPressed);
      }
      
      // Check if level is complete
      if (breakout.checkLevelComplete() && breakout.getLevel() < GAMES_NUMBER - 1) {
        breakout.nextLevel();
      } 
      // Check if game is over
      else if (breakout.isGameOver()) {
        // Update user high score if needed
        if (breakout.getScore() > userHiScore) {
          userHiScore = breakout.getScore();
        }
        
        // Update user level if needed
        if (breakout.getLevel() > userLvl) {
          userLvl = breakout.getLevel();
        }
        
        // Show game over and wait for user input
        breakout.drawGameOver();
        while (waitForThumbPress() < 0) {}
        
        // Return to main menu
        switchToMenu();
      }
      break;
    }
    
    case 2: { // Rehab mode
      // Initialize the mode if we just entered it
      if (rehabMode.getCurrentExercise() == 0) {
        rehabMode.begin();
        
        // Wait for user to start
        while (waitForThumbPress() < 0) {}
        delay(500); // Debounce
        
        // Start first exercise
        rehabMode.startExercise(1);
      }
      
      // Update sensor values
      updateSensorValues();
      
      // Increment exercise tracker
      rehabMode.incrementExerciseTracker();
      
      // Update rehab mode with current sensor values
      rehabMode.update(flex1, flex2, flex3, flex4, force1);
      
      // Check if rehab mode is complete
      if (rehabMode.isComplete()) {
        // Wait for user to continue
        while (force1 < 500) {
          delay(50);
          updateSensorValues();
        }
        delay(500); // Debounce
        
        // Reset and return to main menu
        rehabMode.reset();
        switchToMenu();
      }
      break;
    }
  }
}

//////////////////////////////////////////////////////////////
// LCD FUNCTIONS
//////////////////////////////////////////////////////////////
void initLCD() {
  Config_Init();
  LCD_Init();

  LCD_SetBacklight(100);
  Paint_SetRotate(ROTATE_270);
  Paint_NewImage(170, 320, 270, BLACK);
  Paint_Clear(BLACK);
}

void clearDialog() {
  LCD_SetBacklight(100);
  Paint_SetRotate(ROTATE_270);
  Paint_NewImage(170, 320, 270, BLACK);
  Paint_Clear(BLACK);
}

void drawBoxedString(const uint16_t x, const uint16_t y, const char* string, 
                    const uint16_t foreColor, const uint16_t backgroundColor) {
  Paint_DrawString_EN(x, y, string, &Font20, foreColor, backgroundColor);
}

//////////////////////////////////////////////////////////////
// SENSOR FUNCTIONS
//////////////////////////////////////////////////////////////
void updateSensorValues() {
  // Read flex sensors and update max values
  flex1 = flexSensor.getIndex();
  flex2 = flexSensor.getMiddle();
  flex3 = flexSensor.getRing();
  flex4 = flexSensor.getPinky();
  flexAvg = flexSensor.getAverage();
  flexSensor.updateMaxValues();
  // Update user stats from max values
  userFlex1 = flexSensor.getMaxIndex();
  userFlex2 = flexSensor.getMaxMiddle();
  userFlex3 = flexSensor.getMaxRing();
  userFlex4 = flexSensor.getMaxPinky();
  userFlexAvg = flexSensor.getMaxAverage();
  
  // Read force sensors
  force1 = fsrSensor.getThumb();
  force2 = fsrSensor.getIndex();
  forceAvg = fsrSensor.getAverage();
  fsrSensor.updateMaxValues();
  
  // Read SingleTact sensors
  force5 = singleTact.readSensorNewtons(SENSOR1_ADDRESS);
  force6 = singleTact.readSensorNewtons(SENSOR2_ADDRESS);
  // Update max force value
  userForceAvg = fsrSensor.getMaxValue();
}

//////////////////////////////////////////////////////////////
// USER INTERFACE FUNCTIONS
//////////////////////////////////////////////////////////////
void displayUsers() {
  // Display new user option
  drawBoxedString(10, 45, "New User (0)", YELLOW, BLACK);
  int addr = 0;
  int i = 1;
  
  // Check if user data is stored
  while (EEPROM.read(addr) != 255) {
    // Display existing users
    char userStr[10] = "USER ";
    char buff[10];
    strcat(userStr, itoa(i, buff, 10));
    drawBoxedString(200, addr+10, userStr, YELLOW, BLACK);
    addr += 32;
    i++;
  }
  
  drawBoxedString(10, 130, "0 - Press thumb", GREEN, BLACK);
  drawBoxedString(10, 150, "1-4: Bend fingers 1-4", GREEN, BLACK);
}

int getSelection() {
  updateSensorValues();
  if (fsrSensor.isThumbPressed(500)) {
    return 0;
  }
  if (flex1 > 200) {
    return 1;
  }
  if (flex2 > 215) {
    return 2;
  }
  if (flex3 > 200) {
    return 3;
  }
  if (flex4 > 215) {
    return 4;
  }
  return -1;
}

int waitForThumbPress() {
  if (fsrSensor.isThumbPressed(500)) {
    return 1;
  }
  return -1;
}

void newUser() {
  int UID = 0;
  while (EEPROM.read(UID) != 255) {
    UID = UID + 32;
  }
  UID = UID / 32;
  userID = UID + 1;
}

void loadUserData(int UID) {
  int addr = (UID - 1) * 32;
  userFlex1 = EEPROM.read(addr);
  userFlex2 = EEPROM.read(addr + 1);
  userFlex3 = EEPROM.read(addr + 2);
  userFlex4 = EEPROM.read(addr + 3);
  userFlexAvg = EEPROM.read(addr + 4);
  userForceAvg = EEPROM.read(addr + 5);
  userLvl = EEPROM.read(addr + 6);
  userHiScore = EEPROM.read(addr + 7);
}

void storeUserData(int UID) {
  int addr = (UID - 1) * 32;
  EEPROM.update(addr, userFlex1);
  EEPROM.update(addr + 1, userFlex2);
  EEPROM.update(addr + 2, userFlex3);
  EEPROM.update(addr + 3, userFlex4);
  EEPROM.update(addr + 4, userFlexAvg);
  EEPROM.update(addr + 5, userForceAvg);
  EEPROM.update(addr + 6, userLvl);
  EEPROM.update(addr + 7, userHiScore);
}

//////////////////////////////////////////////////////////////
// MODE SWITCHING FUNCTIONS
//////////////////////////////////////////////////////////////
void switchToGame(int level) {
  HHState = 1;
  breakout.displayControlScreen();
  
  // Wait for thumb press to continue
  while (waitForThumbPress() < 0) {}
  delay(500); // Debounce
  
  breakout.initializeGame(level);
}

void switchToMenu() {
  HHState = 0;
  clearDialog();
}

void switchToRehabMode() {
  HHState = 2;
  rehabMode.reset();
  clearDialog();
}

void shutDown(int UID) {
  clearDialog();
  drawBoxedString(10, 10, "Storing data, please wait", BLUE, WHITE);
  storeUserData(UID);
  delay(100);
  clearDialog();
  drawBoxedString(10, 10, "Data stored!", BLUE, WHITE);
  drawBoxedString(10, 40, "Power off device now", BLUE, WHITE);
  delay(10000); // Wait before power-off
}
