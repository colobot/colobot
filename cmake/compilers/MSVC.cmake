message(STATUS "Detected MSVC compiler")

if(USE_STATIC_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Increase the stack size to 8 MB (the default is 1 MB), needed for CBOT
add_link_options(/STACK:8388608)
