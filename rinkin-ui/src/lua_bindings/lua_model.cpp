#include <lua.hpp>
#include <raylib.h>
#include <rlImGui.h>

extern float heading, pitch, roll;
extern RenderTexture2D model_texture;

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

int lua_display(lua_State *L) {
    rlImGuiImage((const Texture*)&model_texture.texture);
    return 0;
}

static const struct luaL_Reg model_lib[] = {
    {"set_heading", lua_set_heading},
    {"set_pitch", lua_set_pitch},
    {"set_roll", lua_set_roll},
    {"display", lua_display},
    {nullptr, nullptr},
};

int lua_open_model(lua_State *L) {
    luaL_newlib(L, model_lib);
    return 1;
}