set(SOURCES
CBot.cpp
CBotClass.cpp
CBotFunction.cpp
CBotIf.cpp
CBotProgram.cpp
CBotStack.cpp
CBotString.cpp
CBotToken.cpp
CBotTwoOpExpr.cpp
CBotVar.cpp
CBotWhile.cpp
)

if(${CBOT_STATIC})
    add_library(CBot STATIC ${SOURCES})
else()
    add_library(CBot SHARED ${SOURCES})
    install(TARGETS CBot LIBRARY DESTINATION ${COLOBOT_INSTALL_LIB_DIR})
endif()
