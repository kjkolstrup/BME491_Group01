#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"
#include "image.h"

#define SCORE_SIZE 30
#define joystick_x A1
#define joystick_y A0




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

//defines game constants, important values -k
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

//defines 'levels' -k
game_type games[GAMES_NUMBER] =
  // ballsize, playerwidth, playerheight, exponent, top, rows, columns, brickGap, lives, wall[8],                                         initVelx, initVely
{
  { 10,             60,          8,           6,        40 ,     8,       8, 3,       3,  {0x18, 0x66, 0xFF, 0xDB, 0xFF, 0x7E, 0x24, 0x3C} , 28, -28},
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
//defines state of the game, game variables -k
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

void drawBricks() { //testing: if funciton works and game struct info
  int brickxStart =0;
  int brickyStart = 0;
  int brickGap = game->brickGap;
  int brickHeight = 8;
  int brickWidth = 20;
    for (int i = 0; i < game->rows; i++) {
      for (int j = 0; j < game->columns; j++) {
        brickxStart = j* (brickWidth+brickGap)+5;
        brickyStart = i* (brickHeight+brickGap)+5;
        Paint_DrawRectangle(
          brickxStart,              
          brickyStart,
          brickxStart + brickWidth, //xend
          brickyStart+ brickHeight,  //yend
          RED, 1 , 1); //color, line width, fill type
    }
  }
}

void drawPlayer(game_type * game, game_state_type * state) { //draws player and removes old pixels

  Paint_DrawRectangle(state->playerx, state->bottom, (state->playerx + game->playerwidth), (state->bottom + game ->playerheight), YELLOW, 2, 0);
  if (state->playerx != state->playerxold) {
    // remove old pixels
      Paint_DrawRectangle(state->playerxold , state->bottom, state->playerxold+2, (state->bottom + game ->playerheight), BLACK, 2, 0);
      Paint_DrawRectangle(state->playerxold + game->playerwidth-2 , state->bottom, state->playerxold+game->playerwidth, (state->bottom + game ->playerheight), BLACK, 2, 0);

  }
}

void setup()
{
    Config_Init();
    LCD_Init();

    LCD_SetBacklight(100);
    Paint_SetRotate(ROTATE_270);
    Paint_NewImage(170,320,270,BLACK);
    Paint_Clear(BLACK);

    gameSize = {0, 0, 170, 320};
    game = &games[0];
    drawBricks();
    state.bottom = 150;

    
}


void loop()
{
//GOAL : draw bricks and move paddle with joystick DONE! 3/10
//GOAL : make a ball bounce off bricks and paddle
    //drawBricks function, test from the func def
    int joyx = analogRead(joystick_x);
    int joyy = analogRead(joystick_y);
      state.playerxold = state.playerx;
      state.playerxold = state.playerx;

    if ( joyx > 600){
      state.playerx = state.playerxold+5;
    } else if(joyx < 250 ){
      state.playerx = state.playerxold-5;
    }



    drawPlayer(game,&state);
    delay(100);
  }

        





/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
