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
