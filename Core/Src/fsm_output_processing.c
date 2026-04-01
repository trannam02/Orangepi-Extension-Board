#include "fsm_output_processing.h"
#include "fsm_input_processing.h"
#include "queue_utils.h"

// Khai báo extern vì các Queue này đã được cấp phát ở fsm_input_processing.c
extern MessageQueue_t o_UPLINKQueue;
extern MessageQueue_t o_RS485Queue;
extern MessageQueue_t o_KNXQueue;

static uint8_t txData[MAX_BUFFER_LEN];
uint8_t o_outputLedType = 0;


static uint8_t knx_tx_buffer[KNX_MAX_TX_LEN];
static uint8_t knx_tx_length = 0;             // Tổng chiều dài gói tin
static uint8_t knx_tx_index = 0;              // Vị trí byte đang gửi
static uint8_t knx_tx_is_sending = 0;             // Cờ trạng thái (1 = đang bận gửi)

static uint8_t processed_uplink = 0;
static uint8_t processed_rs485 = 0;
static uint8_t processed_knx = 0;
// Function prototype
void outputLed();
void outputRS485();
void outputKNX();
void outputUPLINK();

void output_processing_init() {
    HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
}

void output_processing_run() {
	processed_uplink = 0;
	processed_rs485 = 0;
	processed_knx = 0;

    outputKNX();
    outputRS485();
    outputUPLINK();
    outputLed();
}

void outputUPLINK() {
	static uint8_t dma_rs485_buffer[MAX_BUFFER_LEN];
    while(processed_uplink < CONCURENCY_RATE+5){
    	if (o_UPLINKQueue.count > 0 && huart2.gState == HAL_UART_STATE_READY) {
    	        if (Queue_Pop(&o_UPLINKQueue, dma_rs485_buffer)) {
    	            uint8_t len = dma_rs485_buffer[0];
    	            if (len > 0) {
//    	                LOG_INFO("DMA Transmit UPLINK Started: len=%d", len);
    	                HAL_UART_Transmit_DMA(&huart2, &dma_rs485_buffer[1], len);
    	            }
    	        }
    	    }
    	processed_uplink++;
    }
}

void knx_fsm_send_next_chunk(void) {
    if (!knx_tx_is_sending) return;

    uint8_t remaining = knx_tx_length - knx_tx_index;

    if (remaining > 0) {
        uint8_t chunk_size = (remaining >= 2) ? 2 : 1;

        HAL_UART_Transmit_DMA(&huart3, &knx_tx_buffer[knx_tx_index], chunk_size);
        LOG_WARN("KNX transmit chunk: %02X %02X", knx_tx_buffer[knx_tx_index], knx_tx_buffer[knx_tx_index+1]);
        knx_tx_index += chunk_size;

        if (knx_tx_index < knx_tx_length) {
        	clearTimer(3);
            setTimer(3, 5*TICK);
        } else {
        	knx_tx_is_sending = 0;
            LOG_WARN("KNX TX FSM Complete (%d bytes)", knx_tx_length);
        }
    }
}

void outputKNX() {
    if (o_KNXQueue.count > 0 && !knx_tx_is_sending && huart3.gState == HAL_UART_STATE_READY) {
        if (Queue_Pop(&o_KNXQueue, txData)) {
            uint8_t len = txData[0];
            if (len > 0 && len < KNX_MAX_TX_LEN) {
            	memcpy(knx_tx_buffer, &txData[1], len);
            	knx_tx_length = len;
            	knx_tx_index = 0;
            	knx_tx_is_sending = 1;
            	triggerTimerNow(3);
            }
        }
    }
    if(getTimer(3)){
    	knx_fsm_send_next_chunk();
    	clearTimer(3);
    	setTimer(3, 5*TICK);
    };
}

void outputRS485() {
	static uint8_t dma_rs485_buffer[MAX_BUFFER_LEN];
	while(processed_rs485 < CONCURENCY_RATE+5){
		processed_rs485++;

		if (o_RS485Queue.count > 0 && huart1.gState == HAL_UART_STATE_READY) {

		        if (Queue_Pop(&o_RS485Queue, dma_rs485_buffer)) {
		            uint8_t len = dma_rs485_buffer[0];
		            if (len > 0) {
		                HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, 1);
		                HAL_UART_Transmit_DMA(&huart1, &dma_rs485_buffer[1], len);
		            }
		        }
		    }
	}

}
void outputLed() {
    switch(o_outputLedType) {
        case LED_CODE_OFF:
            HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
            break;
        case LED_CODE_ON:
            HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
            break;
        case LED_CODE_BLINK_1HZ:
            if (getTimer(1)) {
                HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
                clearTimer(1);
                setTimer(1, MS(500));
            }
            break;
        case LED_CODE_BLINK_5HZ:
            if (getTimer(1)) {
                HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
                clearTimer(1);
                setTimer(1, MS(200));
            }
            break;
        default:
            break;
    }
}
