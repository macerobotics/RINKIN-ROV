#include <mpv/client.h>
#include <mpv/render.h>
#include <imgui.h>
#include <raylib.h>
#include <atomic>

class Video {
public:
    Video(const char *url, unsigned int width, unsigned int height);
    ~Video();
    bool start();
    void stop();
    ImTextureID get_texture();

private:
    void mpv_wakeup_cb();
    void mpv_render_cb();
    const char *url;
    const unsigned int width, height;
    mpv_handle *mpv = nullptr;
    mpv_render_context *mpv_rd = nullptr;
    Texture2D texture;
    std::atomic_bool has_events, render_cb_called;
    void *pixels;

    friend void mpv_wakeup_cb_wrapper(void *ctx);
    friend void mpv_render_cb_wrapper(void *ctx);
};