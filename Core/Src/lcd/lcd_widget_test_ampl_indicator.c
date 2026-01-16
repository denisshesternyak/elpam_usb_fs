#include "lcd_widget_test_basic_indicator.h"
#include "lcd_widget_test_ampl_indicator.h"

static const char* label = "Amplifires";
StatusBasic statusAmpl;

//static void DrawCenteredLabelWithLines(uint16_t baseX, uint16_t baseY, const char* text, uint16_t width)
//{
//    uint16_t textLen = strlen(text) * Font_7x10.width;
//    uint16_t lineWidth = (width - textLen - 10) / 2;
//
//    hx8357_outline_rect(baseX, baseY + 8, baseX + lineWidth, baseY + 8, 1, COLOR_WHITE);
//
//    hx8357_write_string(baseX + lineWidth + 5, baseY, text, &Font_7x10, COLOR_WHITE, COLOR_BLACK);
//
//    hx8357_outline_rect(baseX + lineWidth + 5 + textLen + 5, baseY + 5, baseX + width, baseY + 5, 1, COLOR_WHITE);
//}


void TestAmplDisplay_DrawAll(uint16_t baseX, uint16_t baseY)
{
	//test
	statusAmpl.items[0] = true;
	statusAmpl.items[1] = true;

	statusAmpl.baseX = baseX;
	statusAmpl.baseY = baseY;
	strcpy(statusAmpl.label, label);

	TestBasicDisplay_DrawAll(&statusAmpl);
}

void TestAmplDisplay_SetStatus(uint8_t index, bool status)
{
    if (index >= STATUS_COUNT) return;

    if (statusAmpl.items[index] != status) {
    	statusAmpl.items[index] = status;
        DrawStatusBox(index, &statusAmpl);
    }
}
