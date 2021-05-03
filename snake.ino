
#include "LedControl.h"

/*
wiring:
 pin 12 is connected to DataIn
 pin 11 is connected to CLK
 pin 10 is connected to LOAD
buttons are connected to the following pins
*/

#define LEFT  2
#define RIGHT 3
#define UP    4
#define DOWN  5

//define the starting snake_len of the snake (must be < 8)
#define START_LEN 3

//setup our LedControl instance
LedControl lc = LedControl(12,11,10,1);

//setup our start velocities
int vx = 1;
int vy = 0;

//define the snake array with shape (64,2)
//i.e. a max of 64 parts and 2 (x,y) values per part
int snake[64][2];
//we can only call extend_snake when the snake already
//has one block, so we define it with one
int snake_len = 1;
//an x,y pair of the apple location
int apple[2];
//boolean - is the game over?
bool gameover = false;

void setup(){
    //setup the chip (wake it, set brighntess, clear it)
    lc.shutdown(0,false);
    lc.setIntensity(0,15);
    lc.clearDisplay(0);
    pinMode(RIGHT, INPUT_PULLUP);
    pinMode(DOWN,  INPUT_PULLUP);
    pinMode(UP,    INPUT_PULLUP);
    pinMode(LEFT,  INPUT_PULLUP);
    randomSeed(analogRead(0));
    snake[0][0] = random(0,8);
    snake[0][1] = random(0,8);
    for (int i = snake_len; i < START_LEN; i++){
        extend_snake();
    }
    new_apple();
}

void loop(){
    if (am_i_dead()) return;
    check_buttons();
    move_snake();
    check_eaten_apple();
    draw();
    delay(150);
}

bool am_i_dead(){
    for (int i = 0; i < snake_len-2; i++){
        if (snake[i][0] == snake[snake_len-1][0] && snake[i][1] == snake[snake_len-1][1]){
            return true;
        }
    }
    return false;
}

void check_eaten_apple(){
    if (snake[snake_len-1][0] == apple[0] && snake[snake_len-1][1] == apple[1]){    //eaten an apple
        extend_snake();
        new_apple();
    }
}

void new_apple() {
     apple[0] = random(0,8);
     apple[1] = random(0,8);
}

void move_snake(){
    for (int i = 0; i < snake_len-1; i++){
        snake[i][0] = snake[i+1][0];
        snake[i][1] = snake[i+1][1];
    }
    snake[snake_len-1][0] = (snake[snake_len - 2][0] + vx + 8) % 8;
    snake[snake_len-1][1] = (snake[snake_len - 2][1] + vy + 8) % 8;
}

void extend_snake(){
    snake[snake_len][0] = (snake[snake_len - 1][0] + vx + 8) % 8;
    snake[snake_len][1] = (snake[snake_len - 1][1] + vy + 8) % 8;
    snake_len++;
}

void draw(){
    lc.clearDisplay(0);
    for (int i = 0; i < snake_len ; i++){
        lc.setLed(0, snake[i][0], snake[i][1], HIGH);
    }
    lc.setLed(0, apple[0], apple[1], HIGH);
}

void check_buttons(){
    if (!digitalRead(RIGHT) && vx != -1){
        vx = 1;
        vy = 0;
    } else if (!digitalRead(DOWN) && vy != 1){
        vx = 0;
        vy = -1;
    } else if (!digitalRead(UP) && vy != -1){
        vx = 0;
        vy = 1;
    } else if (!digitalRead(LEFT) && vx != 1){
        vx = -1;
        vy = 0;
    }
}
