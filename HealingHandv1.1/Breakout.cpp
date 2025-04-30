#include "Breakout.h"

// Score format for display
char scoreFormat[] = "%04d";

Breakout::Breakout() {
  currentLevel = 0;
  gameSize = {0, 0, 320, 170};
  setupGameLevels();
}

void Breakout::setupGameLevels() {
  // Define all game levels
  // Parameters: ballsize, playerwidth, playerheight, exponent, top, rows, columns, brickGap, lives, wall[8], initVelx, initVely
  
  games[0] = { 5, 40, 8, 6, 0, 8, 8, 3, 3, {0x18, 0x66, 0xFF, 0xDB, 0xFF, 0x7E, 0x24, 0x3C}, 100, -100 };
  games[1] = { 10, 50, 8, 6, 40, 8, 8, 3, 3, {0xFF, 0x99, 0xFF, 0xE7, 0xBD, 0xDB, 0xE7, 0xFF}, 28, -28 };
  games[2] = { 10, 50, 8, 6, 40, 8, 8, 3, 3, {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55}, 28, -28 };
  games[3] = { 8, 50, 8, 6, 40, 8, 8, 3, 3, {0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF}, 34, -34 };
  games[4] = { 10, 40, 8, 6, 40, 8, 8, 3, 3, {0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF}, 28, -28 };
  games[5] = { 10, 40, 8, 6, 40, 8, 8, 3, 3, {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 28, -28 };
  games[6] = { 12, 64, 8, 6, 60, 4, 2, 3, 4, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 20, -20 };
  games[7] = { 12, 60, 8, 6, 60, 5, 3, 3, 4, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 22, -22 };
  games[8] = { 10, 56, 8, 6, 30, 6, 4, 3, 4, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 24, -24 };
  games[9] = { 10, 52, 8, 6, 30, 7, 5, 3, 4, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 26, -26 };
  games[10] = { 8, 48, 8, 6, 30, 8, 6, 3, 3, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 28, -28 };
  games[11] = { 8, 44, 8, 6, 30, 8, 7, 3, 3, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 30, -30 };
  games[12] = { 8, 40, 8, 6, 30, 8, 8, 3, 3, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 32, -32 };
  games[13] = { 8, 36, 8, 6, 40, 8, 8, 3, 3, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 34, -34 };
  games[14] = { 8, 36, 8, 6, 40, 8, 8, 3, 3, {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 34, -34 };
  games[15] = { 8, 36, 8, 6, 40, 8, 8, 3, 3, {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}, 34, -34 };
}

void Breakout::displayControlScreen() {
  clearScreen();
  drawBoxedString(10, 10, "BLOCK BREAKOUT", LIGHTBLUE, WHITE);
  drawBoxedString(10, 35, "Move the paddle by ", GREEN, BLACK);
  drawBoxedString(10, 55, "bending your fingers", GREEN, BLACK);
  drawBoxedString(10, 80, "Break blocks to lvl up", YELLOW, BLACK);
  drawBoxedString(10, 105, "Activate powerups by pressing thumb", MAGENTA, BLACK);
  drawBoxedString(10, 150, "Press thumb to cont.", BLUE, WHITE);
  // The main code will handle waiting for the thumb press was having issue in the cpp 
}

void Breakout::initializeGame(int level) {
  // Set current level
  currentLevel = level;
  if (currentLevel >= GAMES_NUMBER) {
    currentLevel = GAMES_NUMBER - 1;
  }
  if (currentLevel < 0) {
    currentLevel = 0;
  }
  
  // Set current game to the selected level
  currentGame = &games[currentLevel];
  
  // Setup game state for this level
  setupState();
  
  // Clear screen and draw initial game elements
  clearScreen();
  updateLives();
  updateScore();
  setupWall();
}

void Breakout::setupState() {
  // Initialize game state
  setupStateSizes();
  
  // Reset wall state
  for (int i = 0; i < currentGame->rows; i++) {
    state.wallState[i] = 0;
  }
  
  // Set initial player position
  state.playerx = 320 / 2 - currentGame->playerwidth / 2;
  state.playerxold = state.playerx;
  
  // Set initial ball position and velocity
  state.bally = state.bottom << currentGame->exponent;
  state.ballyold = state.bottom << currentGame->exponent;
  state.ballx = (state.playerx + (currentGame->playerwidth / 2)) << currentGame->exponent;
  state.ballxold = state.ballx;
  
  // Set initial velocity
  state.velx = currentGame->initVelx;
  state.vely = currentGame->initVely;
  
  // Set remaining lives
  state.remainingLives = currentGame->lives;
  
  // Reset score (only for a new game, not for level progression)
  if (currentLevel == 0) {
    state.score = 0;
  }
  
  // Initialize power-up related variables
  state.powerUpFlag = 0;
  state.powerUpType = 0;
  state.playerSpeed = 1;
  state.powerUpCounter = 0;
}

void Breakout::setupStateSizes() {
  // Set game boundaries
  state.top = SCORE_SIZE;
  state.bottom = 170 - 30;
  
  // Set brick dimensions
  state.brickwidth = 320 / currentGame->columns;
  state.brickheight = 170 / 24;
}

void Breakout::setupWall() {
  // Set wall boundaries
  state.walltop = currentGame->top + 40;
  state.wallbottom = state.walltop + currentGame->rows * state.brickheight;
  
  // Color array for rows
  int colors[] = {RED, RED, BLUE, BLUE, YELLOW, YELLOW, GREEN, GREEN};
  
  // Setup wall state and draw bricks
  for (int i = 0; i < currentGame->rows; i++) {
    for (int j = 0; j < currentGame->columns; j++) {
      if (isBrickIn(currentGame->wall, j, i)) {
        setBrick(state.wallState, j, i);
        drawBrick(j, i, colors[i]);
      }
    }
  }
}

void Breakout::update(int flex1, int flex2) {
  // Move player based on flex sensors
  if (flex1 < 500) {
    state.playerx = state.playerxold + state.playerSpeed;
  } else if (flex2 < 500) {
    state.playerx = state.playerxold - state.playerSpeed;
  }
  
  // Restrict player to screen boundaries
  if (state.playerx >= 320 - currentGame->playerwidth) {
    state.playerx = 320 - currentGame->playerwidth;
  }
  if (state.playerx < 0) {
    state.playerx = 0;
  }
  
  // Draw player in new position
  drawPlayer();
  
  // Store old position to remove old pixels
  state.playerxold = state.playerx;
  
  // Check max speed
  if (abs(state.vely) > ((1 << currentGame->exponent) - 1)) {
    state.vely = ((1 << currentGame->exponent) - 1) * ((state.vely > 0) - (state.vely < 0));
  }
  if (abs(state.velx) > ((1 << currentGame->exponent) - 1)) {
    state.velx = ((1 << currentGame->exponent) - 1) * ((state.velx > 0) - (state.velx < 0));
  }
  
  // Update ball position
  state.ballx += state.velx;
  state.bally += state.vely;
  
  // Check ball collisions and exit
  checkBrickCollision(state.ballx >> currentGame->exponent, state.bally >> currentGame->exponent);
  checkBorderCollision(state.ballx >> currentGame->exponent, state.bally >> currentGame->exponent);
  checkBallExit(state.ballx >> currentGame->exponent, state.bally >> currentGame->exponent);
  
  // Draw ball in new position
  drawBall();
  
  // Store old position to remove old pixels
  state.ballxold = state.ballx;
  state.ballyold = state.bally;
  
  // Increment velocity (game gets faster as score increases)
  state.velx = (40 + (state.score >> 3)) * ((state.velx > 0) - (state.velx < 0));
  state.vely = (40 + (state.score >> 3)) * ((state.vely > 0) - (state.vely < 0));
}

void Breakout::checkAndActivatePowerUp(bool thumbPressed) {
  if (state.powerUpFlag && thumbPressed) {
    activatePowerUp(state.powerUpType);
    state.powerUpFlag = 0;
    Paint_DrawRectangle(155, 5, 165, 15, BLACK, 1, 1);
  }
}

bool Breakout::noBricks() {
  for (int i = 0; i < currentGame->rows; i++) {
    if (state.wallState[i]) return false;
  }
  return true;
}

bool Breakout::isGameOver() {
  return (state.remainingLives <= 0);
}

bool Breakout::checkLevelComplete() {
  return noBricks();
}

void Breakout::nextLevel() {
  if (currentLevel < GAMES_NUMBER - 1) {
    currentLevel++;
    initializeGame(currentLevel);
  }
}

void Breakout::activatePowerUp(int type) {
  // Handle different power-up types
  switch(type) {
    case 0: {
      // Increase score
      state.score += (currentLevel + 1) * 5;
      updateScore();
      break;
    }
    case 1: {
      // Gain a life
      state.remainingLives += 1;
      currentGame->lives += 1;
      updateLives();
      break;
    }
    case 2: {
      // Increase paddle speed
      state.playerSpeed += 1;
      break;
    }
  }
}

void Breakout::displayPowerUp(int type) {
  drawBoxedString(10, 150, "You got a power-up!", YELLOW, BLACK);
  
  switch (type) {
    case 0: {
      // Score increase power-up
      Paint_DrawRectangle(155, 5, 165, 15, MAGENTA, 1, 1);
      drawBoxedString(10, 150, "Score Increase!      ", MAGENTA, BLACK);
      break;
    }
    case 1: {
      // Extra life power-up
      Paint_DrawRectangle(155, 5, 165, 15, CYAN, 1, 1);
      drawBoxedString(10, 150, "Extra Life!          ", CYAN, BLACK);
      break;
    }
    case 2: {
      // Paddle speed power-up
      Paint_DrawRectangle(155, 5, 165, 15, GREEN, 1, 1);
      drawBoxedString(10, 150, "Paddle Speed Up!     ", GREEN, BLACK);
      break;
    }
  }
  
  delay(1000);
  drawBoxedString(10, 150, "                      ", BLACK, BLACK);
}

void Breakout::drawPlayer() {
  // Draw new player position
  Paint_DrawRectangle(
    state.playerx, 
    state.bottom, 
    (state.playerx + currentGame->playerwidth), 
    (state.bottom + currentGame->playerheight), 
    YELLOW, 1, 0
  );
  
  // Remove old player position
  if (state.playerx != state.playerxold) {
    // Remove old pixels - if moving right, erase left edge
    if (state.playerx > state.playerxold) {
      Paint_DrawRectangle(
        state.playerxold, 
        state.bottom, 
        state.playerxold + state.playerSpeed, 
        (state.bottom + currentGame->playerheight), 
        BLACK, 1, 0
      );
    }
    // If moving left, erase right edge
    else {
      Paint_DrawRectangle(
        state.playerxold + currentGame->playerwidth - state.playerSpeed, 
        state.bottom, 
        state.playerxold + currentGame->playerwidth + 5, 
        (state.bottom + currentGame->playerheight), 
        BLACK, 1, 0
      );
    }
  }
}

void Breakout::drawBall() {
  // Erase old ball
  int x_old = state.ballxold >> currentGame->exponent;
  int y_old = state.ballyold >> currentGame->exponent;
  Paint_DrawRectangle(
    x_old, 
    y_old, 
    x_old + currentGame->ballsize, 
    y_old + currentGame->ballsize, 
    BLACK, 1, 0
  );
  
  // Draw new ball
  int x = state.ballx >> currentGame->exponent;
  int y = state.bally >> currentGame->exponent;
  Paint_DrawRectangle(
    x, 
    y, 
    x + currentGame->ballsize, 
    y + currentGame->ballsize, 
    CYAN, 1, 1
  );
}

void Breakout::drawBrick(int xBrick, int yBrickRow, uint16_t backgroundColor) {
  int brickxStart = (state.brickwidth * xBrick) + currentGame->brickGap;
  int brickyStart = state.walltop + (state.brickheight * yBrickRow) + currentGame->brickGap;
  int brickWidth = state.brickwidth - currentGame->brickGap;
  int brickHeight = state.brickheight - currentGame->brickGap;
  
  Paint_DrawRectangle(
    brickxStart,
    brickyStart,
    brickxStart + brickWidth,
    brickyStart + brickHeight,
    backgroundColor, 1, 1
  );
}

void Breakout::updateScore() {
  char buffer[5];
  snprintf(buffer, sizeof(buffer), scoreFormat, state.score);
  drawBoxedString(320 - 70, 6, buffer, YELLOW, PRIMARY_DARK_COLOR);
}

void Breakout::updateLives() {
  // Draw empty circles for total lives
  for (int i = 0; i < currentGame->lives; i++) {
    Paint_DrawCircle((1+i)*15, 15, 5, BLACK, 5, 1);
  }
  
  // Fill circles for remaining lives
  for (int i = 0; i < state.remainingLives; i++) {
    Paint_DrawCircle((1+i)*15, 15, 5, YELLOW, 5, 1);
  }
}

void Breakout::drawGameOver() {
  drawBoxedString(10, 80, "GAME OVER", YELLOW, BLACK);
  drawBoxedString(10, 110, "Press thumb to return to Main Menu", YELLOW, BLACK);
}

void Breakout::checkBrickCollision(uint16_t x, uint16_t y) {
  int x1 = x + currentGame->ballsize;
  int y1 = y + currentGame->ballsize;
  int collisions = 0;
  
  collisions += checkCornerCollision(x, y);
  collisions += checkCornerCollision(x1, y1);
  collisions += checkCornerCollision(x, y1);
  collisions += checkCornerCollision(x1, y);
  
  if (collisions > 0) {
    // Reverse vertical velocity
    state.vely = -state.vely;
    
    // Check if we need to reverse horizontal velocity
    if ((((x % state.brickwidth) == 0) && (state.velx < 0))
        || ((((x + currentGame->ballsize) % state.brickwidth) == 0) && (state.velx > 0))) {
      state.velx = -state.velx;
    }
  }
}

int Breakout::checkCornerCollision(uint16_t x, uint16_t y) {
  if ((y > state.walltop) && (y < state.wallbottom)) {
    int yBrickRow = (y - state.walltop) / state.brickheight;
    int xBrickColumn = (x / state.brickwidth);
    
    if (isBrickIn(state.wallState, xBrickColumn, yBrickRow)) {
      hitBrick(xBrickColumn, yBrickRow);
      return 1;
    }
  }
  return 0;
}

void Breakout::hitBrick(int xBrick, int yBrickRow) {
  // Add points based on row
  state.score += pointsForRow[yBrickRow];
  
  // Flash brick
  drawBrick(xBrick, yBrickRow, WHITE);
  delay(16);
  drawBrick(xBrick, yBrickRow, BLUE);
  delay(8);
  
  // Remove brick
  drawBrick(xBrick, yBrickRow, BLACK);
  unsetBrick(state.wallState, xBrick, yBrickRow);
  
  // Update score display
  updateScore();
  
  // Check if player should receive power-up
  if(!state.powerUpFlag && state.score > 0) {
    state.powerUpCounter += 1;
    if (state.powerUpCounter > 4) {
      state.powerUpFlag = 1;
      state.powerUpCounter = 0;
      state.powerUpType = random(3); // Random power-up type (0-2)
      displayPowerUp(state.powerUpType);
    }
  }
}

void Breakout::checkBorderCollision(uint16_t x, uint16_t y) {
  // Check right wall collision
  if (x + currentGame->ballsize >= 320) {
    state.velx = -abs(state.velx);
  }
  
  // Check left wall collision
  if (x <= 0) {
    state.velx = abs(state.velx);
  }
  
  // Check top wall/score area collision
  if (y <= SCORE_SIZE) {
    state.vely = abs(state.vely);
  }
  
  // Check paddle collision
  if (((y + currentGame->ballsize) >= state.bottom)
      && ((y + currentGame->ballsize) <= (state.bottom + currentGame->playerheight))
      && (x >= state.playerx)
      && (x <= (state.playerx + currentGame->playerwidth))) {
    
    // Change velocity based on where ball hits paddle
    if (x > (state.playerx + currentGame->playerwidth - 6)) {
      state.velx = state.velx - 1; // Hit right side, add left spin
    } else if (x < state.playerx + 6) {
      state.velx = state.velx + 1; // Hit left side, add right spin
    }
    
    // Reverse vertical direction
    state.vely = -abs(state.vely);
  }
}

void Breakout::checkBallExit(uint16_t x, uint16_t y) {
  // Check if ball went below screen
  if ((y + currentGame->ballsize) >= 170) {
    // Lost a life
    state.remainingLives--;
    updateLives();
    
    // Reset ball position and velocity
    delay(500);
    state.bally = state.bottom << currentGame->exponent;
    state.vely = -abs(state.vely);
    state.ballx = (state.playerx + (currentGame->playerwidth / 2)) << currentGame->exponent;
  }
}

void Breakout::setBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] | BIT_MASK[x];
}

void Breakout::unsetBrick(int wall[], uint8_t x, uint8_t y) {
  wall[y] = wall[y] & ~BIT_MASK[x];
}

bool Breakout::isBrickIn(int wall[], uint8_t x, uint8_t y) {
  return wall[y] & BIT_MASK[x];
}

void Breakout::drawBoxedString(const uint16_t x, const uint16_t y, const char* string, 
                           const uint16_t foreColor, const uint16_t backgroundColor) {
  Paint_DrawString_EN(x, y, string, &Font20, foreColor, backgroundColor);
}

void Breakout::clearScreen() {
  Paint_Clear(BLACK);
}