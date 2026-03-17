#ifndef FSM_MAIN_PROCESSING
#define FSM_MAIN_PROCESSING

#include <main.h>
#include <stdint.h>
#include <string.h>
#include <fsm_input_processing.h>
#include <fsm_output_processing.h>
#include <crc.h>
// USER SETTING

#define HEADER_RS485 0x07
#define HEADER_KNX 0x01
#define HEADER_ERROR_SYS 0x02
#define HEADER_RESET 0x03
#define HEADER_KNX_ERROR 0x04


#define STATE_INIT 1
#define STATE_WAITTING 2
#define STATE_RECEIVE_KNX_FRAME 3
#define STATE_RECEIVE_RS485_FRAME 4
#define STATE_BTN_PRESS_5S 5
#define STATE_PROCESS_DOWNLINK 6

#define NUMBER_COUPLER 3
#define MAX_RETRY_POLLING 3

#define TEMP_TIMER_2 3000
#define TEMP_TIMER_3 1000

// END USER SETTING

void main_processing_init();
void main_processing_run();
void poll_interval_10ms();

#endif
