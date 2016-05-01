set(SOURCES
    CBotCallMethode.cpp
    CBotClass.cpp
    CBotCStack.cpp
    CBotDebug.cpp
    CBotDefParam.cpp
    CBotExternalCall.cpp
    CBotFileUtils.cpp
    CBotProgram.cpp
    CBotStack.cpp
    CBotToken.cpp
    CBotTypResult.cpp
    CBotUtils.cpp
    CBotInstr/CBotBlock.cpp
    CBotInstr/CBotBoolExpr.cpp
    CBotInstr/CBotBreak.cpp
    CBotInstr/CBotCase.cpp
    CBotInstr/CBotCatch.cpp
    CBotInstr/CBotCondition.cpp
    CBotInstr/CBotDefArray.cpp
    CBotInstr/CBotDefBoolean.cpp
    CBotInstr/CBotDefClass.cpp
    CBotInstr/CBotDefFloat.cpp
    CBotInstr/CBotDefInt.cpp
    CBotInstr/CBotDefString.cpp
    CBotInstr/CBotDo.cpp
    CBotInstr/CBotEmpty.cpp
    CBotInstr/CBotExpression.cpp
    CBotInstr/CBotExprLitBool.cpp
    CBotInstr/CBotExprLitNan.cpp
    CBotInstr/CBotExprLitNull.cpp
    CBotInstr/CBotExprLitNum.cpp
    CBotInstr/CBotExprLitString.cpp
    CBotInstr/CBotExprUnaire.cpp
    CBotInstr/CBotExprVar.cpp
    CBotInstr/CBotFieldExpr.cpp
    CBotInstr/CBotFor.cpp
    CBotInstr/CBotFunction.cpp
    CBotInstr/CBotIf.cpp
    CBotInstr/CBotIndexExpr.cpp
    CBotInstr/CBotInstrCall.cpp
    CBotInstr/CBotInstr.cpp
    CBotInstr/CBotInstrMethode.cpp
    CBotInstr/CBotInstrUtils.cpp
    CBotInstr/CBotLeftExpr.cpp
    CBotInstr/CBotLeftExprVar.cpp
    CBotInstr/CBotListArray.cpp
    CBotInstr/CBotListExpression.cpp
    CBotInstr/CBotListInstr.cpp
    CBotInstr/CBotLogicExpr.cpp
    CBotInstr/CBotNew.cpp
    CBotInstr/CBotParExpr.cpp
    CBotInstr/CBotPostIncExpr.cpp
    CBotInstr/CBotPreIncExpr.cpp
    CBotInstr/CBotReturn.cpp
    CBotInstr/CBotSwitch.cpp
    CBotInstr/CBotThrow.cpp
    CBotInstr/CBotTry.cpp
    CBotInstr/CBotTwoOpExpr.cpp
    CBotInstr/CBotWhile.cpp
    CBotVar/CBotVarArray.cpp
    CBotVar/CBotVarBoolean.cpp
    CBotVar/CBotVarClass.cpp
    CBotVar/CBotVar.cpp
    CBotVar/CBotVarFloat.cpp
    CBotVar/CBotVarInt.cpp
    CBotVar/CBotVarPointer.cpp
    CBotVar/CBotVarString.cpp
    stdlib/Compilation.cpp
    stdlib/FileFunctions.cpp
    stdlib/MathFunctions.cpp
    stdlib/StringFunctions.cpp
)

generate_headers(HEADERS "${SOURCES}")

# Includes
set(LOCAL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

include_directories(${LOCAL_INCLUDES})


if(CBOT_STATIC)
    add_library(CBot STATIC ${SOURCES} ${HEADERS})
else()
    add_library(CBot SHARED ${SOURCES} ${HEADERS})
    install(TARGETS CBot LIBRARY
            DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})
endif()

group_sources("${SOURCES}" "Source Files")
group_sources("${HEADERS}" "Header Files")
