#include "fsm_main_processing.h"
#include "fsm_input_processing.h"
#include "queue_utils.h"

// Khai báo các Queue (External từ các file khác hoặc định nghĩa ở đây)
extern MessageQueue_t i_ORPQueue;
extern MessageQueue_t i_rs485Queue;
extern MessageQueue_t i_KNXQueue;
extern MessageQueue_t o_RS485Queue;
extern MessageQueue_t o_KNXQueue;
extern MessageQueue_t o_UPLINKQueue;

static uint8_t state = STATE_INIT;

static uint8_t couplerX = 0;
static uint8_t flag_waiting_response[NUMBER_COUPLER];
static uint8_t counter = 0;

void poll_interval_10ms() {
    if (getTimer(2) == 1) {
        clearTimer(2);

        uint8_t CMD_POLL[6] = {0x06, 0x05, couplerX, 0x00, 0x00, 0x20}; // Lưu ý byte [0] là length
        // Push thẳng vào Output Queue RS485
        Queue_Push(&o_RS485Queue, CMD_POLL, CMD_POLL[0]);

        setTimer(3, TEMP_TIMER_3); // Bắt đầu timeout
    }

    if (getTimer(3) == 1) {
        clearTimer(3);
        counter++;

        if (counter >= MAX_RETRY_POLLING) {
            counter = 0;
            couplerX = (couplerX + 1) % NUMBER_COUPLER;
            setTimer(2, TEMP_TIMER_2); // Trở lại vòng lặp poll thiết bị tiếp theo
            return;
        }

        // Retry: Gửi lại
        uint8_t CMD_POLL[6] = {0x06, 0x05, couplerX, 0x00, 0x00, 0x20};
        Queue_Push(&o_RS485Queue, CMD_POLL, CMD_POLL[0]);
        setTimer(3, TEMP_TIMER_3);
    }
}

void main_processing_init() {
    state = STATE_INIT;
    for (uint8_t i = 0; i < NUMBER_COUPLER; i++) {
        flag_waiting_response[i] = 0;
    }
    setTimer(2, TEMP_TIMER_2);
}

void main_processing_run() {
    uint8_t rxData[MAX_BUFFER_LEN];
    uint8_t txData[MAX_BUFFER_LEN];
    uint8_t processed = 0;

    switch (state) {
        case STATE_INIT:
            state = STATE_WAITTING;
            break;

        case STATE_WAITTING:
            if (i_ORPQueue.count > 0) state = STATE_PROCESS_DOWNLINK;
            else if (i_rs485Queue.count > 0) state = STATE_RECEIVE_RS485_FRAME;
            else if (i_KNXQueue.count > 0) state = STATE_RECEIVE_KNX_FRAME;
            else if (i_inputBtn1PressFlag) {
                i_inputBtn1PressFlag = 0;
                state = STATE_BTN_PRESS_5S;
            }
            break;

        case STATE_PROCESS_DOWNLINK:
            // Lấy từ OrangePi đẩy xuống RS485 hoặc KNX
            while (processed < CONCURENCY_RATE && Queue_Pop(&i_ORPQueue, rxData)) {
                // Giả định Format rxData: [Length][Header][Payload...][CRC]
                uint8_t total_len = rxData[0];
                uint8_t header = rxData[1];
                uint8_t payload_size = total_len - 2; // Bỏ length và header (CHÚ Ý 1)

                if (header == HEADER_RS485) {
                    txData[0] = payload_size + 1; // Length mới = payload + 1 byte CRC
                    memcpy(&txData[1], &rxData[2], payload_size);
                    txData[1 + payload_size] = crc8(&txData[1], payload_size);

                    Queue_Push(&o_RS485Queue, txData, txData[0] + 1); // +1 chứa byte length
                    LOG_DEBUG("Downlink routed to RS485");

                } else if (header == HEADER_KNX) {
                    txData[0] = payload_size + 1;
                    memcpy(&txData[1], &rxData[2], payload_size);
                    txData[1 + payload_size] = crc8(&txData[1], payload_size);

                    Queue_Push(&o_KNXQueue, txData, txData[0] + 1);
                    LOG_DEBUG("Downlink routed to KNX");
                }
                processed++;
            }
            state = STATE_WAITTING;
            break;

        case STATE_RECEIVE_RS485_FRAME:
            // Lấy từ RS485 đẩy lên OrangePi
            while (processed < CONCURENCY_RATE && Queue_Pop(&i_rs485Queue, rxData)) {
                // Kiểm tra ACK hoặc Response
//                uint8_t ACK_PACKAGE[6] = {0x06, 0x05, 0x01, 0x00, 0x03, 0x60}; // Sửa byte[0] thành 0x06 để so khớp độ dài
//                if (memcmp(rxData, ACK_PACKAGE, 6) == 0) {
//                    LOG_INFO("ACK RECEIVED");
//                    counter = 0;
//                    clearTimer(3);
//                    setTimer(2, TEMP_TIMER_2);
//                    continue; // Xử lý xong ACK, không đẩy lên OrangePi
//                } else if (rxData[2] == couplerX) {
//                    LOG_INFO("RESPONSE RECEIVED");
//                    counter = 0;
//                    clearTimer(3);
//                }

                // Format up lên Orange Pi: [Length][Header][Payload...][CRC]
                uint8_t payload_size = rxData[0] - 1; // rxData[0] chứa length gói nhận, trừ 1 byte old CRC

                txData[0] = payload_size + 2; // Length = payload + Header + CRC
                txData[1] = HEADER_RS485;
                memcpy(&txData[2], &rxData[1], payload_size);
                txData[2 + payload_size] = crc8(&txData[1], payload_size + 1); // Tính CRC cho Header + Payload

                Queue_Push(&o_UPLINKQueue, txData, txData[0] + 1);
                processed++;
            }
            state = STATE_WAITTING;
            break;

        case STATE_RECEIVE_KNX_FRAME:
            // Lấy từ KNX đẩy lên OrangePi
            while (processed < CONCURENCY_RATE && Queue_Pop(&i_KNXQueue, rxData)) {
                uint8_t payload_size = rxData[0] - 1;

                txData[0] = payload_size + 2;
                txData[1] = HEADER_KNX;
                memcpy(&txData[2], &rxData[1], payload_size);
                txData[2 + payload_size] = crc8(&txData[1], payload_size + 1);

                Queue_Push(&o_UPLINKQueue, txData, txData[0] + 1);
                processed++;
            }
            state = STATE_WAITTING;
            break;

        case STATE_BTN_PRESS_5S:
            if (o_outputLedType == LED_CODE_BLINK_5HZ) {
                o_outputLedType = LED_CODE_OFF;
            } else {
                o_outputLedType = LED_CODE_BLINK_5HZ;
                clearTimer(1);
                setTimer(1, 200);
            }
            state = STATE_WAITTING;
            break;

        default:
            state = STATE_WAITTING;
            break;
    }
}
