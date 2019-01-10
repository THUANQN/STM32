#include "net.h"
// Tham khao website http://www.omnisecu.com/tcpip/index.php
//--------------------------------------------------
extern UART_HandleTypeDef huart1;
uint8_t net_buf[ENC28J60_MAXFRAME];
extern uint8_t macaddr[6];
uint8_t ipaddr[4]=IP_ADDR;
char str1[60]={0};
USART_prop_ptr usartprop;
extern char str[20];
uint32_t clock_cnt=0; // so giay
//--------------------------------------------------
void net_ini(void)
{
  usartprop.usart_buf[0]=0;
  usartprop.usart_cnt=0;
  usartprop.is_ip=0;
  sprintf(str1,"Hello ENC28J60 Ethernet Shield !\r\n");
  HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  enc28j60_ini();
}
//--------------------------------------------------
// Reference: https://www.youtube.com/watch?v=dXartoyj2ow
// EX : Calculate checksum IPv4 Packet
//  |0 Version 3||4 IHL  7||8 DSCP 13||14 ECN 15||16        Total Length         31|
//  |0             Identification             15||16 Flag 18||19 Fragment Offset 31|
//  |0   Time to Live   7||8    Protocol      15||16      Header Checksum        31|
//  |0                             Source IP Address                             31|
//  |0                          Destination IP Address                           31|
//  |0                             Option + Padding                              31|
//  Value
//  |0   0100  3||4 0101 7||8    00000000     15||16    00000000 00011100        31|  -->   01000101 00000000 + 00000000 00011100
//  |0           00000000 00000001            15||16 000  18||19 00000 00000000  31|  --> + 00000000 00000001 + 00000000 00000000
//  |0     00000100     7||8    00010001      15||16      Header Checksum        31|  --> + 00000100 00010001
//  |0                               10.12.14.5                                  31|  --> + 00001010 00001100 + 00001110 00000101
//  |0                                 12.6.7.9                                  31|  --> + 00001100 00000110 + 00000111 00001001
//  |0                             Option + Padding                              31|
// => SUM = 01110100 01001110 => Header Checksum = ~SUM = 100010111 10110001
// sum of data by word, after ~sum is header checksum
/**
 * @Description : Checksum of array data 8 bit
 * @Input       : *ptr array data 8 bit (Note: array data received is little endien)
 *                len  length of array
 * @Output      : checksum value
 */
uint16_t checksum(uint8_t *ptr, uint16_t len)
{
  uint32_t sum = 0;
	while(len>1)
  {
    sum += (uint16_t) (((uint32_t)*ptr<<8)|*(ptr+1));
	ptr+=2;
    len-=2;
  }
	if(len) sum+=((uint32_t)*ptr)<<8;
	while (sum>>16) sum=(uint16_t)sum+(sum>>16);
	return ~be16toword((uint16_t)sum); // re-convert
}
//--------------------------------------------------
/**
 * @Description : Received ICMP packet from ethernet bus and REPLY if it have REQUEST
 * @Use         : Before use, recommend check Protocol of IP packet is ICMP 1
 * @Input       : *frame pointer type @enc28j60_frame_ptr store frame
 *                len    length of Sum data is include leng of data[] + sizeof(icmp_pkt_ptr).
 * @Output       : res
 */
uint8_t icmp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
  uint8_t res=0;
  ip_pkt_ptr *ip_pkt = (void*)frame->data;
	icmp_pkt_ptr *icmp_pkt = (void*)ip_pkt->data;
	if ((len>=sizeof(icmp_pkt_ptr))&&(icmp_pkt->msg_tp==ICMP_REQ))
  {
    sprintf(str1,"icmp request\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	icmp_pkt->msg_tp=ICMP_REPLY;
    icmp_pkt->cs=0;
    icmp_pkt->cs=checksum((void*)icmp_pkt,len);
	ip_send(frame,len+sizeof(ip_pkt_ptr));
  }
  return res;
}
//--------------------------------------------------
/**
 * @Description : Received IP packet from ethernet bus
 * @Use         : Before use, recommend check Ethernet Type of frame ethernet is IP protocol 0800
 * @Input       : *frame pointer type @enc28j60_frame_ptr store frame
 *                len    length of Sum data is include leng of data[] + sizeof(ip_pkt_ptr).
 * @Output       : res
 */
uint8_t ip_read(enc28j60_frame_ptr *frame, uint16_t len)
{
  uint8_t res=0;
  uint16_t len1;
  ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	if((ip_pkt->verlen==0x45)&&(!memcmp(ip_pkt->ipaddr_dst,ipaddr,4)))
  {
    len1 = be16toword(ip_pkt->len) - sizeof(ip_pkt_ptr); //len1 = len - sizeof(ip_pkt_ptr)
    // Test be16toword(ip_pkt->len)) == len :))
//    sprintf(str1,"\r\nlen (1) = 0x%04X\r\n", be16toword(ip_pkt->len));
//    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//    sprintf(str1,"\r\nlen (2) = 0x%04X\r\n", len);

//    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//    sprintf(str1,"\r\nip_cs 0x%04X\r\n", ip_pkt->cs);
//    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//    ip_pkt->cs=0;
//    sprintf(str1,"ip_cs 0x%04X\r\n", checksum((void*)ip_pkt,sizeof(ip_pkt_ptr)));
//    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
		if (ip_pkt->prt==IP_ICMP)
    {
      icmp_read(frame,len1);
    }
    else if (ip_pkt->prt==IP_TCP)
    {

    }
    else if (ip_pkt->prt==IP_UDP)
    {

    } 
  }
  return res;
}
//--------------------------------------------------
/**
 * @Description : Send IP packet to frame ethernet
 * @Use         : Before use, recommend check Ethernet Type of frame ethernet is IP protocol 0800
 * @Input       : *frame pointer type @enc28j60_frame_ptr store data you want to send
 *                len length of Sum data is include leng of data[] + sizeof(ip_pkt_ptr).
 * @Output      : res
 */
uint8_t ip_send(enc28j60_frame_ptr *frame, uint16_t len)
{
  uint8_t res=0;
  ip_pkt_ptr *ip_pkt = (void*)frame->data;
  ip_pkt->len=be16toword(len);
  ip_pkt->fl_frg_of=0;
  ip_pkt->ttl=128;
  ip_pkt->cs = 0;
  memcpy(ip_pkt->ipaddr_dst,ip_pkt->ipaddr_src,4);
  memcpy(ip_pkt->ipaddr_src,ipaddr,4);
  ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
  eth_send(frame,len);
  return res;
}
//--------------------------------------------------
/**
 * @Description : Received frame (type Ethernet II) from ethernet bus
 * @Input       : *frame pointer type @enc28j60_frame_ptr store frame
 *                len    length of Sum data is include leng of data[] + sizeof(enc28j60_frame_ptr).
 * @Output       : None
 */
void eth_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	// Check if data payload exist
	if (len>=sizeof(enc28j60_frame_ptr))
  {
    if(frame->type==ETH_ARP)
    {
     sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; arp\r\n",
     frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
     frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5], len);
     HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
     res = arp_read(frame,len-sizeof(enc28j60_frame_ptr));
		 if (res == 1) // Ham sizeof voi struct se bo qua thanh phan mang[] nen ta chi doc kich thuoc data (payload) khong doc lai header ethernet frame
		 {
			 arp_send(frame); // Reply
		 } else if (res == 2) // Reply for other ARP send to
		 {
			 arp_table_fill(frame); // Bang nay se thu thap tat ca cac ARP reply (do ta su dung arp_request cac arp se reply )
		 }
    }
    else if (frame->type==ETH_IP)
    {
     sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; ip\r\n",
     frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
     frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5], len);
     HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	 ip_read(frame,len-sizeof(enc28j60_frame_ptr)); // Sua lai cho dung voi ly thuyet
    }
  }
}
//--------------------------------------------------
/**
 * @Description : Send frame (type Ethernet II) to ethernet bus
 * @Input       : *frame pointer type @enc28j60_frame_ptr store frame you want to send
 *                len    length of data payload (Sum is leng of data[] + sizeof(enc28j60_frame_ptr) because
 *                       struct enc28j60_frame_ptr is not include uint8_t data[])
 * @Output       : None
 */
void eth_send(enc28j60_frame_ptr *frame, uint16_t len)
{
  memcpy(frame->addr_dest,frame->addr_src,6);
  memcpy(frame->addr_src,macaddr,6);
  enc28j60_packetSend((void*)frame,len + sizeof(enc28j60_frame_ptr));
}
//--------------------------------------------------
void net_poll(void)
{
  uint16_t len;
  uint8_t ip[4]={0};
  enc28j60_frame_ptr *frame=(void*)net_buf;
	while ((len=enc28j60_packetReceive(net_buf,sizeof(net_buf)))>0)
  {
    eth_read(frame,len);
    if (usartprop.is_ip==1) //Trang thai yeu cau ARP
    {
      HAL_UART_Transmit(&huart1,usartprop.usart_buf,usartprop.usart_cnt,0x1000);
      HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,0x1000);
      ip_extract((char*)usartprop.usart_buf,usartprop.usart_cnt,ip);
      // In ra de thu nghiem  ham chuyen doi ip_extract co chinh xac khong
      //sprintf(str1,"%d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
      //HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
      arp_request(ip);
      usartprop.is_ip = 0;
      usartprop.usart_cnt=0;

    }
  }
}
//--------------------------------------------------
void UART1_RxCpltCallback(void)
{
	uint8_t b;
	b = str[0];
	// Neu chung ta vuot qua do dai bo dem
	if (usartprop.usart_cnt>20)
	{
	usartprop.usart_cnt=0;
	}
	else if (b == 'a')
	{
	usartprop.is_ip=1;//Trang thai yeu cau ARP
	}
	else
	{
	usartprop.usart_buf[usartprop.usart_cnt] = b;
	usartprop.usart_cnt++;
	}
	HAL_UART_Receive_IT(&huart1,(uint8_t*)str,1); // Nhan ngat va luu vao str[0]
}
//-----------------------------------------------
// Chuyen doi gia tri chuoi IP thanh so 32bit
void ip_extract(char* ip_str,uint8_t len, uint8_t* ipextr)
{
  uint8_t offset = 0;
  uint8_t i;
  char ss2[5] = {0};
  char *ss1;
  int ch = '.';
  for(i=0;i<3;i++)
  {
    ss1 = strchr(ip_str,ch);
    offset = ss1-ip_str+1;
    strncpy(ss2,ip_str,offset);
    ss2[offset]=0; //NULL
    ipextr[i] = atoi(ss2);
    ip_str+=offset;
    len-=offset;
  }
  strncpy(ss2,ip_str,len);
  ss2[len]=0; /// NULL
  ipextr[3] = atoi(ss2);
}
//-----------------------------------------------
void TIM_PeriodElapsedCallback(void)
{
  clock_cnt++; // Lay giay va viet chung vao clock_cnt
}
//-----------------------------------------------
