#include "lcd_widget_batteries_indicator.h"
#include "hx8357d.h"

#define STATUS_INDICATOR_SIZE       15
#define STATUS_INDICATOR_RIGHT_X    230

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
    if (!ind) return;

    uint16_t textX = baseX + INDICATOR_START_X;
    uint16_t textY = baseY + INDICATOR_START_Y + index * INDICATOR_SPACING_Y;

    uint16_t rectX = baseX + STATUS_INDICATOR_RIGHT_X - STATUS_INDICATOR_SIZE;
    uint16_t rectY = textY + 2;

    uint16_t fillColor = ind->status ? COLOR_GREEN : COLOR_RED;

    hx8357_write_string(textX, textY, ind->label, &Font_11x18, COLOR_WHITE, COLOR_BLACK);
    hx8357_draw_rect(rectX, rectY, STATUS_INDICATOR_SIZE, STATUS_INDICATOR_SIZE, 1, COLOR_WHITE, fillColor);
}

void BatteriesDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{
	batteriesDisplayBaseX = baseX;
	batteriesDisplayBaseY = baseY;

//	const uint16_t totalHeight = INDICATOR_COUNT * INDICATOR_SPACING_Y + INDICATOR_SPACING_Y;
//	const uint16_t totalWidth = STATUS_INDICATOR_RIGHT_X + 17;
//
//	hx8357_outline_rect(baseX, baseY, totalWidth, totalHeight, 1, STATUS_TEXT_COLOR);

//	baseX += 10;
//	baseY += 10;

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


