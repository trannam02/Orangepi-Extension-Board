/*
 * knx.h
 *
 *  Created on: Mar 18, 2026
 *      Author: Nam
 */

#ifndef INC_KNX_H_
#define INC_KNX_H_

#include <stdint.h>

uint8_t knx_checksum(uint8_t *data, uint8_t len);
uint8_t encode_knx_tpuart(uint8_t* raw_knx, uint8_t raw_len, uint8_t* encoded_buffer);

#define KNX_MAX_TX_LEN 64
#endif /* INC_KNX_H_ */
