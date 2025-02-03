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

void ui_init();
void ui_exit();
void draw_frame();
enum operation update_ui();

enum state {
	IN_MENU,
	IN_SELECTION,
};

enum operation {
	OP_UPDATE,
	OP_EXIT,
	OP_NONE,
};

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

