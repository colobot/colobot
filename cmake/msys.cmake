# Hacks for MSYS
if (MSYS)
    message(STATUS "Detected MSYS build")

    set(PLATFORM_LINUX 0)
    set(PLATFORM_OTHER 0)

    add_compile_options(-U__STRICT_ANSI__) # fixes putenv()
endif()
