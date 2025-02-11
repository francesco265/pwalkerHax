#pragma once

#include <3ds.h>

#define XOR_VALUE 0xAA

u16 swap16(u16 value);
u32 swap32(u32 value);
void xor_data(void *data, u16 size);
void decode_string(char *out, const u16 *in);
void string_to_img(void *dst, const u8 width, const char *str, bool centered);
void string_upper(char *dst, const char *src);
int msleep(const int msec);
