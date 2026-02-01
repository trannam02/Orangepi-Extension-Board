#include <fsm_main_processing.h>

static uint8_t state = STATE_INIT;

void main_processing_init(){
	state = STATE_INIT;
};
void main_processing_run(){
	switch(state){
		case STATE_INIT:
		{
			state = STATE_WAITTING;
			break;
		}
		case STATE_WAITTING:
		{
			if(rs485NumEl != 0){
				state = STATE_RECEIVE_RS485_FRAME;
				break;
			}
			if(inputBtn1PressFlag){
				inputBtn1PressFlag = 0;
				state = STATE_BTN_PRESS_5S;
				break;
			};
			break;
		}
		case STATE_RECEIVE_RS485_FRAME:
		{
			// copy all data from rs485 queue to uplink queue
			for(uint8_t i = 0; i < rs485NumEl; i++){
				// copy data roi add them header
				// dua data vao ouput buffer for uplink
			};
			rs485NumEl = 0;
			state = STATE_WAITTING;
			break;
		}
		case STATE_RECEIVE_KNX_FRAME:
		{
			state = STATE_WAITTING;
			break;
		}
		case STATE_BTN_PRESS_5S: // RESET SYSTEM - ten tam thoi la press 5s
		{
			if(outputLedType == LED_CODE_BLINK_5HZ){
				outputLedType = LED_CODE_OFF;
			}else{
				outputLedType = LED_CODE_BLINK_5HZ;
				clearTimer(1);
				setTimer(1, 200);
			}

			state = STATE_WAITTING;
			break;
		}
		default:
		{
			break;
		};

	}
	// Flag Uart 1 - RX

	// Flag Uart 2 - RX

	// Flag Uart 3 - RX

	// Flag button press 5s

};
