#include <lua.hpp>
#include "lua_video.h"
#include "lua_imgui.h"
#include "lua_udp_client.h"

void lua_register_bindings(struct lua_State *L) {
    luaL_requiref(L, "video", lua_open_video, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "ImGui", lua_open_imgui, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "udp_client", lua_open_udp_client, 1);
    lua_pop(L, 1);
}