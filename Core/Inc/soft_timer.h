#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#include <main.h>
#include <stdint.h>

// USER SETTING

#define MAX_TIMER 6
#define TICK 100 // 1 tick timer = 100uS

// convert time
#define TICK_PER_MS  (1000 / TICK)      // = 10
#define TICK_PER_SEC (1000000 / TICK)   // = 10000
#define MS(x)   ((x) * TICK_PER_MS)
#define SEC(x)  ((x) * TICK_PER_SEC)

void setTimer(uint8_t index, uint32_t counter);
void triggerTimerNow(uint8_t index);
uint8_t getTimer(uint8_t index);
void clearTimer(uint8_t index);
void timerRun();

/*
 * Timer 0: Button scanner
 * Timer 1: Led Blink
 * Timer 2: Polling Package
 * Timer 3: Send KNX package
 * Timer 4: Receive KNX package
 * Timer 5: Buzzer timing
 * */

#endif
