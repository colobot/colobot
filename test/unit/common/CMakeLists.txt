set(SRC_DIR ${colobot_SOURCE_DIR}/src)

include_directories(
${SRC_DIR}
${GTEST_INCLUDE_DIR}
)

add_executable(image_test ${SRC_DIR}/common/image.cpp image_test.cpp)
target_link_libraries(image_test ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${PNG_LIBRARIES})
add_test(NAME image_test
         COMMAND ${CMAKE_BINARY_DIR}/image_test ${CMAKE_SOURCE_DIR}/test/envs/opengl/tex1.png ${CMAKE_BINARY_DIR}/tex1_test.png)

file(COPY colobot.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(profile_test ${SRC_DIR}/common/profile.cpp ${SRC_DIR}/common/logger.cpp profile_test.cpp)
set_target_properties(profile_test PROPERTIES COMPILE_DEFINITIONS "DEV_BUILD=1")
target_link_libraries(profile_test gtest ${Boost_LIBRARIES})

add_test(NAME profile_test
         COMMAND ${CMAKE_BINARY_DIR}/profile_test
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
