#include "updates.h"
#include "utils.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char latest_version[16];
bool new_version_available = false;

u16 updates_parse_string(const char *str)
{
	u16 loc = 0;

	for (int i = 0; str[i]; i++) {
		if (str[i] == '/')
			loc = i;
	}

	return ++loc;
}

bool updates_download()
{
	char download_url[512];
	u32 status_code;
	httpcContext context;
	Result ret;

	httpcInit(0);
	sprintf(download_url, REPO_URL_DL, latest_version);

	do {
		httpcOpenContext(&context, HTTPC_METHOD_GET, download_url, 0);

		httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
		httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
		httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
		httpcBeginRequest(&context);

		httpcGetResponseStatusCode(&context, &status_code);
		if ((status_code >= 301 && status_code <= 303) || (status_code >= 307 && status_code <= 308)) {
			httpcGetResponseHeader(&context, "Location", download_url, sizeof(download_url));
			httpcCloseContext(&context);
		}
	} while ((status_code >= 301 && status_code <= 303) || (status_code >= 307 && status_code <= 308));

	if (status_code != 200) {
		httpcCloseContext(&context);
		httpcExit();
		return false;
	}

	char filename[64];
	sprintf(filename, "pwalkerHax_%s.3dsx", latest_version);

	FILE *fp = fopen(filename, "wb");
	u8 *buffer = (u8 *) malloc(0x2000);
	u32 total, current = 0, downloaded = 0;

	if (!fp || !buffer) {
		httpcCloseContext(&context);
		httpcExit();
		return false;
	}

	httpcGetDownloadSizeState(&context, NULL, &total);
	do {
		ret = httpcDownloadData(&context, buffer, 0x2000, &downloaded);
		fwrite(buffer, 1, downloaded, fp);
		current += downloaded;
		progress_bar(current, total, 25);
	} while (ret == HTTPC_RESULTCODE_DOWNLOADPENDING);

	fclose(fp);
	free(buffer);
	httpcCloseContext(&context);
	httpcExit();
	return current == total;
}

void updates_check(const char* current_version)
{
	char buf[256];
	u32 status_code;
	httpcContext context;

	httpcInit(0);
	httpcOpenContext(&context, HTTPC_METHOD_GET, REPO_URL, 0);

	httpcBeginRequest(&context);
	httpcGetResponseStatusCode(&context, &status_code);

	if (status_code == 302) {
		httpcGetResponseHeader(&context, "Location", buf, sizeof(buf));
		strcpy(latest_version, buf + updates_parse_string(buf));
		if (strcmp(latest_version, current_version) != 0) {
			printf("New version available: %s\n", latest_version);
			printf("Press SELECT to download\n");
			new_version_available = true;
		}
	}

	httpcCloseContext(&context);
	httpcExit();
}

bool updates_available()
{
	return new_version_available;
}
