/*
 * logger.h
 *
 *  Created on: Feb 8, 2026
 *      Author: Nam
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_



#include "main.h" // Để lấy định nghĩa UART_HandleTypeDef

// --- CẤU HÌNH ---
// Chọn UART bạn muốn dùng để Log
extern UART_HandleTypeDef huart2;
#define LOG_UART &huart2

// --- ĐỊNH NGHĨA CÁC LEVEL ---
typedef enum {
    LOG_LEVEL_NONE  = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4
} LogLevel_t;

// Biến toàn cục lưu level hiện tại (được khai báo bên .c)
extern LogLevel_t system_log_level;

// --- HÀM KHỞI TẠO & CÀI ĐẶT ---
void Log_Init(LogLevel_t level);
void Log_SetLevel(LogLevel_t level);

// --- HÀM XỬ LÝ CHÍNH (INTERNAL) ---
void Log_Write(LogLevel_t level, const char *format, ...);

// --- MACRO ĐỂ GỌI CHO TIỆN ---
// Dùng các macro này trong code của bạn
#define LOG_ERROR(...) Log_Write(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WARN(...)  Log_Write(LOG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_INFO(...)  Log_Write(LOG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_DEBUG(...) Log_Write(LOG_LEVEL_DEBUG, __VA_ARGS__)


#endif /* INC_LOGGER_H_ */
