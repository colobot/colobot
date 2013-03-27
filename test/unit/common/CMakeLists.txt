set(SRC_DIR ${colobot_SOURCE_DIR}/src)

include_directories(
${SRC_DIR}
${GTEST_INCLUDE_DIR}
)

add_executable(image_test ${SRC_DIR}/common/image.cpp image_test.cpp)
target_link_libraries(image_test ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${PNG_LIBRARIES})

file(COPY colobot.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# add_executable(profile_test ${SRC_DIR}/common/profile.cpp ${SRC_DIR}/common/logger.cpp profile_test.cpp)
# target_link_libraries(profile_test gtest ${Boost_LIBRARIES})

# add_test(profile_test ./profile_test)
