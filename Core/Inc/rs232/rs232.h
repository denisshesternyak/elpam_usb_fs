
#ifndef INC_RS232_RS232_H_
#define INC_RS232_RS232_H_

#include "main.h"
#include <stdint.h>

#define CMD_LENGTH 		7	// Command length — 7 characters
#define ARMING_TIME  	10000
#define ACTIVATION_CMD_TIMEOUT  	100


typedef void (*rs232_cmd_handler_t)(void);
typedef void (*rs232_volume_handler_t)(int step);


void rs232_init(UART_HandleTypeDef *huart);
void rs232_process(void);

// Functions for registering handlers
void rs232_register_arm(rs232_cmd_handler_t handler);
void rs232_register_all_clear_1(rs232_cmd_handler_t handler);
void rs232_register_all_clear_2(rs232_cmd_handler_t handler);
void rs232_register_alarm(rs232_cmd_handler_t handler);
void rs232_register_chemical(rs232_cmd_handler_t handler);
void rs232_register_disarm(rs232_cmd_handler_t handler);
void rs232_register_cancel(rs232_cmd_handler_t handler);
void rs232_register_quiet_test(rs232_cmd_handler_t handler);
void rs232_register_reserve_1(rs232_cmd_handler_t handler);
void rs232_register_reserve_2(rs232_cmd_handler_t handler);
void rs232_register_reserve_3(rs232_cmd_handler_t handler);
void rs232_register_report(rs232_cmd_handler_t handler);
void rs232_register_remote_pa(rs232_cmd_handler_t handler);
void rs232_register_reset(rs232_cmd_handler_t handler);
void rs232_register_volume_up(rs232_volume_handler_t handler);
void rs232_register_volume_down(rs232_volume_handler_t handler);
void rs232_register_unknown(rs232_cmd_handler_t handler);


#endif /* INC_RS232_RS232_H_ */
