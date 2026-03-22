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

uint8_t poll_state = POLL_STATE_IDLE;

void poll_processing_run() {
    switch(poll_state) {
        case POLL_STATE_IDLE:
        {
            if(getTimer(2) == 1) { // getTimer(2) dùng làm timer đếm 10ms
                clearTimer(2);

                uint8_t CMD_POLL[6] = {0x05, 0x05, couplerX, 0x00, 0x00, 0x20};

                if (Queue_Push(&o_RS485Queue, CMD_POLL, CMD_POLL[0] + 1)) {
                    LOG_DEBUG("Goi POLL den coupler %d", couplerX);

                    clearTimer(3);
                    setTimer(3, POLL_TIMEOUT); // getTimer(3) dùng làm timer timeout
                    poll_state = POLL_STATE_WAIT_RESPONSE;
                } else {
                    // Nếu Queue đang đầy (kẹt), thử lại sau 1ms
                	clearTimer(2);
                    setTimer(2, POLL_INTERVAL);
                }
            }
            break;
        }
        case POLL_STATE_WAIT_RESPONSE:
        {
            if(getTimer(3) == 1) { // Hết 5ms mà chưa có trạm nào trả lời
                clearTimer(3);
                LOG_WARN("POLL Timeout coupler %d", couplerX);

                // Tăng index, modulo 3 và chuyển về trạng thái nghỉ 10ms để gọi trạm tiếp theo
                couplerX = (couplerX + 1) % NUMBER_COUPLER;
                clearTimer(2);
                setTimer(2, POLL_INTERVAL);
                poll_state = POLL_STATE_IDLE;
            }
            break;
        }
    }
}

void main_processing_init() {
    state = STATE_INIT;
    clearTimer(2);
    setTimer(2, POLL_INTERVAL);
}

void main_processing_run() {

	{ // other fsm
		poll_processing_run();
	}


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
                uint8_t payload_size = total_len - 2; // Bỏ length (1) và header (1) |   payload   | crc ko can bo (CHÚ Ý 1)

                // =========================================================
				// ĐOẠN CHECK CRC (THÊM MỚI)
				// =========================================================
				uint8_t received_crc = rxData[total_len];

				// LƯU Ý: Bạn cần xem lại OrangePi đang tính CRC cho [Header + Payload] hay chỉ [Payload].
				// - Nếu tính cho cả Header + Payload: crc8(&rxData[1], payload_size + 1)
				// - Nếu chỉ tính cho Payload: crc8(&rxData[2], payload_size)
				uint8_t calculated_crc = crc8(&rxData[1], payload_size + 1);

				if (calculated_crc != received_crc) {
					LOG_ERROR("ORP Downlink CRC FAILED! Calc: %02X | Recv: %02X. Drop package!", calculated_crc, received_crc);
					processed++; // Vẫn tính là đã xử lý 1 gói để không kẹt FSM
					continue;    // Bỏ qua các bước dưới, quay lại đầu vòng while bốc gói tiếp theo
				}
				// =========================================================

                if (header == HEADER_RS485) {
                    txData[0] = payload_size + 1; // Length mới = payload + 1 byte CRC
                    memcpy(&txData[1], &rxData[2], payload_size);
                    txData[1 + payload_size] = crc8(&txData[1], payload_size);

                    Queue_Push(&o_RS485Queue, txData, txData[0] + 1); // +1 chứa byte length
                    LOG_DEBUG("Downlink routed to RS485");

                } else if (header == HEADER_KNX) {
//                    txData[0] = payload_size + 1;
//                    memcpy(&txData[1], &rxData[2], payload_size);
//                    txData[1 + payload_size] = crc8(&txData[1], payload_size);
//
//                    Queue_Push(&o_KNXQueue, txData, txData[0] + 1);
//                    LOG_DEBUG("Downlink routed to KNX");
                	uint8_t* raw_knx_data = &rxData[2];

					uint8_t tpuart_buffer[2 * payload_size + 10];
					uint8_t encoded_len = encode_knx_tpuart(raw_knx_data, payload_size, tpuart_buffer);

					// =========================================================
					// ĐOẠN CODE LOG DỮ LIỆU ĐỂ DEBUG (THÊM MỚI)
					// =========================================================
					LOG_WARN("HEHE");
					char raw_hex[128] = {0};
					char enc_hex[256+10] = {0};
					int offset_raw = 0;
					int offset_enc = 0;

					// Giới hạn chiều dài in ra để chống tràn mảng string (tối đa 40 byte raw, 80 byte encode)
					uint8_t p_raw_len = (payload_size > 40) ? 40 : payload_size;
					uint8_t p_enc_len = (encoded_len > 80) ? 80 : encoded_len;

					// Tạo chuỗi Hex cho gói KNX gốc
					for (int i = 0; i < p_raw_len; i++) {
						offset_raw += sprintf(raw_hex + offset_raw, "%02X ", raw_knx_data[i]);
					}
					LOG_WARN("HOHO");
					// Tạo chuỗi Hex cho gói TP-UART sau khi encode
					for (int i = 0; i < p_enc_len; i++) {
						offset_enc += sprintf(enc_hex + offset_enc, "%02X ", tpuart_buffer[i]);
					}

					// In ra màn hình console
					LOG_WARN("KNX RAW (Len: %d): %s", payload_size, raw_hex);
					LOG_WARN("KNX ENC (Len: %d): %s", encoded_len, enc_hex);
					// =========================================================

					// 3. Đóng gói vào KNX Output Queue
					txData[0] = encoded_len; // Ghi chiều dài thực tế cần gửi DMA
					memcpy(&txData[1], tpuart_buffer, encoded_len);

					// PUSH VÀO QUEUE (Cộng 1 byte length ở index 0)
					Queue_Push(&o_KNXQueue, txData, encoded_len + 1);
					LOG_WARN("Downlink routed and encoded for NCN5130 (KNX)");
                }
                processed++;
            }
            state = STATE_WAITTING;
            break;

        case STATE_RECEIVE_RS485_FRAME:
            // Lấy từ RS485 đẩy lên OrangePi
            while (processed < CONCURENCY_RATE && Queue_Pop(&i_rs485Queue, rxData)) {

                // LUÔN LUÔN đếm gói tin đã được lấy ra khỏi Queue
                processed++;

                // =========================================================
                // [SAFETY CHECK] CHỐNG SẬP RAM (TRÁNH UNDERFLOW)
                // =========================================================
                if (rxData[0] < 1) {
                    LOG_WARN("Drop garbage packet with len=0");
                    continue;
                }

                // =========================================================
                // ĐOẠN CHECK CRC
                // =========================================================
                uint8_t payload_size = rxData[0] - 1;
                uint8_t received_crc = rxData[rxData[0]];
                uint8_t calculated_crc = crc8(&rxData[1], payload_size);

                if (calculated_crc != received_crc) {
                    LOG_ERROR("RS485 CRC FAILED! Calc: %02X | Recv: %02X. Drop package!", calculated_crc, received_crc);
                    continue;
                }
                // =========================================================


                // =======================================================
                // [LOGIC MỚI] KIỂM TRA PHẢN HỒI CỦA GÓI POLL
                // =======================================================
                if (poll_state == POLL_STATE_WAIT_RESPONSE) {
                    bool is_my_response = false;
                    bool is_my_ack = false;
                    // Kịch bản 1: Nhận được gói ACK cố định
                    if (rxData[0] == 0x06 && rxData[1] == 0x01 && rxData[2] == 0x02 && rxData[3] == couplerX) {
                        is_my_response = true;
                        is_my_ack = true;
                        LOG_WARN("Nhan ACK tu coupler %d", couplerX);
                    }
                    // Kịch bản 2: Gói data trả về chứa thông tin couplerX
                    // BẮT BUỘC: Đảm bảo độ dài gói tin >= 4 trước khi soi byte index [3]
                    else if (rxData[0] >= 4 && rxData[3] == couplerX) {
                        is_my_response = true;
                        LOG_WARN("Nhan DATA phan hoi tu coupler %d", couplerX);
                    }

                    // NẾU ĐÚNG LÀ PHẢN HỒI MÌNH ĐANG ĐỢI:
                    if (is_my_response) {
                        clearTimer(3); // 1. Hủy Timer Timeout ngay lập tức
                        couplerX = (couplerX + 1) % NUMBER_COUPLER; // 2. Tăng index cho lần sau
                        clearTimer(2);
                        setTimer(2, POLL_INTERVAL); // 3. Set nghỉ 10ms trước khi gọi trạm tiếp theo
                        poll_state = POLL_STATE_IDLE;  // 4. Đưa FSM Poll về trạng thái chờ

                        // NẾU LÀ GÓI ACK RỖNG -> KHÔNG LÀM GÌ CẢ (CHỈ TIẾP TỤC VÒNG LẶP)
                        if(is_my_ack){
                            continue;
                        }

                        // PROCESS DAY DATA LÊN ORANGE PI
                        txData[0] = payload_size + 2; // Length = payload + Header + CRC
                        txData[1] = HEADER_RS485;
                        memcpy(&txData[2], &rxData[1], payload_size);
                        txData[2 + payload_size] = crc8(&txData[1], payload_size + 1); // Tính CRC cho Header + Payload
                        Queue_Push(&o_UPLINKQueue, txData, txData[0] + 1);
                    }
                    // Nếu không phải phản hồi mình cần -> Lệnh continue ngầm (hết vòng lặp)
                }
                // Nếu không ở trạng thái WAIT_RESPONSE -> Lệnh continue ngầm (hết vòng lặp)
                // =======================================================
            }
            state = STATE_WAITTING;
            break;

        case STATE_RECEIVE_KNX_FRAME:
            // Lấy từ KNX đẩy lên OrangePi
            while (processed < CONCURENCY_RATE && Queue_Pop(&i_KNXQueue, rxData)) {

                uint8_t total_len = rxData[0];

                if(total_len < 5){
                	LOG_WARN("Drop KNX gargbage len=%d", total_len);
                	processed++;
                	continue;
                }

				uint8_t received_checksum = rxData[total_len];
				uint8_t calculated_checksum = knx_checksum(&rxData[1], total_len - 1);
				if (calculated_checksum != received_checksum) {
					LOG_ERROR("KNX Frame Checksum FAILED! Calc: %02X | Recv: %02X", calculated_checksum, received_checksum);
					processed++;
					continue;
				}

				uint8_t payload_size = total_len;
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
                setTimer(1, MS(200));
            }
            state = STATE_WAITTING;
            break;

        default:
            state = STATE_WAITTING;
            break;
    }
}
