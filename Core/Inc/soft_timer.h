#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#include <main.h>
#include <stdint.h>

// USER SETTING

#define MAX_TIMER 4
#define TICK 1 // 1 tick timer = 1ms

void setTimer(uint8_t index, uint32_t counter);
uint8_t getTimer(uint8_t index);
void clearTimer(uint8_t index);
void timerRun();

/*
 * Timer 0: Button scanner
 * Timer 1: Led Blink
 * Timer 2: Polling Package
 *
 * */

#endif
