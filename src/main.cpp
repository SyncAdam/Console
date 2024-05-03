#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdlib>
#include "src\GinaESP.h"
#include "MPU6050.h"
//pins are defined in the user setup header file

//bruuuuh
#include <SPI.h>

#define JOYX 0
#define JOYY 2
#define JOYBTN 4

#define MPU_SDA 21
#define MPU_SCL 22

#define WIDTH 240
#define HEIGHT 320

TFT_eSPI TFTscreen = TFT_eSPI(240, 320);
u_int8_t buffer [WIDTH * HEIGHT];
MPU6050_Base mpu;

void setup() {
  //initialize the screen
  TFTscreen.init();
  // set the background color to black
  TFTscreen.fillScreen(TFT_BLACK);

  mpu.initialize();
  
  TFTscreen.setTextSize(2);

  pinMode(JOYX, INPUT);
  pinMode(JOYY, INPUT);
  pinMode(JOYBTN, INPUT);
}

void screenSaver()
{
  long readTime = millis();
  const int readInterval = 150;
  long updateDelayTime = millis();

  int delayTime = 0;

  const uint8_t nBalls = 10;
  const uint8_t ballRadius = 30;
  const uint8_t ballSpeed = 5;

  //same code but with nBalls amount of balls
  uint16_t x[nBalls];
  uint16_t y[nBalls];
  bool goingLeft[nBalls];
  bool goingUp[nBalls];
  uint16_t color[nBalls];

  for(int i = 0; i < nBalls; i++)
  {
    x[i] =  10 + rand() % 220;
    y[i] =  10 + rand() % 300;
    goingLeft[i] = rand() % 2;
    goingUp[i] = rand() % 2;
  }
  //fill color array with random colors
  for(int i = 0; i < nBalls; i++)
  {
    color[i] = colors[rand() % 24];
  }

    long time = millis();
    while (true) {

      for(int i = 0; i < WIDTH * HEIGHT; i++)
      {
        buffer[i] = TFT_BLACK;
      }

      drawFPS(TFTscreen, buffer, time);
      time = millis();

      //update the position of the balls
      for(int i = 0; i < nBalls; i++)
      {
        if (x[i] <= ballRadius) 
        {
          goingLeft[i] = false;
        } 
        else if (x[i] >= 240 - ballRadius)
        {
          goingLeft[i] = true;
        }

        if (y[i] <= ballRadius)
        {
          goingUp[i] = false;
        }
        else if (y[i] >= 320 - ballRadius)
        {
          goingUp[i] = true;
        }      

        if (goingLeft[i])
        {
          x[i] -= ballSpeed;
        }
        else
        {
          x[i] += ballSpeed;
        }

        if (goingUp[i])
        {
          y[i] -= ballSpeed;
        }
        else
        {
          y[i] += ballSpeed;
        }

        //draw the balls
        fillCircle(TFTscreen, buffer, x[i], y[i], ballRadius, color[i]);
      }
      drawScreen(TFTscreen, buffer);
    }

}

void testPeriphs()
{

}

void loop() {
  screenSaver();
}
