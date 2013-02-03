set(SRC_DIR ${colobot_SOURCE_DIR}/src)

include_directories(
${SRC_DIR}
${GTEST_INCLUDE_DIR}
math
common
)

set(UT_SOURCES
main.cpp
math/geometry_test.cpp
math/matrix_test.cpp
math/vector_test.cpp
)

add_executable(colobot_ut ${UT_SOURCES})
target_link_libraries(colobot_ut gtest)

add_test(colobot_ut ./colobot_ut)

# TODO: change the unit cases to independent automated tests to be included in colobot_ut
add_subdirectory(common)
add_subdirectory(ui)
