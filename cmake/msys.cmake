# Hacks for MSYS
if (MSYS)
    message(STATUS "Detected MSYS build")

    add_compile_options(-U__STRICT_ANSI__) # fixes putenv()
endif()
