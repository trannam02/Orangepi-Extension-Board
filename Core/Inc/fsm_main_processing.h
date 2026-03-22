#ifndef FSM_MAIN_PROCESSING
#define FSM_MAIN_PROCESSING

#include <main.h>
#include <stdint.h>
#include <string.h>
#include <fsm_input_processing.h>
#include <fsm_output_processing.h>
#include <crc.h>
#include <knx.h>
#include <soft_timer.h>
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

#define MAX_RETRY_POLLING 3

#define POLL_INTERVAL SEC(1)
#define POLL_TIMEOUT MS(500)

// for poll
#define POLL_STATE_IDLE 0
#define POLL_STATE_WAIT_RESPONSE 1
#define NUMBER_COUPLER    3


// END USER SETTING

void main_processing_init();
void main_processing_run();


#endif
