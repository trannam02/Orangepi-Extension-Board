#include "soft_timer.h"

uint8_t timerFlag[MAX_TIMER];
uint32_t timerCounter[MAX_TIMER];

void setTimer(uint8_t index, uint32_t counter){
	if(index >= 0 && index < MAX_TIMER){
		timerCounter[index] = counter;
	};
};

void triggerTimerNow(uint8_t index){
	timerFlag[index] = 1;
};

uint8_t getTimer(uint8_t index){
	if(index >= 0 && index < MAX_TIMER){
		return timerFlag[index];
	};
	return 0;
};
void clearTimer(uint8_t index){
	timerFlag[index] = 0;
	timerCounter[index] = 0;
};
void timerRun(){
	for(int i = 0; i < MAX_TIMER; i++){
		if(timerCounter[i] > 0){
			timerCounter[i]--;
		};
		if(timerCounter[i] <= 0){
			timerFlag[i] = 1;
		};
	};
};
