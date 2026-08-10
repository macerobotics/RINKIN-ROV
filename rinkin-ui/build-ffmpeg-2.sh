#!/bin/bash
set -euxo pipefail
cd lib/ffmpeg-8.0
./configure \
  --disable-encoders \
  --disable-hwaccels \
  --disable-muxers \
  --disable-filters \
  --disable-indevs \
  --disable-outdevs \
  --disable-programs \
  --disable-doc \
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
  --disable-libopus \
  --disable-lzma \
  --disable-decoder=opus \
  --disable-bzlib \
  --extra-cflags="-Os -fPIC" \
  --extra-ldflags="-s"
make -j8