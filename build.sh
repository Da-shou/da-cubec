export BUILD="build"

# Compile project using CMake
if [[ "$OSTYPE" == "msys" ]]; then
	cmake -G "MinGW Makefiles" -S . -B "$BUILD"
elif [[ "$OSTYPE" == "linux-gnu" ]]; then
	cmake -S . -B "$BUILD"
fi

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
if [[ -f "./$BUILD/da_renderer" ]] && [[ $1 == "run" ]]; then
	"./$BUILD/da_renderer"
fi
