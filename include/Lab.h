#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdlib>
#include <Adafruit_MPU6050.h>

#include "GinaESP.h"
#include "macros.h"

#define MAP_SIZE_X 7
#define MAP_SIZE_Y 7

#define MAP_POSITION_X 20
#define MAP_POSITION_Y 40

namespace Lab
{
    enum primitive_shapes
    {
        CIRCLE,
        RECTANGLE,
        TRIANGLE
    };

    typedef struct wall
    {
        int shape = RECTANGLE;
        int orientation;
        float bounce_coefficient = 0.6;
        int16_t x, y;
        int16_t color = TFT_WHITE;
    } wall;

    enum wallOrientation
    {
        VERTICAL,
        HORIZONTAL
    };

    typedef struct ball
    {
        int shape = CIRCLE;
        int16_t x, y;
        int16_t color;
        float radius = 8;
        float acc_x = 0;
        float acc_y = 0;
        float speed_x = 0;
        float speed_y = 0;
    } ball;

    enum holeSpot
    {
        TOP,
        LEFT,
        BOTTOM,
        RIGHT,
        MIDDLE
    };

    typedef struct block
    {
        int16_t x, y;
        wall walls[4];
        int hole;
    } block;

    typedef struct map
    {
        wall horizontal_walls[MAP_SIZE_X][MAP_SIZE_Y + 1];
        wall vertical_walls[MAP_SIZE_Y][MAP_SIZE_X + 1];
    } map;

    void getInput(bool* button1, bool* button2, int* joyx, int* joyy, bool* joy_button, float* ax, float* ay);
    void initialize(Adafruit_MPU6050* fmpu);
    int playLab(Adafruit_MPU6050* fmpu, TFT_eSPI &TFTscreen);
    int gameLoop(TFT_eSPI &TFTscreen);
    void checkCollisions(ball* gameball, wall Walls[], int nWalls);
    void drawObjects(TFT_eSPI &TFTscreen, ball gameball, wall Walls[], int nWalls);
    void generateWallsForBlock(block* b);

    bool button1, button2;
    int joyx, joyy;
    bool joy_button;
    int ballx, bally;
    
    Adafruit_MPU6050 mpu;

    const int max_speed = 7;
    const int max_acceleration = 3;
    const float acceleration_sensitivity = 0.2;

    const int wall_length = 28;
    const int wall_thickness = 3;

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

    void drawObjects(TFT_eSPI &TFTscreen, ball gameball, block blocks[], int nblocks)
    {
        GinaESP::fillCircle(TFTscreen, gameball.x, gameball.y, gameball.radius, gameball.color);
        for(int j = 0; j < nblocks; j++)
        {
            for(int i = 0; i < 4; i++)
            {
                if(blocks[j].walls[i].orientation == VERTICAL)
                {
                    GinaESP::fillRect(TFTscreen, blocks[j].walls[i].x, blocks[j].walls[i].y, blocks[j].walls[i].x - wall_thickness, blocks[j].walls[i].y + wall_length, blocks[j].walls[i].color);
                }
                else if(blocks[j].walls[i].orientation == HORIZONTAL)
                {
                    GinaESP::fillRect(TFTscreen, blocks[j].walls[i].x, blocks[j].walls[i].y, blocks[j].walls[i].x + wall_length, blocks[j].walls[i].y + wall_thickness, blocks[j].walls[i].color);
                }
                else
                {
                    continue;
                }
            }
        }
    }

    void checkCollisions(ball* gameball, wall Walls[], int nWalls)
    {   
        const float bounce_coeff = 0.5;

        int16_t dx = gameball->x + gameball->speed_x;
        int16_t dy = gameball->y + gameball->speed_y;
        for(int i = 0; i < nWalls; i++)
        {
            switch(Walls[i].orientation)
            {
                case VERTICAL:
                    if((dx + gameball->radius > Walls[i].x - wall_thickness &&
                        dx - gameball->radius < Walls[i].x) &&
                    (gameball->y + gameball->radius > Walls[i].y &&
                        gameball->y - gameball->radius < Walls[i].y + wall_length))
                    {
                        gameball->speed_x = -gameball->speed_x * Walls[i].bounce_coefficient;
                        return;
                    }
                    if((dy + gameball->radius > Walls[i].y &&
                        dy - gameball->radius < Walls[i].y + wall_length) &&
                    (gameball->x + gameball->radius > Walls[i].x - wall_thickness &&
                        gameball->x - gameball->radius < Walls[i].x))
                    {
                        gameball->speed_y = -gameball->speed_y * Walls[i].bounce_coefficient;
                        return;
                    }
                    break;

                case HORIZONTAL:
                    if((dy + gameball->radius > Walls[i].y - wall_thickness &&
                        dy - gameball->radius < Walls[i].y) &&
                    (gameball->x + gameball->radius > Walls[i].x &&
                        gameball->x - gameball->radius < Walls[i].x + wall_length))
                    {
                        gameball->speed_y = -gameball->speed_y  * Walls[i].bounce_coefficient;
                        return;
                    }
                    if((dx + gameball->radius > Walls[i].x &&
                        dx - gameball->radius < Walls[i].x + wall_length) &&
                    (gameball->y + gameball->radius > Walls[i].y - wall_thickness &&
                        gameball->y - gameball->radius < Walls[i].y))
                    {
                        gameball->speed_x = -gameball->speed_x  * Walls[i].bounce_coefficient;
                        return;
                    }
                    break;
            }
        }
    }

    void deleteWall(wall* w)
    {
        w->x = -100;
        w->y = -100;
        w->orientation = -1;
    }

    block* getBallsBlock(ball gameball, block gamemap[], int nblocks)
    {
        int x = gameball.x - MAP_POSITION_X;
        int y = gameball.y - MAP_POSITION_Y;

        int block_x = x % (wall_thickness + wall_length);
        int block_y = y % (wall_thickness + wall_length);

        return &gamemap[block_y * MAP_SIZE_X + block_x];
    }

    void generateWallsForBlock(block* b, bool walls[])
    {
        if(walls[0])
        {
            b->walls[0].x = b->x + wall_thickness;
            b->walls[0].y = b->y;
            b->walls[0].orientation = HORIZONTAL;
        }
        else
        {
            deleteWall(&(b->walls[0]));
        }

        if(walls[1])
        {
            b->walls[1].x = b->x + wall_thickness;
            b->walls[1].y = b->y + wall_thickness;
            b->walls[1].orientation = VERTICAL;
        }
        else
        {
            deleteWall(&(b->walls[1]));
        }

        if(walls[2])
        {
            b->walls[2].x = b->x + wall_thickness;
            b->walls[2].y = b->y + wall_thickness + wall_length;
            b->walls[2].orientation = HORIZONTAL;
        }
        else
        {
            deleteWall(&(b->walls[2]));
        }

        if(walls[3])
        {
            b->walls[3].x = b->x + 2 * wall_thickness + wall_length;
            b->walls[3].y = b->y + wall_thickness;
            b->walls[3].orientation = VERTICAL;
        }
        else
        {
            deleteWall(&(b->walls[3]));
        }
    }

    int gameLoop(TFT_eSPI &TFTscreen)
    {
        float ax, ay;
        getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ax, &ay);
        
        ball firstBall;
        firstBall.x = WIDTH / 2;
        firstBall.y = HEIGHT / 2 + 30;
        firstBall.color = TFT_BLUE;

        wall firstEverWall;
        firstEverWall.orientation = HORIZONTAL;
        firstEverWall.x = (WIDTH + wall_length - wall_thickness) / 2;
        firstEverWall.y = (HEIGHT - wall_length + wall_thickness) / 2;
        firstEverWall.color = TFT_WHITE;
        firstEverWall.bounce_coefficient = 1.3;

        block firstBlock;
        firstBlock.x = MAP_POSITION_X;
        firstBlock.y = MAP_POSITION_Y;
        bool walls [4] = {true, false, true, false};
        generateWallsForBlock(&firstBlock, walls);

        block secondBlock;
        secondBlock.x = firstBlock.x + wall_thickness + wall_length;
        secondBlock.y = firstBlock.y;
        generateWallsForBlock(&secondBlock, walls);

        block map[2] = {firstBlock, secondBlock};

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

            checkCollisions(&firstBall, getBallsBlock(firstBall, map, MAP_SIZE_X * MAP_POSITION_Y)->walls, 4);

            if(dx < WIDTH && dx > 0)
            {
                firstBall.x += firstBall.speed_x;
            }
            else
            {
                firstBall.speed_x = 0;
            }
            if(dy < HEIGHT && dy > 0)
            {
                firstBall.y += firstBall.speed_y;
            }
            else
            {
                firstBall.speed_y = 0;
            }
            drawObjects(TFTscreen, firstBall, map, 2);
            GinaESP::drawScreen(TFTscreen);
        }

        return 0;
    }

}