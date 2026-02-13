import ninja_syntax
import os
import sys
import glob

imgui_dir = "lib/imgui"
raylib_dir = "lib/raylib/src"
rlimgui_dir = "lib/rlImGui"
implot_dir = "lib/implot"
lua_dir = "lib/lua-5.4.8/src"
ffmpeg_dir = "lib/ffmpeg-8.0"
zlib_dir = "lib/zlib"
ffmpeg_configure_flags = " ".join([
    '--disable-encoders',
	'--disable-hwaccels',
	'--disable-muxers',
	'--disable-filters',
	'--disable-indevs',
	'--disable-outdevs',
	'--disable-programs',
	'--disable-doc',
	'--disable-libdrm',
	'--disable-v4l2-m2m',
	'--disable-vaapi',
	'--disable-vdpau',
	'--disable-vulkan',
	'--disable-alsa',
	'--disable-sndio',
	'--disable-xlib',
	'--disable-sdl2',
	'--disable-cuda',
	'--disable-nvdec',
	'--disable-nvenc',
	'--disable-opencl',
	'--disable-libopus',
	'--disable-lzma',
	'--disable-decoder=opus',
	'--disable-bzlib',
	'--extra-cflags="-Os -fPIC"',
	'--extra-ldflags="-s"',
])

libraylib_a = raylib_dir + "/libraylib.a"
liblua_a = lua_dir + "/liblua.a"
libz_a = "build/zlib/libz.a"

lib_ffmpeg_a = ["build/ffmpeg/" + a for a in [
    'libavformat/libavformat.a',
    'libavcodec/libavcodec.a',
    'libswscale/libswscale.a',
    'libavutil/libavutil.a'
]]

static_libs = [libraylib_a, liblua_a]

srcs = glob.glob("src/**/*.cpp", recursive=True)
srcs += [imgui_dir + "/" + f for f in ["imgui.cpp", "imgui_demo.cpp", "imgui_draw.cpp", "imgui_tables.cpp", "imgui_widgets.cpp"]]
srcs += [rlimgui_dir + "/rlImGui.cpp"]
srcs += [implot_dir + "/" + f for f in ["implot.cpp", "implot_items.cpp"]]
objs = [os.path.splitext(f)[0] + ".o" for f in srcs]

include_dirs = ["-I" + d for d in [imgui_dir, raylib_dir, raylib_dir + "/../examples/shaders", rlimgui_dir, lua_dir, ffmpeg_dir, implot_dir]]
cpp_flags = ["-std=c++11", "-pedantic", "-Wall"] + include_dirs
cpp_flags_debug = cpp_flags + ["-g"]
cpp_flags_release = cpp_flags + ["-DNDEBUG", "-Os"]

writer = ninja_syntax.Writer(sys.stdout)
writer.variable("cxx", os.getenv("CXX", "g++"))
writer.variable("cpp_flags_debug", " ".join(cpp_flags_debug))
writer.variable("cpp_flags_release", " ".join(cpp_flags_release))
writer.rule("cpp_debug", "$cxx $cpp_flags_debug -MD -MF $out.d $cflags -c -o $out $in", depfile="$out.d")
writer.rule("cpp_release", "$cxx $cpp_flags_release -MD -MF $out.d $cflags -c -o $out $in", depfile="$out.d")
writer.rule("link", "$cxx -o $out $in $ldflags")
writer.rule("make", "make -j -C $dir $target")

writer.rule("configure", "mkdir -p $build_dir && cd $build_dir && $cmd $flags", generator=True)

writer.build("build/ffmpeg/Makefile", "configure", inputs=None, variables={"build_dir": "build/ffmpeg", "cmd": "../../lib/ffmpeg-8.0/configure", "flags": ffmpeg_configure_flags})
writer.build(lib_ffmpeg_a, "make", variables={"dir": "build/ffmpeg"}, implicit="build/ffmpeg/Makefile")

for src, obj in zip(srcs, objs):
    writer.build("build/debug/" + obj, "cpp_debug", src)
    writer.build("build/release/" + obj, "cpp_release", src)
writer.build("bin/debug/rinkin", "link", ["build/debug/" + o for o in objs] + [libraylib_a, liblua_a] + lib_ffmpeg_a + [libz_a])
writer.build("bin/release/rinkin", "link", ["build/release/" + o for o in objs] + [libraylib_a, liblua_a] + lib_ffmpeg_a + [libz_a])

writer.build(libraylib_a, "make", variables={"dir": raylib_dir, "target": "PLATFORM=PLATFORM_DESKTOP"})
writer.build(liblua_a, "make", variables={"dir": lua_dir, "target": "linux"})

writer.build("build/zlib/Makefile", "configure", variables={"build_dir": "build/zlib", "cmd": "../../lib/zlib/configure", "flags": ""})
writer.build(libz_a, "make", variables={"dir": "build/zlib"}, implicit="build/zlib/Makefile")