#ifndef UART_H
#define UART_H

#include <main.h>
#include <stdint.h>
#include <string.h>
#include <logger.h>
// USER SETTING

#define RX_MAX_BUFFER_SIZE 100
#define UART_TX_AVAILABLE_FLAG 1
#define UART_TX_UN_AVAILABLE_FLAG 0

// END USER SETTING
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern UART_HandleTypeDef huart2;

extern uint8_t uart1RxBuffer[RX_MAX_BUFFER_SIZE+1];
extern uint8_t uart1RxFlag;
extern uint8_t uart1TxFlag;

void uart1ProcessWhenIdleFlag();
void uartManualInit();

#endif
