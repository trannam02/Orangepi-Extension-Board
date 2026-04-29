#include "fsm_output_processing.h"
#include "fsm_input_processing.h"
#include "queue_utils.h"

extern TIM_HandleTypeDef htim1;

// Khai báo extern vì các Queue này đã được cấp phát ở fsm_input_processing.c
extern MessageQueue_t o_UPLINKQueue;
extern MessageQueue_t o_RS485Queue;
extern MessageQueue_t o_KNXQueue;

static uint8_t txData[MAX_BUFFER_LEN];
uint8_t o_outputLedType = 0;
BuzzerCode_t o_outputBuzzerType = 0;

static uint8_t knx_tx_buffer[KNX_MAX_TX_LEN];
static uint8_t knx_tx_length = 0;             // Tổng chiều dài gói tin
static uint8_t knx_tx_index = 0;              // Vị trí byte đang gửi
static uint8_t knx_tx_is_sending = 0;             // Cờ trạng thái (1 = đang bận gửi)

static uint8_t processed_uplink = 0;
static uint8_t processed_rs485 = 0;
static uint8_t processed_knx = 0;
// Function prototype
void outputLed();
void outputBuzzer();
void outputRS485();
void outputKNX();
void outputUPLINK();

void output_processing_init() {
    HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
    o_outputBuzzerType = BUZZER_CODE_OFF;
}

void output_processing_run() {
	processed_uplink = 0;
	processed_rs485 = 0;
	processed_knx = 0;

    outputKNX();
    outputRS485();
    outputUPLINK();
    outputLed();
    outputBuzzer();
}

void outputUPLINK() {
	static uint8_t dma_rs485_buffer[MAX_BUFFER_LEN];
    while(processed_uplink < CONCURENCY_RATE+5){
    	if (o_UPLINKQueue.count > 0 && huart2.gState == HAL_UART_STATE_READY) {
    	        if (Queue_Pop(&o_UPLINKQueue, dma_rs485_buffer)) {
    	            uint8_t len = dma_rs485_buffer[0];
    	            if (len > 0) {
//    	                LOG_INFO("DMA Transmit UPLINK Started: len=%d", len);
    	                HAL_UART_Transmit_DMA(&huart2, &dma_rs485_buffer[1], len);
    	            }
    	        }
    	    }
    	processed_uplink++;
    }
}

void knx_fsm_send_next_chunk(void) {
    if (!knx_tx_is_sending) return;

    uint8_t remaining = knx_tx_length - knx_tx_index;

    if (remaining > 0) {
        uint8_t chunk_size = (remaining >= 2) ? 2 : 1;

        HAL_UART_Transmit_DMA(&huart3, &knx_tx_buffer[knx_tx_index], chunk_size);
        LOG_WARN("KNX transmit chunk: %02X %02X", knx_tx_buffer[knx_tx_index], knx_tx_buffer[knx_tx_index+1]);
        knx_tx_index += chunk_size;

        if (knx_tx_index < knx_tx_length) {
        	clearTimer(3);
            setTimer(3, 5*TICK);
        } else {
        	knx_tx_is_sending = 0;
            LOG_WARN("KNX TX FSM Complete (%d bytes)", knx_tx_length);
        }
    }
}

void outputKNX() {
    if (o_KNXQueue.count > 0 && !knx_tx_is_sending && huart3.gState == HAL_UART_STATE_READY) {
        if (Queue_Pop(&o_KNXQueue, txData)) {
            uint8_t len = txData[0];
            if (len > 0 && len < KNX_MAX_TX_LEN) {
            	memcpy(knx_tx_buffer, &txData[1], len);
            	knx_tx_length = len;
            	knx_tx_index = 0;
            	knx_tx_is_sending = 1;
            	triggerTimerNow(3);
            }
        }
    }
    if(getTimer(3)){
    	knx_fsm_send_next_chunk();
    	clearTimer(3);
    	setTimer(3, 5*TICK);
    };
}

void outputRS485() {
	static uint8_t dma_rs485_buffer[MAX_BUFFER_LEN];
	while(processed_rs485 < CONCURENCY_RATE+5){
		processed_rs485++;

		if (o_RS485Queue.count > 0 && huart1.gState == HAL_UART_STATE_READY) {

		        if (Queue_Pop(&o_RS485Queue, dma_rs485_buffer)) {
		            uint8_t len = dma_rs485_buffer[0];
		            if (len > 0) {
		                HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, 1);
		                HAL_UART_Transmit_DMA(&huart1, &dma_rs485_buffer[1], len);
		            }
		        }
		    }
	}

}
void outputLed() {
    static uint8_t led_step = 0;
    static uint8_t last_led_type = LED_CODE_OFF;

    // ==============================================================
    // [BẢO VỆ CHUYỂN TRẠNG THÁI]
    // Nếu có lệnh đổi kiểu chớp đột ngột, phải reset lại tiến trình và Timer
    // ==============================================================
    if (o_outputLedType != last_led_type) {
        led_step = 0;
        clearTimer(1);
        last_led_type = o_outputLedType;
    }

    switch(o_outputLedType) {
        case LED_CODE_OFF:
            if (led_step == 0) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
                led_step = 1;
            }
            break;

        case LED_CODE_ON:
            if (led_step == 0) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
                led_step = 1;
            }
            break;

        case LED_CODE_BLINK_1HZ:
            if (led_step == 0) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
                setTimer(1, MS(500));
                led_step = 1;
            }
            else if (led_step == 1 && getTimer(1)) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
                clearTimer(1);
                setTimer(1, MS(500));
                led_step = 2;
            }
            else if (led_step == 2 && getTimer(1)) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
                clearTimer(1);
                setTimer(1, MS(500));
                led_step = 1;
            }
            break;

        case LED_CODE_BLINK_4HZ: // 0.25Hz moi dung haha
            if (led_step == 0) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
                setTimer(1, MS(2000));
                led_step = 1;
            }
            else if (led_step == 1 && getTimer(1)) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, SET);
                clearTimer(1);
                setTimer(1, MS(2000));
                led_step = 2;
            }
            else if (led_step == 2 && getTimer(1)) {
                HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, RESET);
                clearTimer(1);
                setTimer(1, MS(2000));
                led_step = 1;
            }
            break;

        default:
            o_outputLedType = LED_CODE_OFF;
            break;
    }
}



void outputBuzzer() {
    static uint8_t buzzer_step = 0;
    static BuzzerCode_t last_buzzer_type = BUZZER_CODE_OFF;

    // [Bảo vệ]: Nếu có lệnh đổi kiểu kêu đột ngột, phải reset lại tiến trình
    if (o_outputBuzzerType != last_buzzer_type) {
        buzzer_step = 0;
        setBuzzerPWM(0); // Tắt còi ngay lập tức
        clearTimer(5);
        last_buzzer_type = o_outputBuzzerType;
    }

    switch(o_outputBuzzerType) {
        case BUZZER_CODE_OFF:
        	setBuzzerPWM(0);
            break;

        case BUZZER_CODE_FOR_CONNECTED_COUPLER:
            // Kịch bản: Kêu 2kHz (100ms) -> Nghỉ (100ms) -> Kêu 2kHz (100ms) -> Tự tắt
            if (buzzer_step == 0) {
                setBuzzerPWM(3000);
                setTimer(5, MS(100));
                buzzer_step = 1;
            }
            else if (buzzer_step == 1 && getTimer(5)) {
                setBuzzerPWM(0);
                clearTimer(5);
                setTimer(5, MS(100));
                buzzer_step = 2;
            }
            else if (buzzer_step == 2 && getTimer(5)) {
                setBuzzerPWM(3000);
                clearTimer(5);
                setTimer(5, MS(100));
                buzzer_step = 3;
            }
            else if (buzzer_step == 3 && getTimer(5)) {
                setBuzzerPWM(0);
                o_outputBuzzerType = BUZZER_CODE_OFF; // Kêu xong thì tự trả về trạng thái OFF
            }
            break;
        case BUZZER_CODE_FOR_DISCONNECTED_COUPLER:
			if (buzzer_step == 0) {
				setBuzzerPWM(3000);
				setTimer(5, MS(100));
				buzzer_step = 1;
			}
			else if (buzzer_step == 1 && getTimer(5)) {
				setBuzzerPWM(0);
				o_outputBuzzerType = BUZZER_CODE_OFF; // Kêu xong tự tắt
			}
			break;
        case BUZZER_CODE_FOR_BEEP_1S:
            if (buzzer_step == 0) {
                setBuzzerPWM(2000);
                setTimer(5, MS(200));
                buzzer_step = 1;
            }
            else if (buzzer_step == 1 && getTimer(5)) {
                setBuzzerPWM(0);
                o_outputBuzzerType = BUZZER_CODE_OFF; // Kêu xong tự tắt
            }
            break;

        case BUZZER_CODE_ALARM:
            // Kịch bản: Kêu tít tít lặp đi lặp lại vô hạn (Giống chớp LED)
        	setBuzzerPWM(250);
            break;

        default:
            o_outputBuzzerType = BUZZER_CODE_OFF;
            break;
    }
}
void setBuzzerPWM(uint16_t freq) {
    if (freq == 0) {
        // Tần số 0 -> Tắt còi
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    } else {
        // Tính toán thanh ghi ARR để ra đúng tần số
        uint32_t arr_value = 1000000 / freq;

        __HAL_TIM_SET_AUTORELOAD(&htim1, arr_value);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, arr_value / 2); // Duty 50%

        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    }
}
