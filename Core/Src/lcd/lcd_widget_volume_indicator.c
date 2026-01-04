
#include "lcd_display.h"
#include "lcd_widget_volume_indicator.h"
#include <stdio.h>

VolumeIndicator_t volumeIndicator;
static char debug_msg[32];

const int valid_volume_levels_tmp[] = {
    80, 83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122
};

void draw_txt_volume(VolumeIndicator_t* ind)
{

	sprintf(debug_msg, "level:%d   Volume: %d dB", ind->level, valid_volume_levels_tmp[ind->level]);

	LCD_WriteStringAligned(200, debug_msg, &Font_11x18, COLOR_YELLOW, COLOR_BLACK, ALIGN_CENTER);
}


static void fill_single_bar2(VolumeIndicator_t* ind, uint8_t index, uint16_t color)
{
    uint16_t innerX = ind->x + ind->frameWidth;
    uint16_t innerY = ind->y + ind->frameWidth;
    uint16_t innerWidth = ind->width - 2 * ind->frameWidth;
    uint16_t innerHeight = ind->height - 2 * ind->frameWidth;

    uint16_t totalBarWidth = innerWidth - (ind->numBars - 1) * ind->barSpacing;
    uint16_t barWidth = totalBarWidth / ind->numBars;

    uint32_t barStart = innerX + index * (barWidth + ind->barSpacing);
    uint32_t barEnd = barStart + barWidth;

    LCD_FillRectangle(barStart, innerY, barEnd - barStart, innerHeight, color);

    // Отладка
    sprintf(debug_msg, "Vol: %d dB, Level: %d", valid_volume_levels_tmp[index], ind->level);
    LCD_WriteStringAligned(200, debug_msg, &Font_11x18, COLOR_YELLOW, COLOR_BLACK, ALIGN_CENTER);

    osDelay(3);
}

static void fill_single_bar(VolumeIndicator_t* ind, uint8_t index, uint16_t color)
{
    uint16_t innerX = ind->x + ind->frameWidth;
    uint16_t innerY = ind->y + ind->frameWidth;
    uint16_t innerWidth = ind->width - 2 * ind->frameWidth;
    uint16_t innerHeight = ind->height - 2 * ind->frameWidth;

    // Общая ширина пробелов
    uint16_t totalSpacing = (ind->numBars - 1) * ind->barSpacing;
    uint16_t totalBarArea = innerWidth - totalSpacing;

    // Базовая ширина полосы
    uint16_t baseBarWidth = totalBarArea / ind->numBars;
    uint16_t remainder = totalBarArea % ind->numBars;

    // Каждая полоса может быть на 1 пиксель шире, если "её очередь"
    uint16_t currentBarWidth = baseBarWidth + (index < remainder ? 1 : 0);

    // Позиция начала
    uint32_t barStart = innerX;
    for (uint8_t i = 0; i < index; i++) {
        uint16_t w = baseBarWidth + (i < remainder ? 1 : 0);
        barStart += w + ind->barSpacing;
    }

    LCD_FillRectangle(barStart, innerY, currentBarWidth, innerHeight, color);

    // Отладка
	sprintf(debug_msg, "Vol: %-3d dB, Level: %-3d", valid_volume_levels_tmp[index], ind->level);
	LCD_WriteStringAligned(200, debug_msg, &Font_11x18, COLOR_YELLOW, COLOR_BLACK, ALIGN_CENTER);

	osDelay(3);
}


void VolumeIndicator_Init(VolumeIndicator_t* indicator, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          uint8_t numBars, uint16_t barColor, uint16_t bgColor, uint16_t inactiveBgColor,
                          uint16_t frameColor, uint8_t frameWidth, uint8_t barSpacing)
{
    indicator->x = x;
    indicator->y = y;
    indicator->width = width;
    indicator->height = height;
    indicator->numBars = numBars;
    indicator->barColor = barColor;
    indicator->bgColor = bgColor;
    indicator->inactiveBgColor = inactiveBgColor;
    indicator->frameColor = frameColor;
    indicator->frameWidth = frameWidth;
    indicator->level = 1;
    indicator->barSpacing = barSpacing;
}

void VolumeIndicator_Draw(VolumeIndicator_t* indicator)
{
	uint16_t innerX = indicator->x + indicator->frameWidth;
	uint16_t innerY = indicator->y + indicator->frameWidth;
	uint16_t innerWidth = indicator->width - 2 * indicator->frameWidth;
	uint16_t innerHeight = indicator->height - 2 * indicator->frameWidth;

	LCD_DrawRect(indicator->x, indicator->y, indicator->width, indicator->height, indicator->frameColor);
	LCD_FillRectangle(innerX, innerY, innerWidth, innerHeight, indicator->inactiveBgColor);
	osDelay(3);


	for (uint8_t i = 0; i < indicator->level; i++) {
		fill_single_bar(indicator, i, indicator->barColor);
	}
	osDelay(3);
}

void VolumeIndicator_SetLevel(VolumeIndicator_t* indicator, uint8_t newLevel)
{
    if (newLevel > indicator->numBars) {
        newLevel = indicator->numBars;
    }

    uint8_t oldLevel = indicator->level;
//    if (newLevel == oldLevel) {
//        return;
//    }

    indicator->level = newLevel;

    uint16_t innerX = indicator->x + indicator->frameWidth;
    uint16_t innerY = indicator->y + indicator->frameWidth;
    uint16_t innerWidth = indicator->width - 2 * indicator->frameWidth;
    uint16_t innerHeight = indicator->height - 2 * indicator->frameWidth;
    uint16_t totalBarWidth = innerWidth - (indicator->numBars - 1) * indicator->barSpacing;

    if (newLevel > oldLevel) {
        
        for (uint8_t i = oldLevel; i < newLevel; i++) {
            uint32_t barStart = innerX + ((uint32_t)i * totalBarWidth) / indicator->numBars + i * indicator->barSpacing;
            uint32_t barEnd   = innerX + ((uint32_t)(i + 1) * totalBarWidth) / indicator->numBars + i * indicator->barSpacing;
            LCD_FillRectangle(barStart, innerY, barEnd - barStart, innerHeight, indicator->barColor);
            osDelay(3);
        }
    } else if (newLevel < oldLevel) {
        
        for (uint8_t i = newLevel; i < oldLevel; i++) {
            uint32_t barStart = innerX + ((uint32_t)i * totalBarWidth) / indicator->numBars + i * indicator->barSpacing;
            uint32_t barEnd   = innerX + ((uint32_t)(i + 1) * totalBarWidth) / indicator->numBars + i * indicator->barSpacing;
            LCD_FillRectangle(barStart, innerY, barEnd - barStart, innerHeight, indicator->inactiveBgColor);
            osDelay(3);
        }
    }
//    else if (newLevel == oldLevel && newLevel > 0) {
//
//        for (uint8_t i = 0; i < newLevel; i++) {
//            uint32_t barStart = innerX + ((uint32_t)i * totalBarWidth) / indicator->numBars + i * indicator->barSpacing;
//            uint32_t barEnd   = innerX + ((uint32_t)(i + 1) * totalBarWidth) / indicator->numBars + i * indicator->barSpacing;
//            LCD_FillRectangle(barStart, innerY, barEnd - barStart, innerHeight, indicator->barColor);
//            osDelay(3);
//        }
//    }
}

void VolumeIndicator_SetLevelSilent(VolumeIndicator_t* ind, uint8_t level)
{
    if (level > ind->numBars) level = ind->numBars;
    ind->level = level;
}

void VolumeIndicator_StepUp(VolumeIndicator_t* ind)
{
    uint8_t current_count = ind->level - 1;
    if (current_count >= NUM_VOLUME_BARS) return;

    uint8_t index = current_count;
    fill_single_bar(ind, index, ind->barColor);
    osDelay(3);
}

void VolumeIndicator_StepDown(VolumeIndicator_t* ind)
{
    uint8_t current_count = ind->level + 1;
    if (current_count == 0) return;

    uint8_t index = current_count - 1;
    fill_single_bar(ind, index, ind->inactiveBgColor);
    osDelay(3);
}


