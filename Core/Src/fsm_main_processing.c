#include <fsm_main_processing.h>

static uint8_t state = STATE_INIT;

//static uint8_t temp_buffer[(O_RS485_TX_MAX_BUFFER_SIZE + 1) * O_RS485_MAX_QUEUE_SIZE];
//static uint32_t donvi_size = sizeof(uint8_t) * (O_RS485_TX_MAX_BUFFER_SIZE + 1);
void main_processing_init(){
	state = STATE_INIT;
};
void main_processing_run(){
	switch(state){
		case STATE_INIT:
		{
			state = STATE_WAITTING;
			break;
		}
		case STATE_WAITTING:
		{
			if(i_rs485NumEl != 0){
				state = STATE_RECEIVE_RS485_FRAME;
				break;
			}
			if(inputBtn1PressFlag){
				inputBtn1PressFlag = 0;
				state = STATE_BTN_PRESS_5S;
				break;
			};
			break;
		}
		case STATE_RECEIVE_RS485_FRAME:
		{
			// copy all data from rs485 queue to uplink queue

			// this WRONG, just write for test
			// push all data to rs485 output buffer
			uint8_t o_controng = O_RS485_MAX_QUEUE_SIZE - o_RS485QueueNumEl;
			uint8_t sl_cp = i_rs485NumEl;
			uint8_t index = i_rs485QueueIndex;
			if(sl_cp <= o_controng){
				// do nothing
			}else {
				sl_cp = o_controng;
				index = (i_rs485QueueIndex + RS485_MAX_QUEUE - (i_rs485NumEl - o_controng)) % RS485_MAX_QUEUE;
			}

			// VAN CON BUG CHO NAY, NEU OUTPUT TRAN, INPUT KHONG COPY QUA,
			// THI QUAY VE STATE WAITTING ROI LAI NHAY VAO STATE NAY, INFINITI LOOP
			if(sl_cp <= 0){
				// do nothing
				state = STATE_WAITTING;
				break;
			}

			uint32_t donvi_size = sizeof(uint8_t) * (O_RS485_TX_MAX_BUFFER_SIZE + 1);

			LOG_DEBUG("Before receive: o_RS485QueueNumEl=%d o_RS485QueueIndex=%d", o_RS485QueueNumEl, o_RS485QueueIndex);
			LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_RS485QueueNumEl, o_RS485QueueIndex);
			int tail = (o_RS485QueueIndex + O_RS485_MAX_QUEUE_SIZE - o_RS485QueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
			for (int i = 0; i < o_RS485QueueNumEl; i++) {
				int current_pos = (tail + i) % O_RS485_MAX_QUEUE_SIZE;
				LOG_DEBUG("[%d] %s", current_pos, o_RS485Queue[current_pos]);
			}
			LOG_DEBUG("---------------------------------------------");

//			if(index == sl_cp){
//				LOG_DEBUG("IF: index = sl_cp");
//				// input: cpy tu dau, sl = sl_cp
//				memset(temp_buffer, 0, (O_RS485_TX_MAX_BUFFER_SIZE + 1) * O_RS485_MAX_QUEUE_SIZE);
//
//				memcpy(temp_buffer, i_rs485Queue, donvi_size * sl_cp);
//
//				// output:
//				uint8_t o_controng_to_end = (O_RS485_MAX_QUEUE_SIZE - o_RS485QueueIndex);
//				if(sl_cp <= o_controng_to_end){
//					memcpy(o_RS485Queue + o_RS485QueueIndex, temp_buffer, donvi_size * sl_cp);
//				}else{
//					 memcpy(o_RS485Queue + o_RS485QueueIndex, temp_buffer,  donvi_size * o_controng_to_end);
//					 memcpy(o_RS485Queue, temp_buffer + donvi_size * o_controng_to_end, donvi_size*(sl_cp - o_controng_to_end));
//				}
//			}else if (index < sl_cp){
//				LOG_DEBUG("IF: index < sl_cp");
//				// cpy tu dau, sl = index
//				// cpy tu vi tri: max - (sl_cp - index), sl = (sl_cp - index)
//				memset(temp_buffer, 0, (O_RS485_TX_MAX_BUFFER_SIZE + 1) * O_RS485_MAX_QUEUE_SIZE);
//
//				memcpy(temp_buffer, i_rs485Queue + (O_RS485_MAX_QUEUE_SIZE - sl_cp + index), donvi_size * (sl_cp - index));
//				memcpy(temp_buffer + donvi_size * (sl_cp - index), i_rs485Queue, donvi_size * index);
//
//				uint8_t o_controng_to_end = O_RS485_MAX_QUEUE_SIZE - o_RS485QueueIndex;
//				if(sl_cp <= o_controng_to_end){
//					 memcpy(o_RS485Queue + o_RS485QueueIndex, temp_buffer, donvi_size * sl_cp);
//				}else{
//					 memcpy(o_RS485Queue + o_RS485QueueIndex, temp_buffer,  donvi_size * o_controng_to_end);
//					 memcpy(o_RS485Queue, temp_buffer + donvi_size * o_controng_to_end, donvi_size*(sl_cp - o_controng_to_end));
//				}
//			}else { // index > sl_cp
//				// cpy tu (index - sl_cp), sl = sl_cp
//				LOG_DEBUG("IF: index > sl_cp");
//				memset(temp_buffer, 0, (O_RS485_TX_MAX_BUFFER_SIZE + 1) * O_RS485_MAX_QUEUE_SIZE);
//
//				// --- DÒNG CODE CỦA BẠN ---
//				// Lưu ý: Bạn đang copy TỪ temp_buffer (chưa khởi tạo) VÀO rs485Queue?
//				memcpy(temp_buffer, i_rs485Queue + (index - sl_cp), donvi_size * sl_cp);
//
//				// output:
//				uint8_t o_controng_to_end = O_RS485_MAX_QUEUE_SIZE - o_RS485QueueIndex;
//				if(sl_cp <= o_controng_to_end){
//					 memcpy(o_RS485Queue + o_RS485QueueIndex, temp_buffer, donvi_size * sl_cp);
//				}else{
//					 memcpy(o_RS485Queue + o_RS485QueueIndex, temp_buffer,  donvi_size * o_controng_to_end);
//					 memcpy(o_RS485Queue, temp_buffer + donvi_size * o_controng_to_end, donvi_size*(sl_cp - o_controng_to_end));
//				}
//			}
			if (sl_cp > 0) {
			    int input_tail = (i_rs485QueueIndex + RS485_MAX_QUEUE - i_rs485NumEl) % RS485_MAX_QUEUE;
			    int current_out_head = o_RS485QueueIndex;
			    int input_len_to_end = RS485_MAX_QUEUE - input_tail;
			    int chunk1_size = (sl_cp < input_len_to_end) ? sl_cp : input_len_to_end;
			    int out_space_to_end = O_RS485_MAX_QUEUE_SIZE - current_out_head;

			    if (chunk1_size <= out_space_to_end) {
			        memcpy(o_RS485Queue[current_out_head], i_rs485Queue[input_tail], donvi_size * chunk1_size);
			    } else {
			        memcpy(o_RS485Queue[current_out_head], i_rs485Queue[input_tail], donvi_size * out_space_to_end);
			        memcpy(o_RS485Queue[0], i_rs485Queue[input_tail + out_space_to_end], donvi_size * (chunk1_size - out_space_to_end));
			    }
			    current_out_head = (current_out_head + chunk1_size) % O_RS485_MAX_QUEUE_SIZE;
			    int chunk2_size = sl_cp - chunk1_size;

			    if (chunk2_size > 0) {
			        out_space_to_end = O_RS485_MAX_QUEUE_SIZE - current_out_head;
			        if (chunk2_size <= out_space_to_end) {
			            memcpy(o_RS485Queue[current_out_head], i_rs485Queue[0], donvi_size * chunk2_size);
			        } else {
			            memcpy(o_RS485Queue[current_out_head], i_rs485Queue[0], donvi_size * out_space_to_end);
			            memcpy(o_RS485Queue[0], i_rs485Queue[out_space_to_end], donvi_size * (chunk2_size - out_space_to_end));
			        }
			    }
			}
//			LOG_DEBUG("IF DONE");
//			if(i_rs485NumEl <= o_controng){
////				i_rs485QueueIndex = (i_rs485QueueIndex + RS485_MAX_QUEUE - sl_cp) % RS485_MAX_QUEUE;
//				i_rs485NumEl = i_rs485NumEl - sl_cp;
//			}else{
//				i_rs485NumEl = i_rs485NumEl - sl_cp;
//			}
			i_rs485NumEl = i_rs485NumEl - sl_cp;

			o_RS485QueueIndex = (o_RS485QueueIndex + sl_cp) % RS485_MAX_QUEUE;
			o_RS485QueueNumEl = o_RS485QueueNumEl + sl_cp;

			LOG_DEBUG("After receive: o_RS485QueueNumEl=%d o_RS485QueueIndex=%d", o_RS485QueueNumEl, o_RS485QueueIndex);
			{
				LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_RS485QueueNumEl, o_RS485QueueIndex);
				int tail = (o_RS485QueueIndex + O_RS485_MAX_QUEUE_SIZE - o_RS485QueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
				for (int i = 0; i < o_RS485QueueNumEl; i++) {
					int current_pos = (tail + i) % O_RS485_MAX_QUEUE_SIZE;
					LOG_DEBUG("[%d] %s", current_pos, o_RS485Queue[current_pos]);
				}
				LOG_DEBUG("---------------------------------------------");
			}

			state = STATE_WAITTING;
			break;
		}
		case STATE_RECEIVE_KNX_FRAME:
		{
			state = STATE_WAITTING;
			break;
		}
		case STATE_BTN_PRESS_5S: // RESET SYSTEM - ten tam thoi la press 5s
		{
			if(outputLedType == LED_CODE_BLINK_5HZ){
				outputLedType = LED_CODE_OFF;
			}else{
				outputLedType = LED_CODE_BLINK_5HZ;
				clearTimer(1);
				setTimer(1, 200);
			}

			state = STATE_WAITTING;
			break;
		}
		default:
		{
			break;
		};

	}
	// Flag Uart 1 - RX

	// Flag Uart 2 - RX

	// Flag Uart 3 - RX

	// Flag button press 5s

};
