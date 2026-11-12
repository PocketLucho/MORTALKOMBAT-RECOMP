CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -g -Wno-deprecated-declarations
INC = -Isrc/emu -Isrc/mame -Isrc/board
CORE_SRC = src/emu/emu.cpp \
  src/mame/cpu/tms34010/tms34010.cpp src/mame/cpu/tms34010/34010dsm.cpp \
  src/mame/cpu/m6809/m6809.cpp src/mame/cpu/m6809/6x09dasm.cpp \
  src/mame/cpu/adsp2100/adsp2100.cpp src/mame/cpu/adsp2100/2100dasm.cpp \
  src/mame/sound/okiadpcm.cpp src/mame/ymfm/ymfm_opm.cpp src/mame/ymfm/ymfm_misc.cpp src/mame/ymfm/ymfm_adpcm.cpp src/mame/ymfm/ymfm_ssg.cpp \
  src/board/midway_video.cpp src/board/williams_adpcm.cpp src/board/tunit.cpp src/board/dcs.cpp src/board/wunit.cpp
BUILD ?= build/linux
CORE_OBJ = $(patsubst %.cpp,$(BUILD)/%.o,$(CORE_SRC))

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -MMD -MP -c $< -o $@

$(BUILD)/libcore.a: $(CORE_OBJ)
	ar rcs $@ $^

all: $(BUILD)/cattest

-include $(CORE_OBJ:.o=.d)


# ---------------- application (Windows: MSYS2 MinGW64 or llvm-mingw cross) ----------------
SDL2W ?= /mingw64
WINCC ?= x86_64-w64-mingw32-gcc
WINCXX ?= x86_64-w64-mingw32-g++
APP_SRC = src/app/main.cpp src/app/games.cpp src/app/romload.cpp src/app/netplay.cpp src/app/gl_present.cpp src/third_party/miniz/miniz.c
WIN_OUT ?= dist/MKRecomp
win:
	@mkdir -p $(WIN_OUT)
	$(WINCC) -O2 -c src/third_party/miniz/miniz.c -o build/win/miniz.o
	$(WINCXX) -std=c++20 -O2 -Isrc/emu -Isrc/mame -Isrc/board -Isrc/app -Isrc/third_party/miniz -I$(SDL2W)/include/SDL2 \
	  src/app/main.cpp src/app/games.cpp src/app/romload.cpp src/app/netplay.cpp src/app/gl_present.cpp build/win/miniz.o build/win/libcore.a \
	  -L$(SDL2W)/lib -lmingw32 -lSDL2main -lSDL2 -mwindows -static -lwinmm -lsetupapi -loleaut32 -lole32 -limm32 -lversion -lgdi32 -luser32 -lshell32 -luuid -ladvapi32 -lcfgmgr32 -lws2_32 -lwinpthread \
	  -o $(WIN_OUT)/MKRecomp.exe

# ---------------- application (Linux, for testing) ----------------
SDL2L ?= /usr
# static SDL2 by default; LINUX_SDL='-lSDL2' links the system library instead
LINUX_SDL ?= -Wl,-Bstatic -lSDL2 -Wl,-Bdynamic
APP_INC = -Isrc/emu -Isrc/mame -Isrc/board -Isrc/app -Isrc/third_party/miniz
$(BUILD)/miniz.o: src/third_party/miniz/miniz.c
	$(CC) -O2 -c $< -o $@
linux-app: $(BUILD)/libcore.a $(BUILD)/miniz.o
	$(CXX) $(CXXFLAGS) $(APP_INC) -I$(SDL2L)/include/SDL2 src/app/main.cpp src/app/games.cpp src/app/romload.cpp src/app/netplay.cpp src/app/gl_present.cpp \
	  $(BUILD)/miniz.o $(BUILD)/libcore.a -L$(SDL2L)/lib $(LINUX_SDL) -static-libstdc++ -static-libgcc -lm -ldl -lpthread -o $(BUILD)/MKRecomp
$(BUILD)/cattest: tools/headless/cattest.cpp src/app/games.cpp src/app/romload.cpp $(BUILD)/libcore.a $(BUILD)/miniz.o
	$(CXX) $(CXXFLAGS) $(APP_INC) $< src/app/games.cpp src/app/romload.cpp $(BUILD)/miniz.o $(BUILD)/libcore.a -o $@
.PHONY: win linux-app
