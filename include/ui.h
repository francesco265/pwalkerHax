#pragma once

#include "pokewalker.h"
#include <citro2d.h>

#define COLOR_BG	C2D_Color32(0xEE, 0x83, 0x29, 0xFF)
#define COLOR_BG2	C2D_Color32(0xCD, 0x52, 0x41, 0xFF)
#define COLOR_SEL	C2D_Color32(0x08, 0x41, 0x52, 0xFF)
#define COLOR_TEXT	C2D_Color32(0xF0, 0xF0, 0xF0, 0xFF)
#define COLOR_SB1	C2D_Color32(0x08, 0x41, 0x52, 0xFF)
#define COLOR_SB2	C2D_Color32(0xD5, 0xD5, 0xD5, 0xFF)

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define VER	"1.2"

void ui_init();
void ui_exit();
void ui_draw();
enum operation ui_update();

enum state {
	IN_MENU,
	IN_SELECTION,
};

enum operation {
	OP_UPDATE,
	OP_EXIT,
	OP_NONE,
};

enum entry_type {
	ENTRY_ACTION,
	ENTRY_CHANGEMENU,
	ENTRY_SELATTR,
	ENTRY_NUMATTR,
};

typedef struct {
	const u16 len;
	u16 selected;
} menu_properties;

typedef struct {
	menu_properties props;
	const char **options;
} selection_menu;

typedef struct menu menu;

typedef struct {
	const char *text;
	const enum entry_type type;
	union {
		void (*callback)(void);
		menu *new_menu;
		selection_menu sel_menu;
		struct {
			u32 value;
			u32 min;
			u32 max;
		} num_attr;
	};
} menu_entry;

typedef struct menu {
	menu_properties props;
	const char *title;
	menu_entry *entries;
} menu;

