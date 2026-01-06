#include <audio_cmd.h>
#include <audio_regs.h>

extern I2C_HandleTypeDef hi2c5;

void audio_write_cmd(uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(&hi2c5, AIC32X4_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 500);
}

uint8_t audio_read_cmd(uint8_t reg)
{
	uint8_t value;
	HAL_I2C_Mem_Read(&hi2c5, AIC32X4_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 500);
	return value;
}

