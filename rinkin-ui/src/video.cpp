#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "video.h"
#include <rlgl.h>


void mpv_wakeup_cb_wrapper(void *ctx) {
    Video *v = (Video*)ctx;
    return v->mpv_wakeup_cb();
}

void mpv_render_cb_wrapper(void *ctx) {
    Video *v = (Video*)ctx;
    v->mpv_render_cb();
}

Video::Video(const char *url, unsigned int width, unsigned int height) : width(width), height(height), has_events(false), render_cb_called(false) {
    this->url = strdup(url);
    if(!this->url) {
        fprintf(stderr, "failed to allocate memory");
        exit(1);
    }
    pixels = calloc(3, width * height);
    if(!pixels) {
        free((void*)this->url);
        fprintf(stderr, "failed to allocate memory\n");
        exit(1);
    }
    texture.id = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);
    texture.width= width;
    texture.height = height;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    texture.mipmaps = 1;
}

Video::~Video() {
    free((void*)url);
    stop();
}

bool Video::start() {
    if(mpv) return true;
    mpv = mpv_create();
    if(!mpv) {
        fprintf(stderr, "failed to initialize mpv\n");
        return false;
    }
    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, "profile", "low-latency");
    if(mpv_initialize(mpv) < 0) {
        fprintf(stderr, "failed to initialize mpv\n");
        return false;
    }
    mpv_request_log_messages(mpv, "debug");
    int advanced_control = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_SW},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advanced_control},
        {(mpv_render_param_type)0, 0},
    };
    if(mpv_render_context_create(&mpv_rd, mpv, params) < 0) {
        fprintf(stderr, "failed to initialize MPV context");
        mpv_destroy(mpv);
        mpv = nullptr;
        return false;
    }
    mpv_set_wakeup_callback(mpv, mpv_wakeup_cb_wrapper, this);
    mpv_render_context_set_update_callback(mpv_rd, mpv_render_cb_wrapper, this);

    const char *cmd[] = {"loadfile", url, nullptr};
    mpv_command_async(mpv, 0, cmd);
    
    //libvlc_video_set_format(mp, "RV24", width, height, width * 3);
    //if(libvlc_media_player_play(mp) == -1) {
    //    stop();
    //    return false;
    //}
    return true;
}

void Video::stop() {
    if(mpv_rd) {
        mpv_render_context_free(mpv_rd);
        mpv_rd = nullptr;
    }
    if(mpv) {
        mpv_destroy(mpv);
        mpv = nullptr;
    }
}

ImTextureID Video::get_texture() {
    if(!mpv || !mpv_rd) return (ImTextureID)&texture;
    if(render_cb_called) {
        render_cb_called = false;
        uint64_t flags = mpv_render_context_update(mpv_rd);
        if (flags & MPV_RENDER_UPDATE_FRAME) {
            int size[2] = {(int)width, (int)height};
            size_t pitch = width * 3;
            mpv_render_param params[] = {
                {MPV_RENDER_PARAM_SW_SIZE, size},
                {MPV_RENDER_PARAM_SW_FORMAT, (void*)"rgb24"},
                {MPV_RENDER_PARAM_SW_STRIDE, &pitch},
                {MPV_RENDER_PARAM_SW_POINTER, pixels},
                {(mpv_render_param_type)0, 0},
            };
            int r = mpv_render_context_render(mpv_rd, params);
            if(r < 0) {
                printf("mpv_render_context_render error: %s\n", mpv_error_string(r));
                stop();
                return (ImTextureID)&texture;
            }
            UpdateTexture(texture, pixels);
        }
        
    }
    if(has_events) {
        while(true) {
            mpv_event *e = mpv_wait_event(mpv, 0);
            if(e->event_id == MPV_EVENT_NONE)
                break;
        }
        has_events = false;
    }
    return (ImTextureID)&texture;
}

void Video::mpv_wakeup_cb() {
    has_events = true;
}

void Video::mpv_render_cb() {
    render_cb_called = true;
    
}
