# CBot shared library is built separately
add_subdirectory(CBot) 


# Configure options
option(DEBUG "Enable debug output" ON)

set(PLATFORM_LIBS "")

# Source files
# Commented out files are still dependent on DirectX or WinAPI

set(SOURCES
app/CClass.cpp
app/main.cpp
#app/routines.cpp
app/CBotDoc.cpp
app/CBotConsole.cpp
)

set(LIBS
CBot
)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/CBot)

add_executable(CBot_console ${SOURCES})

target_link_libraries(CBot_console ${LIBS})
