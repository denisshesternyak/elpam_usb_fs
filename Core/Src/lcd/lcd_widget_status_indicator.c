#include "lcd_widget_status_indicator.h"
#include "hx8357d.h"

#define STATUS_INDICATOR_SIZE       15
#define STATUS_INDICATOR_RIGHT_X    230

#define INDICATOR_START_X           0
#define INDICATOR_START_Y           0
#define INDICATOR_SPACING_Y         25

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
    if (!ind) return;

    uint16_t textX = baseX + INDICATOR_START_X;
    uint16_t textY = baseY + INDICATOR_START_Y + index * INDICATOR_SPACING_Y;

    uint16_t rectX = baseX + STATUS_INDICATOR_RIGHT_X - STATUS_INDICATOR_SIZE;
    uint16_t rectY = textY + 2;

    uint16_t fillColor = ind->status ? COLOR_GREEN : COLOR_RED;

    hx8357_write_string(textX, textY, ind->label, &Font_11x18, COLOR_WHITE, COLOR_BLACK);
    hx8357_draw_rect(rectX, rectY, STATUS_INDICATOR_SIZE, STATUS_INDICATOR_SIZE, 1, COLOR_WHITE, fillColor);
}

void StatusDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{
    statusDisplayBaseX = baseX;
    statusDisplayBaseY = baseY;

//	const char* label = "Fault indication";
//
//	hx8357_fill_rect(baseX, baseY, STATUS_INDICATOR_RIGHT_X, 1, COLOR_WHITE);
//
//	FontDef *font = &Font_11x18;
//	int textWidth = font->width * strlen(label);
//	uint16_t textX = baseX + (STATUS_INDICATOR_RIGHT_X - textWidth) / 2;
//	uint16_t textY = baseY - font->height / 2;
//	hx8357_write_string(textX, textY, label, font, COLOR_WHITE, COLOR_BLACK);

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
