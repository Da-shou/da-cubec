export BUILD="build"
export EXECUTABLE="da-cubec"

clang-format -i src/*.*
clang-format -i include/*.*

if [[ $2 == tidy ]] || [[ $1 == tidy ]]; then
	find src -name '*.c' | xargs clang-tidy --header-filter='src/.*'
	find include -name '*.h' | xargs clang-tidy --header-filter='src/.*'
fi

# Compile project using CMake
if [[ "$OSTYPE" == "msys" ]]; then
	cmake -G "MinGW Makefiles" -S . -B "$BUILD"
	cmake -B "$BUILD" --target doc
elif [[ "$OSTYPE" == "linux-gnu" ]]; then
	cmake -S . -B "$BUILD"
	cmake --build "$BUILD" --target doc
fi

# Clean files if asked to
if [[ $1 == "clean" ]]; then
	cmake --build "$BUILD" --target clean
else
	cmake --build "$BUILD"
fi

# Move compile_commands to root so clangd sees it.
if [[ -f "./$BUILD/compile_commands.json" ]]; then
	mv $BUILD/compile_commands.json .
fi

# Run program if successfully compiled
if [[ $1 == "run" ]]; then
	if [[ "$OSTYPE" == "msys" ]]; then
  	"./$EXECUTABLE.exe"
  elif [[ "$OSTYPE" == "linux-gnu" ]]; then
    "./$EXECUTABLE"
  fi
fi
