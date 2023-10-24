CC  := clang
CXX := clang++
$(shell mkdir -p build)

LIBFILES := build/foobar2000_SDK.lib \
            build/foobar2000_sdk_helpers.lib \
			build/foobar2000_component_client.lib \
			build/libPPUI.lib \
			build/pfc.lib \
			build/ubjson.lib

all: $(LIBFILES) build/foo_mpris.lib
	lld-link -debug /out:build/foo_mpris.dll \
		/SUBSYSTEM:WINDOWS \
		/IMPLIB:build/foo_mpris.lib \
		//DLL $(OBJFILES) \
		$(LIBFILES) \
		ws2_32.lib foobar2000/shared/shared-x64.lib kernel32.lib user32.lib \
		gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib \
		oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib \
		gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib \
		oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib

# winepath -u "$(wine cmd /c set | awk -F '=' '/^APPDATA/ { print $2 }' | tr -d '\r')\\foobar-v2\\user-components-x64\\foo_mpris"

install: all
	cp build/foo_mpris.dll //wsl.localhost/Debian/home/ally/.wine/drive_c/users/ally/AppData/Roaming/foobar2000-v2/user-components-x64/foo_mpris/

SRCFILES := $(shell find src/ -name '*.cpp')
OBJFILES := $(patsubst src/%.cpp,build/%.obj,$(SRCFILES))
CFLAGS   := -std=c++17 -O2 -DNDEBUG -DFOOMPRIS_EXPORTS -D_WINDOWS -D_USRDLL -D_WINDLL -D_UNICODE -DUNICODE -D_MD -D_DLL -D_CRT_SECURE_NO_WARNINGS -Wno-unused-value
INCLUDE  := -Ifoobar2000 -I. -Iwtl -I../..

build/%.obj: src/%.cpp src/socket.hpp
	$(CXX) -c -g -gcodeview -o $@ $< $(INCLUDE) $(CFLAGS)

build/foo_mpris.lib: $(OBJFILES)
	llvm-lib /out:$@ $^

# foobar2000_SDK --------------------------------

foobar2000_SDK-SOURCEFILES := $(wildcard foobar2000/SDK/*.cpp)
foobar2000_SDK-OBJFILES    := $(patsubst foobar2000/SDK/%.cpp,build/foobar2000_SDK/%.obj,$(foobar2000_SDK-SOURCEFILES))
foobar2000_SDK-INCLUDE     := -Ifoobar2000 -I.
foobar2000_SDK-CFLAGS      := -std=c++17 -O2 -DNDEBUG -D_UNICODE -DUNICODE -D_MD -D_DLL -Wno-deprecated

$(shell mkdir -p build/foobar2000_SDK)

build/foobar2000_SDK/%.obj: foobar2000/SDK/%.cpp
	$(CXX) -c -o $@ $< $(foobar2000_SDK-INCLUDE) $(foobar2000_SDK-CFLAGS)

build/foobar2000_SDK.lib: $(foobar2000_SDK-OBJFILES)
	llvm-lib /out:$@ $^

# -----------------------------------------------

# foobar2000_sdk_helpers ------------------------

foobar2000_sdk_helpers-SOURCEFILES := $(wildcard foobar2000/helpers/*.cpp)
foobar2000_sdk_helpers-OBJFILES    := $(patsubst foobar2000/helpers/%.cpp,build/foobar2000_sdk_helpers/%.obj,$(foobar2000_sdk_helpers-SOURCEFILES))
foobar2000_sdk_helpers-INCLUDE     := -Ifoobar2000 -I. -Iwtl
foobar2000_sdk_helpers-CFLAGS      := -std=c++17 -O2 -DNDEBUG -D_UNICODE -DUNICODE -D_MD -D_DLL -Wno-deprecated -Wno-unused-value -Wno-nonportable-include-path

$(shell mkdir -p build/foobar2000_sdk_helpers)

build/foobar2000_sdk_helpers/%.obj: foobar2000/helpers/%.cpp
	$(CXX) -c -o $@ $< $(foobar2000_sdk_helpers-INCLUDE) $(foobar2000_sdk_helpers-CFLAGS)

build/foobar2000_sdk_helpers.lib: $(foobar2000_sdk_helpers-OBJFILES)
	llvm-lib /out:$@ $^

# foobar2000_component_client -------------------

foobar2000_component_client-SOURCEFILES := $(wildcard foobar2000/foobar2000_component_client/*.cpp)
foobar2000_component_client-OBJFILES    := $(patsubst foobar2000/foobar2000_component_client/%.cpp,build/foobar2000_component_client/%.obj,$(foobar2000_component_client-SOURCEFILES))
foobar2000_component_client-INCLUDE     := -Ifoobar2000 -I.
foobar2000_component_client-CFLAGS      := -std=c++17 -O2 -DNDEBUG -D_UNICODE -DUNICODE -D_MD -D_DLL -Wno-deprecated -Wno-unused-value -Wno-nonportable-include-path

$(shell mkdir -p build/foobar2000_component_client)

build/foobar2000_component_client/%.obj: foobar2000/foobar2000_component_client/%.cpp
	$(CXX) -c -o $@ $< $(foobar2000_component_client-INCLUDE) $(foobar2000_component_client-CFLAGS)

build/foobar2000_component_client.lib: $(foobar2000_component_client-OBJFILES)
	llvm-lib /out:$@ $^

# -----------------------------------------------

# libPPUI ---------------------------------------

libPPUI-SOURCEFILES := $(wildcard libPPUI/*.cpp)
libPPUI-OBJFILES    := $(patsubst libPPUI/%.cpp,build/libPPUI/%.obj,$(libPPUI-SOURCEFILES))
libPPUI-INCLUDE     := -I. -Iwtl
libPPUI-CFLAGS      := -std=c++17 -O2 -DNDEBUG -D_UNICODE -DUNICODE -D_MD -D_DLL -Wno-deprecated -Wno-unused-value -Wno-inconsistent-missing-override

$(shell mkdir -p build/libPPUI)

build/libPPUI/%.obj: libPPUI/%.cpp
	$(CXX) -c -o $@ $< $(libPPUI-INCLUDE) $(libPPUI-CFLAGS)

build/libPPUI.lib: $(libPPUI-OBJFILES)
	llvm-lib /out:$@ $^

# -----------------------------------------------

# pfc -------------------------------------------

pfc-SOURCEFILES := $(wildcard pfc/*.cpp)
pfc-OBJFILES    := $(patsubst pfc/%.cpp,build/pfc/%.obj,$(pfc-SOURCEFILES))
pfc-INCLUDE     := -Iwtl
pfc-CFLAGS      := -std=c++17 -O2 -DNDEBUG -D_UNICODE -DUNICODE -D_MD -D_DLL -Wno-deprecated -Wno-unused-value -Wno-inconsistent-missing-override

$(shell mkdir -p build/pfc)

build/pfc/%.obj: pfc/%.cpp
	$(CXX) -c -o $@ $< $(pfc-INCLUDE) $(pfc-CFLAGS)

build/pfc.lib: $(pfc-OBJFILES)
	llvm-lib /out:$@ $^

# -----------------------------------------------

# ubjson ----------------------------------------

ubjson-SOURCEFILES := $(wildcard ../../ubjson/*.c)
ubjson-OBJFILES    := $(patsubst ../../ubjson/%.c,build/ubjson/%.obj,$(ubjson-SOURCEFILES))
ubjson-INCLUDE     := 
ubjson-CFLAGS      := -std=c99 -O2 -DNDEBUG -D_UNICODE -DUNICODE -D_MD -D_DLL -Wno-deprecated -Wno-unused-value -Wno-inconsistent-missing-override

$(shell mkdir -p build/ubjson)

build/ubjson/%.obj: src/ubjson/%.c
	$(CC) -c -o $@ $< $(ubjson-INCLUDE) $(ubjson-CFLAGS)

build/ubjson.lib: $(ubjson-OBJFILES)
	llvm-lib /out:$@ $^

# -----------------------------------------------

.PHONY: clean
clean:
	rm -r build/
