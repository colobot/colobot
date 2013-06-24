set(SRC_DIR ${colobot_SOURCE_DIR}/src)

include_directories(
.
${SRC_DIR}
${GTEST_INCLUDE_DIR}
${GMOCK_INCLUDE_DIR}
${CLIPBOARD_INCLUDE_DIR}
)

# Platform-dependent implementation of CSystemUtils
if(PLATFORM_WINDOWS)
    set(SYSTEM_CPP_MODULE "system_windows.cpp")
elseif(PLATFORM_LINUX)
    set(SYSTEM_CPP_MODULE "system_linux.cpp")
    set(ADDITIONAL_LIB "-lX11")
else()
    set(SYSTEM_CPP_MODULE "system_other.cpp")
    set(ADDITIONAL_LIB "-lX11")
endif()

add_executable(edit_test
${SRC_DIR}/app/system.cpp
${SRC_DIR}/app/${SYSTEM_CPP_MODULE}
${SRC_DIR}/common/event.cpp
${SRC_DIR}/common/logger.cpp
${SRC_DIR}/common/misc.cpp
${SRC_DIR}/common/profile.cpp
${SRC_DIR}/common/iman.cpp
${SRC_DIR}/common/stringutils.cpp
${SRC_DIR}/graphics/engine/text.cpp
${SRC_DIR}/ui/button.cpp
${SRC_DIR}/ui/control.cpp
${SRC_DIR}/ui/edit.cpp
${SRC_DIR}/ui/scroll.cpp
stubs/app_stub.cpp
stubs/engine_stub.cpp
stubs/particle_stub.cpp
stubs/restext_stub.cpp
stubs/robotmain_stub.cpp
edit_test.cpp)

target_link_libraries(edit_test gtest gmock clipboard ${SDL_LIBRARY} ${SDLTTF_LIBRARY} ${Boost_LIBRARIES} ${ADDITIONAL_LIB})



add_test(edit_test ./edit_test)
