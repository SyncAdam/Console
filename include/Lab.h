#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdlib>
#include "GinaESP.h"
#include <Adafruit_MPU6050.h>
#include "macros.h"

namespace Lab
{
    
    void getInput(bool* button1, bool* button2, int* joyx, int* joyy, bool* joy_button);
    void initialize(Adafruit_MPU6050* fmpu);
    int playLab(Adafruit_MPU6050* fmpu, TFT_eSPI &TFTscreen);
    int gameLoop(TFT_eSPI &TFTscreen);

    bool button1, button2;
    int joyx, joyy;
    bool joy_button;
    int ballx, bally;
    
    Adafruit_MPU6050 mpu;

    void initialize(Adafruit_MPU6050* fmpu)
    {
        Lab::mpu = *fmpu;
    }

    void getInput(bool* button1, bool* button2, int* joyx, int* joyy, bool* joy_button, int16_t* ax, int16_t* ay)
    {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        *ax = a.acceleration.x;
        *ay = a.acceleration.y;

        *joyx = analogRead(JOYX);
        *joyy = analogRead(JOYY);
        *joy_button = !digitalRead(JOY_BUTTON);

        *button1 = digitalRead(BUTTON1);
        *button2 = digitalRead(BUTTON2);
    }

    int playLab(Adafruit_MPU6050* fmpu, TFT_eSPI &TFTscreen)
    {
        initialize(fmpu);
        return(gameLoop(TFTscreen));
    }

    int gameLoop(TFT_eSPI &TFTscreen)
    {
        int16_t ax, ay;
        getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ax, &ay);
        
        uint16_t x, y;
        uint16_t color;

        x = WIDTH/2;
        y = HEIGHT/2;

        color = GinaESP::colors[rand() % 24];

        for(int i = 0; i < WIDTH * HEIGHT; i++)
        {
            GinaESP::buffer[i] = TFT_BLACK;
        }
        GinaESP::drawScreen(TFTscreen);

        Serial.println("Starting gameloop....");

        while(!button1)
        {
            for(int i = 0; i < WIDTH * HEIGHT; i++)
            {
                GinaESP::buffer[i] = TFT_BLACK;
            }
            getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ax, &ay);
            uint16_t dx = x + ax;
            uint16_t dy = y + ay;
            if(dx < WIDTH && dx > 0)
            {
                x = dx;
            }
            if(dy < HEIGHT && dy > 0)
            {
                y = dy;
            }
            GinaESP::fillCircle(TFTscreen, x, y, 10, color);
            GinaESP::drawScreen(TFTscreen);
        }

        return 0;
    }

}