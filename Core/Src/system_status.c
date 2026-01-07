// system_status.c
#include "system_status.h"
#include <string.h>
#include "lcd/lcd_widget_volume_indicator.h"

// Global status
system_status_t system_status;

// Comparison of enum → symbol for protocol
static char mode_to_char(system_mode_t mode)
{
    switch (mode) {
        case SYSTEM_MODE_ARMING:           return 'B';
        case SYSTEM_MODE_ALL_CLEAR_1:      return 'F';
        case SYSTEM_MODE_ALL_CLEAR_2:      return 'G';
        case SYSTEM_MODE_ALARM_WAIL:       return 'H';
        case SYSTEM_MODE_CHEMICAL:         return 'L';
        case SYSTEM_MODE_CANCEL_IMMEDIATE: return 'A';
        case SYSTEM_MODE_CANCEL_DELAYED:   return 'C';
        case SYSTEM_MODE_QUIET_TEST:       return 'O';
        case SYSTEM_MODE_FUTURE_SIREN_1:   return 'I';
        case SYSTEM_MODE_FUTURE_SIREN_2:   return 'J';
        case SYSTEM_MODE_FUTURE_SIREN_3:   return 'K';
        case SYSTEM_MODE_REPORT:           return 'D';
        case SYSTEM_MODE_VOICE:            return 'N';
        case SYSTEM_MODE_RESET:            return 'E';
        case SYSTEM_MODE_IDLE:             return '0';  // или ' ', зависит от системы
        default:                           return ' ';  // SYSTEM_MODE_UNKNOWN → пробел
    }
}

// Get the current mode symbol
char system_status_get_mode_char(void)
{
    return mode_to_char(system_status.mode);
}


// Initialization of state
void system_status_init(void)
{
    memset(&system_status, 0, sizeof(system_status));
    system_status.mode = SYSTEM_MODE_IDLE;

    // Все драйверы OK по умолчанию
    for (int i = 0; i < 10; i++) {
        system_status.amplifier_driver[i] = true;
    }

    // По умолчанию: дверь закрыта?
    system_status.door_sensor = false;  // или true — зависит от начального состояния
    system_status.flood_sensor = false;
    system_status.ac_voltage = true;
    system_status.battery_voltage = true;
    system_status.charger_unit = true;
}

void set_system_mode(system_mode_t mode)
{
    system_status.mode = mode;
}

void system_status_reset(void)
{
    // Шаг 1: Сохраняем значения БАЙТА 1 и БАЙТА 14 из ТЕКУЩЕГО состояния
    char saved_mode_char = system_status_get_mode_char();
    bool saved_charger_unit = system_status.charger_unit;

    // Шаг 2: Полный сброс всей структуры
    memset(&system_status, 0, sizeof(system_status));

    // Шаг 3: Восстанавливаем байт 1 и байт 14
    if (saved_mode_char == 'A') {
        set_system_mode(SYSTEM_MODE_STANDBY);
    } else if (saved_mode_char == 'B') {
        set_system_mode(SYSTEM_MODE_ARMING);
    } else if (saved_mode_char == 'E') {
        set_system_mode(SYSTEM_MODE_RESET);
    } else {
        set_system_mode(SYSTEM_MODE_IDLE);  // или STANDBY, зависит от логики
    }

    system_status.charger_unit = saved_charger_unit;

    // Шаг 4: Устанавливаем остальные поля согласно спецификации
    // A 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 1 1 0 1
    // 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22

    system_status.operating_current = false;      // байт 11
    system_status.battery_voltage = false;        // байт 12
    system_status.ac_voltage = false;             // байт 15
    system_status.flood_sensor = false;           // байт 16
    system_status.door_sensor = false;            // байт 17

    system_status.max_volume = true;              // байт 18 = 1
    system_status.main_tone = true;               // байт 19 = 1
    system_status.secondary_tone = true;          // байт 20 = 1
    system_status.not_used = false;               // байт 21 = 0


    // Байт 22: door_sensor = true → 1 (по умолчанию: дверь открыта)
    system_status.door_sensor = true;

    // Сброс громкости
    //system_set_volume(DEF_VALUE_VOLUME);
}
