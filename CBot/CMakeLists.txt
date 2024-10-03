if(CBOT_STATIC)
    add_library(CBot STATIC)
else()
    add_library(CBot SHARED)

    install(TARGETS CBot
            LIBRARY DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})
endif()

set_target_properties(CBot PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VISIBILITY_INLINES_HIDDEN ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_sources(CBot PRIVATE
    src/CBot/CBot.h
    src/CBot/CBotCStack.cpp
    src/CBot/CBotCStack.h
    src/CBot/CBotClass.cpp
    src/CBot/CBotClass.h
    src/CBot/CBotDebug.cpp
    src/CBot/CBotDebug.h
    src/CBot/CBotDefParam.cpp
    src/CBot/CBotDefParam.h
    src/CBot/CBotDefines.h
    src/CBot/CBotEnums.h
    src/CBot/CBotExternalCall.cpp
    src/CBot/CBotExternalCall.h
    src/CBot/CBotFileUtils.cpp
    src/CBot/CBotFileUtils.h
    src/CBot/CBotInstr/CBotBlock.cpp
    src/CBot/CBotInstr/CBotBlock.h
    src/CBot/CBotInstr/CBotBoolExpr.cpp
    src/CBot/CBotInstr/CBotBoolExpr.h
    src/CBot/CBotInstr/CBotBreak.cpp
    src/CBot/CBotInstr/CBotBreak.h
    src/CBot/CBotInstr/CBotCase.cpp
    src/CBot/CBotInstr/CBotCase.h
    src/CBot/CBotInstr/CBotCatch.cpp
    src/CBot/CBotInstr/CBotCatch.h
    src/CBot/CBotInstr/CBotCondition.cpp
    src/CBot/CBotInstr/CBotCondition.h
    src/CBot/CBotInstr/CBotDefArray.cpp
    src/CBot/CBotInstr/CBotDefArray.h
    src/CBot/CBotInstr/CBotDefBoolean.cpp
    src/CBot/CBotInstr/CBotDefBoolean.h
    src/CBot/CBotInstr/CBotDefClass.cpp
    src/CBot/CBotInstr/CBotDefClass.h
    src/CBot/CBotInstr/CBotDefFloat.cpp
    src/CBot/CBotInstr/CBotDefFloat.h
    src/CBot/CBotInstr/CBotDefInt.cpp
    src/CBot/CBotInstr/CBotDefInt.h
    src/CBot/CBotInstr/CBotDefString.cpp
    src/CBot/CBotInstr/CBotDefString.h
    src/CBot/CBotInstr/CBotDo.cpp
    src/CBot/CBotInstr/CBotDo.h
    src/CBot/CBotInstr/CBotEmpty.cpp
    src/CBot/CBotInstr/CBotEmpty.h
    src/CBot/CBotInstr/CBotExprLitBool.cpp
    src/CBot/CBotInstr/CBotExprLitBool.h
    src/CBot/CBotInstr/CBotExprLitChar.cpp
    src/CBot/CBotInstr/CBotExprLitChar.h
    src/CBot/CBotInstr/CBotExprLitNan.cpp
    src/CBot/CBotInstr/CBotExprLitNan.h
    src/CBot/CBotInstr/CBotExprLitNull.cpp
    src/CBot/CBotInstr/CBotExprLitNull.h
    src/CBot/CBotInstr/CBotExprLitNum.cpp
    src/CBot/CBotInstr/CBotExprLitNum.h
    src/CBot/CBotInstr/CBotExprLitString.cpp
    src/CBot/CBotInstr/CBotExprLitString.h
    src/CBot/CBotInstr/CBotExprRetVar.cpp
    src/CBot/CBotInstr/CBotExprRetVar.h
    src/CBot/CBotInstr/CBotExprUnaire.cpp
    src/CBot/CBotInstr/CBotExprUnaire.h
    src/CBot/CBotInstr/CBotExprVar.cpp
    src/CBot/CBotInstr/CBotExprVar.h
    src/CBot/CBotInstr/CBotExpression.cpp
    src/CBot/CBotInstr/CBotExpression.h
    src/CBot/CBotInstr/CBotFieldExpr.cpp
    src/CBot/CBotInstr/CBotFieldExpr.h
    src/CBot/CBotInstr/CBotFor.cpp
    src/CBot/CBotInstr/CBotFor.h
    src/CBot/CBotInstr/CBotFunction.cpp
    src/CBot/CBotInstr/CBotFunction.h
    src/CBot/CBotInstr/CBotIf.cpp
    src/CBot/CBotInstr/CBotIf.h
    src/CBot/CBotInstr/CBotIndexExpr.cpp
    src/CBot/CBotInstr/CBotIndexExpr.h
    src/CBot/CBotInstr/CBotInstr.cpp
    src/CBot/CBotInstr/CBotInstr.h
    src/CBot/CBotInstr/CBotInstrCall.cpp
    src/CBot/CBotInstr/CBotInstrCall.h
    src/CBot/CBotInstr/CBotInstrMethode.cpp
    src/CBot/CBotInstr/CBotInstrMethode.h
    src/CBot/CBotInstr/CBotInstrUtils.cpp
    src/CBot/CBotInstr/CBotInstrUtils.h
    src/CBot/CBotInstr/CBotLeftExpr.cpp
    src/CBot/CBotInstr/CBotLeftExpr.h
    src/CBot/CBotInstr/CBotLeftExprVar.cpp
    src/CBot/CBotInstr/CBotLeftExprVar.h
    src/CBot/CBotInstr/CBotListArray.cpp
    src/CBot/CBotInstr/CBotListArray.h
    src/CBot/CBotInstr/CBotListExpression.cpp
    src/CBot/CBotInstr/CBotListExpression.h
    src/CBot/CBotInstr/CBotListInstr.cpp
    src/CBot/CBotInstr/CBotListInstr.h
    src/CBot/CBotInstr/CBotLogicExpr.cpp
    src/CBot/CBotInstr/CBotLogicExpr.h
    src/CBot/CBotInstr/CBotNew.cpp
    src/CBot/CBotInstr/CBotNew.h
    src/CBot/CBotInstr/CBotParExpr.cpp
    src/CBot/CBotInstr/CBotParExpr.h
    src/CBot/CBotInstr/CBotPostIncExpr.cpp
    src/CBot/CBotInstr/CBotPostIncExpr.h
    src/CBot/CBotInstr/CBotPreIncExpr.cpp
    src/CBot/CBotInstr/CBotPreIncExpr.h
    src/CBot/CBotInstr/CBotRepeat.cpp
    src/CBot/CBotInstr/CBotRepeat.h
    src/CBot/CBotInstr/CBotReturn.cpp
    src/CBot/CBotInstr/CBotReturn.h
    src/CBot/CBotInstr/CBotSwitch.cpp
    src/CBot/CBotInstr/CBotSwitch.h
    src/CBot/CBotInstr/CBotThrow.cpp
    src/CBot/CBotInstr/CBotThrow.h
    src/CBot/CBotInstr/CBotTry.cpp
    src/CBot/CBotInstr/CBotTry.h
    src/CBot/CBotInstr/CBotTwoOpExpr.cpp
    src/CBot/CBotInstr/CBotTwoOpExpr.h
    src/CBot/CBotInstr/CBotWhile.cpp
    src/CBot/CBotInstr/CBotWhile.h
    src/CBot/CBotProgram.cpp
    src/CBot/CBotProgram.h
    src/CBot/CBotNamespace.cpp
    src/CBot/CBotNamespace.h
    src/CBot/CBotStack.cpp
    src/CBot/CBotStack.h
    src/CBot/CBotToken.cpp
    src/CBot/CBotToken.h
    src/CBot/CBotTypResult.cpp
    src/CBot/CBotTypResult.h
    src/CBot/CBotUtils.cpp
    src/CBot/CBotUtils.h
    src/CBot/CBotVar/CBotVar.cpp
    src/CBot/CBotVar/CBotVar.h
    src/CBot/CBotVar/CBotVarValue.h
    src/CBot/CBotVar/CBotVarArray.cpp
    src/CBot/CBotVar/CBotVarArray.h
    src/CBot/CBotVar/CBotVarBoolean.cpp
    src/CBot/CBotVar/CBotVarBoolean.h
    src/CBot/CBotVar/CBotVarByte.h
    src/CBot/CBotVar/CBotVarChar.h
    src/CBot/CBotVar/CBotVarClass.cpp
    src/CBot/CBotVar/CBotVarClass.h
    src/CBot/CBotVar/CBotVarDouble.h
    src/CBot/CBotVar/CBotVarFloat.cpp
    src/CBot/CBotVar/CBotVarFloat.h
    src/CBot/CBotVar/CBotVarInt.cpp
    src/CBot/CBotVar/CBotVarInt.h
    src/CBot/CBotVar/CBotVarLong.h
    src/CBot/CBotVar/CBotVarPointer.cpp
    src/CBot/CBotVar/CBotVarPointer.h
    src/CBot/CBotVar/CBotVarShort.h
    src/CBot/CBotVar/CBotVarString.cpp
    src/CBot/CBotVar/CBotVarString.h
    src/CBot/stdlib/Compilation.cpp
    src/CBot/stdlib/Compilation.h
    src/CBot/stdlib/FileFunctions.cpp
    src/CBot/stdlib/MathFunctions.cpp
    src/CBot/stdlib/StringFunctions.cpp
    src/CBot/stdlib/stdlib.h
    src/CBot/stdlib/stdlib_public.h
)

target_include_directories(CBot PUBLIC src)
target_link_libraries(CBot PRIVATE
    Colobot-Common
)

if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("src/CBot" CBot)
endif()
