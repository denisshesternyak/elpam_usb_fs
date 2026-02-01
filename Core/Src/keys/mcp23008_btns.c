#include <mcp23008_btns.h>
#include "mcp23008.h"

const char* ButtonToString(Button_t btn)
{
    switch(btn) {

        case BTN_UP:     		return "BTN_UP";
        case BTN_DOWN:   		return "BTN_DOWN";
        case BTN_LEFT:   		return "BTN_LEFT";
        case BTN_RIGHT:  		return "BTN_RIGHT";
        case BTN_ENTER: 		return "BTN_ENTER";
        case BTN_ESC:   		return "BTN_ESC";
        case BTN_A:      		return "BTN_A";
        case BTN_B:      		return "BTN_B";
        case BTN_TEST:      	return "BTN_TEST";
        case BTN_ANNOUNCEMENT:  return "BTN_ANNOUNCEMENT";
        case BTN_MESSAGE:      	return "BTN_MESSAGE";
        case BTN_ALARM:      	return "BTN_ALARM";
        case BTN_CXL:      		return "BTN_CXL";
        case BTN_ARM:      		return "BTN_ARM";
        case BTN_NONE:   		return "BTN_NONE";
        default:         		return "UNKNOWN_BUTTON";
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

void mcp23008_btns_init(void)
{
//    char msg[64];
//    Print_Msg("mcp23008_btns_init\r\n");
//
//    sprintf(msg, "0x00, 0x06, 0x09: 0x%02x 0x%02x 0x%02x\r\n", mcp23008_read_reg(0x00), mcp23008_read_reg(0x06), mcp23008_read_reg(0x09));
//    Print_Msg(msg);

    mcp23008_init_keyboard();

//    sprintf(msg, "0x00, 0x06, 0x09: 0x%02x 0x%02x 0x%02x\r\n", mcp23008_read_reg(0x00), mcp23008_read_reg(0x06), mcp23008_read_reg(0x09));
//    Print_Msg(msg);
//
//    sprintf(msg, "Read columns 0x%02x\r\n", mcp23008_read_columns());
//    Print_Msg(msg);
}

bool mcp23008_keys_poll(ButtonEvent_t *out_event)
{
    if (!out_event) return false;

    static uint8_t last_stable_key = 0xFF;
    static uint32_t last_change_time = 0;

    uint8_t current_key = mcp23008_scan_keyboard();
    uint32_t now = HAL_GetTick();

    if (current_key == last_stable_key)
    {
    	last_change_time = now;
    	return false;
    }

	if ((now - last_change_time) < DEBOUNCE_MS)
	{
		return false;
	}

	if (last_stable_key != 0xFF)
	{
		Button_t btn = (Button_t) last_stable_key;
		if (btn != BTN_NONE)
		{
			out_event->button = btn;
			out_event->action =  BA_RELEASED;
		}
	}

	if (current_key != 0xFF)
	{
		Button_t btn = (Button_t) current_key;
		if (btn != BTN_NONE)
		{
			char msg[64];
			sprintf(msg, "--btn: %d\r\n", btn);
			Print_Msg(msg);
			out_event->button = btn;
			out_event->action = BA_PRESSED;
			last_stable_key = current_key;
			last_change_time = now;
			return true;
		}
	}

	last_stable_key = 0xFF;
	last_change_time = now;

    return false;
}


/*
bool mcp23008_btns_poll(ButtonEvent_t *out_event)
{
    if (!out_event) return false;

    uint8_t scanned_key = mcp23008_scan_keyboard();
    bool any_pressed = (scanned_key != 0xFF);

    uint32_t now = HAL_GetTick();

    for (uint8_t i = 0; i < NUM_KEYS; i++) {
        bool pressed_now = (any_pressed && (i == scanned_key));

        if (pressed_now != key_states[i].current_state) {
            key_states[i].current_state = pressed_now;
            key_states[i].last_change_tick = now;
        }

        if (pressed_now != key_states[i].stable_state &&
            (now - key_states[i].last_change_tick >= DEBOUNCE_TIME_MS)) {

            key_states[i].stable_state = pressed_now;

            Button_t button = bit_to_button_map[i];
            if (button != BTN_NONE) {
                out_event->button = button;
                out_event->action = pressed_now ? BUTTON_PRESSED : BUTTON_RELEASED;
                return true;
            }
        }
    }

    return false;
}
*/
