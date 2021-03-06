/*
 * udp.c
 *
 *  Created on: Jul 8, 2018
 *      Author: THUANQN
 */
#include "udp.h"
//--------------------------------------------------
extern UART_HandleTypeDef huart1;
extern char str1[60];
//--------------------------------------------------
/**
 * @Description : Read UDP packet to frame ethernet and Reply UDP Packet
 * @Use         : Before use, recommend check Ethernet Type of frame ethernet is IP protocol 0800 and Protocol of IP Packet is UDP
 * @Input       : *frame pointer type @enc28j60_frame_ptr store frame
 *                len length data of ip_pkt_ptr structure
 * @Output      : res
 */
uint8_t udp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
  uint8_t res=0;
  sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; ip\r\n",
    frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],
    frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
    frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],
    frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],len);
  HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  ip_pkt_ptr *ip_pkt = (void*)(frame->data);
  sprintf(str1,"%d.%d.%d.%d-%d.%d.%d.%d udp request\r\n",
    ip_pkt->ipaddr_src[0],ip_pkt->ipaddr_src[1],ip_pkt->ipaddr_src[2],ip_pkt->ipaddr_src[3],
    ip_pkt->ipaddr_dst[0],ip_pkt->ipaddr_dst[1],ip_pkt->ipaddr_dst[2],ip_pkt->ipaddr_dst[3]);
  HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  udp_pkt_ptr *udp_pkt = (void*)(ip_pkt->data);
  sprintf(str1,"%u-%u\r\n", be16toword(udp_pkt->port_src),be16toword(udp_pkt->port_dst));
  HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  HAL_UART_Transmit(&huart1,udp_pkt->data,len - sizeof(udp_pkt_ptr),0x1000);
  HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,0x1000);
  // Reply
  udp_reply(frame,len);
  return res;
}
//--------------------------------------------------
/**
 * @Description : Reply UDP Packet to frame
 * @Use         : Before use, recommend check Ethernet Type of frame ethernet is IP protocol 0800 and Protocol of IP Packet is UDP
 * @Input       : *frame pointer type @enc28j60_frame_ptr store frame you want to send
 *                len length data of ip_pkt_ptr structure
 * @Output      : res
 */
uint8_t udp_reply(enc28j60_frame_ptr *frame, uint16_t len)
{
  uint8_t res=0;
  uint16_t port;
  ip_pkt_ptr * ip_pkt = ( void *) (frame-> data);
  udp_pkt_ptr * udp_pkt = ( void *) (ip_pkt-> data);
  port = udp_pkt-> port_dst ;
  udp_pkt-> port_dst = udp_pkt-> port_src;
  udp_pkt-> port_src = port;
  strcpy((char*)udp_pkt->data,"UDP Reply:\r\nHello from UDP Server to UDPClient!!!\r\n");
  len = strlen((char*)udp_pkt->data) + sizeof(udp_pkt_ptr);
  udp_pkt->len = be16toword(len);
  udp_pkt->cs=0;
  udp_pkt->cs=checksum((uint8_t*)udp_pkt-8, len+8, 1);
  ip_send(frame,len+sizeof(ip_pkt_ptr));
  return res;
}
//--------------------------------------------------
