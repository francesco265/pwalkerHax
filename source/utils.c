#include "utils.h"
#include "font.h"
#include <string.h>
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

	// both in and out have size 8
	for (i = 0; in[i] != 0xFFFF; i++) {
		cur_char = in[i];
		// TODO add other characters
		if (cur_char >= 0x00A2 && cur_char <= 0x00AB)
			out[i] = cur_char - 0x72;
		else if (cur_char >= 0x00AC && cur_char <= 0x00C5)
			out[i] = cur_char - 0x6B;
		else if (cur_char >= 0x00C6 && cur_char <= 0x00DF)
			out[i] = cur_char - 0x65;
		else if (cur_char >= 0x0121 && cur_char <= 0x012A)
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

void render_string(void *dst, const u8 width, const char *str, u16 x_offset, s16 y_offset, u8 color)
{
	u16 *buf = (u16 *) dst;
	u32 pixdata;

	for (u8 i = 0; str[i]; i++) {
		for (u8 c = 0; c < font_width[str[i] - 0x20]; c++) {
			pixdata = font[str[i] - 0x20][c] | font[str[i] - 0x20][c + 6] << 8;

			// shift on y axis
			if (y_offset > 0)
				pixdata <<= y_offset;
			else if (y_offset < 0)
				pixdata >>= -y_offset;

			pixdata = (pixdata << 8 | (pixdata & 0xFFFF)) & 0xFF00FF;

			// 1bpp to 2bpp conversion
			switch (color) {
				case 1:
					pixdata = pixdata << 8;
					break;
				case 3:
					pixdata = pixdata << 8 | pixdata;
					break;
			}

			buf[x_offset] |= pixdata;
			buf[x_offset + width] |= pixdata >> 16;
			x_offset++;
		}
	}
}

// Converts a string to a 2bpp Nx16 image, dst must point to a buffer of size N * 4
void string_to_img(void *dst, const u8 width, const char *str, bool centered) {
	u8 start_x, pix_strlen = 0;

	// Calculate string length
	for (u8 i = 0; str[i]; i++)
		pix_strlen += font_width[str[i] - 0x20];
	start_x = centered ? (width - pix_strlen) / 2 : 2;

	// Each column needs 4 bytes to encode 16 pixels
	memset(dst, 0, width * 4);
	render_string(dst, width, str, start_x + 1, 0, 1);
	render_string(dst, width, str, start_x + 1, 1, 1);
	render_string(dst, width, str, start_x, 1, 1);
	render_string(dst, width, str, start_x, 0, 3);
}
