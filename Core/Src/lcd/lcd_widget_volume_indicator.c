#include "lcd_widget_volume_indicator.h"
#include "hx8357d.h"
#include <stdio.h>

VolumeIndicator_t volumeIndicator;
static char debug_msg[32];

const int valid_volume_levels_tmp[] = {
    80, 83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116, 119, 122
};

void draw_txt_volume()
{
	sprintf(debug_msg, "level:%d   Volume: %d dB", volumeIndicator.level, valid_volume_levels_tmp[volumeIndicator.level]);

	hx8357_write_alignedX_string(0, 200, debug_msg, &Font_11x18, COLOR_YELLOW, COLOR_BLACK, ALIGN_CENTER);
}

static void fill_single_bar(uint8_t index)
{
	const uint16_t barW = ((VOLUME_BAR_W - VOLUME_BAR_SPACING_X) / NUM_VOLUME_BARS);
	const uint16_t barH = VOLUME_BAR_H - (VOLUME_BAR_SPACING_Y * 2);
	const uint16_t barX = volumeIndicator.x + (index * barW) + VOLUME_BAR_SPACING_X;
	const uint16_t barY = VOLUME_BAR_Y + VOLUME_BAR_SPACING_Y;

	hx8357_fill_rect(barX, barY, barW - VOLUME_BAR_SPACING_X, barH, COLOR_BLUE);

	sprintf(debug_msg, "Vol: %-3d dB, Level: %-3d", valid_volume_levels_tmp[index], volumeIndicator.level);
	hx8357_write_alignedX_string(0, 200, debug_msg, &Font_11x18, COLOR_YELLOW, COLOR_BLACK, ALIGN_CENTER);

//	osDelay(3);
}


//void VolumeIndicator_Init(VolumeIndicator_t* indicator, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
//                          uint8_t numBars, uint16_t barColor, uint16_t bgColor, uint16_t inactiveBgColor,
//                          uint16_t frameColor, uint8_t frameWidth, uint8_t barSpacing)
//{
//    indicator->x = x;
//    indicator->y = y;
//    indicator->width = width;
//    indicator->height = height;
//    indicator->numBars = numBars;
//    indicator->barColor = barColor;
//    indicator->bgColor = bgColor;
//    indicator->inactiveBgColor = inactiveBgColor;
//    indicator->frameColor = frameColor;
//    indicator->frameWidth = frameWidth;
//    indicator->level = 1;
//    indicator->barSpacing = barSpacing;
//}

//VolumeIndicator_Init(
//        &volumeIndicator,
//        30, 250, 420, 30,
//        NUM_VOLUME_BARS,      //
//        COLOR_DARKGRAY,       // barColor
//        0x00FF,               // bgColor
//        0xC618,               // inactiveBgColor
//        0x0D00,               // frameColor
//        1,                    // frameWidth
//        1                     // barSpacing
//    );

void VolumeIndicator_Draw()
{
	const uint16_t x = (hx8357_get_width() - VOLUME_BAR_W) / 2;
	hx8357_draw_rect(x, VOLUME_BAR_Y, VOLUME_BAR_W, VOLUME_BAR_H, VOLUME_OUTLINE_SIZE, COLOR_WHITE, COLOR_BLACK);

	volumeIndicator.x = x;
	volumeIndicator.y = VOLUME_BAR_Y;

//	osDelay(3);

	for (uint8_t i = 0; i < 15; i++) {
		fill_single_bar(i);
	}
//	osDelay(3);
}

void VolumeIndicator_SetLevel(uint8_t newLevel)
{
    if (newLevel > NUM_VOLUME_BARS) {
        newLevel = NUM_VOLUME_BARS;
    }

    uint8_t oldLevel = volumeIndicator.level;
//    if (newLevel == oldLevel) {
//        return;
//    }

    volumeIndicator.level = newLevel;

    uint16_t innerX = volumeIndicator.x + volumeIndicator.frameWidth;
    uint16_t innerY = volumeIndicator.y + volumeIndicator.frameWidth;
    uint16_t innerWidth = volumeIndicator.width - 2 * volumeIndicator.frameWidth;
    uint16_t innerHeight = volumeIndicator.height - 2 * volumeIndicator.frameWidth;
    uint16_t totalBarWidth = innerWidth - (NUM_VOLUME_BARS - 1) * volumeIndicator.barSpacing;

    if (newLevel > oldLevel) {
        
        for (uint8_t i = oldLevel; i < newLevel; i++) {
            uint32_t barStart = innerX + ((uint32_t)i * totalBarWidth) / NUM_VOLUME_BARS + i * volumeIndicator.barSpacing;
            uint32_t barEnd   = innerX + ((uint32_t)(i + 1) * totalBarWidth) / NUM_VOLUME_BARS + i * volumeIndicator.barSpacing;
            hx8357_fill_rect(barStart, innerY, barEnd - barStart, innerHeight, COLOR_DARKGRAY);
//            osDelay(3);
        }
    } else if (newLevel < oldLevel) {
        
        for (uint8_t i = newLevel; i < oldLevel; i++) {
            uint32_t barStart = innerX + ((uint32_t)i * totalBarWidth) / NUM_VOLUME_BARS + i * volumeIndicator.barSpacing;
            uint32_t barEnd   = innerX + ((uint32_t)(i + 1) * totalBarWidth) / NUM_VOLUME_BARS + i * volumeIndicator.barSpacing;
            hx8357_fill_rect(barStart, innerY, barEnd - barStart, innerHeight, COLOR_GRAY);
//            osDelay(3);
        }
    }
//    else if (newLevel == oldLevel && newLevel > 0) {
//
//        for (uint8_t i = 0; i < newLevel; i++) {
//            uint32_t barStart = innerX + ((uint32_t)i * totalBarWidth) / NUM_VOLUME_BARS + i * volumeIndicator.barSpacing;
//            uint32_t barEnd   = innerX + ((uint32_t)(i + 1) * totalBarWidth) / NUM_VOLUME_BARS + i * volumeIndicator.barSpacing;
//            hx8357_fill_rect(barStart, innerY, barEnd - barStart, innerHeight, COLOR_DARKGRAY);
//            osDelay(3);
//        }
//    }
}

void VolumeIndicator_SetLevelSilent(uint8_t level)
{
    if (level > NUM_VOLUME_BARS) level = NUM_VOLUME_BARS;
    volumeIndicator.level = level;
}

void VolumeIndicator_StepUp()
{
    uint8_t current_count = volumeIndicator.level - 1;
    if (current_count >= NUM_VOLUME_BARS) return;

    uint8_t index = current_count;
    fill_single_bar(index);
//    osDelay(3);
}

void VolumeIndicator_StepDown()
{
    uint8_t current_count = volumeIndicator.level + 1;
    if (current_count == 0) return;

    uint8_t index = current_count - 1;
    fill_single_bar(index);
//    osDelay(3);
}


