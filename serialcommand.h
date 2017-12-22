#pragma once

#include "stm32f1xx_hal.h"
#include "string.h"
#include "stdbool.h"

#define RX_DATA_SIZE  32
                                 

typedef struct {
	const char* command;
	void(*function)();
} SerialCommand_Command;

typedef struct {
	UART_HandleTypeDef* p_uart;
	uint8_t command_cnt;
	SerialCommand_Command *commands;
} SerialCommand_Config;

struct serialcommand {
	void(*configure)(SerialCommand_Config* config_in, uint8_t cnt_in);	
	char*(*next)(void);	
};

extern const struct serialcommand SerialCommand;

