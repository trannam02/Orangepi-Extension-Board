#ifndef UART_H
#define UART_H

#include <main.h>
#include <stdint.h>
#include <string.h>
// USER SETTING

#define RX_MAX_BUFFER_SIZE 255

// END USER SETTING
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

extern uint8_t uart1RxBuffer[RX_MAX_BUFFER_SIZE+1];
extern uint8_t uart1RxFlag;

void uart1ProcessWhenIdleFlag();
void uartManualInit();

#endif
