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
    for (i = 0; i < (raw_len - 1); i++) {
        encoded_buffer[idx++] = 0x80 + i;       // Byte Control (0x80 + index)
        encoded_buffer[idx++] = raw_knx[i];     // Byte Data
    }
    encoded_buffer[idx++] = 0x40 + i;           // Byte Control cuối (0x40 + index)
    encoded_buffer[idx++] = raw_knx[i];         // Byte Data cuối
    return idx;
}
