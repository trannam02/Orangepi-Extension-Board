#include <uart.h>

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

//	HAL_UART_Receive_DMA(&huart2, dmaUart2RxBuffer, RX_MAX_BUFFER_SIZE);
//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart3, dmaUart3RxBuffer, RX_MAX_BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
};

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//}

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
        memcpy(uart1RxBuffer, &dmaUart1RxBuffer[oldPos1], len);
    }else {
        uint16_t len1 = RX_MAX_BUFFER_SIZE - oldPos1;
        uint16_t len2 = newPos;

        len = len1 + len2;

        memcpy(uart1RxBuffer, &dmaUart1RxBuffer[oldPos1], len1);
        memcpy(&uart1RxBuffer[len1], &dmaUart1RxBuffer[0], len2);
    }

    uart1RxBuffer[len] = '\0';
    uart1RxFlag = 1;

    oldPos1 = newPos;

    LOG_DEBUG("HEHE");
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
        memcpy(uart2RxBuffer, &dmaUart2RxBuffer[oldPos2], len);
    }else {
        uint16_t len1 = RX_MAX_BUFFER_SIZE - oldPos2;
        uint16_t len2 = newPos;

        len = len1 + len2;

        memcpy(uart2RxBuffer, &dmaUart2RxBuffer[oldPos2], len1);
        memcpy(&uart2RxBuffer[len1], &dmaUart2RxBuffer[0], len2);
    }

    uart2RxBuffer[len] = '\0';
    uart2RxFlag = 1;

    oldPos2 = newPos;
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
        memcpy(uart3RxBuffer, &dmaUart3RxBuffer[oldPos3], len);
    }else {
        uint16_t len1 = RX_MAX_BUFFER_SIZE - oldPos3;
        uint16_t len2 = newPos;

        len = len1 + len2;

        memcpy(uart3RxBuffer, &dmaUart3RxBuffer[oldPos3], len1);
        memcpy(&uart3RxBuffer[len1], &dmaUart3RxBuffer[0], len2);
    }

    uart3RxBuffer[len] = '\0';
    uart3RxFlag = 1;

    oldPos3 = newPos;
}
