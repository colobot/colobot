set(SOURCES
CClass.cpp
main.cpp
CBotDoc.cpp
CBotConsole.cpp
)

set(LIBS
CBot
)

include_directories(${colobot_SOURCE_DIR}/src)

add_executable(CBot_console ${SOURCES})

target_link_libraries(CBot_console ${LIBS})
