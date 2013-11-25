# Hacks for MSYS
if (MSYS AND (NOT MXE))
    message(STATUS "Detected MSYS build")

    set(PLATFORM_WINDOWS 1)
    set(PLATFORM_LINUX 0)
    set(PLATFORM_OTHER 0)

    set(COLOBOT_CXX_FLAGS "${COLOBOT_CXX_FLAGS} -U__STRICT_ANSI__") # fixes putenv()
    set(USE_SDL_MAIN 1) # fixes SDL_main
endif()
