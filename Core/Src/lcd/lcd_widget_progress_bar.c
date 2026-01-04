
#include "lcd_widget_progress_bar.h"
#include "lcd_display.h"  // LCD_DrawPixel, LCD_FillRectangle, LCD_WriteString
#include <string.h>
#include <stdio.h>
//

#define PROGRESS_BAR_H   	20
#define PROGRESS_BAR_W      350
#define PROGRESS_BAR_Y      200

#define PROGRESS_BORDER_COLOR     0xFFFF
#define PROGRESS_FILL_COLOR       0x07E0
#define PROGRESS_BACKGROUND_COLOR 0x0000
#define PROGRESS_TEXT_COLOR       0xFFFF

static uint8_t lastProgress = 255;
static bool borderDrawn = false;


static char text[6];
void MenuDrawProgress_tmp(uint8_t progressPercent)
{
    if (progressPercent > 100)
        progressPercent = 100;

    if (progressPercent == lastProgress)
        return;

    uint8_t prevProgress = lastProgress;
    lastProgress = progressPercent;

    uint16_t x = ((LCD_GetWidth() - PROGRESS_BAR_W) / 2);
    uint16_t y = PROGRESS_BAR_Y;
    uint16_t width = PROGRESS_BAR_W;
    uint16_t height = PROGRESS_BAR_H;

   
    uint16_t innerX = x + 1;
    uint16_t innerY = y + 1;
    uint16_t innerW = width - 2;
    uint16_t innerH = height - 2;

    uint16_t prevWidth = (innerW * prevProgress) / 100;
    uint16_t fillWidth = (innerW * progressPercent) / 100;

  
    LCD_DrawRect(x, y, width, height, PROGRESS_BORDER_COLOR);

    if (fillWidth < prevWidth) {
        
        LCD_FillRectangle(innerX + fillWidth, innerY, prevWidth - fillWidth, innerH, PROGRESS_BACKGROUND_COLOR);
    } else {
        
        LCD_FillRectangle(innerX + prevWidth, innerY, fillWidth - prevWidth, innerH, PROGRESS_FILL_COLOR);
    }

    
    snprintf(text, sizeof(text), "%d%%", progressPercent);
    const FontDef* font = &Font_7x10;
    uint16_t textWidth = strlen(text) * font->width;
    uint16_t textHeight = font->height;
    uint16_t textX = x + (width - textWidth) / 2;
    uint16_t textY = y + (height - textHeight) / 2;

   
    LCD_FillRectangle(textX, textY, textWidth, textHeight, PROGRESS_BACKGROUND_COLOR);

    uint16_t progressEndX = innerX + fillWidth;

    if (progressEndX > textX) {
        
        uint16_t fillTextWidth = (progressEndX > textX + textWidth) ? textWidth : (progressEndX - textX);
        LCD_WriteString(textX, textY, text, font, PROGRESS_TEXT_COLOR, PROGRESS_FILL_COLOR);

        
        if (fillTextWidth < textWidth) {
            
            char tempText[6];
            strncpy(tempText, text + fillTextWidth/font->width, sizeof(tempText));
            LCD_WriteString(textX + fillTextWidth, textY, tempText, font, PROGRESS_TEXT_COLOR, PROGRESS_BACKGROUND_COLOR);
        }
    } else {
        
        LCD_WriteString(textX, textY, text, font, PROGRESS_TEXT_COLOR, PROGRESS_BACKGROUND_COLOR);
    }
}

void MenuDrawProgress(uint8_t progressPercent)
{

    if (progressPercent > 100)
        progressPercent = 100;

    if (progressPercent == lastProgress)
        return;

    uint8_t prevProgress = lastProgress;
    lastProgress = progressPercent;

    uint16_t x = ((LCD_GetWidth() - PROGRESS_BAR_W) / 2);
    uint16_t y = PROGRESS_BAR_Y;
    uint16_t width = PROGRESS_BAR_W;
    uint16_t height  = PROGRESS_BAR_H;

    uint16_t innerX =  x + 1;
    uint16_t innerY = PROGRESS_BAR_Y + 1;
    uint16_t innerW = PROGRESS_BAR_W - 2;
    uint16_t innerH = PROGRESS_BAR_H - 2;

    uint16_t prevWidth = (innerW * prevProgress) / 100;
    uint16_t fillWidth = (innerW * progressPercent) / 100;

    //
    //if (!borderDrawn)
    {
        LCD_DrawRect(x, y, width, height, PROGRESS_BORDER_COLOR);
        //borderDrawn = true;
    }

    if (fillWidth < prevWidth) {
        
      //  LCD_FillRectangle(innerX + fillWidth, innerY, prevWidth - fillWidth, innerH, PROGRESS_BACKGROUND_COLOR);
    }
    else {
        LCD_FillRectangle(innerX + prevWidth, innerY, fillWidth - prevWidth, innerH, PROGRESS_FILL_COLOR);
    }


    snprintf(text, sizeof(text), "%d%%", progressPercent);

    const FontDef* font = &Font_7x10;

    uint16_t textWidth  = strlen(text) * font->width;
    uint16_t textHeight = font->height;

    uint16_t textX = x + (width - textWidth) / 2;
    uint16_t textY = y + (height - textHeight) / 2;

    LCD_FillRectangle(textX, textY, textWidth, textHeight, PROGRESS_BACKGROUND_COLOR);

    
    LCD_WriteString(textX, textY, text, font, PROGRESS_BORDER_COLOR, PROGRESS_BACKGROUND_COLOR);
    //LCD_WriteString_NoBG(textX, textY, text, font, PROGRESS_BORDER_COLOR);


}

void MenuResetProgressBar(void)
{
    
    lastProgress = 255;
    borderDrawn = false;
}

