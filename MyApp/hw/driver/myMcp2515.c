#include "myMcp2515.h"
#include "spi.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/* Private Types and Constants                                                */
/* ========================================================================== */
typedef struct {
  uint8_t cnf1;
  uint8_t cnf2;
  uint8_t cnf3;
} mcp2515Timing_t;

/* Bit Timing Table for 8 MHz Oscillator */
static const mcp2515Timing_t s_timing_8mhz[] = {
  [MCP2515_SPEED_125KBPS]  = { 0x01, 0xB1, 0x05 },
  [MCP2515_SPEED_250KBPS]  = { 0x00, 0xB1, 0x05 },
  [MCP2515_SPEED_500KBPS]  = { 0x00, 0x90, 0x02 },
  [MCP2515_SPEED_1000KBPS] = { 0x00, 0x80, 0x00 },
};

/* Bit Timing Table for 16 MHz Oscillator */
static const mcp2515Timing_t s_timing_16mhz[] = {
  [MCP2515_SPEED_125KBPS]  = { 0x03, 0xB1, 0x05 },
  [MCP2515_SPEED_250KBPS]  = { 0x01, 0xB1, 0x05 },
  [MCP2515_SPEED_500KBPS]  = { 0x00, 0xB1, 0x05 },
  [MCP2515_SPEED_1000KBPS] = { 0x00, 0x90, 0x02 },
};

/* ========================================================================== */
/* Private State Variables                                                    */
/* ========================================================================== */
static bool s_is_ready = false;

/* Software RX FIFO (Circular Ring Buffer) */
static mcp2515Msg_t s_rx_fifo[MCP2515_RX_FIFO_SIZE];
static volatile uint8_t s_fifo_head = 0;
static volatile uint8_t s_fifo_tail = 0;
static volatile uint8_t s_fifo_count = 0;

/* User registered RX callback */
static mcp2515RxCallback_t s_rx_callback = NULL;

/* ========================================================================== */
/* Low-Level SPI & CS Helper Functions                                        */
/* ========================================================================== */
static inline void mcp2515_cs_low(void) {
  HAL_GPIO_WritePin(MCP2515_CS_PORT, MCP2515_CS_PIN, GPIO_PIN_RESET);
}

static inline void mcp2515_cs_high(void) {
  HAL_GPIO_WritePin(MCP2515_CS_PORT, MCP2515_CS_PIN, GPIO_PIN_SET);
}

static uint8_t mcp2515_spi_transfer(uint8_t tx_data) {
  uint8_t rx_data = 0;
  HAL_SPI_TransmitReceive(&hspi2, &tx_data, &rx_data, 1, 100);
  return rx_data;
}

static void mcp2515_write_reg(uint8_t reg, uint8_t val) {
  mcp2515_cs_low();
  mcp2515_spi_transfer(MCP_INSTRUCTION_WRITE);
  mcp2515_spi_transfer(reg);
  mcp2515_spi_transfer(val);
  mcp2515_cs_high();
}

static uint8_t mcp2515_read_reg(uint8_t reg) {
  mcp2515_cs_low();
  mcp2515_spi_transfer(MCP_INSTRUCTION_READ);
  mcp2515_spi_transfer(reg);
  uint8_t val = mcp2515_spi_transfer(0x00);
  mcp2515_cs_high();
  return val;
}

static void mcp2515_write_regs(uint8_t reg, const uint8_t *buf, uint8_t len) {
  mcp2515_cs_low();
  mcp2515_spi_transfer(MCP_INSTRUCTION_WRITE);
  mcp2515_spi_transfer(reg);
  for (uint8_t i = 0; i < len; i++) {
    mcp2515_spi_transfer(buf[i]);
  }
  mcp2515_cs_high();
}

static void mcp2515_bit_modify(uint8_t reg, uint8_t mask, uint8_t data) {
  mcp2515_cs_low();
  mcp2515_spi_transfer(MCP_INSTRUCTION_BIT_MODIFY);
  mcp2515_spi_transfer(reg);
  mcp2515_spi_transfer(mask);
  mcp2515_spi_transfer(data);
  mcp2515_cs_high();
}

static bool mcp2515_reset(void) {
  mcp2515_cs_low();
  mcp2515_spi_transfer(MCP_INSTRUCTION_RESET);
  mcp2515_cs_high();
  HAL_Delay(10);

  /* After reset, device enters Configuration mode (0x80) */
  uint8_t stat = mcp2515_read_reg(MCP_CANSTAT) & MCP_MODE_MASK;
  return (stat == MCP_MODE_CONFIG);
}

/* ========================================================================== */
/* FIFO Helper Functions                                                      */
/* ========================================================================== */
static bool fifo_push(const mcp2515Msg_t *msg) {
  if (s_fifo_count >= MCP2515_RX_FIFO_SIZE) {
    return false; /* FIFO full */
  }
  s_rx_fifo[s_fifo_head] = *msg;
  s_fifo_head = (s_fifo_head + 1) % MCP2515_RX_FIFO_SIZE;
  s_fifo_count++;
  return true;
}

/* ========================================================================== */
/* Public API Functions                                                       */
/* ========================================================================== */

bool mcp2515SetMode(mcp2515OpMode_t mode) {
  mcp2515_bit_modify(MCP_CANCTRL, MCP_MODE_MASK, (uint8_t)mode);

  uint32_t start = HAL_GetTick();
  while ((HAL_GetTick() - start) < 50) {
    uint8_t stat = mcp2515_read_reg(MCP_CANSTAT) & MCP_MODE_MASK;
    if (stat == (uint8_t)mode) {
      return true;
    }
  }
  return false;
}

bool mcp2515InitConfig(mcp2515Speed_t speed, mcp2515Clock_t clock) {
  s_is_ready = false;
  s_fifo_head = 0;
  s_fifo_tail = 0;
  s_fifo_count = 0;

  mcp2515_cs_high();
  HAL_Delay(10);

  /* 1. Software Reset */
  if (!mcp2515_reset()) {
    printf("[MCP2515] Reset failed! Check SPI2 wiring & CS(PC4).\r\n");
    return false;
  }

  /* 2. Configure Baud Rate Timing Registers (CNF1, CNF2, CNF3) */
  const mcp2515Timing_t *timing = (clock == MCP2515_CLK_16MHZ) ? &s_timing_16mhz[speed] : &s_timing_8mhz[speed];
  mcp2515_write_reg(MCP_CNF1, timing->cnf1);
  mcp2515_write_reg(MCP_CNF2, timing->cnf2);
  mcp2515_write_reg(MCP_CNF3, timing->cnf3);

  /* 3. Initialize TX Buffers Control Registers */
  mcp2515_write_reg(MCP_TXB0CTRL, 0x00);
  mcp2515_write_reg(MCP_TXB1CTRL, 0x00);
  mcp2515_write_reg(MCP_TXB2CTRL, 0x00);

  /* 4. Configure RX Buffers: Accept All Messages, Rollover Enabled */
  mcp2515_write_reg(MCP_RXB0CTRL, MCP_RXB_RXM_ALL | MCP_RXB_BUKT_ENABLE);
  mcp2515_write_reg(MCP_RXB1CTRL, MCP_RXB_RXM_ALL);

  /* 5. Clear all Filter & Mask Registers */
  uint8_t zero_buf[4] = {0, 0, 0, 0};
  mcp2515_write_regs(MCP_RXF0SIDH, zero_buf, 4);
  mcp2515_write_regs(MCP_RXF1SIDH, zero_buf, 4);
  mcp2515_write_regs(MCP_RXF2SIDH, zero_buf, 4);
  mcp2515_write_regs(MCP_RXF3SIDH, zero_buf, 4);
  mcp2515_write_regs(MCP_RXF4SIDH, zero_buf, 4);
  mcp2515_write_regs(MCP_RXF5SIDH, zero_buf, 4);
  mcp2515_write_regs(MCP_RXM0SIDH, zero_buf, 4);
  mcp2515_write_regs(MCP_RXM1SIDH, zero_buf, 4);

  /* 6. Enable Interrupts (RX0IE, RX1IE, ERRIE) */
  mcp2515_write_reg(MCP_CANINTE, MCP_RX0IE | MCP_RX1IE | MCP_ERRIE);
  mcp2515_write_reg(MCP_CANINTF, 0x00); /* Clear any pending flags */

  /* 7. Switch to Normal Mode */
  if (!mcp2515SetMode(MCP2515_OPMODE_NORMAL)) {
    printf("[MCP2515] Failed to enter Normal Mode! CANSTAT: 0x%02X\r\n", mcp2515_read_reg(MCP_CANSTAT));
    return false;
  }

  s_is_ready = true;
  printf("[MCP2515] Initialized successfully in Normal Mode!\r\n");
  return true;
}

bool mcp2515Init(void) {
  /* Default: 500 kbps, 8 MHz Crystal */
  return mcp2515InitConfig(MCP2515_SPEED_500KBPS, MCP2515_CLK_8MHZ);
}

bool mcp2515SendMessage(const mcp2515Msg_t *msg) {
  if (!s_is_ready || msg == NULL) {
    return false;
  }

  uint8_t tx_ctrl_reg = 0;
  uint8_t rts_cmd = 0;

  /* Find an available TX buffer */
  if ((mcp2515_read_reg(MCP_TXB0CTRL) & MCP_TXB_TXREQ) == 0) {
    tx_ctrl_reg = MCP_TXB0CTRL;
    rts_cmd = MCP_INSTRUCTION_RTS | 0x01;
  } else if ((mcp2515_read_reg(MCP_TXB1CTRL) & MCP_TXB_TXREQ) == 0) {
    tx_ctrl_reg = MCP_TXB1CTRL;
    rts_cmd = MCP_INSTRUCTION_RTS | 0x02;
  } else if ((mcp2515_read_reg(MCP_TXB2CTRL) & MCP_TXB_TXREQ) == 0) {
    tx_ctrl_reg = MCP_TXB2CTRL;
    rts_cmd = MCP_INSTRUCTION_RTS | 0x04;
  } else {
    return false; /* All buffers are busy */
  }

  uint8_t buf[13] = {0};
  uint8_t dlc = msg->dlc > 8 ? 8 : msg->dlc;

  if (msg->is_ext) {
    /* Extended ID (29-bit) */
    buf[0] = (uint8_t)(msg->id >> 21);
    buf[1] = (uint8_t)(((msg->id >> 13) & 0xE0) | 0x08 | ((msg->id >> 16) & 0x03));
    buf[2] = (uint8_t)(msg->id >> 8);
    buf[3] = (uint8_t)(msg->id & 0xFF);
  } else {
    /* Standard ID (11-bit) */
    buf[0] = (uint8_t)(msg->id >> 3);
    buf[1] = (uint8_t)((msg->id & 0x07) << 5);
    buf[2] = 0;
    buf[3] = 0;
  }

  buf[4] = dlc;
  if (msg->is_rtr) {
    buf[4] |= 0x40; /* RTR bit */
  }

  for (uint8_t i = 0; i < dlc; i++) {
    buf[5 + i] = msg->data[i];
  }

  /* Write SIDH, SIDL, EID8, EID0, DLC, Data (starts at tx_ctrl_reg + 1) */
  mcp2515_write_regs(tx_ctrl_reg + 1, buf, 5 + dlc);

  /* Send Request-to-Send instruction */
  mcp2515_cs_low();
  mcp2515_spi_transfer(rts_cmd);
  mcp2515_cs_high();

  return true;
}

static void parse_rx_buffer(uint8_t instruction, uint8_t flag_mask) {
  uint8_t buf[13] = {0};

  mcp2515_cs_low();
  mcp2515_spi_transfer(instruction);
  for (int i = 0; i < 13; i++) {
    buf[i] = mcp2515_spi_transfer(0x00);
  }
  mcp2515_cs_high();

  /* Clear interrupt flag in CANINTF */
  mcp2515_bit_modify(MCP_CANINTF, flag_mask, 0x00);

  mcp2515Msg_t msg;
  memset(&msg, 0, sizeof(msg));

  uint8_t sidh = buf[0];
  uint8_t sidl = buf[1];
  uint8_t eid8 = buf[2];
  uint8_t eid0 = buf[3];
  uint8_t dlc  = buf[4];

  if (sidl & 0x08) {
    /* Extended ID */
    msg.is_ext = true;
    msg.id = ((uint32_t)sidh << 21) |
             (((uint32_t)sidl & 0xE0) << 13) |
             (((uint32_t)sidl & 0x03) << 16) |
             ((uint32_t)eid8 << 8) |
             eid0;
    msg.is_rtr = (dlc & 0x40) != 0;
  } else {
    /* Standard ID */
    msg.is_ext = false;
    msg.id = ((uint32_t)sidh << 3) | (sidl >> 5);
    msg.is_rtr = (dlc & 0x40) != 0;
  }

  msg.dlc = dlc & 0x0F;
  if (msg.dlc > 8) {
    msg.dlc = 8;
  }

  for (uint8_t i = 0; i < msg.dlc; i++) {
    msg.data[i] = buf[5 + i];
  }

  /* Store into FIFO */
  fifo_push(&msg);

  /* Invoke user callback if registered */
  if (s_rx_callback != NULL) {
    s_rx_callback(&msg);
  }
}

bool mcp2515ReadMessage(mcp2515Msg_t *msg) {
  if (!s_is_ready || msg == NULL) {
    return false;
  }

  /* First check software FIFO */
  if (mcp2515GetRxFifo(msg)) {
    return true;
  }

  /* Otherwise check hardware flags */
  uint8_t intf = mcp2515_read_reg(MCP_CANINTF);
  if (intf & MCP_RX0IF) {
    parse_rx_buffer(MCP_INSTRUCTION_READ_RX_BUFF, MCP_RX0IF);
    return mcp2515GetRxFifo(msg);
  } else if (intf & MCP_RX1IF) {
    parse_rx_buffer(MCP_INSTRUCTION_READ_RX_BUFF | 0x04, MCP_RX1IF);
    return mcp2515GetRxFifo(msg);
  }

  return false;
}

bool mcp2515IsMessageAvailable(void) {
  if (s_fifo_count > 0) {
    return true;
  }
  uint8_t intf = mcp2515_read_reg(MCP_CANINTF);
  return (intf & (MCP_RX0IF | MCP_RX1IF)) != 0;
}

void mcp2515IsrHandler(void) {
  if (!s_is_ready) {
    return;
  }

  uint8_t intf = mcp2515_read_reg(MCP_CANINTF);
  while (intf & (MCP_RX0IF | MCP_RX1IF)) {
    if (intf & MCP_RX0IF) {
      parse_rx_buffer(MCP_INSTRUCTION_READ_RX_BUFF, MCP_RX0IF);
    }
    if (intf & MCP_RX1IF) {
      parse_rx_buffer(MCP_INSTRUCTION_READ_RX_BUFF | 0x04, MCP_RX1IF);
    }
    intf = mcp2515_read_reg(MCP_CANINTF);
  }

  /* Clear error flags if set */
  if (intf & (MCP_ERRIF | MCP_MERRF)) {
    mcp2515_bit_modify(MCP_CANINTF, MCP_ERRIF | MCP_MERRF, 0x00);
  }
}

void mcp2515SetRxCallback(mcp2515RxCallback_t callback) {
  s_rx_callback = callback;
}

bool mcp2515GetRxFifo(mcp2515Msg_t *msg) {
  if (msg == NULL || s_fifo_count == 0) {
    return false;
  }

  *msg = s_rx_fifo[s_fifo_tail];
  s_fifo_tail = (s_fifo_tail + 1) % MCP2515_RX_FIFO_SIZE;
  s_fifo_count--;
  return true;
}

uint8_t mcp2515GetRxFifoCount(void) {
  return s_fifo_count;
}

uint8_t mcp2515ReadStatus(void) {
  mcp2515_cs_low();
  mcp2515_spi_transfer(MCP_INSTRUCTION_READ_STATUS);
  uint8_t status = mcp2515_spi_transfer(0x00);
  mcp2515_cs_high();
  return status;
}

uint8_t mcp2515GetErrorFlags(void) {
  return mcp2515_read_reg(MCP_EFLG);
}

bool mcp2515IsReady(void) {
  return s_is_ready;
}
