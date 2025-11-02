#include <string.h>
#include <string>
#include <errno.h>
#include <assert.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#endif
#include <lua.hpp>
#include "lua_udp.h"
#include "../util.h"

#ifndef _WIN32
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
#define closesocket close
#endif


static int udp_table_ref = LUA_NOREF;
static SOCKET sock = -1;
static SOCKADDR_IN in_addr;

static int lua_udp_init(lua_State *L) {
    const char *ip = luaL_checkstring(L, 1);
    const int port = luaL_checkinteger(L, 2);
    luaL_argcheck(L, port >= 0 && port <= 65535, 2, "invalid port");
    if(sock != -1) luaL_error(L, "UDP already started");
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1) luaL_error(L, "failed to create socket");
    memset(&in_addr, 0, sizeof(SOCKADDR_IN));
    if(inet_pton(AF_INET, ip, &in_addr.sin_addr) != 1)
        luaL_argcheck(L, false, 1, "invalid ip address");
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(port);
    return 1;
}

static int lua_udp_send(lua_State *L) {
    if(sock == -1) luaL_error(L, "UDP not initialized");
    size_t len;
    const char *data = luaL_checklstring(L, 1, &len);
    if(sendto(sock, data, len, 0, (SOCKADDR*)&in_addr, sizeof(SOCKADDR_IN)) != len)
        luaL_error(L, "failed to send");
    return 0;
}

static bool data_available() {
    if(sock == -1) return false;
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    return select(sock + 1, &rfds, NULL, NULL, &tv) > 0;
}

void lua_udp_callback(lua_State *L) {
    assert(udp_table_ref != LUA_NOREF);
    if(!data_available()) return;
    lua_rawgeti(L, LUA_REGISTRYINDEX, udp_table_ref);
    assert(lua_type(L, -1) == LUA_TTABLE);
    lua_getfield(L, -1, "on_receive");
    if(lua_type(L, -1) == LUA_TFUNCTION) {
        char buffer[1500];
        socklen_t in_addr_size = sizeof(SOCKADDR_IN);
        ssize_t bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (SOCKADDR*)&in_addr, &in_addr_size);
        if(bytes > 0) {
            lua_pushlstring(L, buffer, bytes);
            int ret = lua_pcall(L, 1, 0, 0);
            if(ret != LUA_OK) ERROR("%s", lua_tostring(L, -1));
        }
    }
    lua_settop(L, 0);
}

static int lua_udp_close(lua_State *L) {
    if(sock != -1) {
        closesocket(sock);
        sock = -1;
    }
    return 0;
}

static const struct luaL_Reg udp_lib[] = {
    {"init", lua_udp_init},
    {"send", lua_udp_send},
    {"close", lua_udp_close},
    {nullptr, nullptr},
};

int lua_open_udp(lua_State *L) {
    luaL_newlib(L, udp_lib);
    lua_pushvalue(L, -1);
    udp_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 1;
}