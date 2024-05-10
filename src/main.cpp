#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdlib>
#include "GinaESP.h"
#include "MPU6050.h"
//pins are defined in the user setup header file

#include <SPI.h>
#include <Wire.h>

#define JOYX 2
#define JOYY 0
#define JOY_BUTTON 4
#define BUTTON1 33
#define BUTTON2 32

#define MPU_SDA 21
#define MPU_SCL 22

#define WIDTH 240
#define HEIGHT 320

TFT_eSPI TFTscreen = TFT_eSPI(WIDTH, HEIGHT);
u_int8_t buffer [WIDTH * HEIGHT];
u_int8_t buffer2 [WIDTH * HEIGHT];
MPU6050_Base mpu;

u_int16_t x_middle = 1945;
u_int16_t y_middle = 2543;

const u_int8_t tolerance = 40;

bool button1, button2;

int joyx, joyy;
bool joy_button;

void setup() {

  Wire.begin();
  //initialize the screen
  TFTscreen.init();
  // set the background color to black
  TFTscreen.fillScreen(TFT_BLACK);

  Serial.begin(9600);

  mpu.initialize();

  Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  TFTscreen.setTextSize(2);

  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  /*
  pinMode(JOYX, INPUT);
  pinMode(JOYY, INPUT);
  pinMode(JOY_BUTTON, INPUT);
  */

  button1 = digitalRead(BUTTON1);
  button2 = digitalRead(BUTTON2);

  joyx = analogRead(JOYX);
  joyy = analogRead(JOYY);
  joy_button = !digitalRead(JOY_BUTTON);
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


  joyx = analogRead(JOYX);
  joyy = analogRead(JOYY);
  joy_button = !digitalRead(JOY_BUTTON);

  Serial.print("button1: ");
  Serial.println(button1);
  Serial.print("button2: ");
  Serial.println(button2);
  Serial.print("joyx: ");
  Serial.println(joyx);
  Serial.print("joyy: ");
  Serial.println(joyy);
  Serial.print("joy_button: ");
  Serial.println(joy_button);
  Serial.println("============");


  /*
    Serial.print("ax: ");
    Serial.println(mpu.getAccelerationX());
    Serial.print("ay: ");
    Serial.println(mpu.getAccelerationY());
    Serial.print("az: ");
    Serial.println(mpu.getAccelerationZ());
    Serial.println("============");
  */


  delay(500);
}

void loop() {
  screenSaver();
}
