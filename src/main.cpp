#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdlib>
#include "GinaESP.h"
#include <Adafruit_MPU6050.h>
#include "Lab.h"
#include "macros.h"
//pins are defined in the user setup header file

#include <SPI.h>
#include <Wire.h>

TFT_eSPI TFTscreen = TFT_eSPI(WIDTH, HEIGHT);
Adafruit_MPU6050 mpu;

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

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  else
  {
    Serial.println("MPU6050 successfully initialized");
  }

  mpu.setClock(MPU6050_INTR_8MHz);
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Clock: ");
  Serial.println(mpu.getClock());
  GinaESP::initGraphics();

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
    color[i] = GinaESP::colors[rand() % 24];
  }

    long time = millis();
    while (true) {

      GinaESP::drawFPS(TFTscreen, time);
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
        GinaESP::fillCircle(TFTscreen, x[i], y[i], ballRadius, color[i]);
      }
      GinaESP::drawScreen(TFTscreen);
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

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.print("AccelX:");
  Serial.println(a.acceleration.x);
  Serial.print("AccelY:");
  Serial.println(a.acceleration.y);

  Serial.println("");

  delay(500);
}

void loop() {
  Lab::playLab(&mpu, TFTscreen);
  //testPeriphs();
}
