#ifndef INC_LCD_LCD_WIDGET_TEST_BASIC_INDICATOR_H_
#define INC_LCD_LCD_WIDGET_TEST_BASIC_INDICATOR_H_

#include "hx8357d.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define STATUS_TEXT_SIZE		  64
#define STATUS_COUNT              10
#define STATUS_BOX_SIZE           32
#define STATUS_BOX_SPACING_X      13
#define STATUS_BOX_SPACING_Y      35
#define STATUS_LABEL_OFFSET_Y     15

typedef struct {
	uint16_t baseX;
	uint16_t baseY;
	char label[STATUS_TEXT_SIZE];
	bool items[STATUS_COUNT];
}StatusBasic;

void DrawStatusBox(uint8_t index, StatusBasic *status);
void TestBasicDisplay_DrawAll(StatusBasic *status);

#endif /* INC_LCD_LCD_WIDGET_TEST_BASIC_INDICATOR_H_ */
