/*
  HealingHand Device - Arduino Rehabilitation System

for final version:

check all DELETE, REPLACE, UPDATE comments
reorganize into multiple files?
more rehab exercises?


*/



/*PIN OUT FOR LCD AND JOYSTICK
LCD:
  BL 9
  RST 8
  DC 7
  CS 10
  CLK 13 52
  DIN 11 51
  GND GND
  VCC 5V

JOYSTICK:
  GND GND
  +5V 3.3V
  VRX A0
  VRY A1
*/
/*
  Features:
  - Breakout game for motor training
  - Finger mobility rehabilitation mode
  - User profile management
  - Grip strength and finger bend angle tracking

  Hardware:
  - Arduino Mega2560
  - AZ-Delivery 2.4 TFT LCD Display
  - Flex sensors for finger position
  - Force sensors for grip strength measurement
*/

//for LCD function
#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"
//for data storage
#include <EEPROM.h>
//for sensor function
#define FLEX1A A0
#define FLEX1B A1
#define FLEX2A A2
#define FLEX2B A3
#define FLEX3A A4
#define FLEX3B A5
#define FLEX4A A6
#define FLEX4B A7

#define FORCE1 A8
#define FORCE2 A9
#define FORCE3 A10
#define FORCE4 A11

#include <Wire.h> //For I2C/SMBus

// Constants for the four pressure sensors
const float FORCE_SCALE = 0.01; 
const byte SENSOR1_ADDRESS = 0x06; 
const byte SENSOR2_ADDRESS = 0x08; 
int16_t baseline1 = 0; // Store baseline value for taring sensors
int16_t baseline2 = 0; 


//DELETE
//artifact of previous control regimen
#define joystick_x A1
#define joystick_y A0

#define buttonA 2
#define buttonB 3

//for LCD/game function
#define BLACK   0x0000
#define BLUE    0x001F
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

#define SCORE_SIZE 30
#define REHAB_TIMEOUT 10000  // Timeout for each exercise in milliseconds
//UPDATE
#define COMPLETE_THRESHOLD 800  // Threshold to consider exercise complete
char scoreFormat[] = "%04d";

typedef struct gameSize_type {
  int16_t x, y, width, height;
} gameSize_type;

gameSize_type gameSize;
uint16_t backgroundColor = BLACK;
int level;

const uint8_t BIT_MASK[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint8_t pointsForRow[] = {7, 7, 5, 5, 3, 3 , 1, 1};
#define GAMES_NUMBER 16

//defines game structure, important values
typedef struct game_type {
  int ballsize;
  int playerwidth;
  int playerheight;
  int exponent;
  int top;
  int rows;
  int columns;
  int brickGap;
  int lives;
  int wall[GAMES_NUMBER];
  int initVelx;
  int initVely;
} game_type;

//defines levels
game_type games[GAMES_NUMBER] =
  // ballsize, playerwidth, playerheight, exponent, top, rows, columns, brickGap, lives, wall[8],                                         initVelx, initVely
{
  { 5,              40,          8,           6,        0 ,      8,       8, 3,       3,  {0x18, 0x66, 0xFF, 0xDB, 0xFF, 0x7E, 0x24, 0x3C} , 100, -100},
  { 10,             50,          8,           6,        40 ,     8,       8, 3,       3,  {0xFF, 0x99, 0xFF, 0xE7, 0xBD, 0xDB, 0xE7, 0xFF} , 28, -28},
  { 10,             50,          8,           6,        40 ,     8,       8, 3,       3,  {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55} , 28, -28},
  { 8,              50,          8,           6,        40 ,     8,       8, 3,       3,  {0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF} , 34, -34},
  { 10,             40,          8,           6,        40 ,     8,       8, 3,       3,  {0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF} , 28, -28},
  { 10,             40,          8,           6,        40 ,     8,       8, 3,       3,  {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA} , 28, -28},
  { 12,             64,          8,           6,        60 ,     4,       2, 3,       4,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 20, -20},
  { 12,             60,          8,           6,        60 ,     5,       3, 3,       4,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 22, -22},
  { 10,             56,          8,           6,        30 ,     6,       4, 3,       4,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 24, -24},
  { 10,             52,          8,           6,        30 ,     7,       5, 3,       4,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 26, -26},
  { 8,              48,          8,           6,        30 ,     8,       6, 3,       3,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 28, -28},
  { 8,              44,          8,           6,        30 ,     8,       7, 3,       3,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 30, -30},
  { 8,              40,          8,           6,        30 ,     8,       8, 3,       3,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 32, -32},
  { 8,              36,          8,           6,        40 ,     8,       8, 3,       3,  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , 34, -34},
  { 8,              36,          8,           6,        40 ,     8,       8, 3,       3,  {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA} , 34, -34}
};

game_type* game;
//defines state of the game
typedef struct game_state_type {
  uint16_t ballx;
  uint16_t bally;
  uint16_t ballxold;
  uint16_t ballyold;
  int velx;
  int vely;
  int playerx;
  int playerxold;
  int wallState[8];
  int score;
  int remainingLives;
  int top;
  int bottom;
  int walltop;
  int wallbottom ;
  int brickheight;
  int brickwidth;
  int powerUpFlag;
  int powerUpType;
  int playerSpeed;
  int powerUpCounter;

} game_state_type;

game_state_type state;

//variables used for reporting to user
//these update when a new max value is reached while playing the game
int userFlex1, userFlex2, userFlex3, userFlex4, userFlexAvg, userForceAvg, userLvl, userHiScore, userID = 0;
//variables used for keeping track of things in code
//these update every loop
int flex1, flex2, flex3, flex4, flexAvg, force1, force2, force3, force4, force5, force6, forceAvg = 0;
//state variables for keeping track of device function
int rehabExercise = 0;  // Current exercise in rehab mode
int exerciseTracker = 0;  // variable to track the number of times an exercise has been attempted
int exerciseComplete = 0;  // Flag to track if current exercise is complete
int totalExercises = 4;  // Total number of exercises in the sequence
int successCount = 0;  // Counter for successful exercises

int selection = -1; //controller for game interface
int HHState = 0; //overall device state-tracker

// Function prototypes

//Singletact force sensor functions
void scanI2C();
void calibratePressureSensors();
void readAndDisplayPressureData();
void calibrateSingleSensor(byte sensorAddress, int sensorNumber);
short readDataFromSensor(byte address);

//LCD functions
void initLCD();
void clearDialog();
void drawBoxedString(const uint16_t x, const uint16_t y, const char* string, const uint16_t foreColor, const uint16_t backgroundColor);

//Game functions
void displayControlScreen();
void newGame(game_type* newGame, game_state_type* state);
void setupStateSizes(game_type* game, game_state_type* state);
void setupState(game_type* game, game_state_type* state);
void updateLives(int lives, int remainingLives);
void setupWall(game_type* game, game_state_type* state);
void drawBrick(game_state_type* state, int xBrick, int yBrickRow, uint16_t backgroundColor);
boolean noBricks(game_type* game, game_state_type* state);
void drawPlayer(game_type* game, game_state_type* state);
void drawBall(int x, int y, int xold, int yold, int ballsize);
void updateScore(int score);
void checkBrickCollision(game_type* game, game_state_type* state, uint16_t x, uint16_t y);
int checkCornerCollision(game_type* game, game_state_type* state, uint16_t x, uint16_t y);
void hitBrick(game_state_type* state, int xBrick, int yBrickRow);
void checkBorderCollision(game_type* game, game_state_type* state, uint16_t x, uint16_t y);
void checkBallCollisions(game_type* game, game_state_type* state, uint16_t x, uint16_t y);
void checkBallExit(game_type* game, game_state_type* state, uint16_t x, uint16_t y);
void setBrick(int wall[], uint8_t x, uint8_t y);
void unsetBrick(int wall[], uint8_t x, uint8_t y);
boolean isBrickIn(int wall[], uint8_t x, uint8_t y);
int readGameControls(game_type* game, game_state_type* state, const int16_t lastSelected);
void displayPowerUp(int type);

//Menu/control functions
void displayUsers();
int getSelection();
int waitForThumbPress();
void newUser();
void loadUserData(int UID);
void storeUserData(int UID);
void switchToGame(int level);
void switchToMenu();
void shutDown(int UID);

//sensor interface functions
void updateSensorValues();
int readFlexSensor(int pin1, int pin2);
int flex2Deg(int reading);
int readAdaForce(int pin);
int readSingleTact(const byte sensorAddress);

// Rehab mode functions
void drawRehabExercise(const char* instructionText, const char* holdText, int fingerNum);
void drawFingerDiagram(int fingerNum);
void displayRehabProgress(int completed, int total);
void finishRehabMode();
void switchToRehabMode();

//////////////////////////////////////////////////////////////
// ARDUINO SETUP
//////////////////////////////////////////////////////////////
void setup() {
  initLCD();
  gameSize = {0, 0, 320, 170};
//DELETE
  EEPROM.update(0,128);
  EEPROM.update(6,0);
  EEPROM.update(32,128);
  EEPROM.update(38,0);
  EEPROM.update(64,128);
  EEPROM.update(70,0);
  EEPROM.update(96,128);
  EEPROM.update(102,0);
  EEPROM.update(128,255);

  //from singletact file
  Wire.begin();
  TWBR=12;
  //REPLACE
  Serial.begin(57600);  // start serial for output
  Serial.flush();
  
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  // Run I2C scanner to check for connected devices
  scanI2C();
  //REPLACE
  Serial.println("SingleTact multiple sensor value in N and lbs.");
  Serial.println("----------------------------------------");
  
  // Allow system to stabilize before calibration
  delay(2000);
  
  // Perform initial taring of all pressure sensors

  //REPLACE
  Serial.println("Performing initial calibration, make sure no pressure is applied to sensors...");
  calibratePressureSensors();
  Serial.println("Calibration complete!");

  //Display start up screen

  drawBoxedString(80,10,"HealingHand",LIGHTBLUE,WHITE);
  drawBoxedString(10,35,"Developed by BME Group01: AG, DH, GI, KK, EN",BLUE,WHITE);
  drawBoxedString(10,110,"UNR CoE Capstone 2025",DARKBLUE,WHITE);
  drawBoxedString(10,150,"Press thumb to start",GREEN,BLACK);
  while (waitForThumbPress()<0){}
  clearDialog();

  drawBoxedString(10,10,"Select User",LIGHTBLUE,WHITE);
  displayUsers();
  //get user selection
  userID = -1;
  while (userID == -1){
    userID = getSelection();
  }
  clearDialog();
  
    //handle new user and load user data
  if (userID == 0){
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
      drawBoxedString(10,10,"MAIN MENU",LIGHTBLUE,WHITE);
      drawBoxedString(10,35,"1: View Stats",YELLOW,BLACK);
      drawBoxedString(10,60,"2: Play BLOCK BREAKOUT",YELLOW,BLACK);
      drawBoxedString(10,85,"3: Mobility Rehab",YELLOW,BLACK);
      drawBoxedString(10,110,"4: Shut Down",YELLOW,BLACK);
      drawBoxedString(10,135,"1-4: Bend fingers 1-4",GREEN,BLACK);
      
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
          strcat(userStr,itoa(userID,buff,10));
          strcat(userStr," STATS");
          drawBoxedString(90,10,userStr,LIGHTBLUE,WHITE);
  
          drawBoxedString(10,32,"Flex(deg)|1 |2 |3 |4  ",YELLOW, RED);
          char flexStr[20] = "avg:" ;
          strcat(flexStr,itoa(userFlexAvg,buff,10));
          strcat(flexStr," |");
          strcat(flexStr,itoa(userFlex1,buff,10));
          strcat(flexStr,"|");
          strcat(flexStr,itoa(userFlex2,buff,10));
          strcat(flexStr,"|");
          strcat(flexStr,itoa(userFlex3,buff,10));
          strcat(flexStr,"|");
          strcat(flexStr,itoa(userFlex4,buff,10));
          drawBoxedString(10,54, flexStr,YELLOW,RED);
  
          char forceString[10] = "Force: ";
          strcat(forceString,itoa(userForceAvg,buff,10));
          strcat(forceString,"N");
          drawBoxedString(10,76,forceString,MAGENTA,BLACK);
  
          char lvlString[15] = "High Score: ";
          strcat(lvlString,itoa(userHiScore,buff,10)); 
          drawBoxedString(10,98,lvlString,BLUE,WHITE);
  
          char lvlString2[20] = "Current Level: ";
          strcat(lvlString2,itoa(userLvl,buff,10));
          drawBoxedString(10,120,lvlString2,BLUE,WHITE);
  
          drawBoxedString(10,142,"Press thumb to continue",GREEN,BLACK);
          while (waitForThumbPress()<0){}
          clearDialog();
        }
          break;
          
        case 2:{ // Play Breakout
          switchToGame(userLvl);
          Serial.println(HHState);    
      }
      break;
          
        case 3: { // Mobility Rehab
          switchToRehabMode();
        }
          break;
        case 4: {// Shut Down
          shutDown(userID);
        }
          break;
      }
      break;
    }
    
    case 1: { //play game
      //read sensor values for proper game function
      selection = readGameControls(game, &state, selection);
    
      drawPlayer(game, &state);
      // store old position to remove old pixels
      state.playerxold = state.playerx;

      // check max speed
      if (abs( state.vely) > ((1 << game->exponent) - 1)) {
        state.vely = ((1 << game->exponent) - 1) * ((state.vely > 0) - (state.vely < 0));
      }
      if (abs( state.velx) > ((1 << game->exponent) - 1)) {
        state.velx = ((1 << game->exponent) - 1) * ((state.velx > 0) - (state.velx < 0));
      }

      state.ballx += state.velx;
      state.bally += state.vely;

      // check ball collisions and exit
      checkBallCollisions(game, &state, state.ballx >> game->exponent, state.bally >> game->exponent);
      checkBallExit(game, &state, state.ballx >> game->exponent, state.bally >> game->exponent);

      // draw ball in new position
      drawBall(state.ballx >> game->exponent, state.bally >> game->exponent, state.ballxold >> game->exponent, state.ballyold >> game->exponent, game->ballsize );

      // store old position to remove old pixels
      state.ballxold = state.ballx;
      state.ballyold = state.bally;

      // increment velocity
      state.velx = (40 + (state.score >> 3 )) * ( (state.velx > 0) - (state.velx < 0));
      state.vely = (40 + (state.score >> 3 )) * ( (state.vely > 0) - (state.vely < 0));

      // if no bricks go to next level
      if (noBricks(game, &state) && level < GAMES_NUMBER) {
        level++;
        newGame( &games[level], &state);
      } else if ( state.remainingLives <= 0) {
        if (state.score > userHiScore){
          userHiScore = state.score;
        }
        if (level > userLvl){
          userLvl = level;
        }
        state.score = 0;
        drawBoxedString(10,80,"GAME OVER",YELLOW,BLACK);
        drawBoxedString(10,110,"Press thumb to return to Main Menu",YELLOW,BLACK);
        while (waitForThumbPress()<0){}
        switchToMenu();
      }
      break;
    }

    case 2: { //rehab mode
      // Initialize the mode if we just entered it
      if (rehabExercise == 0) {
        clearDialog();
        drawBoxedString(10, 10, "Mobility Rehabilitation", LIGHTBLUE, WHITE);
        drawBoxedString(10, 45, "Follow the prompts to", YELLOW, BLACK);
        drawBoxedString(10, 70, "perform finger exercises", YELLOW, BLACK);
        drawBoxedString(10, 105, "Press thumb sensor to start", GREEN, BLACK);
        
        // Wait for user to start
        while (waitForThumbPress() < 0) {}
        delay(500); // Debounce
        
        // Start first exercise
        rehabExercise = 1;
        exerciseTracker = 0;
        exerciseComplete = 0;
        successCount = 0;
        clearDialog();
      }
      
      // Update sensor values
      updateSensorValues();
      
      // Display progress bar
      displayRehabProgress(successCount, totalExercises);
      // Check if we need to timeout and move to next exercise
      if ((exerciseTracker > 7) && !exerciseComplete) {
        Serial.println("Timeout reached, moving on");
        // Move to next exercise if timeout
        rehabExercise++;
        exerciseTracker =0;
        exerciseComplete = 0;
        
        if (rehabExercise > totalExercises) {
          // End of all exercises
          Serial.println("all exercises finished");
          finishRehabMode();
          //break;
        }
        clearDialog();
      }
      exerciseTracker += 1;

      // Handle different exercises
      switch (rehabExercise) {
        case 1:
          // Index finger to thumb
          drawRehabExercise("Touch INDEX finger to thumb", 
                          "Hold for 2 seconds",
                          1);
          // Check if exercise is completed (using sensor data)
          if (readAdaForce(FORCE1) > COMPLETE_THRESHOLD && flex1 < 400 && !exerciseComplete && exerciseTracker > 3) {
            Serial.println("Exercise 1 completed");
            // Exercise completed
            exerciseComplete = 1;
            successCount++;
            drawBoxedString(10, 110, "Great job!", GREEN, BLACK);
            delay(2000);
            
            // Move to next exercise
            Serial.println("moving to next exercise");
            rehabExercise++;
            exerciseTracker =0;
            exerciseComplete = 0;
            clearDialog();
          }
          break;
          
        case 2:
          // Middle finger to thumb
          drawRehabExercise("Touch MIDDLE finger tothumb", 
                          "Hold for 2 seconds",
                          2);
          
          // Check if exercise is completed (using sensor data)
          if (readAdaForce(FORCE1) > COMPLETE_THRESHOLD && flex2 < 400 && !exerciseComplete && exerciseTracker > 3) {
            // Exercise completed
            exerciseComplete = 1;
            successCount++;
            drawBoxedString(10, 110, "Excellent work!", GREEN, BLACK);
            delay(2000);
            
            // Move to next exercise
            rehabExercise++;
            exerciseTracker =0;
            exerciseComplete = 0;
            clearDialog();
          }
          break;
          
        case 3:
          // Ring finger to thumb
          drawRehabExercise("Touch RING finger to  thumb", 
                          "Hold for 2 seconds",
                          3);
          
          // Check if exercise is completed (using sensor data)
          if (readAdaForce(FORCE1) > COMPLETE_THRESHOLD && flex3 < 400 && !exerciseComplete && exerciseTracker > 3) {
            // Exercise completed
            exerciseComplete = 1;
            successCount++;
            drawBoxedString(10, 110, "Well done!", GREEN, BLACK);
            delay(2000);
            
            // Move to next exercise
            rehabExercise++;
            exerciseTracker =0;
            exerciseComplete = 0;
            clearDialog();
          }
          break;
          
        case 4:
          // Pinky finger to thumb
          drawRehabExercise("Touch PINKY finger to thumb", 
                          "Hold for 2 seconds",
                          4);
          
          // Check if exercise is completed (using sensor data)
          if (readAdaForce(FORCE1) > COMPLETE_THRESHOLD && flex4 < 400 && !exerciseComplete && exerciseTracker > 3) {
            // Exercise completed
            exerciseComplete = 1;
            successCount++;
            drawBoxedString(10, 110, "Perfect!", GREEN, BLACK);
            delay(2000);
            
            // All exercises complete
            finishRehabMode();
          }
          break;
      }
      break;
    }
  }
}


//////////////////////////////////////////////////////////////////////////////////
//SINGLETACT IMPLEMENTATION///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
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
  
  Wire.beginTransmission(SENSOR1_ADDRESS);
  if (Wire.endTransmission() == 0) sensor1Found = true;
  
  Wire.beginTransmission(SENSOR2_ADDRESS);
  if (Wire.endTransmission() == 0) sensor2Found = true;
  
  
  if (!sensor1Found) {
    Serial.println("WARNING: Sensor 1 (0x06) not found on I2C bus!");
  }
  
  if (!sensor2Found) {
    Serial.println("WARNING: Sensor 2 (0x08) not found on I2C bus!");
  }
  
  Serial.println("----------------------------------------");
}

void calibratePressureSensors() {
  // Take multiple readings and average them for more stable baselines
  const int numSamples = 10;
  int32_t total1 = 0, total2 = 0, total3 = 0, total4 = 0;
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
  
  // Calculate average for sensor 2 if we have valid samples
  if (validSamples2 > 0) {
    baseline2 = total2 / validSamples2;
    Serial.print("Sensor 2 baseline: ");
    Serial.println(baseline2);
  } else {
    Serial.println("Warning: Could not get valid readings for sensor 2 calibration");
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
  
  Serial.println("----------------------------------------");
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
    }
  } else {
    Serial.println("Warning: Could not get valid readings for calibration");
  }
}

short readDataFromSensor(byte address) {
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

// Mode switching functions
void switchToGame(int level) {
  HHState = 1;
  displayControlScreen();
  newGame(&games[level], &state);
}

void switchToMenu() {
  HHState = 0;
  clearDialog();
}

void switchToRehabMode() {
  HHState = 2;
  rehabExercise = 0;
  clearDialog();
}

void shutDown(int UID){
  clearDialog();
  drawBoxedString(10,10,"Storing data, please wait",BLUE,WHITE);
  storeUserData(UID);
  delay(100);
  clearDialog();
  drawBoxedString(10,10,"Data stored!",BLUE,WHITE);
  drawBoxedString(10,40,"Power off device now",BLUE,WHITE);
  delay(10000);
}

// LCD handling functions
void initLCD() {
  Config_Init();
  LCD_Init();

  LCD_SetBacklight(100);
  Paint_SetRotate(ROTATE_90);
  Paint_NewImage(170, 320, 90, BLACK);
  Paint_Clear(BLACK);
}

void clearDialog() {
  LCD_SetBacklight(100);
  Paint_SetRotate(ROTATE_90);
  Paint_NewImage(170, 320, 90, BLACK);
  Paint_Clear(BLACK);
}

void drawBoxedString(const uint16_t x, const uint16_t y, const char* string, const uint16_t foreColor, const uint16_t backgroundColor) {
  Paint_DrawString_EN(x, y, string, &Font20, foreColor, backgroundColor);
}

// Sensor reading functions
void updateSensorValues() {
  //add logic to keep track of maximum values
  flex1 = readFlexSensor(FLEX1A, FLEX1B);
  flex2 = readFlexSensor(FLEX2A, FLEX2B);
  flex3 = readFlexSensor(FLEX3A, FLEX3B);
  flex4 = readFlexSensor(FLEX4A, FLEX4B);
  flexAvg = (flex1 + flex2 + flex3 + flex4) / 4;

  force1 = readAdaForce(FORCE1);
  force2 = readAdaForce(FORCE2);
  force3 = readAdaForce(FORCE3);
  force4 = readAdaForce(FORCE4);

  force5 = readSingleTact(SENSOR1_ADDRESS);
  force6 = readSingleTact(SENSOR2_ADDRESS);
  forceAvg = 0; //not sure how to calc yet

  if (flex2Deg(flex1) > userFlex1){
    userFlex1 = flex2Deg(flex1);
  }
  if (flex2Deg(flex2) > userFlex2){
    userFlex2 = flex2Deg(flex2);
  }
  if (flex2Deg(flex3) > userFlex3){
    userFlex3 = flex2Deg(flex3);
  }
  if (flex2Deg(flex4) > userFlex4){
    userFlex4 = flex2Deg(flex4);
  }
  if (flex2Deg(flexAvg) > userFlexAvg){
    userFlexAvg = flex2Deg(flexAvg);
  }
  if (forceAvg > userForceAvg){
    userForceAvg = forceAvg;
  }
  
}

int readFlexSensor(int pin1, int pin2) {
  int readA = analogRead(pin1);
  int readB = analogRead(pin2);
  int reading = readB - readA;
  return reading;
}

int flex2Deg(int reading) {
  //calibrated using tracker/matlab method
  int deg = (-0.0013 * reading * reading) + (1.0877*reading) - 59.9341;
  return deg;
}

int readAdaForce(int pin) {
  int fsrVolt = analogRead(pin);
  return fsrVolt;
}

int force2N(int reading){
  int fsrForce = 0;
  int fsrResistance = 5000 - reading;     // fsrVoltage is in millivolts so 5V = 5000mV
  fsrResistance *= 10000;                // 10K resistor
  fsrResistance /= reading;
 
  int fsrConductance = 1000000;       // we measure in micromhos so 
  fsrConductance /= fsrResistance;
  if (fsrConductance <= 1000) {
    fsrForce = fsrConductance / 80;
  } else {
    fsrForce = fsrConductance - 1000;
    fsrForce /= 30;           
  }
  return fsrForce;
}

int readSingleTact(const byte sensorAddress) {
  //should be as simple as copy/paste
  short data1 = readDataFromSensor(sensorAddress);
  if (data1 != -1) { // If sensor reading was successful
    // Apply baseline correction
    short correctedData1 = data1 - baseline1;
    
    // Convert to force units
    float forceNewtons1 = max(0.0, correctedData1 * FORCE_SCALE); // Prevent negative values
    float forcePounds1 = forceNewtons1 * 0.224809; // Convert to pounds

    return forceNewtons1;
  }
  return 0;
}

// User interface functions
void displayUsers() {
  //display new user option
  drawBoxedString(10, 45, "New User (0)", YELLOW, BLACK);
  int addr = 0;
  int i = 1;
  //check if user data is stored
  while (EEPROM.read(addr) != 255) {
    //display the user as it exists
    char userStr[10] = "USER ";
    char buff[10];
    strcat(userStr, itoa(i, buff, 10));
    drawBoxedString(200, addr+10, userStr, YELLOW, BLACK);
    addr += 32;
    i++;
  }
  drawBoxedString(10,130,"0 - Press thumb",GREEN,BLACK);
  drawBoxedString(10,150,"1-4: Bend fingers 1-4",GREEN,BLACK);
}
//UPDATE
int getSelection() {
  if (readAdaForce(FORCE1) > 500) {
    return 0;
  }
  if (readFlexSensor(FLEX1A, FLEX1B) < 500) {
    return 1;
  }
  if (readFlexSensor(FLEX2A, FLEX2B) < 500) {
    return 2;
  }
  if (readFlexSensor(FLEX3A, FLEX3B) < 500) {
    return 3;
  }
  if (readFlexSensor(FLEX4A, FLEX4B) < 500) {
    return 4;
  }
  return -1;
}

int waitForThumbPress() {
  if (readAdaForce(FORCE1) > 500) {
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

// Game functions
void displayControlScreen(){
  clearDialog();
  drawBoxedString(10, 10, "BLOCK BREAKOUT", LIGHTBLUE, WHITE);
  drawBoxedString(10, 35, "Move the paddle by ", GREEN, BLACK);
  drawBoxedString(10, 55, "bending your fingers", GREEN, BLACK);
  drawBoxedString(10, 80, "Break blocks to lvl up", YELLOW, BLACK);
  drawBoxedString(10,105, "Activate powerups by  pressing thumb", MAGENTA,BLACK);
  drawBoxedString(10, 150, "Press thumb to cont.",BLUE,WHITE);
  while (waitForThumbPress() < 0){};
}

void newGame(game_type* newGame, game_state_type* state) {
  game = newGame;
  setupState(game, state);

  clearDialog();
  updateLives(game->lives, state->remainingLives);
  updateScore(state->score);
  setupWall(game, state);
}

void setupStateSizes(game_type* game, game_state_type* state) {
  state->bottom = 170 - 30;
  state->brickwidth = 320 / game->columns;
  state->brickheight = 170 / 24;
}

void setupState(game_type* game, game_state_type* state) {
  setupStateSizes(game, state);
  for (int i = 0; i < game->rows; i++) {
    state->wallState[i] = 0;
  }
  state->playerx = 320 / 2 - game->playerwidth / 2;
  state->remainingLives = game->lives;
  state->bally = state->bottom << game->exponent;
  state->ballyold = state->bottom << game->exponent;
  state->velx = game->initVelx;
  state->vely = game->initVely;

  state->playerSpeed = 1;
  state->powerUpCounter = 0;
}

void updateLives(int lives, int remainingLives) {
  for (int i = 0; i < lives; i++) {
    Paint_DrawCircle((1+i)*15, 15, 5, BLACK, 5, 1);
  }
  for (int i = 0; i < remainingLives; i++) {
    Paint_DrawCircle((1+i)*15, 15, 5, YELLOW, 5, 1);
  }
}

void setupWall(game_type* game, game_state_type* state) {
  int colors[] = {RED, RED, BLUE, BLUE, YELLOW, YELLOW, GREEN, GREEN};
  state->walltop = game->top + 40;
  state->wallbottom = state->walltop + game->rows * state->brickheight;
  for (int i = 0; i < game->rows; i++) {
    for (int j = 0; j < game->columns; j++) {
      if (isBrickIn(game->wall, j, i)) {
        setBrick(state->wallState, j, i);
        drawBrick(state, j, i, colors[i]);
      }
    }
  }
}

void drawBrick(game_state_type* state, int xBrick, int yBrickRow, uint16_t backgroundColor) {
  int brickxStart = (state->brickwidth * xBrick) + game->brickGap;
  int brickyStart = state->walltop + (state->brickheight * yBrickRow) + game->brickGap;
  //add *2 to brickGap to increase game speed (looks lamer)
  int brickWidth = state->brickwidth - game->brickGap;
  int brickHeight = state->brickheight - game->brickGap;
  
  Paint_DrawRectangle(
    brickxStart,
    brickyStart,
    brickxStart + brickWidth,   //xend
    brickyStart + brickHeight,  //yend
    backgroundColor, 1, 1);   
}

boolean noBricks(game_type* game, game_state_type* state) {
  for (int i = 0; i < game->rows; i++) {
    if (state->wallState[i]) return false;
  }
  return true;
}

void drawPlayer(game_type* game, game_state_type* state) {
  // paint
  Paint_DrawRectangle(state->playerx, state->bottom, (state->playerx + game->playerwidth), (state->bottom + 1), YELLOW, 1, 0);

  if (state->playerx != state->playerxold) {
    // remove old pixels
    if (state->playerx > state->playerxold) {
      Paint_DrawRectangle(state->playerxold, state->bottom, state->playerxold + state->playerSpeed, (state->bottom + 1), BLACK, 1, 0);
    }
    else {
      Paint_DrawRectangle(state->playerxold + game->playerwidth - state->playerSpeed, state->bottom, state->playerxold + game->playerwidth + 5, (state->bottom + 1), BLACK, 1, 0);
    }
  }
}

void drawBall(int x, int y, int xold, int yold, int ballsize) {
  Paint_DrawRectangle(xold, yold, xold + ballsize, yold + ballsize, BLACK, 1, 0);
  //draw new ball
  Paint_DrawRectangle(x, y, x + ballsize, y + ballsize, CYAN, 1, 1);
}

void updateScore(int score) {
  char buffer[5];
  snprintf(buffer, sizeof(buffer), scoreFormat, score);
  drawBoxedString(320 - 70, 6, buffer, YELLOW, PRIMARY_DARK_COLOR);
}

void checkBrickCollision(game_type* game, game_state_type* state, uint16_t x, uint16_t y) {
  int x1 = x + game->ballsize;
  int y1 = y + game->ballsize;
  int collissions = 0;
  collissions += checkCornerCollision(game, state, x, y);
  collissions += checkCornerCollision(game, state, x1, y1);
  collissions += checkCornerCollision(game, state, x, y1);
  collissions += checkCornerCollision(game, state, x1, y);
  if (collissions > 0) {
    state->vely = (-1 * state->vely);
    if ((((x % state->brickwidth) == 0) && (state->velx < 0))
        || ((((x + game->ballsize) % state->brickwidth) == 0) && (state->velx > 0))) {
      state->velx = (-1 * state->velx);
    }
  }
}

int checkCornerCollision(game_type* game, game_state_type* state, uint16_t x, uint16_t y) {
  if ((y > state->walltop) && (y < state->wallbottom)) {
    int yBrickRow = (y - state->walltop) / state->brickheight;
    int xBrickColumn = (x / state->brickwidth);
    if (isBrickIn(state->wallState, xBrickColumn, yBrickRow)) {
      hitBrick(state, xBrickColumn, yBrickRow);
      return 1;
    }
  }
  return 0;
}

void hitBrick(game_state_type* state, int xBrick, int yBrickRow) {
  state->score += pointsForRow[yBrickRow];
  drawBrick(state, xBrick, yBrickRow, WHITE);
  delay(16);
  drawBrick(state, xBrick, yBrickRow, BLUE);
  delay(8);
  drawBrick(state, xBrick, yBrickRow, backgroundColor);
  unsetBrick(state->wallState, xBrick, yBrickRow);
  updateScore(state->score);
  //check if player should receive powerup !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  if(!state->powerUpFlag && state->score > 0){
    state->powerUpCounter += 1;
    if (state->powerUpCounter > 4){
      state->powerUpFlag = 1;
      state->powerUpCounter = 0;
          //------------------      (max - min + 1) + min 
      state->powerUpType = rand() % (2 - 0 + 1) + 0;
      displayPowerUp(state->powerUpType); //powerup indicator at top of screen display type as well :)))))))
    }
  }
}

void checkBorderCollision(game_type* game, game_state_type* state, uint16_t x, uint16_t y) {
  // check wall collision
  if (x + game->ballsize >= 320) {
    state->velx = -abs(state->velx);
  }
  if (x <= 0) {
    state->velx = abs(state->velx);
  }
  if (y <= SCORE_SIZE) {
    state->vely = abs(state->vely);
  }
  if (((y + game->ballsize) >= state->bottom)
      && ((y + game->ballsize) <= (state->bottom + game->playerheight))
      && (x >= state->playerx)
      && (x <= (state->playerx + game->playerwidth))) {
    // change vel x near player borders
    if (x > (state->playerx + game->playerwidth - 6)) {
      state->velx = state->velx - 1;
    } else if (x < state->playerx + 6) {
      state->velx = state->velx + 1;
    }
    state->vely = -abs(state->vely);
  }
}

void checkBallCollisions(game_type* game, game_state_type* state, uint16_t x, uint16_t y) {
  checkBrickCollision(game, state, x, y);
  checkBorderCollision(game, state, x, y);
}

void checkBallExit(game_type* game, game_state_type* state, uint16_t x, uint16_t y) {
  if (((y + game->ballsize) >= 170)) {
    state->remainingLives--;
    updateLives(game->lives, state->remainingLives);
    delay(500);
    state->vely = -abs(state->vely);
  }
}

void setBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] | BIT_MASK[x];
}

void unsetBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] & ~BIT_MASK[x];
}

boolean isBrickIn(int wall[], uint8_t x, uint8_t y) {
  return wall[y] & BIT_MASK[x];
}

int readGameControls(game_type* game, game_state_type* state, const int16_t lastSelected) {
  //move left when index finger is flexed, right when middle finger is flexed
  //UPDATE
  if (flex1 < 500) {
    state->playerx = state->playerxold + state->playerSpeed;
  } else if (flex2 < 500) {
    state->playerx = state->playerxold - state->playerSpeed;
  } 
  if (state->playerx >= 320 - game->playerwidth) state->playerx = 320 - game->playerwidth;
  if (state->playerx < 0) state->playerx = 0;
  if(state->powerUpFlag && readAdaForce(FORCE1)>500 ){
    //powerup types: increase score (0), gain a life (1), increase paddle speed (2)
    switch(state->powerUpType){
      case 0: {
        //increase score
        state->score += (userLvl +1) * 5;
        updateScore(state->score);
      }
      break;
      case 1: {
        //gain a life
        state->remainingLives+=1;
        game->lives += 1;
        updateLives(game->lives,state->remainingLives);
      }
      break;
      case 2: {
        //increase paddle speed
        state->playerSpeed += 1;
      }
      break;
    }
    state->powerUpFlag =0;
    Paint_DrawRectangle(155,5,165,15,BLACK,1,1);
  }
  return 1;
}

void displayPowerUp(int type){
  drawBoxedString(10,150,"You got a power-up!",YELLOW,BLACK);
  switch (type){
    case 0:{
      //powerup icon
      Paint_DrawRectangle(155,5,165,15,MAGENTA,1,1);
      drawBoxedString(10,150,"Score Increase!      ",MAGENTA,BLACK);
    }
    break;
    case 1:{
      //powerup icon
      Paint_DrawRectangle(155,5,165,15,CYAN,1,1);
      drawBoxedString(10,150,"Extra Life!          ",CYAN,BLACK);
    }
    break;
    case 2: {
      //powerup icon
      Paint_DrawRectangle(155,5,165,15,GREEN,1,1);
      drawBoxedString(10,150,"Paddle Speed Up!     ",GREEN,BLACK);
    }
  }
  delay(1000);
  drawBoxedString(10,150,"                      ",BLACK,BLACK);
}

// Rehabilitation mode functions
void drawRehabExercise(const char* instructionText, const char* holdText, int fingerNum) {
  drawBoxedString(10, 10, "Finger Mobility", BLUE, WHITE);
  drawBoxedString(10, 35, instructionText, YELLOW, BLACK);
  drawBoxedString(10, 125, holdText, YELLOW, BLACK);  
  // Draw finger diagram
  drawFingerDiagram(fingerNum);
}

void drawFingerDiagram(int fingerNum) {
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

void displayRehabProgress(int completed, int total) {
  // Draw progress text
  char progressText[20];
  sprintf(progressText, "Progress: %d/%d", completed, total);
  drawBoxedString(10, 80, progressText, GREEN, BLACK);
  
  // Draw progress bar
  int barWidth = 100;
  int progress = (barWidth * completed) / total;
  
  Paint_DrawRectangle(10, 105, 10 + barWidth, 120, WHITE, 1, 0);
  if (progress > 0) {
    Paint_DrawRectangle(10, 105, 10 + progress, 120, GREEN, 1, 1);
  }
}

void finishRehabMode() {
  clearDialog();
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
  
  // Wait for user to continue
  while (readAdaForce(FORCE1) < 500) {
    delay(50);
  }
  delay(500); // Debounce
  
  // Reset and return to main menu
  rehabExercise = 0;
  switchToMenu();
}
