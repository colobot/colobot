cmake_minimum_required(VERSION 2.8)

include_directories(. include/clipboard)

add_definitions(-DLIB_COMPILE=1)


if(PLATFORM_WINDOWS)
    set(CLIPBOARD_SRC src/clipboardWin32.c)
elseif(PLATFORM_LINUX)
    set(CLIPBOARD_SRC src/clipboardX11.c)
else()
    set(CLIPBOARD_SRC src/clipboardX11.c)
endif()

include_directories(${SDL_INCLUDE_DIR})
add_library(clipboard STATIC src/utf.c ${CLIPBOARD_SRC})
