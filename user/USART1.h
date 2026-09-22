#ifndef __USART1_H__
#define __USART1_H__

#include "main.h"
#include "RingBuffer/ringbuffer.h"

#define USART1_RX_BUF_SIZE   256
#define USART1_TX_BUF_SIZE   1024

extern struct rt_ringbuffer usart1_recv_ring_buf;

void Usart1_Init(void);
void Usart1_SendString(char *str);
void Usart1_SendData(uint8_t *data, uint16_t len);
void Usart1_printf(char *fmt, ...);
unsigned char Usart1_RecProcess(uint8_t *buf, uint16_t max_len, uint16_t *recv_len);

/* Called by the only HAL_UART_RxCpltCallback() in the project, which lives in
   USART2.c. Do not define HAL_UART_RxCpltCallback() again in this file. */
void Usart1_RxCpltHandler(void);

#endif
