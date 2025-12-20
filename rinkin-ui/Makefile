EXE = rinkin
IMGUI_DIR = lib/imgui
RAYLIB_DIR = lib/raylib/src
RLIMGUI_DIR = lib/rlImGui
IMPLOT_DIR = lib/implot
LUA_DIR = lib/lua-5.4.8/src
FFMPEG_DIR = lib/ffmpeg-8.0
ZLIB_DIR = lib/zlib
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
ZLIB = $(ZLIB_DIR)/libz.a
STATIC_LIBS = $(LIBRAYLIB) $(LIBLUA) $(LIBFFMPEG) $(ZLIB)

INCLUDE_DIRS = -I$(IMGUI_DIR) -Ilib/rlImGui -I$(RAYLIB_DIR) -I$(RAYLIB_DIR)/../examples/shaders -I$(LUA_DIR) -I$(FFMPEG_DIR) -I$(IMPLOT_DIR)
CXXFLAGS = -std=c++11 -pedantic -Wall -MMD -MP $(INCLUDE_DIRS) -g -Os
LDFLAGS = $(FFMPEG_LDFLAGS) -lpthread
ifeq ($(TARGET),windows)
	LDFLAGS += -lws2_32 -lgdi32 -lopengl32 -lwinmm -lncrypt -lsecur32 -lcrypt32
endif
ifeq ($(TARGET),rpi)
	CXXFLAGS += -DGLSL_VERSION=120
endif
AR ?= ar
SRCS = $(shell find src -name *.cpp)
SRCS += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SRCS += $(RLIMGUI_DIR)/rlImGui.cpp
SRCS += $(IMPLOT_DIR)/implot.cpp $(IMPLOT_DIR)/implot_items.cpp
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
	$(MAKE) -C $(RAYLIB_DIR) PLATFORM=PLATFORM_DESKTOP $(RAYLIB_PLATFORM_OS)

$(LIBLUA):
	$(MAKE) -C $(LUA_DIR) AR="$(AR) rcu" liblua.a

$(FFMPEG_CONFIGURE):
	cd $(FFMPEG_DIR) && ./configure $(FFMPEG_CONFIGURE_FLAGS)

$(LIBFFMPEG) &: $(FFMPEG_CONFIGURE)
	$(MAKE) -C $(FFMPEG_DIR)

$(ZLIB):
	cd $(ZLIB_DIR) && ./configure
	$(MAKE) -C $(ZLIB_DIR)

$(OBJS): $(LIBFFMPEG)

WINDOWS_FFMPEG_CONFIGURE_FLAGS = \
	--target-os=mingw32 \
	--arch=x86_64 \
	--enable-cross-compile \
	--cross-prefix=x86_64-w64-mingw32-

windows: 
	$(MAKE) TARGET=windows CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ FFMPEG_CROSS_FLAGS="$(WINDOWS_FFMPEG_CONFIGURE_FLAGS)" RAYLIB_PLATFORM_OS="PLATFORM_OS=WINDOWS"

rpi:
	$(MAKE) TARGET=rpi CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ AR=aarch64-linux-gnu-ar RANLIB=aarch64-linux-gnu-ranlib GRAPHICS=GRAPHICS_API_OPENGL_21

.PHONY: run clean fullclean lsp

run: bin/$(EXE)
	./bin/$(EXE)

clean:
	rm -rf bin build

fullclean: clean
	make -C $(RAYLIB_DIR) clean
	make -C $(LUA_DIR) clean
	make -C $(FFMPEG_DIR) distclean || true
	make -C $(ZLIB_DIR) distclean
	rm -f .vscode/compile_commands.json

lsp: .vscode/compile_commands.json

.vscode/compile_commands.json:
	bear --output $@ -- make clean all

-include $(DEPS)
