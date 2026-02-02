#include "main.h"
#include "lcd_menu.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "queue.h"
//#include "lcd_display.h"
//#include "lcd_widget_siren_info_indicator.h"
//#include "lcd_widget_password.h"
//#include "system_status.h"

//#include "ff.h"
#include <string.h>
#include <stdio.h>
#include "hx8357d.h"
#include "lcd_color_rgb565.h"
#include "Speaker-1_80x74.h"
#include "m2_80x74.h"
#include "lcd_widget_progress_bar.h"
#include "lcd_widget_volume_indicator.h"
#include "lcd_widget_faults_indicator.h"
#include "lcd_widget_batteries_indicator.h"
#include "lcd_widget_test_drivers_indicator.h"
#include "lcd_widget_test_ampl_indicator.h"
#include "lcd_widget_report_indicator.h"
#include "audio_types.h"

//bool isResetPasswordAfterIdle = false;
//
//extern QueueHandle_t audioQueue;

//
static bool isPlayAudioFile = false;
static bool isBacklightOn = false;

#define MAX_MENU_POOL 40

static uint32_t lastInteractionTick = 0;
static Menu menuPool[MAX_MENU_POOL];
uint8_t menuPoolIndex = 0;

extern osMessageQueueId_t xAudioQueueHandle;
extern RTC_HandleTypeDef hrtc;

#define MAX_FILENAME_LEN 		64
#define CLOCK_MAX_SYMBOLS		6

typedef struct {
    uint8_t current_symbol;
    int8_t  change_value;
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
} ClockEdit_t;

ClockEdit_t clock;


char messageFilenames[MAX_MENU_ITEMS][MAX_FILENAME_LEN];
const char* selectedFile = NULL;

extern Audio_Player_t player;

Menu* currentMenu = NULL;
Menu* idleMenu = NULL;
Menu* rootMenu = NULL;
Menu* messagesMenu = NULL;
Menu* sirenMenu = NULL;
Menu* announcementMenu = NULL;
Menu* testMenu = NULL;
Menu* reportMenu = NULL;
Menu* messagePlayMenu = NULL;
Menu* batteriesTestMenu = NULL;
Menu* apmplifiresTestMunu = NULL;
Menu* driversTestMenu = NULL;
Menu* alarm_info_menu = NULL;
Menu* maintenanceMenu = NULL;
Menu* clockMenu = NULL;
Menu* languageMenu = NULL;
Menu* sinusMenu = NULL;
Menu* sinusInfoMenu = NULL;

//Menu* passwordMenu = NULL;
//
//static uint8_t volumeValue = 10;
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
MenuImage menu_speaker_img = {
		.image = speaker_img,
		.x = ((HX8357_TFTWIDTH/2) - (SPEAKER_IMG_WIDTH /2)),
		.y = 120,
		.w = SPEAKER_IMG_WIDTH,
		.h = SPEAKER_IMG_HEIGHT
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

MenuImage menu_microfon_img = {
		.image = microfon_img,
		.x = ((HX8357_TFTWIDTH/2) - (MICROFON_IMG_WIDTH /2)),
		.y = 120,
		.w = MICROFON_IMG_WIDTH,
		.h = MICROFON_IMG_HEIGHT
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////


#define MAX_VISIBLE_ITEMS 12

#define TIME_HEIGHT 9

#define MENU_ITEM_HEIGHT 20

#define DEBUG_INFO_Y 290
#define STATUS_BAR_HEIGHT 24  
#define LOGO_X_POS 10
#define LOGO_Y_POS 5

#define TIME_X_POS 340       
#define TIME_Y_POS 5

#define SERIAL_X_POS 340     
#define SERIAL_Y_POS 20

#define STATUS_BAR_LINE_Y_POS  35
#define TITLE_MENU_Y_POS  (STATUS_BAR_LINE_Y_POS + 10)

#define SIREN_Y_POS  (TITLE_MENU_Y_POS + 45)
#define IDLE_Y_POS  (TITLE_MENU_Y_POS + 25)
#define BAT_Y_POS  (TITLE_MENU_Y_POS + 25)
#define AMP_Y_POS  (TITLE_MENU_Y_POS + 50)
#define DRV_Y_POS  (TITLE_MENU_Y_POS + 50)
#define REPORT_Y_POS  (TITLE_MENU_Y_POS + 50)

#define MENU_BASE_X 20
#define MENU_BASE_Y (TITLE_MENU_Y_POS + 25)

#define SERIAL_NUMBER "123456"

///////////////////////////////////////////////////////////////////
void RunSilentTest(void);
void RunBatteriesTest(void);
void RunAmplifiresTest(void);
void RunDriversTest(void);
void run_clock(void);
void prepare_clock(void);
static void menu_init_language(void);

static bool hot_key_handle_button(ButtonEvent_t event);
//void passwordMenu_handle_button_press(ButtonEvent_t event);
static void handle_button_press(ButtonEvent_t event);
static void clockMenu_handle_button_press(ButtonEvent_t event);
static void languageMenu_handle_button_press(ButtonEvent_t event);
static void sinus_info_menu_handler(ButtonEvent_t event);
static void alarm_info_menu_handler(ButtonEvent_t event);
static void idle_menu_handler(ButtonEvent_t event);

///////////////////////////////////////////////////////////////////

static void clear_position(Menu* menu);
static void clear_menu(Menu* menu);
static void basic_init_menu(Menu* menu);

static void button_up_handler(void);
static void button_down_handler(void);
static void button_enter_handler(void);
static void button_esc_handler(void);

static void draw_status_bar(void);
static void draw_menuScreen(bool forceFullRedraw);
static void display_menu_item(uint8_t visualIndex, uint8_t index, const MenuItem* item, bool selected, bool dummy);
static void menu_draw_image(Menu *m);
static void MenuLoadSDCardMessages(void);
static void MenuLoadSDCardSirens(void);
static void prepare_sinuse_items(void);
static void draw_menu_clock(void);

static void siren_post_action(void);

static void BLK_ON()  { HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET); }
static void BLK_OFF() { HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET); }

static void clear_position(Menu* menu)
{
	menu->scrollOffset = 0;
	menu->currentSelection = 0;
	menu->oldSelection = 0;
	menu->oldOffset = 0;
}

static void clear_menu(Menu* menu)
{
    if (!menu) return;
    if (menu->type != MENU_TYPE_LIST) return;

    clear_position(menu);
    menu->itemCount = 0;

    for (int i = 0; i < MAX_MENU_ITEMS; ++i)
    {
        for (int lang = 0; lang < LANG_COUNT; ++lang)
        {
            menu->items[i].name[lang] = NULL;
        }

        menu->items[i].prepareAction = NULL;
        menu->items[i].postAction = NULL;
        menu->items[i].submenu = NULL;
    }
}

static void basic_init_menu(Menu* menu)
{
    if (!menu) return;

    menu->parent = NULL;
    menu->type = MENU_TYPE_UNK;
    menu->scrollOffset = 0;
    menu->itemCount = 0;
    menu->currentSelection = 0;
    menu->oldSelection = 0;
    menu->oldOffset = 0;

    for (int i = 0; i < MAX_MENU_ITEMS; ++i)
    {
        for (int lang = 0; lang < LANG_COUNT; ++lang)
        {
            menu->items[i].name[lang] = NULL;
        }
        menu->items[i].prepareAction = NULL;
        menu->items[i].postAction = NULL;
        menu->items[i].submenu = NULL;
    }

    for (int lang = 0; lang < LANG_COUNT; ++lang) {
        menu->screenText[lang] = NULL;
    }

    menu->textFilename = NULL;
    menu->imageData = NULL;
    menu->buttonHandler = NULL;
}

void InitMenuPool(void)
{
	for (int i = 0; i < MAX_MENU_POOL; ++i) {
		basic_init_menu(&menuPool[i]);
	}
	menuPoolIndex = 0;
}

void menu_init(void)
{
	InitMenuPool();

//	passwordMenu = &menuPool[menuPoolIndex++];
	idleMenu = &menuPool[menuPoolIndex++];
    rootMenu = &menuPool[menuPoolIndex++];
    sirenMenu = &menuPool[menuPoolIndex++];
    alarm_info_menu = &menuPool[menuPoolIndex++];
	messagesMenu = &menuPool[menuPoolIndex++];
	messagePlayMenu = &menuPool[menuPoolIndex++];
	announcementMenu = &menuPool[menuPoolIndex++];
	testMenu = &menuPool[menuPoolIndex++];
	batteriesTestMenu = &menuPool[menuPoolIndex++];
	apmplifiresTestMunu = &menuPool[menuPoolIndex++];
	driversTestMenu = &menuPool[menuPoolIndex++];
	reportMenu = &menuPool[menuPoolIndex++];
	maintenanceMenu = &menuPool[menuPoolIndex++];
	clockMenu = &menuPool[menuPoolIndex++];
	languageMenu = &menuPool[menuPoolIndex++];
	sinusMenu = &menuPool[menuPoolIndex++];
	sinusInfoMenu = &menuPool[menuPoolIndex++];

//	passwordMenu->type = MENU_TYPE_PASSWORD;
//	passwordMenu->parent = rootMenu;
//	passwordMenu->buttonHandler = passwordMenu_handle_button_press;
//
//	///////////////////////////////////////////////////////////////////////
	idleMenu->parent = idleMenu;
	idleMenu->type = MENU_TYPE_IDLE;
	idleMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_FAULT_IND, LANG_EN);
	idleMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_FAULT_IND, LANG_HE);
	idleMenu->buttonHandler = idle_menu_handler;

//	///////////////////////////////////////////////
    rootMenu->parent = idleMenu;
    rootMenu->type = MENU_TYPE_LIST;
    rootMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_MENU, LANG_EN);
	rootMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_MENU, LANG_HE);
    rootMenu->buttonHandler = handle_button_press;
    rootMenu->items[0] = (MenuItem){ .name = {
    		get_root_menu_items_str(STR_ROOT_ITEM_SIREN, LANG_EN),
    		get_root_menu_items_str(STR_ROOT_ITEM_SIREN, LANG_HE) },
			.prepareAction = &MenuLoadSDCardSirens,
			.submenu = sirenMenu };
    rootMenu->items[1] = (MenuItem) { .name = {
    		get_root_menu_items_str(STR_ROOT_ITEM_MESSAGES, LANG_EN),
			get_root_menu_items_str(STR_ROOT_ITEM_MESSAGES, LANG_HE) },
			.prepareAction = &MenuLoadSDCardMessages,
			.submenu = messagesMenu };
    rootMenu->items[2] = (MenuItem){ .name = {
    		get_root_menu_items_str(STR_ROOT_ITEM_ANNOUNCEMENT, LANG_EN),
			get_root_menu_items_str(STR_ROOT_ITEM_ANNOUNCEMENT, LANG_HE) },
			.submenu = announcementMenu };
    rootMenu->items[3] = (MenuItem){ .name = {
    		get_root_menu_items_str(STR_ROOT_ITEM_TESTS, LANG_EN),
			get_root_menu_items_str(STR_ROOT_ITEM_TESTS, LANG_HE) },
			.submenu = testMenu };
    rootMenu->items[4] = (MenuItem){ .name = {
    		get_root_menu_items_str(STR_ROOT_ITEM_REPORT, LANG_EN),
			get_root_menu_items_str(STR_ROOT_ITEM_REPORT, LANG_HE) },
			.submenu = reportMenu };
    rootMenu->items[5] = (MenuItem){ .name = {
    		get_root_menu_items_str(STR_ROOT_ITEM_MAINTENANCE, LANG_EN),
			get_root_menu_items_str(STR_ROOT_ITEM_MAINTENANCE, LANG_HE) },
			.submenu = maintenanceMenu };
	rootMenu->itemCount = ROOT_MENU_ITEM_COUNT;

//    /////////////////////////////////////////
    // "Siren"
    sirenMenu->parent = rootMenu;
    sirenMenu->type = MENU_TYPE_LIST;
    sirenMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_SIREN, LANG_EN);
    sirenMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_SIREN, LANG_HE);
    sirenMenu->buttonHandler = handle_button_press;

    alarm_info_menu->parent = sirenMenu;
    alarm_info_menu->type = MENU_TYPE_SIREN_INFO;
    alarm_info_menu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_ALARM_INFO, LANG_EN);
    alarm_info_menu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_ALARM_INFO, LANG_HE);
    alarm_info_menu->buttonHandler = alarm_info_menu_handler;

//    //-----------------------------------------------------------------------------------------------------------
	messagesMenu->parent = rootMenu;
	messagesMenu->type = MENU_TYPE_LIST;
	messagesMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_MESSAGES, LANG_EN);
	messagesMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_MESSAGES, LANG_HE);
	messagesMenu->buttonHandler = handle_button_press;

	// play message
	messagePlayMenu->parent = messagesMenu;
	messagePlayMenu->type = MENU_TYPE_MESSAGE_PLAY;
	messagePlayMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_PLAY_MESSAGES, LANG_EN);
    messagePlayMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_PLAY_MESSAGES, LANG_HE);
	messagePlayMenu->imageData = &menu_speaker_img;
	messagePlayMenu->buttonHandler = handle_button_press;

//    //-----------------------------------------------------------------------------------------------------------
	announcementMenu->parent = rootMenu;
	announcementMenu->currentSelection = 0;
	announcementMenu->itemCount = 0;
	announcementMenu->scrollOffset = 0;
	announcementMenu->type = MENU_TYPE_ANNOUNCEMENT;
	announcementMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_ANNOUNCEMENT, LANG_EN);
	announcementMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_ANNOUNCEMENT, LANG_HE);
	announcementMenu->imageData = &menu_microfon_img;
//	announcementMenu->buttonHandler = VolumeControlButtonHandler;
	announcementMenu->buttonHandler = handle_button_press;

//	// VolumeIndicator_Init(&volumeIndicator, 30, 250, 420, 30,
//	// 		             //30,
//	// 					 system_get_volume(),
//	// 					 COLOR_DARKGRAY, 0x00FF, 0xC618, 0x0D00, 1, 1);


	testMenu->parent = rootMenu;
	testMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_TEST, LANG_EN);
	testMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_TEST, LANG_HE);
	testMenu->type = MENU_TYPE_LIST;
	testMenu->buttonHandler = handle_button_press;
	testMenu->items[0] = (MenuItem){ .name = {
			get_test_menu_items_str(STR_TEST_ITEM_SILENT_TEST, LANG_EN),
			get_test_menu_items_str(STR_TEST_ITEM_SILENT_TEST, LANG_HE) },
			.postAction = &RunSilentTest };
	testMenu->items[1] = (MenuItem){ .name = {
			get_test_menu_items_str(STR_TEST_ITEM_BATTERIES_TEST, LANG_EN),
			get_test_menu_items_str(STR_TEST_ITEM_BATTERIES_TEST, LANG_HE) },
			.postAction = &RunBatteriesTest,
			.submenu = batteriesTestMenu };
	testMenu->items[2] = (MenuItem){ .name = {
			get_test_menu_items_str(STR_TEST_ITEM_APLIFIERS_TEST, LANG_EN),
			get_test_menu_items_str(STR_TEST_ITEM_APLIFIERS_TEST, LANG_HE) },
			.postAction = &RunAmplifiresTest,
			.submenu = apmplifiresTestMunu };
	testMenu->items[3] = (MenuItem){ .name = {
			get_test_menu_items_str(STR_TEST_ITEM_DRIVERS_TEST, LANG_EN),
			get_test_menu_items_str(STR_TEST_ITEM_DRIVERS_TEST, LANG_HE) },
			.postAction = &RunDriversTest,
			.submenu = driversTestMenu };
	testMenu->items[4] = (MenuItem){ .name = {
			get_test_menu_items_str(STR_TEST_ITEM_SINUS_TEST, LANG_EN),
			get_test_menu_items_str(STR_TEST_ITEM_SINUS_TEST, LANG_HE) },
			.prepareAction = &prepare_sinuse_items,
			.submenu = sinusMenu };
	testMenu->itemCount = TEST_MENU_ITEM_COUNT;

//	//---------------------------------------------------------------
	batteriesTestMenu->parent = testMenu;
	batteriesTestMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_BATTERIES_TEST, LANG_EN);
	batteriesTestMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_BATTERIES_TEST, LANG_HE);
	batteriesTestMenu->type = MENU_TYPE_TEST_BAT;
	batteriesTestMenu->buttonHandler = handle_button_press;

//
//	//--------------------------------------------------------
	apmplifiresTestMunu->parent = testMenu;
	apmplifiresTestMunu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_APLIFIERS_TEST, LANG_EN);
	apmplifiresTestMunu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_APLIFIERS_TEST, LANG_HE);
	apmplifiresTestMunu->type = MENU_TYPE_TEST_AMP;
	apmplifiresTestMunu->buttonHandler = handle_button_press;

//
//	//-----------------------------------
	driversTestMenu->parent = testMenu;
	driversTestMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_DRIVERS_TEST, LANG_EN);
	driversTestMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_DRIVERS_TEST, LANG_HE);
	driversTestMenu->type = MENU_TYPE_TEST_DRIV;
	driversTestMenu->buttonHandler = handle_button_press;

	sinusMenu->parent = testMenu;
	sinusMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_GENERATE_SINUS, LANG_EN);
	sinusMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_GENERATE_SINUS, LANG_HE);
	sinusMenu->type = MENU_TYPE_LIST;
	sinusMenu->buttonHandler = handle_button_press;

	sinusInfoMenu->parent = sinusMenu;
	sinusInfoMenu->type = MENU_TYPE_SIREN_INFO;
	sinusInfoMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_SINUS_INFO, LANG_EN);
    sinusInfoMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_SINUS_INFO, LANG_HE);
//	sinusInfoMenu->buttonHandler = sinus_info_menu_handler;
	sinusInfoMenu->buttonHandler = handle_button_press;

//	////////////////////////////////////////////////////////
	reportMenu->parent = rootMenu;
	reportMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_REPORT, LANG_EN);
	reportMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_REPORT, LANG_HE);
	reportMenu->type = MENU_TYPE_REPORT;
	reportMenu->buttonHandler = handle_button_press;

	maintenanceMenu->parent = rootMenu;
	maintenanceMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_MAINTENANCE, LANG_EN);
    maintenanceMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_MAINTENANCE, LANG_HE);
	maintenanceMenu->type = MENU_TYPE_LIST;
	maintenanceMenu->buttonHandler = handle_button_press;
	maintenanceMenu->items[0] = (MenuItem){ .name = {
			get_maintenance_menu_items_str(STR_TIME_AND_DATE, LANG_EN),
			get_maintenance_menu_items_str(STR_TIME_AND_DATE, LANG_HE) },
			.prepareAction = &prepare_clock,
			.postAction = &run_clock };
	maintenanceMenu->items[1] = (MenuItem){ .name = {
			get_maintenance_menu_items_str(STR_LANGUAGES, LANG_EN),
			get_maintenance_menu_items_str(STR_LANGUAGES, LANG_HE) },
			.prepareAction = &menu_init_language,
			.submenu = languageMenu };
	maintenanceMenu->itemCount = MAINTENCE_MENU_ITEM_COUNT;

	clockMenu->parent = maintenanceMenu;
	clockMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_TIME_AND_DATE, LANG_EN);
	clockMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_TIME_AND_DATE, LANG_HE);
	clockMenu->type = MENU_TYPE_CLOCK;
	clockMenu->buttonHandler = clockMenu_handle_button_press;

	languageMenu->parent = maintenanceMenu;
	languageMenu->screenText[LANG_EN] = get_menu_header_str(STR_HEADER_LANGUAGES, LANG_EN);
	languageMenu->screenText[LANG_HE] = get_menu_header_str(STR_HEADER_LANGUAGES, LANG_HE);
	languageMenu->type = MENU_TYPE_LIST;
	languageMenu->buttonHandler = languageMenu_handle_button_press;


	BLK_ON();
	isBacklightOn = true;

    currentMenu = rootMenu;

	draw_status_bar();
	update_date_time();
	draw_menuScreen(true);
}


static void menu_draw_image(Menu *menu)
{
	if (!menu->imageData) return;

	hx8357_draw_image(menu->imageData->x, menu->imageData->y,
					  menu->imageData->w, menu->imageData->h,
					  (const uint16_t*)menu->imageData->image);
}

//void MenuActionSystemReset(void)
//{
//
//    NVIC_SystemReset();
//}
//
//
////#action
//void PlayMessageStart(void)
//{
//	uint8_t index = currentMenu->currentSelection;
//	if (!currentMenu || index >= currentMenu->itemCount) return;
//
//	const char* filepath = currentMenu->items[index].filepath;
//	if (!filepath || !messagePlayMenu) return;
//
//	messagePlayMenu->textFilename = filepath;
//	currentMenu = messagePlayMenu;
//}

//void PlayMessageStartPost(void)
//{
//	isPlayAudioFile = true;
//	xQueueSend(audioQueue, &messagePlayMenu->textFilename, 0);
//}

bool PlayMessageProgress(const uint8_t value)
{
	if (isPlayAudioFile == false) return false;

	MenuDrawProgress(value);
	//MenuDrawProgress((LCD_GetWidth() - PROGRESS_BAR_W) / 2, PROGRESS_BAR_Y, PROGRESS_BAR_W, PROGRESS_BAR_HEIGHT, value);

	//MenuDrawProgress2(&bar, value);
	return true;
}

void PlayMessageEnd(void)
{
	isPlayAudioFile = false;
	currentMenu = currentMenu->parent;
	draw_menuScreen(true);
}

void MenuShowMessages(void)
{
    if (!messagesMenu || !messagePlayMenu) return;

    clear_menu(messagesMenu);

//    DIR dir;
//    FILINFO fno;
//    FRESULT res;
//    uint8_t count = 0;
//
//    res = f_opendir(&dir, "0:/message");
//    if (res != FR_OK) return;
//
//    while ((f_readdir(&dir, &fno) == FR_OK) && fno.fname[0] && count < MAX_MESSAGES) {
//        if (strncmp(fno.fname, "msg_", 4) == 0) {
//            strncpy(messageFilenames[count], fno.fname, MAX_FILENAME_LEN);
//
//            messagesMenu->items[count].name[LANG_EN] = messageFilenames[count];
//            messagesMenu->items[count].name[LANG_HE] = messageFilenames[count];
//            messagesMenu->items[count].prepareAction = &PlaySelectedMessageWrapper;
//            messagesMenu->items[count].submenu = messagePlayMenu;
//            messagesMenu->items[count].filepath = messageFilenames[count];
//
//            count++;
//        }
//    }
//    f_closedir(&dir);
//
//    messagesMenu->itemCount = count;
//    messagesMenu->currentSelection = 0;
//    currentMenu = messagesMenu;

}


void RunSilentTest(void)
{
	currentMenu = batteriesTestMenu;
	clear_position(currentMenu);
	draw_menuScreen(true);
	RunBatteriesTest();

	//for(int i =0; i<2000;i++)
	  {
	    osDelay(2000);
	  }

	currentMenu = apmplifiresTestMunu;
	clear_position(currentMenu);
	draw_menuScreen(true);
	RunAmplifiresTest();

	//for(int i =0; i<2000;i++)
	{
		osDelay(2000);
	}

	currentMenu = driversTestMenu;
	clear_position(currentMenu);
	draw_menuScreen(true);
	RunDriversTest();

	//for(int i =0; i<2000;i++)
	{
		osDelay(2000);
	}


	currentMenu = testMenu; //driversTestMenu->parent;
	clear_position(currentMenu);
	draw_menuScreen(true);
}

void RunBatteriesTest(void)
{

	BatteriesDisplay_SetStatus(0, true);
	BatteriesDisplay_SetStatus(1, false);
	BatteriesDisplay_SetStatus(2, true);
}

void RunAmplifiresTest(void)
{

	TestAmplDisplay_SetStatus(0, true);
	TestAmplDisplay_SetStatus(1, true);
	TestAmplDisplay_SetStatus(2, true);
	TestAmplDisplay_SetStatus(3, true);
	TestAmplDisplay_SetStatus(4, false);
	TestAmplDisplay_SetStatus(5, true);
	TestAmplDisplay_SetStatus(6, true);
	TestAmplDisplay_SetStatus(7, true);
	TestAmplDisplay_SetStatus(8, false);
	TestAmplDisplay_SetStatus(9, true);
}

void RunDriversTest(void)
{

}

void prepare_clock(void)
{
	HAL_RTC_GetTime(&hrtc, &clock.sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &clock.sDate, RTC_FORMAT_BIN);
}

void run_clock(void)
{
	clock.change_value = 0;
	clock.current_symbol = 0;
	currentMenu = clockMenu;
	draw_menuScreen(true);
}

void MenuLoadSDCardSirens(void)
{
	if (!sirenMenu) return;

	clear_menu(sirenMenu);

	static const char* list[] =
	{
	  "ALARM 1",
	  "ALARM 2",
	  "ALARM 3",
	  "ALARM 4",
	  "ALARM 5",
	  "ALARM 6",
	  "ALARM 7",
	  "ALARM 8",
	  "ALARM 9",
	  "ALARM 10",
	  "ALARM 11",
	  "ALARM 12",
	  "ALARM 13",
	  "ALARM 14"
	};

	const uint8_t count = sizeof(list) / sizeof(list[0]);

	for (uint8_t i = 0; i < count && i < MAX_MENU_ITEMS; ++i)
	{
		strncpy(messageFilenames[i], list[i], MAX_FILENAME_LEN);

		MenuItem* item = &sirenMenu->items[i];
		for (uint8_t j = 0; j < LANG_COUNT; j++)
		{
			item->name[j] = messageFilenames[i];
		}
		item->menu = alarm_info_menu;
		item->postAction = &siren_post_action;
	}

	sirenMenu->itemCount = count;
}

void MenuLoadSDCardMessages(void)
{
    if (!messagesMenu || !messagePlayMenu) return;

   clear_menu(messagesMenu);

    static const char* dummyFilenames[] = {
        "msg1_hello.wav",
        "msg2_alert.wav",
        "msg3_night.wav",
        "msg4_test.wav",
		"msg5_test-00.wav",
		"msg6_test-11.wav",
		"msg7_test-22.wav",
		"msg8_test-33.wav",
		"msg9_test-44.wav",
		"msg10_test-55.wav",
		"msg11_test-66.wav",
		"msg12_test-77.wav",
		"msg13_test-77.wav",
		"msg14_test-77.wav",
		"msg15_test-77.wav",
		"msg16_test-77.wav"
    };

    const uint8_t count = sizeof(dummyFilenames) / sizeof(dummyFilenames[0]);

    for (uint8_t i = 0; i < count && i < MAX_MENU_ITEMS; ++i)
    {
        strncpy(messageFilenames[i], dummyFilenames[i], MAX_FILENAME_LEN);

        MenuItem* item = &messagesMenu->items[i];
		for (uint8_t j = 0; j < LANG_COUNT; j++)
		{
			item->name[j] = messageFilenames[i];
		}
		item->menu = alarm_info_menu;
		item->postAction = &siren_post_action;
    }

    messagesMenu->itemCount = count;
}

static void prepare_sinuse_items(void)
{
    if (!sinusMenu) return;

	clear_menu(sinusMenu);

	for (uint8_t i = 0; i < SINUS_ITEM_COUNT; i++)
	{
    	MenuItem* item = &sinusMenu->items[i];
    	for (uint8_t j = 0; j < LANG_COUNT; j++)
		{
    		item->name[j] = get_sinus_menu_items_str(i, j);
		}
    	item->menu = sinusInfoMenu;
    	item->postAction = &siren_post_action;
    }

	sinusMenu->itemCount = SINUS_ITEM_COUNT;
}

void menu_init_language(void)
{
	if (!languageMenu) return;

	clear_menu(languageMenu);

	for (uint8_t i = 0; i < LANG_COUNT && i < MAX_MENU_ITEMS; ++i)
	{
		char langSel = (GetLanguage() == i) ? '*' : ' ';
		sprintf(messageFilenames[i], "%s %c", LanguageToString(i), langSel);

		for (uint8_t j = 0; j < LANG_COUNT; j++)
		{
			languageMenu->items[i].name[j] = messageFilenames[i];
		}
	}

	languageMenu->itemCount = LANG_COUNT;
}

static void siren_post_action(void)
{
	if (!currentMenu ||
			currentMenu->currentSelection >= currentMenu->itemCount ||
			AUDIO_PRIORITY_LOW < player.current_priority)
		return;

	MenuItem* item = &currentMenu->items[currentMenu->currentSelection];

	if (!item || !item->menu) return;

	player.type_input = AUDIO_SIN;
	player.current_sin = (SinTask_t)currentMenu->currentSelection;
	player.audio_state = AUDIO_START;
	player.priority = AUDIO_PRIORITY_LOW;
	xQueueSend(xAudioQueueHandle, &player.audio_state, portMAX_DELAY);

	currentMenu = item->menu;
	clear_position(currentMenu);

	currentMenu->textFilename = item->name[GetLanguage()];

	MenuResetProgressBar();

	draw_menuScreen(true);
}

 void IncreaseVolume(void)
 {
     if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT) {
         uint8_t level = volumeIndicator.level;

         if (level < volumeIndicator.numBars) {
             level++;
             VolumeIndicator_SetLevel(level);
 			//system_set_volume(volumeIndicator.level);
         }
     }
 }

 void DecreaseVolume(void)
 {
     if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT) {
         uint8_t level = volumeIndicator.level;

         if (level > 0) {
             level--;
             VolumeIndicator_SetLevel(level);
// 			system_set_volume(volumeIndicator.level);
         }
     }
 }
//
////
////void IncreaseVolume(void)
////{
////    if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT) {
////        uint8_t current_bars = volumeIndicator.level;
////
////        if (current_bars < NUM_VOLUME_BARS) {
////            uint8_t new_bars = current_bars + 1;
////            int new_volume_db = volume_bars_to_db(new_bars);
////
////
////
////            VolumeIndicator_StepUp(&volumeIndicator);
////            system_set_volume(new_volume_db);
////
////        }
////    }
////}
////
////void DecreaseVolume(void)
////{
////    if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT) {
////        uint8_t current_bars = volumeIndicator.level;
////
////        if (current_bars > 0) {
////            uint8_t new_bars = current_bars - 1;
////            int new_volume_db = volume_bars_to_db(new_bars);
////
////
////
////            VolumeIndicator_StepDown(&volumeIndicator);
////            system_set_volume(new_volume_db);
////        }
////    }
////}
//
//void IncreaseVolume(void)
//{
//    if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT) {
//    	if (system_get_volume() >= MAX_VOLUME){
//    		return;
//    	}
//
//       int new_volume = system_get_volume() + VOLUME_STEP;
//
//        if (new_volume <= MAX_VOLUME) {
//        	system_set_volume(new_volume);
//
//        	VolumeIndicator_StepUp(&volumeIndicator);
//        }
//    }
//}
//
//void DecreaseVolume(void)
//{
//    if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT) {
//        int new_volume = system_get_volume() - VOLUME_STEP;
//        if (new_volume >= MIN_VOLUME) {
//        	system_set_volume(new_volume);
//        	VolumeIndicator_StepDown(&volumeIndicator);
//            //
//
//
//        }
//    }
//}

void Draw_MENU_TYPE_IDLE()
{
	FaultsDisplay_DrawAll(MENU_BASE_X, IDLE_Y_POS);
}

void Draw_MENU_TYPE_ANNOUNCEMENT(void)
{
	menu_draw_image(currentMenu);
	VolumeIndicator_Draw(&volumeIndicator);

	// Set the initial level
	//uint8_t initial_bars = volume_db_to_bars(system_get_volume());
	//VolumeIndicator_SetLevel(&volumeIndicator, initial_bars);

}

void Draw_MENU_TYPE_SIREN_INFO(void)
{
	if (currentMenu->textFilename)
	{
		hx8357_write_alignedX_string(0, SIREN_Y_POS, currentMenu->textFilename, &Font_11x18, COLOR_MAGENTA, COLOR_BLACK, ALIGN_CENTER);
	}

	menu_draw_image(currentMenu);

	MenuDrawProgress(0);

	osDelay(3);
}

void Draw_MENU_TYPE_REPORT(void)
{
	ReportIndicator_DrawAll(MENU_BASE_X, REPORT_Y_POS);
}

void Draw_MENU_TYPE_TEST_BAT(void)
{
	BatteriesDisplay_DrawAll(MENU_BASE_X, BAT_Y_POS);
}

void Draw_MENU_TYPE_TEST_DRIV(void)
{
	TestDrvDisplay_DrawAll(MENU_BASE_X, DRV_Y_POS);
}

void Draw_MENU_TYPE_TEST_AMP(void)
{
	TestAmplDisplay_DrawAll(MENU_BASE_X, AMP_Y_POS);
}

void Draw_MENU_TYPE_MESSAGE_PLAY(void)
{
	menu_draw_image(currentMenu);

	isPlayAudioFile = true;
	MenuDrawProgress(0);

	osDelay(3);
}

void draw_menu_clock(void)
{
	RTC_TimeTypeDef* sTime = &clock.sTime;
	RTC_DateTypeDef* sDate = &clock.sDate;

	if (clock.change_value != 0)
	{
		if(clock.current_symbol == 0)
		{
			int8_t data = sDate->Date;
			data += clock.change_value;

			if (data > 31) sDate->Date = 1;
			else if (data < 1) sDate->Date = 31;
			else sDate->Date = data;
		}
		if(clock.current_symbol == 1)
		{
			int8_t data = sDate->Month;
			data += clock.change_value;

			if (data > 12) sDate->Month = 1;
			else if (data < 1) sDate->Month = 12;
			else sDate->Month = data;
		}
		if(clock.current_symbol == 2)
		{
			int8_t data = sDate->Year;
			data += clock.change_value;

			if (data > 99) sDate->Year = 0;
			else if (data < 0) sDate->Year = 99;
			else sDate->Year = data;
		}

		if(clock.current_symbol == 3)
		{
			int8_t data = sTime->Hours;
			data += clock.change_value;

			if (data > 23) sTime->Hours = 0;
			else if (data < 0) sTime->Hours = 23;
			else sTime->Hours = data;
		}

		if(clock.current_symbol == 4)
		{
			int8_t data = sTime->Minutes;
			data += clock.change_value;

			if (data > 59) sTime->Minutes = 0;
			else if (data < 0) sTime->Minutes = 59;
			else sTime->Minutes = data;
		}
		if(clock.current_symbol == 5)
		{
			int8_t data = sTime->Seconds;
			data += clock.change_value;

			if (data > 59) sTime->Seconds = 0;
			else if (data < 0) sTime->Seconds = 59;
			else sTime->Seconds = data;
		}

		HAL_RTC_SetTime(&hrtc, sTime, RTC_FORMAT_BIN);
		HAL_RTC_SetDate(&hrtc, sDate, RTC_FORMAT_BIN);
		clock.change_value = 0;
	}

	FontDef* font = &Font_16x26;
	const uint8_t fontW = font->width;
	const uint16_t baseX = 99;
	const uint16_t y     = 153;

	char buf[32];
	snprintf(buf, sizeof(buf), "%02d/%02d/%02d  %02d:%02d:%02d",
			 sDate->Date, sDate->Month, sDate->Year,
			 sTime->Hours, sTime->Minutes, sTime->Seconds);

	Print_Msg(buf);
	Print_Msg("\r\n");

	hx8357_write_alignedX_string(baseX, y, buf, font, COLOR_YELLOW, COLOR_BLACK, ALIGN_LEFT);

	uint16_t field_color   = COLOR_YELLOW;
	uint16_t field_bg      = COLOR_BLUE;
	uint16_t field_offset  = 0;

	const uint8_t pos_in_str[CLOCK_MAX_SYMBOLS]   = {0,3,6,10,13,16};

	if (clock.current_symbol < CLOCK_MAX_SYMBOLS)
	{
		field_offset = pos_in_str[clock.current_symbol] * fontW;

		char field_str[4];
		switch (clock.current_symbol)
		{
			case 0: snprintf(field_str, sizeof(field_str), "%02d", sDate->Date);    break;
			case 1: snprintf(field_str, sizeof(field_str), "%02d", sDate->Month);   break;
			case 2: snprintf(field_str, sizeof(field_str), "%02d", sDate->Year);    break;
			case 3: snprintf(field_str, sizeof(field_str), "%02d", sTime->Hours);   break;
			case 4: snprintf(field_str, sizeof(field_str), "%02d", sTime->Minutes); break;
			case 5: snprintf(field_str, sizeof(field_str), "%02d", sTime->Seconds); break;
		}

		hx8357_write_alignedX_string(baseX + field_offset, y, field_str, font, field_color, field_bg, ALIGN_LEFT);
	}
}

static void display_menu_item(uint8_t visualIndex, uint8_t index, const MenuItem* item, bool selected, bool dummy)
{
	uint16_t y_pos = MENU_BASE_Y + (visualIndex * MENU_ITEM_HEIGHT);
    uint16_t text_color = selected ? COLOR_WHITE : COLOR_GREEN;
    uint16_t bg_color   = selected ? COLOR_BLUE  : COLOR_BLACK;

    hx8357_fill_rect(MENU_BASE_X , y_pos,  hx8357_get_width() - (MENU_BASE_X*2), MENU_ITEM_HEIGHT, bg_color);

    char upd_text[MAX_FILENAME_LEN];
    Alignment align;
    Language lang = GetLanguage();
    const char* text = (dummy) ? "..." : item->name[GetLanguage()];

    if (currentMenu == languageMenu || currentMenu == maintenanceMenu || lang == LANG_EN)
    {
    	sprintf(upd_text, "%d. %s", index+1, text);
    	align = ALIGN_LEFT;
    }
	else if(lang == LANG_HE)
	{
		sprintf(upd_text, "%s .%d", text, index+1);
    	align = ALIGN_RIGHT;
	}

    hx8357_write_alignedX_string(MENU_BASE_X, y_pos, upd_text, &Font_11x18, text_color, bg_color, align);
}

void Draw_MENU_TYPE_LIST()
{
	uint8_t old_selection = currentMenu->oldSelection;
	uint8_t old_offset = currentMenu->oldOffset;

	uint8_t selection = currentMenu->currentSelection;
	uint8_t item_count = currentMenu->itemCount;
	uint8_t offset = currentMenu->scrollOffset;

	uint8_t visible_count = (item_count > MAX_VISIBLE_ITEMS) ? MAX_VISIBLE_ITEMS : item_count;
	uint8_t end_index = offset + visible_count;

	bool show_scroll_up = offset > 0;
	bool show_scroll_down = (offset + MAX_VISIBLE_ITEMS) < item_count;

	bool selection_changed = old_selection != selection;
	bool offset_changed = old_offset != offset;

/*
    if (!selection_changed && !offset_changed)
    {
        return;
    }
*/
    if (selection_changed && !offset_changed)
    {
        if (old_selection >= offset && old_selection < end_index)
        {
            uint8_t visual_index = old_selection - offset;
            display_menu_item(visual_index, old_selection, &currentMenu->items[old_selection], false, false);
        }

        if (selection >= offset && selection < end_index)
        {
            uint8_t visual_index = selection - offset;
            display_menu_item(visual_index, selection, &currentMenu->items[selection], true, false);
        }
    }
    else
    {
        for (uint8_t i = offset; i < end_index; ++i)
        {
            uint8_t visual_index = i - offset;
			bool dummy = (show_scroll_up && visual_index == 0) || (show_scroll_down && visual_index == visible_count-1);
            display_menu_item(visual_index, i, &currentMenu->items[i], (i == selection), dummy);
            osDelay(3);
        }
    }

    currentMenu->oldSelection = selection;
    currentMenu->oldOffset = offset;
}

void draw_menuScreen(bool forceFullRedraw)
{
    static Menu* prevMenu = NULL;

    if (forceFullRedraw || prevMenu != currentMenu)
    {
        uint16_t bg_color = COLOR_BLACK;

        hx8357_fill_rect(MENU_BASE_X, TITLE_MENU_Y_POS,
						 hx8357_get_width() - (MENU_BASE_X*2),
						 hx8357_get_height() - TITLE_MENU_Y_POS,
						 bg_color);
        osDelay(1);

    	const char* text = currentMenu->screenText[GetLanguage()];
    	if (text) {
    		hx8357_write_alignedX_string(0, TITLE_MENU_Y_POS, text, &Font_11x18, COLOR_WHITE, bg_color, ALIGN_CENTER);
    	}
    }

	if (currentMenu->type == MENU_TYPE_IDLE)
	{
		Draw_MENU_TYPE_IDLE();
	}
//	else if (currentMenu->type == MENU_TYPE_PASSWORD)
//	{
//		 Draw_MENU_TYPE_PASSWORD();
//	}
	else if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT)
	{
		Draw_MENU_TYPE_ANNOUNCEMENT();
	}
	else if (currentMenu->type == MENU_TYPE_SIREN_INFO)
	{
		Draw_MENU_TYPE_SIREN_INFO();
	}
	else if (currentMenu->type == MENU_TYPE_REPORT)
	{
		Draw_MENU_TYPE_REPORT();
	}
	else if (currentMenu->type == MENU_TYPE_TEST_BAT)
	{
		Draw_MENU_TYPE_TEST_BAT();
	}
	else if (currentMenu->type == MENU_TYPE_TEST_DRIV)
	{
		Draw_MENU_TYPE_TEST_DRIV();
	}
	else if (currentMenu->type == MENU_TYPE_TEST_AMP)
	{
		Draw_MENU_TYPE_TEST_AMP();
	}
	else if (currentMenu->type == MENU_TYPE_MESSAGE_PLAY)
	{
		Draw_MENU_TYPE_MESSAGE_PLAY();
	}
	else if (currentMenu->type == MENU_TYPE_LIST)
	{
		Draw_MENU_TYPE_LIST();
	}
	else if(currentMenu->type == MENU_TYPE_CLOCK)
	{
		draw_menu_clock();
	}

	prevMenu = currentMenu;

    osDelay(1);

#ifdef DEBUG_PRINT_BUTTON_STATE_2
   // DrawDebugInfo(&lastButtonEvent);
#endif
}

//static char debugInfo[50];
//
//void DrawDebugInfo(const ButtonEvent_t* event)
//{
//return;
//	//LCD_FillRectangle(0, DEBUG_INFO_Y, LCD_GetWidth(), LCD_GetHeight(), debug_bg_color);
//
//
//	sprintf(debugInfo,
//			//sizeof(debugInfo),
//		"Btn:%s %s Level:%d Sel:%d/%d",
//		ButtonToString(event->button),
//		ButtonActionToString(event->action),
//		0,//currentLevel,
//		currentMenu->currentSelection + 1,
//		currentMenu->itemCount);
//
//	LCD_WriteString(10, DEBUG_INFO_Y, debugInfo, &Font_7x10, COLOR_WHITE, COLOR_DARKGRAY);
//}



void draw_status_bar()
{
	char serialStr[20];
	hx8357_fill_rect(0, 0, hx8357_get_width(), STATUS_BAR_LINE_Y_POS, COLOR_BLACK);
	hx8357_write_alignedX_string(0, LOGO_Y_POS, "EES-3000", &Font_11x18, COLOR_WHITE, COLOR_BLACK, ALIGN_LEFT);

	snprintf(serialStr, sizeof(serialStr), "Serial: %s", SERIAL_NUMBER);
	hx8357_write_alignedX_string(0, SERIAL_Y_POS, serialStr, &Font_7x10, COLOR_YELLOW, COLOR_BLACK, ALIGN_RIGHT);

	hx8357_fill_rect(0, STATUS_BAR_LINE_Y_POS-1, hx8357_get_width(), 1, COLOR_GRAY);
}

void update_date_time()
{
	if (!isBacklightOn) return;

	if (!player.is_playing) lastInteractionTick++;

	if (currentMenu != idleMenu && lastInteractionTick >= INACTIVITY_TIMEOUT_MS)
	{
		currentMenu = idleMenu;
		draw_menuScreen(true);
		lastInteractionTick = 0;
	}
	else if (currentMenu == idleMenu && isBacklightOn && (lastInteractionTick >= BACKLIGHT_TIMEOUT_MS))
	{
		BLK_OFF();
		isBacklightOn = false;
		lastInteractionTick = 0;
		return;
	}

	char clock_str[64];
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	snprintf(clock_str, sizeof(clock_str), "%02d/%02d/%02d %02d:%02d:%02d",
			sDate.Date, sDate.Month, sDate.Year,
			sTime.Hours, sTime.Minutes, sTime.Seconds);

	hx8357_write_alignedX_string(0, TIME_Y_POS, clock_str, &Font_7x10, COLOR_YELLOW, COLOR_BLACK, ALIGN_RIGHT);
	return;
}

void update_progress_bar(uint8_t value)
{
	if(!player.is_playing || (currentMenu != sinusInfoMenu && currentMenu != alarm_info_menu)) return;
	MenuDrawProgress(value);
}

//void ShowUartCommand(void)
//{
//	//LCD_FillRectangle(0, 0, LCD_GetWidth(), STATUS_BAR_LINE_Y_POS -1, COLOR_BLACK);
//	LCD_WriteString(LOGO_X_POS, LOGO_Y_POS, "EES-3000", &Font_7x10, COLOR_WHITE, COLOR_BLACK);
//
//}

void menu_handle_button(ButtonEvent_t event)
{
	if (!currentMenu || (event.button == BTN_NONE) || (event.action == BA_RELEASED)) return;

	lastInteractionTick = 0;

	if (!isBacklightOn)
	{
		BLK_ON();
		isBacklightOn = true;
		return;
	}

	if (hot_key_handle_button(event)) return;

	if (currentMenu->buttonHandler) {
	    currentMenu->buttonHandler(event);
	    return;
	}
}

bool hot_key_handle_button(ButtonEvent_t event)
{
	switch (event.button)
    {
	case BTN_TEST:
		currentMenu = testMenu;
		clear_position(currentMenu);
		draw_menuScreen(true);
		return true;
	case BTN_ANNOUNCEMENT:
		if (player.current_priority < AUDIO_PRIORITY_LOW) return true;
		player.last_time_announcement = 0;
		player.is_announcement = true;
		currentMenu = announcementMenu;
		clear_position(currentMenu);
		draw_menuScreen(true);
		return true;
	case BTN_MESSAGE:
		MenuLoadSDCardMessages();
		currentMenu = messagesMenu;
		clear_position(currentMenu);
		draw_menuScreen(true);
		return true;
	case BTN_ALARM:
		MenuLoadSDCardSirens();
		currentMenu = sirenMenu;
		clear_position(currentMenu);
		draw_menuScreen(true);
		return true;
	case BTN_ARM:
	    player.last_time_arming = 0;
	    player.is_arming = true;
		return true;
	case BTN_CXL:
		if (!player.is_playing ||
			AUDIO_PRIORITY_LOW < player.current_priority) return true;

		player.priority = AUDIO_PRIORITY_LOW;
		player.audio_state = AUDIO_STOP;
		xQueueSend(xAudioQueueHandle, &player.audio_state, portMAX_DELAY);
		return true;
	default:
		return false;
	}
}

void handle_button_press(ButtonEvent_t event)
{
	if (!currentMenu) return;

    switch(event.button)
    {
        case BTN_UP:
        	button_up_handler();
        	break;
        case BTN_DOWN:
        	button_down_handler();
        	break;
        case BTN_ENTER:
        	button_enter_handler();
        	break;
        case BTN_ESC:
        	button_esc_handler();
        	break;
        default:
        	return;
    }
}


static void button_up_handler(void)
{
	if (!currentMenu || currentMenu->itemCount == 0) return;

	if (currentMenu->currentSelection == 0)
	{
		currentMenu->currentSelection = currentMenu->itemCount-1;
		if(currentMenu->itemCount > MAX_VISIBLE_ITEMS)
			currentMenu->scrollOffset = currentMenu->itemCount-MAX_VISIBLE_ITEMS;
	}
	else
	{
		currentMenu->currentSelection--;

		if (currentMenu->currentSelection < currentMenu->itemCount-MAX_VISIBLE_ITEMS+1 &&
			currentMenu->scrollOffset > 0)
			currentMenu->scrollOffset--;
	}

	draw_menuScreen(false);
}

static void button_down_handler(void)
{
	if (!currentMenu || currentMenu->itemCount == 0) return;

	if (currentMenu->currentSelection >= currentMenu->itemCount - 1)
	{
		currentMenu->currentSelection = 0;
		currentMenu->scrollOffset = 0;
	}
	else
	{
		currentMenu->currentSelection++;

		if (currentMenu->currentSelection > MAX_VISIBLE_ITEMS-2 &&
			currentMenu->currentSelection != currentMenu->itemCount - 1)
			currentMenu->scrollOffset++;
	}

	draw_menuScreen(false);
}

static void button_enter_handler(void)
{
	if (currentMenu->itemCount == 0) return;

	MenuItem* item = &currentMenu->items[currentMenu->currentSelection];
	if (!item) return;

	if (item->prepareAction) {
		item->prepareAction();
	}

	if (item->submenu) {
		currentMenu = item->submenu;
		clear_position(currentMenu);
		draw_menuScreen(true);
	}

	if (item->postAction) {
		item->postAction();
	}
}

static void button_esc_handler(void)
{
	if(player.is_playing)
	{
		if (AUDIO_PRIORITY_LOW < player.current_priority) return;
		player.priority = AUDIO_PRIORITY_LOW;
		player.audio_state = AUDIO_STOP;
		xQueueSend(xAudioQueueHandle, &player.audio_state, portMAX_DELAY);
	}

	if (!currentMenu->parent) return;

	currentMenu = currentMenu->parent;

	draw_menuScreen(true);
}

//void passwordMenu_handle_button_press(ButtonEvent_t event)
//{
//	if ((!passwordMenu) && (currentMenu != passwordMenu)) return;
//
//	switch(event.button)
//	{
//		//case BTN__BACK:	 Password_Reset(true);      break;
//		case BTN_ESC:
//				Password_Backspace();  break;
//		case BTN_ENTER:
//						Password_Enter();
//						if (Password_IsCorrect())
//						{
//							currentMenu = rootMenu;
//							draw_menuScreen(true);
//						}
//						break;
//
//		case BTN_A:
//						Password_AddChar('1'); break;
//		case BTN_B:
//					Password_AddChar('1'); break;
//
//		case BTN_1:          Password_AddChar('1'); break;
//		case BTN_2:          Password_AddChar('2'); break;
//		case BTN_3:          Password_AddChar('3'); break;
//		case BTN_4:          Password_AddChar('4'); break;
//		case BTN_5:          Password_AddChar('5'); break;
//		case BTN_6:          Password_AddChar('6'); break;
//		case BTN_7:          Password_AddChar('7'); break;
//		case BTN_8:          Password_AddChar('8'); break;
//		case BTN_9:          Password_AddChar('9'); break;
//		case BTN_0:          Password_AddChar('0'); break;
//
//	}
//}
//

void idle_menu_handler(ButtonEvent_t event)
{
	if (!rootMenu) return;

	switch (event.button)
    {
	case BTN_ENTER:
		currentMenu = rootMenu;
		clear_position(currentMenu);
		draw_menuScreen(true);
		break;
	default:
		break;
    }
}

void alarm_info_menu_handler(ButtonEvent_t event)
{
	if (!alarm_info_menu) return;

	switch(event.button)
	{
		case BTN_ESC:
			if (alarm_info_menu->parent == NULL) return;
			currentMenu = alarm_info_menu->parent;
			draw_menuScreen(true);
			break;

		default:
			break;
	}
}

//void sinus_info_menu_handler(ButtonEvent_t event)
//{
//	if (!sinusInfoMenu) return;
//
//	switch(event.button)
//	{
//		case BTN_ESC:
//			if (sinusInfoMenu->parent != NULL) {
//				currentMenu = sinusInfoMenu->parent;
//				draw_menuScreen(true);
//			}
//			break;
//		default:
//				return;
//	}
//
//}

//void VolumeControlButtonHandler(ButtonEvent_t event)
//{
//	 if (!currentMenu) return;
//
//	switch (event.button)
//	{
//		case BTN_UP:
//			IncreaseVolume(); // VOLUME +
//			break;
//		case BTN_DOWN:
//			DecreaseVolume(); // VOLUME -
//			break;
//		case BTN_ENTER:
//			break;
//		case BTN_ESC:
//			currentMenu = currentMenu->parent;
//			draw_menuScreen(true);
//			break;
//
//		default:
//			return;
//	}
//
//}

void clockMenu_handle_button_press(ButtonEvent_t event)
{
	if (!clockMenu) return;

	switch(event.button)
	{
		case BTN_ENTER:
			clock.current_symbol++;
			if (clock.current_symbol >= CLOCK_MAX_SYMBOLS) clock.current_symbol = 0;
			draw_menu_clock();
			return;
		case BTN_UP:
			clock.change_value = 1;
			draw_menu_clock();
			return;
		case BTN_DOWN:
			clock.change_value = -1;
			draw_menu_clock();
			return;
		default:
			break;
	}
	handle_button_press(event);
}

void languageMenu_handle_button_press(ButtonEvent_t event)
{
	if (!languageMenu || languageMenu->itemCount == 0) return;

	switch(event.button)
	{
		case BTN_ENTER:
			SetLanguage((Language)languageMenu->currentSelection);
			menu_init_language();
			draw_menuScreen(false);
			return;
		default:
			break;
	}
	handle_button_press(event);
}



