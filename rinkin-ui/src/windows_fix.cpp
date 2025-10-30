#ifdef _WIN32

#include <winsock2.h>
#include "windows_fix.h"
#include "util.h"

void windows_networking_init() {
    WSADATA wsa;
	if(WSAStartup(MAKEWORD(2, 2), &wsa) < 0)
		FATAL("WSAStartup failed");
}

void windows_networking_cleanup() {
    WSACleanup();
}

#endif