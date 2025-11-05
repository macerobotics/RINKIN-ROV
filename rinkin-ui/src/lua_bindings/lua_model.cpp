#include <lua.hpp>
#include <raylib.h>

extern float heading, pitch, roll;

int lua_set_heading(lua_State *L) {
    heading = DEG2RAD * luaL_checknumber(L, 1);
    return 0;
}

int lua_set_pitch(lua_State *L) {
    pitch = DEG2RAD * luaL_checknumber(L, 1);
    return 0;
}

int lua_set_roll(lua_State *L) {
    roll = DEG2RAD * luaL_checknumber(L, 1);
    return 0;
}

static const struct luaL_Reg model_lib[] = {
    {"set_heading", lua_set_heading},
    {"set_pitch", lua_set_pitch},
    {"set_roll", lua_set_roll},
    {nullptr, nullptr},
};

int lua_open_model(lua_State *L) {
    luaL_newlib(L, model_lib);
    return 1;
}