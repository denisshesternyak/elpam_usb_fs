#ifndef INC_LCD_LCD_DISPLAY_H_
#define INC_LCD_LCD_DISPLAY_H_

#include "cmsis_os.h"
#include "lcd_color_rgb565.h"
#include "fonts.h"
#include "lcd_screen_rotation.h"
#include "defines.h"

#ifdef USE_LCD_ILI9341
	#include "lcd_ili9341_320_240.h"
#endif

#ifdef USE_LCD_ST7796
	#include "lcd_st7796_480_320.h"
#endif

#ifdef USE_LCD_ILI_9488
	#include "lcd_ili9488_480_320.h"
#endif


typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} Alignment;

void LCD_Init(void);
uint16_t LCD_GetWidth(void);
uint16_t LCD_GetHeight(void);

void LCD_SetRotation(ScreenRotation_t rotation);
void LCD_InvertColors(bool invert);
void LCD_WriteStringAligned(uint16_t y, const char* text, const FontDef* font, uint16_t textColor, uint16_t bgColor, Alignment align);
void LCD_WriteString(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor);
void LCD_WriteNString(int x, int y, const char* text, size_t len, const FontDef* font, uint16_t textColor, uint16_t bgColor);
void LCD_WriteChar(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color, uint16_t bgcolor);

void LCD_WriteChar_NoBG(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color);
void LCD_WriteString_NoBG(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color);

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void LCD_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_FillScreen(uint16_t color);

void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color);
//void LCD_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color, bool fill);
void LCD_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);

void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);

#endif /* INC_LCD_LCD_DISPLAY_H_ */
