#ifndef __USART2_H__
#define __USART2_H__

#include "main.h"
#include "RingBuffer/ringbuffer.h"

#define USART2_RX_BUF_SIZE   256
#define USART2_TX_BUF_SIZE   1024

extern struct rt_ringbuffer usart2_recv_ring_buf;

void Usart2_Init(void);
void Usart2_SendString(char *str);
void Usart2_SendData(uint8_t *data, uint16_t len);
void Usart2_printf(char *fmt, ...);
unsigned char Usart2_RecProcess(uint8_t *buf, uint16_t max_len, uint16_t *recv_len);

#endif
