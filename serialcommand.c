#include "serialcommand.h"

static SerialCommand_Config* config;
static uint8_t command_cnt;

char UartRxBuffer[RX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
volatile uint32_t BufPtrIn = 0;/* Increment this pointer or roll it back to
                               start address when data are received over USART */
uint32_t BufPtrOut = 0; /* Increment this pointer or roll it back to start address when data are sent over USB */

char* last, *token;

static void configure(SerialCommand_Config* config_in, uint8_t cnt_in) {
	config = config_in;
	command_cnt = cnt_in;
	if (HAL_UART_Receive_IT(config->p_uart, (uint8_t *)UartRxBuffer, 1) != HAL_OK) {
	    /* Transfer error in reception process */
		Error_Handler();
	}
}
static char* next() {
	char *nextToken;
	nextToken = strtok_r(NULL, " ", &last); 
	return nextToken; 
}

static void clear_buffer(void) {
	uint8_t i;
	for (i = 0; i < RX_DATA_SIZE; i++)
	{
		UartRxBuffer[i] = ' ';
	}
	BufPtrIn = 0;
	BufPtrOut = 0;
}
static void process_command(void) {
	uint8_t i;

	token = strtok_r(UartRxBuffer, " ", &last);
	for (i = 0; i < command_cnt; i++)
	{
		if (strncmp(token, config->commands[i].command, strlen(config->commands[i].command)) == 0) 
		{
			config->commands[i].function();	
		}
	}
}
//timer to check uart buffer for any new commands
void uartCallback(void const * argument) {
	uint32_t buffptr;
	uint32_t buffsize;
	uint8_t i;

	if (BufPtrOut != BufPtrIn) {
		if (BufPtrOut > BufPtrIn) /* rollback */ {
			buffsize = RX_DATA_SIZE - BufPtrOut;
		}
		else {
			buffsize = BufPtrIn - BufPtrOut;
		}

		buffptr = BufPtrOut;

		BufPtrOut += buffsize;
		if (BufPtrOut == RX_DATA_SIZE) {
			BufPtrOut = 0;
		}
		for (i = 0; i < buffsize; i++) {
			//user has finished entering their command - let's show them what they've won
			if (UartRxBuffer[buffptr + i] == '\r') {
				process_command();		
				clear_buffer();		
				HAL_UART_Abort(config->p_uart);
				HAL_UART_Receive_IT(config->p_uart, (uint8_t *)(UartRxBuffer + BufPtrIn), 1);
			}			
		}
	}
}
//uart irq - restart the uart irq to pull off the next for the next byte and manage the ring buffer
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	//echo the character back over uart
	HAL_UART_Transmit_IT(huart, (uint8_t *)(UartRxBuffer + BufPtrIn), 1);

	//increment the "next" pointer
	BufPtrIn++;

	//wrap the ring buffer if we've overflowed
	if (BufPtrIn == RX_DATA_SIZE) {
		BufPtrIn = 0;
	}

	//re-arm the uart irq
	HAL_UART_Receive_IT(huart, (uint8_t *)(UartRxBuffer + BufPtrIn), 1);
	
}

const struct serialcommand SerialCommand = { 
	.configure = configure,
	.next = next
};
