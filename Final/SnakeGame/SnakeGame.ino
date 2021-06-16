
#include "LedControl.h"
#include <Joystick.h>
#include <AxisJoystick.h>

/*
  Pin 11 is connected to DataIn
  Pin 13 is connected to CLK
  Pin 10 is connected to LOAD
*/

#define SW_PIN A2   // Joystick select button
#define VRX_PIN A1  // Joystick x-axis
#define VRY_PIN A0  // Joystick y-axis

// Create joystick
Joystick* joystick;

// The starting length of the snake
#define START_LENGTH 3

const int numDevices = 1;     // Number of MAX7219s used
const long scrollDelay = 80;  // Scrolling speed of text
unsigned long bufferLong [14] = {0};

// LedControl(DATA, CLK, LOAD, number of matrices)
LedControl lc = LedControl(11, 13, 10, numDevices);                                   

// Scrolled texts
const unsigned char scrollText[] PROGMEM = {"  G A M E  O V E R  "};
const unsigned char scrollTextStart[] PROGMEM = {"  P R E S S  S T A R T  "};

// Start velocities by direction
int x_direction = 1;
int y_direction = 0;

// Snake array with shape (64,2), 64 for 8x8 dots, 2 for x and y values
int snake[64][2];
int snake_length = 1;

// Apple array for x and y values
int apple[2];

// Boolean to check if game is ended
bool game_over = false;


void setup() {

  lc.shutdown(0, false);    // Open matrix
  lc.setIntensity(0, 15);   // Set brightness of the matrix
  lc.clearDisplay(0);       // Clear the display of the matrix

  joystick = new AxisJoystick(SW_PIN, VRX_PIN, VRY_PIN);

  // Initialize the pseudo-random number generator
  randomSeed(analogRead(0));
  snake[0][0] = random(0, 8);
  snake[0][1] = random(0, 8);

  // Extend the length of the snake to starting length
  for (int i = snake_length; i < START_LENGTH; i++) {
    extendSnake();
  }
  
  createApple();  // Create a new apple

  for (int x = 0; x < numDevices; x++) {
    lc.shutdown(x, false);      
    lc.setIntensity(x, 8);      
    lc.clearDisplay(x);        
  }
}


void loop() {
  bool game_start = false; // Boolean to check if game is started 

  // Display "Press Play" text until player presses on joystick's button
  scrollMessage(scrollTextStart);

  // The player pressed on the start button
  game_start = true;

  
  while (game_start) {
    
    if (isSnakeDead()) {
      // The snake is dead, "Game Over" text is displayed
      lc.clearDisplay(0);
      scrollMessage(scrollText);
      game_start = false;

      // Turn the snake values to default
      snake_length = 1;
      snake[0][0] = random(0, 8);
      snake[0][1] = random(0, 8);
      
      for (int i = snake_length; i < START_LENGTH; i++) {
        extendSnake();
      }
      
      createApple();
      lc.clearDisplay(0);
      
      for (int x = 0; x < numDevices; x++)
      {
        lc.shutdown(x, false);      //The MAX72XX is in power-saving mode on startup
        lc.setIntensity(x, 8);      // Set the brightness to default value
        lc.clearDisplay(x);         // and clear the display
      }
      return;
    }

    // The game is started
    checkDirection(joystick->singleRead());

    // Move each point of snake on matrix
    for (int i = 0; i < snake_length - 1; i++) {
      snake[i][0] = snake[i + 1][0];
      snake[i][1] = snake[i + 1][1];
    }
    snake[snake_length - 1][0] = (snake[snake_length - 2][0] + x_direction + 8) % 8;
    snake[snake_length - 1][1] = (snake[snake_length - 2][1] + y_direction + 8) % 8;

    // Check if apple is eaten and grow the length of the snake
    if (snake[snake_length - 1][0] == apple[0] && snake[snake_length - 1][1] == apple[1]) {
      extendSnake();
      createApple();
    }
    
    draw();
    delay(150);
  }
}

// Return true if the snake is dead
bool isSnakeDead() {
  for (int i = 0; i < snake_length - 2; i++) {
    // If any point on the snake is come across with itself return true 
    if (snake[i][0] == snake[snake_length - 1][0] && snake[i][1] == snake[snake_length - 1][1]) {
      return true;
    }
  }
  return false;
}

// Generate an apple on a random place
void createApple() {
  int x_coor = random(0, 8);
  int y_coor = random(0, 8);
  if (snake[0][0] != x_coor && snake[0][1] != y_coor) {
    apple[0] = x_coor;
    apple[1] = y_coor;
  } else {
    createApple();
  }
}


// Extend the length of the snake
void extendSnake() {
  snake[snake_length][0] = (snake[snake_length - 1][0] + x_direction + 8) % 8;
  snake[snake_length][1] = (snake[snake_length - 1][1] + y_direction + 8) % 8;
  snake_length++;
}


// Draw snake and apple on matrix
void draw() {
  lc.clearDisplay(0);  // Clear the display of the matrix
  for (int i = 0; i < snake_length ; i++) {
    lc.setLed(0, snake[i][0], snake[i][1], HIGH);  // Set the each point of the snake on matrix
  }
  lc.setLed(0, apple[0], apple[1], HIGH); // Set the generated apple on matrix with its x and y value 
}


void checkDirection(const Joystick::Move move) {

  if (move == Joystick::Move::UP && x_direction != -1) {
    x_direction = 1;
    y_direction = 0;
  } else if (move == Joystick::Move::LEFT && y_direction != 1) {
    x_direction = 0;
    y_direction = -1;
  } else if (move == Joystick::Move::RIGHT && y_direction != -1) {
    x_direction = 0;
    y_direction = 1;
  } else if (move == Joystick::Move::DOWN && x_direction != 1) {
    x_direction = -1;
    y_direction = 0;
  }
}


void scrollFont() {
  for (int counter = 0x20; counter < 0x80; counter++) {
    loadBufferLong(counter);
    delay(500);
  }
}


// Scroll text
void scrollMessage(const unsigned char * messageString) {
  int counter = 0;
  int myChar = 0;
  do {
    // Read back a char
    myChar =  pgm_read_byte_near(messageString + counter);
    if (myChar != 0) {
      loadBufferLong(myChar);
    }
    counter++;
    if(joystick->singleRead() == Joystick::Move::PRESS) return;
  }
  while (myChar != 0);
}


//Numeric Font Matrix 
const unsigned char font5x7 [] PROGMEM = {      
  B00000000,  //Space Char
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  3,

  B01000000,  //!
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B00000000,
  B01000000,
  2,

  B10100000,  //"
  B10100000,
  B10100000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  4,

  B01010000,  //#
  B01010000,
  B11111000,
  B01010000,
  B11111000,
  B01010000,
  B01010000,
  6,

  B00100000,  //$
  B01111000,
  B10100000,
  B01110000,
  B00101000,
  B11110000,
  B00100000,
  6,

  B11000000,  //%
  B11001000,
  B00010000,
  B00100000,
  B01000000,
  B10011000,
  B00011000,
  6,

  B01100000,  //&
  B10010000,
  B10100000,
  B01000000,
  B10101000,
  B10010000,
  B01101000,
  6,

  B11000000,  //'
  B01000000,
  B10000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  3,

  B00100000,  //(
  B01000000,
  B10000000,
  B10000000,
  B10000000,
  B01000000,
  B00100000,
  4,

  B10000000,  //)
  B01000000,
  B00100000,
  B00100000,
  B00100000,
  B01000000,
  B10000000,
  4,

  B00000000,  //*
  B00100000,
  B10101000,
  B01110000,
  B10101000,
  B00100000,
  B00000000,
  6,

  B00000000,  //+
  B00100000,
  B00100000,
  B11111000,
  B00100000,
  B00100000,
  B00000000,
  6,

  B00000000,  //,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01000000,
  B10000000,
  3,

  B00000000,  //-
  B00000000,
  B11111000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  6,

  B00000000,  //.
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B11000000,
  3,

  B00000000,  ///
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B10000000,
  B00000000,
  6,

  B01110000,  //0
  B10001000,
  B10011000,
  B10101000,
  B11001000,
  B10001000,
  B01110000,
  6,

  B01000000,  //1
  B11000000,
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B01110000,  //2
  B10001000,
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B11111000,
  6,

  B11111000,  //3
  B00010000,
  B00100000,
  B00010000,
  B00001000,
  B10001000,
  B01110000,
  6,

  B00010000,  //4
  B00110000,
  B01010000,
  B10010000,
  B11111000,
  B00010000,
  B00010000,
  6,

  B11111000,  //5
  B10000000,
  B11110000,
  B00001000,

  B00001000,
  B10001000,
  B01110000,
  6,

  B00110000,  //6
  B01000000,
  B10000000,
  B11110000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B11111000,  //7
  B10001000,
  B00001000,
  B00010000,
  B00100000,
  B00100000,
  B00100000,
  6,

  B01110000,  //8
  B10001000,
  B10001000,
  B01110000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B01110000,  //9
  B10001000,
  B10001000,
  B01111000,
  B00001000,
  B00010000,
  B01100000,
  6,

  B00000000,  //:
  B11000000,
  B11000000,
  B00000000,
  B11000000,
  B11000000,
  B00000000,
  3,

  B00000000,  //;
  B11000000,
  B11000000,
  B00000000,
  B11000000,
  B01000000,
  B10000000,
  3,

  B00010000,  //<
  B00100000,
  B01000000,
  B10000000,
  B01000000,
  B00100000,
  B00010000,
  5,

  B00000000,  //=
  B00000000,
  B11111000,
  B00000000,
  B11111000,
  B00000000,
  B00000000,
  6,

  B10000000,  //>
  B01000000,
  B00100000,
  B00010000,
  B00100000,
  B01000000,
  B10000000,
  5,

  B01110000,  //?
  B10001000,
  B00001000,
  B00010000,
  B00100000,
  B00000000,
  B00100000,
  6,

  B01110000,  //@
  B10001000,
  B00001000,
  B01101000,
  B10101000,
  B10101000,
  B01110000,
  6,

  B01110000,  //A
  B10001000,
  B10001000,
  B10001000,
  B11111000,
  B10001000,
  B10001000,
  6,

  B11110000,  //B
  B10001000,
  B10001000,
  B11110000,
  B10001000,
  B10001000,
  B11110000,
  6,

  B01110000,  //C
  B10001000,
  B10000000,
  B10000000,
  B10000000,
  B10001000,
  B01110000,
  6,

  B11100000,  //D
  B10010000,
  B10001000,
  B10001000,
  B10001000,
  B10010000,
  B11100000,
  6,

  B11111000,  //E
  B10000000,
  B10000000,
  B11110000,
  B10000000,
  B10000000,
  B11111000,
  6,

  B11111000,  //F
  B10000000,
  B10000000,
  B11110000,
  B10000000,
  B10000000,
  B10000000,
  6,

  B01110000,  //G
  B10001000,
  B10000000,
  B10111000,
  B10001000,
  B10001000,
  B01111000,
  6,

  B10001000,  //H
  B10001000,
  B10001000,
  B11111000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B11100000,  //I
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B00111000,  //J
  B00010000,
  B00010000,
  B00010000,
  B00010000,
  B10010000,
  B01100000,
  6,

  B10001000,  //K
  B10010000,
  B10100000,
  B11000000,
  B10100000,
  B10010000,
  B10001000,
  6,

  B10000000,  //L
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B11111000,
  6,

  B10001000,  //M
  B11011000,
  B10101000,
  B10101000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B10001000,  //N
  B10001000,
  B11001000,
  B10101000,
  B10011000,
  B10001000,
  B10001000,
  6,

  B01110000,  //O
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B11110000,  //P
  B10001000,
  B10001000,
  B11110000,
  B10000000,
  B10000000,
  B10000000,
  6,

  B01110000,  //Q
  B10001000,
  B10001000,
  B10001000,
  B10101000,
  B10010000,
  B01101000,
  6,

  B11110000,  //R
  B10001000,
  B10001000,
  B11110000,
  B10100000,
  B10010000,
  B10001000,
  6,

  B01111000,  //S
  B10000000,
  B10000000,
  B01110000,
  B00001000,
  B00001000,
  B11110000,
  6,

  B11111000,  //T
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  6,

  B10001000,  //U
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B10001000,

  B01110000,
  6,

  B10001000,  //V
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B01010000,
  B00100000,
  6,

  B10001000,  //W
  B10001000,
  B10001000,
  B10101000,
  B10101000,
  B10101000,
  B01010000,
  6,

  B10001000,  //X
  B10001000,
  B01010000,
  B00100000,
  B01010000,
  B10001000,
  B10001000,
  6,

  B10001000,  //Y
  B10001000,
  B10001000,
  B01010000,
  B00100000,
  B00100000,
  B00100000,
  6,

  B11111000,  //Z
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B10000000,
  B11111000,
  6,

  B11100000,  //[
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B11100000,
  4,

  B00000000,  //(Backward Slash)
  B10000000,
  B01000000,
  B00100000,
  B00010000,
  B00001000,
  B00000000,
  6,

  B11100000,  //]
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B11100000,
  4,

  B00100000,  //^
  B01010000,
  B10001000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  6,

  B00000000,  //_
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11111000,
  6,

  B10000000,  //`
  B01000000,
  B00100000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  4,

  B00000000,  //a
  B00000000,
  B01110000,
  B00001000,
  B01111000,
  B10001000,
  B01111000,
  6,

  B10000000,  //b
  B10000000,
  B10110000,
  B11001000,
  B10001000,
  B10001000,
  B11110000,
  6,

  B00000000,  //c
  B00000000,
  B01110000,
  B10001000,
  B10000000,
  B10001000,
  B01110000,
  6,

  B00001000,  //d
  B00001000,
  B01101000,
  B10011000,
  B10001000,
  B10001000,
  B01111000,
  6,

  B00000000,  //e
  B00000000,
  B01110000,
  B10001000,
  B11111000,
  B10000000,
  B01110000,
  6,

  B00110000,  //f
  B01001000,
  B01000000,
  B11100000,
  B01000000,
  B01000000,
  B01000000,
  6,

  B00000000,  //g
  B01111000,
  B10001000,
  B10001000,
  B01111000,
  B00001000,
  B01110000,
  6,

  B10000000,  //h
  B10000000,
  B10110000,
  B11001000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B01000000,  //i
  B00000000,
  B11000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B00010000,  //j
  B00000000,
  B00110000,
  B00010000,
  B00010000,
  B10010000,
  B01100000,
  5,

  B10000000,  //k
  B10000000,
  B10010000,
  B10100000,
  B11000000,
  B10100000,
  B10010000,
  5,

  B11000000,  //l
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B00000000,  //m
  B00000000,
  B11010000,
  B10101000,
  B10101000,
  B10001000,
  B10001000,
  6,

  B00000000,  //n
  B00000000,
  B10110000,
  B11001000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B00000000,  //o
  B00000000,
  B01110000,
  B10001000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B00000000,  //p
  B00000000,
  B11110000,
  B10001000,
  B11110000,
  B10000000,
  B10000000,
  6,

  B00000000,  //q
  B00000000,
  B01101000,
  B10011000,
  B01111000,
  B00001000,
  B00001000,
  6,

  B00000000,  //r
  B00000000,
  B10110000,
  B11001000,
  B10000000,
  B10000000,
  B10000000,
  6,

  B00000000,  //s
  B00000000,
  B01110000,
  B10000000,
  B01110000,
  B00001000,
  B11110000,
  6,

  B01000000,  //t
  B01000000,
  B11100000,
  B01000000,
  B01000000,
  B01001000,
  B00110000,
  6,

  B00000000,  //u
  B00000000,
  B10001000,
  B10001000,
  B10001000,
  B10011000,
  B01101000,
  6,

  B00000000,  //v
  B00000000,
  B10001000,
  B10001000,
  B10001000,
  B01010000,
  B00100000,
  6,

  B00000000,  //w
  B00000000,
  B10001000,
  B10101000,
  B10101000,
  B10101000,
  B01010000,
  6,

  B00000000,  //x
  B00000000,
  B10001000,
  B01010000,
  B00100000,
  B01010000,
  B10001000,
  6,

  B00000000,  //y
  B00000000,
  B10001000,
  B10001000,
  B01111000,
  B00001000,
  B01110000,
  6,

  B00000000,  //z
  B00000000,
  B11111000,
  B00010000,
  B00100000,
  B01000000,
  B11111000,
  6,

  B00100000,  //{
  B01000000,
  B01000000,
  B10000000,
  B01000000,
  B01000000,
  B00100000,
  4,

  B10000000,  //|
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  2,

  B10000000,  //}
  B01000000,
  B01000000,
  B00100000,
  B01000000,
  B01000000,
  B10000000,
  4,

  B00000000,  //~
  B00000000,
  B00000000,
  B01101000,
  B10010000,
  B00000000,
  B00000000,
  6,

  B01100000,  // (Space char)
  B10010000,
  B10010000,
  B01100000,
  B00000000,
  B00000000,
  B00000000,
  5,
};

// Load character into scroll buffer
void loadBufferLong(int ascii) {
  if (ascii >= 0x20 && ascii <= 0x7f) {
    for (int a = 0; a < 7; a++) {               // Loop 7 times for a 5x7 font
      unsigned long c = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + a);     // Index into character table to get row data
      unsigned long x = bufferLong [a * 2];   // Load current scroll buffer
      x = x | c;                              // OR the new character onto end of current
      bufferLong [a * 2] = x;                 // Store in buffer
    }
    byte count = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + 7);    // Index into character table for kerning data
    for (byte x = 0; x < count; x++) {
      rotateBufferLong();
      printBufferLong();
      delay(scrollDelay);
    }
  }
}


// Rotate the buffer
void rotateBufferLong() {
  for (int a = 0; a < 7; a++) {               // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a * 2];   // Get low buffer entry
    byte b = bitRead(x, 31);                // Copy high order bit that gets lost in rotation
    x = x << 1;                             // Rotate left one bit
    bufferLong [a * 2] = x;                 // Store new low buffer
    x = bufferLong [a * 2 + 1];             // Get high buffer entry
    x = x << 1;                             // Rotate left one bit
    bitWrite(x, 0, b);                      // Store saved bit
    bufferLong [a * 2 + 1] = x;             // Store new high buffer
  }
}


// Display Buffer on LED matrix
void printBufferLong() {
  for (int a = 0; a < 7; a++) {             // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a * 2 + 1]; // Get high buffer entry
    byte y = x;                             // Mask off first character
    lc.setRow(3, a, y);                     // Send row to relevent MAX7219 chip
    x = bufferLong [a * 2];                 // Get low buffer entry
    y = (x >> 24);                          // Mask off second character
    lc.setRow(2, a, y);                     // Send row to relevent MAX7219 chip
    y = (x >> 16);                          // Mask off third character
    lc.setRow(1, a, y);                     // Send row to relevent MAX7219 chip
    y = (x >> 8);                           // Mask off forth character
    lc.setRow(0, a, y);                     // Send row to relevent MAX7219 chip
  }
}
