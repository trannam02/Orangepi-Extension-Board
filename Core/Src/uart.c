#include <uart.h>

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

uint8_t dmaUart1RxBuffer[RX_MAX_BUFFER_SIZE + 1]; // plus 1 for \0
uint8_t uart1RxBuffer[RX_MAX_BUFFER_SIZE+1];
uint8_t uart1RxFlag = 0;

void uartManualInit(){
	HAL_UART_Receive_DMA(&huart1, dmaUart1RxBuffer, RX_MAX_BUFFER_SIZE);
	// uart 2, 3
};

void uart1ProcessWhenIdleFlag(){
	__HAL_UART_CLEAR_IDLEFLAG(&huart1);
	HAL_UART_DMAStop(&huart1);
	uint16_t remain = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
	uint16_t rxLength = RX_MAX_BUFFER_SIZE - remain;
	if(rxLength > 0){
		memcpy(uart1RxBuffer, dmaUart1RxBuffer, rxLength);
		uart1RxBuffer[rxLength] = '\0';
		uart1RxFlag = 1;
	};
	HAL_UART_Receive_DMA(&huart1, dmaUart1RxBuffer, RX_MAX_BUFFER_SIZE);
;};
