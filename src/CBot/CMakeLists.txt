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
    CBotInstr/CBotWhile.cpp
    CBotInstr/CBotDo.cpp
    CBotInstr/CBotFor.cpp
    CBotInstr/CBotListExpression.cpp
    CBotInstr/CBotSwitch.cpp
    CBotInstr/CBotCase.cpp
    CBotInstr/CBotBreak.cpp
    CBotInstr/CBotTry.cpp
    CBotInstr/CBotCatch.cpp
    CBotInstr/CBotThrow.cpp
    CBotInstr/CBotExprAlpha.cpp
    CBotInstr/CBotExprNum.cpp
    CBotInstr/CBotNew.cpp
    CBotInstr/CBotExprNan.cpp
    CBotInstr/CBotExprNull.cpp
    CBotInstr/CBotExprBool.cpp
    CBotInstr/CBotLeftExprVar.cpp
    CBotInstr/CBotPreIncExpr.cpp
)

# Includes
set(LOCAL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(${LOCAL_INCLUDES})


if(CBOT_STATIC)
    add_library(CBot STATIC ${SOURCES})
else()
    add_library(CBot SHARED ${SOURCES})
    install(TARGETS CBot LIBRARY
            DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})
endif()

