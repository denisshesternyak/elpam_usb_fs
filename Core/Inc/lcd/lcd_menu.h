

#ifndef INC_SCREENS_LCD_MENU_H_
#define INC_SCREENS_LCD_MENU_H_

//#include "lcd_display.h"
#include <mcp23008_btns.h>
#include "lcd_lang.h"
//#include "ds3231.h"
//#include "lcd_widget_volume_indicator.h"
#include <stdbool.h>

#define MAX_MENU_ITEMS 			25
#define INACTIVITY_TIMEOUT_MS 	20
#define BACKLIGHT_TIMEOUT_MS 	10

typedef struct {
    const void* image;
    uint16_t x, y;
    uint16_t w, h;
} MenuImage;

typedef enum {
	MENU_TYPE_UNK,
    MENU_TYPE_LIST,
	MENU_TYPE_MAIN,
	MENU_TYPE_PASSWORD,
	MENU_TYPE_SIREN_INFO,
    //MENU_TYPE_SCREEN,
	MENU_TYPE_MESSAGE_PLAY,
	MENU_TYPE_IDLE,
	MENU_TYPE_ANNOUNCEMENT,
	MENU_TYPE_REPORT,
	MENU_TYPE_TEST_BAT,
	MENU_TYPE_TEST_AMP,
	MENU_TYPE_TEST_DRIV,
	MENU_TYPE_CLOCK
} MenuType;

typedef enum {
	LCD_EVENT_IDLE,
	LCD_EVENT_BTN,
	LCD_EVENT_PROGRESS,
	LCD_EVENT_RTC
} LCDEvent;

typedef struct {
	LCDEvent event;
	ButtonEvent_t btn;
	uint32_t value;
} LCDTaskEvent_t;

typedef void (*MenuButtonHandler)(ButtonEvent_t event);
typedef void (*MenuAction)(void);

typedef struct MenuItem {
    const char* name[LANG_COUNT];
    MenuAction prepareAction;
    MenuAction postAction;
    struct Menu* submenu;
    const char* filepath;
} MenuItem;


typedef struct Menu {
    struct Menu* parent;
	MenuType type;
	uint8_t scrollOffset;
	uint8_t oldOffset;
    MenuItem items[MAX_MENU_ITEMS];
    uint8_t itemCount;
    uint8_t currentSelection;
    uint8_t oldSelection;
	const char* screenText[LANG_COUNT];
	char* textFilename;
	MenuImage* imageData;

	MenuButtonHandler buttonHandler;
} Menu;

//void _basic_init_menu(Menu* m);

void menu_handle_button(ButtonEvent_t event);

bool hot_key_handle_button(ButtonEvent_t event);

void HandleButtonPress(ButtonEvent_t event);
void clockMenu_HandleButtonPress(ButtonEvent_t event);
void languageMenu_HandleButtonPress(ButtonEvent_t event);
//void passwordMenu_HandleButtonPress(ButtonEvent_t event);
void alarmInfoMenu_HandleButtonPress(ButtonEvent_t event);
void sinusInfoMenu_HandleButtonPress(ButtonEvent_t event);

void Menu_Init(void);

void DrawStatusBar(void);
void UpdateDateTime(void);
void DrawMenuScreen(bool forceFullRedraw);
void DisplayMenuItem(uint8_t index, const MenuItem* item, bool selected, bool dummy);
void MenuDrawImage(Menu *m);
void ClearMenu(Menu* menu);
void MenuLoadSDCardMessages(void);
void MenuLoadSDCardSirens(void);
void Draw_MENU_TYPE_CLOCK(void);

//void MenuShowMessages(void);
//void ShowUartCommand(void);


void sirenPrepareAction(void);
void sirenPostAction(void);
//
////---------------------------------------------
//// MENU_TYPE_ANNOUNCEMENT
//void VolumeControlButtonHandler(ButtonEvent_t event);
//void DecreaseVolume();
//void IncreaseVolume();
////---------------------------------------------
//
//
////---------------------------------------------
void PlayMessageStart(void);
bool PlayMessageProgress(const uint8_t value);
void PlayMessageEnd(void);
////---------------------------------------------
//
////void GoToRootMenu(void) ;
////void MenuBack(void) ;
//
//
//// debug
//const char* ButtonToString(Button_t btn) ;
//const char* ButtonActionToString(ButtonAction_t ba);
//void DrawDebugInfo(const ButtonEvent_t* event);

void test_count_up_menu();
void UpdateProgressBar(uint8_t value);

#endif /* INC_SCREENS_LCD_MENU_H_ */
