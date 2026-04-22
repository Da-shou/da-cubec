BUILD    := build
EXE      := da-cubec
OSTYPE   := $(shell uname -s)

# Detect OS and set CMake generator
ifeq ($(OS), Windows_NT)
    CMAKE_FLAGS := -G "MinGW Makefiles"
else
    CMAKE_FLAGS :=
endif

.PHONY: all clean docs tidy format configure

all: format configure
	cmake --build $(BUILD)
	@if [ -f $(BUILD)/compile_commands.json ]; then mv $(BUILD)/compile_commands.json .; fi

configure:
	cmake $(CMAKE_FLAGS) -S . -B $(BUILD)

format:
	clang-format -i src/*.*
	clang-format -i include/*.*

tidy: format configure
	find src -name '*.c' | xargs clang-tidy --header-filter='src/.*'
	find include -name '*.h' | xargs clang-tidy --header-filter='src/.*'

docs: configure
	cmake --build $(BUILD) --target doc_pdf

clean: configure
	cmake --build $(BUILD) --target clean
