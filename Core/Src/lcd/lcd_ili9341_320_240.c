//#include "stm32f4xx_hal.h"
#include "lcd_ili9341_320_240.h"
#include "main.h"
#include "cmsis_os.h" 
#include <stdlib.h>
#include <string.h>
#include "lcd_color_rgb565.h"

/*** Redefine if necessary ***/
#define ILI9341_SPI_PORT hspi1

extern SPI_HandleTypeDef ILI9341_SPI_PORT;


volatile uint16_t LCD_HEIGHT = ILI9341_HEIGHT; //320
volatile uint16_t LCD_WIDTH	 = ILI9341_WIDTH; // 240

#define PIXEL_CHUNK 64   // Optimal for most STM32
//#define PIXEL_CHUNK 128  // Number of pixels for one WriteData call (can be adjusted depending on the SPI buffer size)


uint16_t ILI9341_GetHeight(void)
{
	return LCD_HEIGHT;
}

uint16_t ILI9341_GetWidth(void)
{
	return LCD_WIDTH;
}

static void ILI9341_Select()
{
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
}

void ILI9341_Unselect(void) {
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

static void ILI9341_Reset() {
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_RESET);
    osDelay(5);
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_SET);
}

static void ILI9341_WriteCommand(uint8_t cmd) {
	HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET); // 0
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_RESET); // 0
    HAL_SPI_Transmit(&ILI9341_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);   // send
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);   // 1
}

static void ILI9341_WriteData(uint8_t* buff, size_t buff_size)
{
	HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET); // 0
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);   // 1

    // split data in small chunks because HAL can't send more then 64K at once
    while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&ILI9341_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;
        buff_size -= chunk_size;
    }
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);   // 1
}

static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // column address set
    ILI9341_WriteCommand(0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ILI9341_WriteData(data, sizeof(data));
    }

    // row address set
    ILI9341_WriteCommand(0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ILI9341_WriteData(data, sizeof(data));
    }

    // write to RAM
    ILI9341_WriteCommand(0x2C); // RAMWR
}

void ILI9341_Init()
{
	ILI9341_Unselect();

    ILI9341_Select();
    ILI9341_Reset();

    // command list is based on https://github.com/martnak/STM32-ILI9341

    // SOFTWARE RESET
    ILI9341_WriteCommand(0x01);
    osDelay(1000);
        
    // POWER CONTROL A
    ILI9341_WriteCommand(0xCB);
    {
        uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER CONTROL B
    ILI9341_WriteCommand(0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL A
    ILI9341_WriteCommand(0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL B
    ILI9341_WriteCommand(0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER ON SEQUENCE CONTROL
    ILI9341_WriteCommand(0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // PUMP RATIO CONTROL
    ILI9341_WriteCommand(0xF7);
    {
        uint8_t data[] = { 0x20 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER CONTROL,VRH[5:0]
    ILI9341_WriteCommand(0xC0);
    {
        uint8_t data[] = { 0x23 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_WriteCommand(0xC1);
    {
        uint8_t data[] = { 0x10 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // VCM CONTROL
    ILI9341_WriteCommand(0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // VCM CONTROL 2
    ILI9341_WriteCommand(0xC7);
    {
        uint8_t data[] = { 0x86 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // MEMORY ACCESS CONTROL
    ILI9341_WriteCommand(0x36);
    {
        uint8_t data[] = { 0x48 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // PIXEL FORMAT
    ILI9341_WriteCommand(0x3A);
    {
        uint8_t data[] = { 0x55 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_WriteCommand(0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // DISPLAY FUNCTION CONTROL
    ILI9341_WriteCommand(0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // 3GAMMA FUNCTION DISABLE
    ILI9341_WriteCommand(0xF2);
    {
        uint8_t data[] = { 0x00 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // GAMMA CURVE SELECTED
    ILI9341_WriteCommand(0x26);
    {
        uint8_t data[] = { 0x01 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POSITIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // NEGATIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        ILI9341_WriteData(data, sizeof(data));
    }

    // EXIT SLEEP
    ILI9341_WriteCommand(0x11);
    osDelay(120);

    // TURN ON DISPLAY
    ILI9341_WriteCommand(0x29);

    // MADCTL
    ILI9341_WriteCommand(0x36);
    {
        uint8_t data[] = { ILI9341_ROTATION };
        ILI9341_WriteData(data, sizeof(data));
    }

    ILI9341_Unselect();

    ILI9341_FillScreen(COLOR_BLACK);
    ILI9341_SetRotation(SCREEN_HORIZONTAL_1);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= LCD_WIDTH) || (y >= LCD_HEIGHT))
        return;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    ILI9341_WriteData(data, sizeof(data));
    ILI9341_Unselect();
}

void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;

    ILI9341_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                ILI9341_WriteData(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                ILI9341_WriteData(data, sizeof(data));
            }
        }
    }
}

void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
    ILI9341_Select();

    while(*str) {
        if(x + font.width >= LCD_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= LCD_HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        ILI9341_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    ILI9341_Unselect();
}


void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {

    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) return;
    if ((x + w - 1) >= LCD_WIDTH)  w = LCD_WIDTH - x;
    if ((y + h - 1) >= LCD_HEIGHT) h = LCD_HEIGHT - y;

    ILI9341_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint32_t totalPixels = w * h;
    uint8_t buffer[PIXEL_CHUNK * 2];	// Buffer: each pixel - 2 bytes (RGB565)

    for (int i = 0; i < PIXEL_CHUNK; i++) {
        buffer[2 * i]     = color >> 8;      // High byte
        buffer[2 * i + 1] = color & 0xFF;    // low byte
    }

    // Sending data in blocks
    while (totalPixels >= PIXEL_CHUNK) {
        ILI9341_WriteData(buffer, sizeof(buffer));
        totalPixels -= PIXEL_CHUNK;
    }

    // Remaining pixels (if any)
    if (totalPixels > 0) {
        ILI9341_WriteData(buffer, totalPixels * 2);
    }
}

void ILI9341_FillScreen(uint16_t color)
{
    //ILI9341_SetAddressWindow(0, 0, LCD_WIDTH, LCD_HEIGHT);
    ILI9341_FillRectangle(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
	if (w == 0 || h == 0) return;
	if((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) return;
    if((x + w - 1) >= LCD_WIDTH) return;
    if((y + h - 1) >= LCD_HEIGHT) return;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, (x + w - 1), (y + h - 1));

//    ILI9341_WriteData((uint8_t*)data, sizeof(uint16_t)*w*h);
    for (uint32_t i = 0; i < w * h; i++)
    {
           uint16_t color = data[i];
           uint8_t high = color >> 8;
           uint8_t low  = color & 0xFF;
           ILI9341_WriteData(&high, 1);
           ILI9341_WriteData(&low ,1);
    }
    ILI9341_Unselect();
}

void ILI9341_InvertColors(bool invert) {
    ILI9341_Select();
    ILI9341_WriteCommand(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
    ILI9341_Unselect();
}


/*Ser rotation of the screen - changes x0 and y0*/
void ILI9341_SetRotation(ScreenRotation_t Rotation)
{
	ILI9341_WriteCommand(0x36);
    osDelay(1);

    uint8_t data;
    switch (Rotation)
    {
        case SCREEN_VERTICAL_1:
            data = 0x40 | 0x08;
            LCD_WIDTH = 240;
            LCD_HEIGHT = 320;
            break;
        case SCREEN_HORIZONTAL_1:
            data = 0x20 | 0x08;
            LCD_WIDTH = 320;
            LCD_HEIGHT = 240;
            break;
        case SCREEN_VERTICAL_2:
            data = 0x80 | 0x08;
            LCD_WIDTH = 240;
            LCD_HEIGHT = 320;
            break;
        case SCREEN_HORIZONTAL_2:
            data = 0x40 | 0x80 | 0x20 | 0x08;
            LCD_WIDTH = 320;
            LCD_HEIGHT = 240;
            break;
        default:
            return;
    }

    ILI9341_WriteData(&data, 1);
}

//////////////////////////////////

