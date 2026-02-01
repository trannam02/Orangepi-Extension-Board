#ifndef FSM_MAIN_PROCESSING
#define FSM_MAIN_PROCESSING

#include <main.h>
#include <stdint.h>
#include <string.h>
#include <fsm_input_processing.h>
#include <fsm_output_processing.h>

// USER SETTING

#define STATE_INIT 1
#define STATE_WAITTING 2
#define STATE_RECEIVE_KNX_FRAME 3
#define STATE_RECEIVE_RS485_FRAME 4
#define STATE_BTN_PRESS_5S 5
#define STATE_PROCESS_DOWNLINK 6

// END USER SETTING

void main_processing_init();
void main_processing_run();

#endif
