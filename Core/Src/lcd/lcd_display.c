
#include "lcd_display.h"
#include <stdlib.h>
#include <string.h>

extern osMutexId_t lcdMutexHandle;

void _LCD_DrawPixelRaw(uint16_t x, uint16_t y, uint16_t color);
static uint16_t _LCD_GetWidthRaw(void);
static void _LCD_WriteStringRaw(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor);
void _LCD_DrawLineRaw(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void _LCD_WriteCharRaw(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color, uint16_t bgcolor);


void LCD_Init(void)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

#ifdef USE_LCD_ILI9341
	ILI9341_Init();
#endif

#ifdef	USE_LCD_ST7796
	ST7796_Init();
#endif //USE_LCD_ST7796

#ifdef USE_LCD_ILI_9488
	ILI9488_Init();
#endif //USE_LCD_ILI_9488

	osMutexRelease(lcdMutexHandle);
}


uint16_t LCD_GetHeight(void)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	uint16_t result = 0;

	#ifdef USE_LCD_ILI9341
		result = ILI9341_GetHeight();
	#endif

	#ifdef	USE_LCD_ST7796
		result = ST7796_GetHeight();
	#endif //USE_LCD_ST7796

	#ifdef	USE_LCD_ILI_9488
		result = ILI9488_GetHeight();
	#endif //USE_LCD_ILI_9488

	osMutexRelease(lcdMutexHandle);
	return result;
}

uint16_t LCD_GetWidth(void)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	uint16_t result = 0;
	#ifdef USE_LCD_ILI9341
		result = ILI9341_GetWidth();
	#endif

	#ifdef	USE_LCD_ST7796
		result = ST7796_GetWidth();
	#endif //USE_LCD_ST7796


	#ifdef	USE_LCD_ILI_9488
		result = ILI9488_GetWidth();
	#endif //USE_LCD_ILI_9488

	osMutexRelease(lcdMutexHandle);
	return result;
}

static uint16_t _LCD_GetWidthRaw(void)
{
	#ifdef USE_LCD_ILI9341
		return ILI9341_GetWidth();
	#endif

	#ifdef USE_LCD_ST7796
		return ST7796_GetWidth();
	#endif

	#ifdef	USE_LCD_ILI_9488
		return  ILI9488_GetWidth();
	#endif //USE_LCD_ILI_9488
}

static void _LCD_WriteStringRaw(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor)
{
	#ifdef USE_LCD_ILI9341
		ILI9341_WriteString(x, y, str, font, color, bgcolor);
	#endif

	#ifdef USE_LCD_ST7796
		ST7796_WriteString(x, y, str, font, color, bgcolor);
	#endif

	#ifdef USE_LCD_ILI_9488
			ILI9488_WriteString(x, y, str, font, color, bgcolor);
	#endif
}

void LCD_WriteStringAligned(uint16_t y, const char* text, const FontDef* font, uint16_t textColor, uint16_t bgColor, Alignment align)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	uint16_t textWidth = strlen(text) * font->width;
	uint16_t x;

	switch (align) {
		case ALIGN_LEFT:
			x = 0;
			break;
		case ALIGN_CENTER:
			x = (_LCD_GetWidthRaw() - textWidth) / 2;
			break;
		case ALIGN_RIGHT:
			x = _LCD_GetWidthRaw() - textWidth;
			break;
	}

	_LCD_WriteStringRaw(x, y, text, font, textColor, bgColor);

	osMutexRelease(lcdMutexHandle);
}


void _LCD_DrawLineRaw(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t dx = abs(x1 - x0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = -abs(y1 - y0);
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    int16_t e2;

    while (1)
    {
    	_LCD_DrawPixelRaw(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

    int16_t dx = abs(x1 - x0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = -abs(y1 - y0);
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    int16_t e2;

    while (1)
    {
    	_LCD_DrawPixelRaw(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }

    osMutexRelease(lcdMutexHandle);
}

void LCD_WriteNString(int x, int y, const char* text, size_t len, const FontDef* font, uint16_t textColor, uint16_t bgColor)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

    for (size_t i = 0; i < len; ++i)
    {
        if (text[i] == '\0') break;

        _LCD_WriteCharRaw(x, y, text[i], font, textColor, bgColor);

        x += font->width;
    }

    osMutexRelease(lcdMutexHandle);
}

void LCD_WriteString(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_WriteString(x, y, str, font, color, bgcolor);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_WriteString(x, y, str, font, color, bgcolor);
	#endif //USE_LCD_ST7796

	#ifdef USE_LCD_ILI_9488
		ILI9488_WriteString(x, y, str, font, color, bgcolor);
	#endif

	osMutexRelease(lcdMutexHandle);
}

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	_LCD_DrawLineRaw(x, y, x + width - 1, y, color);

	_LCD_DrawLineRaw(x, y + height - 1, x + width - 1, y + height - 1, color);

	_LCD_DrawLineRaw(x, y, x, y + height - 1, color);

	_LCD_DrawLineRaw(x + width - 1, y, x + width - 1, y + height - 1, color);

    osMutexRelease(lcdMutexHandle);
}

void _LCD_DrawPixelRaw(uint16_t x, uint16_t y, uint16_t color)
{

	#ifdef USE_LCD_ILI9341
		ILI9341_DrawPixel(x, y, color);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_DrawPixel(x, y, color);
	#endif //USE_LCD_ST7796

	#ifdef	USE_LCD_ILI_9488
		ILI9488_DrawPixel(x, y, color);
	#endif //USE_LCD_ST7796

}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_DrawPixel(x, y, color);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_DrawPixel(x, y, color);
	#endif //USE_LCD_ST7796

	#ifdef	USE_LCD_ILI_9488
		ILI9488_DrawPixel(x, y, color);
	#endif //USE_LCD_ST7796
	osMutexRelease(lcdMutexHandle);
}


void LCD_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_FillRectangle(x, y, w, h, color);
	#endif //LCD_LCD_ILI9341


	#ifdef	USE_LCD_ST7796
		ST7796_FillRectangle(x, y, w, h, color);
	#endif //USE_LCD_ST7796

	#ifdef	USE_LCD_ILI_9488
		ILI9488_FillRectangle(x, y, w, h, color);
	#endif //USE_LCD_ST7796

	osMutexRelease(lcdMutexHandle);
}

void LCD_FillScreen(uint16_t color)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_FillScreen(color);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_FillScreen(color);
	#endif //USE_LCD_ST7796

	#ifdef USE_LCD_ILI_9488
		ILI9488_FillScreen(color);
	#endif //USE_LCD_ST7796

	osMutexRelease(lcdMutexHandle);
}

void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_DrawImage(x, y, w, h, data);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_DrawImage(x, y, w, h, data);
	#endif //USE_LCD_ST7796

	#ifdef USE_LCD_ILI_9488
		ILI9488_DrawImage(x, y, w, h, data);
	#endif //USE_LCD_ST7796

	osMutexRelease(lcdMutexHandle);
}

void LCD_InvertColors(bool invert)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_InvertColors(invert);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_InvertColors(invert);
	#endif //USE_LCD_ST7796

	#ifdef USE_LCD_ILI_9488
		ILI9488_InvertColors(invert);
	#endif //USE_LCD_ST7796

	osMutexRelease(lcdMutexHandle);
}

void _LCD_WriteCharRaw(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color, uint16_t bgcolor)
{
	#ifdef USE_LCD_ILI9341
		ILI9341_WriteChar(x, y, ch, font, color, bgcolor);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_WriteChar(x, y, ch, font, color, bgcolor);
	#endif //USE_LCD_ST7796

	#ifdef	USE_LCD_ILI_9488
		ILI9488_WriteChar(x, y, ch, font, color, bgcolor);
	#endif //USE_LCD_ST7796
}


void LCD_WriteChar_NoBG(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

		#ifdef USE_LCD_ILI9341
			//ILI9341_WriteChar(x, y, ch, font, color, bgcolor);
		#endif //LCD_LCD_ILI9341

		#ifdef	USE_LCD_ST7796
			ST7796_WriteChar_NoBG(x, y, ch, font, color);
		#endif //USE_LCD_ST7796

		#ifdef	USE_LCD_ILI_9488
				//ILI9488_WriteChar_NoBG(x, y, ch, font, color);
		#endif //USE_LCD_ST7796

		osMutexRelease(lcdMutexHandle);
}

void LCD_WriteString_NoBG(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

		#ifdef USE_LCD_ILI9341
			//ILI9341_WriteChar(x, y, ch, font, color, bgcolor);
		#endif //LCD_LCD_ILI9341

		#ifdef	USE_LCD_ST7796
			ST7796_WriteString_NoBG(x, y, str, font, color);
		#endif //USE_LCD_ST7796

		#ifdef	USE_LCD_ILI_9488
				//
		#endif //USE_LCD_ST7796

	osMutexRelease(lcdMutexHandle);
}

void LCD_WriteChar(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color, uint16_t bgcolor)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_WriteChar(x, y, ch, font, color, bgcolor);
	#endif //LCD_LCD_ILI9341

	#ifdef USE_LCD_ST7796
		ST7796_WriteChar(x, y, ch, font, color, bgcolor);
	#endif //USE_LCD_ST7796

	#ifdef USE_LCD_ILI_9488
		ILI9488_WriteChar(x, y, ch, font, color, bgcolor);
	#endif //USE_LCD_ST7796

	osMutexRelease(lcdMutexHandle);
}

void LCD_SetRotation(ScreenRotation_t Rotation)
{
	osMutexAcquire(lcdMutexHandle, osWaitForever);

	#ifdef USE_LCD_ILI9341
		ILI9341_SetRotation(Rotation);
	#endif //LCD_LCD_ILI9341

	#ifdef	USE_LCD_ST7796
		ST7796_SetRotation(Rotation);
	#endif //USE_LCD_ST7796

	#ifdef USE_LCD_ILI_9488
		ILI9488_SetRotation(Rotation);
	#endif //USE_LCD_ST7796

	osMutexRelease(lcdMutexHandle);
}


void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        LCD_DrawPixel(x0 + x, y0 + y, color);
        LCD_DrawPixel(x0 + y, y0 + x, color);
        LCD_DrawPixel(x0 - y, y0 + x, color);
        LCD_DrawPixel(x0 - x, y0 + y, color);
        LCD_DrawPixel(x0 - x, y0 - y, color);
        LCD_DrawPixel(x0 - y, y0 - x, color);
        LCD_DrawPixel(x0 + y, y0 - x, color);
        LCD_DrawPixel(x0 + x, y0 - y, color);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}


void LCD_FillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        
        LCD_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
        LCD_DrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
        LCD_DrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
        LCD_DrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);

        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void LCD_DrawCircleExt(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color, bool fill) 
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        if (fill) {
            
            LCD_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
            LCD_DrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
            LCD_DrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
            LCD_DrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
        } else {
            
            LCD_DrawPixel(x0 + x, y0 + y, color);
            LCD_DrawPixel(x0 + y, y0 + x, color);
            LCD_DrawPixel(x0 - y, y0 + x, color);
            LCD_DrawPixel(x0 - x, y0 + y, color);
            LCD_DrawPixel(x0 - x, y0 - y, color);
            LCD_DrawPixel(x0 - y, y0 - x, color);
            LCD_DrawPixel(x0 + y, y0 - x, color);
            LCD_DrawPixel(x0 + x, y0 - y, color);
        }

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}


void LCD_DrawCircleQuarter(uint16_t x0, uint16_t y0, uint16_t r, uint8_t corner, uint16_t color)
{
    int x = 0;
    int y = r;
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;

    while (x <= y) {
        if (corner & 0x1) LCD_DrawPixel(x0 - x, y0 - y, color);
        if (corner & 0x2) LCD_DrawPixel(x0 + x, y0 - y, color);
        if (corner & 0x4) LCD_DrawPixel(x0 - x, y0 + y, color);
        if (corner & 0x8) LCD_DrawPixel(x0 + x, y0 + y, color);

        if (corner & 0x1) LCD_DrawPixel(x0 - y, y0 - x, color);
        if (corner & 0x2) LCD_DrawPixel(x0 + y, y0 - x, color);
        if (corner & 0x4) LCD_DrawPixel(x0 - y, y0 + x, color);
        if (corner & 0x8) LCD_DrawPixel(x0 + y, y0 + x, color);

        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
    }
}


void LCD_DrawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{
    
    LCD_DrawLine(x + r, y, x + w - r - 1, y, color);
    LCD_DrawLine(x + r, y + h - 1, x + w - r - 1, y + h - 1, color);

   
    LCD_DrawLine(x, y + r, x, y + h - r - 1, color);
    LCD_DrawLine(x + w - 1, y + r, x + w - 1, y + h - r - 1, color);

    LCD_DrawCircleQuarter(x + r,     y + r,     r, 0x1, color);
    LCD_DrawCircleQuarter(x + w - r - 1, y + r,     r, 0x2, color);
    LCD_DrawCircleQuarter(x + r,     y + h - r - 1, r, 0x4, color);
    LCD_DrawCircleQuarter(x + w - r - 1, y + h - r - 1, r, 0x8, color);
}


void LCD_FillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color)
{
    LCD_FillRectangle(x + radius, y, w - 2 * radius, h, color);

    LCD_FillRectangle(x, y + radius, radius, h - 2 * radius, color);
    LCD_FillRectangle(x + w - radius, y + radius, radius, h - 2 * radius, color);

    int16_t dx, dy;
    int16_t rsq = radius * radius;
    for (dy = 0; dy < radius; dy++) {
        for (dx = 0; dx < radius; dx++) {
            if ((dx*dx + dy*dy) <= rsq) {
                LCD_DrawPixel(x + radius - dx - 1, y + radius - dy - 1, color);
                LCD_DrawPixel(x + w - radius + dx, y + radius - dy - 1, color);
                LCD_DrawPixel(x + radius - dx - 1, y + h - radius + dy, color);
                LCD_DrawPixel(x + w - radius + dx, y + h - radius + dy, color);
            }
        }
    }
}



#include <math.h>

#define DEG_TO_RAD(x) ((x) * 3.1415926f / 180.0f)


void LCD_DrawArcQuarter(uint16_t cx, uint16_t cy, uint16_t r, float startAngle, float endAngle, uint16_t color)
{
    float step = 5.0f;
    int x1 = cx + r * cosf(DEG_TO_RAD(startAngle));
    int y1 = cy + r * sinf(DEG_TO_RAD(startAngle));

    for (float angle = startAngle + step; angle <= endAngle; angle += step) {
        int x2 = cx + r * cosf(DEG_TO_RAD(angle));
        int y2 = cy + r * sinf(DEG_TO_RAD(angle));
        LCD_DrawLine(x1, y1, x2, y2, color);
        x1 = x2;
        y1 = y2;
    }
}


void LCD_DrawRoundRectArc(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{

    LCD_DrawLine(x + r, y, x + w - r, y, color);             
    LCD_DrawLine(x + r, y + h - 1, x + w - r, y + h - 1, color);
    LCD_DrawLine(x, y + r, x, y + h - r, color);             
    LCD_DrawLine(x + w - 1, y + r, x + w - 1, y + h - r, color); 

    
    LCD_DrawArcQuarter(x + r, y + r, r, 180, 270, color); 
    LCD_DrawArcQuarter(x + w - r - 1, y + r, r, 270, 360, color); 
    LCD_DrawArcQuarter(x + r, y + h - r - 1, r, 90, 180, color); 
    LCD_DrawArcQuarter(x + w - r - 1, y + h - r - 1, r, 0, 90, color);
}
