
#include "lcd_display.h"
#include "lcd_color_rgb565.h"
#include "lcd_widget_status_indicator.h"


#define INDICATOR_COUNT (sizeof(indicators) / sizeof(indicators[0]))


#define STATUS_INDICATOR_SIZE       15
#define STATUS_INDICATOR_RIGHT_X    200  
#define STATUS_TEXT_COLOR           COLOR_WHITE
#define STATUS_BG_COLOR             COLOR_BLACK
#define STATUS_COLOR_OK             COLOR_GREEN
#define STATUS_COLOR_FAIL           COLOR_RED


#define INDICATOR_START_X           0
#define INDICATOR_START_Y           0
#define INDICATOR_SPACING_Y         20


typedef struct {
    const char* label;
    bool status;
} StatusIndicator;


static StatusIndicator indicators[] = {
    {"AC Voltage",      true},
    {"Battery Voltage", true},
    {"Charger Unit",    false},
    {"Amp",            false},
    {"Drv",            true},
    {"C&M Interface",   false},
    {"Door",            false},
    {"Flood",           true},
};

#define INDICATOR_COUNT (sizeof(indicators) / sizeof(indicators[0]))

static uint16_t statusDisplayBaseX = 0;
static uint16_t statusDisplayBaseY = 0;


static void DrawStatusIndicator(int index, uint16_t baseX, uint16_t baseY)
{
    const StatusIndicator* ind = &indicators[index];
    if (ind == NULL) return;

    uint16_t y = baseY + INDICATOR_START_Y + index * INDICATOR_SPACING_Y;
    uint16_t x = baseX + INDICATOR_START_X;

    uint16_t rectX = baseX + STATUS_INDICATOR_RIGHT_X - STATUS_INDICATOR_SIZE;
    uint16_t rectY = y + 2;

    uint16_t fillColor = ind->status ? STATUS_COLOR_OK : STATUS_COLOR_FAIL;

    LCD_WriteString(x, y, ind->label, &Font_11x18, STATUS_TEXT_COLOR, STATUS_BG_COLOR);

    LCD_DrawRect(rectX, rectY, STATUS_INDICATOR_SIZE, STATUS_INDICATOR_SIZE, STATUS_TEXT_COLOR);

    //LCD_FillRectangle(rectX + 1, rectY + 1,	  5, 5,	  fillColor);

    LCD_FillRectangle(rectX + 1, rectY + 1, STATUS_INDICATOR_SIZE - 2, STATUS_INDICATOR_SIZE - 2, fillColor);
}

void StatusDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{
    statusDisplayBaseX = baseX;
    statusDisplayBaseY = baseY;

	const uint16_t totalHeight = INDICATOR_COUNT * INDICATOR_SPACING_Y;
	const uint16_t totalWidth  = STATUS_INDICATOR_RIGHT_X + 2;

	
	    if ((baseX + totalWidth) > LCD_GetWidth() ||
	        (baseY + totalHeight) > LCD_GetHeight())
	    {
	        
	        return;
	    }

	LCD_DrawRect(baseX - 2, baseY - 2, totalWidth + 4, totalHeight + 3, STATUS_TEXT_COLOR);
	//osDelay(1);

    for (uint8_t i = 0; i < INDICATOR_COUNT; ++i)
    {
       DrawStatusIndicator(i, baseX, baseY);
    }
}

void StatusDisplay_SetStatus(uint8_t index, bool status)
{
    if (index < 0 || index >= INDICATOR_COUNT)
    {
        return;
    }

    if (indicators[index].status != status)
    {
        indicators[index].status = status;
        DrawStatusIndicator(index, statusDisplayBaseX, statusDisplayBaseY);
    }
}
