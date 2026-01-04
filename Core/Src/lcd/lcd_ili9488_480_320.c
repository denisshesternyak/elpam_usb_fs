
#include "defines.h"


/////////////
//         ILI9488_SendCommand(0x3A);
//         ILI9488_SendData(0x55);  // 16-bit color
//
//         ILI9488 according to the datasheet:
//
//             0x55 = 16-bit (RGB565)
//             0x66 = 18-bit (RGB666)
//
//  ⚠️ But! Via SPI, many Chinese ILI9488 modules expect exactly 18 bits (3 bytes per pixel),
//    and at 0x55 the screen remains white.
//

#ifdef USE_LCD_ILI_9488

#include "lcd_ili9488_480_320.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"

#include "lcd_color_rgb565.h"

//#include "lcd_color_rgb666.h"   // only  18bit

extern SPI_HandleTypeDef hspi1;

//definition PINS
/*
 * CS       - PB8
 * DC       - PC9
 * RESET    - PB9
 * SPI_SCK  - PA5 (SPI1)
 * SPI_MOSI - PA7 (SPI1)
 * LED      - PC8
 */


#define LCD_CS_GPIO_Port    GPIOB
#define LCD_CS_Pin          GPIO_PIN_8

#define LCD_DC_GPIO_Port    GPIOC
#define LCD_DC_Pin          GPIO_PIN_9

#define LCD_RESET_GPIO_Port GPIOB
#define LCD_RESET_Pin       GPIO_PIN_9

#define PIXEL_CHUNK 64   // Optimal for most STM32

static ScreenRotation_t lcd_rotation = ROTATE_90_DEG;
static uint8_t ili9488_fill_buffer[PIXEL_CHUNK * 3]; // Each pixel = 3 bytes (RGB666)

typedef struct {
    uint8_t r;  // 6 бит
    uint8_t g;  // 6 бит
    uint8_t b;  // 6 бит
} rgb666_t;

rgb666_t rgb565_to_rgb666_struct(uint16_t rgb565) {
    uint8_t r5 = (rgb565 >> 11) & 0x1F;
    uint8_t g6 = (rgb565 >>  5) & 0x3F;
    uint8_t b5 = rgb565 & 0x1F;

    rgb666_t c;
    c.r = (r5 << 1) | (r5 >> 4);
    c.g = g6;
    c.b = (b5 << 1) | (b5 >> 4);

    return c;
}

static void ILI9488_Select(void) {
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}

static void ILI9488_Unselect(void) {
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void ILI9488_DC_Command(void) {
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
}

static void ILI9488_DC_Data(void) {
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
}

static void ILI9488_SendCommand(uint8_t cmd) {
    ILI9488_Select();
    ILI9488_DC_Command();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    ILI9488_Unselect();
}

static void ILI9488_SendData(uint8_t data) {
    ILI9488_Select();
    ILI9488_DC_Data();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    ILI9488_Unselect();
}

// static void ILI9488_SendDataArray(const uint8_t* data, size_t length) {
//     ILI9488_Select();
//     ILI9488_DC_Data();
//     HAL_SPI_Transmit(&hspi1, (uint8_t*)data, length, HAL_MAX_DELAY);
//     ILI9488_Unselect();
// }

static void ILI9488_SendDataArray(const uint8_t* buff, size_t buff_size) {
    //HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(ST7796_DC_GPIO_Port, ST7796_DC_Pin, GPIO_PIN_SET);
   ILI9488_Select();
   ILI9488_DC_Data();

    while (buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&hspi1, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;
        buff_size -= chunk_size;
    }
    //HAL_GPIO_WritePin(ST7796_CS_GPIO_Port, ST7796_CS_Pin, GPIO_PIN_SET);
    ILI9488_Unselect();
}


static void ILI9488_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) 
{
    ILI9488_SendCommand(0x2A); //Column Address Set 
    {
    	uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
    	ILI9488_SendDataArray(data, sizeof(data));
	}
    
    ILI9488_SendCommand(0x2B);
    {
    	uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
    	ILI9488_SendDataArray(data, sizeof(data));
    }
    ILI9488_SendCommand(0x2C);
}

void ILI9488_Reset(void)
{
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
    osDelay(5);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
    
}

void ILI9488_Init(void) {
    ILI9488_Unselect();
    ILI9488_Select();


    ILI9488_Reset();
    osDelay(150); 


    osDelay(105);
    ILI9488_SendCommand(0x01);
    osDelay(5);


    // Normal Display Mode On
    ILI9488_SendCommand(0x13);



    // Positive Gamma Control
     ILI9488_SendCommand(0xE0);
     ILI9488_SendData(0x00);
     ILI9488_SendData(0x03);
     ILI9488_SendData(0x09);
     ILI9488_SendData(0x08);
     ILI9488_SendData(0x16);
     ILI9488_SendData(0x0A);
     ILI9488_SendData(0x3F);
     ILI9488_SendData(0x78);
     ILI9488_SendData(0x4C);
     ILI9488_SendData(0x09);
     ILI9488_SendData(0x0A);
     ILI9488_SendData(0x08);
     ILI9488_SendData(0x16);
     ILI9488_SendData(0x1A);
     ILI9488_SendData(0x0F);


     // Negative Gamma Control
	ILI9488_SendCommand(0XE1);
	ILI9488_SendData(0x00);
	ILI9488_SendData(0x16);
	ILI9488_SendData(0x19);
	ILI9488_SendData(0x03);
	ILI9488_SendData(0x0F);
	ILI9488_SendData(0x05);
	ILI9488_SendData(0x32);
	ILI9488_SendData(0x45);
	ILI9488_SendData(0x46);
	ILI9488_SendData(0x04);
	ILI9488_SendData(0x0E);
	ILI9488_SendData(0x0D);
	ILI9488_SendData(0x35);
	ILI9488_SendData(0x37);
	ILI9488_SendData(0x0F);

     //++
	 // Power Control 1  (Vreg1out, Verg2out)
	 ILI9488_SendCommand(0XC0);
	 ILI9488_SendData(0x17);
	 ILI9488_SendData(0x15);
	 osDelay(5);

	 // Power Control 2 // Power Control 2 (VGH,VGL)
	 ILI9488_SendCommand(0xC1);
	 ILI9488_SendData(0x41);
	 osDelay(5);

	/// Power Control 3 (Vcom)
	ILI9488_SendCommand(0xC5); // VCOM Control
//	ILI9488_SendData(0x00);
//	ILI9488_SendData(0x12);
//	ILI9488_SendData(0x80);
	ILI9488_SendData(0x1B);  // VMH = 0x1A (примерно 4.25V)
	ILI9488_SendData(0x1B);  // VML = 0x1A (примерно -1.3V)
	ILI9488_SendData(0x80);  // VMO = Mode 1
	osDelay(5);


         ILI9488_SendCommand(0x36); // Memory Access Control
         ILI9488_SendData(0x48);          // MX, BGR

/////////////
//         ILI9488_SendCommand(0x3A);
//         ILI9488_SendData(0x55);  // 16-bit color
//
//         ILI9488 according to the datasheet:
//
//             0x55 = 16-bit (RGB565)
//             0x66 = 18-bit (RGB666)
//
//         ⚠️ But! Via SPI, many Chinese ILI9488 modules expect exactly 18 bits (3 bytes per pixel),
//    and at 0x55 the screen remains white.
//
	// Interface Pixel Format
	ILI9488_SendCommand(0x3A); // Pixel Interface Format
	//ILI9488_SendData(0x55);  // 16-bit colour
	ILI9488_SendData(0x66);   // 18-bit colour (RGB666)

//
        ILI9488_SendCommand(0xF7); // Adjust Control
           ILI9488_SendData(0xA9);
           ILI9488_SendData(0x51);
           ILI9488_SendData(0x2C);
           ILI9488_SendData(0x82);

//        ILI9488_SendCommand(0xB0); // Interface Mode Control
//        ILI9488_SendData(0x00);

        // Frame rate (60Hz)
         ILI9488_SendCommand(0xB1); // Frame Rate Control
         ILI9488_SendData(0xA0);

         // Display Inversion Control (2-dot)
        //ILI9488_SendCommand(0xB4); // Display Inversion Control
        //ILI9488_SendData(0x02);
        //ILI9488_SendData(0x00);

        // Display Function Control RGB/MCU Interface Control
        ILI9488_SendCommand(0xB6); // Display Function Control
        ILI9488_SendData(0x02);
        ILI9488_SendData(0x02);
//        ILI9488_SendData(0x3B);
//
//        ILI9488_SendCommand(0xB7); // Entry Mode Set
//        ILI9488_SendData(0xC6);

         ILI9488_SendCommand(0xF7); // Adjust Control 3
         ILI9488_SendData(0xA9);
         ILI9488_SendData(0x51);
         ILI9488_SendData(0x2C);
         ILI9488_SendData(0x82);

        // Sleep Out
	   ILI9488_SendCommand(0x11);
	   osDelay(120);

     ILI9488_SendCommand(0x29); // Display ON
     osDelay(20);


     //ILI9488_SendCommand(0x20); //Display Inversion OFF
   //ILI9488_SendCommand(0x21); //Display Inversion ON

     //ILI9488_SendCommand(0x20); // Inversion Off

   // ILI9488_SendCommand(0x22);  //All Pixels OFF
    // ILI9488_SendCommand(0x23); // All Pixels ON


    ILI9488_FillScreen(COLOR_BLACK);
    ILI9488_SetRotation(lcd_rotation);
    ILI9488_Unselect();


}

uint16_t ILI9488_GetWidth(void) {
    return (lcd_rotation == ROTATE_0_DEG || lcd_rotation == ROTATE_180_DEG) ? 320 : 480;
}

uint16_t ILI9488_GetHeight(void) {
    return (lcd_rotation == ROTATE_0_DEG || lcd_rotation == ROTATE_180_DEG) ? 480 : 320;
}

void ILI9488_SetRotation(ScreenRotation_t rotation) {
    lcd_rotation = rotation;
    ILI9488_SendCommand(0x36);
    switch(rotation) {
        case ROTATE_0_DEG:   ILI9488_SendData(0x48); break;
        case ROTATE_90_DEG:  ILI9488_SendData(0x28); break;
        case ROTATE_180_DEG: ILI9488_SendData(0x88); break;
        case ROTATE_270_DEG: ILI9488_SendData(0xE8); break;
    }
}


//
//void ILI9488_SetRotation(ScreenRotation_t rotation)
//{
//	lcd_rotation = rotation;
//
//    uint8_t madctl = 0;
//#define ST7796_MADCTL     0x36
//#define ST7796_MADCTL_MY  0x80
//#define ST7796_MADCTL_MX  0x40
//#define ST7796_MADCTL_MV  0x20
//#define ST7796_MADCTL_BGR 0x08
//
//
//    switch (rotation)
//    {
//        case ROTATE_0_DEG:
//            madctl = ST7796_MADCTL_MX | ST7796_MADCTL_BGR;
//            break;
//
//        case ROTATE_90_DEG:
//            madctl = ST7796_MADCTL_MV | ST7796_MADCTL_BGR;
//            break;
//
//        case ROTATE_180_DEG:
//            madctl = ST7796_MADCTL_MY | ST7796_MADCTL_BGR;
//            break;
//
//        case ROTATE_270_DEG:
//            madctl = ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR;
//            break;
//
//        default:
//            return;
//    }
//
//
//    ILI9488_SendCommand(0x36);
//    //ST7796_WriteCommand(ST7796_MADCTL);
//    ILI9488_SendData(madctl);
//}

// RGB666 !!!! ONLY
void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint16_t color565) {
    if (x >= ILI9488_GetWidth() || y >= ILI9488_GetHeight()) return;

    ILI9488_SetAddressWindow(x, y, x, y);

    rgb666_t c = rgb565_to_rgb666_struct(color565); 
	uint8_t data[] = { c.r << 2, c.g << 2, c.b << 2 };

    ILI9488_Select();
    ILI9488_DC_Data();
    HAL_SPI_Transmit(&hspi1, data, 3, HAL_MAX_DELAY);
    ILI9488_Unselect();
}

// RGB666 !!!! ONLY
void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color565)
{

	if ((x >= ILI9488_GetWidth()) || (y >= ILI9488_GetHeight())) return;
	if ((x + w - 1) >= ILI9488_GetWidth())  w = ILI9488_GetWidth() - x;
	if ((y + h - 1) >= ILI9488_GetHeight()) h = ILI9488_GetHeight() - y;


	ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint32_t totalPixels = (uint32_t)(w * h);

    rgb666_t c = rgb565_to_rgb666_struct(color565); 
    uint8_t pixel[] = { c.r << 2, c.g << 2, c.b << 2 };

    for (uint32_t i = 0; i < PIXEL_CHUNK; i++) {
        uint8_t* dst = &ili9488_fill_buffer[i * 3];

        dst[0] = pixel[0];
        dst[1] = pixel[1];
        dst[2] = pixel[2];
    }

    while (totalPixels >= PIXEL_CHUNK) {
        ILI9488_SendDataArray(ili9488_fill_buffer, sizeof(ili9488_fill_buffer));
        totalPixels -= PIXEL_CHUNK;
    }

    if (totalPixels > 0) {
        ILI9488_SendDataArray(ili9488_fill_buffer, totalPixels * 3);
    }
}

void ILI9488_FillScreen(uint16_t color565) {
    ILI9488_FillRectangle(0, 0, ILI9488_GetWidth(), ILI9488_GetHeight(), color565);
}

void ILI9488_InvertColors(bool invert) {
    ILI9488_SendCommand(invert ? 0x21 : 0x20);
}

//need fix rgb666
void ILI9488_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if (w == 0 || h == 0) return;
    if (x + w > ILI9488_GetWidth())  w = ILI9488_GetWidth() - x;
    if (y + h > ILI9488_GetHeight()) h = ILI9488_GetHeight() - y;

    ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    ILI9488_Select();
    ILI9488_DC_Data();
    //HAL_SPI_Transmit(&hspi1, (uint8_t*)data, w * h * 2, HAL_MAX_DELAY);
    for (uint32_t i = 0; i < w * h; i++) {
            uint16_t color = data[i];

           // rgb666_t c = rgb565_to_rgb666_struct(color565); 
           // uint8_t pixel[] = { c.r << 2, c.g << 2, c.b << 2 };


            uint8_t r = ((color >> 11) & 0x1F) << 3;
            uint8_t g = ((color >> 5)  & 0x3F) << 2;
            uint8_t b = (color & 0x1F) << 3;
            uint8_t pixel[3] = {r, g, b};
            HAL_SPI_Transmit(&hspi1, pixel, 3, HAL_MAX_DELAY);
        }
    ILI9488_Unselect();
}

//static uint8_t data_char[2];

void ILI9488_WriteChar(uint16_t x, uint16_t y, char ch, const FontDef* font, uint16_t color565, uint16_t bgcolor565) {
//    if (x + font->width > ILI9488_GetWidth() || y + font->height > ILI9488_GetHeight()) return;
//	if (ch < 32) return;
//	if (x >= ILI9488_GetWidth() || y >= ILI9488_GetHeight()) return;
//	if (x + font->width > ILI9488_GetWidth()) return;
//	if (y + font->height > ILI9488_GetHeight()) return;
//
//	uint16_t offset = (ch - 32) * font->height;  // один столбец на символ
//	for (uint16_t i = 0; i < font->height; i++) {
//	        for (uint16_t j = 0; j < font->width; j++) {
//	            uint16_t pixel_color = font->data[offset + i * font->width + j];
//
//	            // Если цвет нулевой — фон, иначе — символ
//	            if (pixel_color == 0x0000) {
//	                ILI9488_DrawPixel(x + j, y + i, bgcolor565);
//	            } else {
//	                ILI9488_DrawPixel(x + j, y + i, pixel_color);  // уже RGB565!
//	            }
//	        }
//	 }





	    if ((x + font->width > ILI9488_GetWidth()) || (y + font->height > ILI9488_GetHeight()))
	        return;

	    ILI9488_SetAddressWindow(x, y, x + font->width - 1, y + font->height - 1);

	    // Конвертируем цвета один раз
	   rgb666_t c_fg = rgb565_to_rgb666_struct(color565);
	   rgb666_t c_bg = rgb565_to_rgb666_struct(bgcolor565);

	   uint8_t data[3];  // буфер на 3 байта (RGB666)
	   uint32_t offset = (ch - 32) * font->height;

	   for (uint32_t i = 0; i < font->height; i++)
	   {
	       uint16_t row = font->data[offset + i];  // одна строка шрифта (битовая маска)

		   for (uint32_t j = 0; j < font->width; j++)
		   {
			   // Проверяем бит (от старшего, как в твоём коде)
			   if (row & (1 << (15 - j))) {
				   data[0] = c_fg.r << 2;
				   data[1] = c_fg.g << 2;
				   data[2] = c_fg.b << 2;
			   } else {
				   data[0] = c_bg.r << 2;
				   data[1] = c_bg.g << 2;
				   data[2] = c_bg.b << 2;
			   }
			   // Отправляем один пиксель (3 байта)
			   ILI9488_SendDataArray(data, 3);
		   }
	   }
//	    for (uint32_t i = 0; i < font->height; i++)
//	    {
//	        uint16_t b = font->data[(ch - 32) * font->height + i];
//
//	        for (uint32_t j = 0; j < font->width; j++)
//	        {
//	            if (b & (1 << (15 - j))) {
//
//	            	data_char[0] = color565 >> 8;
//					data_char[1] = color565 & 0xFF;
//					ILI9488_SendDataArray(data_char, sizeof(data_char));
//	            } else {
//	            	data_char[0] = bgcolor565 >> 8;
//					data_char[1] = bgcolor565 & 0xFF;
//					ILI9488_SendDataArray(data_char, sizeof(data_char));
//	            }
//	        }
//	    }


//    uint16_t i, j;
//    for (i = 0; i < font->height; i++) {
//        uint8_t row = font->data[(ch - 32) * font->height + i];
//        for (j = 0; j < font->width; j++) {
//            if (row & (1 << j)) {
//                ILI9488_DrawPixel(x + j, y + i, color565);
//            } else {
//                ILI9488_DrawPixel(x + j, y + i, bgcolor565);
//            }
//        }
//    }
}

void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, const FontDef* font, uint16_t color, uint16_t bgcolor) {
    while (*str) {
        ILI9488_WriteChar(x, y, *str, font, color, bgcolor);
        x += font->width;
        str++;
    }
}

#endif //USE_LCD_ILI_9488
