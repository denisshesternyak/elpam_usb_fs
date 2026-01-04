
#ifndef SRC_LCD_LCD_WIDGET_VOLUME_INDICATOR_H_
#define SRC_LCD_LCD_WIDGET_VOLUME_INDICATOR_H_

#include <stdint.h>

/**
 * @brief Configuration and state structure for a volume bar indicator widget.
 *
 * This structure holds both the visual properties and current state of a
 * horizontal bar-style volume indicator. It supports a fixed number of bars,
 * customizable colors, spacing, and frame, and tracks the current volume level.
 */
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

void VolumeIndicator_Init(VolumeIndicator_t* indicator, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                         uint8_t numBars, uint16_t barColor, uint16_t bgColor, uint16_t inactiveBgColor,
                         uint16_t frameColor, uint8_t frameWidth, uint8_t barSpacing);

void VolumeIndicator_Draw(VolumeIndicator_t* indicator);
void VolumeIndicator_SetLevel(VolumeIndicator_t* indicator, uint8_t level);
void VolumeIndicator_SetLevelSilent(VolumeIndicator_t* ind, uint8_t level);

void VolumeIndicator_StepUp(VolumeIndicator_t* ind);
void VolumeIndicator_StepDown(VolumeIndicator_t* ind);

#endif /* SRC_LCD_LCD_WIDGET_VOLUME_INDICATOR_H_ */
