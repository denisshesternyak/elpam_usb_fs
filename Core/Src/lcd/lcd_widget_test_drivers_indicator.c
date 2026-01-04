/*
 * lcd_widget_test_dev_indicator.c
 *
 *  Created on: Jun 24, 2025
 *      Author: roman
 */


#include <lcd_widget_test_drivers_indicator.h>
#include "lcd_display.h"
#include "lcd_color_rgb565.h"
#include <string.h>

#define STATUS_COUNT              10
#define STATUS_BOX_SIZE           40
#define STATUS_BOX_SPACING        5
#define STATUS_TEXT_COLOR         COLOR_WHITE
#define STATUS_BG_COLOR           COLOR_BLACK
#define STATUS_OK_COLOR           COLOR_GREEN
#define STATUS_FAIL_COLOR         COLOR_RED

#define STATUS_LABEL_OFFSET_Y     -13
#define STATUS_BOX_TEXT_Y_OFFSET   8
#define STATUS_BOX_TEXT_X_OFFSET   6

static bool statusArray[STATUS_COUNT] = {0};
static uint16_t statusBaseX = 0;
static uint16_t statusBaseY = 0;

typedef struct {
    const char* label;
    bool status;
} DriverIndicator;


static DriverIndicator indicators[] = {
		{"1", true},
		{"2", true},
		{"3", true},
		{"4", false},
		{"5", true},
		{"6", true},
		{"7", true},
		{"8", false},
		{"9", true},
		{"10", true}
};

static void DrawStatusBox(uint8_t index, uint16_t baseX, uint16_t baseY)
{
    if (index >= STATUS_COUNT) return;

    uint16_t x = baseX + index * (STATUS_BOX_SIZE + STATUS_BOX_SPACING);
    uint16_t y = baseY;

    const DriverIndicator* ind = &indicators[index];
    uint16_t fillColor = ind->status ? STATUS_OK_COLOR : STATUS_FAIL_COLOR;

    LCD_WriteString(x + 14, y + STATUS_LABEL_OFFSET_Y, ind->label, &Font_7x10, STATUS_TEXT_COLOR, STATUS_BG_COLOR);
    LCD_DrawRect(x, y, STATUS_BOX_SIZE, STATUS_BOX_SIZE, STATUS_TEXT_COLOR);
    LCD_FillRectangle(x + 1, y + 1, STATUS_BOX_SIZE - 2, STATUS_BOX_SIZE - 2, fillColor);
}

static const char* label = "Drivers";

void TestDevDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{
    statusBaseX = baseX;
    statusBaseY = baseY;

    uint16_t width = STATUS_COUNT * STATUS_BOX_SIZE + (STATUS_COUNT - 1) * STATUS_BOX_SPACING;
    uint16_t rectX = baseX - 4;
    uint16_t rectY = baseY - 25;
    uint16_t rectWidth = width + 8;
    uint16_t rectHeight = STATUS_BOX_SIZE + 30;

    LCD_DrawRect(rectX, rectY, rectWidth, rectHeight, STATUS_TEXT_COLOR);

    int textWidth = Font_7x10.width * strlen(label);
    uint16_t textX = rectX + (rectWidth - textWidth) / 2;
    uint16_t textY = rectY - Font_7x10.height / 2; 

    LCD_WriteString(textX, textY, label, &Font_7x10, COLOR_WHITE, COLOR_BLACK);

    for (uint8_t i = 0; i < STATUS_COUNT; ++i)
    {
        DrawStatusBox(i, baseX, baseY);
        osDelay(5);
    }
}

void TestDevDisplay_SetStatus(uint8_t index, bool status)
{
    if (index >= STATUS_COUNT) return;

    if (statusArray[index] != status) {
        statusArray[index] = status;
        DrawStatusBox(index, statusBaseX, statusBaseY);
    }
}
