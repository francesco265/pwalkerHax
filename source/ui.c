#include "ui.h"
#include "i2c.h" // TODO remove
#include <stdlib.h>

void call_poke_add_watts();
void open_gift_item_menu();
void call_poke_gift_item();

// Main menu
menu_entry main_menu_entries[] = {
	{"Get Pokewalker info", false, .callback = poke_get_data},
	{"Add watts", false, .callback = call_poke_add_watts},
	{"Gift item", false, .callback = open_gift_item_menu},
};

menu main_menu = {
	.title = "Main menu",
	.entries = main_menu_entries,
	.props = {.len = sizeof(main_menu_entries) / sizeof(main_menu_entries[0]), .selected = 0},
};

// Gift item menu
menu_entry gift_item_menu_entries[] = {
	{"Select item", true, .sel_menu = {.options = item_list, .props = {.len = sizeof(item_list) / sizeof(item_list[0]), .selected = 0}}},
	{"Send item", false, .callback = call_poke_gift_item},
};

menu gift_item_menu = {
	.title = "Gift item",
	.entries = gift_item_menu_entries,
	.props = {.len = sizeof(gift_item_menu_entries) / sizeof(gift_item_menu_entries[0]), .selected = 0},
};

// Currently active menu
static menu *g_active_menu = &main_menu;
static enum state g_state = IN_MENU;
static C3D_RenderTarget *target;
static C2D_TextBuf textbuf;

void ui_init()
{
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	consoleInit(GFX_TOP, NULL);

	target = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	textbuf = C2D_TextBufNew(256);
}

void ui_exit()
{
	C2D_TextBufDelete(textbuf);
	C2D_Fini();
	C3D_Fini();
}

void draw_string(float x, float y, float size, const char *str, bool centered, int flags)
{
	C2D_Text text;
	float scale;

	C2D_TextBufClear(textbuf);
	C2D_TextParse(&text, textbuf, str);
	scale = size / 30;
	x = centered ? (SCREEN_WIDTH - text.width * scale) / 2 : x;
	C2D_TextOptimize(&text);
	C2D_DrawText(&text, C2D_WithColor | flags, x, y, 0.0f, scale, scale, COLOR_TEXT);
}

void draw_top(const char *str)
{
	C2D_DrawRectSolid(0, 0, 0, SCREEN_WIDTH, 30, COLOR_BG2);
	C2D_DrawRectSolid(0, 28, 0, SCREEN_WIDTH, 2, C2D_Color32(0, 0, 0, 255));
	draw_string(0, 5, 20, str, true, 0);

}

void draw_menu()
{
	C2D_Text text_dx;
	draw_top(g_active_menu->title);

	// Text starts at y = 30, 15px font height and 5px padding top and bottom
	for (u16 i = 0; i < g_active_menu->props.len; i++) {

		// Highlight selected entry
		if (i == g_active_menu->props.selected)
			C2D_DrawRectSolid(3,
					33.0f + i * 25, 0,
					SCREEN_WIDTH - 6,
					19, COLOR_SEL);
			
		draw_string(15, 35.0f + i * 25, 15, g_active_menu->entries[i].text, false, 0);

		if (g_active_menu->entries[i].is_selection) {
			C2D_TextParse(&text_dx, textbuf, g_active_menu->entries[i].sel_menu.options[g_active_menu->entries[i].sel_menu.props.selected]);
			C2D_TextOptimize(&text_dx);
			C2D_DrawText(&text_dx, C2D_WithColor, SCREEN_WIDTH - text_dx.width * 0.5 - 15, 35.0f + i * 25, 0.0f, 0.5f, 0.5f, COLOR_TEXT);
		}
	}
}

void draw_scrollbar(u16 first, u16 last, u16 total)
{
	float height = SCREEN_HEIGHT - 36;
	float width = 10;
	float scroll_start = ceil(((height - 4) / total) * first);
	float scroll_height = ceil(((height - 4) / total) * (last - first));

	C2D_DrawRectSolid(SCREEN_WIDTH - width - 3, 33, 0, width, height, COLOR_SB2);
	C2D_DrawRectSolid(SCREEN_WIDTH - 8 - 4, 35 + scroll_start, 0, 8, scroll_height, COLOR_SB1);
}

void draw_selection_menu()
{
	u16 avail_lines, cur, line, first, draw_start;
	char strbuf[10];
	selection_menu *sel_menu = &g_active_menu->entries[g_active_menu->props.selected].sel_menu;

	draw_top(g_active_menu->entries[g_active_menu->props.selected].text);

	// 12px font height and 3px padding top and bottom
	avail_lines = (SCREEN_HEIGHT - 30) / 18;
	cur = sel_menu->props.selected - (avail_lines / 2) > 0 ? sel_menu->props.selected - (avail_lines / 2) : 0;
	draw_start = 30 + (SCREEN_HEIGHT - 30 - avail_lines * 18) / 2;

	if ((sel_menu->props.len - cur) < avail_lines)
		cur = sel_menu->props.len - avail_lines > 0 ? sel_menu->props.len - avail_lines : 0;
	first = cur;

	line = 0;
	while (cur < sel_menu->props.len && line < avail_lines) {

		if (cur == sel_menu->props.selected)
			C2D_DrawRectSolid(3,
					draw_start + line * 18, 0,
					SCREEN_WIDTH - 6,
					14, COLOR_SEL);

		sprintf(strbuf, "%03d", cur);
		draw_string(6, draw_start + 2 + line * 18, 12, strbuf, false, 0);
		draw_string(0, draw_start + 2 + line * 18, 12, sel_menu->options[cur], true, 0);

		cur++;
		line++;
	}

	draw_scrollbar(first, cur - 1, sel_menu->props.len);
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

	if (!item) {
		printf("Please select an item\n");
		return;
	}
	
	poke_gift_item(item);
}

void open_gift_item_menu()
{
	g_active_menu = &gift_item_menu;
	g_active_menu->props.selected = 0;
	//print_menu();
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

void draw_frame()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

	C2D_TargetClear(target, COLOR_BG);
	C2D_SceneBegin(target);

	if (g_state == IN_SELECTION)
		draw_selection_menu();
	else
		draw_menu();

	C3D_FrameEnd(0);
}

enum operation update_ui()
{
	static u16 old_selected = 0;

	gspWaitForVBlank();
	hidScanInput();
	u32 kDown = hidKeysDown() | (hidKeysDownRepeat() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT));

	if (kDown) {
		if (kDown & KEY_START) {
			return OP_EXIT;
		} else if (kDown & KEY_UP) {
			move_selection(-1);
			return OP_UPDATE;
		} else if (kDown & KEY_DOWN) {
			move_selection(1);
			return OP_UPDATE;
		} else if (kDown & KEY_LEFT && g_state == IN_SELECTION) {
			move_selection(-10);
			return OP_UPDATE;
		} else if (kDown & KEY_RIGHT && g_state == IN_SELECTION) {
			move_selection(10);
			return OP_UPDATE;
		} else if (kDown & KEY_A) {
			if (g_state == IN_SELECTION) {
				// We are in a selection menu
				g_state = IN_MENU;
				old_selected = 0;
			} else if (g_active_menu->entries[g_active_menu->props.selected].is_selection) {
				// We are about to enter a selection menu
				old_selected = g_active_menu->entries[g_active_menu->props.selected].sel_menu.props.selected;
				g_state = IN_SELECTION;
			} else {
				// We are about to call a function
				g_active_menu->entries[g_active_menu->props.selected].callback();
			}
			return OP_UPDATE;
		} else if (kDown & KEY_B) {
			if (g_state == IN_SELECTION) {
				g_active_menu->entries[g_active_menu->props.selected].sel_menu.props.selected = old_selected;
				g_state = IN_MENU;
				old_selected = 0;
			} else {
				g_active_menu = &main_menu;
				consoleClear();
				printf("pwalkerHax v0.1\n\n");
			}
			return OP_UPDATE;
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
	return OP_NONE;
}

