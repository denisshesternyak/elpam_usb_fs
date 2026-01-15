

#ifndef INC_LCD_LCD_WIDGET_PROGRESS_BAR_H_
#define INC_LCD_LCD_WIDGET_PROGRESS_BAR_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROGRESS_BAR_H   	20
#define PROGRESS_BAR_W      350
#define PROGRESS_BAR_Y      200

#define PROGRESS_BORDER_COLOR     0xFFFF
#define PROGRESS_FILL_COLOR       0x07E0
#define PROGRESS_BACKGROUND_COLOR 0x0000
#define PROGRESS_TEXT_COLOR       0xFFFF

void MenuDrawProgress(uint8_t progressPercent);
void MenuResetProgressBar(void);

#endif /* INC_LCD_LCD_WIDGET_PROGRESS_BAR_H_ */
