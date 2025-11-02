EXE = rinkin
IMGUI_DIR = lib/imgui
RAYLIB_DIR = lib/raylib/src
RLIMGUI_DIR = lib/rlImGui
LUA_DIR = lib/lua-5.4.8/src
FFMPEG_DIR = lib/ffmpeg-8.0
FFMPEG_CONFIGURE = $(FFMPEG_DIR)/config.h
FFMPEG_CONFIGURE_FLAGS = \
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
	--extra-ldflags="-s" \
	$(FFMPEG_CROSS_FLAGS)

LIBRAYLIB = $(RAYLIB_DIR)/libraylib.a
LIBLUA = $(LUA_DIR)/liblua.a
LIBFFMPEG = $(FFMPEG_DIR)/libavformat/libavformat.a $(FFMPEG_DIR)/libavcodec/libavcodec.a $(FFMPEG_DIR)/libswscale/libswscale.a $(FFMPEG_DIR)/libavutil/libavutil.a 
STATIC_LIBS = $(LIBRAYLIB) $(LIBLUA) $(LIBFFMPEG)

#FFMPEG_LDFLAGS = -L$(FFMPEG_DIR)/libavformat -L$(FFMPEG_DIR)/libavcodec -L$(FFMPEG_DIR)/libswscale/ -L$(FFMPEG_DIR)/libavutil/ -lavformat -lavcodec -lavutil -lswscale
#FFMPEG_LDFLAGS = -lavformat -lavcodec -lamake fullvutil -lswscale

INCLUDE_DIRS = -I$(IMGUI_DIR) -Ilib/rlImGui -I$(RAYLIB_DIR) -I$(RAYLIB_DIR)/../examples/shaders -I$(LUA_DIR) -I$(FFMPEG_DIR)
CXXFLAGS = -std=c++11 -pedantic -Wall -MMD -MP $(INCLUDE_DIRS) -g
LDFLAGS = $(FFMPEG_LDFLAGS) -lpthread -lz
SRCS = $(shell find src -name *.cpp)
SRCS += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SRCS += $(RLIMGUI_DIR)/rlImGui.cpp
OBJS = $(SRCS:%=build/%.o)
DEPS = $(OBJS:.o=.d)

all: bin/$(EXE)

bin/$(EXE): $(OBJS) $(STATIC_LIBS)
	mkdir -p bin
	$(CXX) $^ -o $@ $(LDFLAGS)

build/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBRAYLIB):
	$(MAKE) -C $(RAYLIB_DIR) $(RAYLIB_PLATFORM_OS)

$(LIBLUA):
	$(MAKE) -C $(LUA_DIR) liblua.a

$(FFMPEG_CONFIGURE):
	cd $(FFMPEG_DIR) && ./configure $(FFMPEG_CONFIGURE_FLAGS)

$(LIBFFMPEG) &: $(FFMPEG_CONFIGURE)
	$(MAKE) -C $(FFMPEG_DIR)

$(OBJS): $(LIBFFMPEG)

WINDOWS_FFMPEG_CONFIGURE_FLAGS = \
	--target-os=mingw32 \
	--arch=x86_64 \
	--enable-cross-compile \
	--cross-prefix=x86_64-w64-mingw32-

windows: 
	$(MAKE) CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LDFLAGS="$(LDFLAGS) -lws2_32 -lgdi32 -lopengl32 -lwinmm -lncrypt -lsecur32 -lcrypt32" FFMPEG_CROSS_FLAGS="$(WINDOWS_FFMPEG_CONFIGURE_FLAGS)" RAYLIB_PLATFORM_OS="PLATFORM_OS=WINDOWS"

.PHONY: run clean fullclean lsp

run: bin/$(EXE)
	./bin/$(EXE)

clean:
	rm -rf bin build

fullclean: clean
	make -C $(RAYLIB_DIR) clean
	make -C $(LUA_DIR) clean
	make -C $(FFMPEG_DIR) distclean
	rm -f .vscode/compile_commands.json

lsp: .vscode/compile_commands.json

.vscode/compile_commands.json:
	bear --output $@ -- make clean all

-include $(DEPS)
