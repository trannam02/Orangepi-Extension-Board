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

void input_processing_run() {
    switch(state) {
        case STATE_INPUT_INIT:
            input_processing_init();
            break;

        case STATE_INPUT_WAITTING:
            // 1. Nhận từ Orange Pi (UART2)
            if (uart2RxFlag == 1) {
                uart2RxFlag = 0;
                // Code cũ của bạn: byte 0 là chiều dài
                if (!Queue_Push(&i_ORPQueue, uart2RxBuffer, uart2RxBuffer[0])) {
                    LOG_INFO("ORP Input queue overflow, drop package!");
                }
            }

            // 2. Nhận từ RS485 (UART1)
            if (uart1RxFlag == 1) {
                uart1RxFlag = 0;
                // CHÚ Ý LỚN: Code cũ của bạn dùng uart1RxBuffer[1] làm chiều dài!
                if (!Queue_Push(&i_rs485Queue, uart1RxBuffer, uart1RxBuffer[1])) {
                    LOG_INFO("RS485 Input queue overflow, drop package!");
                }
            }

            // 3. Nhận từ KNX (UART3)
            if (uart3RxFlag == 1) {
                uart3RxFlag = 0;
                // CHÚ Ý LỚN: Code cũ của bạn dùng uart3RxBuffer[1] làm chiều dài!
                if (!Queue_Push(&i_KNXQueue, uart3RxBuffer, uart3RxBuffer[1])) {
                    LOG_INFO("KNX Input queue overflow, drop package!");
                }
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
