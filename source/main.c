#include "pokewalker.h"
#include "i2c.h" // TODO remove
#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>

void call_poke_add_watts();

static const char *g_menu_entries[] = { "Get Pokewalker data (test connection)", "Add watts" };
static const u8 g_menu_len = sizeof(g_menu_entries) / sizeof(g_menu_entries[0]);
void (*g_menu_functions[])(void) = { poke_get_data, call_poke_add_watts };
static PrintConsole top, bottom;

void print_menu(u8 selected)
{
	consoleSelect(&bottom);
	consoleClear();
	printf("\nActions:\n\n");
	for (u8 i = 0; i < g_menu_len; i++) {
		if (i == selected)
			printf(" > %s\n", g_menu_entries[i]);
		else
			printf(" - %s\n", g_menu_entries[i]);
	}
	consoleSelect(&top);
}

void call_poke_add_watts()
{
	// TODO add selection of watts using the numpad
	poke_add_watts(100);
}

int main(int argc, char* argv[])
{
	u8 selected = 0;

	gfxInitDefault();
	consoleInit(GFX_TOP, &top);
	consoleInit(GFX_BOTTOM, &bottom);
	ir_init();

	consoleSelect(&top);
	printf("Welcome to pwalkerHax v0.1\n");
	print_menu(selected);

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown) {
			if (kDown & KEY_START)
				break; // break in order to return to hbmenu
			else if (kDown & KEY_UP) {
				selected = selected > 0 ? selected - 1 : 0;
				print_menu(selected);
			} else if (kDown & KEY_DOWN) {
				selected = selected < g_menu_len - 1 ? selected + 1 : selected;
				print_menu(selected);
			} else if (kDown & KEY_A) {
				consoleClear();
				g_menu_functions[selected]();
			} else if (kDown & KEY_X) {
				// TODO remove
				consoleClear();
				// Print I2C registers
				for (u8 reg = 0x08; reg <= 0x78; reg += 0x08) {
					if (reg == 0x68)
						continue;
					u8 val = I2C_read(reg);
					printf("reg: 0x%02X, val: 0x%02X\n", reg, val);
				}
				u8 lcr = I2C_read(0x18);
				I2C_write(0x18, 0xBF);
				printf("efr: 0x%02X\n", I2C_read(0x10));
				for (u8 reg = 0x20; reg <= 0x38; reg += 0x08) {
					u8 val = I2C_read(reg);
					printf("reg: 0x%02X, val: 0x%02X\n", reg, val);
				}
				I2C_write(0x18, lcr);
				printf("-------------\n");
			}
		}
	}

	gfxExit();
	return 0;
}

