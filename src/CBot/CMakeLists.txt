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

add_library(CBot SHARED ${SOURCES})
