#include "utils.h"
#include <errno.h>

u16 convert_endian16(u16 value)
{
	return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
}

u32 convert_endian32(u32 value)
{
	return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

void xor_data(void *data, u16 size)
{
	u8 *ptr8 = (u8 *) data;

	for (u16 i = 0; i < size; i++)
		ptr8[i] ^= XOR_VALUE;
}

int msleep(int msec)
{
    struct timespec ts;
    int res;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

// https://bulbapedia.bulbagarden.net/wiki/Character_encoding_(Generation_IV)
void decode_string(char *out, const u16 *in)
{
	u8 i;
	u16 cur_char;

	// both out and have size 8
	for (i = 0; in[i] != 0xFFFF; i++) {
		cur_char = in[i];
		// TODO add other characters
		if (cur_char >= 0x0121 && cur_char <= 0x012A)
			out[i] = cur_char - 0xF1;
		else if (cur_char >= 0x012B && cur_char <= 0x0144)
			out[i] = cur_char - 0xEA;
		else if (cur_char >= 0x0145 && cur_char <= 0x015E)
			out[i] = cur_char - 0xE4;
		else
			out[i] = '?';
	}
	out[i] = 0;
}
