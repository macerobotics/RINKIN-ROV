#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <lua.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>
#include <rlgl.h>
#include "lua_video.h"
#include "../util.h"

static const char metatable_name[] = "video";

struct Video {
    Video() : is_running(false) {}
    std::string url;
    unsigned int width, height;
    Texture2D texture;
    // FFmpeg stuff
    AVFormatContext *format_ctx = nullptr;
    AVStream *video_stream = nullptr;
    AVCodecContext *video_codec_ctx = nullptr;
    AVFrame *frame = nullptr, *rgb_frame = nullptr;
    AVPacket *packet = nullptr;
    SwsContext *sws_ctx = nullptr;
    std::atomic_bool is_running;
    std::mutex rgb_frame_mutex;
    std::thread *thread;
};

static Video *checkvideo(lua_State *L) {
    void *ud = luaL_checkudata(L, 1, metatable_name);
    luaL_argcheck(L, ud != NULL, 1, "`video' expected");
    return (Video*)ud;
}

static void video_stop(Video *v) {
    if(v->is_running.exchange(false)) {
        av_frame_free(&v->frame);
        av_frame_free(&v->rgb_frame);
        av_packet_unref(v->packet);
        av_packet_free(&v->packet);
        avcodec_free_context(&v->video_codec_ctx);
        sws_freeContext(v->sws_ctx);
        avformat_close_input(&v->format_ctx);
        std::vector<unsigned char> black(v->width * v->height * 3, 0);
        UpdateTexture(v->texture, black.data());
    }
}

#define LUA_AV_ERROR(x) \
    do { \
        char buf[AV_ERROR_MAX_STRING_SIZE]; \
        av_strerror(x, buf, AV_ERROR_MAX_STRING_SIZE); \
        luaL_error(L, "%s", buf); \
    } while(0)

static int lua_video_new(lua_State *L) {
    const char *url = luaL_checkstring(L, 1);
    int width = luaL_checkinteger(L, 2);
    if(width <= 0) luaL_error(L, "invalid video width");
    int height = luaL_checkinteger(L, 3);
    if(height <= 0) luaL_error(L, "invalid video height");
    Video *v = (Video*)lua_newuserdata(L, sizeof(Video));
    new (v) Video(); // placement new
    luaL_getmetatable(L, metatable_name);
    lua_setmetatable(L, -2);
    v->url = std::string(url);
    v->width = (unsigned int)width;
    v->height = (unsigned int)height;
    v->texture.id = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);
    v->texture.width= width;
    v->texture.height = height;
    v->texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    v->texture.mipmaps = 1;
    return 1;
}

static void thread_func(Video *v) {
    while(v->is_running) {
        while(av_read_frame(v->format_ctx, v->packet) >= 0) {
            if(v->packet->stream_index == v->video_stream->index) {
                int ret = avcodec_send_packet(v->video_codec_ctx, v->packet);
                av_packet_unref(v->packet);
                if(ret < 0) {
                    TraceLog(LOG_ERROR, "error sending packet");
                    continue;
                }
                while(ret >= 0) {
                    ret = avcodec_receive_frame(v->video_codec_ctx, v->frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    v->rgb_frame_mutex.lock();
                    sws_scale(v->sws_ctx, (uint8_t const *const *)v->frame->data, v->frame->linesize, 0,
                              v->frame->height, v->rgb_frame->data, v->rgb_frame->linesize);
                    v->rgb_frame_mutex.unlock();
                }
                break;
            }
        }
    }
}

int lua_video_start(lua_State *L) {
    Video *v = checkvideo(L);
    if(v->is_running) return 0;
//FFmpeg stuff
    AVDictionary *options = nullptr;
    if(av_dict_set(&options, "flags", "nobuffer", 0) < 0) luaL_error(L, "failed to set FFmpeg options");
    if(av_dict_set(&options, "flags", "low_delay", 0) < 0) luaL_error(L, "failed to set FFmpeg options");
    if(av_dict_set(&options, "max_delay", "0", 0) < 0) luaL_error(L, "failed to set FFmpeg options");
    v->format_ctx = avformat_alloc_context();
    int rc = avformat_open_input(&v->format_ctx, v->url.c_str(), nullptr, &options);
    av_dict_free(&options);
    if(rc < 0) LUA_AV_ERROR(rc);

    TraceLog(LOG_INFO, "CODEC: format %s", v->format_ctx->iformat->long_name);

    rc = avformat_find_stream_info(v->format_ctx, nullptr);
    if(rc < 0) LUA_AV_ERROR(rc);

    AVCodecParameters *video_params = nullptr;
    for(unsigned int i = 0; i < v->format_ctx->nb_streams; i++) {
        AVStream *tmp_stream = v->format_ctx->streams[i];
        AVCodecParameters *tmp_params = tmp_stream->codecpar;
        if(tmp_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            v->video_stream = tmp_stream;
            video_params = tmp_params;
            TraceLog(LOG_INFO, "CODEC: Resolution: %d x %d, type: %d", video_params->width, video_params->height, video_params->codec_id);
        }
    }
    if(!v->video_stream) luaL_error(L, "failed to find video stream");

    // DEbug
    TraceLog(LOG_INFO, "CODEC: Resolution: %d x %d, type: %d", video_params->width, video_params->height, video_params->codec_id);

    const AVCodec *video_codec = avcodec_find_decoder(video_params->codec_id);
    if(!video_codec) luaL_error(L, "failed to find video codec");

    TraceLog(LOG_INFO, "CODEC: %s ID %d, Bit rate %ld", video_codec->name, video_codec->id, video_params->bit_rate);
    TraceLog(LOG_INFO, "FPS: %d/%d, TBR: %d/%d, TimeBase: %d/%d", v->video_stream->avg_frame_rate.num,
             v->video_stream->avg_frame_rate.den, v->video_stream->r_frame_rate.num,
             v->video_stream->r_frame_rate.den, v->video_stream->time_base.num, v->video_stream->time_base.den);

    v->video_codec_ctx = avcodec_alloc_context3(video_codec);
    if(!v->video_codec_ctx) luaL_error(L, "failed to allocate video codec context");

    rc = avcodec_parameters_to_context(v->video_codec_ctx, video_params);
    if(rc < 0) LUA_AV_ERROR(rc);

    // Debug
    TraceLog(LOG_INFO, "Codec context: width = %d, height = %d", v->video_codec_ctx->width, v->video_codec_ctx->height);

    rc = avcodec_open2(v->video_codec_ctx, video_codec, NULL);
    if(rc < 0) luaL_error(L, "failed to open codec");

    v->frame = av_frame_alloc();
    if(!v->frame) luaL_error(L, "failed to allocate frame");

    v->packet = av_packet_alloc();
    if(!v->packet) luaL_error(L, "failed to allocate packet");

    TraceLog(LOG_INFO, "Codec context: width = %d, height = %d", v->video_codec_ctx->width, v->video_codec_ctx->height);

    v->sws_ctx = sws_getContext(v->video_codec_ctx->width, v->video_codec_ctx->height, v->video_codec_ctx->pix_fmt,
                             v->width, v->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, 0, 0, 0);
    if(!v->sws_ctx) luaL_error(L, "failed to get sws context");

    v->rgb_frame = av_frame_alloc();
    if(!v->rgb_frame) luaL_error(L, "failed to allocate RGB frame");

    v->rgb_frame->format = AV_PIX_FMT_RGB24;
    v->rgb_frame->width = v->width;
    v->rgb_frame->height = v->height;

    rc = av_frame_get_buffer(v->rgb_frame, 0);
    if(rc < 0) LUA_AV_ERROR(rc);

    v->is_running = true;
    v->thread = new std::thread(thread_func, v);
    return 0;
}

int lua_video_stop(lua_State *L) {
    Video *v = checkvideo(L);
    video_stop(v);
    return 0;
}

int lua_video_display(lua_State *L) {
    Video *v = checkvideo(L);
    if(v->is_running) {
        v->rgb_frame_mutex.lock();
        UpdateTexture(v->texture, v->rgb_frame->data[0]);
        v->rgb_frame_mutex.unlock();
    }
    rlImGuiImage((const Texture*)&v->texture);
    return 0;
}

static int lua_video_to_string(lua_State *L) {
    Video *v = checkvideo(L);
    lua_pushfstring(L, "video: \"%s\"", v->url.c_str());
    return 1;
}

static int lua_video_gc(lua_State *L) {
    Video *v = checkvideo(L);
    video_stop(v);
    v->~Video();
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