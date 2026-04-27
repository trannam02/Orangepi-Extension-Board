#include "fsm_input_processing.h"
#include "queue_utils.h" // Thư viện Ring Buffer

// --- ĐỊNH NGHĨA VÀ CẤP PHÁT BỘ NHỚ CHO CÁC QUEUE (GLOBAL) ---
MessageQueue_t i_ORPQueue;
MessageQueue_t i_rs485Queue;
MessageQueue_t i_KNXQueue;
MessageQueue_t o_UPLINKQueue;
MessageQueue_t o_RS485Queue;
MessageQueue_t o_KNXQueue;

uint8_t i_inputBtn1PressFlag = 0;
uint8_t i_inputBtn1LongPressFlag = 0;
uint8_t i_inputBtn2PressFlag = 0;
uint8_t i_inputBtn2LongPressFlag = 0;

static uint8_t state = STATE_INPUT_INIT;


#define KNX_RX_TIMEOUT MS(50)
#define KNX_MAX_ACC_SIZE 64

static uint8_t knx_rx_acc[KNX_MAX_ACC_SIZE]; // Bộ đệm gom toàn bộ gói tin
static uint16_t knx_rx_acc_len = 0;          // Chiều dài hiện tại của bộ đệm gom
static uint8_t is_receiving = 0;             // Cờ đánh dấu đang trong quá trình gom mảng
static uint16_t old_ptr = 0;

static uint8_t out_data[KNX_MAX_ACC_SIZE];
uint8_t KNX_READ_STATE = 1;
uint8_t length = 0;

void input_processing_init() {
    // Khởi tạo các hàng đợi
    Queue_Init(&i_ORPQueue, I_ORP_MAX_QUEUE_SIZE);
    Queue_Init(&i_rs485Queue, RS485_MAX_QUEUE);
    Queue_Init(&i_KNXQueue, I_KNX_MAX_QUEUE_SIZE);
    Queue_Init(&o_UPLINKQueue, O_UPLINK_MAX_QUEUE_SIZE);
    Queue_Init(&o_RS485Queue, O_RS485_MAX_QUEUE_SIZE);
    Queue_Init(&o_KNXQueue, O_KNX_MAX_QUEUE_SIZE);
    state = STATE_INPUT_WAITTING;
}

void knx_read()
{
	switch(KNX_READ_STATE){

		case 1: // waitting first byte trigger
		{
			uint16_t current_ptr = RX_MAX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);
			if (current_ptr != old_ptr)
			{
				clearTimer(4);
				setTimer(4, KNX_RX_TIMEOUT);
				KNX_READ_STATE = 2;
			}
			break;
		}
		case 2: //
		{
			if(getTimer(4)){
				uint16_t new_ptr = RX_MAX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);
				length = 1;
				while (old_ptr != new_ptr && length < KNX_MAX_ACC_SIZE)
				{
					out_data[length++] = dmaUart3RxBuffer[old_ptr];
					old_ptr = (old_ptr + 1) % RX_MAX_BUFFER_SIZE;
				}
				out_data[0] = length - 1;
				uart3RxFlag = 1;
				KNX_READ_STATE = 1;
			}

			break;
		}
		default:
			break;
	}
}

void input_processing_run() {

	knx_read();

    switch(state) {
        case STATE_INPUT_INIT:
            input_processing_init();
            state = STATE_INPUT_WAITTING;
            break;

        case STATE_INPUT_WAITTING:
            // 1. Orange Pi (UART2)
        	if(system_state == SYSTEM_STATE_RUNNING){

				// 2. RS485 (UART1)
				if (uart1RxFlag == 1) {
					uart1RxFlag = 0;
					if (!Queue_Push(&i_rs485Queue, uart1RxBuffer, uart1RxBuffer[0] + 1)) {
						LOG_INFO("RS485 Input queue overflow, drop package!");
					}
				}

				// 3. KNX (UART3)
				if (uart3RxFlag == 1) {
					uart3RxFlag = 0;
					if (!Queue_Push(&i_KNXQueue, out_data, out_data[0] + 1)) {
						LOG_INFO("KNX Input queue overflow, drop package!");
					}
				}
        	}

        	// che do nhan config thi duong nay van chay
        	if (uart2RxFlag == 1) {
				uart2RxFlag = 0;
				if (!Queue_Push(&i_ORPQueue, uart2RxBuffer, uart2RxBuffer[0] + 1)) {
					LOG_INFO("ORP Input queue overflow, drop package!");
				}
			}

            // 4. Button
            if (getButtonPressFlag(0)) {
                setButtonPressFlag(0, 0);
                i_inputBtn1PressFlag = 1;
                LOG_DEBUG("Input Button 1 Pressed");
            }
//            if (getButtonPressFlag(1)) {
//				setButtonPressFlag(1, 0);
//				i_inputBtn2PressFlag = 1;
//				LOG_DEBUG("Input Button 1 Long Pressed");
//			}
            if (getButtonLongPressFlag(0)) {
				setButtonLongPressFlag(0, 0);
				i_inputBtn1LongPressFlag = 1;
				LOG_DEBUG("Input Button 1 Long Pressed");
			}
//			if (getButtonLongPressFlag(1)) {
//				setButtonLongPressFlag(1, 0);
//				i_inputBtn2LongPressFlag = 1;
//				LOG_DEBUG("Input Button 2 Long Pressed");
//			}
            break;

        default:
            state = STATE_INPUT_WAITTING;
            break;
    }
}
