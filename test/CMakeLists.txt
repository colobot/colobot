# Unit test data
set(TEST_FILES
    data/colobot.json
)

file(COPY ${TEST_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Targets
add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

add_executable(Colobot-UnitTests
    src/main.cpp

    src/app/app_test.cpp

    src/CBot/CBot_test.cpp
    src/CBot/CBotFileUtils_test.cpp
    src/CBot/CBotToken_test.cpp

    src/common/config_file_test.cpp
    src/common/stringutils_test.cpp
    src/common/timeutils_test.cpp

    #src/graphics/engine/lightman_test.cpp

    src/math/func_test.cpp
    src/math/geometry_test.cpp
    src/math/matrix_test.cpp
    src/math/vector_test.cpp
)

target_include_directories(Colobot-UnitTests PRIVATE
    src/common
    src/math
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${COLOBOT_LOCAL_INCLUDES}
)

target_link_libraries(Colobot-UnitTests PRIVATE
    GTest::GTest
    hippomocks
    Colobot-Base
    cpptrace::cpptrace
)

# Needed for shared library builds on windows:  copy cpptrace.dll to the same directory as the
# executable for your_target
if(WIN32)
  add_custom_command(
    TARGET Colobot-UnitTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:cpptrace::cpptrace>
    $<TARGET_FILE_DIR:Colobot-UnitTests>
  )
endif()

gtest_discover_tests(Colobot-UnitTests
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    DISCOVERY_MODE PRE_TEST
)

# GoogleTest isn't compatible with -Wsuggest-override -Werror:
# see https://github.com/google/googletest/issues/1063
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    target_compile_options(Colobot-UnitTests PRIVATE "-Wno-suggest-override")
endif()

if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("test/unit" Colobot-UnitTests)
endif()
