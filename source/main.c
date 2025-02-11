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

	printf("pwalkerHax v0.1\n\n");
	ui_draw();
	while (aptMainLoop()) {
		op = ui_update();
		if (op == OP_EXIT)
			break;
		else if (op == OP_UPDATE)
			ui_draw();
	}

	ui_exit();
	gfxExit();
	return 0;
}
