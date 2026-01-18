
#ifndef SRC_LCD_LCD_WIDGET_VOLUME_INDICATOR_H_
#define SRC_LCD_LCD_WIDGET_VOLUME_INDICATOR_H_

#include <stdint.h>
#include "defines.h"

#define  VOLUME_BAR_Y				250
#define  VOLUME_BAR_W				411
#define  VOLUME_BAR_H				30
#define  VOLUME_BAR_SPACING_X 		6
#define  VOLUME_BAR_SPACING_Y 		8
#define  VOLUME_OUTLINE_SIZE 		1

typedef struct {
    uint16_t x;                   ///< X coordinate of the top-left corner on screen
    uint16_t y;                   ///< Y coordinate of the top-left corner on screen
    uint16_t width;               ///< Total width of the indicator in pixels
    uint16_t height;              ///< Total height of the indicator in pixels
    uint8_t numBars;              ///< Number of individual bars in the indicator (e.g., 6, 10)
    uint16_t barColor;            ///< Color of active (filled) bars
    uint16_t bgColor;             ///< Background color outside the inner area (rarely used directly)
    uint16_t inactiveBgColor;     ///< Color of inactive (empty) bar positions
    uint16_t frameColor;          ///< Color of the border frame around the indicator
    uint8_t level;                ///< Current fill level (number of active bars, 0 to numBars)
    uint8_t frameWidth;           ///< Width of the border frame in pixels
    uint8_t barSpacing;           ///< Spacing in pixels between individual bars
} VolumeIndicator_t;

extern VolumeIndicator_t volumeIndicator;

void VolumeIndicator_Draw();
void VolumeIndicator_SetLevel(uint8_t level);
void VolumeIndicator_SetLevelSilent(uint8_t level);

void VolumeIndicator_StepUp();
void VolumeIndicator_StepDown();

#endif /* SRC_LCD_LCD_WIDGET_VOLUME_INDICATOR_H_ */
