

#ifndef INC_LCD_LCD_ST7796_480_320_H_
#define INC_LCD_LCD_ST7796_480_320_H_

#include <stdint.h>
#include "fonts.h"
#include <stdbool.h>
#include "lcd_screen_rotation.h"

void ST7796_Init(void);
uint16_t ST7796_GetHeight(void);
uint16_t ST7796_GetWidth(void);
void ST7796_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7796_WriteString(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor);
void ST7796_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7796_FillScreen(uint16_t color);
void ST7796_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ST7796_InvertColors(bool invert);
void ST7796_WriteChar(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color, uint16_t bgcolor);

void ST7796_WriteChar_NoBG(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color);
void ST7796_WriteString_NoBG(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color);


void ST7796_SetRotation(ScreenRotation_t Rotation);

#endif /* INC_LCD_LCD_ST7796_480_320_H_ */
