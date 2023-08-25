target_sources(Colobot-Base PRIVATE
    config_file.cpp
    config_file.h
    error.h
    event.cpp
    event.h
    font_loader.cpp
    font_loader.h
    global.h
    image.cpp
    image.h
    ioutils.h
    key.cpp
    key.h
    language.cpp
    language.h
    logger.cpp
    logger.h
    profiler.cpp
    profiler.h
    regex_utils.cpp
    regex_utils.h
    restext.cpp
    restext.h
    settings.cpp
    settings.h
    singleton.h
    timeutils.cpp
    timeutils.h

    resources/inputstream.cpp
    resources/inputstream.h
    resources/inputstreambuffer.cpp
    resources/inputstreambuffer.h
    resources/outputstream.cpp
    resources/outputstream.h
    resources/outputstreambuffer.cpp
    resources/outputstreambuffer.h
    resources/resourcemanager.cpp
    resources/resourcemanager.h
    resources/sdl_file_wrapper.cpp
    resources/sdl_file_wrapper.h
    resources/sdl_memory_wrapper.cpp
    resources/sdl_memory_wrapper.h
    resources/sndfile_wrapper.cpp
    resources/sndfile_wrapper.h

    system/system.cpp
    system/system.h
    system/system_other.cpp
    system/system_other.h
    system/${SYSTEM_CPP_MODULE}
    system/${SYSTEM_H_MODULE}

    thread/worker_thread.h
)

# Configure file
configure_file(config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/config.h")
configure_file(version.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/version.h")
