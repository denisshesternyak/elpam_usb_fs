
#include "lcd_widget_progress_bar.h"
#include "hx8357d.h"
#include <string.h>
#include <stdio.h>

static uint8_t lastProgress = 255;
static bool borderDrawn = false;

void MenuDrawProgress(uint8_t progressPercent)
{
    if (progressPercent == 0) borderDrawn = false;
    if (progressPercent > 100) progressPercent = 100;
    //if (progressPercent == lastProgress) return;

    lastProgress = progressPercent;

    char text[8];
    const uint16_t x = (hx8357_get_width() - PROGRESS_BAR_W) / 2;
    const uint16_t y = PROGRESS_BAR_Y;
    const uint16_t width = PROGRESS_BAR_W;
    const uint16_t height = PROGRESS_BAR_H;

    const uint16_t outline = 1;
    const uint16_t innerX = x + outline;
    const uint16_t innerY = y + outline;
    const uint16_t innerW = width - (outline*2);
    const uint16_t innerH = height - (outline*2);
	const uint16_t bar_w = (innerW * progressPercent) / 100;

    if (!borderDrawn) {
	    hx8357_draw_rect(x, y, width, height, outline, PROGRESS_BORDER_COLOR, PROGRESS_BACKGROUND_COLOR);
        borderDrawn = true;
    }

    snprintf(text, sizeof(text), "%d%%", progressPercent);

    FontDef* font = &Font_11x18;
    uint16_t textW = strlen(text) * font->width;
    uint16_t textH = font->height;

    uint16_t textX = x + (width - textW) / 2;
    uint16_t textY = y + (height - textH) / 2;

    hx8357_fill_rect(innerX, innerY, bar_w, innerH, PROGRESS_FILL_COLOR);

    hx8357_write_string(textX, textY, text, font, PROGRESS_BORDER_COLOR, PROGRESS_BACKGROUND_COLOR);
}

void MenuResetProgressBar(void)
{
    lastProgress = 255;
    borderDrawn = false;
}

