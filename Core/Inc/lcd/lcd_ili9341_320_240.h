
#ifndef __ILI9341_H__
#define __ILI9341_H__

#include "fonts.h"
#include <stdbool.h>
#include "main.h"

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

/* pin-to-pin
------------------------------
ILI9341     STM32
LED         VCC
SCK         PA5 (SPI1_SCK)
MOSI        PA7 (SPI1_MOSI)
DC          PC9
RESET       PB9
CS          PB8
GND         GND
VCC         VCC
-----------------------------
*/

#define ILI9341_DC_GPIO_Port     GPIOC
#define ILI9341_DC_Pin           GPIO_PIN_9

#define ILI9341_RES_GPIO_Port    GPIOB
#define ILI9341_RES_Pin          GPIO_PIN_9

#define ILI9341_CS_GPIO_Port     GPIOB
#define ILI9341_CS_Pin           GPIO_PIN_8


// default orientation
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)

// rotate right
/*
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
*/

// rotate left
/*
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
*/

// upside down
/*
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR)
*/

typedef enum {
    SCREEN_VERTICAL_1   = 0,   // Orientation 0°
    SCREEN_HORIZONTAL_1 = 1,   // Orientation 90°
    SCREEN_VERTICAL_2   = 2,   // Orientation 180°
    SCREEN_HORIZONTAL_2 = 3    // Orientation 270°
} ScreenRotation_t;


void ILI9341_Init(void);
uint16_t ILI9341_GetHeight(void);
uint16_t ILI9341_GetWidth(void);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
//void ILI9341_WriteStringAligned(uint16_t y, const char* text, const FontDef* font, uint16_t textColor, uint16_t bgColor, Alignment align);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9341_InvertColors(bool invert);
void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9341_SetRotation(ScreenRotation_t Rotation);


#endif // __ILI9341_H__
