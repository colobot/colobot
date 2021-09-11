# Platform-dependent tests
if(PLATFORM_WINDOWS)
    set(PLATFORM_TESTS common/system/system_windows_test.cpp)
elseif(PLATFORM_LINUX)
    set(PLATFORM_TESTS common/system/system_linux_test.cpp)
endif()

set(TEST_FILES
    common/colobot.ini
)

file(COPY ${TEST_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Targets

add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

add_executable(colobot_ut
    main.cpp
    app/app_test.cpp
    CBot/CBotToken_test.cpp
    CBot/CBot_test.cpp
    common/config_file_test.cpp
    graphics/engine/lightman_test.cpp
    math/func_test.cpp
    math/geometry_test.cpp
    math/matrix_test.cpp
    math/vector_test.cpp
)

# Platform-dependent tests
if(PLATFORM_WINDOWS)
    target_sources(colobot_ut PRIVATE common/system/system_windows_test.cpp)
elseif(PLATFORM_LINUX)
    target_sources(colobot_ut PRIVATE common/system/system_linux_test.cpp)
endif()

target_include_directories(colobot_ut PRIVATE
    common
    math
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${COLOBOT_LOCAL_INCLUDES}
)

target_link_libraries(colobot_ut PRIVATE GTest::GTest hippomocks colobotbase)

gtest_discover_tests(colobot_ut
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# GoogleTest isn't compatible with -Wsuggest-override -Werror:
# see https://github.com/google/googletest/issues/1063
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    target_compile_options(colobot_ut PRIVATE "-Wno-suggest-override")
endif()

if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("test/unit" colobot_ut)
endif()
