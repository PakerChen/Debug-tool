#ifndef __USART3_H__
#define __USART3_H__

#include "main.h"
#include "RingBuffer/ringbuffer.h"

#define USART3_RX_BUF_SIZE   256
#define USART3_TX_BUF_SIZE   1024

extern struct rt_ringbuffer usart3_recv_ring_buf;

void Usart3_Init(void);
void Usart3_SendString(char *str);
void Usart3_SendData(uint8_t *data, uint16_t len);
void Usart3_printf(char *fmt, ...);
unsigned char Usart3_RecProcess(uint8_t *buf, uint16_t max_len, uint16_t *recv_len);

/* Called by the only HAL_UART_RxCpltCallback() in the project, which lives in
   USART2.c. Do not define HAL_UART_RxCpltCallback() again in this file. */
void Usart3_RxCpltHandler(void);

#endif
