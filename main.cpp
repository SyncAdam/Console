#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdlib>
#include <Adafruit_MPU6050.h>

#include "GinaESP.h"
#include "Lab.h"
#include "macros.h"
#include "Agar.h" 
//pins are defined in the user setup header file

#include <SPI.h>
#include <Wire.h>

typedef struct menutext{
  String text;
  int x;
  int y;
  int size;
  uint16_t color;
} MenuText;

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

void printMenuElements(MenuText t[])
{
  for(int i = 2; i < 4; i++)
  {
    TFTscreen.setTextColor(t[i].color);
    TFTscreen.setCursor(t[i].x, t[i].y);
    TFTscreen.setTextSize(t[i].size);
    TFTscreen.print(t[i].text);
  }
}

void createMenuElements(MenuText t[])
{
  /*
  t[0].x = 93;
  t[0].y = 140;
  t[0].color = ILI9341_BLUE;
  t[0].size = 2;
  t[0].text = "Snake";

  t[1].x = 100;
  t[1].y = 170;
  t[1].color = ILI9341_BLUE;
  t[1].size = 2;
  t[1].text = "Dino";
  */

  t[2].x = 60;
  t[2].y = 100;
  t[2].color = ILI9341_BLUE;
  t[2].size = 2;
  t[2].text = "Labyrinthe";

  t[3].x = 100;
  t[3].y = 130;
  t[3].color = ILI9341_BLUE;
  t[3].size = 2;
  t[3].text = "Agar";
}

void drawMenuRect(MenuText t)
{
  TFTscreen.drawRect(t.x - 4, t.y -4, t.text.length() * 10 + (t.text.length()-1)*2 + 8, 22, ILI9341_WHITE);
}

void undrawMenuRect(MenuText t)
{
  TFTscreen.drawRect(t.x - 4, t.y -4, t.text.length() * 10 + (t.text.length()-1)*2 + 8, 22, ILI9341_BLACK);
}

//main menu method
int mainMenu()
{
  int selected = 0;  //selector index
  int hoverindex = 2;
  int dhoverindex = hoverindex;

  //Erase screen
  TFTscreen.fillScreen(ILI9341_BLACK);

  //Create and store menu elements
  MenuText Texts[4];
  createMenuElements(Texts);

  //Print "Choose your game!" on screen
  TFTscreen.setCursor(20,60);
  TFTscreen.setTextColor(ILI9341_WHITE);
  TFTscreen.setTextSize(2);
  TFTscreen.print("Choose your game!");
  
  //Print all the MenuText elements
  //Note: the rectangle is drawn on the MenuText element which is currently chosen 
  printMenuElements(Texts);
  drawMenuRect(Texts[hoverindex]);

  //Waiting until user selects a game
  //Until then if a button is pressed, the selection rectangle is moved in function of which button is pressed 
  while(!selected){
    dhoverindex = hoverindex;
    int joyy = analogRead(JOYY);
    bool button2 = digitalRead(BUTTON2);
    if(joyy < 100)
    {
      hoverindex++;
      if(hoverindex == 4) hoverindex = 2;
      undrawMenuRect(Texts[dhoverindex]);
      drawMenuRect(Texts[hoverindex]);
      delay(250);
    } 
    else if(joyy > 4000){
      hoverindex--;
      if(hoverindex < 2) hoverindex = 3;
      undrawMenuRect(Texts[dhoverindex]);
      drawMenuRect(Texts[hoverindex]);
      delay(150);
    }
    if(button2){
      delay(200);     //added a bit of delay for accidental touch protection
      button2 = digitalRead(BUTTON2);
      if(button2) return hoverindex;
    }
  }
}

int agarSubMenu() {
  int selected = 0;
  int hoverindex = 0;
  int dhoverindex = hoverindex;
  
  TFTscreen.fillScreen(ILI9341_BLACK);

  MenuText Texts[2];
  Texts[0].x = 60;
  Texts[0].y = 100;
  Texts[0].color = ILI9341_BLUE;
  Texts[0].size = 2;
  Texts[0].text = "- ennemies";

  Texts[1].x = 60;
  Texts[1].y = 130;
  Texts[1].color = ILI9341_BLUE;
  Texts[1].size = 2;
  Texts[1].text = "+ ennemies";

  TFTscreen.setCursor(20, 60);
  TFTscreen.setTextColor(ILI9341_WHITE);
  TFTscreen.setTextSize(2);
  TFTscreen.print("Choose your mode!");

  TFTscreen.setTextColor(Texts[0].color);
  TFTscreen.setCursor(Texts[0].x, Texts[0].y);
  TFTscreen.setTextSize(Texts[0].size);
  TFTscreen.print(Texts[0].text);

  TFTscreen.setTextColor(Texts[1].color);
  TFTscreen.setCursor(Texts[1].x, Texts[1].y);
  TFTscreen.setTextSize(Texts[1].size);
  TFTscreen.print(Texts[1].text);

  TFTscreen.drawRect(Texts[hoverindex].x - 4, Texts[hoverindex].y - 4, Texts[hoverindex].text.length() * 10 + (Texts[hoverindex].text.length() - 1) * 2 + 8, 22, ILI9341_WHITE);

  while (!selected) {
    dhoverindex = hoverindex;
    int joyy = analogRead(JOYY);
    bool button2 = digitalRead(BUTTON2);
    if (joyy < 100) {
      hoverindex++;
      if (hoverindex == 2) hoverindex = 0;
      TFTscreen.drawRect(Texts[dhoverindex].x - 4, Texts[dhoverindex].y - 4, Texts[dhoverindex].text.length() * 10 + (Texts[dhoverindex].text.length() - 1) * 2 + 8, 22, ILI9341_BLACK);
      TFTscreen.drawRect(Texts[hoverindex].x - 4, Texts[hoverindex].y - 4, Texts[hoverindex].text.length() * 10 + (Texts[hoverindex].text.length() - 1) * 2 + 8, 22, ILI9341_WHITE);
      delay(250);
    } else if (joyy > 4000) {
      hoverindex--;
      if (hoverindex < 0) hoverindex = 1;
      TFTscreen.drawRect(Texts[dhoverindex].x - 4, Texts[dhoverindex].y - 4, Texts[dhoverindex].text.length() * 10 + (Texts[dhoverindex].text.length() - 1) * 2 + 8, 22, ILI9341_BLACK);
      TFTscreen.drawRect(Texts[hoverindex].x - 4, Texts[hoverindex].y - 4, Texts[hoverindex].text.length() * 10 + (Texts[hoverindex].text.length() - 1) * 2 + 8, 22, ILI9341_WHITE);
      delay(150);
    }
    if (button2) {
      delay(200);
      button2 = digitalRead(BUTTON2);
      if (button2) return hoverindex;
    }
  }
}

void loop() {
  int choice = mainMenu();
  switch(choice) {
    case 2:
      Lab::playLab(&mpu, TFTscreen);
      break;
    case 3:
      int agarChoice = agarSubMenu();
      if (agarChoice == 0) {
        playAgar(TFTscreen);
      } else if (agarChoice == 1) {
        playAgarWithEnemies(TFTscreen);
      }
      break;
  }
}
