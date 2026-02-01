#include "fsm_output_processing.h"

uint8_t outputLedType = 0;

static uint8_t state = STATE_OUTPUT_INIT;

void outputLed();

void output_processing_init(){
	HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
};
void output_processing_run(){
	// output data processing
	switch(state){
		case STATE_OUTPUT_INIT:
		{
			state = STATE_OUTPUT_WAITING;
			break;
		}
		case STATE_OUTPUT_WAITING:
		{
//			if(){ // buffer uart co data
//
//			}
//			if(){ // buffer knx co data
//
//				}
//			if(){ // buffer uplink co data
//
//				}
			break;
		}
		case STATE_OUTPUT_PROCESSING_RS485: // process buffer uart co data
		{
			state = STATE_OUTPUT_WAITING;
			break;
		}
		case STATE_OUTPUT_PROCESSING_KNX: // process buffer knx co data
		{
			state = STATE_OUTPUT_WAITING;
			break;
		}
		case STATE_OUTPUT_PROCESSING_UPLINK: // process buffer uplink co data
		{
			state = STATE_OUTPUT_WAITING;
			break;
		}
		default:
		{
			break;
		};

	};

	// output led processing
	outputLed();

	// output buzzer processing

};

void outputLed(){
	switch(outputLedType){
		case LED_CODE_OFF:
		{
			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
			break;
		}
		case LED_CODE_ON:
		{
			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
			break;
		}
		case LED_CODE_BLINK_1HZ:
		{
			if(getTimer(1)){
				HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
				clearTimer(1);
				setTimer(1,500);
			}
			break;
		}
		case LED_CODE_BLINK_5HZ:
		{
			if(getTimer(1)){
				HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
				clearTimer(1);
				setTimer(1, 200);
			}
			break;
		}
		default:
		{
			break;
		};

	}
};
