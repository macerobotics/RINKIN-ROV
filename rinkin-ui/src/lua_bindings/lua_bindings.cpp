#include <lua.hpp>
#include "lua_mqtt.h"
#include "lua_video.h"

void lua_register_bindings(struct lua_State *L) {
    luaL_requiref(L, "mqtt", lua_open_mqtt, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "video", lua_open_video, 1);
    lua_pop(L, 1);
}