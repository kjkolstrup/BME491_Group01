#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <Arduino.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"

// Constants
#define SCORE_SIZE 30
#define GAMES_NUMBER 16

// Color definitions
#define BLACK     0x0000
#define BLUE      0x001F
#define LIGHTBLUE 0x039F
#define DARKBLUE  0x0013
#define RED       0xF800
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF
#define PRIMARY_COLOR        0x4A11
#define PRIMARY_LIGHT_COLOR  0x7A17
#define PRIMARY_DARK_COLOR   0x4016
#define PRIMARY_TEXT_COLOR   0x7FFF

// Game size structure
typedef struct gameSize_type {
  int16_t x, y, width, height;
} gameSize_type;

// Game level structure
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

// Game state structure
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
  int wallbottom;
  int brickheight;
  int brickwidth;
  int powerUpFlag;
  int powerUpType;
  int playerSpeed;
  int powerUpCounter;
} game_state_type;

class Breakout {
  public:
    // Constructor
    Breakout();
    
    // Game initialization
    void initializeGame(int level);
    void setupGameLevels();
    void displayControlScreen();
    
    // Main game loop functions
    void update(int flex1, int flex2);
    bool isGameOver();
    
    // Game mechanics
    void movePlayer(int direction);
    void updateBall();
    void checkCollisions();
    bool checkLevelComplete();
    void nextLevel();
    
    // Power-up functions
    void activatePowerUp(int type);
    void displayPowerUp(int type);
    bool hasPowerUp() { return state.powerUpFlag; }
    void checkAndActivatePowerUp(bool thumbPressed);
    
    // Drawing functions
    void drawGame();
    void drawPlayer();
    void drawBall();
    void drawWall();
    void drawBrick(int xBrick, int yBrickRow, uint16_t backgroundColor);
    void updateScore();
    void updateLives();
    void drawGameOver();
    
    // Getters
    int getScore() { return state.score; }
    int getLevel() { return currentLevel; }
    int getLives() { return state.remainingLives; }
    
  private:
    // Game levels and state
    game_type games[GAMES_NUMBER];
    game_type* currentGame;
    game_state_type state;
    int currentLevel;
    gameSize_type gameSize;
    
    // Bit masks for brick wall
    const uint8_t BIT_MASK[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    uint8_t pointsForRow[8] = {7, 7, 5, 5, 3, 3, 1, 1};
    
    // Game state management
    void setupState();
    void setupStateSizes();
    void setupWall();
    
    // Collision detection
    void checkBrickCollision(uint16_t x, uint16_t y);
    int checkCornerCollision(uint16_t x, uint16_t y);
    void hitBrick(int xBrick, int yBrickRow);
    void checkBorderCollision(uint16_t x, uint16_t y);
    void checkBallExit(uint16_t x, uint16_t y);
    
    // Brick wall utilities
    void setBrick(int wall[], uint8_t x, uint8_t y);
    void unsetBrick(int wall[], uint8_t x, uint8_t y);
    bool isBrickIn(int wall[], uint8_t x, uint8_t y);
    bool noBricks();
    
    // Helper drawing functions
    void drawBoxedString(const uint16_t x, const uint16_t y, const char* string, 
                         const uint16_t foreColor, const uint16_t backgroundColor);
    void clearScreen();
};

#endif
