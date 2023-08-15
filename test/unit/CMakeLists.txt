set(TEST_FILES
    common/colobot.json
)

file(COPY ${TEST_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Targets

add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

add_executable(colobot_ut
    main.cpp
    app/app_test.cpp
    CBot/CBot_test.cpp
    CBot/CBotFileUtils_test.cpp
    CBot/CBotToken_test.cpp
    common/config_file_test.cpp
    common/stringutils_test.cpp
    common/timeutils_test.cpp
    #graphics/engine/lightman_test.cpp
    math/func_test.cpp
    math/geometry_test.cpp
    math/matrix_test.cpp
    math/vector_test.cpp)

target_include_directories(colobot_ut PRIVATE
    common
    math
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${COLOBOT_LOCAL_INCLUDES}
)

if(MSVC)
    target_compile_options(colobot_ut PRIVATE /utf-8)
endif()

target_link_libraries(colobot_ut PRIVATE GTest::GTest hippomocks colobotbase)

gtest_discover_tests(colobot_ut
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    DISCOVERY_MODE PRE_TEST
)

# GoogleTest isn't compatible with -Wsuggest-override -Werror:
# see https://github.com/google/googletest/issues/1063
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    target_compile_options(colobot_ut PRIVATE "-Wno-suggest-override")
endif()

if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("test/unit" colobot_ut)
endif()
