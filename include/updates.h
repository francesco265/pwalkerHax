#pragma once
#include <stdbool.h>

#define REPO_URL "https://github.com/francesco265/pwalkerHax/releases/latest"
#define REPO_URL_DL	"https://github.com/francesco265/pwalkerHax/releases/download/%s/pwalkerHax.3dsx"

void updates_check(const char* cur_version);
bool updates_available();
bool updates_download();
