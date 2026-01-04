

#include "lcd_widget_test_ampl_indicator.h"
#include "lcd_display.h"
#include "lcd_color_rgb565.h"
#include <string.h>

#define STATUS_COUNT              10
#define STATUS_BOX_SIZE           40
#define STATUS_BOX_SPACING        5
#define STATUS_TEXT_COLOR         COLOR_WHITE
#define STATUS_BG_COLOR           COLOR_BLACK
#define STATUS_OK_COLOR           COLOR_GREEN+4
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
} AmplIndicator;


static AmplIndicator indicators[] = {
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

    const AmplIndicator* ind = &indicators[index];

    if (ind == NULL) return;


    uint16_t x = baseX + index * (STATUS_BOX_SIZE + STATUS_BOX_SPACING);
    uint16_t y = baseY;
    uint16_t fillColor = ind->status ? STATUS_OK_COLOR : STATUS_FAIL_COLOR;

    LCD_WriteString(x + 8, y + STATUS_LABEL_OFFSET_Y, ind->label, &Font_7x10, STATUS_TEXT_COLOR, STATUS_BG_COLOR);

    LCD_DrawRect(x, y, STATUS_BOX_SIZE, STATUS_BOX_SIZE, STATUS_TEXT_COLOR);
    LCD_FillRectangle(x + 1, y + 1, STATUS_BOX_SIZE - 2, STATUS_BOX_SIZE - 2, fillColor);
}

static void DrawCenteredLabelWithLines(uint16_t baseX, uint16_t baseY, const char* text, uint16_t width)
{
    uint16_t textLen = strlen(text) * Font_7x10.width;
    uint16_t lineWidth = (width - textLen - 10) / 2;

    LCD_DrawLine(baseX, baseY + 8, baseX + lineWidth, baseY + 8, COLOR_WHITE);

    LCD_WriteString(baseX + lineWidth + 5, baseY, text, &Font_7x10, COLOR_WHITE, COLOR_BLACK);

    LCD_DrawLine(baseX + lineWidth + 5 + textLen + 5, baseY + 5, baseX + width, baseY + 5, COLOR_WHITE);
}


void TestAmplDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{
	 statusBaseX = baseX;
	  statusBaseY = baseY;

	const char* label = "Amplifires";

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
        osDelay(3);
    }
}

void TestAmplDisplay_SetStatus(uint8_t index, bool status)
{
    if (index >= STATUS_COUNT) return;

    if (statusArray[index] != status) {
        statusArray[index] = status;
        DrawStatusBox(index, statusBaseX, statusBaseY);

    }
}
