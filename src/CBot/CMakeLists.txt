set(SOURCES
        CBotUtils.cpp
        CBotFileUtils.cpp
        CBotClass.cpp
        CBotProgram.cpp
        CBotStack.cpp
        CBotCStack.cpp
        CBotString.cpp
        CBotToken.cpp
        CBotCall.cpp
        CBotDefParam.cpp
        CBotCallMethode.cpp
        CBotTypResult.cpp
        StringFunctions.cpp
        CBotInstr/CBotInstr.cpp
        CBotInstr/CBotInstrUtils.cpp
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
        CBotInstr/CBotPostIncExpr.cpp
        CBotInstr/CBotExprVar.cpp
        CBotInstr/CBotInstrMethode.cpp
        CBotInstr/CBotInstrCall.cpp
        CBotInstr/CBotListInstr.cpp
        CBotInstr/CBotBlock.cpp
        CBotInstr/CBotExprUnaire.cpp
        CBotInstr/CBotParExpr.cpp
        CBotInstr/CBotBoolExpr.cpp
        CBotInstr/CBotLogicExpr.cpp
        CBotInstr/CBotTwoOpExpr.cpp
        CBotInstr/CBotExpression.cpp
        CBotInstr/CBotIndexExpr.cpp
        CBotInstr/CBotFieldExpr.cpp
        CBotInstr/CBotLeftExpr.cpp
        CBotInstr/CBotCondition.cpp
        CBotInstr/CBotClassInst.cpp
        CBotInstr/CBotIString.cpp
        CBotInstr/CBotFloat.cpp
        CBotInstr/CBotBoolean.cpp
        CBotInstr/CBotEmpty.cpp
        CBotInstr/CBotReturn.cpp
        CBotInstr/CBotIf.cpp
        CBotInstr/CBotListArray.cpp
        CBotInstr/CBotInstArray.cpp
        CBotInstr/CBotInt.cpp
        CBotInstr/CBotFunction.cpp
        CBotVar/CBotVarArray.cpp
        CBotVar/CBotVarPointer.cpp
        CBotVar/CBotVarClass.cpp
        CBotVar/CBotVarBoolean.cpp
        CBotVar/CBotVarString.cpp
        CBotVar/CBotVarFloat.cpp
        CBotVar/CBotVarInt.cpp
        CBotVar/CBotVar.cpp
        CBotKeywordStrings.cpp CBotKeywordStrings.h)

# Includes
set(LOCAL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/..
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

