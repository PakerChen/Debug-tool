#include "USART3.h"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* UART3 receive ring buffer */
struct rt_ringbuffer usart3_recv_ring_buf;
static uint8_t s_usart3_rx_buf[USART3_RX_BUF_SIZE];

/* single-byte interrupt reception */
static uint8_t  s_usart3_rx_byte;
static uint32_t s_usart3_last_rx_tick = 0;

/* line idle this long (ms) => treat received bytes as one complete frame */
#define USART3_FRAME_IDLE_MS   5

void Usart3_Init(void)
{
    rt_ringbuffer_init(&usart3_recv_ring_buf, s_usart3_rx_buf, USART3_RX_BUF_SIZE);

    /* USART3 peripheral & GPIO are already configured by CubeMX
       (MX_USART3_UART_Init: PB10 = TX, PB11 = RX, 115200-8-N-1).
       Here we only enable the RX interrupt. */
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    HAL_UART_Receive_IT(&huart3, &s_usart3_rx_byte, 1);
}

void Usart3_SendData(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart3, data, len, 1000);
}

void Usart3_SendString(char *str)
{
    Usart3_SendData((uint8_t *)str, (uint16_t)strlen(str));
}

static char s_usart3_tx_buf[USART3_TX_BUF_SIZE];
void Usart3_printf(char *fmt, ...)
{
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = vsnprintf(s_usart3_tx_buf, sizeof(s_usart3_tx_buf), fmt, ap);
    va_end(ap);

    if (len <= 0)
        return;
    if (len > (int)sizeof(s_usart3_tx_buf) - 1)
        len = (int)sizeof(s_usart3_tx_buf) - 1;

    Usart3_SendData((uint8_t *)s_usart3_tx_buf, (uint16_t)len);
}

/* Called from the only HAL_UART_RxCpltCallback() in the project (USART2.c),
   once per received byte. HAL_UART_RxCpltCallback() must stay defined in
   exactly one .c file, otherwise the linker reports a duplicate symbol. */
void Usart3_RxCpltHandler(void)
{
    rt_ringbuffer_putchar(&usart3_recv_ring_buf, s_usart3_rx_byte);
    s_usart3_last_rx_tick = HAL_GetTick();
    HAL_UART_Receive_IT(&huart3, &s_usart3_rx_byte, 1);
}

/* Poll this from the main loop. Returns 1 and copies one complete frame into
   buf when the line has been idle for USART3_FRAME_IDLE_MS after the last byte. */
unsigned char Usart3_RecProcess(uint8_t *buf, uint16_t max_len, uint16_t *recv_len)
{
    uint16_t len = (uint16_t)rt_ringbuffer_data_len(&usart3_recv_ring_buf);

    if (len == 0)
        return 0;

    if ((HAL_GetTick() - s_usart3_last_rx_tick) < USART3_FRAME_IDLE_MS)
        return 0;

    if (len > max_len)
        len = max_len;

    rt_ringbuffer_get(&usart3_recv_ring_buf, buf, len);
    *recv_len = len;

    return 1;
}
