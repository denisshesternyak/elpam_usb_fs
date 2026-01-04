
#ifndef INC_KEYS_CH423_KEYS_H_
#define INC_KEYS_CH423_KEYS_H_


#include <stdint.h>
#include <stdbool.h>

#define NUM_KEYS 8
#define DEBOUNCE_MS 30


typedef enum {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_SELECT,//ENTER
    BTN_BACK, // ESC

	BTN_A,
	BTN_B,

    //------------------------
    BTN_TEST,
    BTN_ANNOUNCEMENT,
    BTN_MESSAGE,
    BTN_ALARM,
    BTN_CXL,
    BTN_ARM,
    
    //------------------------
    BTN_0,
    BTN_1,
    BTN_2,
    BTN_3,
    BTN_4,
    BTN_5,
    BTN_6,
    BTN_7,
    BTN_8,
    BTN_9,
    BTN_HASH, // #
    BTN_START, // *

    BTN_NONE
} Button_t;

typedef enum {
    BA_RELEASED = 0,
    BA_PRESSED
} ButtonAction_t;

typedef struct {
    Button_t button;
    ButtonAction_t action;
} ButtonEvent_t;


void ch423_keys_init(void);
bool ch423_keys_poll(ButtonEvent_t *out_event);
const char* ButtonActionToString(ButtonAction_t ba);
const char* ButtonToString(Button_t btn) ;


#endif /* INC_KEYS_CH423_KEYS_H_ */
