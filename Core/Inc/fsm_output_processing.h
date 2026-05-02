/*
 * fsm_output_processing.h
 *
 * xu ly transmit uart, blink led, ....
 *  Created on: Jan 31, 2026
 *      Author: Nam
 */

#ifndef INC_FSM_OUTPUT_PROCESSING_H_
#define INC_FSM_OUTPUT_PROCESSING_H_

#include <main.h>
#include <soft_timer.h>
#include <uart.h>
#include <logger.h>
#include <knx.h>

/////////////////////// DEFINE ///////////////
// concurency rate (1-max queue)
// 1 			= max
// max queue 	= min
#define CONCURENCY_RATE 2

// for uplink
#define O_UPLINK_TX_MAX_BUFFER_SIZE 64
#define O_UPLINK_MAX_QUEUE_SIZE 10

// for RS485 downlink
#define O_RS485_TX_MAX_BUFFER_SIZE 64
#define O_RS485_MAX_QUEUE_SIZE 10

// for KNX downlink
#define O_KNX_TX_MAX_BUFFER_SIZE 64
#define O_KNX_MAX_QUEUE_SIZE 10

// for main state machine
#define STATE_OUTPUT_INIT 1
#define STATE_OUTPUT_WAITING 2
#define STATE_OUTPUT_PROCESSING_RS485 3
#define STATE_OUTPUT_PROCESSING_KNX 4
#define STATE_OUTPUT_PROCESSING_UPLINK 5

// for LED
#define LED_CODE_OFF 1
#define LED_CODE_ON 2
#define LED_CODE_BLINK_1HZ 3
#define LED_CODE_BLINK_4HZ 4

// for Buzzer
#define BUZZER_OFF 0
#define BUZZER_ON 1


#define RS485_OUTPUT_INTERVAL MS(30)

typedef enum {
    BUZZER_CODE_OFF = 0,
    BUZZER_CODE_SUCCESS, // Kêu "Tít - Tít" (âm thanh cao, nhanh)
    BUZZER_CODE_ERROR,   // Kêu "Tò te" (âm trầm, kéo dài)
    BUZZER_CODE_ALARM,    // Kêu "Bíp ... Bíp ..." liên tục (Cảnh báo)
	BUZZER_CODE_FOR_DISCONNECTED_COUPLER,
	BUZZER_CODE_FOR_CONNECTED_COUPLER,
	BUZZER_CODE_FOR_BEEP_1S
} BuzzerCode_t;
extern BuzzerCode_t o_outputBuzzerType;

/////////////////////// EXTERN ///////////////
extern uint8_t o_outputLedType;


// for orange pi uplink
/////////////////////// FUNCTION ///////////////
void output_processing_init();
void output_processing_run();

void setBuzzerPWM(uint16_t freq);


#endif /* INC_FSM_OUTPUT_PROCESSING_H_ */
