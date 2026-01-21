#include "lcd_strings_menu.h"

const char* ButtonToString(Button_t btn)
{
    switch(btn) {
        case BTN_UP:     return "### Fault indication ###;
        case BTN_DOWN:   return "BTN_DOWN";
        case BTN_LEFT:   return "BTN_LEFT";
        case BTN_RIGHT:  return "BTN_RIGHT";
        case BTN_ENTER: return "BTN_ENTER";
        case BTN_ESC:   return "BTN_ESC";
        case BTN_A:      return "BTN_A";
        case BTN_B:      return "BTN_B";
        case BTN_NONE:   return "BTN_NONE";
        default:         return "UNKNOWN_BUTTON";
    }
}
