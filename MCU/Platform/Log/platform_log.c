/**
 ******************************************************************************
 * @file    platform_log.c
 * @brief   平台日志：USART1 / USART2 / USB CDC，可选 USART DMA
 ******************************************************************************
 */

#include "platform_log.h"
#include "usart.h"
#include "stm32f1xx_hal.h"

#if PLATFORM_LOG_ENABLE && (PLATFORM_LOG_BACKEND == PLATFORM_LOG_BACKEND_CDC)
#include "usbd_cdc_if.h"
#endif

#if PLATFORM_LOG_ENABLE

/**
 * @brief  根据配置返回当前日志使用的 UART 句柄
 * @retval 指向 huart1 / huart2 的指针；后端为 CDC 时返回 NULL
 */
static UART_HandleTypeDef *platform_log_uart(void)
{
#if PLATFORM_LOG_BACKEND == PLATFORM_LOG_BACKEND_USART1
  return &huart1;
#elif PLATFORM_LOG_BACKEND == PLATFORM_LOG_BACKEND_USART2
  return &huart2;
#else
  return NULL;
#endif
}

/**
 * @brief  等待 UART 发送完成（忙等待，带超时）
 * @param  huart UART 句柄
 * @retval 无
 */
static void uart_wait_tx_idle(UART_HandleTypeDef *huart)
{
  uint32_t t0 = HAL_GetTick();
  while (HAL_UART_GetState(huart) == HAL_UART_STATE_BUSY_TX)
  {
    if ((HAL_GetTick() - t0) > PLATFORM_LOG_UART_TIMEOUT_MS)
    {
      break;
    }
  }
}

/**
 * @brief  通过 USART/UART 输出一段日志数据
 * @note   若开启 DMA 且已链接 hdmatx，则分片拷贝到静态缓冲后 DMA 发送；否则使用阻塞式 HAL_UART_Transmit
 * @param  data 数据首地址
 * @param  len  字节长度
 * @retval 无
 */
static void log_write_uart(const uint8_t *data, size_t len)
{
  UART_HandleTypeDef *huart = platform_log_uart();
  if (huart == NULL || data == NULL || len == 0U)
  {
    return;
  }

#if PLATFORM_LOG_USE_DMA
  if (huart->hdmatx != NULL)
  {
    static uint8_t s_dma_buf[PLATFORM_LOG_DMA_MAX_CHUNK];
    size_t off = 0U;
    while (off < len)
    {
      size_t chunk = len - off;
      if (chunk > PLATFORM_LOG_DMA_MAX_CHUNK)
      {
        chunk = PLATFORM_LOG_DMA_MAX_CHUNK;
      }
      for (size_t i = 0U; i < chunk; i++)
      {
        s_dma_buf[i] = data[off + i];
      }
      uart_wait_tx_idle(huart);
      if (HAL_UART_Transmit_DMA(huart, s_dma_buf, (uint16_t)chunk) != HAL_OK)
      {
        (void)HAL_UART_Transmit(huart, s_dma_buf, (uint16_t)chunk, PLATFORM_LOG_UART_TIMEOUT_MS);
      }
      uart_wait_tx_idle(huart);
      off += chunk;
    }
    return;
  }
#endif
  (void)HAL_UART_Transmit(huart, (uint8_t *)data, (uint16_t)len, PLATFORM_LOG_UART_TIMEOUT_MS);
}

#if PLATFORM_LOG_BACKEND == PLATFORM_LOG_BACKEND_CDC
/**
 * @brief  通过 USB CDC（虚拟串口）输出一段日志数据
 * @note   分块调用 CDC_Transmit_FS；遇 USBD_BUSY 时延时重试，总等待超过 PLATFORM_LOG_UART_TIMEOUT_MS 则放弃剩余数据
 * @param  data 数据首地址
 * @param  len  字节长度
 * @retval 无
 */
static void log_write_cdc(const uint8_t *data, size_t len)
{
  if (data == NULL || len == 0U)
  {
    return;
  }
  size_t off = 0U;
  while (off < len)
  {
    uint16_t chunk = (uint16_t)((len - off) > 512U ? 512U : (len - off));
    uint32_t t0 = HAL_GetTick();
    for (;;)
    {
      uint8_t st = CDC_Transmit_FS((uint8_t *)&data[off], chunk);
      if (st != USBD_BUSY)
      {
        off += chunk;
        break;
      }
      if ((HAL_GetTick() - t0) > PLATFORM_LOG_UART_TIMEOUT_MS)
      {
        return;
      }
      HAL_Delay(1U);
    }
  }
}
#endif

/**
 * @brief  实际写入底层（UART/CDC），不附加 PLATFORM_LOG_PREFIX_STRING
 * @param  data 数据首地址
 * @param  len  字节长度
 * @retval 无
 */
static void platform_log_sink_write(const uint8_t *data, size_t len)
{
#if PLATFORM_LOG_BACKEND == PLATFORM_LOG_BACKEND_CDC
  log_write_cdc(data, len);
#else
  log_write_uart(data, len);
#endif
}

/**
 * @brief  初始化平台日志模块
 * @note   外设由 CubeMX 初始化；本函数预留扩展（例如 CDC 就绪标志），当前无额外操作
 * @retval 无
 */
void PlatformLog_Init(void)
{
  /* 句柄由 Cube MX 初始化；此处预留给后续扩展（如 CDC 就绪标志） */
}

/**
 * @brief  输出一段原始日志数据（根据 platform_log_config.h 选择 UART 或 CDC）
 * @note   若开启 PLATFORM_LOG_PREFIX_ENABLE，先发送前缀再发送正文；printf 走 __io_putchar，不加重复前缀
 * @param  data 数据首地址，可为文本或二进制
 * @param  len  字节长度
 * @retval 无
 */
void PlatformLog_Write(const uint8_t *data, size_t len)
{
#if PLATFORM_LOG_PREFIX_ENABLE
  {
    static const char s_prefix[] = PLATFORM_LOG_PREFIX_STRING;
    const size_t prefix_len = sizeof(s_prefix) - 1U;
    if (prefix_len > 0U)
    {
      platform_log_sink_write((const uint8_t *)s_prefix, prefix_len);
    }
  }
#endif
  platform_log_sink_write(data, len);
}

#if PLATFORM_LOG_HOOK_STDIO
/**
 * @brief  Newlib 单字符输出钩子，供 printf/puts 等经 _write 调用
 * @param  ch 待发送字符（按 uint8_t 发送）
 * @retval 成功返回传入字符；失败仍返回 ch（与常见 stub 行为一致）
 */
int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;
  platform_log_sink_write(&c, 1U);
  return ch;
}
#endif

#else /* !PLATFORM_LOG_ENABLE */

/**
 * @brief  日志关闭时的空实现，保持 API 可链接
 * @retval 无
 */
void PlatformLog_Init(void) {}

/**
 * @brief  日志关闭时丢弃写入请求
 * @param  data 忽略
 * @param  len  忽略
 * @retval 无
 */
void PlatformLog_Write(const uint8_t *data, size_t len)
{
  (void)data;
  (void)len;
}

#endif /* PLATFORM_LOG_ENABLE */
