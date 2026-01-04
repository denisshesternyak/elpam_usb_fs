

#include "ch423_keys.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c4;

#define CH423_I2C_ADDRESS (0x20 << 1) 

static uint8_t read_ch423_inputs(void)
{
    uint8_t data = 0xFF;
    HAL_I2C_Master_Receive(&hi2c4, CH423_I2C_ADDRESS, &data, 1, 100);
    return data;
}

typedef struct {
    bool current_state;
    bool stable_state;
    int64_t last_change_time;
} KeyState_t;

static KeyState_t key_states[NUM_KEYS];

// 7 3 4 1
// 6 5 0 2
//
static const Button_t bit_to_button_map[NUM_KEYS] = {
	BTN_B,      // bit 0
	BTN_UP,     // bit 1
	BTN_DOWN,   // bit 2
	BTN_RIGHT,  // bit 3
	BTN_A,      // bit 4
    BTN_LEFT,   // bit 5
	BTN_SELECT, // bit 6
    BTN_BACK    // bit 7
};

const char* ButtonToString(Button_t btn)
{
    switch(btn) {
        case BTN_UP:     return "BTN_UP";
        case BTN_DOWN:   return "BTN_DOWN";
        case BTN_LEFT:   return "BTN_LEFT";
        case BTN_RIGHT:  return "BTN_RIGHT";
        case BTN_SELECT: return "BTN_SELECT";
        case BTN_BACK:   return "BTN_BACK";
        case BTN_A:      return "BTN_A";
        case BTN_B:      return "BTN_B";
        case BTN_NONE:   return "BTN_NONE";
        default:         return "UNKNOWN_BUTTON";
    }
}

const char* ButtonActionToString(ButtonAction_t ba)
{
	switch(ba)
	{
		case BA_RELEASED: return "BA_RELEASED";
		case BA_PRESSED:  return "BA_PRESSED";
		default:return "UNKNOWN";
	}
}

void ch423_keys_init(void)
{
    for (int i = 0; i < NUM_KEYS; i++)
    {
        key_states[i] = (KeyState_t){ false, false, 0 };
    }
}

bool ch423_keys_poll(ButtonEvent_t *out_event) {
    uint8_t raw = read_ch423_inputs();
    int64_t now = HAL_GetTick() * 1000;

    for (int i = 0; i < NUM_KEYS; i++) {
        bool pressed = !(raw & (1 << i));
        if (pressed != key_states[i].current_state) {
            key_states[i].current_state = pressed;
            key_states[i].last_change_time = now;
        }

        if ((pressed != key_states[i].stable_state) &&
            (now - key_states[i].last_change_time > DEBOUNCE_MS * 1000)) {

            key_states[i].stable_state = pressed;

            if (bit_to_button_map[i] != BTN_NONE) {
                *out_event = (ButtonEvent_t){
                    .button = bit_to_button_map[i],
                    .action = pressed ? BUTTON_PRESSED : BUTTON_RELEASED
                };
                return true; 
            }
        }
    }

    return false;
}
