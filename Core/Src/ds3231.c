
#include "ds3231.h"
#include "main.h"

//------------------------
// SCL - PB6
// SDA - PB7
//------------------------

rtc_time_t time_rtc;

extern I2C_HandleTypeDef hi2c2;

#define DS3231_ADDRESS 0xD0

#define DS3231_REG_TIME 0x00


uint8_t bin_to_bcd(uint8_t val)
{
  return ((val / 10) << 4) | (val % 10);
}

uint8_t bcd_to_bin(uint8_t val)
{
  return ((val >> 4) * 10) + (val & 0x0F);
}

uint8_t bcd_to_dec(uint8_t val)
{
  return ((val >> 4) * 10) + (val & 0x0F);
}


uint8_t dec_to_bcd(uint8_t val)
{
  return ((val / 10) << 4) | (val % 10);
}

HAL_StatusTypeDef ds3231_set_datetime()
{
	static uint8_t data[8];
    data[0] = DS3231_REG_TIME;
    data[1] = dec_to_bcd(time_rtc.second);
    data[2] = dec_to_bcd(time_rtc.minute);
    data[3] = dec_to_bcd(time_rtc.hour);
    data[4] = dec_to_bcd(0);
    data[5] = dec_to_bcd(time_rtc.date);
    data[6] = dec_to_bcd(time_rtc.month);
    data[7] = bin_to_bcd(time_rtc.year > 2000 ? time_rtc.year - 2000 : time_rtc.year);

    HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c2, DS3231_ADDRESS, data, sizeof(data), HAL_MAX_DELAY);

    if (err != HAL_OK) {
        return err;
    }

    return HAL_OK;
}

HAL_StatusTypeDef ds3231_get_datetime()
{
    uint8_t reg = DS3231_REG_TIME;
    static uint8_t data[7];

    HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c2, DS3231_ADDRESS, &reg, 1, HAL_MAX_DELAY);

    if (err != HAL_OK) {
      return err;
    }

    err = HAL_I2C_Master_Receive(&hi2c2, DS3231_ADDRESS, data, sizeof(data), HAL_MAX_DELAY);
    if (err != HAL_OK) {
        return err;
    }

    time_rtc.second = bcd_to_bin(data[0] & 0x7F);
    time_rtc.minute = bcd_to_bin(data[1]);
    time_rtc.hour   = bcd_to_bin(data[2] & 0x3F);
    time_rtc.date   = bcd_to_bin(data[4]);
    time_rtc.month  = bcd_to_bin(data[5] & 0x1F);
    time_rtc.year   = 2000 + bcd_to_bin(data[6]);

    return HAL_OK;
}
