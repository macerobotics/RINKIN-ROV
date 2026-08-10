#include <lua.hpp>
#include <raylib.h>

static int lua_gamepad_is_available(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    lua_pushboolean(L, IsGamepadAvailable(n));
    return 1;
}

static int lua_gamepad_get_name(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    lua_pushstring(L, GetGamepadName(n));
    return 1;
}

static int lua_gamepad_is_button_pressed(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    int button = luaL_checkinteger(L, 2);
    lua_pushboolean(L, IsGamepadButtonPressed(n, button));
    return 1;
}

static int lua_gamepad_is_button_down(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    int button = luaL_checkinteger(L, 2);
    lua_pushboolean(L, IsGamepadButtonDown(n, button));
    return 1;
}

static int lua_gamepad_is_button_released(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    int button = luaL_checkinteger(L, 2);
    lua_pushboolean(L, IsGamepadButtonReleased(n, button));
    return 1;
}

static int lua_gamepad_is_button_up(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    int button = luaL_checkinteger(L, 2);
    lua_pushboolean(L, IsGamepadButtonUp(n, button));
    return 1;
}

static int lua_gamepad_get_button_pressed(lua_State *L) {
    lua_pushinteger(L, GetGamepadButtonPressed());
    return 1;
}

static int lua_gamepad_get_axis_count(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    lua_pushinteger(L, GetGamepadAxisCount(n));
    return 1;
}

static int lua_gamepad_get_axis_movement(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    int axis = luaL_checkinteger(L, 2);
    lua_pushnumber(L, GetGamepadAxisMovement(n, axis));
    return 1;
}

static int lua_gamepad_set_vibration(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    float left_motor = luaL_checknumber(L, 2);
    float right_motor = luaL_checknumber(L, 3);
    float duration = luaL_checknumber(L, 4);
    SetGamepadVibration(n, left_motor, right_motor, duration);
    return 0;
}

static const struct luaL_Reg gamepad_lib[] = {
    {"is_available", lua_gamepad_is_available},
    {"get_name", lua_gamepad_get_name},
    {"is_button_pressed", lua_gamepad_is_button_pressed},
    {"is_button_down", lua_gamepad_is_button_down},
    {"is_button_released", lua_gamepad_is_button_released},
    {"is_button_up", lua_gamepad_is_button_up},
    {"get_button_pressed", lua_gamepad_get_button_pressed},
    {"get_axis_count", lua_gamepad_get_axis_count},
    {"get_axis_movement", lua_gamepad_get_axis_movement},
    {"set_vibration", lua_gamepad_set_vibration},
    {nullptr, nullptr},
};


int lua_open_gamepad(struct lua_State *L) {
    luaL_newlib(L, gamepad_lib);
    return 1;
}