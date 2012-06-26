README for developers

This file outlines the most important things for developers.

1. Goal.

  This branch is the main development branch into which features from other development branches will be pulled in after completing their goals. Currently these branches include:
   * opengl-dev  branch - branch dedicated to rewriting the graphics engine using SDL and OpenGL
   * ...

2. Build system and organisation of directories.

  The directories in the repository are as following:
   src/CBot              separate CBot library
   src/app               class CApplication and everything concerned with SDL plus other system-dependent code such as displaying a message box, finding files, etc.
   src/common            shared structs, enums, defines, etc.; should not have any external dependencies
   src/graphics/common   interface of graphics engine (CEngine) and device (CDevice), without concrete implementation, shared structs such as Vertex, Material, etc., "effects" classes: CCamera, CLight, CParticle that will use the graphics engine interface
   src/graphics/opengl   concrete implementation of CEngine and CDevice classes in OpenGL: CGLEngine and CGLDevice
   src/graphics/d3d      in (far) future - perhaps a newer implementation in DirectX (9? 10?)
   src/math              mathematical structures and functions
   src/object            non-graphical game engine, that is robots, buildings, etc.; dependent only on interface of graphics engine, not on concrete implementation
   src/ui                2D user interface (menu, buttons, check boxes, etc.); also without dependencies to concrete implementation of graphics engine
   src/sound             sound and music engine written using fmod library
   src/physics           physics engine
   src/script            link with the CBot library
   src/old               old modules that will be replaced by new code

  Other directories, not very important right now, include:
   src/doc               contains the Doxygen mainpage text; it will probably be removed to app/main.cpp or some other place soon
   src/metafile          a separate program for packing data files to .dat format

  The build system is as follows:
   /CMakeLists.txt - definition of project, build type setup, general compiler options and reference to src subdirectory
   /src/CMakeLists.txt - currently defines two targets: colobot_old - the original game comprised of old sources, compiles and runs only on Windows; colobot_new - new implementation in SDL, compiles and runs on Windows and Linux, for now only "hello world"-like; colobot_old target references also CBot library
   /src/CBot/CMakeLists.txt - defines the CBot library target

  There is also a generated header common/config.h with #defines set by CMake.

3. For other things, refer to README files in respective feature development branches.

