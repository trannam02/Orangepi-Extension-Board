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
static uint8_t longPress_state = LONGPRESS_PROCESSING_INIT;

// For couplers info and timeout checking
static uint8_t coupler_arr[32] = {};
static uint8_t number_of_coupler = 0;
static uint8_t couplerX = 0; // range 0 -> 31
static uint32_t timeout1 = 0x00;
static uint32_t timeout2 = 0x00;
static uint32_t timeout3 = 0x00;

// For couplers comm
#define MSG_TYPE_CMD_POLL       0x00 // 00
#define MSG_TYPE_CMD_CONTROL    0x01 // 01
#define MSG_TYPE_RESP_DATA      0x02 // 10
#define MSG_TYPE_PAYLOAD_EMPTY  0x03 // 11

// For detect coupler first time connect, disconnect
static uint32_t firstTime_connect = 0x00000000;
static uint32_t firstTime_disconnect = 0xFFFFFFFF;

// For buzzer and led indication
static uint8_t longPressCounter = 0;

uint8_t setListCoupler(uint8_t * list, uint8_t len){
	if(len < 32 && len >= 0) {
		memcpy(coupler_arr, list, len);
		return 1;
	}else{

	}
	return 0;
};


uint8_t poll_state = POLL_STATE_IDLE;
void poll_processing_run() {
    switch(poll_state) {
        case POLL_STATE_IDLE:
        {
            if(getTimer(2) == 1) {
                clearTimer(2);

                uint8_t CMD_POLL[6] = {0x04, MSG_TYPE_CMD_POLL, coupler_arr[couplerX], 0x00};

                CMD_POLL[4] = crc8(&CMD_POLL[1], 3);

                if (Queue_Push(&o_RS485Queue, CMD_POLL, CMD_POLL[0] + 1)) {
                    LOG_DEBUG("Goi POLL den coupler %d", coupler_arr[couplerX]);

                    clearTimer(3);
                    setTimer(3, POLL_TIMEOUT);
                    poll_state = POLL_STATE_WAIT_RESPONSE;
                } else {
                	clearTimer(2);
                	clearTimer(3);
                	setTimer(2, POLL_INTERVAL);
                	LOG_SPEC_INFO(LOG_CODE_QUEUE_RS485_PUSH_FAIL, HAL_GetTick());
                }
            }
            break;
        }
        case POLL_STATE_WAIT_RESPONSE:
        {
            if(getTimer(3) == 1) {
                clearTimer(3);
                LOG_WARN("POLL Timeout coupler %d", coupler_arr[couplerX]);
                LOG_SPEC_INFO(LOG_CODE_COUPLER_X_TIMEOUT, coupler_arr[couplerX]);

                // timeout

				uint32_t mask = (1U << coupler_arr[couplerX]);

				if (!(timeout1 & mask)) timeout1 |= mask;
				else if (!(timeout2 & mask)) timeout2 |= mask;
				else if (!(timeout3 & mask)) timeout3 |= mask;
				else{
					timeout1 &= ~mask;
					timeout2 &= ~mask;
					timeout3 &= ~mask;

					if (!(firstTime_disconnect & mask)) {
						firstTime_disconnect |= mask;
						firstTime_connect &= ~mask;
						o_outputBuzzerType = BUZZER_CODE_FOR_DISCONNECTED_COUPLER;

						LOG_SPEC_INFO(LOG_CODE_COUPLER_X_DISCONNECT, coupler_arr[couplerX]);
						LOG_WARN("Coupler %d ngat ket noi!", coupler_arr[couplerX]);
					}

					// send report to PI
					uint8_t coupler_disconnect_package[5] = {0x04, 0x0E, 0x03, coupler_arr[couplerX]};
					coupler_disconnect_package[4] = crc8(&coupler_disconnect_package[1], 3);
					if(!Queue_Push(&o_UPLINKQueue, coupler_disconnect_package, coupler_disconnect_package[0] + 1)){
						LOG_SPEC_INFO(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
					};

				}

                // Poll next coupler
                couplerX = (couplerX + 1) % number_of_coupler;
                clearTimer(2);
                setTimer(2, POLL_INTERVAL);
                poll_state = POLL_STATE_IDLE;
            }
            break;
        }
    }
}


void longPress_processing_run(){
	switch(longPress_state){
		case LONGPRESS_PROCESSING_INIT:
			if(longPressCounter == 2 && getButtonReleaseFlag(0)){
				longPressCounter = 0;
				// gui goi bat AP
				uint8_t turnOnAP[6] = {5, 0x0E, 0x01, 0xAB, 0xBA};
				turnOnAP[5] = crc8(&turnOnAP[1], 4);
				if(!Queue_Push(&o_UPLINKQueue, turnOnAP, turnOnAP[0] + 1)){
					LOG_SPEC_INFO(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
				};
			}
			if(longPressCounter == 3 && getButtonReleaseFlag(0)){
				longPressCounter = 0;
				if(system_state == SYSTEM_STATE_CONFIG){
					LOG_SPEC_DEBUG(LOG_CODE_SYSTEM_STATE_RUNNING, HAL_GetTick());
					system_state = SYSTEM_STATE_RUNNING;
					// gui goi tin chuyen sang che do phu hop
					uint8_t switchToRunning[6] = {5, 0x0E, 0x01, 0xBE, 0xEF};
					switchToRunning[5] = crc8(&switchToRunning[1], 4);
					if(!Queue_Push(&o_UPLINKQueue, switchToRunning, switchToRunning[0] + 1)){
						LOG_SPEC_DEBUG(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
					};
				}else{
					LOG_SPEC_INFO(LOG_CODE_SYSTEM_STATE_CONFIG, HAL_GetTick());
					system_state = SYSTEM_STATE_CONFIG;
					// gui goi tin chuyen sang che do phu hop
					uint8_t switchToConfig[6] = {5, 0x0E, 0x01, 0xCA, 0xFE};
					switchToConfig[5] = crc8(&switchToConfig[1], 4);
					if(!Queue_Push(&o_UPLINKQueue, switchToConfig, switchToConfig[0] + 1)){
						LOG_SPEC_DEBUG(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
					};
				};
			}
			if(longPressCounter >= 4){
				o_outputBuzzerType = BUZZER_CODE_ALARM;
				longPress_state = LONGPRESS_PROCESSING_WAITING_FOR_RELEASE;
			}
			break;
		case LONGPRESS_PROCESSING_WAITING_FOR_RELEASE:
			if(getButtonReleaseFlag(0)){
				longPressCounter = 0;
				o_outputBuzzerType = BUZZER_CODE_OFF;
				longPress_state = LONGPRESS_PROCESSING_INIT;
			}
			break;
		default:
			break;
	};
};

void main_processing_init() {
    state = STATE_INIT;
    clearTimer(2);
    setTimer(2, POLL_INTERVAL);


    o_outputLedType = LED_CODE_OFF;
	clearTimer(1);
	setTimer(1, MS(200));
	system_state = SYSTEM_STATE_CONFIG;
}

void main_processing_run() {

	longPress_processing_run();

	if(system_state == SYSTEM_STATE_RUNNING && number_of_coupler > 0){ // other fsm
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
        	if(system_state == SYSTEM_STATE_RUNNING){

				if (i_rs485Queue.count > 0) {
					state = STATE_RECEIVE_RS485_FRAME;
					break;
				};

				if (i_KNXQueue.count > 0) {
					state = STATE_RECEIVE_KNX_FRAME;
					break;
				};
        	}

        	// che do nhan config thi duong nay van chay
        	if (i_ORPQueue.count > 0) {
				state = STATE_PROCESS_DOWNLINK;
				break;
			};

            if (i_inputBtn1PressFlag) {
                i_inputBtn1PressFlag = 0;
                state = STATE_BTN_1_PRESS;
                break;
            }

            if (i_inputBtn1LongPressFlag) {
            	i_inputBtn1LongPressFlag = 0;
                state = STATE_BTN_1_LONGPRESS_1S;
                break;
            }

            break;

        case STATE_PROCESS_DOWNLINK:
            // Lấy từ OrangePi đẩy xuống RS485 hoặc KNX
            while (processed < CONCURENCY_RATE && Queue_Pop(&i_ORPQueue, rxData)) {
            	processed++;

            	if (rxData[0] < 2) {
					LOG_WARN("Drop garbage packet with len=0");
					LOG_SPEC_DEBUG(LOG_CODE_DOWNLINK_DROP_PACKAGE_WITH_LENGTH, rxData[0]);
					continue;
				}
                uint8_t total_len = rxData[0];
                uint8_t header = rxData[1];
                uint8_t payload_size = total_len - 2; // length (1) and header (1)

				uint8_t received_crc = rxData[total_len];
				uint8_t calculated_crc = crc8(&rxData[1], payload_size + 1);

				if (calculated_crc != received_crc) {
					LOG_ERROR("ORP Downlink CRC FAILED! Calc: %02X | Recv: %02X. Drop package!", calculated_crc, received_crc);
					LOG_SPEC_DEBUG(LOG_CODE_DOWNLINK_CRC_FAIL_RECV_CALC, (uint32_t)((0x00000000 | received_crc) << 16) | (uint32_t)((0x00000000 | calculated_crc) << 0));
					continue;
				}

				// xu li goi tin de indicator led
				if(header == HEADER_COMMAND){
					if(	rxData[2] == 0x05 &&
						rxData[3] == 0x01){
						switch(rxData[4]){
						case 0x01:
							o_outputLedType = LED_CODE_BLINK_4HZ;
							break;
						case 0x02:
							o_outputLedType = LED_CODE_BLINK_1HZ;
							break;
						case 0x03:
							o_outputLedType = LED_CODE_ON;
							break;
						case 0x04:
							o_outputLedType = LED_CODE_OFF;
							break;
						default:
							break;
						}
					};

					if( rxData[2] == 0x01 &&
						rxData[3] == 0xFE &&
						rxData[4] == 0xCA){
						system_state = SYSTEM_STATE_CONFIG;
						LOG_SPEC_INFO(LOG_CODE_SYSTEM_STATE_CONFIG, HAL_GetTick());
						if(!Queue_Push(&o_UPLINKQueue, rxData, rxData[0] + 1)){
							LOG_SPEC_DEBUG(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
						};
					};
				}

				if (system_state == SYSTEM_STATE_CONFIG){
					if(rxData[0] == 5 &&
					   rxData[1] == 0x0E &&
					   rxData[2] == 0x01 &&
					   rxData[3] == 0xEF &&
					   rxData[4] == 0xBE){
						o_outputLedType = LED_CODE_OFF;
						system_state = SYSTEM_STATE_RUNNING;
						LOG_SPEC_INFO(LOG_CODE_SYSTEM_STATE_RUNNING, HAL_GetTick());
						if(Queue_Push(&o_UPLINKQueue, rxData, rxData[0] + 1)){// phan hoi goi tuong tu
							LOG_SPEC_DEBUG(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
						};
						continue;
					}

					// check goi tin
					// co tu 1 coupler tro len (len >= 4)
					// byte dau 0E
					// byte thu 2 la 02
					if(rxData[0] >= 4 && rxData[1] == 0x0E && rxData[2] == 0x02){
						uint8_t numberCoupler = rxData[0] - 3; // -1 for function code, -1 for cmd type, -1 for CRC8
						setListCoupler(&rxData[3], numberCoupler);
						number_of_coupler = numberCoupler;

						if(!Queue_Push(&o_UPLINKQueue, rxData, rxData[0] + 1)){
							LOG_SPEC_DEBUG(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
						} // phan hoi goi tuong tu
					}
					continue;
				}

                if (header == HEADER_RS485) {
                	txData[0] = payload_size + 1 + 1; // Length mới = payload + 1 byte CRC + 1 byte MSG_TYPE_CMD_CONTROL
					txData[1] = MSG_TYPE_CMD_CONTROL;
					memcpy(&txData[2], &rxData[2], payload_size);
					txData[2 + payload_size] = crc8(&txData[1], payload_size + 1); // + 1 for MSG type

					// +1 byte length
					if(Queue_Push(&o_RS485Queue, txData, txData[0] + 1)){
						LOG_WARN("ADD QUEUE OK");
					}else{
						LOG_WARN("QUEUE FULL");
						LOG_SPEC_DEBUG(LOG_CODE_QUEUE_RS485_PUSH_FAIL, HAL_GetTick());
					}
					LOG_WARN("Downlink routed to RS485");
					LOG_SPEC_INFO(LOG_CODE_DOWNLINK_ROUTE_TO_RS485, HAL_GetTick());
                } else if (header == HEADER_KNX) {
                	uint8_t* raw_knx_data = &rxData[2];

					uint8_t tpuart_buffer[128];
					uint8_t encoded_len = encode_knx_tpuart(raw_knx_data, payload_size, tpuart_buffer);

					txData[0] = encoded_len;
					memcpy(&txData[1], tpuart_buffer, encoded_len);

					if(!Queue_Push(&o_KNXQueue, txData, encoded_len + 1)){// +1 byte length
						LOG_SPEC_DEBUG(LOG_CODE_QUEUE_KNX_PUSH_FAIL, HAL_GetTick());
					}
					LOG_WARN("Downlink routed to (KNX)");
					LOG_SPEC_INFO(LOG_CODE_DOWNLINK_ROUTE_TO_KNX, HAL_GetTick());
                }
            }
            state = STATE_WAITTING;
            break;

        case STATE_RECEIVE_RS485_FRAME:

            while (processed < CONCURENCY_RATE && Queue_Pop(&i_rs485Queue, rxData)) {
                processed++;

                if (rxData[0] < 1) {
                    LOG_WARN("Drop garbage packet with len=0");
                    LOG_SPEC_DEBUG(LOG_CODE_RS485_DROP_PACKAGE_WITH_LENGTH, rxData[0]);
                    continue;
                }

                uint8_t payload_size = rxData[0] - 1;
                uint8_t received_crc = rxData[rxData[0]];
                uint8_t calculated_crc = crc8(&rxData[1], payload_size);

                if (calculated_crc != received_crc) {
                    LOG_ERROR("RS485 CRC FAILED! Calc: %02X | Recv: %02X. Drop package!", calculated_crc, received_crc);
                    LOG_SPEC_DEBUG(LOG_CODE_RS485_CRC_FAIL_RECV_CALC, (uint32_t)((0x00000000 | received_crc) << 16) | (uint32_t)((0x00000000 | calculated_crc) << 0));
                    continue;
                }


                // =======================================================
                // [LOGIC MỚI] KIỂM TRA PHẢN HỒI CỦA GÓI POLL
                // =======================================================
                if (poll_state == POLL_STATE_WAIT_RESPONSE) {
                    bool is_my_response = false;
                    bool is_my_ack = false;
                    // Kịch bản 1: Nhận được gói ACK cố định
                    if (	rxData[0] == 0x04 &&
                    		rxData[1] == MSG_TYPE_PAYLOAD_EMPTY &&
                    		rxData[2] == coupler_arr[couplerX] &&
                    		rxData[3] == 0x00 &&
                    		rxData[4] == crc8(&rxData[1], 3))
                    {
                        is_my_response = true;
                        is_my_ack = true;
                        LOG_WARN("Nhan ACK tu coupler %d", coupler_arr[couplerX]);
                        LOG_SPEC_DEBUG(LOG_CODE_RS485_RECEIVE_ACK_OF_COUPLER_X, coupler_arr[couplerX]);
                    }
                    // Kịch bản 2: Gói data trả về chứa thông tin couplerX
                    // BẮT BUỘC: Đảm bảo độ dài gói tin >= 4 trước khi soi byte index [3]
                    else if (rxData[0] >= 2 &&
                                        		 rxData[1] == MSG_TYPE_RESP_DATA &&
                                        		 rxData[2] == coupler_arr[couplerX]
                    							 ) {
                                            is_my_response = true;
                        LOG_WARN("Nhan DATA phan hoi tu coupler %d", coupler_arr[couplerX]);
                        LOG_SPEC_DEBUG(LOG_CODE_RS485_RECEIVE_RESPONSE_OF_COUPLER_X, coupler_arr[couplerX]);
                    }else{
                    	LOG_WARN("Nhan pkg nhung k phai data va ack: current couplerid %02X\n", coupler_arr[couplerX]);
                    	LOG_SPEC_DEBUG(LOG_CODE_CHUA_BIET_DEFINE_GI, HAL_GetTick());
                    	for(int i = 1; i < rxData[0]+1; i++){
                    		LOG_WARN("%02X", rxData[i]);
                    	}

                    }

                    if (is_my_response) {
                        clearTimer(3);

                        // process for polling
                        if(1){
                        	uint32_t mask = (1U << coupler_arr[couplerX]);
							timeout1 &= ~mask;
							timeout2 &= ~mask;
							timeout3 &= ~mask;

							if (!(firstTime_connect & mask)) {

								firstTime_connect |= mask;
								firstTime_disconnect &= ~mask;
								o_outputBuzzerType = BUZZER_CODE_FOR_CONNECTED_COUPLER;
							}
							poll_state = POLL_STATE_IDLE;
                        }

                        couplerX = (couplerX + 1) % number_of_coupler;
                        clearTimer(2);
                        setTimer(2, POLL_INTERVAL);


                        // NẾU LÀ GÓI ACK -> KHÔNG LÀM GÌ CẢ (CHỈ TIẾP TỤC VÒNG LẶP)
                        if(is_my_ack){
                            continue;
                        }

                        // PROCESS DAY DATA LÊN ORANGE PI
                        txData[0] = payload_size + 1; // Length = payload + Header + CRC
                        txData[1] = HEADER_RS485;
                        memcpy(&txData[2], &rxData[2], payload_size - 1);
                        txData[3 + payload_size - 1] = crc8(&txData[1], payload_size + 1 - 1); // Tính CRC cho Header + Payload
                        if(!Queue_Push(&o_UPLINKQueue, txData, txData[0] + 1)){
                        	LOG_SPEC_DEBUG(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
                        }
                    }
                    // Nếu không phải phản hồi mình cần -> Lệnh continue ngầm (hết vòng lặp)
                }else{
                	LOG_WARN("IDLING, not waiting for response", coupler_arr[couplerX]);
                	LOG_SPEC_DEBUG(LOG_CODE_CHUA_BIET_DEFINE_GI, HAL_GetTick());
					for(int i = 1; i < rxData[0]+1; i++){
						LOG_WARN("%02X", rxData[i]);
					}
                }

            }
            state = STATE_WAITTING;
            break;

        case STATE_RECEIVE_KNX_FRAME:
            // Lấy từ KNX đẩy lên OrangePi
            while (processed < CONCURENCY_RATE && Queue_Pop(&i_KNXQueue, rxData)) {
            	processed++;

                uint8_t total_len = rxData[0];

                if(total_len < 5){
                	LOG_WARN("Drop KNX gargbage len=%d", total_len);
                	LOG_SPEC_DEBUG(LOG_CODE_KNX_DROP_PACKAGE_WITH_LENGTH, total_len);
                	continue;
                }

				uint8_t received_checksum = rxData[total_len];
				uint8_t calculated_checksum = knx_checksum(&rxData[1], total_len - 1);
				if (calculated_checksum != received_checksum) {

					if(calculated_checksum == 0){ // truong hop nay, knx phan hoi goi da gui + L_Data.con
//						total_len = total_len - 1;
						LOG_WARN("KNX ACK of sent package: isACK=%d", rxData[total_len] >> 7);
					}else{
						LOG_ERROR("KNX Frame Checksum FAILED! Calc: %02X | Recv: %02X", calculated_checksum, received_checksum);
						LOG_SPEC_DEBUG(LOG_CODE_KNX_CHECKSUM_FAIL_RECV_CALC, (uint32_t)((0x00000000 | received_checksum) << 16) | (uint32_t)((0x00000000 | calculated_checksum) << 0));
						continue;
					}

				}

				uint8_t payload_size = total_len;
                txData[0] = payload_size + 2;
                txData[1] = HEADER_KNX;
                memcpy(&txData[2], &rxData[1], payload_size);
                txData[2 + payload_size] = crc8(&txData[1], payload_size + 1);

                if(!Queue_Push(&o_UPLINKQueue, txData, txData[0] + 1)){
                	LOG_SPEC_DEBUG(LOG_CODE_QUEUE_UPLINK_PUSH_FAIL, HAL_GetTick());
                }
            }
            state = STATE_WAITTING;
            break;

        case STATE_BTN_1_PRESS:
//        	o_outputBuzzerType = BUZZER_CODE_ERROR;
            state = STATE_WAITTING;
            break;
        case STATE_BTN_1_LONGPRESS_1S:
        	LOG_WARN("BUTTON PRESS 1s");
        	LOG_SPEC_DEBUG(LOG_CODE_BUTTON_CONTINUE_PRESS_1S, HAL_GetTick());
        	longPressCounter += 1;
        	if(longPressCounter < 4) o_outputBuzzerType = BUZZER_CODE_FOR_BEEP_1S;

			state = STATE_WAITTING;
		break;
        default:
            state = STATE_WAITTING;
            break;
    }
}
