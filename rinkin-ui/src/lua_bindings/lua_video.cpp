#include <string>
#include <atomic>
#include <lua.hpp>
#include <mpv/client.h>
#include <mpv/render.h>
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>
#include <rlgl.h>
#include "lua_video.h"
#include "../util.h"

static const char metatable_name[] = "video";

struct Video {
    std::string url;
    unsigned int width, height;
    mpv_handle *mpv;
    mpv_render_context *mpv_rd;
    Texture2D texture;
    std::atomic_bool has_events, render_cb_called;
    void *pixels;
};

static Video *checkvideo(lua_State *L) {
    void *ud = luaL_checkudata(L, 1, metatable_name);
    luaL_argcheck(L, ud != NULL, 1, "`video' expected");
    return (Video*)ud;
}

static void video_stop(Video *v) {
    memset(v->pixels, 0, v->width * v->height * 3);
    UpdateTexture(v->texture, v->pixels);
    if(v->mpv_rd) {
        mpv_render_context_free(v->mpv_rd);
        v->mpv_rd = nullptr;
    }
    if(v->mpv) {
        mpv_destroy(v->mpv);
        v->mpv = nullptr;
    }
}

static int lua_video_new(lua_State *L) {
    const char *url = luaL_checkstring(L, 1);
    int width = luaL_checkinteger(L, 2);
    if(width <= 0) luaL_error(L, "invalid video width");
    int height = luaL_checkinteger(L, 3);
    if(height <= 0) luaL_error(L, "invalid video height");
    Video *v = (Video*)lua_newuserdata(L, sizeof(Video));
    luaL_getmetatable(L, metatable_name);
    lua_setmetatable(L, -2);
    v->url = std::string(url);
    v->width = (unsigned int)width;
    v->height = (unsigned int)height;
    v->pixels = calloc(width * height, 3);
    if(!v->pixels) luaL_error(L, "failed to allocate memory");
    v->texture.id = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);
    v->texture.width= width;
    v->texture.height = height;
    v->texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    v->texture.mipmaps = 1;
    return 1;
}

static void mpv_wakeup_cb(void *ctx) {
    Video *v = (Video*)ctx;
    v->has_events = true;
}

static void mpv_render_cb(void * ctx) {
    Video *v = (Video*)ctx;
    v->render_cb_called = true;
}

int lua_video_start(lua_State *L) {
    Video *v = checkvideo(L);
    if(v->mpv) {
        return 0;
    }
    v->mpv = mpv_create();
    if(!v->mpv) luaL_error(L, "failed to initialize mpv");
    mpv_set_option_string(v->mpv, "vo", "libmpv");
    mpv_set_option_string(v->mpv, "profile", "low-latency");
    if(mpv_initialize(v->mpv) < 0)
        luaL_error(L, "failed to initialize mpv");
    mpv_request_log_messages(v->mpv, "debug");
    int advanced_control = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_SW},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advanced_control},
        {(mpv_render_param_type)0, 0},
    };
    if(mpv_render_context_create(&v->mpv_rd, v->mpv, params) < 0) {
        mpv_destroy(v->mpv);
        v->mpv = nullptr;
        luaL_error(L, "failed to initialize MPV context");
    }
    mpv_set_wakeup_callback(v->mpv, mpv_wakeup_cb, v);
    mpv_render_context_set_update_callback(v->mpv_rd, mpv_render_cb, v);

    const char *cmd[] = {"loadfile", v->url.c_str(), nullptr};
    mpv_command_async(v->mpv, 0, cmd);
    
    //libvlc_video_set_format(mp, "RV24", width, height, width * 3);
    //if(libvlc_media_player_play(mp) == -1) {
    //    stop();
    //    return false;
    //}
    return 0;
}

int lua_video_stop(lua_State *L) {
    Video *v = checkvideo(L);
    video_stop(v);
    return 0;
}

int display_frame(Video *v) {
    rlImGuiImage((const Texture*)&v->texture);
    return 0;
}

int lua_video_display(lua_State *L) {
    Video *v = checkvideo(L);
    if(!v->mpv || !v->mpv_rd) {
        return display_frame(v);
    }
    if(v->render_cb_called) {
        v->render_cb_called = false;
        uint64_t flags = mpv_render_context_update(v->mpv_rd);
        if (flags & MPV_RENDER_UPDATE_FRAME) {
            int size[2] = {(int)v->width, (int)v->height};
            size_t pitch = v->width * 3;
            mpv_render_param params[] = {
                {MPV_RENDER_PARAM_SW_SIZE, size},
                {MPV_RENDER_PARAM_SW_FORMAT, (void*)"rgb24"},
                {MPV_RENDER_PARAM_SW_STRIDE, &pitch},
                {MPV_RENDER_PARAM_SW_POINTER, v->pixels},
                {(mpv_render_param_type)0, 0},
            };
            int r = mpv_render_context_render(v->mpv_rd, params);
            if(r < 0) {
                LOG("mpv_render_context_render error: %s\n", mpv_error_string(r));
                video_stop(v);
                return display_frame(v);
            }
            UpdateTexture(v->texture, v->pixels);
        }
        
    }
    if(v->has_events) {
        while(true) {
            mpv_event *e = mpv_wait_event(v->mpv, 0);
            if(e->event_id == MPV_EVENT_NONE)
                break;
        }
        v->has_events = false;
    }
    return display_frame(v);
}

static int lua_video_to_string(lua_State *L) {
    Video *v = checkvideo(L);
    lua_pushfstring(L, "video: \"%s\"", v->url.c_str());
    return 1;
}

static int lua_video_gc(lua_State *L) {
    Video *v = checkvideo(L);
    video_stop(v);
    free(v->pixels);
    return 0;
}

static const struct luaL_Reg video_lib[] = {
    {"new", lua_video_new},
    {nullptr, nullptr},
};

static const struct luaL_Reg video_lib_m[] = {
    {"start", lua_video_start},
    {"stop", lua_video_stop},
    {"display", lua_video_display},
    {"__tostring", lua_video_to_string},
    {"__gc", lua_video_gc},
    {nullptr, nullptr},
};

int lua_open_video(lua_State *L) {
    luaL_newmetatable(L, metatable_name);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_setfuncs(L, video_lib_m, 0);
    
    luaL_newlib(L, video_lib);
    return 1;
}