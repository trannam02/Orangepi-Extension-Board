/*
 * logger.c
 *
 *  Created on: Feb 8, 2026
 *      Author: Nam
 */

#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "queue_utils.h"
#include "fsm_input_processing.h"

// Buffer đệm để chứa chuỗi sau khi format
// Tăng lên nếu bạn log những dòng rất dài
#define LOG_BUFFER_SIZE 256

LogLevel_t system_log_level = LOG_LEVEL_INFO; // Mặc định là INFO
void Log_Init(LogLevel_t level) {
    system_log_level = level;
}
void Log_SetLevel(LogLevel_t level) {
    system_log_level = level;
}


LogLevelSpecial_t system_log_level_special = LOG_SPEC_LEVEL_INFO; // Mặc định là INFO

void Log_Spec_Init(LogLevel_t level) {
	system_log_level_special = level;
}
void Log_Spec_SetLevel(LogLevel_t level) {
	system_log_level_special = level;
}

static uint8_t buffer_spec[11] = {0,0x0E,0x04,0,0,0,0,0,0,0,0};

void Log_Special(LogLevelSpecial_t level, LogCode_t code, uint32_t number){
	if (level > system_log_level_special) {
		return;
	}
	// 0E 04 LEVEL (1 byte) CODE(2 byte) NUM (uint32_t) CRC8
	buffer_spec[0] = 10;
//	buffer_spec[1] = 0x0E;
//	buffer_spec[2] = 0x04;
	buffer_spec[3] = level;

	buffer_spec[4] = (code >> 8);
	buffer_spec[5] = (code & 0x00FF);

	buffer_spec[6] = (uint8_t)((number & 0xFF000000) >> 24);
	buffer_spec[7] = (uint8_t)((number & 0x00FF0000) >> 16);
	buffer_spec[8] = (uint8_t)((number & 0x0000FF00) >> 8);
	buffer_spec[9] = (uint8_t)((number & 0x000000FF));

	buffer_spec[10] = crc8(&buffer_spec[1], 9);

 	Queue_Push(&o_UPLINKQueue, buffer_spec, buffer_spec[0] + 1);
}

void Log_Write(LogLevel_t level, const char *format, ...) {
    // 1. Kiểm tra Level: Nếu level của log thấp hơn cài đặt thì bỏ qua
    // Ví dụ: Đang set WARN (2) mà gọi INFO (3) -> 3 > 2 -> Bỏ qua
    if (level > system_log_level) {
        return;
    }

    // 2. Chuẩn bị các biến
    char buffer[LOG_BUFFER_SIZE];
    uint16_t len = 1;
    va_list args;

    // 3. Thêm tiền tố (Prefix) cho đẹp (Tùy chọn)
    // Format: [TICK] [LEVEL] Message
    switch (level) {
        case LOG_LEVEL_ERROR: len += snprintf(buffer + len, LOG_BUFFER_SIZE - len, "[%lu] [ERR] ", HAL_GetTick()); break;
        case LOG_LEVEL_WARN:  len += snprintf(buffer + len, LOG_BUFFER_SIZE - len, "[%lu] [WRN] ", HAL_GetTick()); break;
        case LOG_LEVEL_INFO:  len += snprintf(buffer + len, LOG_BUFFER_SIZE - len, "[%lu] [INF] ", HAL_GetTick()); break;
        case LOG_LEVEL_DEBUG: len += snprintf(buffer + len, LOG_BUFFER_SIZE - len, "[%lu] [DBG] ", HAL_GetTick()); break;
        default: break;
    }

    // 4. Xử lý chuỗi biến đổi (tương tự printf)
    va_start(args, format);
    // vsnprintf an toàn hơn vsprintf vì nó kiểm soát độ dài buffer
    len += vsnprintf(buffer + len, LOG_BUFFER_SIZE - len, format, args);
    va_end(args);

    // 5. Thêm xuống dòng (\r\n) nếu chưa có
    if (len < LOG_BUFFER_SIZE - 2) {
        buffer[len++] = '\r';
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    buffer[0] = len - 1;
    // 6. Gửi ra UART (Blocking Mode như bạn yêu cầu)
    Queue_Push(&o_UPLINKQueue, buffer, buffer[0] + 1);
//    HAL_UART_Transmit(LOG_UART, (uint8_t*)buffer, len, HAL_MAX_DELAY);
}
