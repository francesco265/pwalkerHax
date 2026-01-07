#include "ir.h"
#include "i2c.h"
#include <stdbool.h>
#include <3ds/types.h>

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


void ir_setbitrate(u16 value)
{
	// Disable transmitter and receiver
	I2C_write(REG_EFCR, BIT(1) | BIT(2));
	// Clear and disable FIFOs
	I2C_write(REG_FCR, BIT(1) | BIT(2));
	svcSleepThread(2 * 1000 * 1000); // Don't know if this is really needed

	ir_enable();

	// u8 lcr = I2C_read(REG_LCR);
	// Enable access to DLL and DLH and set 8n1
	I2C_write(REG_LCR, 0x03 | BIT(7));
	I2C_write(REG_DLL, value & 0xFF);
	I2C_write(REG_DLH, (value >> 8) & 0xFF);
	I2C_write(REG_LCR, 0x03);

	svcSleepThread(2 * 1000 * 1000); // Don't know if this is really needed

	// Read DLL and DLH, in Nintendo ir module they do this
	I2C_write(REG_LCR, 0x03 | BIT(7));
	I2C_read(REG_DLL);
	I2C_read(REG_DLH);
	I2C_write(REG_LCR, 0x03);

	// Re-write it
	I2C_write(REG_LCR, 0x03 | BIT(7));
	I2C_write(REG_DLL, value & 0xFF);
	I2C_write(REG_DLH, (value >> 8) & 0xFF);
	I2C_write(REG_LCR, 0x03);
	
	ir_disable();
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
	bool loop = true;

	// Reset and enable RX FIFO
	I2C_write(REG_FCR, 0x03);
	// Enable receiver
	I2C_write(REG_EFCR, 0x04);

	do {
		i = 0;
		// 10 cycles ~ 1ms (?)
		while (!(rxlvl = I2C_read(REG_RXLVL)) && i < timeout)
			i++;
		if (i == timeout)
			break;
		timeout = RX_MAX_WAIT;

		if (tc + rxlvl > size) {
			rxlvl = size - tc;
			loop = false;
		}
		I2C_readArray(REG_FIFO, ptr8 + tc, rxlvl);
		tc += rxlvl;
	} while (loop && tc < 136);

	// Disable transmitter and receiver
	I2C_write(REG_EFCR, 0x06);
	// Disable FIFO
	I2C_write(REG_FCR, 0);

	return tc;
}
