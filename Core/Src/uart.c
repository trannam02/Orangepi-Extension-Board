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

uint8_t uart1TxFlag = UART_TX_AVAILABLE_FLAG; // available
void uartManualInit(){
	HAL_UART_Receive_DMA(&huart1, dmaUart1RxBuffer, RX_MAX_BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart2, dmaUart2RxBuffer, RX_MAX_BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart3, dmaUart3RxBuffer, RX_MAX_BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
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

    LOG_DEBUG("uart 1");
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
	// ĐOẠN CODE MỚI THÊM ĐỂ IN RAW DATA (HEX)
	// ==========================================
	char hexString[128] = {0}; // Mảng chứa chuỗi in ra (đủ chứa khoảng 40 byte hex)
	int offset = 0;

	// Giới hạn số lượng byte in ra để tránh tràn mảng hexString nếu gói quá dài
	uint8_t print_len = (len > 40) ? 40 : len;

	// Lặp qua payload (bắt đầu từ index 1 đến len)
	for(uint8_t i = 0; i <= print_len; i++) {
		// %02X giúp in ra số HEX in hoa, có số 0 ở trước nếu < 10 (ví dụ: 0A, 0B)
		offset += sprintf(hexString + offset, "%02X ", uart2RxBuffer[i]);
	}

	// In ra tổng số byte nhận được và nội dung chuỗi hex
	LOG_WARN("UART2 RX (Len: %d) RAW: %s", len, hexString);
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
    LOG_DEBUG("uart 3");
}
