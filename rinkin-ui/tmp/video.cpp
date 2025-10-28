extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <raylib.h>
#include <rlgl.h>

static const char url[] = "rtsp://192.168.1.18:8554/cam";

int main(int argc, char *argv[]) {
    const int screen_width = 1920, screen_height = 1080;
    InitWindow(screen_width, screen_height, "ffmpeg demo");

    AVDictionary *options = nullptr;
    av_dict_set(&options, "flags", "nobuffer", 0);
    av_dict_set(&options, "flags", "low_delay", 0);
    av_dict_set(&options, "max_delay", "0", 0);
    Texture texture = {0};
    AVFormatContext *format_ctx = avformat_alloc_context();
    struct SwsContext *img_convert_ctx = sws_alloc_context();
    avformat_open_input(&format_ctx, url, nullptr, &options);
    TraceLog(LOG_INFO, "CODEC: format %s", format_ctx->iformat->long_name);
    avformat_find_stream_info(format_ctx, nullptr);
    AVStream *video_stream = nullptr;
    AVCodecParameters *video_params = nullptr;
    AVFrame *rgb_frame = nullptr;
    for(unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        AVStream *tmp_stream = format_ctx->streams[i];
        AVCodecParameters *tmp_params = tmp_stream->codecpar;
        if(tmp_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = tmp_stream;
            video_params = tmp_params;
            TraceLog(LOG_INFO, "CODEC: Resolution: %d x %d, type: %d", video_params->width, video_params->height, video_params->codec_id);
        }
    }

    if(!video_stream) {
        TraceLog(LOG_FATAL, "failed to find video stream");
        return 1;
    }

    const AVCodec *video_codec = avcodec_find_decoder(video_params->codec_id);
    TraceLog(LOG_INFO, "CODEC: %s ID %d, Bit rate %ld", video_codec->name, video_codec->id, video_params->bit_rate);
    TraceLog(LOG_INFO, "FPS: %d/%d, TBR: %d/%d, TimeBase: %d/%d", video_stream->avg_frame_rate.num,
             video_stream->avg_frame_rate.den, video_stream->r_frame_rate.num,
             video_stream->r_frame_rate.den, video_stream->time_base.num, video_stream->time_base.den);

    AVCodecContext *video_codec_ctx = avcodec_alloc_context3(video_codec);
    avcodec_parameters_to_context(video_codec_ctx, video_params);
    avcodec_open2(video_codec_ctx, video_codec, NULL);

    AVFrame *frame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();

    struct SwsContext *sws_ctx = sws_getContext(video_codec_ctx->width, video_codec_ctx->height, video_codec_ctx->pix_fmt,
                             video_codec_ctx->width, video_codec_ctx->height, AV_PIX_FMT_RGB24,
                             SWS_FAST_BILINEAR, 0, 0, 0);

    texture.height  = video_codec_ctx->height;
    texture.width   = video_codec_ctx->width;
    texture.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    texture.mipmaps = 1;
    texture.id = rlLoadTexture(NULL, texture.width, texture.height, texture.format, texture.mipmaps);
    //SetTargetFPS(video_stream->avg_frame_rate.num / video_stream->avg_frame_rate.den);

    rgb_frame = av_frame_alloc();
    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width = video_codec_ctx->width;
    rgb_frame->height = video_codec_ctx->height;
    av_frame_get_buffer(rgb_frame, 0);

    SetTargetFPS(60);
    int v_frame = 0;
    while(!WindowShouldClose()) {
        while(av_read_frame(format_ctx, packet) >= 0) {
            if(packet->stream_index == video_stream->index) {
                int ret = avcodec_send_packet(video_codec_ctx, packet);
                av_packet_unref(packet);
                if(ret < 0) {
                    TraceLog(LOG_ERROR, "error sending packet");
                    continue;
                }
                while(ret >= 0) {
                    ret = avcodec_receive_frame(video_codec_ctx, frame);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
                    sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0,
                              frame->height, rgb_frame->data, rgb_frame->linesize);
                    UpdateTexture(texture, rgb_frame->data[0]);
                }
                break;
            }
        }
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(texture, (Rectangle){0, 0, texture.width, texture.height}, (Rectangle){0, 0, screen_width, screen_height}, (Vector2){0, 0}, 0, WHITE);
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    av_packet_unref(packet);
    av_packet_free(&packet);
    avcodec_free_context(&video_codec_ctx);
    sws_freeContext(sws_ctx);
    avformat_close_input(&format_ctx);
    return 0;
}