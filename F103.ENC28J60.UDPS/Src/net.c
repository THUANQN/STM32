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
uint16_t checksum(uint8_t *ptr, uint16_t len, uint8_t type)
{
  uint32_t sum = 0;
  if(type==1)
  {
    sum+=IP_UDP;
    sum+=len-8;
  }
	while(len>1)
  {
    sum += (uint16_t) (((uint32_t)*ptr<<8)|*(ptr+1));
	ptr+=2;
    len-=2;
  }
	if(len) sum+=((uint32_t)*ptr)<<8;
	while (sum>>16) sum=(uint16_t)sum+(sum>>16);
	return ~be16toword((uint16_t)sum);
}
//--------------------------------------------------
uint8_t icmp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
  uint8_t res=0;
  ip_pkt_ptr *ip_pkt = (void*)frame->data;
	icmp_pkt_ptr *icmp_pkt = (void*)ip_pkt->data;
	if ((len>=sizeof(icmp_pkt_ptr))&&(icmp_pkt->msg_tp==ICMP_REQ))
  {
		// Toi uu hoa tiep nen ta comment
//    sprintf(str1,"icmp request\r\n");
//    HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	icmp_pkt->msg_tp=ICMP_REPLY;
    icmp_pkt->cs=0;
    icmp_pkt->cs=checksum((void*)icmp_pkt,len,0);
	ip_send(frame,len+sizeof(ip_pkt_ptr));
	// Gui goi xong xuoi moi hien thi chuc nang
	sprintf(str1,"%d.%d.%d.%d-%d.%d.%d.%d icmp request\r\n",
	  ip_pkt->ipaddr_dst[0],ip_pkt->ipaddr_dst[1],ip_pkt->ipaddr_dst[2],ip_pkt->ipaddr_dst[3],ip_pkt->ipaddr_src[0],ip_pkt->ipaddr_src[1],
	  ip_pkt->ipaddr_src[2],ip_pkt->ipaddr_src[3]);
	HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
  }
  return res;
}
//--------------------------------------------------
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
    	udp_read(frame,len1);
    } 
  }
  return res;
}
//--------------------------------------------------
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
  ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr),0);
  eth_send(frame,len);
  return res;
}
//--------------------------------------------------
void eth_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	if (len>=sizeof(enc28j60_frame_ptr))
  {
    if(frame->type==ETH_ARP)
    {
    	// Toi uu khong cho xuat lenh nhieu tiet kiem thoi gian thuc thi nen ta comment
//     sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; arp\r\n",
//     frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
//     frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5], len);
//     HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
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
    	// Toi uu khong cho xuat lenh nhieu tiet kiem thoi gian thuc thi nen ta comment
//     sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; ip\r\n",
//     frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
//     frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5], len);
//     HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
	 ip_read(frame,len-sizeof(enc28j60_frame_ptr)); // Sua lai cho dung voi ly thuyet
    }
    else
    {
    	// Ngoai ARP hoac IP xu l� rieng
    	sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; %04X",
    	  frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
    	  frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],
    	  len, be16toword(frame->type));
    	HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
    	HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,0x1000);
    }
  }
}
//--------------------------------------------------
void eth_send(enc28j60_frame_ptr *frame, uint16_t len)
{
  memcpy(frame->addr_dest,frame->addr_src,6);
  memcpy(frame->addr_src,macaddr,6);
  enc28j60_packetSend((void*)frame,len + sizeof(enc28j60_frame_ptr));
}
//--------------------------------------------------
void net_cmd(void)
{
	uint8_t ip[4]={0};
	if (usartprop.is_ip == 1) //Trang thai yeu cau ARP
	{
		HAL_UART_Transmit(&huart1, usartprop.usart_buf, usartprop.usart_cnt,0x1000);
		HAL_UART_Transmit(&huart1, (uint8_t*) "\r\n", 2, 0x1000);
		ip_extract((char*) usartprop.usart_buf, usartprop.usart_cnt, ip);
		// In ra de thu nghiem  ham chuyen doi ip_extract co chinh xac khong
		//sprintf(str1,"%d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
		//HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
		arp_request(ip);
		usartprop.is_ip = 0;
		usartprop.usart_cnt = 0;
	}
}
//--------------------------------------------------
void net_poll(void)
{
  uint16_t len;
//  uint8_t ip[4]={0};
  enc28j60_frame_ptr *frame=(void*)net_buf;
	while ((len=enc28j60_packetReceive(net_buf,sizeof(net_buf)))>0)
  {
    eth_read(frame,len);
//    if (usartprop.is_ip==1) //Trang thai yeu cau ARP
//    {
//      HAL_UART_Transmit(&huart1,usartprop.usart_buf,usartprop.usart_cnt,0x1000);
//      HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,0x1000);
//      ip_extract((char*)usartprop.usart_buf,usartprop.usart_cnt,ip);
//      // In ra de thu nghiem  ham chuyen doi ip_extract co chinh xac khong
//      //sprintf(str1,"%d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
//      //HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//      arp_request(ip);
//      usartprop.is_ip = 0;
//      usartprop.usart_cnt=0;
//
//    }
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
	  net_cmd();
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
    ss2[offset]=0;
    ipextr[i] = atoi(ss2);
    ip_str+=offset;
    len-=offset;
  }
  strncpy(ss2,ip_str,len);
  ss2[len]=0;
  ipextr[3] = atoi(ss2);
}
//-----------------------------------------------
void TIM_PeriodElapsedCallback(void)
{
  clock_cnt++; // Lay giay va viet chung vao clock_cnt
}
//-----------------------------------------------
