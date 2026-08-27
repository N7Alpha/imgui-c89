CC ?= cc
CXX ?= c++
AR ?= ar
LDLIBS = -lm

CPPFLAGS = -Iinclude
CFLAGS = -std=c89 -pedantic -Wall -Wextra -Werror -O0 -g

BUILD_DIR = build
LIB = $(BUILD_DIR)/libimgui_c89.a
OBJECTS = $(BUILD_DIR)/imgui_c89.o $(BUILD_DIR)/imgui_c89_font.o $(BUILD_DIR)/imgui_c89_software.o
SMOKE = $(BUILD_DIR)/smoke
SMOKE_CPP_OBJECT = $(BUILD_DIR)/smoke_cpp.o
SMOKE_CPP = $(BUILD_DIR)/smoke_cpp
TRACE_RUNNER = $(BUILD_DIR)/imgui_c89_trace_runner
TRACE_RUNNER_CPP = $(BUILD_DIR)/imgui_cpp_trace_runner

# Optional pinned Dear ImGui differential runner. Set IMGUI_REF_DIR to a
# checkout of the snapshot named in UPSTREAM.md before invoking `make
# reference`; the normal C89 build never depends on C++ reference sources.
IMGUI_REF_DIR ?=
REFERENCE_RUNNER = $(BUILD_DIR)/imgui_dearimgui_reference_runner

.PHONY: all check translator-check translator-matrix translator-baseline translator-latest-release translator-idiomatic-c89 translator-idiomatic-c89-size translator-idiomatic-c89-test-engine translator-idiomatic-c89-differential dist-idiomatic-c89 translator-latest-release-patched translator-latest-release-size translator-latest-release-test-engine translator-canonical translator-canonical-size translator-canonical-test-engine translator-canonical-differential translator-size-opportunities translator-embedded translator-embedded-compact translator-embedded-compact-full translator-embedded-compact-modular translator-embedded-compact-modular-debug translator-vendor translator-embedded-size translator-compact-modular-size translator-full-parity-size translator-test-engine translator-docking translator-docking-test-engine translator-differential translator-docking-differential translator-portability translator-viewport-sdlgpu translator-object-sizes legacy-check reference clean

all: $(LIB) $(SMOKE) $(SMOKE_CPP) $(TRACE_RUNNER) $(TRACE_RUNNER_CPP)

check: translator-differential translator-test-engine

translator-check:
	PYTHONDONTWRITEBYTECODE=1 python3 translator/test_fixture.py

translator-matrix: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/matrix_smoke.py

translator-baseline: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py

translator-latest-release: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile latest_release
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile latest_release

translator-idiomatic-c89: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile idiomatic_c89
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile idiomatic_c89

translator-idiomatic-c89-size: translator-latest-release translator-idiomatic-c89
	PYTHONDONTWRITEBYTECODE=1 python3 translator/full_library_size.py

translator-idiomatic-c89-test-engine: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile idiomatic_c89_test_engine
	PYTHONDONTWRITEBYTECODE=1 python3 translator/test_engine.py --profile idiomatic_c89_test_engine

translator-idiomatic-c89-differential: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile idiomatic_c89_differential
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile idiomatic_c89_differential
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py --profile idiomatic_c89_differential

dist-idiomatic-c89: translator-idiomatic-c89
	PYTHONDONTWRITEBYTECODE=1 python3 translator/vendor_bundle.py --profile idiomatic_c89 --template idiomatic_dist --output dist/imgui-c89
	$(MAKE) -C dist/imgui-c89 check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/vendor_bundle.py --profile idiomatic_c89 --template idiomatic_dist --output dist/imgui-c89

# Historical aliases retained for existing scripts and local habits.
translator-latest-release-patched: translator-idiomatic-c89

translator-latest-release-size: translator-idiomatic-c89-size

translator-latest-release-test-engine: translator-idiomatic-c89-test-engine

translator-canonical: translator-idiomatic-c89

translator-canonical-size: translator-idiomatic-c89-size

translator-canonical-test-engine: translator-idiomatic-c89-test-engine

translator-canonical-differential: translator-idiomatic-c89-differential

translator-size-opportunities: translator-idiomatic-c89-size
	PYTHONDONTWRITEBYTECODE=1 python3 translator/size_opportunities.py

translator-embedded: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile embedded
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile embedded
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py --profile embedded

translator-embedded-compact: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile embedded_compact
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile embedded_compact
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py --profile embedded_compact

translator-embedded-compact-full: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile embedded_compact_full
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile embedded_compact_full
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py --profile embedded_compact_full

translator-embedded-compact-modular: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile embedded_compact_modular
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile embedded_compact_modular
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py --profile embedded_compact_modular

translator-embedded-compact-modular-debug: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile embedded_compact_modular_debug
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile embedded_compact_modular_debug
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py --profile embedded_compact_modular_debug

translator-vendor: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile embedded_compact_vendor
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile embedded_compact_vendor
	PYTHONDONTWRITEBYTECODE=1 python3 translator/vendor_bundle.py
	$(MAKE) -C build/vendor/imgui-c89 check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/vendor_bundle.py

translator-compact-modular-size: translator-embedded-compact-modular translator-embedded-compact-modular-debug
	PYTHONDONTWRITEBYTECODE=1 python3 translator/compact_modular_size.py

translator-full-parity-size: translator-embedded-compact-full
	PYTHONDONTWRITEBYTECODE=1 python3 translator/full_parity_size.py

translator-embedded-size: translator-baseline translator-embedded translator-embedded-compact
	PYTHONDONTWRITEBYTECODE=1 python3 translator/embedded_size.py

translator-test-engine: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile test_engine
	PYTHONDONTWRITEBYTECODE=1 python3 translator/test_engine.py

translator-docking: translator-check
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile docking
	PYTHONDONTWRITEBYTECODE=1 python3 translator/check_baseline.py --profile docking

translator-docking-test-engine: translator-docking
	PYTHONDONTWRITEBYTECODE=1 python3 translator/build_baseline.py --profile test_engine_docking
	PYTHONDONTWRITEBYTECODE=1 python3 translator/test_engine.py --profile test_engine_docking

translator-differential: translator-baseline
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py

translator-docking-differential: translator-docking
	PYTHONDONTWRITEBYTECODE=1 python3 translator/differential_test.py --profile docking

translator-portability: translator-baseline translator-docking
	PYTHONDONTWRITEBYTECODE=1 python3 translator/portability_test.py

translator-viewport-sdlgpu: translator-docking
	PYTHONDONTWRITEBYTECODE=1 python3 translator/viewport_sdlgpu_test.py

translator-object-sizes: translator-baseline translator-docking
	PYTHONDONTWRITEBYTECODE=1 python3 translator/object_sizes.py

legacy-check: all
	$(SMOKE)
	$(SMOKE_CPP)
	$(TRACE_RUNNER) tests/trace_runner_sample.txt >/dev/null
	$(TRACE_RUNNER_CPP) tests/trace_runner_sample.txt >/dev/null

reference: $(LIB)
	@test -n "$(IMGUI_REF_DIR)" || (echo "set IMGUI_REF_DIR to the pinned Dear ImGui checkout"; exit 2)
	$(CXX) -Iinclude -I$(IMGUI_REF_DIR) -I$(IMGUI_REF_DIR)/misc/fonts -std=c++11 -Wall -Wextra -Werror \
		 tools/imgui_dearimgui_reference_runner.cpp \
		 $(IMGUI_REF_DIR)/imgui.cpp $(IMGUI_REF_DIR)/imgui_draw.cpp \
		 $(IMGUI_REF_DIR)/imgui_tables.cpp $(IMGUI_REF_DIR)/imgui_widgets.cpp \
		 $(LIB) $(LDLIBS) -o $(REFERENCE_RUNNER)
	$(REFERENCE_RUNNER) tests/trace_runner_sample.txt

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/imgui_c89.o: src/imgui_c89.c src/imgui_c89_internal.h include/imgui_c89.h include/imgui_c89_platform.h include/imgui_c89_render.h include/imgui_c89_trace.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/imgui_c89.c -o $@

$(BUILD_DIR)/imgui_c89_font.o: src/imgui_c89_font.c src/imgui_c89_internal.h include/imgui_c89.h include/imgui_c89_font.h third_party/stb_truetype.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -Wno-comment -Ithird_party -c src/imgui_c89_font.c -o $@

$(BUILD_DIR)/imgui_c89_software.o: src/imgui_c89_software.c include/imgui_c89.h include/imgui_c89_render.h include/imgui_c89_software.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/imgui_c89_software.c -o $@

$(LIB): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

$(SMOKE): tests/smoke.c $(LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/smoke.c $(LIB) $(LDLIBS) -o $@

$(SMOKE_CPP_OBJECT): tests/smoke.c include/imgui_c89.h include/imgui_c89_platform.h include/imgui_c89_render.h | $(BUILD_DIR)
	$(CXX) -x c++ $(CPPFLAGS) -std=c++98 -Wall -Wextra -Werror -c tests/smoke.c -o $@

$(SMOKE_CPP): $(SMOKE_CPP_OBJECT) $(LIB)
	$(CXX) $(SMOKE_CPP_OBJECT) $(LIB) $(LDLIBS) -o $@

$(TRACE_RUNNER): tools/imgui_c89_trace_runner.c $(LIB) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tools/imgui_c89_trace_runner.c $(LIB) $(LDLIBS) -o $@

$(TRACE_RUNNER_CPP): tools/imgui_cpp_trace_runner.cpp tools/imgui_c89_trace_runner.c $(LIB) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -std=c++98 -Wall -Wextra -Werror tools/imgui_cpp_trace_runner.cpp $(LIB) $(LDLIBS) -o $@

clean:
	rm -f $(OBJECTS) $(LIB) $(SMOKE) $(SMOKE_CPP_OBJECT) $(SMOKE_CPP) $(TRACE_RUNNER) $(TRACE_RUNNER_CPP)
	rmdir $(BUILD_DIR) 2>/dev/null || true
