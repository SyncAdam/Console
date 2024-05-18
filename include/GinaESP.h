#ifndef GINAESP_H
#define GINAESP_H

#include <cstdint>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "macros.h"

#define WIDTH 240
#define HEIGHT 320

namespace GinaESP{

  const uint16_t colors[24] = {TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_CYAN, TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK, TFT_BROWN, TFT_GOLD, TFT_SILVER, TFT_SKYBLUE, TFT_VIOLET};

  u_int8_t buffer [WIDTH * HEIGHT];

  void clearBuffer();
  void initGraphics();
  void drawBuffer(uint8_t buffer[WIDTH * HEIGHT]);
  void drawScreen(TFT_eSPI &TFTscreen);
  void drawFPS(TFT_eSPI &TFTscreen, long time);
  void fillCircle(TFT_eSPI &TFTscreen, int16_t x, int16_t y, uint8_t radius, uint16_t color);
  void drawHLine(TFT_eSPI &TFTscreen, int16_t x_start, int16_t x_end, int16_t y, uint16_t color);
  void drawVLine(TFT_eSPI &TFTscreen, int16_t y_start, int16_t y_end, int16_t x, uint16_t color);
  void drawRect(TFT_eSPI &TFTscreen);
  void fillRect(TFT_eSPI &TFTscreen, int16_t diagp1_x, int16_t diagp1_y, int16_t diagp2_x, int16_t diagp2_y, uint16_t color);
  void drawTriangle(TFT_eSPI &TFTscreen);

  void initGraphics()
  {
    clearBuffer();
  }

  void clearBuffer()
  {
    for(int i = 0; i < WIDTH * HEIGHT; i++)
    {
        buffer[i] = TFT_BLACK;
    }
  }

  void drawBuffer(uint8_t buffer[WIDTH * HEIGHT])
  {
    int CenterX = WIDTH / 2;
    int CenterY = HEIGHT / 2;

    int index = 0;
    long time = millis();

    for(int j = 0; j < HEIGHT; j++)
    {
      for(int i = 0; i < WIDTH; i++)
      {
        /*
          //create circular pattern
          int dx = i - CenterX;
          int dy = j - CenterY;
          int distance = sqrt(dx * dx + dy * dy + 10000 + time % 1000000);
          int ind = distance % 24;
        */
        //map distance to color array values
        buffer[index] = colors[(i + j) % 24];

        index++;
      }
    }
  }

  void drawScreen(TFT_eSPI &TFTscreen)
  {
      TFTscreen.pushImage(0, 0, WIDTH, HEIGHT, buffer);
  }

  void drawFPS(TFT_eSPI &TFTscreen, long time)
  {
      TFTscreen.setCursor(10, 10);
      TFTscreen.print(1000 / (millis() - time));
  }

  void fillCircle(TFT_eSPI &TFTscreen, int16_t x, int16_t y, uint8_t radius, uint16_t color)
  {
    for(int i = -radius; i <= radius; i++)
    {
        for(int j = -radius; j <= radius; j++)
        {
            if(i * i + j * j <= radius * radius)
            {
                int index = (y + j) * WIDTH + (x + i);
                if(index >= 0 && index < WIDTH * HEIGHT) // Check that the index is within the buffer
                {
                    buffer[index] = color;
                }
            }
        }
    }
  }

  void drawHLine(TFT_eSPI &TFTscreen, int16_t x_start, int16_t x_end, int16_t y, uint16_t color)
  {
    for(int i = min(x_start, x_end); i <= max(x_start, x_end); i++)
    {
      buffer[y * WIDTH + i] = color;
    }
  }

  void drawVLine(TFT_eSPI &TFTscreen, int16_t y_start, int16_t y_end, int16_t x, uint16_t color)
  {
    for(int i = min(y_start, y_end); i <= max(y_start, y_end); i++)
    {
      buffer[i * WIDTH + x] = color;
    }
  }

  void fillRect(TFT_eSPI &TFTscreen, int16_t diagp1_x, int16_t diagp1_y, int16_t diagp2_x, int16_t diagp2_y, uint16_t color)
  {
    for(int i = min(diagp1_x, diagp2_x); i <= max(diagp1_x, diagp2_x); i++)
    {
      drawVLine(TFTscreen, min(diagp1_y, diagp2_y), max(diagp1_y, diagp2_y), i, color);
    }
  }

  int16_t min(int16_t a, int16_t b)
  {
    return a ? a <= b : b;
  }

  int16_t max(int16_t a, int16_t b)
  {
    return a ? a >= b : b;
  }

}

#endif