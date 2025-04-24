/*
  Arduino Touch TFT Breakout

  Classic breakout game

  Parts needed:
      Ardunio UNO
      AZ-Delivery 2.4 TFT LCD Touch Display Arduino Shield or compatible

  This example code is in the public domain.

  Modified 07 11 2020
  By Enrique Albertos


*/
/*PIN OUT FOR LCD AND JOYSTICK
LCD:
  BL 9
  RST 8
  DC 7
  CS 10
  CLK 13 
  DIN 11
  GND GND
  VCC 5V

JOYSTICK:
  GND GND
  +5V 3.3V
  VRX A0
  VRY A1
*/


// #define DEMO_MODE



#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"

#include <EEPROM.h>

#define joystick_x A1
#define joystick_y A0

#define buttonA 2
#define buttonB 3

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

//defines 'levels'
game_type games[GAMES_NUMBER] =
  // ballsize, playerwidth, playerheight, exponent, top, rows, columns, brickGap, lives, wall[8],                                         initVelx, initVely
{
  { 5,             40,          8,           6,        0 ,     8,       8, 3,       3,  {0x18, 0x66, 0xFF, 0xDB, 0xFF, 0x7E, 0x24, 0x3C} , 100, -100},
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


};
game_state_type state;



//////////////////////////////////////////////////////////////
// ARDUINO SETUP
//////////////////////////////////////////////////////////////

void setup()
{
  initLCD();
  gameSize = {0, 0, 320, 170};
  //newGame(&games[0], &state);
  Serial.begin(9600);

  EEPROM.update(0,128);
  EEPROM.update(1,128);
  EEPROM.update(2,128);
  EEPROM.update(3,128);
  EEPROM.update(4,128);
  EEPROM.update(5,128);
  EEPROM.update(32,128);
  EEPROM.update(33,128);
  EEPROM.update(34,128);
}
//////////////////////////////////////////////////////////////
// ARDUINO LOOP
//////////////////////////////////////////////////////////////
//variables used for reporting to user
//these update when a new max value is reached while playing the game
int userFlex1, userFlex2, userFlex3, userFlex4, userFlexAvg, userForceAvg, userLvl, userHiScore, userID = 0;
//variables used for keeping track of things in code
//these update every loop
int flex1, flex2, flex3, flex4, flexAvg, force1, force2, force3, force4, force5, force6, forceAvg =0;
//state variable for keeping track of device function


int selection = -1;
int HHState = 0;

void switchToGame(int level){
  HHState = 1;
  newGame(&games[level],&state);
}

void switchToReview(){
  HHState =2;
  clearDialog();
}
void loop(void)
{
switch((int)HHState){
  case 0: {//startup
    drawBoxedString(10,10,"HealingHand",BLUE,WHITE);
    drawBoxedString(10,30,"press buttonA to start",BLUE,WHITE);
    //while (waitForButtonA()<0){}
    clearDialog();
    drawBoxedString(10,10,"Select User",BLUE,WHITE);
    displayUsers();
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
    char buff[3];
    char userStr[15] = "USER ";
    strcat(userStr,itoa(userID,buff,10));
    strcat(userStr," STATS");
    drawBoxedString(90,10,userStr,BLUE,WHITE);

    drawBoxedString(10,35,"Flex(deg)|1 |2 |3 |4  ",YELLOW, RED);
//display gets messed up when all values are 3 digits
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
    drawBoxedString(10,60, flexStr,YELLOW,RED);

    char forceString[10] = "Force: ";
    strcat(forceString,itoa(userForceAvg,buff,10));
    strcat(forceString,"N");
    drawBoxedString(10,85,forceString,GREEN,MAGENTA);

    char lvlString[15] = "Hi Score: ";
    strcat(lvlString,itoa(userHiScore,buff,10)); 
    drawBoxedString(10,110,lvlString,BLUE,WHITE);

    char lvlString2[20] = "Cont. from level ";
    strcat(lvlString2,itoa(userLvl,buff,10));
    drawBoxedString(10,135,lvlString2,BLUE,WHITE);

    while (waitForButtonA()<0){}
    clearDialog();
    
    switchToGame(0);
  }
  break;
  case 1: {
    selection = readUiSelection(game, &state, selection);

    drawPlayer(game, &state);
    // store old position to remove old pixels
    state.playerxold = state.playerx;

    // calculate new ball position x1 = x0 + vx * dt

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
    state.velx = (20 + (state.score >> 3 )) * ( (state.velx > 0) - (state.velx < 0));
    state.vely = (20 + (state.score >> 3 )) * ( (state.vely > 0) - (state.vely < 0));

    // if no bricks go to next level
    if (noBricks(game, &state) && level < GAMES_NUMBER) {
      level++;
      newGame( &games[level], &state);
    } else if ( state.remainingLives <= 0) {
      switchToReview();
      state.score = 0;
    }
  }
  break;
  case 2 :{
    drawBoxedString(10,10,"HealingHand",BLUE,WHITE);
    drawBoxedString(10,30,"Good Work!",BLUE,WHITE);
    drawBoxedString(10,50,"Push ButtonA for Stats",BLUE,WHITE);
    while (waitForButtonA()<0){}
    clearDialog();
    drawBoxedString(90,10,"USER STATS",BLUE,WHITE);
    drawBoxedString(10,35,"Flex(deg)|1 |2 |3 |4  ",YELLOW, RED);
    char flexStr[20] = "avg:" ;
    char buff[3];
    strcat(flexStr,itoa(userFlexAvg,buff,10));
    strcat(flexStr," |");
    strcat(flexStr,itoa(userFlex1,buff,10));
    strcat(flexStr,"|");
    strcat(flexStr,itoa(userFlex2,buff,10));
    strcat(flexStr,"|");
    strcat(flexStr,itoa(userFlex3,buff,10));
    strcat(flexStr,"|");
    strcat(flexStr,itoa(userFlex4,buff,10));
    drawBoxedString(10,60, flexStr,YELLOW,RED);
    char forceString[10] = "Force: ";
    strcat(forceString,itoa(userForceAvg,buff,10));
    strcat(forceString,"N");
    drawBoxedString(10,85,forceString,GREEN,MAGENTA);
    char lvlString[10] = "Level: ";
    strcat(lvlString,itoa(userLvl,buff,10)); 
    drawBoxedString(10,110,lvlString,BLUE,WHITE);
    char lvlString2[20] = "Retry from level ";
    strcat(lvlString2,itoa(userLvl,buff,10));
    drawBoxedString(10,135,lvlString2,BLUE,WHITE);
    
    //needs to be switched to a contineu playing or shut down option 
    while (waitForButtonA()<0){}
    clearDialog();
    switchToGame(0);

  }
  break;
}
  
}

void displayUsers(){
  //display new user option
  drawBoxedString(10,65,"New User", MAGENTA,WHITE);
  int addr = 0;
  int i =1;
  while (EEPROM.read(addr) != 255){
    //display the user as it exists
    char userStr[10] = "USER ";
    char buff[10];
    strcat(userStr,itoa(i,buff,10));
    drawBoxedString(170, addr+10,userStr ,MAGENTA,WHITE);
    addr+=32;
    i++;
  }
}

void loadUserData(int UID){
  int addr = (UID-1)*32;
  userFlex1 = EEPROM.read(addr);
  userFlex2 = EEPROM.read(addr+1);
  userFlex3 = EEPROM.read(addr+2);
  userFlex4 = EEPROM.read(addr+3);
  userFlexAvg = EEPROM.read(addr+4);
  userForceAvg = EEPROM.read(addr+5);
  userLvl = EEPROM.read(addr+6);
  userHiScore = EEPROM.read(addr+7);
}

void newUser(){
  int UID = 0;
  while (EEPROM.read(UID)!=255){
    UID = UID+32;
  }
  UID = UID/32;
  userID = UID+1;
}

void storeUserData(int UID){
  int addr = (UID-1)*32;
  EEPROM.update(addr,userFlex1);
  EEPROM.update(addr+1,userFlex2);
  EEPROM.update(addr+2,userFlex3);
  EEPROM.update(addr+3,userFlex4);
  EEPROM.update(addr+4,userFlexAvg);
  EEPROM.update(addr+5,userForceAvg);
  EEPROM.update(addr+6,userLvl);
  EEPROM.update(addr+7,userHiScore);
}

int getSelection(){
  if((digitalRead(buttonA) == HIGH) && (digitalRead(buttonB) == HIGH)){
    return 0;
  }
  if ( digitalRead(buttonA) == HIGH){
    return 1;
  }
  if (digitalRead(buttonB) == HIGH){
    return 2;
  }
  return -1;
}

int waitForButtonA(){
  if (digitalRead(buttonA) == LOW){
    return -1;
  }
  return 1;
}

void newGame(game_type* newGame, game_state_type * state) {
  game = newGame;
  setupState(game, state);

  clearDialog();
  updateLives(game->lives, state->remainingLives);
  updateScore(state->score);
  setupWall(game, state);

}

void setupStateSizes(game_type* game, game_state_type * state) {
  state->bottom = 170 - 30;
  state->brickwidth = 320 / game->columns;
  state->brickheight = 170 / 24;
}

void setupState(game_type* game, game_state_type * state) {
  setupStateSizes(game, state);
  for (int i = 0; i < game->rows ; i ++) {
    state->wallState[i] = 0;
  }
  state->playerx = 320 / 2 - game->playerwidth / 2;
  state->remainingLives = game->lives;
  state->bally = state->bottom << game->exponent;
  state->ballyold = state->bottom << game->exponent;
  state->velx = game->initVelx;
  state->vely = game->initVely;
}

void updateLives(int lives, int remainingLives) {
  for (int i = 0; i < lives; i++) {
    Paint_DrawCircle((1+i)*15,15,5,BLACK,5,1);
  }
  for (int i = 0; i < remainingLives; i++) {
    Paint_DrawCircle((1+i)*15,15,5,YELLOW,5,1);
  }
}

void setupWall(game_type * game, game_state_type * state) {
  int colors[] = {RED, RED, BLUE, BLUE,  YELLOW, YELLOW, GREEN, GREEN};
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

void drawBrick(game_state_type * state, int xBrick, int yBrickRow, uint16_t backgroundColor) {
  int brickxStart = (state->brickwidth * xBrick) + game->brickGap;
  int brickyStart = state->walltop + (state->brickheight * yBrickRow) + game->brickGap;
  //add *2 to brickGap to increase game speed (looks lamer)
  int brickWidth = state->brickwidth - game->brickGap ;
  int brickHeight = state->brickheight -  game->brickGap;
  
  Paint_DrawRectangle(
        brickxStart,
        brickyStart,
        brickxStart + brickWidth,   //xend
        brickyStart + brickHeight,  //yend
        backgroundColor , 1, 1);   
}


boolean noBricks(game_type * game, game_state_type * state) {
  for (int i = 0; i < game->rows ; i++) {
    if (state->wallState[i]) return false;
  }
  return true;
}

void drawPlayer(game_type * game, game_state_type * state) {
  // paint
  Paint_DrawRectangle(state->playerx, state->bottom, (state->playerx + game->playerwidth), (state->bottom + 1), YELLOW, 1, 0);
  //tft.fillRect(state->playerx, state->bottom, game->playerwidth, game->playerheight, YELLOW);

  if (state->playerx != state->playerxold) {
    // remove old pixels
    if (state->playerx > state->playerxold) {
      Paint_DrawRectangle(state->playerxold, state->bottom, state->playerxold + 2, (state->bottom + 1), BLACK, 1, 0);
      //tft.fillRect(state->playerx + game->playerwidth, state->bottom, abs(state->playerx - state->playerxold), game->playerheight, backgroundColor);
    }
    else {
      Paint_DrawRectangle(state->playerxold + game->playerwidth - 2, state->bottom, state->playerxold + game->playerwidth + 5, (state->bottom + 1), BLACK, 1, 0);
      //tft.fillRect(state->playerxold, state->bottom, abs(state->playerx - state->playerxold), game->playerheight, backgroundColor);
    }

  }
}

void drawBall(int x, int y, int xold, int yold, int ballsize) {
 Paint_DrawRectangle(xold, yold, xold + ballsize, yold + ballsize, BLACK, 1, 0);
  //draw new ball
  Paint_DrawRectangle(x, y, x + ballsize, y + ballsize, CYAN, 1, 1);

}

void updateScore (int score) {
  char buffer[5];
  snprintf(buffer, sizeof(buffer), scoreFormat, score);
  drawBoxedString(320 - 70, 6, buffer, YELLOW, PRIMARY_DARK_COLOR);
}

void checkBrickCollision(game_type* game, game_state_type * state, uint16_t x, uint16_t y) {
  int x1 = x + game->ballsize;
  int y1 = y + game->ballsize;
  int collissions = 0;
  collissions += checkCornerCollision(game, state, x, y);
  collissions += checkCornerCollision(game, state, x1, y1);
  collissions += checkCornerCollision(game, state, x, y1);
  collissions += checkCornerCollision(game, state, x1, y);
  if (collissions > 0 ) {
    state->vely = (-1 * state->vely);
    if ((((x % state->brickwidth) == 0)  && ( state->velx < 0 ))
        || ((((x + game->ballsize) % state->brickwidth) == 0)  && ( state->velx > 0 )) ) {
      state->velx = (-1 * state->velx);
    }
  }


}
int checkCornerCollision(game_type * game,  game_state_type * state, uint16_t x, uint16_t y) {
  if ((y > state->walltop) && (y < state->wallbottom)) {
    int yBrickRow = ( y -  state->walltop) / state->brickheight;
    int xBrickColumn = (x / state->brickwidth);
    if (isBrickIn(state->wallState, xBrickColumn, yBrickRow) ) {
      hitBrick(state, xBrickColumn, yBrickRow);
      return 1;
    }
  }
  return 0;
}
void hitBrick(game_state_type * state, int xBrick, int yBrickRow) {
  state->score += pointsForRow[yBrickRow];
  drawBrick(state, xBrick, yBrickRow, WHITE);
  delay(16);
  drawBrick(state, xBrick, yBrickRow, BLUE);
  delay(8);
  drawBrick(state, xBrick, yBrickRow, backgroundColor);
  unsetBrick(state->wallState, xBrick, yBrickRow);
  updateScore(state->score);
}

void checkBorderCollision(game_type * game,  game_state_type * state, uint16_t x, uint16_t y) {
  // check wall collision
  if (x + game->ballsize >=  320) {
    state->velx = -abs(state->velx);
  }
  if (x <= 0  ) {
    state->velx = abs(state->velx);
  }
  if (y <= SCORE_SIZE ) {
    state->vely = abs(state->vely);
  }
  if (((y + game->ballsize)  >=  state->bottom)
      && ((y + game->ballsize) <= (state->bottom + game->playerheight))
      && (x >= state->playerx)
      && (x <= (state->playerx + game->playerwidth))) {
    // change vel x near player borders
    if (x > (state->playerx + game->playerwidth - 6)) {
      state->velx = state->velx - 1;
    } else if (x < state->playerx + 6) {
      state->velx = state->velx + 1;
    }
    state->vely = -abs(state->vely) ;
  }
}

void checkBallCollisions(game_type * game, game_state_type * state, uint16_t x, uint16_t y) {
  checkBrickCollision(game, state, x, y);
  checkBorderCollision(game, state, x, y);
}

void checkBallExit(game_type * game, game_state_type * state, uint16_t x, uint16_t y) {
  if (((y + game->ballsize)  >=  170)) {
    state->remainingLives--;
    updateLives(game->lives, state->remainingLives);
    delay(500);
    state->vely = -abs(state->vely) ;
  }
}

void setBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] | BIT_MASK[x];
}

void unsetBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] & ~BIT_MASK[x];
}

boolean isBrickIn(int wall[], uint8_t x, uint8_t y) {
  return wall[y] &  BIT_MASK[x];
}

void initLCD() {
  Config_Init();
  LCD_Init();

  LCD_SetBacklight(100);
  Paint_SetRotate(ROTATE_270);
  Paint_NewImage(170, 320, 270, BLACK);
  Paint_Clear(BLACK);
}


void drawBoxedString(const uint16_t x, const uint16_t y, const char* string, const uint16_t foreColor, const uint16_t backgroundColor) {
  Paint_DrawString_EN(x,y, string, &Font20,foreColor, backgroundColor);
}

void clearDialog() {
  LCD_SetBacklight(100);
  Paint_SetRotate(ROTATE_270);
  Paint_NewImage(170, 320, 270, BLACK);
  Paint_Clear(BLACK);
}

int readUiSelection(game_type * game, game_state_type * state, const int16_t lastSelected ) {
  
  int joyx = analogRead(joystick_x);
  int joyy = analogRead(joystick_y);

  if (joyx > 600) {
    state->playerx = state->playerxold + 2;
  } else if (joyx < 250) {
    state->playerx = state->playerxold - 2;
  } 
  if (state->playerx >= 320 - game->playerwidth) state->playerx = 320 - game->playerwidth;
  if (state->playerx < 0) state->playerx = 0;
  return 1;

}

//SENSOR FUNCTIONS
int readFlexSensor(int pin1, int pin2){
  int readA = analogRead(pin1);
  int readB = analogRead(pin2);
  int reading = readB - readA;
  return reading;
}

int flex2Deg(int reading){
  //does some math to make 0-1023 analog reading degrees
  
  
  return 0;
}

int readAdaForce(int pin){
  int reading = analogRead(pin);
  return reading;
}

int readSingleTact(){
//should be as simple as copy/paste

  return reading;
}

