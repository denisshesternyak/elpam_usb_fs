


#include "lcd_widget_batteries_indicator.h"
#include "lcd_display.h"
#include "lcd_color_rgb565.h"

#define INDICATOR_COUNT (sizeof(indicators) / sizeof(indicators[0]))


#define STATUS_INDICATOR_SIZE       15
#define STATUS_INDICATOR_RIGHT_X    200  
#define STATUS_TEXT_COLOR           COLOR_WHITE
#define STATUS_BG_COLOR             COLOR_BLACK
#define STATUS_COLOR_OK             COLOR_GREEN
#define STATUS_COLOR_FAIL           COLOR_RED


#define INDICATOR_START_X           0
#define INDICATOR_START_Y           0
#define INDICATOR_SPACING_Y         25


typedef struct {
    const char* label;
    bool status;
} BatteryIndicator;


static BatteryIndicator indicators[] = {
    {"Mains",           true},
    {"Under Voltage",   true},
    {"Over  Voltage",   false},
};

#define INDICATOR_COUNT (sizeof(indicators) / sizeof(indicators[0]))

static uint16_t batteriesDisplayBaseX = 0;
static uint16_t batteriesDisplayBaseY = 0;


static void DrawStatusIndicator(int index, uint16_t baseX, uint16_t baseY)
{
    const BatteryIndicator* ind = &indicators[index];

    uint16_t y = baseY + INDICATOR_START_Y + index * INDICATOR_SPACING_Y;
    uint16_t x = baseX + INDICATOR_START_X;

    uint16_t rectX = baseX + STATUS_INDICATOR_RIGHT_X - STATUS_INDICATOR_SIZE;
    uint16_t rectY = y + 2;

    uint16_t fillColor = ind->status ? STATUS_COLOR_OK : STATUS_COLOR_FAIL;

    LCD_WriteString(x, y, ind->label, &Font_11x18, STATUS_TEXT_COLOR, STATUS_BG_COLOR);
    LCD_DrawRect(rectX, rectY, STATUS_INDICATOR_SIZE, STATUS_INDICATOR_SIZE, STATUS_TEXT_COLOR);
    LCD_FillRectangle(rectX + 1, rectY + 1, STATUS_INDICATOR_SIZE - 2, STATUS_INDICATOR_SIZE - 2, fillColor);
}

void BatteriesDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{

	batteriesDisplayBaseX = baseX;
	batteriesDisplayBaseY = baseY;

	const uint16_t totalHeight = INDICATOR_COUNT * INDICATOR_SPACING_Y;
	const uint16_t totalWidth = STATUS_INDICATOR_RIGHT_X+2;

	LCD_DrawRect(baseX - 2, baseY - 2, totalWidth + 5, totalHeight , STATUS_TEXT_COLOR);


    for (uint8_t i = 0; i < INDICATOR_COUNT; ++i)
    {
        DrawStatusIndicator(i, baseX, baseY);
    }
}


void BatteriesDisplay_SetStatus(uint8_t index, bool status)
{
    if (index < 0 || index >= INDICATOR_COUNT)
        return;

    if (indicators[index].status != status)
    {
        indicators[index].status = status;
        DrawStatusIndicator(index, batteriesDisplayBaseX, batteriesDisplayBaseY);
    }
}


