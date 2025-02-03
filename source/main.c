#include "ir.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>

int main(int argc, char* argv[])
{
	enum operation op;

	gfxInitDefault();

	ui_init();
	ir_init();

	draw_frame();
	while (aptMainLoop()) {
		op = update_ui();
		if (op == OP_EXIT)
			break;
		else if (op == OP_UPDATE)
			draw_frame();
	}

	ui_exit();
	gfxExit();
	return 0;
}

