include(FetchContent)

FetchContent_Declare(
    simpleini
    GIT_REPOSITORY https://github.com/brofield/simpleini.git
    GIT_TAG        v4.19
)

message(STATUS "Fetching simpleini...")
FetchContent_MakeAvailable(simpleini)

add_library(simpleini INTERFACE)
target_include_directories(simpleini INTERFACE ${simpleini_SOURCE_DIR})
