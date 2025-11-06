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

static int lua_imgui_begin_child(lua_State *L) {
    const char *str_id = luaL_checkstring(L, 1);
    int vx = luaL_optinteger(L, 2, 0);
    int vy = luaL_optinteger(L, 3, 0);
    int child_flags = luaL_optinteger(L, 4, 0);
    int window_flags = luaL_optinteger(L, 5, 0);
    bool r = ImGui::BeginChild(str_id, ImVec2(vx, vy), child_flags, window_flags);
    lua_pushboolean(L, r);
    return 1;
}

static int lua_imgui_end_child(lua_State *L) {
    ImGui::EndChild();
    return 0;
}

static int lua_imgui_begin_group(lua_State *L) {
    ImGui::BeginGroup();
    return 0;
}

static int lua_imgui_end_group(lua_State *L) {
    ImGui::EndGroup();
    return 0;
}

static int lua_imgui_set_newt_window_pos(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    ImGui::SetNextWindowPos(ImVec2(x, y));
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

static int lua_imgui_sameline(lua_State *L) {
    ImGui::SameLine();
    return 0;
}

static int lua_imgui_separator_text(lua_State *L) {
    const char *label = luaL_checkstring(L, 1);
    ImGui::SeparatorText(label);
    return 0;
}

int lua_open_imgui(lua_State *L) {
    const struct luaL_Reg ImGuiLib[] = {
        {"Begin", lua_imgui_begin},
        {"End", lua_imgui_end},
        {"BeginChild", lua_imgui_begin_child},
        {"EndChild", lua_imgui_end_child},
        {"BeginGroup", lua_imgui_begin_group},
        {"EndGroup", lua_imgui_end_group},
        {"SetNextWindowPos", lua_imgui_set_newt_window_pos},
        {"Text", lua_imgui_text},
        {"Button", lua_imgui_button},
        {"SliderInt", lua_imgui_slider_int},
        {"SameLine", lua_imgui_sameline},
        {"SeparatorText", lua_imgui_separator_text},
        {nullptr, nullptr},
    };
    luaL_newlib(L, ImGuiLib);
    return 1;
}