#include "fsm_output_processing.h"
#include "fsm_input_processing.h"
#include "queue_utils.h"

// Khai báo extern vì các Queue này đã được cấp phát ở fsm_input_processing.c
extern MessageQueue_t o_UPLINKQueue;
extern MessageQueue_t o_RS485Queue;
extern MessageQueue_t o_KNXQueue;

static uint8_t txData[MAX_BUFFER_LEN];
uint8_t o_outputLedType = 0;

// Function prototype
void outputLed();
void outputRS485();
void outputKNX();
void outputUPLINK();

void output_processing_init() {
    HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
}

void output_processing_run() {
    outputKNX();
    outputRS485();
    outputUPLINK();
    outputLed();
}

void outputUPLINK() {
    // Kích hoạt DMA nếu có data trong Queue VÀ phần cứng đang rảnh
    if (o_UPLINKQueue.count > 0 && huart2.gState == HAL_UART_STATE_READY) {

        if (Queue_Pop(&o_UPLINKQueue, txData)) {
            uint8_t len = txData[0]; // Byte 0 định nghĩa sẵn là chiều dài
            if (len > 0) {
                LOG_INFO("DMA Transmit UPLINK Started: len=%d", len);
                HAL_UART_Transmit_DMA(&huart2, &txData[1], len); // Truyền từ byte 1
            }
        }
    }
}

void outputKNX() {
    if (o_KNXQueue.count > 0 && huart3.gState == HAL_UART_STATE_READY) {

        if (Queue_Pop(&o_KNXQueue, txData)) {
            uint8_t len = txData[0];
            if (len > 0) {
                LOG_INFO("DMA Transmit KNX Started: len=%d", len);
                HAL_UART_Transmit_DMA(&huart3, &txData[1], len);
            }
        }
    }
}

void outputRS485() {
    if (o_RS485Queue.count > 0 && huart1.gState == HAL_UART_STATE_READY) {

        if (Queue_Pop(&o_RS485Queue, txData)) {
            uint8_t len = txData[0];
            if (len > 0) {
                uart1TxFlag = UART_TX_UN_AVAILABLE_FLAG; // Giữ lại cờ phục vụ logic riêng của bạn

                // ==========================================
                // ĐOẠN FORMAT HEX LOG TRƯỚC KHI TRUYỀN
                // ==========================================
                char hexString[128] = {0};
                int offset = 0;

                // Giới hạn số lượng byte in ra để tránh tràn buffer
                uint8_t print_len = (len > 40) ? 40 : len;

                // Lặp qua payload (từ index 1 đến len)
                for(uint8_t i = 1; i <= print_len; i++) {
                    offset += sprintf(hexString + offset, "%02X ", txData[i]);
                }

                LOG_WARN("UART1 TX (Len: %d) RAW: %s", len, hexString);
                // ==========================================

                // Kích hoạt DMA truyền dữ liệu
                HAL_UART_Transmit_DMA(&huart1, &txData[1], len);
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
                setTimer(1, 500);
            }
            break;
        case LED_CODE_BLINK_5HZ:
            if (getTimer(1)) {
                HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
                clearTimer(1);
                setTimer(1, 200);
            }
            break;
        default:
            break;
    }
}
