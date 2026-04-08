/*
 * knx.c
 *
 *  Created on: Mar 18, 2026
 *      Author: Nam
 */

#include <knx.h>

// Hàm tính Checksum chuẩn của KNX
uint8_t knx_checksum(uint8_t *data, uint8_t len) {
    uint8_t result = 0;
    for (int i = 0; i < len; i++) {
        result ^= data[i];
    }
    return ~result;
}

// Hàm mã hóa gói tin Raw KNX sang định dạng TP-UART cho NCN5130
// Trả về số byte sau khi đã mã hóa (sẽ gấp đôi số byte gốc)
uint8_t encode_knx_tpuart(uint8_t* raw_knx, uint8_t raw_len, uint8_t* encoded_buffer) {
    uint8_t idx = 0;
    int i;

    // 1. Đóng gói toàn bộ byte Data với prefix 0x80
    for (i = 0; i < raw_len - 1; i++) {
        encoded_buffer[idx++] = 0x80 + i;       // Byte Control (0x80 + index)
        encoded_buffer[idx++] = raw_knx[i];     // Byte Data
    }

    // 2. Tự động tính Checksum cho chuỗi raw_knx
//    uint8_t checksum = knx_checksum(raw_knx, raw_len);

    // 3. Đóng gói byte Checksum ở cuối cùng với prefix 0x40
    encoded_buffer[idx++] = 0x40 + i;           // Byte Control kết thúc (0x40 + index)
    encoded_buffer[idx++] = raw_knx[i];           // Byte Checksum

    return idx;
}




