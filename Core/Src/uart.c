#include <uart.h>
#include <stdio.h>

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

static uint16_t oldPos1 = 0;
static uint16_t oldPos2 = 0;
static uint16_t oldPos3 = 0;

uint8_t dmaUart1RxBuffer[RX_MAX_BUFFER_SIZE + 1]; // plus 1 for \0
uint8_t uart1RxBuffer[RX_MAX_BUFFER_SIZE+1];
uint8_t uart1RxFlag = 0;

uint8_t dmaUart2RxBuffer[RX_MAX_BUFFER_SIZE + 1]; // plus 1 for \0
uint8_t uart2RxBuffer[RX_MAX_BUFFER_SIZE+1];
uint8_t uart2RxFlag = 0;

uint8_t dmaUart3RxBuffer[RX_MAX_BUFFER_SIZE + 1]; // plus 1 for \0
uint8_t uart3RxBuffer[RX_MAX_BUFFER_SIZE+1];
uint8_t uart3RxFlag = 0;

void uartManualInit(){
	HAL_UART_Receive_DMA(&huart1, dmaUart1RxBuffer, RX_MAX_BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart2, dmaUart2RxBuffer, RX_MAX_BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart3, dmaUart3RxBuffer, RX_MAX_BUFFER_SIZE);
//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
};


void uart1ProcessWhenIdleFlag(){
	__HAL_UART_CLEAR_IDLEFLAG(&huart1);

    uint16_t remaining = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
    uint16_t newPos = RX_MAX_BUFFER_SIZE - remaining;
    if (newPos == oldPos1) {
        return;
    }

    uint16_t len = 0;

    if (newPos > oldPos1) {
        len = newPos - oldPos1;
        memcpy(&uart1RxBuffer[1], &dmaUart1RxBuffer[oldPos1], len);
        uart1RxBuffer[0] = (uint8_t)len;
    }else {
        uint16_t len1 = RX_MAX_BUFFER_SIZE - oldPos1;
        uint16_t len2 = newPos;

        len = len1 + len2;

        memcpy(&uart1RxBuffer[1], &dmaUart1RxBuffer[oldPos1], len1);
        memcpy(&uart1RxBuffer[1 + len1], &dmaUart1RxBuffer[0], len2);
        uart1RxBuffer[0] = (uint8_t)len;
    }

    uart1RxFlag = 1;

    oldPos1 = newPos;

    LOG_WARN("uart 1");
}

void uart2ProcessWhenIdleFlag(){
	__HAL_UART_CLEAR_IDLEFLAG(&huart2);

    uint16_t remaining = __HAL_DMA_GET_COUNTER(huart2.hdmarx);
    uint16_t newPos = RX_MAX_BUFFER_SIZE - remaining;
    if (newPos == oldPos2) {
        return;
    }

    uint16_t len = 0;

    if (newPos > oldPos2) {
        len = newPos - oldPos2;
        memcpy(&uart2RxBuffer[1], &dmaUart2RxBuffer[oldPos2], len);
        uart2RxBuffer[0] = (uint8_t)len;
    }else {
        uint16_t len1 = RX_MAX_BUFFER_SIZE - oldPos2;
        uint16_t len2 = newPos;

        len = len1 + len2;

        memcpy(&uart2RxBuffer[1], &dmaUart2RxBuffer[oldPos2], len1);
        memcpy(&uart2RxBuffer[1 + len1], &dmaUart2RxBuffer[0], len2);
        uart2RxBuffer[0] = (uint8_t)len;
    }

    uart2RxFlag = 1;

    oldPos2 = newPos;

    // ==========================================
        // ĐOẠN CODE GOM CHUỖI LOG MỚI
        // ==========================================
//        uint8_t payload_len = uart2RxBuffer[0];
//        char hexString[128] = {0};
//        int offset = 0;
//
//        // Giới hạn số lượng in ra để không bị tràn mảng chuỗi (nếu gói quá dài)
//        uint8_t print_len = (payload_len > 40) ? 40 : payload_len;
//
//        // Lặp từ 0 đến <= print_len để in trọn vẹn từ byte Length (index 0) đến byte cuối cùng
//        for(uint8_t i = 0; i <= print_len; i++) {
//            offset += sprintf(hexString + offset, "%02X ", uart2RxBuffer[i]);
//        }
//
//        LOG_WARN("Downlink receive (Len: %d): %s", payload_len, hexString);
        // ==========================================
}

void uart3ProcessWhenIdleFlag(){
	__HAL_UART_CLEAR_IDLEFLAG(&huart3);

    uint16_t remaining = __HAL_DMA_GET_COUNTER(huart3.hdmarx);
    uint16_t newPos = RX_MAX_BUFFER_SIZE - remaining;
    if (newPos == oldPos3) {
        return;
    }

    uint16_t len = 0;

    if (newPos > oldPos3) {
        len = newPos - oldPos3;
        memcpy(&uart3RxBuffer[1], &dmaUart3RxBuffer[oldPos3], len);
        uart3RxBuffer[0] = (uint8_t)len;
    }else {
        uint16_t len1 = RX_MAX_BUFFER_SIZE - oldPos3;
        uint16_t len2 = newPos;

        len = len1 + len2;

        memcpy(&uart3RxBuffer[1], &dmaUart3RxBuffer[oldPos3], len1);
        memcpy(&uart3RxBuffer[1 + len1], &dmaUart3RxBuffer[0], len2);
        uart3RxBuffer[0] = (uint8_t)len;
    }

    uart3RxFlag = 1;

    oldPos3 = newPos;

}




