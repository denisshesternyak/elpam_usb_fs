#include "mcp23008.h"

extern I2C_HandleTypeDef hi2c4;

static void mcp23008_write_cmd(uint8_t reg, uint8_t value);
static uint8_t mcp23008_read_cmd(uint8_t reg);
static void mcp23008_delay(uint32_t delay);

static void mcp23008_write_cmd(uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(&hi2c4, MCP23008_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
}

static uint8_t mcp23008_read_cmd(uint8_t reg)
{
	uint8_t value = 0;
	HAL_I2C_Mem_Read(&hi2c4, MCP23008_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
	return value;
}

static void mcp23008_delay(uint32_t delay)
{
	while (delay--) __NOP();
}

void mcp23008_init_keyboard()
{
	mcp23008_write_cmd(MCP23008_DDR_A, 0xF0);
	mcp23008_write_cmd(MCP23008_PUR_A,  0xF0);
	mcp23008_write_cmd(MCP23008_GPIO_A,  0x0F);
}

void mcp23008_set_row(uint8_t row_num)
{
    uint8_t value = 0x0F & ~(1 << row_num);
    mcp23008_write_cmd(MCP23008_GPIO_A, value);
}

uint8_t mcp23008_read_columns()
{
    return (mcp23008_read_cmd(MCP23008_GPIO_A) >> 4) & 0x0F;
}

uint8_t mcp23008_read_reg(uint8_t reg)
{
    return mcp23008_read_cmd(reg);
}

uint8_t mcp23008_scan_keyboard()
{
    uint8_t key = 0xFF;

    for (uint8_t row = 0; row < MCP23008_ROW; row++)
    {
        mcp23008_set_row(row);
        mcp23008_delay(100);

        uint8_t cols = mcp23008_read_columns();

        if (cols != 0x0F)
        {
            for (uint8_t col = 0; col < MCP23008_COL; col++)
            {
                if ((cols & (1 << col)) == 0)
                {
                    mcp23008_reset_rows();
                    return row * MCP23008_COL + col;
                }
            }
        }
    }

	mcp23008_reset_rows();
    return key;
}

void mcp23008_reset_rows()
{
    mcp23008_write_cmd(MCP23008_GPIO_A, 0x0F);
}

bool mcp23008_any_key_pressed()
{
    return (mcp23008_read_columns() != 0x0F);
}
