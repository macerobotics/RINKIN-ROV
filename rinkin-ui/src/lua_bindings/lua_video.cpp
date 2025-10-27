#include <lua.hpp>

static int lua_video_new(lua_State *L) {
    const char *url = luaL_checkstring(L, 1);
    int width = luaL_checkinteger(L, 2);
    if(width <= 0) luaL_error(L, "invalid video width");
    int height = luaL_checkinteger(L, 3);
    if(height <= 0) luaL_error(L, "invalid video height");
    #error TODO
    return 1;
}

int lua_open_video(lua_State *L) {
    luaL_newmetatable(L, "video");
    const struct luaL_Reg video_lib[] {
        {"new", lua_video_new},


        {nullptr, nullptr},
    };
    luaL_newlib(L, video_lib);
    return 1;
}