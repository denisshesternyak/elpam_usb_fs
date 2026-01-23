#include "lcd_strings_menu.h"

const char* MenuToString(MenuStrings menu_str)
{
    switch(menu_str) {
        case STR_HEADER_MENU:   return "### Menu ###";
        case STR_HEADER_FAULT_IND:     return "### Fault indication ###";
        default:         return "UNKNOWN_MENU_STRING";
    }
}
