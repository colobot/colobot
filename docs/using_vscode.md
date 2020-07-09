# Using vscode

## Compilation and installation
### Prerequisites

* before compilation you have to instal tools and dependencies (points 1,2,3) - https://github.com/colobot/colobot/wiki/How-to-Build-Colobot%3A-Gold-Edition-Using-MSVC

### Cloning project
execute following command:
```
git clone https://github.com/MrJohn10/colobot.git
```
in order to clone 'data' submodlue you also have to execute: <b> (this module is needed to launch the game)</b>
```
git submodule update --init
```
if you want you can combine this commands and execute:
```
git clone https://github.com/MrJohn10/colobot.git --recurse-submodules
```
### Configuring vscode

* open project folder in vscode
* install extension [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* install extensnion [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* on the status line at the bottom you can choose cmake configuration and compiler

### Adding cmake settings:
* create folder .vscode if there is none. Inside that folder create file settings.json with the following content:
```json
{
    "cmake.configureSettings": {
            "name": "x64-Debug",
            "generator": "Ninja",
            "configurationType": "Debug",
            "inheritEnvironments": [ "msvc_x64_x64" ],
            "CMAKE_TOOLCHAIN_FILE":{input path to toolchain}
            "VCPKG_TARGET_TRIPLET": "x64-windows-static",
            "BOOST_STATIC": "1",
            "GLEW_STATIC": "1",
            "MSVC_STATIC": "1"
        },
    "cmake.buildDirectory": "${workspaceFolder}\\out\\build\\x64-Debug",
    "cmake.installPrefix": "${workspaceFolder}\\out\\build\\x64-Debug",
    "cmake.generator": "ninja"
}
```

### Compilation and installation
* Open cmake extension in the left menu
* click on 'configure all projects'
* on the status line at the bottom click compilation target and choose install
* click compile
* click launch

If you have any problems create an issue or talk to us on our Discord channel: https://discord.gg/TRCJRc