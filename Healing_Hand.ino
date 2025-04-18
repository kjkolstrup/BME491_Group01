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

const uint8_t BIT_MASK[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
uint8_t pointsForRow[] = { 7, 7, 5, 5, 3, 3, 1, 1 };
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
    { 5, 60, 8, 6, 40, 8, 8, 3, 3, { 0x18, 0x66, 0xFF, 0xDB, 0xFF, 0x7E, 0x24, 0x3C }, 28, -28 },
    { 10, 50, 8, 6, 40, 8, 8, 3, 3, { 0xFF, 0x99, 0xFF, 0xE7, 0xBD, 0xDB, 0xE7, 0xFF }, 28, -28 },
    { 10, 50, 8, 6, 40, 8, 8, 3, 3, { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 }, 28, -28 },
    { 8, 50, 8, 6, 40, 8, 8, 3, 3, { 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF }, 34, -34 },
    { 10, 40, 8, 6, 40, 8, 8, 3, 3, { 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF }, 28, -28 },
    { 10, 40, 8, 6, 40, 8, 8, 3, 3, { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA }, 28, -28 },
    { 12, 64, 8, 6, 60, 4, 2, 3, 4, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 20, -20 },
    { 12, 60, 8, 6, 60, 5, 3, 3, 4, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 22, -22 },
    { 10, 56, 8, 6, 30, 6, 4, 3, 4, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 24, -24 },
    { 10, 52, 8, 6, 30, 7, 5, 3, 4, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 26, -26 },
    { 8, 48, 8, 6, 30, 8, 6, 3, 3, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 28, -28 },
    { 8, 44, 8, 6, 30, 8, 7, 3, 3, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 30, -30 },
    { 8, 40, 8, 6, 30, 8, 8, 3, 3, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 32, -32 },
    { 8, 36, 8, 6, 40, 8, 8, 3, 3, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 34, -34 },
    { 8, 36, 8, 6, 40, 8, 8, 3, 3, { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA }, 34, -34 }
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
  int wallbottom;
  int brickheight;
  int brickwidth;
};
game_state_type state;

//8x8 grid of bricks
bool bricks[8][8] = { 1 };


void drawBricks() {  //testing: if funciton works and game struct info
  int brickxStart = 0;
  int brickyStart = 0;
  int brickGap = game->brickGap;
  int brickHeight = 8;
  int brickWidth = (320 / game->columns);
  for (int i = 0; i < game->rows; i++) {
    for (int j = 0; j < game->columns; j++) {
      brickxStart = j * (brickWidth + brickGap) + 5;
      brickyStart = i * (brickHeight + brickGap) + 5;
      Paint_DrawRectangle(
        brickxStart,
        brickyStart,
        brickxStart + brickWidth,   //xend
        brickyStart + brickHeight,  //yend
        RED, 1, 1);                 //color, line width, fill type
    }
  }
}

void drawPlayer(game_type* game, game_state_type* state) {  //draws player and removes old pixels

  Paint_DrawRectangle(state->playerx, state->bottom, (state->playerx + game->playerwidth), (state->bottom + 1), YELLOW, 1, 0);
  if (state->playerx != state->playerxold) {
    // remove old pixels
    if (state->playerxold < state->playerx) {
      Paint_DrawRectangle(state->playerxold, state->bottom, state->playerxold + 8, (state->bottom + 1), BLACK, 1, 0);
    } else if (state->playerxold > state->playerx) {
      Paint_DrawRectangle(state->playerxold + game->playerwidth - 2, state->bottom, state->playerxold + game->playerwidth + 5, (state->bottom + 1), BLACK, 1, 0);
    }
  }
}

void drawBall(int x, int y, int xold, int yold, int ballsize) {
  //erase old ball
  Paint_DrawRectangle(xold, yold, xold + ballsize, yold + ballsize, BLACK, 1, 0);
  //draw new ball
  Paint_DrawRectangle(x, y, x + ballsize, y + ballsize, BLUE, 1, 0);
}

void checkBallCollisions() {
  checkBrickCollisions();
  //checkWallColliisions();
}
void checkBrickCollisions() {
  //pos of ball
  int balltop = state.bally - game->ballsize;
  int ballleft = state.ballx;
  int ballright = state.bally + game->ballsize;

  int brickbottom = (game->brickGap + 8) * game->rows;
  //is the ball within the brick space?
  if (balltop <= brickbottom) {
    //yes, is the ball touching a brick? DOES NOT WORK
    //checking top left corner of ball
    int row = map(ballleft,0,88,0,7);
    int col = map(balltop,0,320,0,7);



    Serial.print(row,DEC);
    Serial.print(col,DEC);

    if (bricks[row][col] == 1) {
      state.vely = -1 * state.vely;
      eraseBrick(row, col);
    }
  }
  if (balltop + 8 >= state.bottom && ((state.ballx > state.playerx) && (state.ballx < state.playerx + game->playerwidth))) {
    state.velx = -1 * state.velx;
    state.vely = -1 * state.vely;
  }
}

void eraseBrick(int row, int col) {
  bricks[row][col] = 0;
  int brickxStart = col * (20 + game->brickGap) + 5;
  int brickyStart = (row - 1) * (8 + game->brickGap) + 5;

  Paint_DrawRectangle(
    brickxStart,
    brickyStart,
    brickxStart + 20,  //xend
    brickyStart + 8,   //yend
    YELLOW, 1, 0);
}

void setup() {
  Serial.begin(9600);

  Config_Init();
  LCD_Init();

  LCD_SetBacklight(100);
  Paint_SetRotate(ROTATE_270);
  Paint_NewImage(170, 320, 270, BLACK);
  Paint_Clear(BLACK);

  gameSize = { 0, 0, 170, 320 };
  game = &games[0];
  drawBricks();
  state.bottom = 150;
  state.ballx = 40;
  state.bally = 140;
  state.velx = 5;
  state.vely = 5;
}


void loop() {
  //GOAL : draw bricks and move paddle with joystick DONE! 3/10
  //GOAL : make a ball bounce off bricks and paddle
  //drawBricks function, test from the func def
  int joyx = analogRead(joystick_x);
  int joyy = analogRead(joystick_y);
  state.playerxold = state.playerx;
  state.playerxold = state.playerx;
  //need to check if the player will move over the edge
  if (joyx > 600) {
    state.playerx = state.playerxold + 8;
  } else if (joyx < 250) {
    state.playerx = state.playerxold - 8;
  }

  //check boundaries
  if (state.playerx < 0) {
    state.playerx = 0;
  } else if (state.playerx > 320 - game->playerwidth) {
    state.playerx = 320 - game->playerwidth;
  }

  state.ballxold = state.ballx;
  state.ballyold = state.bally;

  state.ballx = state.ballxold + state.velx;
  state.bally = state.ballyold - state.vely;

  if (state.ballx < 0) {
    state.ballx = 10;
    state.velx = -1 * state.velx;
  } else if (state.ballx > 320 - game->ballsize) {
    state.ballx = 320 - game->ballsize;
    state.velx = -1 * state.velx;
  }

  drawPlayer(game, &state);
  checkBallCollisions();
  drawBall(state.ballx, state.bally, state.ballxold, state.ballyold, game->ballsize);
  delay(100);
}







/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
