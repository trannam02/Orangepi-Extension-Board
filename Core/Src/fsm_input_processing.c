#include "fsm_input_processing.h"

static uint8_t state = STATE_INPUT_INIT;

// repository for Orange PI
//uint8_t rs485Queue[RX_MAX_BUFFER_SIZE + 1][RS485_MAX_QUEUE]; // plus 1 for '\0'
//uint8_t rs485QueueIndex = 0;
//uint8_t rs485NumEl = 0;

// repository for RS485
//uint8_t rs485Queue[RX_MAX_BUFFER_SIZE + 1][RS485_MAX_QUEUE]; // plus 1 for '\0'
uint8_t i_rs485Queue[RS485_MAX_QUEUE][RX_MAX_BUFFER_SIZE + 1];
uint8_t i_rs485QueueIndex = 0;
uint8_t i_rs485NumEl = 0;

// repository for KNX

// repository for BUTTON
uint8_t inputBtn1PressFlag = 0;


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
				if(getButtonPressFlag(0)){
					setButtonPressFlag(0,0);
					state = STATE_INPUT_BUTTON_PRESS;
				}
				break;
			}
			case STATE_INPUT_RS485_UART1_RECEIVE:
			{
				memcpy(i_rs485Queue[i_rs485QueueIndex], uart1RxBuffer, strlen(uart1RxBuffer) + 1); // queue index la vi tri van con trong
				i_rs485QueueIndex = (i_rs485QueueIndex + 1) % RS485_MAX_QUEUE;
				i_rs485NumEl = (i_rs485NumEl >= RS485_MAX_QUEUE) ? RS485_MAX_QUEUE : i_rs485NumEl + 1;

				LOG_DEBUG("Input UART Received: data=%s index=%d num_el=%d", i_rs485Queue[(i_rs485QueueIndex - 1)%RS485_MAX_QUEUE], i_rs485QueueIndex, i_rs485NumEl);

				state = STATE_INPUT_WAITTING;
				break;
			}
			case STATE_INPUT_KNX_UART2_RECEIVE:
			{

				break;
			}
			case STATE_INPUT_BUTTON_PRESS:
			{
				inputBtn1PressFlag = 1;
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
