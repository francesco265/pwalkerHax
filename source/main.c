#include "pokewalker.h"
#include "i2c.h" // TODO remove
#include "ir.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>

// Currently active menu
static menu *g_active_menu = &main_menu;
static enum state g_state = IN_MENU;
static PrintConsole top, bottom;

void print_menu()
{
	consoleSelect(&bottom);
	consoleClear();
	printf("\n%s\n\n", g_active_menu->title);
	for (u16 i = 0; i < g_active_menu->props.len; i++) {
		if (i == g_active_menu->props.selected)
			printf(" > %s", g_active_menu->entries[i].text);
		else
			printf(" - %s", g_active_menu->entries[i].text);

		if (g_active_menu->entries[i].is_selection)
			printf("\t\t[%s]", g_active_menu->entries[i].sel_menu.options[g_active_menu->entries[i].sel_menu.props.selected]);
		printf("\n");
	}
	consoleSelect(&top);
}

void print_selection_menu()
{
	selection_menu *sel_menu = &g_active_menu->entries[g_active_menu->props.selected].sel_menu;

	consoleSelect(&bottom);
	consoleClear();
	printf("\n%s\n\n", g_active_menu->entries[g_active_menu->props.selected].text);

	u16 cur = sel_menu->props.selected - 10 > 0 ? sel_menu->props.selected - 10 : 0;
	u16 line = 0;
	u16 avail_lines = bottom.consoleHeight - bottom.cursorY - 2;
	if ((sel_menu->props.len - cur) < avail_lines)
		cur = sel_menu->props.len - avail_lines > 0 ? sel_menu->props.len - avail_lines : 0;

	if (cur)
		printf("...\n");
	else
		printf("\n");
	while (cur < sel_menu->props.len && line < avail_lines) {

		if (cur == sel_menu->props.selected)
			printf("[%03d] > %s\n", cur, sel_menu->options[cur]);
		else
			printf("[%03d] - %s\n", cur, sel_menu->options[cur]);

		cur++;
		line++;
	}
	if (cur != sel_menu->props.len)
		printf("...\n");
	else
		printf("\n");
	consoleSelect(&top);
}

void call_poke_add_watts()
{
	char watts_str[5];
	u32 watts = 0;
	SwkbdState swkbd;
	SwkbdButton button = SWKBD_BUTTON_NONE;
	
	swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, 5);
	swkbdSetHintText(&swkbd, "Enter watts to add (max 65535)");
	swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
	swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
	button = swkbdInputText(&swkbd, watts_str, sizeof(watts_str));

	if (button == SWKBD_BUTTON_RIGHT) {
		watts = atoi(watts_str);
		watts = watts > 65535 ? 65535 : watts;
		poke_add_watts(watts);
	}
}

void call_poke_gift_item() {
	u16 item = g_active_menu->entries[0].sel_menu.props.selected;

	if (item)
		//poke_gift_item(item);
		printf("%d\n", item);
	else
		printf("Please select an item\n");
}

void open_gift_item_menu()
{
	g_active_menu = &gift_item_menu;
	g_active_menu->props.selected = 0;
	print_menu();
}

void move_selection(const s16 offset)
{
	menu_properties *props;
	s16 new_selected;

	props = g_state == IN_SELECTION ? &g_active_menu->entries[g_active_menu->props.selected].sel_menu.props : &g_active_menu->props;

	new_selected = props->selected + offset;
	if (new_selected >= props->len)
		new_selected = props->len - 1;
	else if (new_selected < 0)
		new_selected = 0;

	props->selected = new_selected;
}

int main(int argc, char* argv[])
{
	u16 old_selected = 0;

	gfxInitDefault();
	consoleInit(GFX_TOP, &top);
	consoleInit(GFX_BOTTOM, &bottom);
	ir_init();

	consoleSelect(&top);
	printf("Welcome to pwalkerHax v0.1\n\n");
	print_menu();

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown() | (hidKeysDownRepeat() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT));
		if (kDown) {
			if (kDown & KEY_START) {
				break; // break in order to return to hbmenu
			} else if (kDown & KEY_UP) {
				move_selection(-1);
				if (g_state == IN_SELECTION)
					print_selection_menu();
				else
					print_menu();
			} else if (kDown & KEY_DOWN) {
				move_selection(1);
				if (g_state == IN_SELECTION)
					print_selection_menu();
				else
					print_menu();
			} else if (kDown & KEY_LEFT) {
				if (g_state == IN_SELECTION) {
					move_selection(-10);
					print_selection_menu();
				}
			} else if (kDown & KEY_RIGHT) {
				if (g_state == IN_SELECTION) {
					move_selection(10);
					print_selection_menu();
				}
			} else if (kDown & KEY_A) {
				if (g_state == IN_SELECTION) {
					// We are in a selection menu
					g_state = IN_MENU;
					old_selected = 0;
					print_menu();
				} else if (g_active_menu->entries[g_active_menu->props.selected].is_selection) {
					// We are about to enter a selection menu
					old_selected = g_active_menu->entries[g_active_menu->props.selected].sel_menu.props.selected;
					g_state = IN_SELECTION;
					print_selection_menu();
				} else {
					// We are about to call a function
					g_active_menu->entries[g_active_menu->props.selected].callback();
				}
			} else if (kDown & KEY_B) {
				if (g_state == IN_SELECTION) {
					g_active_menu->entries[g_active_menu->props.selected].sel_menu.props.selected = old_selected;
					g_state = IN_MENU;
					old_selected = 0;
				} else {
					g_active_menu = &main_menu;
				}
				print_menu();
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

