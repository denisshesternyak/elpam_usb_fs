/*
 * lcd_lang.c
 *
 *  Created on: Jun 19, 2025
 *      Author: roman
 */


#include "lcd_lang.h"


static Language currentLanguage = LANG_EN;


void SetLanguage(Language lang) {
    
    if (lang >= LANG_EN && lang < LANG_COUNT) {
        currentLanguage = lang;
    }
    
}


Language GetLanguage(void) {
    return currentLanguage;
}
