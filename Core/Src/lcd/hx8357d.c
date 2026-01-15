#include "hx8357d.h"
#include "lcd_color_rgb565.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

static void CS_LOW()  { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); }
static void CS_HIGH() { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); }
static void DC_CMD()  { HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET); }
static void DC_DATA() { HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); }
static void RST_LOW() { HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET); }
static void RST_HIGH(){ HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET); }
static void BLK_ON()  { HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET); }
//static void BLK_OFF() { HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET); }
static void hx8357_set_rotation(ScreenRotation_t rotation);
static void hx8357_send_cmd(uint8_t cmd);
static void hx8357_send_data(uint8_t data);
static void hx8357_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void hx8357_push_color(uint16_t color, uint32_t pixels);

volatile uint16_t LCD_HEIGHT = HX8357_TFTHEIGHT;
volatile uint16_t LCD_WIDTH	 = HX8357_TFTWIDTH;

static void hx8357_send_cmd(uint8_t cmd) {
    DC_CMD();
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

static void hx8357_send_data(uint8_t data) {
    DC_DATA();
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

static void hx8357_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	x1 = x0 + x1 - 1;
	y1 = y0 + y1 - 1;

	hx8357_send_cmd(HX8357_CASET);
	hx8357_send_data(x0 >> 8);
    hx8357_send_data(x0 & 0xFF);
    hx8357_send_data(x1 >> 8);
    hx8357_send_data(x1 & 0xFF);

    hx8357_send_cmd(HX8357_PASET);
    hx8357_send_data(y0 >> 8);
    hx8357_send_data(y0 & 0xFF);
    hx8357_send_data(y1 >> 8);
    hx8357_send_data(y1 & 0xFF);

    hx8357_send_cmd(HX8357_RAMWR);
}

static void hx8357_push_color(uint16_t color, uint32_t pixels)
{
    DC_DATA();
    CS_LOW();

    uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;
	uint32_t len = 0;

	uint8_t buffer[HX8357_BUFFER_SIZE];

	while (pixels > 0) {
		uint32_t chunk = (pixels > HX8357_BUFFER_HALF_SIZE) ? HX8357_BUFFER_HALF_SIZE : pixels;

		for (uint32_t j = 0; j < chunk; j++) {
			buffer[len++] = hi;
			buffer[len++] = lo;
		}

		HAL_SPI_Transmit(&hspi1, buffer, len, HAL_MAX_DELAY);

		pixels -= chunk;
		len = 0;
	}

    CS_HIGH();
}

static void hx8357_set_rotation(ScreenRotation_t rotation) {
	uint8_t data = 0;

	switch (rotation) {
		case SCREEN_VERTICAL_1:
			data = MADCTL_MX | MADCTL_MY | MADCTL_RGB;
			LCD_WIDTH = HX8357_TFTHEIGHT;
			LCD_HEIGHT = HX8357_TFTWIDTH;
			break;
		case SCREEN_HORIZONTAL_1:
			data = MADCTL_MV | MADCTL_MY | MADCTL_RGB;
			LCD_WIDTH = HX8357_TFTWIDTH;
			LCD_HEIGHT = HX8357_TFTHEIGHT;
			break;
		case SCREEN_VERTICAL_2:
			data = MADCTL_RGB;
			LCD_WIDTH = HX8357_TFTHEIGHT;
			LCD_HEIGHT = HX8357_TFTWIDTH;
			break;
		case SCREEN_HORIZONTAL_2:
			data = MADCTL_MX | MADCTL_MV | MADCTL_RGB;
			LCD_WIDTH = HX8357_TFTWIDTH;
			LCD_HEIGHT = HX8357_TFTHEIGHT;
			break;
		default:
		    return;
	}

	hx8357_send_cmd(HX8357_MADCTL);
	hx8357_send_data(data);
}

uint16_t hx8357_get_height(void)
{
	return LCD_HEIGHT;
}

uint16_t hx8357_get_width(void)
{
	return LCD_WIDTH;
}

void hx8357_reset(void)
{
    RST_LOW();
    HAL_Delay(20);
    RST_HIGH();
    HAL_Delay(150);
}

void hx8357_init(void)
{
    BLK_ON();
    hx8357_reset();

    hx8357_send_cmd(HX8357_SWRESET);
    HAL_Delay(100);

    hx8357_send_cmd(HX8357D_SETC);
    hx8357_send_data(0xFF);
    hx8357_send_data(0x83);
    hx8357_send_data(0x57);
    HAL_Delay(300);

    hx8357_send_cmd(HX8357_SETRGB);
    hx8357_send_data(0x80);
    hx8357_send_data(0x00);
    hx8357_send_data(0x06);
    hx8357_send_data(0x06);

    hx8357_send_cmd(HX8357D_SETCOM);
    hx8357_send_data(0x25);

    hx8357_send_cmd(HX8357_SETOSC);
    hx8357_send_data(0x68);
    //hx8357_send_data(0x01);

    hx8357_send_cmd(HX8357_SETPANEL);
    hx8357_send_data(0x05);

    hx8357_send_cmd(HX8357_SETPWR1);
    hx8357_send_data(0x00);
    hx8357_send_data(0x15);
    hx8357_send_data(0x1C);
    hx8357_send_data(0x1C);
    hx8357_send_data(0x83);
    hx8357_send_data(0xAA);

    hx8357_send_cmd(HX8357D_SETSTBA);
    hx8357_send_data(0x50);
    hx8357_send_data(0x50);
    hx8357_send_data(0x01);
    hx8357_send_data(0x3C);
    hx8357_send_data(0x1E);
    hx8357_send_data(0x08);

    hx8357_send_cmd(HX8357D_SETCYC);
    hx8357_send_data(0x02);
    hx8357_send_data(0x40);
    hx8357_send_data(0x00);
    hx8357_send_data(0x2A);
    hx8357_send_data(0x2A);
    hx8357_send_data(0x0D);
    hx8357_send_data(0x78);

    hx8357_send_cmd(HX8357D_SETGAMMA);
    hx8357_send_data(0x02); hx8357_send_data(0x0A); hx8357_send_data(0x11);
    hx8357_send_data(0x1d); hx8357_send_data(0x23); hx8357_send_data(0x35);
    hx8357_send_data(0x41); hx8357_send_data(0x4b); hx8357_send_data(0x4b);
    hx8357_send_data(0x42); hx8357_send_data(0x3A); hx8357_send_data(0x27);
    hx8357_send_data(0x1B); hx8357_send_data(0x08); hx8357_send_data(0x09);
    hx8357_send_data(0x03); hx8357_send_data(0x02); hx8357_send_data(0x0A);
    hx8357_send_data(0x11); hx8357_send_data(0x1d); hx8357_send_data(0x23);
    hx8357_send_data(0x35); hx8357_send_data(0x41); hx8357_send_data(0x4b);
    hx8357_send_data(0x4b); hx8357_send_data(0x42); hx8357_send_data(0x3A);
    hx8357_send_data(0x27); hx8357_send_data(0x1B); hx8357_send_data(0x08);
    hx8357_send_data(0x09); hx8357_send_data(0x03); hx8357_send_data(0x00);
    hx8357_send_data(0x01);

    hx8357_send_cmd(HX8357_COLMOD);
    hx8357_send_data(0x55);

    hx8357_set_rotation(3);

	hx8357_send_cmd(HX8357_INVON);

    hx8357_send_cmd(HX8357_TEON);
    hx8357_send_data(0x00);

    hx8357_send_cmd(HX8357_TEARLINE);
    hx8357_send_data(0x00);
    hx8357_send_data(0x02);

    hx8357_send_cmd(HX8357B_SETDISPMODE);
    hx8357_send_data(0x00);

    hx8357_send_cmd(HX8357_SLPOUT);
    HAL_Delay(150);

    hx8357_send_cmd(HX8357_DISPON);
    HAL_Delay(50);

	hx8357_fill_screen(COLOR_BLACK);
}

void hx8357_fill_screen(uint16_t color)
{
    hx8357_set_window(0, 0, HX8357_TFTWIDTH, HX8357_TFTHEIGHT);
    hx8357_push_color(color, (uint32_t)HX8357_TFTWIDTH * HX8357_TFTHEIGHT);
}

void hx8357_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    hx8357_set_window(x, y, w, h);
    hx8357_push_color(color, (uint32_t)w * h);
}

void hx8357_write_char(uint16_t x, uint16_t y, char ch, FontDef *font, uint16_t color, uint16_t bgcolor)
{
    uint32_t char_offset = (ch - 32) * font->height;
    uint32_t line_bytes = font->width * 2;
    uint8_t line_buffer[64];

    hx8357_set_window(x, y, font->width, font->height);

    DC_DATA();
    CS_LOW();

    uint8_t color_hi = color >> 8;
    uint8_t color_lo = color & 0xFF;
    uint8_t bg_hi    = bgcolor >> 8;
    uint8_t bg_lo    = bgcolor & 0xFF;

    for (uint16_t i = 0; i < font->height; i++)
    {
        uint16_t line = font->data[char_offset + i];
        uint32_t buf_pos = 0;

        for (uint16_t j = 0; j < font->width; j++)
        {
            if (line << j & 0x8000)
            {
                line_buffer[buf_pos++] = color_hi;
                line_buffer[buf_pos++] = color_lo;
            }
            else
            {
                line_buffer[buf_pos++] = bg_hi;
                line_buffer[buf_pos++] = bg_lo;
            }
        }

        HAL_SPI_Transmit(&hspi1, line_buffer, line_bytes, HAL_MAX_DELAY);
    }

    CS_HIGH();
}

void hx8357_write_string(uint16_t x, uint16_t y, const char* str, FontDef *font, uint16_t color, uint16_t bgcolor) {
    while(*str) {
        if(x + font->width > LCD_WIDTH) {
            x = 0;
            if(y + font->height > LCD_HEIGHT) {
                break;
            }
            y += font->height;
        }

        hx8357_write_char(x, y, *str, font, color, bgcolor);
        x += font->width;
        str++;
    }
}

void hx8357_write_alignedX_string(uint16_t y, const char* str, FontDef* font, uint16_t textColor, uint16_t bgColor, Alignment align)
{
	uint16_t textWidth = strlen(str) * font->width;
	uint16_t x = 0;

	switch (align) {
		case ALIGN_LEFT:
			x = 0;
			break;
		case ALIGN_CENTER:
			x = (LCD_WIDTH - textWidth) / 2;
			break;
		case ALIGN_RIGHT:
			x = LCD_WIDTH - textWidth;
			break;
		default:
			x = 0;
			break;
	}

	hx8357_write_string(x, y, str, font, textColor, bgColor);
}

void hx8357_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if (w == 0 || h == 0 || x > LCD_WIDTH || y > LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    hx8357_set_window(x, y, w, y);

	DC_DATA();
	CS_LOW();

    for (uint32_t i = 0; i < w * h; i++)
    {
		uint16_t color = data[i];
		uint8_t r = (color >> 11) & 0x1F;
		uint8_t g = (color >> 5)  & 0x3F;
		uint8_t b = color & 0x1F;

		uint8_t pixel[2];
		pixel[0] = r << 3 | g >> 3;
		pixel[0] = g << 3 | b;
		HAL_SPI_Transmit(&hspi1, pixel, 2, HAL_MAX_DELAY);
	}
	CS_HIGH();
}

//void hx8357_test_draw_rect(void)
//{
//	hx8357_draw_rect(0, HX8357_TFTHEIGHT-50, 50, 50, HX8357_BLUE);
//	hx8357_draw_rect((HX8357_TFTWIDTH/2)-25, 0, 50, 50, HX8357_GREEN);
//	hx8357_draw_rect((HX8357_TFTWIDTH/2)-25, (HX8357_TFTHEIGHT/2)-25, 50, 50, HX8357_WHITE);
//	hx8357_draw_rect((HX8357_TFTWIDTH/2)-25, HX8357_TFTHEIGHT-50, 50, 50, HX8357_YELLOW);
//	hx8357_draw_rect(HX8357_TFTWIDTH-50, 0, 50, 50, HX8357_CYAN);
//	hx8357_draw_rect(HX8357_TFTWIDTH-50, HX8357_TFTHEIGHT-50, 50, 50, HX8357_MAGENTA);
//}
