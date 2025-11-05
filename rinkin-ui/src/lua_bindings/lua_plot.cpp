#include <cstdlib>
#include <lua.hpp>
#include <implot.h>
#include "lua_plot.h"

static const char metatable_name[] = "plot";

struct Plot {
    char *name;
    float *x, *y;
    size_t size;
};

static Plot *checkplot(lua_State *L, int n) {
    void *ud = luaL_checkudata(L, n, metatable_name);
    luaL_argcheck(L, ud != NULL, n, "`plot' expected");
    return (Plot*)ud;
}

static int lua_plot_new(lua_State *L) {
    size_t len;
    const char *name = luaL_checklstring(L, 1, &len);
    size_t size = luaL_checkinteger(L, 2);
    Plot *p = (Plot*)lua_newuserdata(L, sizeof(Plot));
    p->name = (char*)malloc(len + 1);
    if(!p->name) luaL_error(L, "failed to allocate memory");
    strcpy(p->name, name);
    p->x = (float*)malloc(sizeof(float) * size);
    if(!p->x) luaL_error(L, "failed to allocate memory");
    p->y = (float*)malloc(sizeof(float) * size);
    if(!p->y) luaL_error(L, "failed to allocate memory");
    for(size_t i = 0; i < size; i++) {
        p->x[i] = i;
        p->y[i] = 0.0f;
    }
    p->size = size;
    luaL_getmetatable(L, metatable_name);
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_plot_begin(lua_State *L) {
    const char *title_id = luaL_checkstring(L, 1); 
    lua_pushboolean(L, ImPlot::BeginPlot(title_id));
    return 1;
}

static int lua_plot_end(lua_State *L) {
    ImPlot::EndPlot();
    return 0;
}

static const char *conds[] = {"always", "once", nullptr};

static int lua_plot_x_axis_limits(lua_State *L) {
    LUA_NUMBER xmin = luaL_checknumber(L, 1);
    LUA_NUMBER xmax = luaL_checknumber(L, 2);
    int i_cond = luaL_checkoption(L, 3, "once", conds);
    ImPlot::SetupAxesLimits(ImAxis_X1, xmin, xmax, i_cond + 1);
    return 0;
}

static int lua_plot_y_axis_limits(lua_State *L) {
    LUA_NUMBER ymin = luaL_checknumber(L, 1);
    LUA_NUMBER ymax = luaL_checknumber(L, 2);
    int i_cond = luaL_checkoption(L, 3, "once", conds);
    ImPlot::SetupAxesLimits(ImAxis_Y1, ymin, ymax, i_cond + 1);
    return 0;
}

static int lua_plot_append(lua_State *L) {
    Plot *p = checkplot(L, 1);
    float y = luaL_checknumber(L, 2);
    for(size_t i = 0; i < p->size - 1; i++)
        p->y[i] = p->y[i + 1];
    p->y[p->size - 1] = y;
    return 0;
}

static int lua_plot_display(lua_State *L) {
    Plot *p = checkplot(L, 1);
    ImPlot::PlotLine(p->name, p->x, p->y, p->size, 0, 0);
    return 0;
}

static int lua_plot_gc(lua_State *L) {
    Plot *p = checkplot(L, 1);
    free(p->x);
    free(p->y);
    return 0;
}

static const struct luaL_Reg plot_lib[] = {
    {"new", lua_plot_new},
    {"Begin", lua_plot_begin},
    {"End", lua_plot_end},
    {"x_axis_limits", lua_plot_x_axis_limits},
    {"y_axis_limits", lua_plot_y_axis_limits},
    {nullptr, nullptr},
};

static const struct luaL_Reg plot_lib_m[] = {
    {"append", lua_plot_append},
    {"display", lua_plot_display},
    {"__gc", lua_plot_gc},
    {nullptr, nullptr},
};

int lua_open_plot(struct lua_State *L) {
    luaL_newmetatable(L, metatable_name);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, plot_lib_m, 0);
    lua_pop(L, 2);
    luaL_newlib(L, plot_lib);
    return 1;
}
