#pragma once
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* nRF24 Commands */
#define NRF_CMD_R_REGISTER        0x00
#define NRF_CMD_W_REGISTER        0x20
#define NRF_CMD_R_RX_PAYLOAD      0x61
#define NRF_CMD_W_TX_PAYLOAD      0xA0
#define NRF_CMD_FLUSH_TX          0xE1
#define NRF_CMD_FLUSH_RX          0xE2
#define NRF_CMD_REUSE_TX_PL       0xE3
#define NRF_CMD_NOP               0xFF

/* nRF24 Registers */
#define NRF_REG_CONFIG            0x00
#define NRF_REG_EN_AA             0x01
#define NRF_REG_EN_RXADDR         0x02
#define NRF_REG_SETUP_AW          0x03
#define NRF_REG_SETUP_RETR        0x04
#define NRF_REG_RF_CH             0x05
#define NRF_REG_RF_SETUP          0x06
#define NRF_REG_STATUS            0x07
#define NRF_REG_OBSERVE_TX        0x08
#define NRF_REG_RX_ADDR_P0        0x0A
#define NRF_REG_RX_ADDR_P1        0x0B
#define NRF_REG_TX_ADDR           0x10
#define NRF_REG_RX_PW_P0          0x11
#define NRF_REG_RX_PW_P1          0x12
#define NRF_REG_FIFO_STATUS       0x17
#define NRF_REG_DYNPD             0x1C
#define NRF_REG_FEATURE           0x1D

/* STATUS bits */
#define NRF_STATUS_RX_DR          (1<<6)
#define NRF_STATUS_TX_DS          (1<<5)
#define NRF_STATUS_MAX_RT         (1<<4)

/* FIFO_STATUS bits */
#define NRF_FIFO_RX_EMPTY         (1<<0)
#define NRF_FIFO_TX_FULL          (1<<5)

#define NRF_PAYLOAD_SIZE          32  // fixed payload

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *ce_port;  uint16_t ce_pin;
    GPIO_TypeDef *csn_port; uint16_t csn_pin;
} NRF24_t;

/* API */
void NRF24_Begin(NRF24_t *nrf,
                 SPI_HandleTypeDef *hspi,
                 GPIO_TypeDef *ce_port,  uint16_t ce_pin,
                 GPIO_TypeDef *csn_port, uint16_t csn_pin);

void NRF24_InitTX(NRF24_t *nrf, const uint8_t *addr5, uint8_t channel);
void NRF24_InitRX(NRF24_t *nrf, const uint8_t *addr5, uint8_t channel);

bool NRF24_Send(NRF24_t *nrf, const uint8_t *data, uint8_t len, uint32_t timeout_ms);
bool NRF24_Available(NRF24_t *nrf);
uint8_t NRF24_Read(NRF24_t *nrf, uint8_t *data, uint8_t maxlen);
