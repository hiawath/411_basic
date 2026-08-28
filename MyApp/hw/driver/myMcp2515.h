#pragma once

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Pin Configurations                                                         */
/* ========================================================================== */
#define MCP2515_CS_PORT           GPIOC
#define MCP2515_CS_PIN            GPIO_PIN_4

#define MCP2515_INT_PORT          GPIOC
#define MCP2515_INT_PIN           GPIO_PIN_0

#define MCP2515_RX_FIFO_SIZE      16

/* ========================================================================== */
/* MCP2515 SPI Instructions                                                   */
/* ========================================================================== */
#define MCP_INSTRUCTION_RESET         0xC0
#define MCP_INSTRUCTION_READ          0x03
#define MCP_INSTRUCTION_READ_RX_BUFF  0x90
#define MCP_INSTRUCTION_WRITE         0x02
#define MCP_INSTRUCTION_LOAD_TX_BUFF  0x40
#define MCP_INSTRUCTION_RTS           0x80
#define MCP_INSTRUCTION_READ_STATUS   0xA0
#define MCP_INSTRUCTION_RX_STATUS     0xB0
#define MCP_INSTRUCTION_BIT_MODIFY    0x05

/* ========================================================================== */
/* MCP2515 Registers Map                                                      */
/* ========================================================================== */
#define MCP_RXF0SIDH      0x00
#define MCP_RXF0SIDL      0x01
#define MCP_RXF0EID8      0x02
#define MCP_RXF0EID0      0x03
#define MCP_RXF1SIDH      0x04
#define MCP_RXF1SIDL      0x05
#define MCP_RXF1EID8      0x06
#define MCP_RXF1EID0      0x07
#define MCP_RXF2SIDH      0x08
#define MCP_RXF2SIDL      0x09
#define MCP_RXF2EID8      0x0A
#define MCP_RXF2EID0      0x0B
#define MCP_BFPCTRL       0x0C
#define MCP_TXRTSCTRL     0x0D
#define MCP_CANSTAT       0x0E
#define MCP_CANCTRL       0x0F

#define MCP_RXF3SIDH      0x10
#define MCP_RXF3SIDL      0x11
#define MCP_RXF3EID8      0x12
#define MCP_RXF3EID0      0x13
#define MCP_RXF4SIDH      0x14
#define MCP_RXF4SIDL      0x15
#define MCP_RXF4EID8      0x16
#define MCP_RXF4EID0      0x17
#define MCP_RXF5SIDH      0x18
#define MCP_RXF5SIDL      0x19
#define MCP_RXF5EID8      0x1A
#define MCP_RXF5EID0      0x1B
#define MCP_TEC           0x1C
#define MCP_REC           0x1D

#define MCP_RXM0SIDH      0x20
#define MCP_RXM0SIDL      0x21
#define MCP_RXM0EID8      0x22
#define MCP_RXM0EID0      0x23
#define MCP_RXM1SIDH      0x24
#define MCP_RXM1SIDL      0x25
#define MCP_RXM1EID8      0x26
#define MCP_RXM1EID0      0x27
#define MCP_CNF3          0x28
#define MCP_CNF2          0x29
#define MCP_CNF1          0x2A
#define MCP_CANINTE       0x2B
#define MCP_CANINTF       0x2C
#define MCP_EFLG          0x2D

#define MCP_TXB0CTRL      0x30
#define MCP_TXB0SIDH      0x31
#define MCP_TXB0SIDL      0x32
#define MCP_TXB0EID8      0x33
#define MCP_TXB0EID0      0x34
#define MCP_TXB0DLC       0x35
#define MCP_TXB0D0        0x36

#define MCP_TXB1CTRL      0x40
#define MCP_TXB1SIDH      0x41
#define MCP_TXB1SIDL      0x42
#define MCP_TXB1EID8      0x43
#define MCP_TXB1EID0      0x44
#define MCP_TXB1DLC       0x45
#define MCP_TXB1D0        0x46

#define MCP_TXB2CTRL      0x50
#define MCP_TXB2SIDH      0x51
#define MCP_TXB2SIDL      0x52
#define MCP_TXB2EID8      0x53
#define MCP_TXB2EID0      0x54
#define MCP_TXB2DLC       0x55
#define MCP_TXB2D0        0x56

#define MCP_RXB0CTRL      0x60
#define MCP_RXB0SIDH      0x61
#define MCP_RXB0SIDL      0x62
#define MCP_RXB0EID8      0x63
#define MCP_RXB0EID0      0x64
#define MCP_RXB0DLC       0x65
#define MCP_RXB0D0        0x66

#define MCP_RXB1CTRL      0x70
#define MCP_RXB1SIDH      0x71
#define MCP_RXB1SIDL      0x72
#define MCP_RXB1EID8      0x73
#define MCP_RXB1EID0      0x74
#define MCP_RXB1DLC       0x75
#define MCP_RXB1D0        0x76

/* ========================================================================== */
/* Register Bit Definitions                                                   */
/* ========================================================================== */
/* CANCTRL */
#define MCP_MODE_NORMAL       0x00
#define MCP_MODE_SLEEP        0x20
#define MCP_MODE_LOOPBACK     0x40
#define MCP_MODE_LISTENONLY   0x60
#define MCP_MODE_CONFIG       0x80
#define MCP_MODE_MASK         0xE0
#define MCP_CLKOUT_ENABLE     0x04
#define MCP_CLKOUT_DISABLE    0x00

/* CANINTF & CANINTE */
#define MCP_RX0IF             0x01
#define MCP_RX1IF             0x02
#define MCP_TX0IF             0x04
#define MCP_TX1IF             0x08
#define MCP_TX2IF             0x10
#define MCP_ERRIF             0x20
#define MCP_WAKIF             0x40
#define MCP_MERRF             0x80

#define MCP_RX0IE             0x01
#define MCP_RX1IE             0x02
#define MCP_TX0IE             0x04
#define MCP_TX1IE             0x08
#define MCP_TX2IE             0x10
#define MCP_ERRIE             0x20
#define MCP_WAKIE             0x40
#define MCP_MERRE             0x80

/* RXBnCTRL */
#define MCP_RXB_RXM_ALL       0x60 /* Filter & Mask disabled, receive any message */
#define MCP_RXB_RXM_FILTER    0x00 /* Receive all valid messages that meet filter criteria */
#define MCP_RXB_BUKT_ENABLE   0x04 /* Rollover enable (RXB0 -> RXB1) */

/* TXBnCTRL */
#define MCP_TXB_TXREQ         0x08 /* Message Transmit Request bit */

/* ========================================================================== */
/* Types and Data Structures                                                  */
/* ========================================================================== */

/* Baud Rate Settings */
typedef enum {
  MCP2515_SPEED_125KBPS = 0,
  MCP2515_SPEED_250KBPS,
  MCP2515_SPEED_500KBPS,
  MCP2515_SPEED_1000KBPS,
} mcp2515Speed_t;

/* Crystal Oscillator Frequencies */
typedef enum {
  MCP2515_CLK_8MHZ = 0,
  MCP2515_CLK_16MHZ,
} mcp2515Clock_t;

/* Operating Modes */
typedef enum {
  MCP2515_OPMODE_NORMAL = MCP_MODE_NORMAL,
  MCP2515_OPMODE_LOOPBACK = MCP_MODE_LOOPBACK,
  MCP2515_OPMODE_LISTENONLY = MCP_MODE_LISTENONLY,
  MCP2515_OPMODE_CONFIG = MCP_MODE_CONFIG,
  MCP2515_OPMODE_SLEEP = MCP_MODE_SLEEP
} mcp2515OpMode_t;

/* CAN Message Structure */
typedef struct {
  uint32_t id;         /* Standard(11-bit) or Extended(29-bit) CAN ID */
  bool     is_ext;     /* true: Extended frame (29-bit), false: Standard frame (11-bit) */
  bool     is_rtr;     /* true: Remote transmission request, false: Data frame */
  uint8_t  dlc;        /* Data length code (0 ~ 8) */
  uint8_t  data[8];    /* Payload data */
} mcp2515Msg_t;

/* Callback function type for asynchronous RX notification */
typedef void (*mcp2515RxCallback_t)(const mcp2515Msg_t *msg);

/* ========================================================================== */
/* Public Function Prototypes                                                 */
/* ========================================================================== */

/**
 * @brief Initialize MCP2515 with default settings (500 kbps, 8MHz Crystal, Normal Mode).
 * @retval true on success, false otherwise.
 */
bool mcp2515Init(void);

/**
 * @brief Initialize MCP2515 with custom baudrate and oscillator configuration.
 * @param speed Baud rate (125k, 250k, 500k, 1000k)
 * @param clock Crystal frequency (8MHz or 16MHz)
 * @retval true on success, false otherwise.
 */
bool mcp2515InitConfig(mcp2515Speed_t speed, mcp2515Clock_t clock);

/**
 * @brief Set MCP2515 operating mode (Normal, Loopback, ListenOnly, Config, Sleep).
 * @param mode Target operation mode
 * @retval true on success, false otherwise.
 */
bool mcp2515SetMode(mcp2515OpMode_t mode);

/**
 * @brief Send a CAN frame through the first available TX buffer.
 * @param msg Pointer to CAN message to send
 * @retval true on success, false if all buffers are busy or SPI error.
 */
bool mcp2515SendMessage(const mcp2515Msg_t *msg);

/**
 * @brief Directly read a CAN frame from hardware RX buffer (if available).
 * @param msg Pointer to buffer where received frame will be stored
 * @retval true if frame was read, false otherwise.
 */
bool mcp2515ReadMessage(mcp2515Msg_t *msg);

/**
 * @brief Check if hardware RX buffer has pending messages.
 * @retval true if message is waiting in RXB0 or RXB1.
 */
bool mcp2515IsMessageAvailable(void);

/**
 * @brief Interrupt Service Routine Handler (called from EXTI callback on INT pin).
 */
void mcp2515IsrHandler(void);

/**
 * @brief Register a callback function invoked when a CAN message is received via interrupt.
 * @param callback Callback function pointer (NULL to disable)
 */
void mcp2515SetRxCallback(mcp2515RxCallback_t callback);

/**
 * @brief Retrieve a received message from the internal software RX FIFO.
 * @param msg Pointer to store the dequeued CAN message
 * @retval true if message dequeued, false if FIFO empty.
 */
bool mcp2515GetRxFifo(mcp2515Msg_t *msg);

/**
 * @brief Get number of unread messages currently in software RX FIFO.
 * @retval Number of messages in FIFO (0 ~ MCP2515_RX_FIFO_SIZE).
 */
uint8_t mcp2515GetRxFifoCount(void);

/**
 * @brief Read MCP2515 status byte.
 * @retval 8-bit status value.
 */
uint8_t mcp2515ReadStatus(void);

/**
 * @brief Read MCP2515 Error Flag (EFLG) register.
 * @retval 8-bit EFLG register value.
 */
uint8_t mcp2515GetErrorFlags(void);

/**
 * @brief Check if MCP2515 is successfully initialized and ready.
 * @retval true if initialized.
 */
bool mcp2515IsReady(void);

#ifdef __cplusplus
}
#endif
