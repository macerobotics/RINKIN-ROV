EXE = rinkin
IMGUI_DIR = lib/imgui
RAYLIB_DIR = lib/raylib/src
RLIMGUI_DIR = lib/rlImGui
MOSQUITTO_DIR = lib/mosquitto

LIBRAYLIB = $(RAYLIB_DIR)/libraylib.a
LIBMOSQUITTO = $(MOSQUITTO_DIR)/lib/libmosquitto.a
STATIC_LIBS = $(LIBRAYLIB) $(LIBMOSQUITTO)

INCLUDE_DIRS = -I$(IMGUI_DIR) -Ilib/rlImGui -I$(RAYLIB_DIR) -I$(RAYLIB_DIR)/../examples/shaders -Ilib/ini/src -I$(MOSQUITTO_DIR)/include
CXXFLAGS = -std=c++11 -pedantic -Wall -MMD -MP $(INCLUDE_DIRS) -g
LDFLAGS = -L$(RAYLIB_DIR) -L$(MOSQUITTO_DIR) -lGL $(shell pkg-config --libs mpv) -lraylib -lmosquitto
SRCS = $(shell find src -name *.cpp)
SRCS += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SRCS += $(RLIMGUI_DIR)/rlImGui.cpp
SRCS += lib/ini/src/ini.c
OBJS = $(SRCS:%=build/%.o)
DEPS = $(OBJS:.o=.d)

all: bin/$(EXE)

bin/$(EXE): $(OBJS) $(STATIC_LIBS)
	mkdir -p bin
	$(CXX) $^ -o $@ $(LDFLAGS)

build/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/lib/ini/src/ini.c.o: lib/ini/src/ini.c
	mkdir -p $(dir $@)
	$(CC) -ansi -pedantic -Wall -MMD -MP -c $< -o $@

$(LIBRAYLIB):
	make -C $(RAYLIB_DIR)

$(LIBMOSQUITTO):
	make -C $(MOSQUITTO_DIR)/lib WITH_STATIC_LIBRARIES=yes WITH_SHARED_LIBRARIES=no WITH_TLS=no

.PHONY: run clean lsp

run: bin/$(EXE)
	./bin/$(EXE)

clean:
	rm -rf bin build
	make -C $(RAYLIB_DIR) clean
	make -C $(MOSQUITTO_DIR) clean
	rm -f .vscode/compile_commands.json

lsp: .vscode/compile_commands.json

.vscode/compile_commands.json:
	bear --output $@ -- make clean all

-include $(DEPS)
