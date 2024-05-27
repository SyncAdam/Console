#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_MPU6050.h>
#include <cstdlib>

#include "GinaESP.h"
#include "macros.h"
#include "LabMaps.h"

#define MAP_SIZE_X 7
#define MAP_SIZE_Y 7

#define MAP_POSITION_X 3
#define MAP_POSITION_Y 50

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
        bool killOnImpact = false;
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
        float radius = 6;
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

    void initialize(Adafruit_MPU6050* fmpu);
    void getInput(bool* button1, bool* button2, int* joyx, int* joyy, bool* joy_button, float* ax, float* ay);
    int playLab(Adafruit_MPU6050* fmpu, TFT_eSPI &TFTscreen);
    void drawObjects(TFT_eSPI &TFTscreen, ball gameball, wall Walls[], int nWalls);
    void resetBall(ball* gameball);
    bool checkCollisions(ball* gameball, wall Walls[], int nWalls);
    void generateBlocksFromLogic(block blocks[], const int verticals1[], const int horizontals1[]);
    void deleteWall(wall* w);
    void generateWallsForBlock(block* b, int walls[]);
    int labLoop(bool* gameWon, ball *gameBall, block map[], int mapsize, TFT_eSPI &TFTscreen);
    int game(TFT_eSPI &TFTscreen);
    int printMenu(TFT_eSPI &TFTscreen);

    bool button1, button2;
    int joyx, joyy;
    bool joy_button;
    int ballx, bally;
    
    Adafruit_MPU6050 mpu;

    const int max_speed = 7;
    const int max_acceleration = 3;
    const float acceleration_sensitivity = 0.2;

    const int wall_length = 30;
    const int wall_thickness = 3;

    const int start_pos_x = 120;
    const int start_pos_y = 300;

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

    int printMenu(TFT_eSPI &TFTscreen)
    {
        GinaESP::clearBuffer();
        GinaESP::drawScreen(TFTscreen);

        TFTscreen.setTextSize(3);
        TFTscreen.setTextColor(TFT_BROWN);
        TFTscreen.setCursor(30, 100);
        TFTscreen.printf("LABYRINTHE");

        TFTscreen.setTextColor(TFT_WHITE);
        TFTscreen.setTextSize(1);
        TFTscreen.setCursor(45, 300);
        TFTscreen.printf("Press any button to start");

        delay(1500);

        float ax, ay;

        getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ax, &ay);
        while(!button1 && !button2 && !joy_button)
        {
            getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ax, &ay);
            delay(100);
        }
        if(button1)
        {
            return 1;
        }

        return 0;
        

    }

    int playLab(Adafruit_MPU6050* fmpu, TFT_eSPI &TFTscreen)
    {
        initialize(fmpu);

        printMenu(TFTscreen);
        
        return(game(TFTscreen));
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

    void resetBall(ball* gameball)
    {
        gameball->speed_x = 0;
        gameball->speed_y = 0;
        gameball->x = start_pos_x;
        gameball->y = start_pos_y;
    }

    bool checkCollisions(ball* gameball, wall Walls[], int nWalls)
    {   
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
                        if(Walls[i].killOnImpact)
                        {
                            resetBall(gameball);
                        }
                        return true;
                    }
                    if((dy + gameball->radius > Walls[i].y &&
                        dy - gameball->radius < Walls[i].y + wall_length) &&
                    (gameball->x + gameball->radius > Walls[i].x - wall_thickness &&
                        gameball->x - gameball->radius < Walls[i].x))
                    {
                        gameball->speed_y = -gameball->speed_y * Walls[i].bounce_coefficient;
                        if(Walls[i].killOnImpact)
                        {
                            resetBall(gameball);
                        }
                        return true;
                    }
                    break;

                case HORIZONTAL:
                    if((dy + gameball->radius > Walls[i].y - wall_thickness &&
                        dy - gameball->radius < Walls[i].y) &&
                    (gameball->x + gameball->radius > Walls[i].x &&
                        gameball->x - gameball->radius < Walls[i].x + wall_length))
                    {
                        gameball->speed_y = -gameball->speed_y  * Walls[i].bounce_coefficient;
                        if(Walls[i].killOnImpact)
                        {
                            resetBall(gameball);
                        }
                        return true;
                    }
                    if((dx + gameball->radius > Walls[i].x &&
                        dx - gameball->radius < Walls[i].x + wall_length) &&
                    (gameball->y + gameball->radius > Walls[i].y - wall_thickness &&
                        gameball->y - gameball->radius < Walls[i].y))
                    {
                        gameball->speed_x = -gameball->speed_x  * Walls[i].bounce_coefficient;
                        if(Walls[i].killOnImpact)
                        {
                            resetBall(gameball);
                        }
                        return true;
                    }
                    break;
            }
        }
        return false;
    }

    void generateBlocksFromLogic(block blocks[], const int verticals1[], const int horizontals1[])
    {
        int offset_x = MAP_POSITION_X;
        int offset_y = MAP_POSITION_Y;
        for(int i = 0; i < MAP_SIZE_Y; i++)
        {
            for(int j = 0; j < MAP_SIZE_X; j++)
            {
                block b;
                b.x = j * (wall_length + wall_thickness) + offset_x;
                b.y = i * (wall_length + wall_thickness) + offset_y;
                int walls[4] = {horizontals1[j + i * (MAP_SIZE_Y)], verticals1[j + i * (MAP_SIZE_X + 1)], horizontals1[j + (i + 1) * (MAP_SIZE_Y)], verticals1[j + i * (MAP_SIZE_X + 1) + 1]};
                generateWallsForBlock(&b, walls);
                blocks[i * (MAP_SIZE_X) + j] = b;
            }
        }
    }

    void deleteWall(wall* w)
    {
        w->x = -100;
        w->y = -100;
        w->orientation = -1;
    }

    void generateWallsForBlock(block* b, int walls[])
    {

        for(int i = 0; i < 4; i++)
        {
            switch(walls[i])
            {
                case 1:
                    break;

                case 2:
                    b->walls[i].color = TFT_SKYBLUE;
                    b->walls[i].bounce_coefficient = 2.0f;
                    break;
            
                case 3:
                    b->walls[i].color = TFT_YELLOW;
                    b->walls[i].killOnImpact = true;
                    break;
                
                default:
                    deleteWall(&(b->walls[i]));
                    break;
            }
        }

        if(walls[0] != 0)
        {
            b->walls[0].x = b->x + wall_thickness;
            b->walls[0].y = b->y;
            b->walls[0].orientation = HORIZONTAL;
        }
        else
        {
            deleteWall(&(b->walls[0]));
        }

        if(walls[1] != 0)
        {
            b->walls[1].x = b->x + wall_thickness;
            b->walls[1].y = b->y + wall_thickness;
            b->walls[1].orientation = VERTICAL;
        }
        else
        {
            deleteWall(&(b->walls[1]));
        }

        if(walls[2] != 0)
        {
            b->walls[2].x = b->x + wall_thickness;
            b->walls[2].y = b->y + wall_thickness + wall_length;
            b->walls[2].orientation = HORIZONTAL;
        }
        else
        {
            deleteWall(&(b->walls[2]));
        }

        if(walls[3] != 0)
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

    int labLoop(bool* gameWon, ball *gameBall, block map[], int mapsize, TFT_eSPI &TFTscreen, bool firstRound)
    {
        float ax, ay;
        GinaESP::clearBuffer();
        drawObjects(TFTscreen, *gameBall, map, mapsize);
        GinaESP::fillRect(TFTscreen, 0, 0, 30, 30, TFTscreen.color565(211, 211, 211));
        GinaESP::fillRect(TFTscreen, 5, 5, 25, 25, TFT_BLACK);
        GinaESP::drawScreen(TFTscreen);

        delay(1000);

        getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ax, &ay);

        while(!*gameWon)
        {
            GinaESP::clearBuffer();
            getInput(&button1, &button2, &joyx, &joyy, &joy_button, &ay, &ax);

            ax *= acceleration_sensitivity;
            ay *= acceleration_sensitivity;

            if(ax > max_acceleration) ax = max_acceleration;
            if(ax < -max_acceleration) ax = -max_acceleration;
            if(ay > max_acceleration) ay = max_acceleration;
            if(ay < -max_acceleration) ay = -max_acceleration;

            if(gameBall->speed_x <= max_speed && gameBall->speed_x >= -max_speed)
            {
                gameBall->speed_x += ax;
            }
            else if(gameBall->speed_x > max_speed)
            {
                gameBall->speed_x = max_speed;
            }
            else if(gameBall->speed_x < -max_speed)
            {
                gameBall->speed_x = -max_speed;
            }
            if(gameBall->speed_y <= max_speed && gameBall->speed_y >= -max_speed)
            {
                gameBall->speed_y += ay;
            }
            else if(gameBall->speed_y > max_speed)
            {
                gameBall->speed_y = max_speed;
            }
            else if(gameBall->speed_y < -max_speed)
            {
                gameBall->speed_y = -max_speed;
            }

            int16_t dx = gameBall->x + gameBall->speed_x;
            int16_t dy = gameBall->y + gameBall->speed_y;

            for(int i = 0; i < mapsize; i++)
            {
                if(checkCollisions(gameBall, map[i].walls, 4)) break;
            }
            
            if(dx < WIDTH - gameBall->radius && dx > 0 + gameBall->radius)
            {
                gameBall->x += gameBall->speed_x;
            }
            else
            {
                gameBall->speed_x = 0;
            }
            if(dy < HEIGHT - gameBall->radius && dy > 0 + gameBall->radius)
            {
                gameBall->y += gameBall->speed_y;
            }
            else
            {
                gameBall->speed_y = 0;
            }
            if(gameBall->x < 30 && gameBall->y < 30)
            {
                *gameWon = true;
            }
            if(button2 && button1)
            {
                *gameWon = true;
            }

            drawObjects(TFTscreen, *gameBall, map, mapsize);
            GinaESP::fillRect(TFTscreen, 0, 0, 30, 30, TFTscreen.color565(211, 211, 211));
            GinaESP::fillRect(TFTscreen, 5, 5, 25, 25, TFT_BLACK);
            GinaESP::drawScreen(TFTscreen);

            if(button1) return 1;

        }

        if(gameWon) return 0;
        else if(button1) return 1;
        else return 2;

    }

    int game(TFT_eSPI &TFTscreen)
    {        
        int start_pos_x = 120;
        int start_pos_y = 300;

        const short nGames = 5;

        GinaESP::clearBuffer();
        GinaESP::drawScreen(TFTscreen);

        ball firstBall;
        firstBall.color = TFT_NAVY;

        short mapsize = MAP_SIZE_X * MAP_SIZE_Y;

        firstBall.x = start_pos_x;
        firstBall.y = start_pos_y;

        for(int i = 0; i < nGames; i++)
        {
            bool gameWon = false;
            block map[MAP_SIZE_X * MAP_SIZE_Y];
            generateBlocksFromLogic(map, verticals[i], horizontals[i]);
                    
            firstBall.x = start_pos_x;
            firstBall.y = start_pos_y;
            firstBall.speed_x = 0;
            firstBall.speed_y = 0;

            if(labLoop(&gameWon, &firstBall, map, mapsize, TFTscreen, 1 ? i == 0 : 0) != 0)
            {
                if(printMenu(TFTscreen) != 0)
                {
                    return 1;
                }
            }   
        }
        
        return 0;
    }

}