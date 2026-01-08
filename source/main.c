#include "ir.h"
#include "ui.h"
#include "updates.h"
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>

int main(int argc, char* argv[])
{
	enum operation op;
	s32 prio;

	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

	gfxInitDefault();

	ui_init();
	ir_init();

	ui_draw();
	// Disable updates checking and downloading for now, as it seems that httpc
	// doesn't work anymore on GitHub...
	// threadCreate((ThreadFunc) updates_check, (void *) VER, 1024, prio - 1, -2, true);
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
