#include <uart.h>

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

UART_HandleTypeDef huart2;

static uint16_t oldPos = 0;

uint8_t dmaUart1RxBuffer[RX_MAX_BUFFER_SIZE + 1]; // plus 1 for \0
uint8_t uart1RxBuffer[RX_MAX_BUFFER_SIZE+1];
uint8_t uart1RxFlag = 0;

uint8_t uart1TxFlag = UART_TX_AVAILABLE_FLAG; // available
void uartManualInit(){
	HAL_UART_Receive_DMA(&huart1, dmaUart1RxBuffer, RX_MAX_BUFFER_SIZE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	// uart 2, 3
};

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        // Code xử lý khi USART1 gửi xong
    	uart1TxFlag = UART_TX_AVAILABLE_FLAG;
    }
}

/* Callback khi nhận xong */
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if(huart->Instance == USART1)
//    {
//        // Code xử lý khi USART1 nhận xong
//    }
//}

//void uart1ProcessWhenIdleFlag(){
//	__HAL_UART_CLEAR_IDLEFLAG(&huart1);
//	HAL_UART_DMAStop(&huart1);
//	uint16_t remain = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
//	uint16_t rxLength = RX_MAX_BUFFER_SIZE - remain;
//	LOG_DEBUG("IDLE FLAG: rxLength=%d", rxLength);
//	if(rxLength > 0){
//		memcpy(uart1RxBuffer, dmaUart1RxBuffer, rxLength);
//		uart1RxBuffer[rxLength] = '\0';
//		uart1RxFlag = 1;
//	};
//	HAL_UART_Receive_DMA(&huart1, dmaUart1RxBuffer, RX_MAX_BUFFER_SIZE);
//	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
//;};

void uart1ProcessWhenIdleFlag(){
	__HAL_UART_CLEAR_IDLEFLAG(&huart1);

    uint16_t remaining = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
    uint16_t newPos = RX_MAX_BUFFER_SIZE - remaining;

    if (newPos == oldPos) {
        return;
    }

    uint16_t len = 0;

    if (newPos > oldPos) {
        len = newPos - oldPos;
        memcpy(uart1RxBuffer, &dmaUart1RxBuffer[oldPos], len);
    }else {
        uint16_t len1 = RX_MAX_BUFFER_SIZE - oldPos;
        uint16_t len2 = newPos;

        len = len1 + len2;

        memcpy(uart1RxBuffer, &dmaUart1RxBuffer[oldPos], len1);
        memcpy(&uart1RxBuffer[len1], &dmaUart1RxBuffer[0], len2);
    }

    uart1RxBuffer[len] = '\0';
    uart1RxFlag = 1;

    oldPos = newPos;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    // Kiểm tra xem lỗi có phải của UART1 không
    if(huart->Instance == USART1)
    {
        // 1. QUAN TRỌNG NHẤT: Mở khóa cờ TX
        // Dù lỗi gì xảy ra, cũng phải cho phép lần gửi tiếp theo được chạy
    	LOG_DEBUG("HEHEHEHEHEHEHEHEHEHEHEHE");
        uart1TxFlag = UART_TX_AVAILABLE_FLAG;

        // 2. Debug nguyên nhân lỗi (nếu cần)
        // Mã lỗi nằm trong huart->ErrorCode:
        // HAL_UART_ERROR_ORE (Overrun) = 8
        // HAL_UART_ERROR_NE (Noise) = 4
        // HAL_UART_ERROR_FE (Frame) = 2
        // LOG_ERROR("UART Error: %d", huart->ErrorCode);

        // 3. Khôi phục lại việc nhận (RX)
        // Khi lỗi xảy ra, HAL thường tắt luôn DMA RX, cần bật lại
        // Lưu ý: Nếu đang dùng Circular DMA, có thể cần Abort trước rồi bật lại hoặc chỉ cần xóa cờ
        // Cách an toàn nhất cho Circular là không làm gì nếu chỉ lỗi nhẹ,
        // nhưng với lỗi ORE (Overrun) trên F1, cần reset lại quy trình nhận:

        /* Reset RX nếu cần thiết (Tùy chọn, test kỹ phần này) */
        // HAL_UART_DMAStop(huart);
        // MX_DMA_Init(); // Đôi khi cần init lại DMA nếu nó bị lock
        // HAL_UART_Receive_DMA(huart, dmaUart1RxBuffer, RX_MAX_BUFFER_SIZE);
        // __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    }
}
