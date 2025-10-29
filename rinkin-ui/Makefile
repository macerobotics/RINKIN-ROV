EXE = rinkin
IMGUI_DIR = lib/imgui
RAYLIB_DIR = lib/raylib/src
RLIMGUI_DIR = lib/rlImGui
MOSQUITTO_DIR = lib/mosquitto
LUA_DIR = lib/lua-5.4.8/src
FFMPEG_DIR = lib/ffmpeg-8.0

LIBRAYLIB = $(RAYLIB_DIR)/libraylib.a
LIBMOSQUITTO = $(MOSQUITTO_DIR)/lib/libmosquitto.a
LIBLUA = $(LUA_DIR)/liblua.a
LIBFFMPEG = $(FFMPEG_DIR)/libavformat/libavformat.a $(FFMPEG_DIR)/libavcodec/libavcodec.a $(FFMPEG_DIR)/libswscale/libswscale.a $(FFMPEG_DIR)/libavutil/libavutil.a 
STATIC_LIBS = $(LIBRAYLIB) $(LIBMOSQUITTO) $(LIBLUA) $(LIBFFMPEG)

#FFMPEG_LDFLAGS = -L$(FFMPEG_DIR)/libavformat -L$(FFMPEG_DIR)/libavcodec -L$(FFMPEG_DIR)/libswscale/ -L$(FFMPEG_DIR)/libavutil/ -lavformat -lavcodec -lavutil -lswscale
#FFMPEG_LDFLAGS = -lavformat -lavcodec -lavutil -lswscale

INCLUDE_DIRS = -I$(IMGUI_DIR) -Ilib/rlImGui -I$(RAYLIB_DIR) -I$(RAYLIB_DIR)/../examples/shaders -I$(MOSQUITTO_DIR)/include -I$(LUA_DIR) -I$(FFMPEG_DIR)
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
	make -C $(RAYLIB_DIR)

$(LIBMOSQUITTO):
	make -C $(MOSQUITTO_DIR)/lib WITH_STATIC_LIBRARIES=yes WITH_SHARED_LIBRARIES=no WITH_TLS=no

$(LIBLUA):
	make -C $(LUA_DIR) liblua.a

.PHONY: run clean fullclean lsp

run: bin/$(EXE)
	./bin/$(EXE)

clean:
	rm -rf bin build

fullclean:
	make -C $(RAYLIB_DIR) clean
	make -C $(MOSQUITTO_DIR) clean
	make -C $(LUA_DIR) clean
	make -C $(FFMPEG_DIR) clean
	rm -f .vscode/compile_commands.json

lsp: .vscode/compile_commands.json

.vscode/compile_commands.json:
	bear --output $@ -- make clean all

-include $(DEPS)
