#include <lua.hpp>
#include "../mqtt.h"

static int lua_mqtt_connect(lua_State *L) {
    const char *host = luaL_checkstring(L, 1);
    int port = luaL_checkinteger(L, 2);
    mqtt_connect(host, port);
    return 0;
}

static int lua_mqtt_subscribe(lua_State *L) {
    const char *topic = luaL_checkstring(L, 1);
    mqtt_subscribe(topic);
    return 0;
}

static int lua_mqtt_publish(lua_State *L) {
    const char *topic = luaL_checkstring(L, 1);
    const char *payload = luaL_checkstring(L, 2);
    mqtt_publish(topic, payload);
    return 0;
}

int lua_open_mqtt(struct lua_State *L) {
    const struct luaL_Reg mqtt_lib[] {
        {"connect", lua_mqtt_connect},
        {"subscribe", lua_mqtt_subscribe},
        {"publish", lua_mqtt_publish},
        {nullptr, nullptr},
    };
    luaL_newlib(L, mqtt_lib);
    return 1;
}