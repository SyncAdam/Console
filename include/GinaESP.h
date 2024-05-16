#ifndef GINAESP_H
#define GINAESP_H

#include <cstdint>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>
#include <FS.h>
#include <SD.h>
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

void jpegRender(TFT_eSPI &TFTscreen, int xpos, int ypos) {

  //jpegInfo(); // Print information from the JPEG file (could comment this line out)

  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = TFTscreen.getSwapBytes();
  TFTscreen.setSwapBytes(true);
  
  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = mcu_w ? mcu_w < max_x : max_x;
  uint32_t min_h = mcu_h ? max_y > mcu_h : max_y;

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // Fetch data from the file, decode and display
  while (JpegDec.read()) {    // While there is more data in the file
    pImg = JpegDec.pImage ;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= TFTscreen.width() && ( mcu_y + win_h ) <= TFTscreen.height())
      TFTscreen.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ( (mcu_y + win_h) >= TFTscreen.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  TFTscreen.setSwapBytes(swapBytes);
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