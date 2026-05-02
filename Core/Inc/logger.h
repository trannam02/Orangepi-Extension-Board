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

typedef enum {
    LOG_SPEC_LEVEL_NONE  = 0,
    LOG_SPEC_LEVEL_ERROR = 1,
    LOG_SPEC_LEVEL_WARN  = 2,
    LOG_SPEC_LEVEL_INFO  = 3,
    LOG_SPEC_LEVEL_DEBUG = 4
} LogLevelSpecial_t;

typedef enum {
	LOG_CODE_CHUA_BIET_DEFINE_GI = 0,

	LOG_CODE_COUPLER_X_TIMEOUT, // coupler id
	LOG_CODE_COUPLER_X_DISCONNECT, // coupler id

	LOG_CODE_SYSTEM_STATE_RUNNING,
	LOG_CODE_SYSTEM_STATE_CONFIG,

	LOG_CODE_DOWNLINK_DROP_PACKAGE_WITH_LENGTH, // num is length

	LOG_CODE_DOWNLINK_CRC_FAIL_RECV_CALC, // first 16 byte is received crc, second 16 byte is calc crc
	LOG_CODE_RS485_CRC_FAIL_RECV_CALC,	// first 16 byte is received crc, second 16 byte is calc crc
	LOG_CODE_KNX_CHECKSUM_FAIL_RECV_CALC,	// first 16 byte is received crc, second 16 byte is calc crc

	LOG_CODE_RS485_DROP_PACKAGE_WITH_LENGTH, // num is length
	LOG_CODE_RS485_RECEIVE_ACK_OF_COUPLER_X, // coupler id
	LOG_CODE_RS485_RECEIVE_RESPONSE_OF_COUPLER_X, // coupler id

	LOG_CODE_KNX_DROP_PACKAGE_WITH_LENGTH, // num is length

	LOG_CODE_DOWNLINK_ROUTE_TO_RS485,
	LOG_CODE_DOWNLINK_ROUTE_TO_KNX,

	LOG_CODE_BUTTON_CONTINUE_PRESS_1S,

	LOG_CODE_QUEUE_PUSH_FAIL,
	LOG_CODE_QUEUE_RS485_PUSH_FAIL,
	LOG_CODE_QUEUE_KNX_PUSH_FAIL,
	LOG_CODE_QUEUE_UPLINK_PUSH_FAIL,


} LogCode_t;

// Biến toàn cục lưu level hiện tại (được khai báo bên .c)
extern LogLevel_t system_log_level;
extern LogLevelSpecial_t system_log_level_special;
// --- HÀM KHỞI TẠO & CÀI ĐẶT ---
void Log_Init(LogLevel_t level);
void Log_SetLevel(LogLevel_t level);
void Log_Spec_Init(LogLevel_t level);
void Log_Spec_SetLevel(LogLevel_t level);
// --- HÀM XỬ LÝ CHÍNH (INTERNAL) ---
void Log_Write(LogLevel_t level, const char *format, ...);
void Log_Special(LogLevelSpecial_t level, LogCode_t code, uint32_t number);

// --- MACRO ĐỂ GỌI CHO TIỆN ---
// Dùng các macro này trong code của bạn
#define LOG_ERROR(...) Log_Write(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_WARN(...)  Log_Write(LOG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_INFO(...)  Log_Write(LOG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_DEBUG(...) Log_Write(LOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_SPEC_ERROR(code, number) Log_Special(LOG_SPEC_LEVEL_ERROR, (code), (number))
#define LOG_SPEC_WARN(code, number)  Log_Special(LOG_SPEC_LEVEL_WARN,  (code), (number))
#define LOG_SPEC_INFO(code, number)  Log_Special(LOG_SPEC_LEVEL_INFO,  (code), (number))
#define LOG_SPEC_DEBUG(code, number) Log_Special(LOG_SPEC_LEVEL_DEBUG, (code), (number))

#endif /* INC_LOGGER_H_ */
