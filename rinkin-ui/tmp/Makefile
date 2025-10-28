all: video

video: video.cpp
	g++ $< -o $@ -lraylib -lavformat -lavcodec -lavutil -lswscale -lm -lswresample

.PHONY: run

run: video
	./video