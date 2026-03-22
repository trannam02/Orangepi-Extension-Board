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
static uint8_t state = STATE_INPUT_INIT;


#define KNX_RX_TIMEOUT MS(50)
#define KNX_MAX_ACC_SIZE 64

static uint8_t knx_rx_acc[KNX_MAX_ACC_SIZE]; // Bộ đệm gom toàn bộ gói tin
static uint16_t knx_rx_acc_len = 0;          // Chiều dài hiện tại của bộ đệm gom
static uint8_t is_receiving = 0;             // Cờ đánh dấu đang trong quá trình gom mảng
static uint16_t old_ptr = 0;


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

int knx_read(uint8_t *out_data, int max_len)
{
	switch(KNX_READ_STATE){

		case 1: // waitting trigger
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
		case 2:
		{
			if(getTimer(4)){
				uint16_t new_ptr = RX_MAX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);
				int length = 0;

				// 5. Bốc dữ liệu từ old_ptr đến new_ptr
				// Dùng vòng lặp while để xử lý dễ dàng trường hợp wrap-around (con trỏ vòng về 0)
				while (old_ptr != new_ptr && length < max_len)
				{
					out_data[length++] = dmaUart3RxBuffer[old_ptr];
					old_ptr = (old_ptr + 1) % RX_MAX_BUFFER_SIZE;
				}
			}

			break;
		}
		default:
			break;
	}
}

void input_processing_run() {
	if(getTimer(4)){
		if(is_receiving){
			// goi tin ket thuc

			knx_rx_acc[0] = knx_rx_acc_len - 1;
			if (!Queue_Push(&i_KNXQueue, knx_rx_acc, knx_rx_acc[0] + 1)) {
				LOG_INFO("KNX Input queue overflow, drop package!");
			}

			// ==========================================
			// ĐOẠN CODE MỚI THÊM ĐỂ IN RAW DATA (HEX)
			// ==========================================
			uint8_t len = knx_rx_acc_len - 1;
			char hexString[128] = {0}; // Mảng chứa chuỗi in ra (đủ chứa khoảng 40 byte hex)
			int offset = 0;

			// Giới hạn số lượng byte in ra để tránh tràn mảng hexString nếu gói quá dài
			uint8_t print_len = (len > 40) ? 40 : len;

			// Lặp qua payload (bắt đầu từ index 1 đến len)
			for(uint8_t i = 0; i <= print_len; i++) {
				// %02X giúp in ra số HEX in hoa, có số 0 ở trước nếu < 10 (ví dụ: 0A, 0B)
				offset += sprintf(hexString + offset, "%02X ", knx_rx_acc[i]);
			}

			// In ra tổng số byte nhận được và nội dung chuỗi hex
			LOG_WARN("KNX receive and push (Len: %d): %s", len, hexString);
				// ==========================================

			is_receiving = 0;
			knx_rx_acc_len = 0;



		}
	}
    switch(state) {
        case STATE_INPUT_INIT:
            input_processing_init();
            break;

        case STATE_INPUT_WAITTING:
            // 1. Nhận từ Orange Pi (UART2)
            if (uart2RxFlag == 1) {
                uart2RxFlag = 0;
                // Code cũ của bạn: byte 0 là chiều dài
                if (!Queue_Push(&i_ORPQueue, uart2RxBuffer, uart2RxBuffer[0] + 1)) {
                    LOG_INFO("ORP Input queue overflow, drop package!");
                }
            }

            // 2. Nhận từ RS485 (UART1)
            if (uart1RxFlag == 1) {
                uart1RxFlag = 0;
                // CHÚ Ý LỚN: Code cũ của bạn dùng uart1RxBuffer[1] làm chiều dài!
                if (!Queue_Push(&i_rs485Queue, uart1RxBuffer, uart1RxBuffer[0] + 1)) {
                    LOG_INFO("RS485 Input queue overflow, drop package!");
                }
            }

            // 3. Nhận từ KNX (UART3)
            if (uart3RxFlag == 1) {

            	uart3RxFlag = 0;
				uint8_t fragment_len = uart3RxBuffer[0];
				// Kiểm tra tránh tràn mảng
				if (fragment_len > 0 && (knx_rx_acc_len + fragment_len < KNX_MAX_ACC_SIZE)) {
					// Copy mảnh vỡ (từ index 1) vào vị trí tiếp theo của mảng gom
					if(knx_rx_acc_len == 0){
						memcpy(&knx_rx_acc[1], &uart3RxBuffer[1], fragment_len);
						knx_rx_acc_len = 1;
					}else{
						memcpy(&knx_rx_acc[knx_rx_acc_len], &uart3RxBuffer[1], fragment_len);
					}

					knx_rx_acc_len += fragment_len;

					clearTimer(4);
					setTimer(4, KNX_RX_TIMEOUT);
					is_receiving = 1;
				}
                // CHÚ Ý LỚN: Code cũ của bạn dùng uart3RxBuffer[1] làm chiều dài!
//                if (!Queue_Push(&i_KNXQueue, uart3RxBuffer, uart3RxBuffer[0] + 1)) {
//                    LOG_INFO("KNX Input queue overflow, drop package!");
//                }
            }

            // 4. Nhận nút nhấn
            if (getButtonPressFlag(0)) {
                setButtonPressFlag(0, 0);
                i_inputBtn1PressFlag = 1;
                LOG_DEBUG("Input Button Pressed");
            }
            break;

        default:
            state = STATE_INPUT_WAITTING;
            break;
    }
}
