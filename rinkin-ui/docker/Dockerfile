FROM debian:trixie AS build
WORKDIR /work
RUN apt update
RUN apt install -y build-essential mingw-w64 git
RUN apt install -y nasm zlib1g-dev libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
RUN git clone https://github.com/max22-/RINKIN-ROV
WORKDIR /work/RINKIN-ROV/rinkin-ui
RUN git checkout dev
RUN git submodule init && git submodule update
RUN make -j8
RUN mkdir output
RUN mv bin/rinkin output/rinkin-linux-x86_64
RUN make fullclean
RUN cd lib/ffmpeg-8.0 && ./configure --disable-encoders --disable-hwaccels --disable-muxers --disable-filters --disable-indevs --disable-outdevs --disable-programs --disable-doc --disable-libdrm --disable-v4l2-m2m --disable-vaapi --disable-vdpau --disable-vulkan --disable-alsa --disable-sndio --disable-xlib --disable-sdl2 --disable-cuda --disable-nvdec --disable-nvenc --disable-opencl --disable-libopus --disable-lzma --disable-decoder=opus --disable-bzlib --extra-cflags="-Os -fPIC" --extra-ldflags="-s" --target-os=mingw32 --arch=x86_64 --enable-cross-compile --cross-prefix=x86_64-w64-mingw32-
RUN make -j8 windows
RUN mv bin/rinkin.exe output/