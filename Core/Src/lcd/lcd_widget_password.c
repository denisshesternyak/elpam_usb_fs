

#include "lcd_display.h"
#include "lcd_widget_password.h"
#include <string.h>

static bool passwordIsCorrect = false;  

#define STATUS_BAR_LINE_Y_POS  35

#define PASSWORD_TEXT_LINE1_Y 80
#define PASSWORD_TEXT_LINE2_Y 100
#define PASSWORD_BOX_Y 150
#define PASSWORD_BOX_SIZE 40
#define PASSWORD_BOX_SPACING 20

#define MAX_PASSWORD_LENGTH 6

#define COLOR_WHITE     0xFFFF
#define COLOR_BLACK     0x0000
#define COLOR_LIGHTGRAY 0xC618
#define COLOR_HIGHLIGHT 0x7E0F  

static uint8_t passwordLength = 4;
static char passwordBuffer[MAX_PASSWORD_LENGTH] = {0};
static uint8_t passwordIndex = 0;
static const char correctPassword[] = "1111";

bool Password_IsCorrect(void)
{
    return passwordIsCorrect;
}


void OnPasswordComplete(bool success);
void DrawPasswordBox(uint8_t index, bool filled);

void Password_AddChar(char c)
{
    if (passwordIndex >= passwordLength)
        return;

    passwordBuffer[passwordIndex++] = c;

    DrawPasswordBox(passwordIndex - 1, true);

    if (passwordIndex < passwordLength)
        DrawPasswordBox(passwordIndex, false);

//    if (passwordIndex == passwordLength) {
//        bool match = (strncmp(passwordBuffer, correctPassword, passwordLength) == 0);
//        OnPasswordComplete(match);
//    }
}

void Password_Backspace(void)
{
    if (passwordIndex == 0)
        return;

    DrawPasswordBox(passwordIndex, false);

    passwordIndex--;
    passwordBuffer[passwordIndex] = 0;

    DrawPasswordBox(passwordIndex, false);
}

void Password_Reset(bool redraw)
{
    memset(passwordBuffer, 0, sizeof(passwordBuffer));
    passwordIndex = 0;
    passwordIsCorrect = false;
    if (redraw){
    	Draw_MENU_TYPE_PASSWORD();
    }

}

void Password_Enter(void)
{
    if (passwordIndex < passwordLength)
    {
        
        const FontDef* font = &Font_11x18;
        LCD_WriteStringAligned(PASSWORD_BOX_Y + PASSWORD_BOX_SIZE + 20,
                               "Incomplete password", font, COLOR_BLACK, COLOR_WHITE, ALIGN_CENTER);
        return;
    }

    
    bool match = (strncmp(passwordBuffer, correctPassword, passwordLength) == 0);
    OnPasswordComplete(match);
}


void DrawPasswordBox(uint8_t index, bool filled)
{
    if (index >= passwordLength)
        return;

    const FontDef* font = &Font_11x18;

    uint16_t totalWidth = PASSWORD_BOX_SIZE * passwordLength + PASSWORD_BOX_SPACING * (passwordLength - 1);
    uint16_t startX = (LCD_GetWidth() - totalWidth) / 2;
    uint16_t x = startX + index * (PASSWORD_BOX_SIZE + PASSWORD_BOX_SPACING);

    uint16_t fillColor = (index == passwordIndex && passwordIndex < passwordLength)
                         ? COLOR_HIGHLIGHT : COLOR_LIGHTGRAY;

    LCD_FillRectangle(x, PASSWORD_BOX_Y, PASSWORD_BOX_SIZE, PASSWORD_BOX_SIZE, fillColor);
    LCD_DrawRect(x, PASSWORD_BOX_Y, PASSWORD_BOX_SIZE, PASSWORD_BOX_SIZE, COLOR_BLACK);

    if (filled) {
        LCD_WriteChar(x + (PASSWORD_BOX_SIZE / 2) - 5, PASSWORD_BOX_Y + 15, '*', font, COLOR_BLACK, fillColor);
    }
}


void Draw_MENU_TYPE_PASSWORD(void)
{
    //LCD_FillScreen(COLOR_WHITE);
    LCD_FillRectangle(0, STATUS_BAR_LINE_Y_POS+2, LCD_GetWidth(), LCD_GetHeight(), COLOR_WHITE);

    const FontDef* font = &Font_11x18;

    LCD_WriteStringAligned(PASSWORD_TEXT_LINE1_Y, "Enter password to", font, COLOR_BLACK, COLOR_WHITE, ALIGN_CENTER);
    LCD_WriteStringAligned(PASSWORD_TEXT_LINE2_Y, "continue.", font, COLOR_BLACK, COLOR_WHITE, ALIGN_CENTER);

    for (int i = 0; i < passwordLength; i++) {
        bool filled = i < passwordIndex;
        DrawPasswordBox(i, filled);
    }
}


void OnPasswordComplete(bool success)
{
    const FontDef* font = &Font_11x18;
    passwordIsCorrect = success;

    if (success) {
        LCD_WriteStringAligned(PASSWORD_BOX_Y + PASSWORD_BOX_SIZE + 20, "Access Granted", font, COLOR_BLACK, COLOR_WHITE, ALIGN_CENTER);
    } else {
        LCD_WriteStringAligned(PASSWORD_BOX_Y + PASSWORD_BOX_SIZE + 20, "Access Denied", font, COLOR_BLACK, COLOR_WHITE, ALIGN_CENTER);
        osDelay(1000);
        Password_Reset(false);
    }
}
