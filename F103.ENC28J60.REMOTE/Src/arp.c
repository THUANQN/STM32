/*
 * arp.c
 *
 *  Created on: Jul 5, 2018
 *      Author: THUANQN
 */
#include "arp.h"
//--------------------------------------------------
extern UART_HandleTypeDef huart1;
extern uint8_t ipaddr[4];
extern uint8_t ipgate[4];
extern uint8_t ipmask[4];
extern uint8_t macaddr[6];
extern char str1[60];
extern uint32_t clock_cnt;
extern uint8_t net_buf[ENC28J60_MAXFRAME];
uint8_t macbroadcast[6]=MAC_BROADCAST;
uint8_t macnull[6]=MAC_NULL;
//--------------------------------------------------
arp_record_ptr arp_rec[5];
uint8_t current_arp_index=0;
extern USART_prop_ptr usartprop;
//--------------------------------------------------
/**
 * @Description : Read ARP message from frame ethernet (Ethernet II)
 * @Use         : Before use, recommend check Ethernet Type of frame ethernet is ARP protocol 0806
 * @Input       : *frame pointer type @enc28j60_frame_ptr store
 *                len    length of ARP messgae. Note : ARP message no data
 * @Output      :res  1 If ARP message received have opcode is ARP Request and Destination IP Address is ipaddr (192.168.0.197)
 *                    2 If ARP message received have opcode is ARP Reply and Destination IP Address is ipaddr (192.168.0.197)
 *                    0 Else
 */
uint8_t arp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
  uint8_t res=0;
  arp_msg_ptr *msg=(void*)(frame->data);
  if (len>=sizeof(arp_msg_ptr))
  {
    if ((msg->net_tp==ARP_ETH)&&(msg->proto_tp==ARP_IP))
    {
	   if (!memcmp(msg->ipaddr_dst,ipaddr,4)) // loc thong diep cac arp gui den dia chi ethernet shield enc28j60
	   {
	      if (msg->op==ARP_REQUEST) // noi dung thong diep arp la request
          {
	    	// Toi uu khong cho xuat lenh nhieu tiet kiem thoi gian thuc thi nen ta comment
//	        sprintf(str1,"request\r\nmac_src %02X:%02X:%02X:%02X:%02X:%02X\r\n",
//              msg->macaddr_src[0],msg->macaddr_src[1],msg->macaddr_src[2],msg->macaddr_src[3],msg->macaddr_src[4],msg->macaddr_src[5]);
//              HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//            sprintf(str1,"ip_src %d.%d.%d.%d\r\n",
//              msg->ipaddr_src[0],msg->ipaddr_src[1],msg->ipaddr_src[2],msg->ipaddr_src[3]);
//              HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//            sprintf(str1,"mac_dst %02X:%02X:%02X:%02X:%02X:%02X\r\n",msg->macaddr_dst[0],msg->macaddr_dst[1],msg->macaddr_dst[2],msg->macaddr_dst[3],msg->macaddr_dst[4],msg->macaddr_dst[5]);
//              HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//            sprintf(str1,"ip_dst %d.%d.%d.%d\r\n",
//              msg->ipaddr_dst[0],msg->ipaddr_dst[1],msg->ipaddr_dst[2],msg->ipaddr_dst[3]);
//              HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
            res=1; // Detect arp message request
         }
	      else if(msg->op==ARP_REPLY) // noi dung thong diep arp la reply
	      {
	        sprintf(str1,"\r\nreply\r\nmac_src %02X:%02X:%02X:%02X:%02X:%02X\r\n",msg->macaddr_src[0],msg->macaddr_src[1],msg->macaddr_src[2],
	          msg->macaddr_src[3],msg->macaddr_src[4],msg->macaddr_src[5]);
	        HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	        sprintf(str1,"ip_src %d.%d.%d.%d\r\n",msg->ipaddr_src[0],msg->ipaddr_src[1],msg->ipaddr_src[2],msg->ipaddr_src[3]);
	        HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	        sprintf(str1,"mac_dst %02X:%02X:%02X:%02X:%02X:%02X\r\n",msg->macaddr_dst[0],msg->macaddr_dst[1],msg->macaddr_dst[2],
	          msg->macaddr_dst[3],msg->macaddr_dst[4],msg->macaddr_dst[5]);
	        HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	        sprintf(str1,"ip_dst %d.%d.%d.%d\r\n",msg->ipaddr_dst[0],msg->ipaddr_dst[1],msg->ipaddr_dst[2],msg->ipaddr_dst[3]);
	        HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	        res=2;
	      }
	   }
    }
  }
	return res;
}
//--------------------------------------------------
/**
 * @Description : Send ARP message to frame ethernet (Ethernet II) with opcode is ARP_REPLY
 * @Use         : Before use, recommend check Ethernet Type of frame ethernet is ARP protocol 0806
 * @Input       : *frame pointer type @enc28j60_frame_ptr store data you want to send
 * @Output      : None
 */
void arp_send(enc28j60_frame_ptr *frame)
{
  arp_msg_ptr *msg = (void*)frame->data;
  msg->op = ARP_REPLY;
  memcpy(msg->macaddr_dst,msg->macaddr_src,6);
  memcpy(msg->macaddr_src,macaddr,6);
  memcpy(msg->ipaddr_dst,msg->ipaddr_src,4);
  memcpy(msg->ipaddr_src,ipaddr,4);
  memcpy(frame->addr_dest,frame->addr_src,6); // Chen them
  eth_send(frame,sizeof(arp_msg_ptr));
  sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X(%d.%d.%d.%d)-",frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],
    msg->ipaddr_dst[0],msg->ipaddr_dst[1],msg->ipaddr_dst[2],msg->ipaddr_dst[3]);
  HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X(%d.%d.%d.%d) arp request\r\n",frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
    msg->ipaddr_src[0],msg->ipaddr_src[1],msg->ipaddr_src[2],msg->ipaddr_src[3]);
  HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
}
//--------------------------------------------------
/**
 * @Description : Request ARP message to frame ethernet (Ethernet II) with opcode is ARP_REQUEST
 * @Use         : Before use, recommend check Ethernet Type of frame ethernet is ARP protocol 0806
 * @Input       : *ip_addr pointer store IP address you want to request from ENC28J60 module
 * @Output      : 0 If this IP address exist in the arp table fill
 *                1 If not, and send to request ARP message
 */
uint8_t arp_request(uint8_t *ip_addr)
{
  uint8_t i,j;
  uint8_t ip[4];
  uint8_t iptemp = 0;
  for(i=0;i<4;i++)
  {
    iptemp += (ip_addr[i] ^ ipaddr[i]) & ipmask[i];
  }
  enc28j60_frame_ptr *frame=(void*)net_buf;
  // Kiem tra xem dia chi co thuoc mang cuc bo hay khong
  if( iptemp == 0 ) memcpy(ip,ip_addr,4);
  else memcpy(ip,ipgate,4);
  // Kiem tra xem dia chi da ton tai trong bang ARP hay chua va da qua thoi gian
  for(j=0;j<5;j++)
  {
	  if ((clock_cnt-arp_rec[j].sec)>43200)
	  {
	     memset(arp_rec+(sizeof(arp_record_ptr)*j),0,sizeof(arp_record_ptr));// Neu qua gio se xoa het dua ve null
	  }
	  if (!memcmp(arp_rec[j].ipaddr,ip_addr,4))
	  {
		  // Nhin vao bang ARP
		  for(i=0;i<5;i++)
		  {
		    sprintf(str1,"%d.%d.%d.%d - %02X:%02X:%02X:%02X:%02X:%02X -%lu\r\n",arp_rec[i].ipaddr[0],arp_rec[i].ipaddr[1],arp_rec[i].ipaddr[2],arp_rec[i].ipaddr[3],
		      arp_rec[i].macaddr[0],arp_rec[i].macaddr[1],arp_rec[i].macaddr[2],arp_rec[i].macaddr[3],arp_rec[i].macaddr[4],arp_rec[i].macaddr[5],(unsigned long)arp_rec[i].sec);
		    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
		  }
		  memcpy(frame->addr_dest,arp_rec[j].macaddr,6); // Cai vao de lay duoc dia chi khi arp request
		  if((usartprop.is_ip==3)||(usartprop.is_ip==5))//Trang thai gui goi ARP hoac ICMP
		  {
		    net_cmd();
		  }
	      return 0;
	  }
  }
  // Noi dung chinh request
  //enc28j60_frame_ptr *frame=(void*)net_buf;
  arp_msg_ptr *msg = (void*)frame->data;
  msg->net_tp = ARP_ETH;
  msg->proto_tp = ARP_IP;
  msg->macaddr_len = 6;
  msg->ipaddr_len = 4;
  msg->op = ARP_REQUEST;
  memcpy(msg->macaddr_src,macaddr,6);
  memcpy(msg->ipaddr_src,ipaddr,4);
  memcpy(msg->macaddr_dst,macnull,6);
  memcpy(msg->ipaddr_dst,ip_addr,4);
  memcpy(frame->addr_dest,macbroadcast,6);
  memcpy(frame->addr_src,macaddr,6);
  frame->type = ETH_ARP;
  enc28j60_packetSend((void*)frame,sizeof(arp_msg_ptr) + sizeof(enc28j60_frame_ptr));
  return 1;
}
//--------------------------------------------------
/**
 * @Description : Collect IP and MAC Correspond when reveived ARP_REPLAY opcode from ARP message of other machine in network
 * @Input       : *frame pointer type @enc28j60_frame_ptr store
 * @Output      : None
 * @Note        : arp table is only 5 elements.
 */
void arp_table_fill(enc28j60_frame_ptr *frame)
{
  uint8_t i;
  arp_msg_ptr *msg = (void*)frame->data;
  memcpy(arp_rec[current_arp_index].ipaddr,msg->ipaddr_src,4);
  memcpy(arp_rec[current_arp_index].macaddr,msg->macaddr_src,6);
  arp_rec[current_arp_index].sec = clock_cnt;
  if(current_arp_index<4) current_arp_index++;
  else current_arp_index=0;
  // In bang ARP
  for(i=0;i<5;i++)
  {
    sprintf(str1,"%d.%d.%d.%d - %02X:%02X:%02X:%02X:%02X:%02X -%lu\r\n",arp_rec[i].ipaddr[0],arp_rec[i].ipaddr[1],arp_rec[i].ipaddr[2],arp_rec[i].ipaddr[3],arp_rec[i].macaddr[0],arp_rec[i].macaddr[1],arp_rec[i].macaddr[2],
      arp_rec[i].macaddr[3],arp_rec[i].macaddr[4],arp_rec[i].macaddr[5],(unsigned long)arp_rec[i].sec);
    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  }
}
//--------------------------------------------------
