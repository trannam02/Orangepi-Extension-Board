/*
 * button.c
 *
 *  Created on: Feb 1, 2026
 *      Author: Nam
 */


#include <button.h>
#include <logger.h>
void getKeyInput();

void button_init(){
	clearTimer(0);
	setTimer(0, MS(10));
};
void button_run(){
	if(getTimer(0) == 1){
		clearTimer(0);
		setTimer(0, MS(10));
		getKeyInput();
	}
};

int keyReg0s[NO_BUTTON] = {BUTTON_STATE_PRESS,BUTTON_STATE_PRESS };
int keyReg1s[NO_BUTTON] = {BUTTON_STATE_PRESS,BUTTON_STATE_PRESS };
int keyReg2s[NO_BUTTON] = {BUTTON_STATE_PRESS,BUTTON_STATE_PRESS };
int keyRegStables[NO_BUTTON] = {BUTTON_STATE_PRESS,BUTTON_STATE_PRESS} ;

int longPressDurations[NO_BUTTON] = {LONG_PRESS_DURATION,LONG_PRESS_DURATION};

int states[NO_BUTTON] = {RELEASED,RELEASED};
int pressedFlags[NO_BUTTON] = {0,0};
int longPressedFlags[NO_BUTTON] = {0,0};


int getButtonPressFlag(int index){return pressedFlags[index];};
void setButtonPressFlag(int index, int value){pressedFlags[index] = value;};

int getButtonLongPressFlag(int index){return longPressedFlags[index];};
void setButtonLongPressFlag(int index, int value){longPressedFlags[index] = value;};

void getKeyInput() {
	keyReg2s[0] = keyReg1s[0];
	keyReg1s[0] = keyReg0s[0];
	keyReg0s[0] = HAL_GPIO_ReadPin(BTN_1_GPIO_Port, BTN_1_Pin);

	keyReg2s[1] = keyReg1s[1];
	keyReg1s[1] = keyReg0s[1];
	keyReg0s[1] = HAL_GPIO_ReadPin(BTN_2_GPIO_Port, BTN_2_Pin);

	for (int i = 0; i < NO_BUTTON; i++) {
		if ((keyReg0s[i] == keyReg1s[i]) && (keyReg1s[i] == keyReg2s[i])) {
			keyRegStables[i] = keyReg0s[i];
			switch (states[i]) {
			case RELEASED:
				if (keyRegStables[i] == BUTTON_STATE_PRESS) {
					states[i] = PRESSED;
					longPressDurations[i] = LONG_PRESS_DURATION;
				}
				;
				break;
			case PRESSED:
				if (keyRegStables[i] == BUTTON_STATE_RELEASE) {
					pressedFlags[i] = 1;
					HAL_GPIO_TogglePin(LED_DEBUG_KIT_GPIO_Port, LED_DEBUG_KIT_Pin);
					states[i] = RELEASED;
				} else {
					longPressDurations[i]--;
				}
				;
				if (longPressDurations[i] == 0) {
					longPressedFlags[i] = 1;
					states[i] = LONG_PRESSED;
					longPressDurations[i] = LONG_PRESS_DURATION;
				}
				;
				break;
			case LONG_PRESSED:
				if (keyRegStables[i] == BUTTON_STATE_RELEASE) {
					states[i] = RELEASED;
				} else {
					longPressDurations[i]--;
				}
				;
				if (longPressDurations[i] == 0) {
					longPressedFlags[i] = 1;
					longPressDurations[i] = LONG_PRESS_DURATION;
				}
				;
				break;
			default:
				break;
			};
		};
	};
}
;
