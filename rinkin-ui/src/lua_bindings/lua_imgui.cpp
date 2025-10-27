#include <imgui.h>
#include "lua_imgui.h"

static int lua_imgui_begin(lua_State *L) {
    const char *title = luaL_checkstring(L, 1);
    lua_pushboolean(L, ImGui::Begin(title));
    return 1;
}

static int lua_imgui_end(lua_State *L) {
    ImGui::End();
    return 0;
}

static int lua_imgui_text(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    ImGui::Text("%s", text);
    return 0;
}

static int lua_imgui_button(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    lua_pushboolean(L, ImGui::Button(label));
    return 1;
}

static int lua_imgui_slider_int(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    int v = luaL_checkinteger(L, 2);
    const int v_min = luaL_checkinteger(L, 3);
    const int v_max = luaL_checkinteger(L, 4);
    bool modified = ImGui::SliderInt(label, &v, v_min, v_max);
    lua_pushinteger(L, v);
    lua_pushboolean(L, modified);
    return 2;
}

int lua_open_imgui(lua_State *L) {
    const struct luaL_Reg ImGuiLib[] = {
        {"Begin", lua_imgui_begin},
        {"End", lua_imgui_end},
        {"Text", lua_imgui_text},
        {"Button", lua_imgui_button},
        {"SliderInt", lua_imgui_slider_int},
        {nullptr, nullptr},
    };
    luaL_newlib(L, ImGuiLib);
    return 1;
}