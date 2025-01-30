#pragma once

#include "main.h"
#include "pokewalker.h"

typedef struct {
	const u16 len;
	u16 selected;
} menu_properties;

typedef struct {
	menu_properties props;
	const char **options;
} selection_menu;

typedef struct {
	const char *text;
	const bool is_selection;
	union {
		void (*callback)(void);
		selection_menu sel_menu;
	};
} menu_entry;

typedef struct {
	menu_properties props;
	const char *title;
	menu_entry *entries;
} menu;

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
