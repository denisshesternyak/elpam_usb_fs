


#include "lcd_widget_report_indicator.h"
#include "lcd_display.h"
#include "lcd_color_rgb565.h"
#include <string.h>

#define BOX_SIZE       30	
#define BOX_SPACING_X  16  
#define BOX_SPACING_Y  40	

#define TEXT_COLOR     		COLOR_BLACK	 
#define TEXT_NUM_COLOR     	COLOR_WHITE	 
#define TEXT_UNDER_COLOR 	COLOR_WHITE	 
#define BOX_COLOR      		COLOR_WHITE	 
#define BG_COLOR       		COLOR_BLACK  
#define COLOR_OK       		COLOR_GREEN	 
#define COLOR_FAIL     		COLOR_RED	 


#define REPORT_INDICATOR_MAX 24 


typedef struct {
	const char* num;			
    const char* label;   
    const char* symbol;  
    bool status;         
} ReportIndicator;


static ReportIndicator reportIndicators[] = {
    // line 1
	{"1",  "Oper",           "H",  true},
    {"2",  "Oper",           NULL, false},
	{NULL, NULL,              NULL, false}, 
	{NULL, NULL,              NULL, false}, 
	{NULL, NULL,              NULL, false}, 
	{NULL, NULL,              NULL, false}, 
	{NULL, NULL,              NULL, false}, 
	{NULL, NULL,              NULL, false}, 
	{NULL, NULL,              NULL, false}, 
	{NULL, NULL,              NULL, false}, 
 // line 2
	{"3",  "Amp1/Drv1", "1", true},
    {"4",  "Amp2/Drv2", "1", true},
    {"5",  "Amp3/Drv3", "1", true},
    {"6",  "Amp4/Drv4", "1", true},
    {"7",  "Amp5/Drv5", "1", true},
    {"8",  "Amp6/Drv6", "1", true},
    {"9",  "Amp7/Drv7", "1", true},
    {"10", "Amp8/Drv8", "0", false},
    {"11", "Amp9/Drv9", "1", true},
    {"12", "Amp10/Drv10","1",true},
    {NULL, NULL,       NULL, false},
    {"14", "Vol",   "1", true},
    {"15", "Tone",   "1", true},
    {"16", "P.S.",   "1", true},
    {"17", "Oper",  "0", false},
    {"18", "Batt",   "1", true},
    {"19", "Charg", "1", true},
    {"20", " AC",     "1", true},
    {"21", "Flood",  "1", true},
    {"22", "Door",   "1", true},
};

static const uint8_t reportIndicatorCount = sizeof(reportIndicators) / sizeof(reportIndicators[0]);

void DrawOneIndicator(uint8_t index, uint16_t baseX, uint16_t baseY)
{
    if (index >= reportIndicatorCount)
        return;

    const ReportIndicator* ind = &reportIndicators[index];

    if (ind->num == NULL && ind->label == NULL && ind->symbol == NULL)
        return;

    const uint8_t itemsPerRow = 10;
    uint8_t row = index / itemsPerRow;
    uint8_t col = index % itemsPerRow;

    uint16_t x = baseX + col * (BOX_SIZE + BOX_SPACING_X);
    uint16_t y = baseY + row * (BOX_SIZE + BOX_SPACING_Y);


   if (ind->num)
   {
	   uint8_t numLen = strlen(ind->num);
	   int16_t numX = x + (BOX_SIZE - numLen * Font_7x10.width) / 2;
	   int16_t numY = (y >= 10) ? (y - 10) : y;

	   if (numX >= 0 && numX + numLen * Font_7x10.width < LCD_GetWidth())
	   {
		   LCD_WriteString(numX, numY, ind->num, &Font_7x10, TEXT_NUM_COLOR, BG_COLOR);
	   }
   }

  
    LCD_DrawRect(x, y, BOX_SIZE, BOX_SIZE, BOX_COLOR);
    uint16_t fillColor = ind->status ? COLOR_OK : COLOR_FAIL;
    LCD_FillRectangle(x + 1, y + 1, BOX_SIZE - 2, BOX_SIZE - 2, fillColor);

   
    if (ind->symbol) {
        uint8_t len = strlen(ind->symbol);
        int16_t symX = x + (BOX_SIZE - len * Font_7x10.width) / 2;
        int16_t symY = y + (BOX_SIZE - Font_7x10.height) / 2;
        LCD_WriteString(symX, symY, ind->symbol, &Font_7x10, TEXT_COLOR, fillColor);
    }

   
    if (ind->label)
    {
        const char* slash = strchr(ind->label, '/');

        if (slash)
        {
        	size_t len = slash - ind->label;
			size_t maxLen = (LCD_GetWidth() - (x + 2)) / Font_7x10.width;
			if (len > maxLen) len = maxLen;
			LCD_WriteNString(x + 2, y + BOX_SIZE + 2, ind->label, len, &Font_7x10, TEXT_UNDER_COLOR, BG_COLOR);
			LCD_WriteString(x + 2 , (y + BOX_SIZE + 2 + Font_7x10.height), slash + 1, &Font_7x10, TEXT_UNDER_COLOR, BG_COLOR);
        } else {
            LCD_WriteString(x + 2, y + BOX_SIZE + 2, ind->label, &Font_7x10, TEXT_UNDER_COLOR, BG_COLOR);
        }
    }
}

void ReportIndicator_DrawAll(uint16_t baseX, uint16_t baseY)
{
    for (uint8_t i = 0; i < reportIndicatorCount; ++i) {
        DrawOneIndicator(i, baseX, baseY);
        osDelay(3);
    }
}
