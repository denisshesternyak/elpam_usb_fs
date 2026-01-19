#ifndef INC_KEYS_MCP23008_H_
#define INC_KEYS_MCP23008_H_

#include "main.h"
#include "mcp23008_regs.h"
#include <stdbool.h>

#define MCP23008_I2C_ADDRESS 	0x40
#define MCP23008_ROW 			4
#define MCP23008_COL			4

void mcp23008_init_keyboard();
void mcp23008_set_row(uint8_t row_num);
void mcp23008_reset_rows();
uint8_t mcp23008_read_columns();
uint8_t mcp23008_scan_keyboard();
bool mcp23008_any_key_pressed();
uint8_t mcp23008_read_reg(uint8_t reg);

#endif /* INC_KEYS_MCP23008_H_ */
