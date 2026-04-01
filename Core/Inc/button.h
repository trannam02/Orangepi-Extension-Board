/*
 * button.h
 *
 *  Created on: Feb 1, 2026
 *      Author: Nam
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <main.h>
#include <stdint.h>
#include <soft_timer.h>

// define parameter
#define NO_BUTTON 2

#define RELEASED 0
#define PRESSED 1
#define LONG_PRESSED 2

#define LONG_PRESS_DURATION 300 // 10ms -- 1 lan, vay 3s se -- 300 lan


#define BUTTON_STATE_PRESS 0
#define BUTTON_STATE_RELEASE 1

void button_init();
void button_run();

int getButtonPressFlag(int index);
void setButtonPressFlag(int index, int value);
int getButtonLongPressFlag(int index);
void setButtonLongPressFlag(int index, int value);

#endif /* INC_BUTTON_H_ */
