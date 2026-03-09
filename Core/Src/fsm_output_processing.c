#include "fsm_output_processing.h"

// repository for Orange PI
uint8_t o_UPLINKQueue[O_UPLINK_MAX_QUEUE_SIZE][O_UPLINK_TX_MAX_BUFFER_SIZE + 1]; // plus 1 for '\0'
uint8_t o_UPLINKQueueIndex = 0;
uint8_t o_UPLINKQueueNumEl = 0;

// repository for RS485
//uint8_t o_RS485Queue[O_RS485_TX_MAX_BUFFER_SIZE + 1][O_RS485_MAX_QUEUE_SIZE]; // plus 1 for '\0'
uint8_t o_RS485Queue[O_RS485_MAX_QUEUE_SIZE][O_RS485_TX_MAX_BUFFER_SIZE + 1];
uint8_t o_RS485QueueIndex = 0; // index la vi tri phan tu cuoi cung
uint8_t o_RS485QueueNumEl = 0;

// repository for KNX
uint8_t o_KNXQueue[O_KNX_MAX_QUEUE_SIZE][O_KNX_TX_MAX_BUFFER_SIZE + 1];
uint8_t o_KNXQueueIndex = 0; // index la vi tri phan tu cuoi cung
uint8_t o_KNXQueueNumEl = 0;

// repository for LED interface
uint8_t o_outputLedType = 0;

// Function prototype
void outputLed();
void outputRS485();
void outputKNX();
void outputUPLINK();

void output_processing_init(){
	HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
};

void output_processing_run(){
	// output data processing

	for(int i = 0; i < CONCURENCY_RATE; i++){
		// output knx processing
		outputKNX();

		// output RS485 processing
		outputRS485();

		outputUPLINK();
	};


	// output led processing
	outputLed();

	// output buzzer processing

};

void outputUPLINK(){
	uint8_t pos = (o_UPLINKQueueIndex + O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl) % O_UPLINK_MAX_QUEUE_SIZE;
	if(o_UPLINKQueueNumEl > 0){
		// Lấy độ dài trước cho gọn
		uint16_t len = o_UPLINKQueue[pos][0];
		if(len <= 0){
			o_UPLINKQueueNumEl = o_UPLINKQueueNumEl - 1;
			LOG_DEBUG("Warning: Package len=%d - skipped", len);
			return;
		}
		// GỌI HÀM VÀ KIỂM TRA KẾT QUẢ
		LOG_INFO("DMA Transmit: data=%s len=%d pos=%d\n", o_UPLINKQueue[pos], len, pos);


		LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
		int tail = (o_UPLINKQueueIndex + O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl) % O_UPLINK_MAX_QUEUE_SIZE;
		for (int i = 0; i < o_UPLINKQueueNumEl; i++) {
			int current_pos = (tail + i) % O_UPLINK_MAX_QUEUE_SIZE;
			LOG_DEBUG("[%d] %s", current_pos, o_UPLINKQueue[current_pos]);
		}
		LOG_DEBUG("---------------------------------------------");


		HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart2, &o_UPLINKQueue[pos][1], len);

		if (status == HAL_OK) {
			// Chỉ khóa cờ khi DMA thực sự đã bắt đầu chạy
			LOG_DEBUG("UART 1 Tx STARTED successfully");

			// Decrement count (chỉ trừ khi gửi thành công)
			o_UPLINKQueueNumEl = o_UPLINKQueueNumEl - 1;
		}
		else if (status == HAL_BUSY)
		{
			 LOG_DEBUG("UART Busy, retry later...");
		}
		else
		{
			LOG_ERROR("UART Transmit Error: %d", status);
			// Có thể cần xử lý lỗi ở đây (ví dụ reset queue nếu cần)
		}
		LOG_INFO("DMA after transmit: data=%s len=%d o_UPLINKQueueNumEl=%d o_UPLINKQueueIndex=%d\n", o_RS485Queue[pos], len, o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
	}
};

void outputKNX(){
	uint8_t pos = (o_KNXQueueIndex + O_KNX_MAX_QUEUE_SIZE - o_KNXQueueNumEl) % O_KNX_MAX_QUEUE_SIZE;
	if(o_KNXQueueNumEl > 0){
		// Lấy độ dài trước cho gọn
		uint16_t len = o_KNXQueue[pos][0];

		// GỌI HÀM VÀ KIỂM TRA KẾT QUẢ
		LOG_INFO("DMA Transmit: data=%s len=%d pos=%d\n", o_KNXQueue[pos], len, pos);


		LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_KNXQueueNumEl, o_KNXQueueIndex);
		int tail = (o_KNXQueueIndex + O_KNX_MAX_QUEUE_SIZE - o_KNXQueueNumEl) % O_KNX_MAX_QUEUE_SIZE;
		for (int i = 0; i < o_KNXQueueNumEl; i++) {
			int current_pos = (tail + i) % O_KNX_MAX_QUEUE_SIZE;
			LOG_DEBUG("[%d] %s", current_pos, o_KNXQueue[current_pos]);
		}
		LOG_DEBUG("---------------------------------------------");


		HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart3, &o_KNXQueue[pos][1], len);

		if (status == HAL_OK) {
			// Chỉ khóa cờ khi DMA thực sự đã bắt đầu chạy
			LOG_DEBUG("UART 1 Tx STARTED successfully");

			// Decrement count (chỉ trừ khi gửi thành công)
			o_KNXQueueNumEl = o_KNXQueueNumEl - 1;
		}
		else if (status == HAL_BUSY)
		{
			 LOG_DEBUG("UART Busy, retry later...");
		}
		else
		{
			LOG_ERROR("UART Transmit Error: %d", status);
			// Có thể cần xử lý lỗi ở đây (ví dụ reset queue nếu cần)
		}
		LOG_INFO("DMA after transmit: data=%s len=%d o_KNXQueueNumEl=%d o_KNXQueueIndex=%d\n", o_RS485Queue[pos], len, o_KNXQueueNumEl, o_KNXQueueIndex);
	}
};

void outputRS485(){
	uint8_t pos = (o_RS485QueueIndex + O_RS485_MAX_QUEUE_SIZE - o_RS485QueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
	if(o_RS485QueueNumEl > 0){
		// Lấy độ dài trước cho gọn
		uint16_t len = o_RS485Queue[pos][0];

		// GỌI HÀM VÀ KIỂM TRA KẾT QUẢ
		LOG_INFO("DMA Transmit: data=%s len=%d pos=%d\n", o_RS485Queue[pos], len, pos);


		LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_RS485QueueNumEl, o_RS485QueueIndex);
		int tail = (o_RS485QueueIndex + O_RS485_MAX_QUEUE_SIZE - o_RS485QueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
		for (int i = 0; i < o_RS485QueueNumEl; i++) {
			int current_pos = (tail + i) % O_RS485_MAX_QUEUE_SIZE;
			LOG_DEBUG("[%d] %s", current_pos, o_RS485Queue[current_pos]);
		}
		LOG_DEBUG("---------------------------------------------");


		HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart1, &o_RS485Queue[pos][1], len);

		if (status == HAL_OK) {
			// Chỉ khóa cờ khi DMA thực sự đã bắt đầu chạy
			LOG_DEBUG("UART 1 Tx STARTED successfully");
			uart1TxFlag = UART_TX_UN_AVAILABLE_FLAG;

			// Decrement count (chỉ trừ khi gửi thành công)
			o_RS485QueueNumEl = o_RS485QueueNumEl - 1;
		}
		else if (status == HAL_BUSY)
		{
			 LOG_DEBUG("UART Busy, retry later...");
		}
		else
		{
			LOG_ERROR("UART Transmit Error: %d", status);
			// Có thể cần xử lý lỗi ở đây (ví dụ reset queue nếu cần)
		}
		LOG_INFO("DMA after transmit: data=%s len=%d o_RS485QueueNumEl=%d o_RS485QueueIndex=%d\n", o_RS485Queue[pos], len, o_RS485QueueNumEl, o_RS485QueueIndex);
	}
};

void outputLed(){
	switch(o_outputLedType){
		case LED_CODE_OFF:
		{
			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
			break;
		}
		case LED_CODE_ON:
		{
			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
			break;
		}
		case LED_CODE_BLINK_1HZ:
		{
			if(getTimer(1)){
				HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
				clearTimer(1);
				setTimer(1,500);
			}
			break;
		}
		case LED_CODE_BLINK_5HZ:
		{
			if(getTimer(1)){
				HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
				clearTimer(1);
				setTimer(1, 200);
			}
			break;
		}
		default:
		{
			break;
		};

	}
};
