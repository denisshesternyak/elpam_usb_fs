#include "command_dispatcher.h"
#include "system_status.h"
#include "defines.h"
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "defines.h"  // For USE_DEBUG_COMMAND_DISPATCHER
#include <string.h>
#include <stdio.h>
#include "audio_types.h"
#include "lcd_menu.h"

// Extern UART handler for debug output (optional)
extern UART_HandleTypeDef huart2;

extern Audio_Player_t player;
extern osMessageQueueId_t xAudioQueueHandle;
extern osMessageQueueId_t xLCDQueueHandle;

// Buffer for debug messages
#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    static char debug_msg[32];
    #include "../lcd/lcd_display.h"

   
#endif

// ————————————————————————————————————————
// Command handler functions
// Each function is called by rs232.c when a command is received
// ————————————————————————————————————————

/**
 * @brief Handle Arming Operation (*_ARM__)
 * Prepare the siren system for operation (e.g., self-test, power up)
 */
void handle_arm(void)
{
    set_system_mode(SYSTEM_MODE_ARMING);
    player.start_time_arming = HAL_GetTick();
    player.is_arming = true;
	Print_Msg("ARM is started\r\n");

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: ARM\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "B - ARM", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle All Clear 1 (*ACLR1_)
 * Deactivate alarm with clear signal type 1
 */
void handle_all_clear_1(void)
{
    set_system_mode(SYSTEM_MODE_ALL_CLEAR_1);
//    if(player.is_arming)
    {
		Print_Msg("All_clear_1 is start\r\n");

		player.type_input = AUDIO_SIN;
		player.current_sin = SINUS_ALL_CLEAR_90S;
		player.audio_state = AUDIO_START;
		player.priority = AUDIO_PRIORITY_HIGH;
		xQueueSendFromISR(xAudioQueueHandle, &player.audio_state, NULL);
    }

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: ALL CLEAR 1\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "F - ALL CLEAR 1", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle All Clear 2 (*ACLR2_)
 * Deactivate alarm with clear signal type 2
 */
void handle_all_clear_2(void)
{
    set_system_mode(SYSTEM_MODE_ALL_CLEAR_2);
//    if(player.is_arming)
	{
		player.type_input = AUDIO_SIN;
		player.current_sin = SINUS_ALL_CLEAR_120S;
		player.audio_state = AUDIO_START;
		player.priority = AUDIO_PRIORITY_HIGH;
		xQueueSendFromISR(xAudioQueueHandle, &player.audio_state, NULL);
		//Print_Msg("All_clear_2 is start\r\n");
	}

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: ALL CLEAR 2\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "G - ALL CLEAR 2", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Alarm Operation (*IWAIL_)
 * Activate main alarm sound (wail)
 */
void handle_alarm(void)
{
     set_system_mode(SYSTEM_MODE_ALARM_WAIL);
     if(player.is_arming)
     {
		player.type_input = AUDIO_SIN;
		player.current_sin = SINUS_ALARM_90S;
		player.audio_state = AUDIO_START;
		player.priority = AUDIO_PRIORITY_HIGH;
		xQueueSendFromISR(xAudioQueueHandle, &player.audio_state, NULL);
		//Print_Msg("Alarm is start\r\n");
     }

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: ALARM (WAIL)\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "H - ALARM", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Chemical Operation (*CHEM_A)
 * Activate alarm for chemical hazard
 */
void handle_chemical(void)
{
    set_system_mode(SYSTEM_MODE_CHEMICAL);
    if(player.is_arming)
	{
		player.type_input = AUDIO_SIN;
		player.current_sin = SINUS_ABC_120S;
		player.priority = AUDIO_PRIORITY_HIGH;
		player.audio_state = AUDIO_START;
		xQueueSendFromISR(xAudioQueueHandle, &player.audio_state, NULL);
//		Print_Msg("Chemical is start\r\n");
	}



#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: CHEMICAL ALARM\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "L - CHEMICAL ALAR", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Disarm Command (*DISARM)
 * Immediately stop all active alarms and voice
 */
void handle_disarm(void)
{
    set_system_mode(SYSTEM_MODE_CANCEL_IMMEDIATE);
    //player.audio_state = AUDIO_STOP;
    player.start_time_arming =0;
    player.is_arming = false;

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: DISARM\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "A - DISARM", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Cancel Command (*CANCEL)
 * Cancel current operation with delay
 */
void handle_cancel(void)
{
    set_system_mode(SYSTEM_MODE_CANCEL_DELAYED);
    player.audio_state = AUDIO_STOP;
	player.priority = AUDIO_PRIORITY_HIGH;
	xQueueSendFromISR(xAudioQueueHandle, &player.audio_state, NULL);

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: CANCEL\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "C - CANCEL", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Quiet Test (*Q_TEST)
 * Perform silent test (e.g., check circuits without sound)
 */
void handle_quiet_test(void)
{
    set_system_mode(SYSTEM_MODE_QUIET_TEST);


#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: QUIET TEST\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "O - QUIET TEST", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Reserve 1 Command (*_WAIL_)
 * Reserved for future siren type
 */
void handle_reserve_1(void)
{
	set_system_mode(SYSTEM_MODE_FUTURE_SIREN_1);

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: RESERVE 1\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "I - RESERVE 1", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Reserve 2 Command (*PWAIL_)
 * Reserved for future siren type
 */
void handle_reserve_2(void)
{
	set_system_mode(SYSTEM_MODE_FUTURE_SIREN_2);


#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: RESERVE 2\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "J - RESERVE 2", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Reserve 3 Command (*YELP__)
 * Reserved for future siren type
 */
void handle_reserve_3(void)
{
	set_system_mode(SYSTEM_MODE_FUTURE_SIREN_3);


#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: RESERVE 3\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "K - RESERVE 3", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Remote P.A. Command (*_VOICE)
 * Activate voice public address mode
 */
void handle_remote_pa(void)
{
    set_system_mode(SYSTEM_MODE_VOICE);

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: REMOTE PA\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "N - REMOTE PA", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Reset Command (*RESET_)
 * Reset system or protocol state
 */
void handle_reset(void)
{
    system_status_reset();

    //set_system_mode(SYSTEM_MODE_RESET);


#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "CMD: RESET\r\n");
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "E - RESET", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
    // Example: HAL_NVIC_SystemReset();
}

/**
 * @brief Handle Volume Up Command (*VOLnnn, nnn )
 * @param step - volume increase step
 */
void volume_up_handler(int value)
{

    //system_set_volume(value);
	player.new_volume = value;

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "VOL UP: %d (now: %d)\r\n", value, system_get_volume());
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    sprintf(debug_msg, "VOL UP: %d (now: %d)", value, system_get_volume());
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, debug_msg, LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Handle Volume Down Command (*VOLnnn, nnn >= 900)
 * @param step - volume decrease step (1..99 after conversion)
 */
void volume_down_handler(int value)
{
    //system_set_volume(value);
	player.new_volume = value;

#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    //sprintf(debug_msg, "VOL DOWN: %d (now: %d)\r\n", value, system_get_volume());
    //HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
     sprintf(debug_msg, "VOL DOWN: %d (now: %d)", value, system_get_volume());
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, debug_msg, LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

/**
 * @brief Fill 22-character status report for *REPORT command
 * @param response_22 - pointer to 22-byte buffer
 */
void system_fill_report()
{
	char response_22[23] = {0};

    // Byte 1: Mode (one char)
    response_22[0] = system_status_get_mode_char();

    // Byte 2: Not used
    response_22[1] = '0';

    // Bytes 3–12: Driver 1–10
    for (int i = 0; i < 10; i++) {
        response_22[2 + i] = system_status.amplifier_driver[i] ? '1' : '0';
    }

     // Byte 13: Not used
    response_22[12] = '0';

    // Byte 14: Max Volume
    response_22[13] = system_status.max_volume ? '1' : '0';

    // Byte 15: Main Tone
    response_22[14] = system_status.main_tone ? '1' : '0';

    // Byte 16: Secondary Tone
    response_22[15] = system_status.secondary_tone ? '1' : '0';

    // Byte 17: Operating Current
    response_22[16] = system_status.operating_current ? '1' : '0';

    // Byte 18: Battery Voltage
    response_22[17] = system_status.battery_voltage ? '1' : '0';

    // Byte 19: Charger Unit
    response_22[18] = system_status.charger_unit ? '1' : '0';

    // Byte 20: AC Voltage
    response_22[19] = system_status.ac_voltage ? '1' : '0';

    // Byte 21: Flood Sensor
    response_22[20] = system_status.flood_sensor ? '1' : '0';

    // Byte 22: Door Sensor
    response_22[21] = system_status.door_sensor ? '1' : '0';

    HAL_UART_Transmit(&huart2, (uint8_t*)response_22, 22, HAL_MAX_DELAY);
}

/**
 * @brief Handle unknown or malformed command
 */
void handle_unknown_command(void)
{
#if defined(USE_DEBUG_COMMAND_DISPATCHER)
    ///HAL_UART_Transmit(&huart2, (uint8_t*)"ERR:UNKNOWN\r\n", 13, HAL_MAX_DELAY);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "                             ", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
    LCD_WriteString(LCD_USART_TEXT_X, LCD_USART_TEXT_Y, "ERR:UNKNOWN", LCD_USART_TEXT_FONT, COLOR_YELLOW, COLOR_BLACK);
#endif
}

void handle_enter_command(void)
{
	LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .action = BA_PRESSED } };
	lcd_event.btn.button = BTN_ENTER;
	xQueueSendFromISR(xLCDQueueHandle, &lcd_event, NULL);
}

void handle_up_command(void)
{
	LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .action = BA_PRESSED } };
	lcd_event.btn.button = BTN_UP;
	xQueueSendFromISR(xLCDQueueHandle, &lcd_event, NULL);
}

void handle_down_command(void)
{
	LCDTaskEvent_t lcd_event = { .event = LCD_EVENT_BTN, .btn = { .action = BA_PRESSED } };
	lcd_event.btn.button = BTN_DOWN;
	xQueueSendFromISR(xLCDQueueHandle, &lcd_event, NULL);
}

