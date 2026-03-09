// xu li doc tat ca cac input 1 luc va dua vao kho chua chung, de xu ly

#ifndef FMS_INPUT_PROCESSING_H
#define FMS_INPUT_PROCESSING_H

#include <main.h>
#include <stdint.h>
#include <string.h>
#include <uart.h>
#include <button.h>
#include <logger.h>

#define RS485_MAX_QUEUE 10
#define I_KNX_MAX_QUEUE_SIZE 10
#define I_ORP_MAX_QUEUE_SIZE 10
// DEFINE STATE
#define STATE_INPUT_INIT 1
#define STATE_INPUT_WAITTING 2
#define STATE_INPUT_RS485_UART1_RECEIVE 3
#define STATE_INPUT_KNX_UART3_RECEIVE 4
#define STATE_INPUT_BUTTON_PRESS 5
#define STATE_INPUT_ORP_UART2_RECEIVE 6

// for orp
extern uint8_t i_ORPNumEl;
extern uint8_t i_ORPQueue[I_ORP_MAX_QUEUE_SIZE][RX_MAX_BUFFER_SIZE + 1];
extern uint8_t i_ORPQueueIndex;

// for rs485
extern uint8_t i_rs485NumEl;
extern uint8_t i_rs485Queue[RS485_MAX_QUEUE][RX_MAX_BUFFER_SIZE + 1];
extern uint8_t i_rs485QueueIndex;

// for KNX
extern uint8_t i_KNXQueue[I_KNX_MAX_QUEUE_SIZE][RX_MAX_BUFFER_SIZE + 1];
extern uint8_t i_KNXQueueIndex;
extern uint8_t i_KNXNumEl;

extern uint8_t i_inputBtn1PressFlag;

void input_processing_init();
void input_processing_run();

#endif
