README for developers

This file outlines the most important things for developers.

1. Goal.

  This branch is dedicated to rewriting the graphics engine as well as window & event handling, sound and all other parts of the project dependent on WinAPI or DirectX, to new platform-independent libraries: SDL, OpenGL and fmod (for sound).
  The general rule is to rewrite the code so that it resembles the old one as closely as possible so that later on, it can be joined easily with the other modules, which will not be touched yet. Of course, it doesn't mean to sacrifice the usability or flexibility of new interfaces, so some cleaning-up is welcome.


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
   /src/CMakeLists.txt - defines the colobot target - new implementation in SDL, compiles and runs on Windows and Linux, for now only "hello world"-like
   /src/CBot/CMakeLists.txt - defines the CBot library target, currently not referenced by colobot target because it is still WinAPI-dependent

  There is also a generated header common/config.h with #defines set by CMake.


3. Plan of work.

  What is done so far:
   - changes in the build system
   - rewriting of math module and reaching independence from old FPOINT, D3DVECTOR and D3DMATRIX structs
   - first draft of class and struct templates in src/graphics/common

  What remains to be done:
   - in CBot library - remove dependencies to WinAPI and translate the comments from French to English
   - write CApplication class, including handling of SDL events, mouse, joystick, etc.
   - (connected with the above) remove dependencies from src/common
   - complete the CDevice and CEngine interfaces and write the concrete implementations of CGLDevice and CGLEngine
   - write the implementation of other classes in graphics engine, matching the old implementation but using only the new interface
   - write the implementation of new sound module
   - rewrite the old UI classes to use new graphics interface
   - rewrite the old src/object classes to use new graphics interface

  Certain tasks regarding the work should be assigned to each developer using Issues on github, so that the division is clear.


3. Rewriting modules.

  The rewriting rule is the following: every old module/header that needs to be replaced by a new one with all declarations either changed to other names or, preferrably, enclosed in a separate namespace. This way, the new code will be separated from the old one, for the time being, thus making it possible to work on new code and not break anything else. Once the functionality of new code matches the old one, it can be replaced in old code with ease.


4. Documentation.

  All new code should be documented in Doxygen. Also, don't hesitate to add comments in the old code, where you deciphered some use or detail that you want to share.


5. Tests.

  Whenever possible, please write unit tests for your code. Tests should go into test subdirectory in each of the code directories and for now, should be independent of main build system (later on, we will combine them, but for now it will be easier this way).


6. Commiting code.

  Code commited to the repository should not break compilation nor tests. Breaking the compilation of old code can be tolerated but the new one - no! If you are uncertain, or want to make partial commit or something like that, commit to your own fork, or if you must, commit but comment out the code that breaks the build.


7. Whitespace rules.

  Please indent with spaces, 1 indentation level = 4 spaces. Unix line endings. And don't leave whitespace at the end of lines. Thank you :)



I will probably think of something more in the future, but that's it for now. Thanks for reading and good luck :)
