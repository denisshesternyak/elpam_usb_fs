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

uint32_t lastInteractionTick = 0;
#define INACTIVITY_TIMEOUT_MS 	20000
#define BACKLIGHT_TIMEOUT_MS 	10000
//
bool isPlayAudioFile = false;
bool isIdle = false;
bool isBacklightOn = true;

#define MAX_MENU_POOL 40

static Menu menuPool[MAX_MENU_POOL];
uint8_t menuPoolIndex = 0;

extern osMessageQueueId_t xAudioQueueHandle;


#define MAX_FILENAME_LEN 64

char messageFilenames[MAX_MENU_ITEMS][MAX_FILENAME_LEN];
const char* selectedFile = NULL;

extern Player_t player;

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
Menu* alarmInfoMenu = NULL;
Menu* maintenanceMenu = NULL;
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
void MenuLoadSDCardSirens(void);
void MenuLoadSDCardMessages(void);
static void MenuInitLanguage(void);
static void PrepareSinuseItems(void);
///////////////////////////////////////////////////////////////////

//static void BLK_ON()  { HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET); }
//static void BLK_OFF() { HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET); }
static void handle_button_up(void);
static void handle_button_down(void);
static void handle_button_enter(void);
static void handle_button_esc(void);

void _basic_init_menu(Menu* m)
{
    if (m == NULL)
        return;

    m->parent = NULL;
    m->type = MENU_TYPE_UNK;
    m->scrollOffset = 0;
    m->itemCount = 0;
    m->currentSelection = 0;
    m->oldSelection = 0;
    m->oldOffset = 0;

    for (int i = 0; i < MAX_MENU_ITEMS; ++i)
    {
        for (int lang = 0; lang < LANG_COUNT; ++lang)
        {
            m->items[i].name[lang] = NULL;
        }
        m->items[i].prepareAction = NULL;
        m->items[i].postAction = NULL;
        m->items[i].submenu = NULL;
        m->items[i].filepath = NULL;
    }

    for (int lang = 0; lang < LANG_COUNT; ++lang) {
        m->screenText[lang] = NULL;
    }

    m->textFilename = NULL;
    m->imageData = NULL;
    m->buttonHandler = NULL;
}

void InitMenuPool(void)
{
	for (int i = 0; i < MAX_MENU_POOL; ++i) {
		_basic_init_menu(&menuPool[i]);
	}
	menuPoolIndex = 0;
}

void Menu_Init(void)
{
	InitMenuPool();

//	passwordMenu = &menuPool[menuPoolIndex++];
	idleMenu = &menuPool[menuPoolIndex++];
    rootMenu = &menuPool[menuPoolIndex++];
    sirenMenu = &menuPool[menuPoolIndex++];
    alarmInfoMenu = &menuPool[menuPoolIndex++];
	messagesMenu = &menuPool[menuPoolIndex++];
	messagePlayMenu = &menuPool[menuPoolIndex++];
	announcementMenu = &menuPool[menuPoolIndex++];
	testMenu = &menuPool[menuPoolIndex++];
	batteriesTestMenu = &menuPool[menuPoolIndex++];
	apmplifiresTestMunu = &menuPool[menuPoolIndex++];
	driversTestMenu = &menuPool[menuPoolIndex++];
	reportMenu = &menuPool[menuPoolIndex++];
	maintenanceMenu = &menuPool[menuPoolIndex++];
	languageMenu = &menuPool[menuPoolIndex++];
	sinusMenu = &menuPool[menuPoolIndex++];
	sinusInfoMenu = &menuPool[menuPoolIndex++];

//	passwordMenu->type = MENU_TYPE_PASSWORD;
//	passwordMenu->parent = rootMenu;
//	passwordMenu->buttonHandler = passwordMenu_HandleButtonPress;
//
//	///////////////////////////////////////////////////////////////////////
	idleMenu->parent = idleMenu;
	idleMenu->type = MENU_TYPE_IDLE;
	idleMenu->scrollOffset = 0;
	idleMenu->currentSelection = 0;
	idleMenu->screenText[LANG_EN] = "### Fault indication ###";
	idleMenu->screenText[LANG_HE] = "### -Fault indication- ###";
	idleMenu->textFilename  = NULL;
	idleMenu->itemCount = 0;
	idleMenu->buttonHandler = HandleButtonPress;

//	///////////////////////////////////////////////
    rootMenu->parent = idleMenu;
    rootMenu->type = MENU_TYPE_LIST;
    rootMenu->scrollOffset = 0;
    rootMenu->currentSelection = 0;
    rootMenu->screenText[LANG_EN] = "### Menu ###";
    rootMenu->screenText[LANG_HE] = "### -Menu- ###";
    rootMenu->textFilename  = NULL;
    rootMenu->buttonHandler = HandleButtonPress;
    rootMenu->items[0] = (MenuItem){ .name = { "1. Siren", "-Siren- .1" }, .prepareAction = &MenuLoadSDCardSirens, .postAction = NULL, .submenu = sirenMenu  };
    rootMenu->items[1] = (MenuItem){ .name = { "2. Messages", "2. הודעות" }, .prepareAction = &MenuLoadSDCardMessages, .postAction = NULL, .submenu = messagesMenu  };
    rootMenu->items[2] = (MenuItem){ .name = { "3. Announcement", "3. הכרזה" }, .prepareAction = NULL, .postAction = NULL, .submenu = announcementMenu    };
    rootMenu->items[3] = (MenuItem){ .name = { "4. Test", "4. בדיקה" }, .prepareAction = NULL,  .postAction = NULL,.submenu = testMenu    };
    rootMenu->items[4] = (MenuItem){ .name = { "5. Report", "5. דוח" }, .prepareAction = NULL,  .postAction = NULL, .submenu = reportMenu    };
    rootMenu->items[5] = (MenuItem){ .name = { "6. Maintenance", "6. תחזוקה" }, .prepareAction = NULL, .postAction = NULL, .submenu = maintenanceMenu  };
	rootMenu->itemCount = 6;

//    /////////////////////////////////////////
    // "Siren"
    sirenMenu->parent = rootMenu;
    sirenMenu->currentSelection = 0;
    sirenMenu->scrollOffset = 0;
    sirenMenu->itemCount = 0;
    sirenMenu->type = MENU_TYPE_LIST;
    sirenMenu->screenText[LANG_EN] = "### Siren ###";
    sirenMenu->screenText[LANG_HE] = "### -Siren- ###";
    sirenMenu->buttonHandler = HandleButtonPress;

    alarmInfoMenu->parent = sirenMenu;
    alarmInfoMenu->type = MENU_TYPE_SIREN_INFO;
    alarmInfoMenu->screenText[LANG_EN] = "### Alarm info ###";
    alarmInfoMenu->screenText[LANG_HE] = "### -Alarm info- ###";
    alarmInfoMenu->currentSelection = 0;
    alarmInfoMenu->scrollOffset = 0;
    alarmInfoMenu->itemCount = 0;
    alarmInfoMenu->buttonHandler = alarmInfoMenu_HandleButtonPress;

//    //-----------------------------------------------------------------------------------------------------------
	messagesMenu->parent = rootMenu;
	messagesMenu->type = MENU_TYPE_LIST;
	messagesMenu->currentSelection = 0;
	messagesMenu->scrollOffset = 0;
	messagesMenu->screenText[LANG_EN] = "### Messages ###";
	messagesMenu->screenText[LANG_HE] = "### -Messages- ###";
	messagesMenu->itemCount = 0;
	messagesMenu->buttonHandler = HandleButtonPress;

	// play message
	messagePlayMenu->parent = messagesMenu;
	messagePlayMenu->currentSelection = 0;
	messagePlayMenu->scrollOffset = 0;
	messagePlayMenu->itemCount = 0;
	messagePlayMenu->type = MENU_TYPE_MESSAGE_PLAY;
	messagePlayMenu->screenText[LANG_EN] = "### Play message ###";
	messagePlayMenu->screenText[LANG_HE] = "### -Play message- ###";
	messagePlayMenu->textFilename = "filename #1.wav";
	messagePlayMenu->imageData = &menu_speaker_img;
//	messagePlayMenu->buttonHandler = HandleButtonPress;
	messagePlayMenu->buttonHandler = HandleButtonPress;

//    //-----------------------------------------------------------------------------------------------------------
	announcementMenu->parent = rootMenu;
	announcementMenu->currentSelection = 0;
	announcementMenu->itemCount = 0;
	announcementMenu->scrollOffset = 0;
	announcementMenu->type = MENU_TYPE_ANNOUNCEMENT;
	announcementMenu->screenText[LANG_EN] = "### Announcement ###";
	announcementMenu->screenText[LANG_HE] = "### -Announcement- ###";
	announcementMenu->textFilename = NULL;
	announcementMenu->imageData = &menu_microfon_img;
//	announcementMenu->buttonHandler = VolumeControlButtonHandler;
	announcementMenu->buttonHandler = HandleButtonPress;

//	// VolumeIndicator_Init(&volumeIndicator, 30, 250, 420, 30,
//	// 		             //30,
//	// 					 system_get_volume(),
//	// 					 COLOR_DARKGRAY, 0x00FF, 0xC618, 0x0D00, 1, 1);


	testMenu->parent = rootMenu;
	testMenu->screenText[LANG_EN] = "### Tests ###";
	testMenu->screenText[LANG_HE] = "### -Tests- ###";
	testMenu->type = MENU_TYPE_LIST;
	testMenu->currentSelection = 0;
	testMenu->scrollOffset = 0;
	testMenu->buttonHandler = HandleButtonPress;
	testMenu->items[0] = (MenuItem){ .name = { "1. Silent Test", "" },     .postAction = &RunSilentTest, .submenu = NULL    };
	testMenu->items[1] = (MenuItem){ .name = { "2. Batteries Test", "" },  .postAction = &RunBatteriesTest, .submenu = NULL    };
	testMenu->items[2] = (MenuItem){ .name = { "3. Amplifiers Test", "" }, .postAction = &RunAmplifiresTest,   .submenu = NULL    };
	testMenu->items[3] = (MenuItem){ .name = { "4. Drivers Test", "" },    .postAction = &RunDriversTest, .submenu = NULL  };
	testMenu->items[4] = (MenuItem){ .name = { "5. Sinus Test", "" }, .prepareAction = &PrepareSinuseItems, .postAction = NULL, .submenu = sinusMenu  };
	testMenu->itemCount = 5;

//	//---------------------------------------------------------------
	batteriesTestMenu->parent = testMenu;
	batteriesTestMenu->screenText[LANG_EN] = "### Batteries Test ###";
	batteriesTestMenu->screenText[LANG_HE] = "### -Batteries Test- ###";
	batteriesTestMenu->type = MENU_TYPE_TEST_BAT;
	batteriesTestMenu->currentSelection = 0;
	batteriesTestMenu->itemCount = 0;
	batteriesTestMenu->scrollOffset = 0;
//	batteriesTestMenu->buttonHandler = HandleButtonPress;
	batteriesTestMenu->buttonHandler = HandleButtonPress;

//
//	//--------------------------------------------------------
	apmplifiresTestMunu->parent = testMenu;
	apmplifiresTestMunu->screenText[LANG_EN] = "### Amplifires Test ###";
	apmplifiresTestMunu->screenText[LANG_HE] = "### -Amplifires Test- ###";
	apmplifiresTestMunu->type = MENU_TYPE_TEST_AMP;
	apmplifiresTestMunu->currentSelection = 0;
	apmplifiresTestMunu->itemCount = 0;
	apmplifiresTestMunu->scrollOffset = 0;
//	apmplifiresTestMunu->buttonHandler = HandleButtonPress;
	apmplifiresTestMunu->buttonHandler = HandleButtonPress;

//
//	//-----------------------------------
	driversTestMenu->parent = testMenu;
	driversTestMenu->screenText[LANG_EN] = "### Drivers Test ###";
	driversTestMenu->screenText[LANG_HE] = "### -Drivers Test- ###";
	driversTestMenu->type = MENU_TYPE_TEST_DRIV;
	driversTestMenu->currentSelection = 0;
	driversTestMenu->itemCount = 0;
	driversTestMenu->scrollOffset = 0;
//	driversTestMenu->buttonHandler = HandleButtonPress;
	driversTestMenu->buttonHandler = HandleButtonPress;

	sinusMenu->parent = testMenu;
	sinusMenu->screenText[LANG_EN] = "### Generate sinus ###";
	sinusMenu->screenText[LANG_HE] = "### -Generate sinus- ###";
	sinusMenu->type = MENU_TYPE_LIST;
	sinusMenu->currentSelection = 0;
	sinusMenu->scrollOffset = 0;
	sinusMenu->buttonHandler = HandleButtonPress;

	sinusInfoMenu->parent = sinusMenu;
	sinusInfoMenu->type = MENU_TYPE_SIREN_INFO;
	sinusInfoMenu->screenText[LANG_EN] = "### Sinus info ###";
	sinusInfoMenu->screenText[LANG_HE] = "### -Sinus info- ###";
	sinusInfoMenu->currentSelection = 0;
	sinusInfoMenu->scrollOffset = 0;
	sinusInfoMenu->itemCount = 0;
	sinusInfoMenu->buttonHandler = sinusInfoMenu_HandleButtonPress;

	testMenu->items[0].submenu =  NULL; // ,,,,
	testMenu->items[1].submenu = batteriesTestMenu;
	testMenu->items[2].submenu = apmplifiresTestMunu;
	testMenu->items[3].submenu = driversTestMenu;
	testMenu->buttonHandler = HandleButtonPress;

//	////////////////////////////////////////////////////////
	reportMenu->parent = rootMenu;
	reportMenu->screenText[LANG_EN] = "### Report ###";
	reportMenu->screenText[LANG_HE] = "### -Report- ###";
	reportMenu->type = MENU_TYPE_REPORT;
	reportMenu->currentSelection = 0;
	reportMenu->itemCount = 0;
	reportMenu->scrollOffset = 0;
	reportMenu->buttonHandler = HandleButtonPress;

////	reportMenu = &menuPool[menuPoolIndex++];
////	reportMenu->screenText[LANG_EN] = "Repor";
////	reportMenu->screenText[LANG_HE] = "--";

	maintenanceMenu->parent = rootMenu;
	maintenanceMenu->screenText[LANG_EN] = "### Maintenance ###";
	maintenanceMenu->screenText[LANG_HE] = "### -Maintenance- ###";
	maintenanceMenu->type = MENU_TYPE_LIST;
	maintenanceMenu->currentSelection = 0;
	maintenanceMenu->scrollOffset = 0;
	maintenanceMenu->buttonHandler = HandleButtonPress;
	maintenanceMenu->items[0] = (MenuItem){ .name = { "1. Time & Date", "1. Time & Date" },     .prepareAction = NULL, .postAction = NULL, .submenu = NULL    };
	maintenanceMenu->items[1] = (MenuItem){ .name = { "2. Language select", "2. Language select" }, .prepareAction = &MenuInitLanguage, .postAction = NULL, .submenu = languageMenu    };
	maintenanceMenu->itemCount = 2;

	languageMenu->parent = maintenanceMenu;
	languageMenu->screenText[LANG_EN] = "### Language ###";
	languageMenu->screenText[LANG_HE] = "### -Language- ###";
	languageMenu->type = MENU_TYPE_LIST;
	languageMenu->currentSelection = 0;
	languageMenu->itemCount = 0;
	languageMenu->scrollOffset = 0;
	languageMenu->buttonHandler = languageMenu_HandleButtonPress;

    //currentMenu = rootMenu;
	PrepareSinuseItems();
	currentMenu = sinusMenu;
	DrawMenuScreen(true);

    // testing
//	currentMenu = idleMenu;

    //currentMenu = sirenMenu;
    //currentMenu = alarmInfoMenu;
   // currentMenu = messagesMenu; //???
    //currentMenu->action();
//      currentMenu = messagePlayMenu;
    //currentMenu = announcementMenu; // work ok
    //currentMenu = reportMenu; //ok
    //currentMenu = sirenMenu; // ok

////      currentMenu = testMenu;
////      //
////    currentMenu = driversTestMenu; // ok
//   // currentMenu = apmplifiresTestMunu; //ok
//    //currentMenu = batteriesTestMenu; // ok
//
//    //currentMenu = reportMenu;
//    //currentLevel = 0;
//
//    currentMenu = passwordMenu;


}


void MenuDrawImage(Menu *m)
{
	if (m->imageData == NULL) return;

	hx8357_draw_image(m->imageData->x, m->imageData->y,
					  m->imageData->w, m->imageData->h,
					  (const uint16_t*)m->imageData->image);
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
	//lastInteractionTick = osKernelGetTickCount();


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
	DrawMenuScreen(true);
}

void ClearMenu(Menu* menu)
{
    if (!menu) return;
    if (menu->type != MENU_TYPE_LIST) return;

    menu->itemCount = 0;
    menu->currentSelection = 0;
    menu->scrollOffset = 0;

    for (int i = 0; i < MAX_MENU_ITEMS; ++i)
    {
        for (int lang = 0; lang < LANG_COUNT; ++lang)
        {
            menu->items[i].name[lang] = NULL;
        }

        menu->items[i].prepareAction = NULL;
        menu->items[i].postAction = NULL;
        menu->items[i].submenu = NULL;
        menu->items[i].filepath = NULL;
    }
}

void MenuShowMessages(void)
{
    if (!messagesMenu || !messagePlayMenu) return;

    ClearMenu(messagesMenu);

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
	currentMenu->currentSelection = 0;
	DrawMenuScreen(true);
	RunBatteriesTest();

	//for(int i =0; i<2000;i++)
	  {
	    osDelay(2000);
	  }

	currentMenu = apmplifiresTestMunu;
	currentMenu->currentSelection = 0;
	DrawMenuScreen(true);
	RunAmplifiresTest();

	//for(int i =0; i<2000;i++)
	{
		osDelay(2000);
	}

	currentMenu = driversTestMenu;
	currentMenu->currentSelection = 0;
	DrawMenuScreen(true);
	RunDriversTest();

	//for(int i =0; i<2000;i++)
	{
		osDelay(2000);
	}


	currentMenu = testMenu; //driversTestMenu->parent;
	currentMenu->currentSelection = 0;
	DrawMenuScreen(true);
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

void MenuLoadSDCardSirens(void)
{
	if (!sirenMenu) return;

	ClearMenu(sirenMenu);

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

	   sirenMenu->items[i].name[LANG_EN] = messageFilenames[i];
	   sirenMenu->items[i].name[LANG_HE] = messageFilenames[i];
	   //sirenMenu->items[i].prepareAction = &sirenPrepareAction;
	   //sirenMenu->items[i].postAction = &sirenPostAction;
	   sirenMenu->items[i].submenu = alarmInfoMenu;
	   sirenMenu->items[i].filepath = messageFilenames[i];
	}

	sirenMenu->itemCount = count;
	sirenMenu->currentSelection = 0;
	sirenMenu->scrollOffset = 0;
	currentMenu = alarmInfoMenu;
}

void MenuLoadSDCardMessages(void)
{
    if (!messagesMenu || !messagePlayMenu) return;

   ClearMenu(messagesMenu);

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

        messagesMenu->items[i].name[LANG_EN] = messageFilenames[i];
        messagesMenu->items[i].name[LANG_HE] = messageFilenames[i];
//        messagesMenu->items[i].prepareAction = &PlayMessageStart;
//        messagesMenu->items[i].postAction = &PlayMessageStartPost;
//        messagesMenu->items[i].submenu = messagePlayMenu;
        messagesMenu->items[i].filepath = messageFilenames[i];
    }

    messagesMenu->itemCount = count;
    messagesMenu->currentSelection = 0;
    messagesMenu->scrollOffset = 0;
    currentMenu = messagePlayMenu;
}

static void PrepareSinuseItems(void)
{
    if (!sinusMenu) return;

	ClearMenu(sinusMenu);

    static const char* sinus_info[] = {
		"Sinus 420Hz 120s",
		"Sinus 1000Hz 120s",
		"Sinus 1020Hz 120s",
		"Sinus 20000Hz 120s",
		"Sinus 836Hz and 856Hz 60s",
		"Sinus ALARM 90s",
		"Sinus ALL_CLEAR 90s",
		"Sinus ALL CLEAR 120s",
		"Sinus ABC 120s"
	};

    const uint8_t count = sizeof(sinus_info) / sizeof(sinus_info[0]);

    for (uint8_t i = 0; i < count && i < MAX_MENU_ITEMS; ++i)
    {
    	sprintf(messageFilenames[i], "%d. %s", i+1, sinus_info[i]);

        sinusMenu->items[i].name[LANG_EN] = messageFilenames[i];
        sinusMenu->items[i].name[LANG_HE] = messageFilenames[i];
        sinusMenu->items[i].prepareAction = &sirenPrepareAction;
//        sinusMenu->items[i].postAction = &sirenPostAction;
        sinusMenu->items[i].submenu = sinusInfoMenu;
        sinusMenu->items[i].filepath = sinus_info[i];
    }

	sinusMenu->itemCount = count;
	sinusMenu->currentSelection = 0;
	sinusMenu->scrollOffset = 0;
    currentMenu = sinusInfoMenu;
}

void MenuInitLanguage(void)
{
	if (!languageMenu) return;

	//ClearMenu(languageMenu);

	for (uint8_t i = 0; i < LANG_COUNT && i < MAX_MENU_ITEMS; ++i)
	{
		char langSel = (GetLanguage() == i) ? '*' : ' ';
		sprintf(messageFilenames[i], "%d. %s %c", i+1, LanguageToString(i), langSel);
		//strncpy(messageFilenames[i], LanguageToString(i), MAX_FILENAME_LEN);

		languageMenu->items[i].name[LANG_EN] = messageFilenames[i];
		languageMenu->items[i].name[LANG_HE] = messageFilenames[i];
		//sirenMenu->items[i].prepareAction = &sirenPrepareAction;
		//sirenMenu->items[i].postAction = &sirenPostAction;
		//sirenMenu->items[i].submenu = alarmInfoMenu;
		languageMenu->items[i].filepath = messageFilenames[i];
	}

	languageMenu->itemCount = LANG_COUNT;
//	languageMenu->currentSelection = 0;
//	languageMenu->scrollOffset = 0;
	//currentMenu = alarmInfoMenu;
}

void test_count_up_menu()
{
    static bool direction_up = true;

    if (direction_up)
    {
        currentMenu->currentSelection++;
        if (currentMenu->currentSelection > MAX_VISIBLE_ITEMS-2 && currentMenu->currentSelection != currentMenu->itemCount - 1)
        	currentMenu->scrollOffset++;
    }
    else
    {
        currentMenu->currentSelection--;
        if (currentMenu->currentSelection < currentMenu->itemCount-MAX_VISIBLE_ITEMS+1 && currentMenu->scrollOffset > 0)
        	currentMenu->scrollOffset--;
    }

    if (direction_up && currentMenu->currentSelection == currentMenu->itemCount - 1)
    {
        direction_up = false;
    }
    else if (!direction_up && currentMenu->currentSelection == 0)
    {
        direction_up = true;
    }

//    char msg[64];
//    sprintf(msg, "sel %d, off %d, count %d\r\n", currentMenu->currentSelection, currentMenu->scrollOffset, currentMenu->itemCount);
//    Print_Msg(msg);
}

void sirenPrepareAction(void)
{
	if (!currentMenu || currentMenu->currentSelection >= currentMenu->itemCount)
		return;

	const char* filepath = currentMenu->items[currentMenu->currentSelection].filepath;

	if (!filepath || !alarmInfoMenu)
		return;

	currentMenu->textFilename = filepath;
}

//void sirenPostAction(void)
//{
//	isPlayAudioFile = true;
//	xQueueSend(audioQueue, &alarmInfoMenu->textFilename, 0);
//}

void sinusPrepareAction(void)
{
	if (!currentMenu || currentMenu->currentSelection >= currentMenu->itemCount)
		return;

	const char* filepath = currentMenu->items[currentMenu->currentSelection].filepath;

	if (!filepath || !alarmInfoMenu)
		return;

	currentMenu->textFilename = filepath;
}

void sinusPostAction(void)
{
	isPlayAudioFile = true;
	Print_Msg(alarmInfoMenu->textFilename);

//	if(player.is_arming && !player.is_playing)
	{
		player.type_output = AUDIO_SIN;
		player.current_sin = (SinTask_t)currentMenu->currentSelection;
		player.audio_state = AUDIO_START;
	    xQueueSend(xAudioQueueHandle, &player.audio_state, portMAX_DELAY);
	}

//	xQueueSend(audioQueue, &alarmInfoMenu->textFilename, 0);
}

void menu_handle_button(ButtonEvent_t event)
{
//	char msg[64];
	lastInteractionTick = osKernelGetTickCount();

	if (!currentMenu) return;

	if (event.action == BA_RELEASED) return;

//	if (!isBacklightOn)
//	{
//		BLK_ON();
//		isBacklightOn = true;
//		return;
//	}

	if (hot_key_handle_button(event)) return;

	if (isIdle)
	{
//		if(isResetPasswordAfterIdle){
//			Password_Reset(false);
//		}

		isIdle = false;

//		if (Password_IsCorrect())
//		{
//			currentMenu = rootMenu;
//		}
//		else
//		{
//			Password_Reset(false);
//			currentMenu = passwordMenu;
//		}
//		currentMenu = rootMenu;
//		currentMenu->currentSelection = 0;
//		DrawMenuScreen(true);

		return;
	}

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
		currentMenu->currentSelection = 0;
		DrawMenuScreen(true);
		return true;
	case BTN_ANNOUNCEMENT:
		currentMenu = announcementMenu;
		currentMenu->currentSelection = 0;
		DrawMenuScreen(true);
		return true;
	case BTN_MESSAGE:
		MenuLoadSDCardMessages();
		currentMenu = messagesMenu;
		currentMenu->currentSelection = 0;
		DrawMenuScreen(true);
		return true;
	case BTN_ALARM:
		MenuLoadSDCardSirens();
		currentMenu = sirenMenu;
		currentMenu->currentSelection = 0;
		DrawMenuScreen(true);
		return true;
	default:
		return false;
	}
}

void HandleButtonPress(ButtonEvent_t event)
{
	if (!currentMenu) return;

    switch(event.button)
    {
        case BTN_UP:
        	handle_button_up();
        	break;
        case BTN_DOWN:
        	handle_button_down();
        	break;
        case BTN_ENTER:
        	handle_button_enter();
        	break;
        case BTN_ESC:
        	handle_button_esc();
        	break;
        default:
        	return;
    }
}

//void passwordMenu_HandleButtonPress(ButtonEvent_t event)
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
//							DrawMenuScreen(true);
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
void alarmInfoMenu_HandleButtonPress(ButtonEvent_t event)
{
	if (!alarmInfoMenu) return;

	switch(event.button)
	{
		case BTN_ESC:
			isPlayAudioFile = false;
			if (currentMenu->parent != NULL) {
				currentMenu = currentMenu->parent;

				if (currentMenu->type == MENU_TYPE_IDLE){
					isIdle = true;
				}
				DrawMenuScreen(true);
			}
			break;

		default:
				return;
	}

}

void sinusInfoMenu_HandleButtonPress(ButtonEvent_t event)
{
	if (!sinusInfoMenu) return;

	switch(event.button)
	{
		case BTN_ESC:
			isPlayAudioFile = false;
			if (currentMenu->parent != NULL) {
				currentMenu = currentMenu->parent;

				if (currentMenu->type == MENU_TYPE_IDLE){
					isIdle = true;
				}
				DrawMenuScreen(true);
			}
			break;

		default:
				return;
	}

}
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
//			DrawMenuScreen(true);
//			break;
//
//		default:
//			return;
//	}
//
//}

void languageMenu_HandleButtonPress(ButtonEvent_t event)
{
	if (!languageMenu || languageMenu->itemCount == 0) return;

	switch(event.button)
	{
		case BTN_ENTER:
			SetLanguage((Language)languageMenu->currentSelection);
			MenuInitLanguage();
			DrawMenuScreen(false);
			return;
		default:
			break;
	}
	HandleButtonPress(event);
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
//
//#define DEBUG_PRINT_BUTTON_STATE

void DisplayMenuItem(uint8_t visualIndex, const MenuItem* item, bool selected, bool dummy)
{
	//const Menu* submenu = item->submenu;

	uint16_t y_pos = MENU_BASE_Y + (visualIndex * MENU_ITEM_HEIGHT);
    uint16_t text_color = selected ? COLOR_WHITE : COLOR_GREEN;
    uint16_t bg_color   = selected ? COLOR_BLUE  : COLOR_BLACK;

    hx8357_fill_rect(MENU_BASE_X , y_pos,  hx8357_get_width() - (MENU_BASE_X*2), MENU_ITEM_HEIGHT, bg_color);

    const char* text = (dummy) ? "..." : item->name[GetLanguage()];
    //hx8357_write_string(MENU_BASE_X, y_pos, text, &Font_11x18, text_color, bg_color);

    Alignment align = (currentMenu == languageMenu || currentMenu == maintenanceMenu) ? ALIGN_LEFT : (GetLanguage() == LANG_EN) ? ALIGN_LEFT : ALIGN_RIGHT;
    hx8357_write_alignedX_string(MENU_BASE_X, y_pos, text, &Font_11x18, text_color, bg_color, align);
}
//#define DEBUG_PRINT_BUTTON_STATE_2

void Draw_MENU_TYPE_IDLE()
{
	FaultsDisplay_DrawAll(MENU_BASE_X, IDLE_Y_POS);
}

void Draw_MENU_TYPE_ANNOUNCEMENT(void)
{
	MenuDrawImage(currentMenu);
	VolumeIndicator_Draw(&volumeIndicator);

	// Set the initial level
	//uint8_t initial_bars = volume_db_to_bars(system_get_volume());
	//VolumeIndicator_SetLevel(&volumeIndicator, initial_bars);

}

void Draw_MENU_TYPE_SIREN_INFO(void)
{
	if (currentMenu->parent->textFilename)
	{
		hx8357_write_alignedX_string(0, SIREN_Y_POS, currentMenu->parent->textFilename, &Font_11x18, COLOR_MAGENTA, COLOR_BLACK, ALIGN_CENTER);
	}

	MenuDrawImage(currentMenu);

	isPlayAudioFile = true;

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
	MenuDrawImage(currentMenu);

	isPlayAudioFile = true;
	MenuDrawProgress(0);

	osDelay(3);
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
            DisplayMenuItem(visual_index, &currentMenu->items[old_selection], false, false);
        }

        if (selection >= offset && selection < end_index)
        {
            uint8_t visual_index = selection - offset;
            DisplayMenuItem(visual_index, &currentMenu->items[selection], true, false);
        }
    }
    else
    {
        for (uint8_t i = offset; i < end_index; ++i)
        {
            uint8_t visual_index = i - offset;
			bool dummy = (show_scroll_up && visual_index == 0) || (show_scroll_down && visual_index == visible_count-1);
            DisplayMenuItem(visual_index, &currentMenu->items[i], (i == selection), dummy);
            osDelay(3);
        }
    }

    currentMenu->oldSelection = selection;
    currentMenu->oldOffset = offset;
}

void DrawMenuScreen(bool forceFullRedraw)
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

        if (currentMenu->type == MENU_TYPE_IDLE)
        {
			Draw_MENU_TYPE_IDLE();
        }
//        else if (currentMenu->type == MENU_TYPE_PASSWORD)
//        {
//        	 Draw_MENU_TYPE_PASSWORD();
//        }
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
       prevMenu = currentMenu;
    }
    else if (currentMenu->type == MENU_TYPE_LIST)
    {
		Draw_MENU_TYPE_LIST();

//        if (prevSelection != currentMenu->currentSelection)
//        {
//            uint8_t start = currentMenu->scrollOffset;
//
//            if (prevSelection >= start && prevSelection < start + MAX_VISIBLE_ITEMS)
//            {
//                 uint8_t visualIndex = prevSelection - start;
//                 //DisplayMenuItem(visualIndex, &currentMenu->items[prevSelection], false);
//                 DisplayMenuItem(visualIndex, &currentMenu->items[prevSelection], false, false);
//            }
//
//            if (currentMenu->currentSelection >= start && currentMenu->currentSelection < start + MAX_VISIBLE_ITEMS)
//            {
//                 uint8_t visualIndex = currentMenu->currentSelection - start;
//                 //DisplayMenuItem(visualIndex, &currentMenu->items[currentMenu->currentSelection], true);
//                 DisplayMenuItem(visualIndex, &currentMenu->items[currentMenu->currentSelection], true, false);
//            }
//            osDelay(5);
//            prevSelection = currentMenu->currentSelection;
//        }
    }

    osDelay(1);

#ifdef DEBUG_PRINT_BUTTON_STATE_2
   // DrawDebugInfo(&lastButtonEvent);
#endif

    osDelay(1);
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



void DrawStatusBar()
{
	char serialStr[20];
	hx8357_fill_rect(0, 0, hx8357_get_width(), STATUS_BAR_LINE_Y_POS, COLOR_BLACK);
	hx8357_write_alignedX_string(0, LOGO_Y_POS, "EES-3000", &Font_11x18, COLOR_WHITE, COLOR_BLACK, ALIGN_LEFT);
	//hx8357_write_string(LOGO_X_POS, LOGO_Y_POS, "EES-3000", &Font_11x18, COLOR_WHITE, COLOR_BLACK);

	UpdateDateTime();

	snprintf(serialStr, sizeof(serialStr), "Serial: %s", SERIAL_NUMBER);
	hx8357_write_alignedX_string(0, SERIAL_Y_POS, serialStr, &Font_7x10, COLOR_YELLOW, COLOR_BLACK, ALIGN_RIGHT);
	//hx8357_write_string(SERIAL_X_POS, SERIAL_Y_POS, serialStr, &Font_7x10, COLOR_YELLOW, COLOR_BLACK);

	hx8357_fill_rect(0, STATUS_BAR_LINE_Y_POS-1, hx8357_get_width(), 1, COLOR_GRAY);
}

void UpdateDateTime()
{
	//LCD_FillRectangle(TIME_X_POS, TIME_Y_POS, 235, TIME_HEIGHT, COLOR_BLACK);
	hx8357_write_alignedX_string(0, TIME_Y_POS, "DD/MM/YY hh:mm", &Font_7x10, COLOR_YELLOW, COLOR_BLACK, ALIGN_RIGHT);
	return;

//	uint16_t year = time_rtc.year;
//	if (time_rtc.year > 9999)
//	{
//		year = 9999;
//	}
//
//	snprintf(timeStr, sizeof(timeStr), "%02d/%02d/%04d %02d:%02d:%02d",
//										time_rtc.date, time_rtc.month, year,
//										time_rtc.hour,time_rtc.minute, time_rtc.second);
//
//	LCD_WriteString(TIME_X_POS, TIME_Y_POS, timeStr, Font_7x10, COLOR_WHITE, COLOR_BLACK);
}

void SetIdleMenu(void)
{
	if (!isIdle || isBacklightOn)
	{
		uint32_t now = osKernelGetTickCount();

		if (now - lastInteractionTick >= INACTIVITY_TIMEOUT_MS) {
			isIdle = true;

			DrawMenuScreen(true);
			lastInteractionTick = now;
		}
//		else if (isIdle && isBacklightOn && (now - lastInteractionTick >= BACKLIGHT_TIMEOUT_MS)) {
//			BLK_OFF();
//			isBacklightOn = false;
//		}
	}
}


//void ShowUartCommand(void)
//{
//	//LCD_FillRectangle(0, 0, LCD_GetWidth(), STATUS_BAR_LINE_Y_POS -1, COLOR_BLACK);
//	LCD_WriteString(LOGO_X_POS, LOGO_Y_POS, "EES-3000", &Font_7x10, COLOR_WHITE, COLOR_BLACK);
//
//}


static void handle_button_up(void)
{
	if (!currentMenu || currentMenu->itemCount == 0) return;

	if (currentMenu->type == MENU_TYPE_LIST)
	{
		if (currentMenu->currentSelection == 0)
		{
			currentMenu->currentSelection = currentMenu->itemCount-1;
			currentMenu->scrollOffset = currentMenu->itemCount-MAX_VISIBLE_ITEMS;
		}
		else
		{
			currentMenu->currentSelection--;

			if (currentMenu->currentSelection < currentMenu->itemCount-MAX_VISIBLE_ITEMS+1 &&
				currentMenu->scrollOffset > 0)
				currentMenu->scrollOffset--;
		}

		DrawMenuScreen(false);
	}
	else if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT)
	{
		// Decrease volume announcement
	}
}

static void handle_button_down(void)
{
	if (!currentMenu || currentMenu->itemCount == 0) return;

	if (currentMenu->type == MENU_TYPE_LIST)
	{
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

		DrawMenuScreen(false);
	}
	else if (currentMenu->type == MENU_TYPE_ANNOUNCEMENT)
	{
		// Increase volume announcement
	}
}

static void handle_button_enter(void)
{
	if (currentMenu->itemCount == 0) return;

	MenuItem* item = &currentMenu->items[currentMenu->currentSelection];
	if (!item) return;

	if (item->prepareAction) {
		item->prepareAction();
	}

	if (item->submenu) {
		currentMenu = item->submenu;
		currentMenu->scrollOffset = 0;
		currentMenu->currentSelection = 0;
		DrawMenuScreen(true);
	}

	if (item->postAction) {
		item->postAction();
	}
}

static void handle_button_esc(void)
{
	if (!currentMenu->parent) return;

	if (currentMenu->type == MENU_TYPE_MESSAGE_PLAY && isPlayAudioFile)
	{
		isPlayAudioFile = false;
	}

	currentMenu = currentMenu->parent;

	isIdle = currentMenu->type == MENU_TYPE_IDLE;

	DrawMenuScreen(true);
}



