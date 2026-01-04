#ifndef INC_LCD_LCD_ILI9488_480_320_H_
#define INC_LCD_LCD_ILI9488_480_320_H_

#include "defines.h"

#ifdef USE_LCD_ILI_9488

#include <stdint.h>
#include <stdbool.h>
#include "fonts.h"
#include "lcd_screen_rotation.h"

void ILI9488_Init(void);
void ILI9488_SetRotation(ScreenRotation_t rotation);
void ILI9488_InvertColors(bool invert);
void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9488_FillScreen(uint16_t color);
void ILI9488_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);

void ILI9488_WriteChar(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color, uint16_t bgcolor);
void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor);

uint16_t ILI9488_GetWidth(void);
uint16_t ILI9488_GetHeight(void);


#endif //USE_LCD_ILI_9488

#endif /* INC_LCD_LCD_ILI9488_480_320_H_ */
