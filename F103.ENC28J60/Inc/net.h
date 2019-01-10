#ifndef __NET_H
#define __NET_H
//--------------------------------------------------
#include "stm32f1xx_hal.h"
#include <string.h> 
#include <stdlib.h> 
#include <stdint.h>
#include "enc28j60.h"
//--------------------------------------------------
#define IP_ADDR {192,168,0,197}
//--------------------------------------------------
// Ethernet II framing : 64-1518 bytes : https://vi.wikipedia.org/wiki/Frame_Ethernet#Ethernet_II
typedef struct enc28j60_frame{
  uint8_t addr_dest[6];  // Destination MAC Address
  uint8_t addr_src[6];   // Source MAC Address
  uint16_t type;         // Ether Type. Ex 0800 IPv4 protocol, 0806 ARP protocol
  uint8_t data[];        // Data payload. 46-1500 bytes + CRC checksum (4 bytes)
} enc28j60_frame_ptr;
//--------------------------------------------------
typedef struct arp_msg{
  uint16_t net_tp;
  uint16_t proto_tp;
  uint8_t macaddr_len;
  uint8_t ipaddr_len;
  uint16_t op;
  uint8_t macaddr_src[6];
  uint8_t ipaddr_src[4];
  uint8_t macaddr_dst[6];
  uint8_t ipaddr_dst[4];
} arp_msg_ptr;  // ARP message format : http://www.tcpipguide.com/free/t_ARPMessageFormat.htm
//--------------------------------------------------
typedef struct ip_pkt{
  uint8_t verlen;
  uint8_t ts;
  uint16_t len;
  uint16_t id;
  uint16_t fl_frg_of;
  uint8_t ttl;
  uint8_t prt;
  uint16_t cs;
  uint8_t ipaddr_src[4];
  uint8_t ipaddr_dst[4];
  uint8_t data[];
} ip_pkt_ptr; // IP Packet format : https://www.tutorialspoint.com/ipv4/ipv4_packet_structure.htm
//--------------------------------------------------
typedef struct icmp_pkt{
  uint8_t msg_tp;
  uint8_t msg_cd;
  uint16_t cs;
  uint16_t id;
  uint16_t num;
  uint8_t data[];
} icmp_pkt_ptr; // ICMP Packet format : https://www.frozentux.net/iptables-tutorial/chunkyhtml/x281.html
//--------------------------------------------------
#define be16toword(a) ((((a)>>8)&0xff)|(((a)<<8)&0xff00)) // Convert to big endian
#define ETH_ARP be16toword(0x0806)
#define ETH_IP be16toword(0x0800)
//--------------------------------------------------
#define ARP_ETH be16toword(0x0001)  //Hardware type in arp_msg struct : Ethernet 10Mb
#define ARP_IP be16toword(0x0800)  // Protocol type in arp_msg struct : IPv4 (4 bytes address)
#define ARP_REQUEST be16toword(1)  // Opcode in arp_msg struct :request
#define ARP_REPLY be16toword(2)    // Opcode in arp_msg struct :reply
//--------------------------------------------------
#define IP_ICMP 1
#define IP_TCP 6
#define IP_UDP 17
//--------------------------------------------------
#define ICMP_REQ 8
#define ICMP_REPLY 0
//--------------------------------------------------
uint8_t arp_read(enc28j60_frame_ptr *frame, uint16_t len);
void arp_send(enc28j60_frame_ptr *frame);
uint8_t icmp_read(enc28j60_frame_ptr *frame, uint16_t len);
uint8_t ip_read(enc28j60_frame_ptr *frame, uint16_t len);
uint8_t ip_send(enc28j60_frame_ptr *frame, uint16_t len);
void eth_send(enc28j60_frame_ptr *frame, uint16_t len);
void eth_read(enc28j60_frame_ptr *frame, uint16_t len);
void net_ini(void);
void net_poll(void);
#endif /* __NET_H */
