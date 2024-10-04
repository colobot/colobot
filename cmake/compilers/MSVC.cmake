message(STATUS "Detected MSVC compiler")

# Link runtime library statically (currently only works for MSVC)
option(COLOBOT_MSVC_STATIC_RUNTIME "Link the runtime library statically" OFF)

if(COLOBOT_MSVC_STATIC_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

add_compile_options(
    /EHsc
    /utf-8
)

add_link_options(
    /STACK:8388608      # Increase the stack size to 8 MB (the default is 1 MB), needed for CBOT
)
