#include <string.h>
#include <atomic>
#include <queue>
#include <mutex>
#include <string>
#include <thread>
#include <errno.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <lua.hpp>
#include "lua_udp_client.h"

#ifndef _WIN32
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
#define closesocket close
#endif

static const char metatable_name[] = "udp_client";

struct UDPClient {
    SOCKET sock;
    SOCKADDR_IN in_addr;
    std::atomic_bool is_running;
    std::queue<std::string> queue;
    std::mutex queue_mutex;
    std::thread *recv_thread;
};

static UDPClient *checkudp(lua_State *L, int n) {
    void *u = luaL_checkudata(L, n, metatable_name);
    luaL_argcheck(L, u != NULL, n, "`udp_client' expected");
    return (UDPClient*)u;
}

static void thread_func(UDPClient *u) {
    char buffer[1500];
    socklen_t in_addr_size = sizeof(SOCKADDR_IN);
    while(u->is_running) {
        ssize_t bytes = recvfrom(u->sock, buffer, sizeof(buffer), 0, (SOCKADDR*)&u->in_addr, &in_addr_size);
        if(bytes < 0 && errno != EWOULDBLOCK) {
            u->is_running = false;
        } else if(bytes > 0) {
            u->queue_mutex.lock();
            u->queue.push(std::string(buffer, bytes));
            u->queue_mutex.unlock();
        }
    }
}

static int lua_udp_client_new(lua_State *L) {
    const char *ip = luaL_checkstring(L, 1);
    const int port = luaL_checkinteger(L, 2);
    luaL_argcheck(L, port >= 0 && port <= 65535, 2, "invalid port");
    UDPClient *u = (UDPClient*)lua_newuserdata(L, sizeof(UDPClient));
    new (u) UDPClient();
    luaL_getmetatable(L, metatable_name);
    lua_setmetatable(L, -2);
    u->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(u->sock == -1) luaL_error(L, "failed to create socket");
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    #ifdef WIN32
    setsockopt(u->sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    #else
    setsockopt(u->sock, SOL_SOCKET, SO_RCVTIMEO, (void*)&tv, sizeof(tv));
    #endif
    memset(&u->in_addr, 0, sizeof(SOCKADDR_IN));
    if(inet_pton(AF_INET, ip, &u->in_addr.sin_addr) != 1)
        luaL_argcheck(L, false, 1, "invalid ip address");
    u->in_addr.sin_family = AF_INET;
    u->in_addr.sin_port = htons(port);
    u->is_running = true;
    u->recv_thread = new std::thread(thread_func, u);
    return 1;
}

static int lua_udp_client_send(lua_State *L) {
    UDPClient *u = checkudp(L, 1);
    if(u->sock == -1) luaL_error(L, "invalid socket");
    size_t len;
    const char *data = luaL_checklstring(L, 2, &len);
    if(sendto(u->sock, data, len, 0, (SOCKADDR*)&u->in_addr, sizeof(SOCKADDR_IN)) != len)
        luaL_error(L, "failed to send");
    return 0;
}

static int lua_udp_client_available(lua_State *L) {
    UDPClient *u = checkudp(L, 1);
    u->queue_mutex.lock();
    bool r = u->queue.size() > 0;
    u->queue_mutex.unlock();
    lua_pushboolean(L, r);
    return 1;
}

static int lua_udp_client_receive(lua_State *L) {
    UDPClient *u = checkudp(L, 1);
    if(!u->is_running) luaL_error(L, "an error happend (udp thread stopped)");
    u->queue_mutex.lock();
    if(u->queue.size() == 0) {
        u->queue_mutex.unlock();
        luaL_error(L, "no message received");
    }
    std::string r = u->queue.front();
    u->queue.pop();
    u->queue_mutex.unlock();
    lua_pushlstring(L, r.c_str(), r.size());
    return 1;
}

static int lua_udp_client_close(lua_State *L) {
    UDPClient *u = checkudp(L, 1);
    u->is_running = false;
    if(u->recv_thread->joinable()) u->recv_thread->join();
    if(u->sock != -1) {
        closesocket(u->sock);
        u->sock = -1;
    }
    return 0;
}

static int lua_udp_client_gc(lua_State *L) {
    UDPClient *u = checkudp(L, 1);
    u->is_running = false;
    if(u->recv_thread->joinable()) u->recv_thread->join();
    if(u->sock != -1)
        closesocket(u->sock);
    return 0;
}

static const struct luaL_Reg udp_client_lib[] = {
    {"new", lua_udp_client_new},
    {nullptr, nullptr},
};

static const struct luaL_Reg udp_client_lib_m[] = {
    {"send", lua_udp_client_send},
    {"available", lua_udp_client_available},
    {"receive", lua_udp_client_receive},
    {"close", lua_udp_client_close},
    {"__gc", lua_udp_client_gc},
    {nullptr, nullptr},
};


int lua_open_udp_client(lua_State *L) {
    luaL_newmetatable(L, metatable_name);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, udp_client_lib_m, 0);
    lua_pop(L, 2);
    luaL_newlib(L, udp_client_lib);
    return 1;
}