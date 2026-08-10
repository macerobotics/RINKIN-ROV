#!/bin/bash
set -euxo pipefail
cd lib/ffmpeg-8.0
./configure \
  --disable-everything \
  --enable-avformat \
  --enable-avcodec \
  --enable-avutil \
  --enable-swscale \
  --enable-demuxer=rtsp \
  --enable-protocol=rtsp \
  --enable-protocol=tcp \
  --enable-decoder=h264 \
  --enable-decoder=hevc \
  --enable-network \
  --enable-static \
  --disable-shared \
  --disable-programs \
  --disable-doc \
  --enable-small \
  --disable-hwaccels \
  --disable-libdrm \
  --disable-v4l2-m2m \
  --disable-vaapi \
  --disable-vdpau \
  --disable-vulkan \
  --disable-alsa \
  --disable-sndio \
  --disable-xlib \
  --disable-sdl2 \
  --disable-cuda \
  --disable-nvdec \
  --disable-nvenc \
  --disable-opencl \
  --enable-pixelutils \
  --enable-swscale-alpha \
  --enable-parser=h264 \
  --enable-bsf=h264_mp4toannexb \
  --extra-cflags="-Os -fPIC" \
  --extra-ldflags="-s"
make -j8