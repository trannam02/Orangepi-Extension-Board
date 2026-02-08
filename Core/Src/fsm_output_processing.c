#include "fsm_output_processing.h"

// repository for Orange PI
uint8_t o_UplinkQueue[O_UPLINK_MAX_QUEUE_SIZE][O_UPLINK_TX_MAX_BUFFER_SIZE + 1]; // plus 1 for '\0'
uint8_t o_UplinkQueueIndex = 0;
uint8_t o_UplinkQueueNumEl = 0;

// repository for RS485
//uint8_t o_RS485Queue[O_RS485_TX_MAX_BUFFER_SIZE + 1][O_RS485_MAX_QUEUE_SIZE]; // plus 1 for '\0'
uint8_t o_RS485Queue[O_RS485_MAX_QUEUE_SIZE][O_RS485_TX_MAX_BUFFER_SIZE + 1];
uint8_t o_RS485QueueIndex = 0; // index la vi tri phan tu cuoi cung
uint8_t o_RS485QueueNumEl = 0;

// repository for LED interface
uint8_t outputLedType = 0;

static uint8_t state = STATE_OUTPUT_INIT;

void outputLed();

void output_processing_init(){
	HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
	setTimer(3, 1000);
};

void output_processing_run(){
	// output data processing
	switch(state){
		case STATE_OUTPUT_INIT:
		{
			state = STATE_OUTPUT_WAITING;
			break;
		}
		case STATE_OUTPUT_WAITING:
		{
			if(o_RS485QueueNumEl > 0){
				state = STATE_OUTPUT_PROCESSING_RS485;
			}
//			if(){ // buffer knx co data
//
//				}
//			if(){ // buffer uplink co data
//
//				}
			break;
		}
		case STATE_OUTPUT_PROCESSING_RS485: // process buffer uart co data
		{
			// send dma


			state = STATE_OUTPUT_WAITING;
			break;
		}
		case STATE_OUTPUT_PROCESSING_KNX: // process buffer knx co data
		{
			state = STATE_OUTPUT_WAITING;
			break;
		}
		case STATE_OUTPUT_PROCESSING_UPLINK: // process buffer uplink co data
		{
			state = STATE_OUTPUT_WAITING;
			break;
		}
		default:
		{
			break;
		};

	};
	// output rs485 processing
	if(getTimer(3)){
		uint8_t pos = (o_RS485QueueIndex + O_RS485_MAX_QUEUE_SIZE - o_RS485QueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
		if(o_RS485QueueNumEl > 0){
			// Lấy độ dài trước cho gọn
			uint16_t len = strlen((char*)o_RS485Queue[pos]);

			// GỌI HÀM VÀ KIỂM TRA KẾT QUẢ
			LOG_INFO("DMA Transmit: data=%s len=%d pos=%d\n", o_RS485Queue[pos], len, pos);


			LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_RS485QueueNumEl, o_RS485QueueIndex);
			int tail = (o_RS485QueueIndex + O_RS485_MAX_QUEUE_SIZE - o_RS485QueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
			for (int i = 0; i < o_RS485QueueNumEl; i++) {
				int current_pos = (tail + i) % O_RS485_MAX_QUEUE_SIZE;
				LOG_DEBUG("[%d] %s", current_pos, o_RS485Queue[current_pos]);
			}
			LOG_DEBUG("---------------------------------------------");


			HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart1, o_RS485Queue[pos], len);

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
				// HAL_ERROR: Có lỗi xảy ra (ví dụ bộ DMA chưa được Init)
				LOG_ERROR("UART Transmit Error: %d", status);
				// Có thể cần xử lý lỗi ở đây (ví dụ reset queue nếu cần)
			}
			LOG_INFO("DMA after transmit: data=%s len=%d o_RS485QueueNumEl=%d o_RS485QueueIndex=%d\n", o_RS485Queue[pos], len, o_RS485QueueNumEl, o_RS485QueueIndex);
		}

		clearTimer(3);
		setTimer(3, 1000);
	}


	// output led processing
	outputLed();

	// output buzzer processing

};

void outputLed(){
	switch(outputLedType){
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
