#include "fsm_input_processing.h"

static uint8_t state = STATE_INPUT_INIT;

// repository for Orange PI
uint8_t i_ORPQueue[I_ORP_MAX_QUEUE_SIZE][RX_MAX_BUFFER_SIZE + 1]; // plus 1 for '\0'
uint8_t i_ORPQueueIndex = 0;
uint8_t i_ORPNumEl = 0;

// repository for RS485
uint8_t i_rs485Queue[RS485_MAX_QUEUE][RX_MAX_BUFFER_SIZE + 1];
uint8_t i_rs485QueueIndex = 0;
uint8_t i_rs485NumEl = 0;

// repository for KNX
uint8_t i_KNXQueue[I_KNX_MAX_QUEUE_SIZE][RX_MAX_BUFFER_SIZE + 1];
uint8_t i_KNXQueueIndex = 0;
uint8_t i_KNXNumEl = 0;

// repository for BUTTON
uint8_t i_inputBtn1PressFlag = 0;


void input_processing_init(){};
void input_processing_run(){
	switch(state){
			case STATE_INPUT_INIT:
			{
				state = STATE_INPUT_WAITTING;
				break;
			}
			case STATE_INPUT_WAITTING:
			{
				if(uart1RxFlag == 1){
					uart1RxFlag = 0;
					state = STATE_INPUT_RS485_UART1_RECEIVE;
				};
				if(uart2RxFlag == 1){
					uart2RxFlag = 0;
					state = STATE_INPUT_ORP_UART2_RECEIVE;
				};
				if(uart3RxFlag == 1){
					uart3RxFlag = 0;
					state = STATE_INPUT_KNX_UART3_RECEIVE;
				};
				if(getButtonPressFlag(0)){
					setButtonPressFlag(0,0);
					state = STATE_INPUT_BUTTON_PRESS;
				}
				break;
			}
			case STATE_INPUT_ORP_UART2_RECEIVE:
			{
				if(i_ORPNumEl == I_ORP_MAX_QUEUE_SIZE){
					LOG_INFO("Input queue overflow, drop package!!!");
					state = STATE_INPUT_WAITTING;
					break;
				}
				memcpy(i_ORPQueue[i_ORPQueueIndex], uart2RxBuffer, uart2RxBuffer[0]); // queue index la vi tri van con trong
				i_ORPQueueIndex = (i_ORPQueueIndex + 1) % I_ORP_MAX_QUEUE_SIZE;
				i_ORPNumEl = (i_ORPNumEl >= I_ORP_MAX_QUEUE_SIZE) ? I_ORP_MAX_QUEUE_SIZE : i_ORPNumEl + 1;

				LOG_DEBUG("Input UART Received: data=%s index=%d num_el=%d", i_ORPQueue[(i_ORPQueueIndex - 1)%I_ORP_MAX_QUEUE_SIZE], i_ORPQueueIndex, i_ORPNumEl);

				state = STATE_INPUT_WAITTING;
				break;
			}
			case STATE_INPUT_RS485_UART1_RECEIVE:
			{
				if(i_rs485NumEl == RS485_MAX_QUEUE){
					LOG_INFO("Input queue overflow, drop package!!!");
					state = STATE_INPUT_WAITTING;
					break;
				}
				memcpy(i_rs485Queue[i_rs485QueueIndex], uart1RxBuffer, uart1RxBuffer[1]); // queue index la vi tri van con trong
				i_rs485QueueIndex = (i_rs485QueueIndex + 1) % RS485_MAX_QUEUE;
				i_rs485NumEl = (i_rs485NumEl >= RS485_MAX_QUEUE) ? RS485_MAX_QUEUE : i_rs485NumEl + 1;

				LOG_DEBUG("Input UART-RS485 Received: data=%s index=%d num_el=%d", i_rs485Queue[(i_rs485QueueIndex - 1)%RS485_MAX_QUEUE], i_rs485QueueIndex, i_rs485NumEl);

				state = STATE_INPUT_WAITTING;
				break;
			}
			case STATE_INPUT_KNX_UART3_RECEIVE:
			{
				if(i_KNXNumEl == I_KNX_MAX_QUEUE_SIZE){
					LOG_INFO("Input queue overflow, drop package!!!");
					state = STATE_INPUT_WAITTING;
					break;
				}
				memcpy(i_KNXQueue[i_KNXQueueIndex], uart3RxBuffer, uart3RxBuffer[1]); // queue index la vi tri van con trong
				i_KNXQueueIndex = (i_KNXQueueIndex + 1) % I_KNX_MAX_QUEUE_SIZE;
				i_KNXNumEl = (i_KNXNumEl >= I_KNX_MAX_QUEUE_SIZE) ? I_KNX_MAX_QUEUE_SIZE : i_KNXNumEl + 1;

				LOG_DEBUG("Input UART-KNX Received: data=%s index=%d num_el=%d", i_KNXQueue[(i_KNXQueueIndex - 1)%I_KNX_MAX_QUEUE_SIZE], i_KNXQueueIndex, i_KNXNumEl);

				state = STATE_INPUT_WAITTING;
				break;
			}
			case STATE_INPUT_BUTTON_PRESS:
			{
				i_inputBtn1PressFlag = 1;
				LOG_DEBUG("Input Button Pressed");
				state = STATE_INPUT_WAITTING;
				break;
			}
			default:
			{
				break;
			};

		};
};
