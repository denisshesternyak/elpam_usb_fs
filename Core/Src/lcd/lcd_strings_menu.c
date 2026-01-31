#include "lcd_strings_menu.h"

static const char* menu_header_str[][2] = {
	{ "### Menu ###", "### -Menu- ###" },
	{ "### Fault indication ###", "### -Fault indication- ###" },
	{ "### Siren ###", "### -Siren- ###" },
	{ "### Alarm info ###", "### -Alarm info- ###" },
	{ "### Messages ###", "### -Messages- ###" },
	{ "### Play message ###", "### -Play message- ###" },
	{ "### Announcement ###", "### -Announcement- ###" },
	{ "### Tests ###", "### -Tests- ###" },
	{ "### Batteries Test ###", "### -Batteries Test- ###" },
	{ "### Amplifires Test ###", "### -Amplifires Test- ###" },
	{ "### Drivers Test ###", "### -Drivers Test- ###" },
	{ "### Generate sinus ###", "### -Generate sinus- ###" },
	{ "### Sinus info ###", "### -Sinus info- ###" },
	{ "### Report ###", "### -Report- ###" },
	{ "### Maintenance ###", "### -Maintenance- ###" },
	{ "### Time and Date ###", "### -Time and Date- ###" },
	{ "### Language ###", "### -Language- ###" }
};
const uint8_t MENU_HEADER_ITEM_COUNT = ARRAY_SIZE(menu_header_str);

//static const char* menu_items_str[][2] = {
//	{ "Siren", "-Siren-" },
//	{ "Messages", "-Messages-" },
//	{ "Announcement", "-Announcement-" },
//	{ "Test", "-Test-" },
//	{ "Report", "-Report-" },
//	{ "Maintenance", "-Maintenance-" },
//	{ "Silent Test", "-Silent Test-" },
//	{ "Batteries Test", "-Batteries Test-" },
//	{ "Amplifiers Test", "-Amplifiers Test-" },
//	{ "Drivers Test", "-Drivers Test-" },
//	{ "Sinus Test", "-Sinus Test-" },
//	{ "Time and Date", "-Time and Date-" },
//	{ "Language select", "-Language select-" },
//	{ "Sinus 420Hz 120s", "סינוס 420 הרץ 120 שניות" },
//	{ "Sinus 1000Hz 120s", "סינוס 1000 הרץ 120 שניות" },
//	{ "Sinus 1020Hz 120s", "סינוס 1020 הרץ 120 שניות" },
//	{ "Sinus 20000Hz 120s", "סינוס 20000 הרץ 120 שניות" },
//	{ "Sinus 836Hz and 856Hz 60s", "סינוס 836 הרץ 856 הרץ 60 שניות" },
//	{ "Sinus ALARM 90s", "90s אזעקת סינוסים שנות" },
//	{ "Sinus ALL CLEAR 90s", "90s סינוסים צלולים משנות" },
//	{ "Sinus ALL CLEAR 120s", "120s סינוסים צלולים משנות" },
//	{ "Sinus ABC 120s", "120s ABC סינוס" }
//};
//const uint8_t MENU_ITEMS_COUNT = ARRAY_SIZE(menu_items_str);

static const char* root_menu_items_str[][2] = {
	{ "Siren", "-Siren-" },
	{ "Messages", "-Messages-" },
	{ "Announcement", "-Announcement-" },
	{ "Test", "-Test-" },
	{ "Report", "-Report-" },
	{ "Maintenance", "-Maintenance-" }
};
const uint8_t ROOT_MENU_ITEM_COUNT = ARRAY_SIZE(root_menu_items_str);

static const char* test_menu_items_str[][2] = {
	{ "Silent Test", "-Silent Test-" },
	{ "Batteries Test", "-Batteries Test-" },
	{ "Amplifiers Test", "-Amplifiers Test-" },
	{ "Drivers Test", "-Drivers Test-" },
	{ "Sinus Test", "-Sinus Test-" }
};
const uint8_t TEST_MENU_ITEM_COUNT = ARRAY_SIZE(test_menu_items_str);

static const char* maintenance_menu_items_str[][2] = {
	{ "Time and Date", "Time and Date" },
	{ "Language select", "Language select" }
};
const uint8_t MAINTENCE_MENU_ITEM_COUNT = ARRAY_SIZE(maintenance_menu_items_str);

static const char* sinus_menu_items_str[][2] = {
	{ "Sinus 420Hz 120s", "סינוס 420 הרץ 120 שניות" },
	{ "Sinus 1000Hz 120s", "סינוס 1000 הרץ 120 שניות" },
	{ "Sinus 1020Hz 120s", "סינוס 1020 הרץ 120 שניות" },
	{ "Sinus 20000Hz 120s", "סינוס 20000 הרץ 120 שניות" },
	{ "Sinus 836Hz and 856Hz 60s", "סינוס 836 הרץ 856 הרץ 60 שניות" },
	{ "Sinus ALARM 90s", "90s אזעקת סינוסים שנות" },
	{ "Sinus ALL CLEAR 90s", "90s סינוסים צלולים משנות" },
	{ "Sinus ALL CLEAR 120s", "120s סינוסים צלולים משנות" },
	{ "Sinus ABC 120s", "120s ABC סינוס" }
};
const uint8_t SINUS_ITEM_COUNT = ARRAY_SIZE(sinus_menu_items_str);

const char* get_menu_header_str(uint8_t task, Language lang)
{
    if (task >= MENU_HEADER_ITEM_COUNT) return "";
    return menu_header_str[task][lang];
}

const char* get_root_menu_items_str(uint8_t task, Language lang)
{
    if (task >= ROOT_MENU_ITEM_COUNT) return "";
    return root_menu_items_str[task][lang];
}

const char* get_test_menu_items_str(uint8_t task, Language lang)
{
    if (task >= TEST_MENU_ITEM_COUNT) return "";
    return test_menu_items_str[task][lang];
}

const char* get_maintenance_menu_items_str(uint8_t task, Language lang)
{
    if (task >= MAINTENCE_MENU_ITEM_COUNT) return "";
    return maintenance_menu_items_str[task][lang];
}

const char* get_sinus_menu_items_str(uint8_t task, Language lang)
{
    if (task >= SINUS_ITEM_COUNT) return "";
    return sinus_menu_items_str[task][lang];
}

//char* get_menu_items_str(uint8_t task, Language lang)
//{
//    if (task >= MENU_ITEMS_COUNT) return "";
//    const char* text = menu_items_str[task][lang];
//
//    static char item_str[64];
//
//    if(GetLanguage() == LANG_EN)
//	{
//		sprintf(item_str[task], "%d. %s", task+1, text);
//	}
//	else if(GetLanguage() == LANG_EN)
//	{
//		sprintf(item_str[task], "%s .%d", text, task+1);
//	}
//    return item_str;
//}


