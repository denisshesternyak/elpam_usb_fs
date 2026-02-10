

#ifndef INC_SCREENS_LCD_MENU_H_
#define INC_SCREENS_LCD_MENU_H_

//#include "lcd_display.h"
#include <mcp23008_btns.h>
#include "lcd_lang.h"
#include "lcd_strings_menu.h"

//#include "ds3231.h"
//#include "lcd_widget_volume_indicator.h"
#include <stdbool.h>

#define MAX_MENU_ITEMS 			25
#define INACTIVITY_TIMEOUT_MS 	60
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
	MENU_TYPE_CLOCK,
	MENU_TYPE_MOTOROLA
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
    struct Menu* menu;
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
	const char* textFilename;
	MenuImage* imageData;

	MenuButtonHandler buttonHandler;
} Menu;

//void _basic_init_menu(Menu* m);

void menu_init(void);
void menu_handle_button(ButtonEvent_t event);
void update_date_time(void);
void update_progress_bar(uint8_t value);

#endif /* INC_SCREENS_LCD_MENU_H_ */
