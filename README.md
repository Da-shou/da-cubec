# da-cubec
## Simple Minecraft clone in C, using glad/GLFW3
The goals of this project are pretty simple. Make a multiplatform minecraft clone using glad/GLFW3.
The project must be buildable on all machines. The project will use CMake and Clang for the formatting.

### Libraries
- GLFW3
  - Used to create a standardized window for the program within the target OS. Works on Windows, Linux and MacOS. 
- glad
  - Well-made and well-documentated framework for working with OpenGL. Has many variants for working with SDL, DirectX etc. I will just be using the core OpenGL features.
- cglm
  - C version of the glm C++ matrix/vector SIMD-optmized library. Allows for super fast matrix and vector operations which are essential to 3D rendering

### Building the project
To make building the project as easy as possible, all libraries are contained within the project. Building has been tested on Windows 10/11 and Linux. **CMake is required.**

From the root folder, to build the executable located in the `build` folder :
```shell
./build.sh
```

To clean all build files and remake CMake files :
```shell
./build.sh clean
```

To build executable and run the project :
```shell
./build.sh run
```

### About
This is a fun project I work on during my free time because I love GPU Programming and the C language. Feel free to use it to learn about OpenGL yourself or to correct anything you'd feel is badly made.
