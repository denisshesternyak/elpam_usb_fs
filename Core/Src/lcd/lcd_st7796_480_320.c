
#include "defines.h"

#ifdef USE_LCD_ST7796

#include "lcd_st7796_480_320.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"
#include "lcd_color_rgb565.h"


#define PIXEL_CHUNK 16   // Optimal for most STM32
static uint8_t buffer_FillRect[PIXEL_CHUNK * 2];  // Each pixel = 2 bytes (RGB565)
static uint8_t data_char[2];

//static ScreenRotation_t lcd_rotation = ROTATE_90_DEG;
static ScreenRotation_t lcd_rotation = ROTATE_270_DEG;


#define ST7796_SPI_PORT hspi1

extern SPI_HandleTypeDef ST7796_SPI_PORT;


#define ST7796_DC_GPIO_Port     GPIOC
#define ST7796_DC_Pin           GPIO_PIN_9

#define ST7796_RES_GPIO_Port    GPIOB
#define ST7796_RES_Pin          GPIO_PIN_9

#define ST7796_CS_GPIO_Port     GPIOB
#define ST7796_CS_Pin           GPIO_PIN_8


#define ST7796_CASET    0x2A  
#define ST7796_RASET    0x2B 
#define ST7796_RAMWR    0x2C  

#define ST7796_MADCTL     0x36
#define ST7796_MADCTL_MY  0x80
#define ST7796_MADCTL_MX  0x40
#define ST7796_MADCTL_MV  0x20
#define ST7796_MADCTL_BGR 0x08  


#define ST7796_ROTATION 0x48  


static void ST7796_Select()
{
    HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_RESET);
}

void ST7796_Unselect(void) {
    HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_SET);
}

static void ST7796_Reset() {
	
    HAL_GPIO_WritePin(ST7796_RES_GPIO_Port, ST7796_RES_Pin, GPIO_PIN_RESET);
    osDelay(5);
    HAL_GPIO_WritePin(ST7796_RES_GPIO_Port, ST7796_RES_Pin, GPIO_PIN_SET);
}


static void ST7796_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7796_DC_GPIO_Port, ST7796_DC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ST7796_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_SET);
}


static void ST7796_WriteData(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ST7796_DC_GPIO_Port, ST7796_DC_Pin, GPIO_PIN_SET);
   
    while (buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&ST7796_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;
        buff_size -= chunk_size;
    }
    HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_SET);
}



static void ST7796_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // column address set
    ST7796_WriteCommand(0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ST7796_WriteData(data, sizeof(data));
    }

    // row address set
    ST7796_WriteCommand(0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ST7796_WriteData(data, sizeof(data));
    }

    // write to RAM
    ST7796_WriteCommand(0x2C); // RAMWR
}



void ST7796_WriteChar(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color, uint16_t bgcolor)
{
    if ((x + font->width > ST7796_GetWidth()) || (y + font->height > ST7796_GetHeight()))
        return;

    ST7796_SetAddressWindow(x, y, x + font->width - 1, y + font->height - 1);

    for (uint32_t i = 0; i < font->height; i++)
    {
        uint16_t b = font->data[(ch - 32) * font->height + i];

        for (uint32_t j = 0; j < font->width; j++)
        {
            if (b & (1 << (15 - j))) {

            	data_char[0] = color >> 8;      
				data_char[1] = color & 0xFF;    
				ST7796_WriteData(data_char, sizeof(data_char));
            } else {
            	data_char[0] = bgcolor >> 8;    
				data_char[1] = bgcolor & 0xFF;  
				ST7796_WriteData(data_char, sizeof(data_char));
            }
        }
    }
}

void ST7796_WriteChar_NoBG(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color)
{
    if ((x + font->width > ST7796_GetWidth()) || (y + font->height > ST7796_GetHeight()))
        return;

    uint8_t pixelData[2] = {color >> 8, color & 0xFF};

    for (uint32_t i = 0; i < font->height; i++)
    {
        uint16_t lineData = font->data[(ch - 32) * font->height + i];

        for (uint32_t j = 0; j < font->width; j++)
        {
            if (lineData & (1 << (15 - j)))
            {
                
                ST7796_SetAddressWindow(x + j, y + i, x + j, y + i);
                
                ST7796_WriteData(pixelData, sizeof(pixelData));
            }
            
        }
    }
}


void ST7796_WriteChar_NoBG1(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color)
{
    if ((x + font->width > ST7796_GetWidth()) || (y + font->height > ST7796_GetHeight()))
        return;

    ST7796_SetAddressWindow(x, y, x + font->width - 1, y + font->height - 1);

    uint16_t bgcolor = COLOR_YELLOW;

    for (uint32_t i = 0; i < font->height; i++)
        {
            uint16_t b = font->data[(ch - 32) * font->height + i];

            for (uint32_t j = 0; j < font->width; j++)
            {
                if (b & (1 << (15 - j))) {

                	data_char[0] = color >> 8;      
    				data_char[1] = color & 0xFF;    
    				ST7796_WriteData(data_char, sizeof(data_char));
                } else {
                	data_char[0] = bgcolor >> 8;    
    				data_char[1] = bgcolor & 0xFF;  
    				//ST7796_WriteData(data_char, sizeof(data_char));
    				ST7796_WriteData(NULL, 0);  
                }
            }
        }

}

void ST7796_FillRectangle_NoBUF(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if ((x >= ST7796_GetWidth()) || (y >= ST7796_GetHeight())) return;
    if (w == 0 || h == 0) return;

    if ((uint32_t)x + w > ST7796_GetWidth())  w = ST7796_GetWidth() - x;
    if ((uint32_t)y + h > ST7796_GetHeight()) h = ST7796_GetHeight() - y;

    ST7796_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint32_t totalPixels = (uint32_t)w * h;

    while (totalPixels--) {
        uint8_t data[2] = { color >> 8, color & 0xFF };
        ST7796_WriteData(data, 2);
    }
}

void ST7796_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if ((x >= ST7796_GetWidth()) || (y >= ST7796_GetHeight())) return;
    if ((x + w - 1) >= ST7796_GetWidth())  w = ST7796_GetWidth() - x;
    if ((y + h - 1) >= ST7796_GetHeight()) h = ST7796_GetHeight() - y;


    ST7796_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint32_t totalPixels = w * h;


    for (int i = 0; i < PIXEL_CHUNK; i++) {
    	buffer_FillRect[2 * i]     = color >> 8;      // MSB
    	buffer_FillRect[2 * i + 1] = color & 0xFF;    // LSB
    }

    while (totalPixels >= PIXEL_CHUNK) {
        ST7796_WriteData(buffer_FillRect, sizeof(buffer_FillRect));
        totalPixels -= PIXEL_CHUNK;
    }

    if (totalPixels > 0) {
        ST7796_WriteData(buffer_FillRect, totalPixels * 2);
    }
}

void ST7796_FillScreen(uint16_t color)
{
	ST7796_FillRectangle(0, 0, ST7796_GetWidth(), ST7796_GetHeight(), color);
}

void ST7796_InvertColors(bool invert)
{
	ST7796_Select();
	ST7796_WriteCommand(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
	ST7796_Unselect();
}

void ST7796_Init(void)
{
    ST7796_Unselect();
    ST7796_Select();

   
    ST7796_Reset();
    osDelay(150);  

   
    ST7796_WriteCommand(0xF0);
    {
        uint8_t data[] = { 0xC3 };
        ST7796_WriteData(data, sizeof(data));
    }

    ST7796_WriteCommand(0xF0);
    {
        uint8_t data[] = { 0x96 };
        ST7796_WriteData(data, sizeof(data));
    }

    
    ST7796_WriteCommand(0x36);
    {
        
        uint8_t data[] = { 0x48 };  // MX + BGR
        
        ST7796_WriteData(data, sizeof(data));
    }

    // Pixel Format Set (16 бит: 5-6-5 RGB)
    ST7796_WriteCommand(0x3A);
    {
        uint8_t data[] = { 0x05 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Interface Mode Control
    ST7796_WriteCommand(0xB0);
    {
        uint8_t data[] = { 0x80 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Display Function Control
    ST7796_WriteCommand(0xB6);
    {
        uint8_t data[] = { 0x00, 0x02 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Porch Setting (Front / Back porch)
    ST7796_WriteCommand(0xB5);
    {
        uint8_t data[] = { 0x02, 0x03, 0x00, 0x04 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Frame Rate Control
    ST7796_WriteCommand(0xB1);
    {
        uint8_t data[] = { 0x80, 0x10 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Display Inversion Control 
    ST7796_WriteCommand(0xB4);
    {
        uint8_t data[] = { 0x00 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Entry Mode Set 
    ST7796_WriteCommand(0xB7);
    {
        uint8_t data[] = { 0xC6 };
        ST7796_WriteData(data, sizeof(data));
    }

    // VCOM Control
    ST7796_WriteCommand(0xC5);
    {
        uint8_t data[] = { 0x1C };
        ST7796_WriteData(data, sizeof(data));
    }

    // Set LCM Control
    ST7796_WriteCommand(0xE4);
    {
        uint8_t data[] = { 0x31 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Power Control 2
    ST7796_WriteCommand(0xE8);
    {
        uint8_t data[] = {
            0x40, 0x8A, 0x00, 0x00,
            0x29, 0x19, 0xA5, 0x33
        };
        ST7796_WriteData(data, sizeof(data));
    }

    // Panel Driving Setting
    ST7796_WriteCommand(0xC2);
    ST7796_WriteCommand(0xA7); 

    // Positive Gamma Control
    ST7796_WriteCommand(0xE0);
    {
        uint8_t data[] = {
            0xF0, 0x09, 0x13, 0x12, 0x12,
            0x2B, 0x3C, 0x44, 0x4B,
            0x1B, 0x18, 0x17, 0x1D, 0x21
        };
        ST7796_WriteData(data, sizeof(data));
    }

    // Negative Gamma Control
    ST7796_WriteCommand(0xE1);
    {
        uint8_t data[] = {
            0xF0, 0x09, 0x13, 0x0C, 0x0D,
            0x27, 0x3B, 0x44, 0x4D,
            0x0B, 0x17, 0x17, 0x1D, 0x21
        };
        ST7796_WriteData(data, sizeof(data));
    }

    
    ST7796_WriteCommand(0xF0);
    {
        uint8_t data[] = { 0x3C };
        ST7796_WriteData(data, sizeof(data));
    }

    ST7796_WriteCommand(0xF0);
    {
        uint8_t data[] = { 0x69 };
        ST7796_WriteData(data, sizeof(data));
    }

    // Normal Display Mode On
    ST7796_WriteCommand(0x13);

    // Sleep Out
    ST7796_WriteCommand(0x11);

    // Display ON
    ST7796_WriteCommand(0x29);

    osDelay(20);

    ST7796_FillScreen(COLOR_BLACK);
    ST7796_SetRotation(lcd_rotation);
    ST7796_Unselect();
}

uint16_t ST7796_GetWidth(void) {
    return (lcd_rotation == ROTATE_0_DEG || lcd_rotation == ROTATE_180_DEG) ? 320 : 480;
}

uint16_t ST7796_GetHeight(void) {
    return (lcd_rotation == ROTATE_0_DEG || lcd_rotation == ROTATE_180_DEG) ? 480 : 320;
}


void ST7796_SetRotation(ScreenRotation_t rotation)
{
	lcd_rotation = rotation;

    uint8_t madctl = 0;

    switch (rotation)
    {
        case ROTATE_0_DEG:
            madctl = ST7796_MADCTL_MX | ST7796_MADCTL_BGR;
            break;

        case ROTATE_90_DEG:
            madctl = ST7796_MADCTL_MV | ST7796_MADCTL_BGR;
            break;

        case ROTATE_180_DEG:
            madctl = ST7796_MADCTL_MY | ST7796_MADCTL_BGR;
            break;

        case ROTATE_270_DEG:
            madctl = ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR;
            break;

        default:
            return;
    }


    
    ST7796_WriteCommand(ST7796_MADCTL);
    ST7796_WriteData(&madctl, 1);
}

void ST7796_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
	 if ((x >= ST7796_GetWidth()) || (y >= ST7796_GetHeight())) return;

	 ST7796_SetAddressWindow(x, y, x, y);  

	 uint8_t data[] = { color >> 8, color & 0xFF };
	 ST7796_WriteData(data, sizeof(data));
}

void ST7796_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
	if (w == 0 || h == 0) return;
	if((x >= ST7796_GetWidth()) || (y >= ST7796_GetHeight())) return;
    if((x + w - 1) >= ST7796_GetWidth()) return;
    if((y + h - 1) >= ST7796_GetHeight()) return;

    ST7796_SetAddressWindow(x, y, (x + w - 1), (y + h - 1));

    for (uint32_t i = 0; i < w * h; i++)
    {
	   uint16_t color = data[i];
	   uint8_t high = color >> 8;
	   uint8_t low  = color & 0xFF;
	   ST7796_WriteData(&high, 1); 
	   ST7796_WriteData(&low, 1);
    }
}

void ST7796_WriteString_NoBG(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color)
{
	if(str == NULL || *str == '\0' || x >= ST7796_GetWidth() || y >= ST7796_GetHeight())
	{
	   return;
	}


	while(*str)
	{
		if(x + font->width >= ST7796_GetWidth())
		{
			x = 0;
			y += font->height;

			if(y + font->height >= ST7796_GetHeight())
			{
				break;
			}


			if(*str == '\0') {
				break;
			}

		}

		ST7796_WriteChar_NoBG(x, y, *str, font, color);

		x += font->width;
		str++;
	}
}

void ST7796_WriteString(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor)
{
	if(str == NULL || *str == '\0' || x >= ST7796_GetWidth() || y >= ST7796_GetHeight())
	{
	   return;
	}

    while (*str)
    {
        if (x + font->width >= ST7796_GetWidth())
        {
            x = 0;
            y += font->height;

            if (y + font->height >= ST7796_GetHeight())
            {
                break;
            }


			if(*str == '\0') {
				break;
			}

//            if(*str == ' ') {
//                // skip spaces in the beginning of the new line
//                str++;
//                continue;
//            }
        }

        ST7796_WriteChar(x, y, *str, font, color, bgcolor);

        x += font->width;
        str++;
    }
}


#endif //USE_LCD_ST7796
