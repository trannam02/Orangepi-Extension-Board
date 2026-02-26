#include <fsm_main_processing.h>

static uint8_t state = STATE_INIT;

uint8_t o_controng;
uint8_t sl_cp;
uint8_t input_read_idx;
uint8_t output_write_idx;
uint8_t payload_size;
uint8_t current_src;
uint8_t current_dst;
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
			if(i_ORPNumEl != 0){
				state = STATE_PROCESS_DOWNLINK;
				break;
			};
			if(i_rs485NumEl != 0){
				state = STATE_RECEIVE_RS485_FRAME;
				break;
			}
			if(i_KNXNumEl != 0){
				state = STATE_RECEIVE_KNX_FRAME;
				break;
			};
			if(i_inputBtn1PressFlag){
				i_inputBtn1PressFlag = 0;
				state = STATE_BTN_PRESS_5S;
				break;
			};
			break;
		}
		case STATE_PROCESS_DOWNLINK:
		{
			// neu sai crc => drop + noti (ACK)
			// check header
			// /*TODO*/hien tai, neu output day, thi goi tin nam trong input buffer
			// se bi bo qua va drop luon

			sl_cp = i_KNXNumEl;
			if(sl_cp <= CONCURENCY_RATE){
				// do nothing
			}else {
				sl_cp = CONCURENCY_RATE;
			}

			// VAN CON BUG CHO NAY, NEU OUTPUT TRAN, INPUT KHONG COPY QUA,
			// THI QUAY VE STATE WAITTING ROI LAI NHAY VAO STATE NAY, INFINITY LOOP
			if(sl_cp <= 0){
				// do nothing
				LOG_DEBUG("sl_cp <= 0, skip process");
				state = STATE_WAITTING;
				break;
			}

			// Process move data from input queue to output queue
			if (sl_cp > 0) {
				// Tail của Input (Gói cũ nhất)
				input_read_idx = (i_KNXQueueIndex + I_KNX_MAX_QUEUE_SIZE - i_KNXNumEl) % I_KNX_MAX_QUEUE_SIZE;
				// Head của Output (Vị trí trống tiếp theo)
				int rs485_output_write_idx = o_RS485QueueIndex;
				int knx_output_write_idx = o_KNXQueueIndex;

				for (int i = 0, j = 0, k = 0; i < sl_cp; i++) {
					current_src = (input_read_idx + i) % I_KNX_MAX_QUEUE_SIZE;

					// Check CRC

					// Check header
					if(i_ORPQueue[current_src][0] == HEADER_RS485){
						if(o_RS485QueueNumEl >= O_RS485_MAX_QUEUE_SIZE){
							continue;
						};

						current_dst = (rs485_output_write_idx + j) % O_RS485_MAX_QUEUE_SIZE;
						j += 1;

						payload_size = strlen((uint8_t*)&i_ORPQueue[current_src]) - 1 - 1; // minus 1 for header, 1 for CRC8
						memcpy(&o_RS485Queue[current_dst], i_ORPQueue[current_src][1], payload_size);
						o_RS485Queue[current_dst][1 + payload_size] = crc8((uint8_t*)&o_RS485Queue[current_dst], 1+payload_size);

						o_RS485Queue[current_dst][1 + payload_size + 1] = '\0'; // add this for easy output process

						o_RS485QueueIndex = (o_RS485QueueIndex + 1) % O_RS485_MAX_QUEUE_SIZE;
						o_RS485QueueNumEl = o_RS485QueueNumEl + 1;
					}else if(i_ORPQueue[current_src][0] == HEADER_KNX){
						if(o_KNXQueueNumEl >= O_KNX_MAX_QUEUE_SIZE){
							continue;
						};

						current_dst = (knx_output_write_idx + k) % O_KNX_MAX_QUEUE_SIZE;
						k += 1;

						payload_size = strlen((uint8_t*)&i_ORPQueue[current_src]) - 1 - 1; // minus 1 for header, 1 for CRC8
						memcpy(&o_KNXQueue[current_dst], i_ORPQueue[current_src][1], payload_size);
						o_KNXQueue[current_dst][1 + payload_size] = crc8((uint8_t*)&o_KNXQueue[current_dst], 1+payload_size);

						o_KNXQueue[current_dst][1 + payload_size + 1] = '\0'; // add this for easy output process

						o_KNXQueueIndex = (o_KNXQueueIndex + 1) % O_KNX_MAX_QUEUE_SIZE;
						o_KNXQueueNumEl = o_KNXQueueNumEl + 1;
					}else{ // xu ly cac truong hop khac

					}

				}

				// Process index and number elements
				i_ORPNumEl = i_ORPNumEl - sl_cp;
			}


			LOG_DEBUG("After receive: o_UPLINKQueueNumEl=%d o_UPLINKQueueIndex=%d", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
			{
				LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
				int tail = (o_UPLINKQueueIndex + O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
				for (int i = 0; i < o_UPLINKQueueNumEl; i++) {
					int current_pos = (tail + i) % O_RS485_MAX_QUEUE_SIZE;
					LOG_DEBUG("[%d] %s", current_pos, o_UPLINKQueue[current_pos]);
				}
				LOG_DEBUG("---------------------------------------------");
			}
			state = STATE_WAITTING;
			break;
		}
		case STATE_RECEIVE_RS485_FRAME:
		{
			// neu sai crc => drop + noti (ACK)

			// copy all data from rs485 queue to uplink queue

			o_controng = O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl;
			sl_cp = i_rs485NumEl;
			if(sl_cp <= o_controng){
				// do nothing
			}else {
				sl_cp = o_controng;
			}

			if(sl_cp <= CONCURENCY_RATE){
				// do nothing
			}else {
				sl_cp = CONCURENCY_RATE;
			}

			// VAN CON BUG CHO NAY, NEU OUTPUT TRAN, INPUT KHONG COPY QUA,
			// THI QUAY VE STATE WAITTING ROI LAI NHAY VAO STATE NAY, INFINITY LOOP
			if(sl_cp <= 0){
				// do nothing
				LOG_DEBUG("sl_cp <= 0, skip process");
				state = STATE_WAITTING;
				break;
			}

			LOG_DEBUG("Before receive: o_UPLINKQueueNumEl=%d o_UPLINKQueueIndex=%d", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
			LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
			int tail = (o_UPLINKQueueIndex + O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl) % O_UPLINK_MAX_QUEUE_SIZE;
			for (int i = 0; i < o_UPLINKQueueNumEl; i++) {
				int current_pos = (tail + i) % O_UPLINK_MAX_QUEUE_SIZE;
				LOG_DEBUG("[%d] %s", current_pos, o_UPLINKQueue[current_pos]);
			}
			LOG_DEBUG("---------------------------------------------");

			// Process move data from input queue to output queue
			if (sl_cp > 0) {
			    // Tail của Input (Gói cũ nhất)
			    input_read_idx = (i_rs485QueueIndex + RS485_MAX_QUEUE - i_rs485NumEl) % RS485_MAX_QUEUE;
			    // Head của Output (Vị trí trống tiếp theo)
			    output_write_idx = o_UPLINKQueueIndex;

			    for (int i = 0; i < sl_cp; i++) {
			        current_src = (input_read_idx + i) % RS485_MAX_QUEUE;
			        current_dst = (output_write_idx + i) % O_RS485_MAX_QUEUE_SIZE;

			        o_UPLINKQueue[current_dst][0] = HEADER_RS485; // Byte Header 1

			        payload_size = strlen((uint8_t*)&i_rs485Queue[current_src]);
			        memcpy(&o_UPLINKQueue[current_dst][1], i_rs485Queue[current_src], payload_size);
			        // crc8 calc only for header + payload (not for \0)
			        LOG_DEBUG("HIHIHIHI %s", i_rs485Queue[current_src]);
			        o_UPLINKQueue[current_dst][1 + payload_size] = crc8((uint8_t*)&o_UPLINKQueue[current_dst], 1+payload_size);

			        o_UPLINKQueue[current_dst][1 + payload_size + 1] = '\0'; // add this for easy output process
			    }

			    // Process index and number elements
			    i_rs485NumEl = i_rs485NumEl - sl_cp;
				o_UPLINKQueueIndex = (o_UPLINKQueueIndex + sl_cp) % O_UPLINK_MAX_QUEUE_SIZE;
				o_UPLINKQueueNumEl = o_UPLINKQueueNumEl + sl_cp;
			}


			LOG_DEBUG("ALO After receive: o_UPLINKQueueNumEl=%d o_UPLINKQueueIndex=%d", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
			{
				LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
				int tail = (o_UPLINKQueueIndex + O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
				for (int i = 0; i < o_UPLINKQueueNumEl; i++) {
					int current_pos = (tail + i) % O_RS485_MAX_QUEUE_SIZE;
					LOG_DEBUG("[%d] %s", current_pos, o_UPLINKQueue[current_pos]);
				}
				LOG_DEBUG("---------------------------------------------");
			}

			state = STATE_WAITTING;
			break;
		}
		case STATE_RECEIVE_KNX_FRAME:
		{
			// neu sai crc => drop + noti (ACK)

			o_controng = O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl;
			sl_cp = i_KNXNumEl;
			if(sl_cp <= o_controng){
				// do nothing
			}else {
				sl_cp = o_controng;
			}

			if(sl_cp <= CONCURENCY_RATE){
				// do nothing
			}else {
				sl_cp = CONCURENCY_RATE;
			}

			// VAN CON BUG CHO NAY, NEU OUTPUT TRAN, INPUT KHONG COPY QUA,
			// THI QUAY VE STATE WAITTING ROI LAI NHAY VAO STATE NAY, INFINITY LOOP
			if(sl_cp <= 0){
				// do nothing
				LOG_DEBUG("sl_cp <= 0, skip process");
				state = STATE_WAITTING;
				break;
			}

			// Process move data from input queue to output queue
			if (sl_cp > 0) {
				// Tail của Input (Gói cũ nhất)
				input_read_idx = (i_KNXQueueIndex + I_KNX_MAX_QUEUE_SIZE - i_KNXNumEl) % I_KNX_MAX_QUEUE_SIZE;
				// Head của Output (Vị trí trống tiếp theo)
				output_write_idx = o_UPLINKQueueIndex;


				uint32_t payload_size = O_UPLINK_TX_MAX_BUFFER_SIZE - 1 - 1; // decrease 1 for header, 1 for CRC8

				for (int i = 0; i < sl_cp; i++) {
					current_src = (input_read_idx + i) % I_KNX_MAX_QUEUE_SIZE;
					current_dst = (output_write_idx + i) % O_RS485_MAX_QUEUE_SIZE;

					o_UPLINKQueue[current_dst][0] = HEADER_KNX; // Byte Header 1

					payload_size = strlen((uint8_t*)&i_KNXQueue[current_src]);
					memcpy(&o_UPLINKQueue[current_dst][1], i_KNXQueue[current_src], payload_size);
					o_UPLINKQueue[current_dst][1 + payload_size] = crc8((uint8_t*)&o_UPLINKQueue[current_dst], 1+payload_size);

					o_UPLINKQueue[current_dst][1 + payload_size + 1] = '\0'; // add this for easy output process
				}

				// Process index and number elements
				i_KNXNumEl = i_KNXNumEl - sl_cp;
				o_UPLINKQueueIndex = (o_UPLINKQueueIndex + sl_cp) % O_UPLINK_MAX_QUEUE_SIZE;
				o_UPLINKQueueNumEl = o_UPLINKQueueNumEl + sl_cp;
			}


			LOG_DEBUG("After receive: o_UPLINKQueueNumEl=%d o_UPLINKQueueIndex=%d", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
			{
				LOG_DEBUG("--- QUEUE DUMP (Count: %d | Head Index: %d) ---", o_UPLINKQueueNumEl, o_UPLINKQueueIndex);
				int tail = (o_UPLINKQueueIndex + O_UPLINK_MAX_QUEUE_SIZE - o_UPLINKQueueNumEl) % O_RS485_MAX_QUEUE_SIZE;
				for (int i = 0; i < o_UPLINKQueueNumEl; i++) {
					int current_pos = (tail + i) % O_RS485_MAX_QUEUE_SIZE;
					LOG_DEBUG("[%d] %s", current_pos, o_UPLINKQueue[current_pos]);
				}
				LOG_DEBUG("---------------------------------------------");
			}
			state = STATE_WAITTING;
			break;
		}
		case STATE_BTN_PRESS_5S: // RESET SYSTEM - ten tam thoi la press 5s
		{
			if(o_outputLedType == LED_CODE_BLINK_5HZ){
				o_outputLedType = LED_CODE_OFF;
			}else{
				o_outputLedType = LED_CODE_BLINK_5HZ;
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
