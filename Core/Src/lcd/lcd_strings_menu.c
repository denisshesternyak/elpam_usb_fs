#include "lcd_strings_menu.h"

static const char* sinus_items[][2] = {
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

const uint8_t SINUS_ITEM_COUNT = ARRAY_SIZE(sinus_items);

const char* sinus_item_str(uint8_t task, Language lang)
{
    if (task >= SINUS_ITEM_COUNT) {
        return "";
    }
    return sinus_items[task][lang];
}



const char* MenuToString(MenuStrings menu_str)
{
    switch(menu_str) {
        case STR_HEADER_MENU:   return "### Menu ###";
        case STR_HEADER_FAULT_IND:     return "### Fault indication ###";
        default:         return "UNKNOWN_MENU_STRING";
    }
}
