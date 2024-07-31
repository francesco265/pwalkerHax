#include "ir.h"
#include "i2c.h"
#include <stdbool.h>
#include <3ds/types.h>

void ir_setbitrate(u16 value)
{
	u8 lcr = I2C_read(REG_LCR);

	// Enable access to DLL and DLH
	I2C_write(REG_LCR, lcr | BIT(7));
	// Disable sleep mode
	I2C_write(REG_IER, 0);

	I2C_write(REG_DLL, value & 0xFF);
	I2C_write(REG_DLH, (value >> 8) & 0xFF);

	I2C_write(REG_LCR, lcr);
	I2C_write(REG_IER, BIT(4));
}

bool ir_init(void)
{
	static bool inited = false;

	if (inited)
		return false;
	inited = true;

	I2C_init();
	ir_setbitrate(10);

	return true;
}

void ir_enable(void)
{
	// Disable sleep mode
	I2C_write(REG_IER, 0);
	// IOState must be 0
	I2C_write(REG_IOSTATE, 0);
}

void ir_disable(void)
{
	// Enable sleep mode
	I2C_write(REG_IER, BIT(4));
	I2C_write(REG_IOSTATE, BIT(0));
}

void ir_send_data(void *data, u32 size)
{
	u8 *ptr8 = (u8 *) data, txlvl;
	u32 sent;

	// Reset and enable TX FIFO
	I2C_write(REG_FCR, 0x05);
	// Enable transmitter
	I2C_write(REG_EFCR, 0x02);

	do {
		txlvl = I2C_read(REG_TXLVL);
		sent = size > txlvl ? txlvl : size;
		I2C_writeArray(REG_FIFO, ptr8, sent);
		ptr8 += sent;
		size -= sent;
	} while (size);

	// Wait until THR and TSR are empty
	while (!(I2C_read(REG_LSR) & BIT(6)));

	// Disable transmitter and receiver
	I2C_write(REG_EFCR, 0x06);
	// Disable FIFO
	I2C_write(REG_FCR, 0);
}

u32 ir_recv_data(void *data, u32 size)
{
	u8 *ptr8 = (u8 *) data, rxlvl;
	u16 i, timeout = RX_TIMEOUT;
	u32 tc = 0;
	bool loop = true, enter = true;

	// Reset and enable RX FIFO
	I2C_write(REG_FCR, 0x03);
	// Enable receiver
	I2C_write(REG_EFCR, 0x04);

	do {
		i = 0;
		// 10 cycles ~ 1ms (?)
		while (!(I2C_read(REG_LSR) & BIT(0)) && i < timeout)
			i++;
		if (i == timeout)
			break;
		if (enter) {
			timeout = RX_MAX_WAIT;
			enter = false;
		}

		rxlvl = I2C_read(REG_RXLVL);
		if (tc + rxlvl > size) {
			rxlvl = size - tc;
			loop = false;
		}
		I2C_readArray(REG_FIFO, ptr8 + tc, rxlvl);
		tc += rxlvl;
	} while (loop);

	// Disable transmitter and receiver
	I2C_write(REG_EFCR, 0x06);
	// Disable FIFO
	I2C_write(REG_FCR, 0);

	return tc;
}
