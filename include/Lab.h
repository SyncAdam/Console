#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdlib>
#include "GinaESP.h"
#include <Adafruit_MPU6050.h>
#include "macros.h"

namespace Lab
{
    const int wall_length = 28;
    const int wall_thickness = 6;
    typedef struct wall
    {
        int orientation;
        int16_t x, y;
        int16_t color;
    } wall;

    enum wallOrientation
    {
        VERTICAL,
        HORIZONTAL
    };

    typedef struct ball
    {
        int16_t x, y;
        int16_t color;
        float acc_x = 0;
        float acc_y = 0;
        float speed_x = 0;
        float speed_y = 0;
    } ball;
    
    void getInput(bool* button1, bool* button2, int* joyx, int* joyy, bool* joy_button, float* ax, float* ay);
    void initialize(Adafruit_MPU6050* fmpu);
    int playLab(Adafruit_MPU6050* fmpu, TFT_eSPI &TFTscreen);
    int gameLoop(TFT_eSPI &TFTscreen);
    void checkCollisions(ball gameball, wall Walls[], int nWalls);
    void drawObjects(TFT_eSPI &TFTscreen, ball gameball, wall Walls[], int nWalls);

    const int max_speed = 7;
    const int max_acceleration = 3;
    const float acceleration_sensitivity = 0.2;

    bool button1, button2;
    int joyx, joyy;
    bool joy_button;
    int ballx, bally;
    
    Adafruit_MPU6050 mpu;

    void initialize(Adafruit_MPU6050* fmpu)
    {
        Lab::mpu = *fmpu;
    }

    void getInput(bool* button1, bool* button2, int* joyx, int* joyy, bool* joy_button, float* ax, float* ay)
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

    void drawObjects(TFT_eSPI &TFTscreen, ball gameball, wall Walls[], int nWalls)
    {
        GinaESP::fillCircle(TFTscreen, gameball.x, gameball.y, 8, gameball.color);
        for(int i = 0; i < nWalls; i++)
        {
            if(Walls[i].orientation == VERTICAL)
            {
                GinaESP::fillRect(TFTscreen, Walls[i].x, Walls[i].y, Walls[i].x - wall_thickness, Walls[i].y + wall_length, Walls[i].color);
            }
            else if(Walls[i].orientation == HORIZONTAL)
            {
                GinaESP::fillRect(TFTscreen, Walls[i].x, Walls[i].y, Walls[i].x + wall_length, Walls[i].y + wall_thickness, Walls[i].color);
            }
        }
    }

    void checkCollisions(ball gameball, wall Walls[], int nWalls)
    {
        
    }

    int gameLoop(TFT_eSPI &TFTscreen)
    {
        float ax, ay;
        getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ax, &ay);
        
        ball firstBall;
        firstBall.x = WIDTH / 2;
        firstBall.y = HEIGHT / 2;
        firstBall.color = TFT_BLUE;

        wall firstEverWall;
        firstEverWall.orientation = VERTICAL;
        firstEverWall.x = (WIDTH + wall_length - wall_thickness) / 2;
        firstEverWall.y = (HEIGHT - wall_length + wall_thickness) / 2;
        firstEverWall.color = TFT_DARKGREY;

        for(int i = 0; i < WIDTH * HEIGHT; i++)
        {
            GinaESP::buffer[i] = TFT_BLACK;
        }
        GinaESP::drawScreen(TFTscreen);

        Serial.println("Starting gameloop....");

        while(!button1)
        {
            GinaESP::clearBuffer();
            getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ay, &ax);

            ax *= acceleration_sensitivity;
            ay *= acceleration_sensitivity;

            if(ax > max_acceleration) ax = max_acceleration;
            if(ax < -max_acceleration) ax = -max_acceleration;
            if(ay > max_acceleration) ay = max_acceleration;
            if(ay < -max_acceleration) ay = -max_acceleration;

            if(firstBall.speed_x <= max_speed && firstBall.speed_x >= -max_speed)
            {
                firstBall.speed_x += ax;
            }
            else if(firstBall.speed_x > max_speed)
            {
                firstBall.speed_x = max_speed;
            }
            else if(firstBall.speed_x < -max_speed)
            {
                firstBall.speed_x = -max_speed;
            }
            if(firstBall.speed_y <= max_speed && firstBall.speed_y >= -max_speed)
            {
                firstBall.speed_y += ay;
            }
            else if(firstBall.speed_y > max_speed)
            {
                firstBall.speed_y = max_speed;
            }
            else if(firstBall.speed_y < -max_speed)
            {
                firstBall.speed_y = -max_speed;
            }

            int16_t dx = firstBall.x + firstBall.speed_x;
            int16_t dy = firstBall.y + firstBall.speed_y;

            if(dx < WIDTH && dx > 0)
            {
                firstBall.x = dx;
            }
            else
            {
                firstBall.speed_x = 0;
            }
            if(dy < HEIGHT && dy > 0)
            {
                firstBall.y = dy;
            }
            else
            {
                firstBall.speed_y = 0;
            }
            drawObjects(TFTscreen, firstBall, &firstEverWall, 1);
            GinaESP::drawScreen(TFTscreen);
        }

        return 0;
    }

}