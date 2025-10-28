all: video

FFMPEG_DIR = ../lib/ffmpeg-8.0
LIBFFMPEG = $(FFMPEG_DIR)/libavformat/libavformat.a $(FFMPEG_DIR)/libavcodec/libavcodec.a $(FFMPEG_DIR)/libswscale/libswscale.a $(FFMPEG_DIR)/libavutil/libavutil.a 

video: video.cpp 
	g++ $< $(LIBFFMPEG) -o $@ -lraylib -lz

.PHONY: run

run: video
	./video