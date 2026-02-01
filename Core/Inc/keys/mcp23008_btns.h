#ifndef INC_KEYS_MCP23008_BTNS_H_
#define INC_KEYS_MCP23008_BTNS_H_

#include <stdint.h>
#include <stdbool.h>

#define NUM_KEYS 		16
#define DEBOUNCE_MS 	20

//      Col0  Col1  Col2  Col3
// Row0:  0     1     2     3
// Row1:  4     5     6     7
// Row2:  8     9    10    11
// Row3: 12    13    14    15
typedef enum {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
	BTN_ENTER,
	BTN_ESC,

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

void mcp23008_btns_init(void);
bool mcp23008_keys_poll(ButtonEvent_t *out_event);
const char* ButtonActionToString(ButtonAction_t ba);
const char* ButtonToString(Button_t btn) ;


#endif /* INC_KEYS_MCP23008_BTNS_H_ */
