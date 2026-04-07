#include "nrf24l01.h"
#include <string.h>

/* ------------ low level helpers ------------ */
static inline void csn_low(NRF24_t *n)  { HAL_GPIO_WritePin(n->csn_port, n->csn_pin, GPIO_PIN_RESET); }
static inline void csn_high(NRF24_t *n) { HAL_GPIO_WritePin(n->csn_port, n->csn_pin, GPIO_PIN_SET); }
static inline void ce_low(NRF24_t *n)   { HAL_GPIO_WritePin(n->ce_port,  n->ce_pin,  GPIO_PIN_RESET); }
static inline void ce_high(NRF24_t *n)  { HAL_GPIO_WritePin(n->ce_port,  n->ce_pin,  GPIO_PIN_SET); }

static uint8_t spi_rw(NRF24_t *n, uint8_t byte)
{
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(n->hspi, &byte, &rx, 1, 50);
    return rx;
}

static uint8_t read_reg(NRF24_t *n, uint8_t reg)
{
    csn_low(n);
    spi_rw(n, NRF_CMD_R_REGISTER | (reg & 0x1F));
    uint8_t val = spi_rw(n, NRF_CMD_NOP);
    csn_high(n);
    return val;
}

static void write_reg(NRF24_t *n, uint8_t reg, uint8_t val)
{
    csn_low(n);
    spi_rw(n, NRF_CMD_W_REGISTER | (reg & 0x1F));
    spi_rw(n, val);
    csn_high(n);
}

static void read_buf(NRF24_t *n, uint8_t reg, uint8_t *buf, uint8_t len)
{
    csn_low(n);
    spi_rw(n, NRF_CMD_R_REGISTER | (reg & 0x1F));
    for(uint8_t i=0;i<len;i++) buf[i] = spi_rw(n, NRF_CMD_NOP);
    csn_high(n);
}

static void write_buf(NRF24_t *n, uint8_t reg, const uint8_t *buf, uint8_t len)
{
    csn_low(n);
    spi_rw(n, NRF_CMD_W_REGISTER | (reg & 0x1F));
    for(uint8_t i=0;i<len;i++) spi_rw(n, buf[i]);
    csn_high(n);
}

static void cmd(NRF24_t *n, uint8_t command)
{
    csn_low(n);
    spi_rw(n, command);
    csn_high(n);
}

static void clear_irqs(NRF24_t *n)
{
    // Write 1s to clear RX_DR, TX_DS, MAX_RT
    write_reg(n, NRF_REG_STATUS, NRF_STATUS_RX_DR | NRF_STATUS_TX_DS | NRF_STATUS_MAX_RT);
}

/* ------------ public API ------------ */
void NRF24_Begin(NRF24_t *nrf,
                 SPI_HandleTypeDef *hspi,
                 GPIO_TypeDef *ce_port,  uint16_t ce_pin,
                 GPIO_TypeDef *csn_port, uint16_t csn_pin)
{
    nrf->hspi = hspi;
    nrf->ce_port = ce_port;   nrf->ce_pin = ce_pin;
    nrf->csn_port = csn_port; nrf->csn_pin = csn_pin;

    ce_low(nrf);
    csn_high(nrf);
    HAL_Delay(5);
}

static void common_radio_setup(NRF24_t *nrf, const uint8_t *addr5, uint8_t channel)
{
    ce_low(nrf);
    HAL_Delay(2);

    // basic sane defaults
    write_reg(nrf, NRF_REG_SETUP_AW, 0x03);       // 5-byte address
    write_reg(nrf, NRF_REG_RF_CH, channel & 0x7F);// channel 0..127
    write_reg(nrf, NRF_REG_RF_SETUP, 0x06);       // 1Mbps, 0dBm (bits depend, this is common)
    write_reg(nrf, NRF_REG_SETUP_RETR, 0x2F);     // 750us, 15 retries
    write_reg(nrf, NRF_REG_EN_AA, 0x01);          // Auto-ACK pipe0
    write_reg(nrf, NRF_REG_EN_RXADDR, 0x01);      // Enable pipe0

    // set addresses (pipe0 + tx must match for auto-ack)
    write_buf(nrf, NRF_REG_RX_ADDR_P0, addr5, 5);
    write_buf(nrf, NRF_REG_TX_ADDR, addr5, 5);

    // fixed payload size
    write_reg(nrf, NRF_REG_RX_PW_P0, NRF_PAYLOAD_SIZE);

    // disable dynamic payload by default (simple)
    write_reg(nrf, NRF_REG_FEATURE, 0x00);
    write_reg(nrf, NRF_REG_DYNPD, 0x00);

    clear_irqs(nrf);
    cmd(nrf, NRF_CMD_FLUSH_RX);
    cmd(nrf, NRF_CMD_FLUSH_TX);
}

void NRF24_InitTX(NRF24_t *nrf, const uint8_t *addr5, uint8_t channel)
{
    common_radio_setup(nrf, addr5, channel);

    // CONFIG: PWR_UP=1, PRIM_RX=0, CRC=2 bytes (EN_CRC=1, CRCO=1)
    write_reg(nrf, NRF_REG_CONFIG, (1<<3) | (1<<2) | (1<<1));
    HAL_Delay(2);
    ce_low(nrf);
}

void NRF24_InitRX(NRF24_t *nrf, const uint8_t *addr5, uint8_t channel)
{
    common_radio_setup(nrf, addr5, channel);

    // CONFIG: PWR_UP=1, PRIM_RX=1, CRC=2 bytes
    write_reg(nrf, NRF_REG_CONFIG, (1<<3) | (1<<2) | (1<<1) | (1<<0));
    HAL_Delay(2);

    ce_high(nrf);  // RX listens when CE=1
}

bool NRF24_Send(NRF24_t *nrf, const uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
    if (len > NRF_PAYLOAD_SIZE) len = NRF_PAYLOAD_SIZE;

    ce_low(nrf);
    clear_irqs(nrf);
    cmd(nrf, NRF_CMD_FLUSH_TX);

    // write payload
    csn_low(nrf);
    spi_rw(nrf, NRF_CMD_W_TX_PAYLOAD);
    for(uint8_t i=0;i<NRF_PAYLOAD_SIZE;i++)
    {
        uint8_t b = (i < len) ? data[i] : 0x00;
        spi_rw(nrf, b);
    }
    csn_high(nrf);

    // pulse CE to send
    ce_high(nrf);
    HAL_Delay(1);
    ce_low(nrf);

    // wait TX_DS or MAX_RT
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms)
    {
        uint8_t st = read_reg(nrf, NRF_REG_STATUS);

        if (st & NRF_STATUS_TX_DS)
        {
            clear_irqs(nrf);
            return true;
        }
        if (st & NRF_STATUS_MAX_RT)
        {
            clear_irqs(nrf);
            cmd(nrf, NRF_CMD_FLUSH_TX);
            return false;
        }
    }
    return false;
}

bool NRF24_Available(NRF24_t *nrf)
{
    uint8_t fifo = read_reg(nrf, NRF_REG_FIFO_STATUS);
    return ((fifo & NRF_FIFO_RX_EMPTY) == 0);
}

uint8_t NRF24_Read(NRF24_t *nrf, uint8_t *data, uint8_t maxlen)
{
    if (maxlen > NRF_PAYLOAD_SIZE) maxlen = NRF_PAYLOAD_SIZE;

    csn_low(nrf);
    spi_rw(nrf, NRF_CMD_R_RX_PAYLOAD);
    for(uint8_t i=0;i<maxlen;i++) data[i] = spi_rw(nrf, NRF_CMD_NOP);
    csn_high(nrf);

    clear_irqs(nrf);
    return maxlen;
}
