FROM debian:trixie AS build
WORKDIR /work
RUN apt update
RUN apt install -y build-essential gcc-aarch64-linux-gnu g++-aarch64-linux-gnu mingw-w64 git ninja-build python3 zip
RUN apt install -y nasm zlib1g-dev libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
RUN git clone https://github.com/max22-/RINKIN-ROV
WORKDIR /work/RINKIN-ROV/rinkin-ui
RUN git checkout dev
RUN git submodule init && git submodule update
RUN python3 generate_ninja.py > build.ninja
RUN ninja

FROM scratch AS export
COPY --from=build /work/RINKIN-ROV/rinkin-ui/bin /
