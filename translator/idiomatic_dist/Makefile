CC ?= cc
AR ?= ar

CPPFLAGS ?= -Iinclude
CFLAGS ?= -O2
C89FLAGS = -std=c89 -pedantic
LDLIBS = -lm

SOURCES = src/imgui.c src/imgui_draw.c src/imgui_tables.c \
	src/imgui_widgets.c src/imgui_c89_api.c
OBJECTS = $(SOURCES:src/%.c=build/%.o)
LIBRARY = lib/libimgui_c89.a

.PHONY: all check clean

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	mkdir -p lib
	$(AR) rcs $@ $(OBJECTS)

build/%.o: src/%.c include/imgui_c89.h include/imgui_c89_internal.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(C89FLAGS) -c $< -o $@

build/imgui_c89_api.o: include/imgui_c89_api.h

build/smoke: examples/smoke.c $(LIBRARY)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(C89FLAGS) $< $(LIBRARY) \
		$(LDLIBS) -o $@

check: build/smoke
	./build/smoke

clean:
	rm -rf build lib
