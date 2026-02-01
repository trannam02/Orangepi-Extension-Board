// xu li doc tat ca cac input 1 luc va dua vao kho chua chung, de xu ly

#ifndef FMS_INPUT_PROCESSING_H
#define FMS_INPUT_PROCESSING_H

#include <main.h>
#include <stdint.h>
#include <string.h>
#include <uart.h>
#include <button.h>

#define RS485_MAX_QUEUE 10


// DEFINE STATE
#define STATE_INPUT_INIT 1
#define STATE_INPUT_WAITTING 2
#define STATE_INPUT_RS485_UART1_RECEIVE 3
#define STATE_INPUT_KNX_UART2_RECEIVE 4
#define STATE_INPUT_BUTTON_PRESS 5

extern uint8_t rs485NumEl;
extern uint8_t rs485Queue[RX_MAX_BUFFER_SIZE + 1][RS485_MAX_QUEUE];
extern uint8_t rs485QueueIndex;

extern uint8_t inputBtn1PressFlag;

void input_processing_init();
void input_processing_run();

#endif
