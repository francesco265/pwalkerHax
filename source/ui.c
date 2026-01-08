#include "ui.h"
#include "updates.h"
#include <stdlib.h>

void call_poke_add_watts();
void call_poke_gift_pokemon();
void call_poke_gift_item();

// Add watts menu
menu_entry add_watts_menu_entries[] = {
	{"Enter watts to add", ENTRY_NUMATTR, .num_attr = {.value = 100, .min = 1, .max = 65535}},
	{"Set today steps (optional)", ENTRY_NUMATTR, .num_attr = {.value = 0, .min = 0, .max = 99999}},
	{"Set total steps to max", ENTRY_SELATTR, .sel_menu = {.options = yn_list, .props = {.len = 2, .selected = 0}}},
	{"Add watts and set steps", ENTRY_ACTION, .callback = call_poke_add_watts},
};

menu add_watts_menu = {
	.title = "Add watts and steps",
	.entries = add_watts_menu_entries,
	.props = {.len = sizeof(add_watts_menu_entries) / sizeof(add_watts_menu_entries[0]), .selected = 0},
};

// Gift item menu
menu_entry gift_item_menu_entries[] = {
	{"Select item", ENTRY_SELATTR, .sel_menu = {.options = item_list, .props = {.len = sizeof(item_list) / sizeof(item_list[0]), .selected = 0}}},
	{"Send item", ENTRY_ACTION, .callback = call_poke_gift_item},
};

menu gift_item_menu = {
	.title = "Gift item",
	.entries = gift_item_menu_entries,
	.props = {.len = sizeof(gift_item_menu_entries) / sizeof(gift_item_menu_entries[0]), .selected = 0},
};

// Gift Pokemon menu
menu_entry gift_pokemon_menu_entries[] = {
	{"Select Pokemon", ENTRY_SELATTR, .sel_menu = {.options = poke_list, .props = {.len = sizeof(poke_list) / sizeof(poke_list[0]), .selected = 0}}},
	{"Select held item", ENTRY_SELATTR, .sel_menu = {.options = item_list, .props = {.len = sizeof(item_list) / sizeof(item_list[0]), .selected = 0}}},
	{"Select move 1", ENTRY_SELATTR, .sel_menu = {.options = move_list, .props = {.len = sizeof(move_list) / sizeof(move_list[0]), .selected = 0}}},
	{"Select move 2", ENTRY_SELATTR, .sel_menu = {.options = move_list, .props = {.len = sizeof(move_list) / sizeof(move_list[0]), .selected = 0}}},
	{"Select move 3", ENTRY_SELATTR, .sel_menu = {.options = move_list, .props = {.len = sizeof(move_list) / sizeof(move_list[0]), .selected = 0}}},
	{"Select move 4", ENTRY_SELATTR, .sel_menu = {.options = move_list, .props = {.len = sizeof(move_list) / sizeof(move_list[0]), .selected = 0}}},
	{"Select level", ENTRY_NUMATTR, .num_attr = {.value = 50, .min = 1, .max = 100}},
	{"Gender", ENTRY_SELATTR, .sel_menu = {.options = gender_list, .props = {.len = 2, .selected = 0}}},
	{"Shiny", ENTRY_SELATTR, .sel_menu = {.options = yn_list, .props = {.len = 2, .selected = 0}}},
	{"Select ability", ENTRY_SELATTR, .sel_menu = {.options = ability_list, .props = {.len = sizeof(ability_list) / sizeof(ability_list[0]), .selected = 0}}},
	{"Send Pokemon", ENTRY_ACTION, .callback = call_poke_gift_pokemon},
};

menu gift_pokemon_menu = {
	.title = "Gift Pokemon",
	.entries = gift_pokemon_menu_entries,
	.props = {.len = sizeof(gift_pokemon_menu_entries) / sizeof(gift_pokemon_menu_entries[0]), .selected = 0},
};
//
// Main menu
menu_entry main_menu_entries[] = {
	{"Get Pokewalker info", ENTRY_ACTION, .callback = poke_get_data},
	{"Add watts and steps", ENTRY_CHANGEMENU, .new_menu = &add_watts_menu},
	{"Gift Pokemon", ENTRY_CHANGEMENU, .new_menu = &gift_pokemon_menu},
	{"Gift item", ENTRY_CHANGEMENU, .new_menu = &gift_item_menu},
	{"Dump ROM", ENTRY_ACTION, .callback = poke_dump_rom},
	{"Dump EEPROM (user data)", ENTRY_ACTION, .callback = poke_dump_eeprom}
};

menu main_menu = {
	.title = "Main menu",
	.entries = main_menu_entries,
	.props = {.len = sizeof(main_menu_entries) / sizeof(main_menu_entries[0]), .selected = 0},
};


// Currently active menu
static menu *g_active_menu = &main_menu;
static enum state g_state = IN_MENU;
static C3D_RenderTarget *target;
static C2D_TextBuf textbuf;
static PrintConsole logs;

void ui_init()
{
	PrintConsole header;

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	consoleInit(GFX_TOP, &header);
	consoleInit(GFX_TOP, &logs);

	consoleSetWindow(&header, 0, 1, header.consoleWidth, 2);
	consoleSetWindow(&logs, 0, 3, logs.consoleWidth, logs.consoleHeight - 3);

	consoleSelect(&header);
	printf("pwalkerHax v%s\n---", VER);
	consoleSelect(&logs);

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
	x = x < 0 ? SCREEN_WIDTH - text.width * scale + x : x;
	C2D_TextOptimize(&text);
	C2D_DrawText(&text, C2D_WithColor | flags, x, y, 0.0f, scale, scale, COLOR_TEXT);
}

void draw_top(const char *str)
{
	C2D_DrawRectSolid(0, 0, 0, SCREEN_WIDTH, 30, COLOR_BG2);
	C2D_DrawRectSolid(0, 28, 0, SCREEN_WIDTH, 2, C2D_Color32(0, 0, 0, 255));
	draw_string(0, 5, 20, str, true, 0);

}
void draw_scrollbar(u16 first, u16 last, u16 total)
{
	float height = SCREEN_HEIGHT - 36;
	float width = 10;
	float scroll_start = ceil(((height - 4) / total) * first);
	float scroll_height = ceil(((height - 4) / total) * (last - first + 1));

	// The scrollbar has a width of 10 and is placed 3 pixels from the right edge
	C2D_DrawRectSolid(SCREEN_WIDTH - width - 3, 33, 0, width, height, COLOR_SB2);
	C2D_DrawRectSolid(SCREEN_WIDTH - 8 - 4, 35 + scroll_start, 0, 8, scroll_height, COLOR_SB1);
}

void draw_menu(u16 font_size, u16 padding, menu_properties props)
{
	u16 avail_lines, cur, line, first, draw_start, height;
	char strbuf[20];
	selection_menu *sel_menu;

	if (g_state == IN_SELECTION)
		draw_top(g_active_menu->entries[g_active_menu->props.selected].text);
	else
		draw_top(g_active_menu->title);

	height = font_size + padding * 2;
	avail_lines = (SCREEN_HEIGHT - 30) / height;
	cur = props.selected - (avail_lines / 2) > 0 ? props.selected - (avail_lines / 2) : 0;
	draw_start = 30 + (SCREEN_HEIGHT - 30 - avail_lines * height) / 2;

	if ((props.len - cur) < avail_lines)
		cur = props.len - avail_lines > 0 ? props.len - avail_lines : 0;
	first = cur;

	line = 0;
	while (cur < props.len && line < avail_lines) {
		if (cur == props.selected) {
			u16 w = props.len > avail_lines ? 19 : 6;
			C2D_DrawRectSolid(3,
					draw_start + padding - (int) (padding / 2), 0,
					SCREEN_WIDTH - w,
					font_size + 2 * (int) (padding / 2), COLOR_SEL);
		}

		if (g_state == IN_SELECTION) {
			sel_menu = &g_active_menu->entries[g_active_menu->props.selected].sel_menu;
			sprintf(strbuf, "%03d", cur);
			draw_string(6, draw_start + padding, font_size, strbuf, false, 0);
			draw_string(0, draw_start + padding, font_size, sel_menu->options[cur], true, 0);
		} else {
			draw_string(15, draw_start + padding, font_size, g_active_menu->entries[cur].text, false, 0);

			switch (g_active_menu->entries[cur].type) {
				case ENTRY_SELATTR:
					draw_string(-21, draw_start + padding, font_size, g_active_menu->entries[cur].sel_menu.options[g_active_menu->entries[cur].sel_menu.props.selected], false, 0);
					break;
				case ENTRY_NUMATTR:
					sprintf(strbuf, "%d", g_active_menu->entries[cur].num_attr.value);
					draw_string(-21, draw_start + padding, font_size, strbuf, false, 0);
					break;
			}
		}

		cur++;
		line++;
		draw_start += height;
	}

	if (props.len > avail_lines)
		draw_scrollbar(first, cur - 1, props.len);
}

s32 numpad_input(const char *hint_text, u8 digits)
{
	char buf[32];
	SwkbdState swkbd;
	SwkbdButton button = SWKBD_BUTTON_NONE;

	swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, digits);
	swkbdSetHintText(&swkbd, hint_text);
	swkbdSetValidation(&swkbd, SWKBD_NOTBLANK_NOTEMPTY, 0, 0);
	swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
	button = swkbdInputText(&swkbd, buf, sizeof(buf));

	return button == SWKBD_BUTTON_RIGHT ? atoi(buf) : -1;
}

// menu_entry must be of type ENTRY_SELATTR
void goto_item(menu_entry *entry)
{
	char str[] = "Go to item";
	s32 value = numpad_input(str, 3);

	if (value >= 0 && value < entry->sel_menu.props.len)
		entry->sel_menu.props.selected = value;
}

// menu_entry must be of type ENTRY_NUMATTR
void set_numattr(menu_entry *entry)
{
	char strbuf[64];
	s32 value;

	sprintf(strbuf, "%s (min %d, max %d)", entry->text, entry->num_attr.min, entry->num_attr.max);
	value = numpad_input(strbuf, 5);

	if (value != -1) {
		value = value > entry->num_attr.max ? entry->num_attr.max : value;
		value = value < (s16) entry->num_attr.min ? entry->num_attr.min : value;
		entry->num_attr.value = value;
	}
}

void call_poke_add_watts()
{
	u16 watts = g_active_menu->entries[0].num_attr.value;
	u32 today_steps = g_active_menu->entries[1].num_attr.value;
	bool max_steps = g_active_menu->entries[2].sel_menu.props.selected == 1;
	printf("Adding %d watts\n", watts);
	poke_add_watts(watts, today_steps, max_steps);
}

void call_poke_gift_pokemon()
{
	pokemon_data poke_data = { 0 };
	pokemon_extradata poke_extra = { 0 };

	poke_data.poke = g_active_menu->entries[0].sel_menu.props.selected;
	poke_data.held_item = g_active_menu->entries[1].sel_menu.props.selected;
	for (u8 i = 0; i < 4; i++)
		poke_data.moves[i] = g_active_menu->entries[i + 2].sel_menu.props.selected;
	poke_data.level = g_active_menu->entries[6].num_attr.value;
	poke_data.variants = g_active_menu->entries[7].sel_menu.props.selected == 1 ? 0x20 : 0;
	poke_data.flags = g_active_menu->entries[8].sel_menu.props.selected == 1 ? 0x02 : 0;

	poke_extra.location_met = 2008; // Distant land
	poke_extra.pokeball_type = 4;	// Pokeball
	poke_extra.ability = g_active_menu->entries[9].sel_menu.props.selected;

	if (!poke_data.poke || !poke_data.moves[0]) {
		printf("Please select a Pokemon and at least one move\n");
		return;
	}
	poke_gift_pokemon(poke_data, poke_extra);
}

void call_poke_gift_item() {
	u16 item = g_active_menu->entries[0].sel_menu.props.selected;

	if (!item) {
		printf("Please select an item\n");
		return;
	}
	poke_gift_item(item);
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

void ui_draw()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

	C2D_TargetClear(target, COLOR_BG);
	C2D_SceneBegin(target);

	if (g_state == IN_SELECTION)
		draw_menu(12, 3, g_active_menu->entries[g_active_menu->props.selected].sel_menu.props);
	else
		draw_menu(15, 5, g_active_menu->props);

	C3D_FrameEnd(0);
}

enum operation ui_update()
{
	menu_entry *selected_entry = &g_active_menu->entries[g_active_menu->props.selected];
	static u16 old_selected = 0;

	gspWaitForVBlank();
	hidScanInput();
	u32 kDown = hidKeysDown() | (hidKeysDownRepeat() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT));

	if (kDown) {
		if (kDown & KEY_START) {
			return OP_EXIT;
		} else if (kDown & KEY_UP) {
			move_selection(-1);
		} else if (kDown & KEY_DOWN) {
			move_selection(1);
		} else if (kDown & KEY_LEFT) {
			move_selection(-10);
		} else if (kDown & KEY_RIGHT) {
			move_selection(10);
		} else if (kDown & KEY_SELECT && updates_available()) {
			printf("Downloading new version...\n");
			if (updates_download())
				printf("New version of pwalkerHax downloaded!\nDelete this version and open the new one!\n");
			else
				printf("Download failed!\n");
		} else if (kDown & KEY_Y && g_state == IN_SELECTION) {
			goto_item(selected_entry);
		} else if (kDown & KEY_A) {
			if (g_state == IN_SELECTION) {
				// We are in a selection menu
				g_state = IN_MENU;
				old_selected = 0;
			} else {
				switch (selected_entry->type) {
					case ENTRY_ACTION:
						selected_entry->callback();
						break;
					case ENTRY_CHANGEMENU:
						g_active_menu = selected_entry->new_menu;
						g_active_menu->props.selected = 0;
						consoleClear();
						break;
					case ENTRY_SELATTR:
						old_selected = selected_entry->sel_menu.props.selected;
						g_state = IN_SELECTION;
						break;
					case ENTRY_NUMATTR:
						set_numattr(selected_entry);
						break;
				}
			}
		} else if (kDown & KEY_B) {
			if (g_state == IN_SELECTION) {
				selected_entry->sel_menu.props.selected = old_selected;
				g_state = IN_MENU;
				old_selected = 0;
			} else {
				g_active_menu = &main_menu;
				consoleClear();
			}
		} 
		return OP_UPDATE;
	}
	return OP_NONE;
}

