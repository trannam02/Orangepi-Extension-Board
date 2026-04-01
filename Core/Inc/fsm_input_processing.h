// xu li doc tat ca cac input 1 luc va dua vao kho chua chung, de xu ly

#ifndef FMS_INPUT_PROCESSING_H
#define FMS_INPUT_PROCESSING_H

#include <main.h>
#include <stdint.h>
#include <string.h>
#include <uart.h>
#include <button.h>
#include <logger.h>
#include "queue_utils.h"
#include <soft_timer.h>
#include <global.h>

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


#define O_UPLINK_TX_MAX_BUFFER_SIZE 64
#define O_UPLINK_MAX_QUEUE_SIZE 10

// for RS485 downlink
#define O_RS485_TX_MAX_BUFFER_SIZE 64
#define O_RS485_MAX_QUEUE_SIZE 10

// for KNX downlink
#define O_KNX_TX_MAX_BUFFER_SIZE 64
#define O_KNX_MAX_QUEUE_SIZE 10








// for orp
extern MessageQueue_t i_ORPQueue;
extern MessageQueue_t i_rs485Queue;
extern MessageQueue_t i_KNXQueue;
extern MessageQueue_t o_UPLINKQueue;
extern MessageQueue_t o_RS485Queue;
extern MessageQueue_t o_KNXQueue;

extern uint8_t i_inputBtn1PressFlag;
extern uint8_t i_inputBtn1LongPressFlag;
extern uint8_t i_inputBtn2PressFlag;
extern uint8_t i_inputBtn2LongPressFlag;

void input_processing_init();
void input_processing_run();

#endif
