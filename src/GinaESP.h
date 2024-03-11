#ifndef GINAESP_H
#define GINAESP_H

#include <cstdint>
#include <TFT_eSPI.h>

#define WIDTH 240
#define HEIGHT 320

const uint16_t colors[24] = {TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_CYAN, TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK, TFT_BROWN, TFT_GOLD, TFT_SILVER, TFT_SKYBLUE, TFT_VIOLET};

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

void drawScreen(TFT_eSPI &TFTscreen, uint8_t buffer[WIDTH * HEIGHT])
{
    TFTscreen.pushImage(0, 0, WIDTH, HEIGHT, buffer);
}

void drawFPS(TFT_eSPI &TFTscreen, uint8_t buffer[WIDTH * HEIGHT], long time)
{
    TFTscreen.setCursor(10, 10);
    TFTscreen.print(1000 / (millis() - time));
}

void fillCircle(TFT_eSPI &TFTscreen, uint8_t buffer[WIDTH * HEIGHT], uint16_t x, uint16_t y, uint8_t radius, uint16_t color)
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

void drawHLine()
{

}

void drawVLine()
{

}

void drawLine()
{

}

void drawRect()
{

}

void drawTriangle()
{

}



#endif