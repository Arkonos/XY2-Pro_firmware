
#pragma once

#include "board/yBoard.h"
#include "src/lcd/tft/tft_color.h"
#include "1romdata/romdata.h"

extern volatile bool serialPause;

typedef struct
{
  uint32_t SYSCLK_Frequency;  /*!< returns SYSCLK clock frequency expressed in Hz */
  uint32_t HCLK_Frequency;    /*!< returns HCLK clock frequency expressed in Hz */
  uint32_t PCLK1_Frequency;   /*!< returns PCLK1 clock frequency expressed in Hz */
  uint32_t PCLK2_Frequency;   /*!< returns PCLK2 clock frequency expressed in Hz */
  uint32_t USB_Frequency;
  uint32_t SDIO_Frequency;
}RCC_ClocksTypeDef;
void RCC_GetClocksFreq(RCC_ClocksTypeDef* RCC_Clocks);

#define CONST_CMD(x) gcode.process_subcommands_now(F(x))
#define QUEUE_CMD(x) queue.enqueue_one(F(x))
#define JOIN_CMD(x)  queue.enqueue_one(x)
extern const uint8_t tronxy_version[4];

#define SDPIC_SIGN                0x5AA5A55A
#define SDPIC_ICON_SIZE           80
#define SDPIC_MAIN_SIZE           160
#define SDPIC_MAIN_SIGN           ";gimage:"
#define SDPIC_ICON_SIGN           ";simage:"

enum SOCKET_ST
{
    SOCKET_ST_IDLE = 0,
    SOCKET_GET_IP
};