

#ifndef INC_DS3231_H_
#define INC_DS3231_H_

#include <stdint.h>
#include "stm32h7xx_hal.h" 

#pragma pack(push, 1)
typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t date;
    uint8_t month;
    uint16_t year;
    // uint8_t dummy;
} rtc_time_t;
#pragma pack(pop)

extern rtc_time_t time_rtc;

HAL_StatusTypeDef ds3231_get_datetime();
HAL_StatusTypeDef ds3231_set_datetime();

#endif /* INC_DS3231_H_ */
