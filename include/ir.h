#pragma once

#include <3ds/types.h>

// IR I2C registers
#define REG_FIFO	0x00	// Receive / Transmit Holding Register
#define REG_DLL		0x00	// Baudrate Divisor Latch Register Low
#define REG_IER		0x08	// Interrupt Enable Register
#define REG_DLH		0x08	// Baudrate Divisor Latch Register High
#define REG_FCR		0x10	// FIFO Control Register
#define REG_EFR		0x10	// Enhanced Feature Register
#define REG_LCR		0x18	// Line Control Register
#define REG_MCR		0x20	// Modem Control Register
#define REG_LSR		0x28	// Line Status Register
#define REG_TXLVL	0x40	// Transmitter FIFO Level Register
#define REG_RXLVL	0x48	// Receiver FIFO Level Register
#define REG_IOSTATE	0x58	// IOState Register
#define	REG_EFCR	0x78	// Extra Features Control Register

// Parameters
#define	RX_MAX_WAIT		30		// 30cycles ~ 3ms (?)
#define RX_TIMEOUT		40000	// 4s (?)

// Initialize and exit IR
bool ir_init(void);
// Call these functions before and after sending/receiving data
void ir_enable(void);
void ir_disable(void);
// Send and receive IR data
void ir_send_data(void *data, u32 size);
u32 ir_recv_data(void *data, u32 size);
