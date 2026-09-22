#include "USART2.h"
#include "USART1.h"
#include "USART3.h"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* UART2 receive ring buffer */
struct rt_ringbuffer usart2_recv_ring_buf;
static uint8_t s_usart2_rx_buf[USART2_RX_BUF_SIZE];

/* single-byte interrupt reception */
static uint8_t  s_usart2_rx_byte;
static uint32_t s_usart2_last_rx_tick = 0;

/* line idle this long (ms) => treat received bytes as one complete frame */
#define USART2_FRAME_IDLE_MS   5

void Usart2_Init(void)
{
    rt_ringbuffer_init(&usart2_recv_ring_buf, s_usart2_rx_buf, USART2_RX_BUF_SIZE);

    /* USART2 peripheral & GPIO are already configured by CubeMX
       (MX_USART2_UART_Init). Here we only enable the RX interrupt. */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    HAL_UART_Receive_IT(&huart2, &s_usart2_rx_byte, 1);
}

void Usart2_SendData(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart2, data, len, 1000);
}

void Usart2_SendString(char *str)
{
    Usart2_SendData((uint8_t *)str, (uint16_t)strlen(str));
}

static char s_usart2_tx_buf[USART2_TX_BUF_SIZE];
void Usart2_printf(char *fmt, ...)
{
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = vsnprintf(s_usart2_tx_buf, sizeof(s_usart2_tx_buf), fmt, ap);
    va_end(ap);

    if (len <= 0)
        return;
    if (len > (int)sizeof(s_usart2_tx_buf) - 1)
        len = (int)sizeof(s_usart2_tx_buf) - 1;

    Usart2_SendData((uint8_t *)s_usart2_tx_buf, (uint16_t)len);
}

/* HAL callback: fires after each byte is received in interrupt mode.
   This is the ONLY definition of HAL_UART_RxCpltCallback() in the project,
   so every UART must be dispatched from here. */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        rt_ringbuffer_putchar(&usart2_recv_ring_buf, s_usart2_rx_byte);
        s_usart2_last_rx_tick = HAL_GetTick();
        HAL_UART_Receive_IT(&huart2, &s_usart2_rx_byte, 1);
    }
    else if (huart->Instance == USART1)
    {
        Usart1_RxCpltHandler();
    }
    else if (huart->Instance == USART3)
    {
        Usart3_RxCpltHandler();
    }
}

/* Poll this from the main loop. Returns 1 and copies one complete frame into
   buf when the line has been idle for USART2_FRAME_IDLE_MS after the last byte. */
unsigned char Usart2_RecProcess(uint8_t *buf, uint16_t max_len, uint16_t *recv_len)
{
    uint16_t len = (uint16_t)rt_ringbuffer_data_len(&usart2_recv_ring_buf);

    if (len == 0)
        return 0;

    if ((HAL_GetTick() - s_usart2_last_rx_tick) < USART2_FRAME_IDLE_MS)
        return 0;

    if (len > max_len)
        len = max_len;

    rt_ringbuffer_get(&usart2_recv_ring_buf, buf, len);
    *recv_len = len;

    return 1;
}
