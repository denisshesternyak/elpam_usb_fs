/*
 * lcd_color_rgb666.h
 *
 *  Created on: Jun 20, 2025
 *      Author: roman
 */

#ifndef INC_LCD_LCD_COLOR_RGB666_H_
#define INC_LCD_LCD_COLOR_RGB666_H_

#include <stdint.h>

// =================================================
//           RGB666 Color Definitions (18-bit)
// =================================================
// Format: 0xRRGGBB (but only 6 bits per channel)
// Usage: uint32_t color = COLOR_RED;
// =================================================

#define RGB666(r,g,b)   (((uint32_t)(r & 0x3F) << 16) | ((uint32_t)(g & 0x3F) << 8) | (b & 0x3F))

// === Basic Colors ===
#define COLOR_BLACK     RGB666(0,   0,   0)
#define COLOR_WHITE     RGB666(63,  63,  63)
#define COLOR_RED       RGB666(63,  0,   0)
#define COLOR_GREEN     RGB666(0,   63,  0)
#define COLOR_BLUE      RGB666(0,   0,   63)
#define COLOR_YELLOW    RGB666(63,  63,  0)
#define COLOR_CYAN      RGB666(0,   63,  63)
#define COLOR_MAGENTA   RGB666(63,  0,   63)

// === Grayscale ===
#define COLOR_GRAY       RGB666(32, 32, 32)
#define COLOR_LIGHTGRAY  RGB666(48, 48, 48)
#define COLOR_DARKGRAY   RGB666(16, 16, 16)

// === Extended Colors ===
#define COLOR_ORANGE    RGB666(63, 32, 0)
#define COLOR_PURPLE    RGB666(32, 0, 32)
#define COLOR_PINK      RGB666(63, 16, 32)
#define COLOR_LIME      RGB666(32, 63, 0)
#define COLOR_TEAL      RGB666(0, 32, 32)
#define COLOR_NAVY      RGB666(0, 0, 32)

// === Special Colors ===
#define COLOR_OLIVE     RGB666(32, 32, 0)
#define COLOR_MAROON    RGB666(32, 0, 0)

// =================================================
//   Extract components (0..63 each)
// =================================================
#define RGB666_R(c)   (((c) >> 16) & 0x3F)
#define RGB666_G(c)   (((c) >> 8)  & 0x3F)
#define RGB666_B(c)   ((c) & 0x3F)

// =================================================
//   Convert RGB666 -> 3 bytes for ILI9488
// =================================================
static inline void RGB666_to_bytes(uint32_t color, uint8_t out[3]) {
    out[0] = RGB666_R(color) << 2; // расширяем 6 бит в 8
    out[1] = RGB666_G(color) << 2;
    out[2] = RGB666_B(color) << 2;
}

#endif /* INC_LCD_LCD_COLOR_RGB666_H_ */
