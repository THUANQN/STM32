/*
 * udp.h
 *
 *  Created on: Jul 8, 2018
 *      Author: THUANQN
 */

#ifndef UDP_H_
#define UDP_H_
//--------------------------------------------------
#define LOCAL_PORT 333
//--------------------------------------------------
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "enc28j60.h"
#include "net.h"
//--------------------------------------------------
typedef struct udp_pkt {
uint16_t port_src;// cong cua nguoi gui
uint16_t port_dst;// cong cua nguoi nhan
uint16_t len;// chieu dai
uint16_t cs;// header checksum
uint8_t data[];// Data
} udp_pkt_ptr;
//--------------------------------------------------
uint8_t udp_read(enc28j60_frame_ptr *frame, uint16_t len);
uint8_t udp_reply(enc28j60_frame_ptr *frame, uint16_t len);
uint8_t udp_send(uint8_t *ip_addr, uint16_t port);

#endif /* UDP_H_ */
