#include "CBot/CBotKeywordStrings.h"

#include <map>

//! \brief Keeps the string corresponding to keyword ID
// Map is filled with id-string pars that are needed for CBot language parsing
static const std::map<EID, const char*> s_keywordString = {
    {ID_IF,         "if"},
    {ID_ELSE,       "else"},
    {ID_WHILE,      "while"},
    {ID_DO,         "do"},
    {ID_FOR,        "for"},
    {ID_BREAK,      "break"},
    {ID_CONTINUE,   "continue"},
    {ID_SWITCH,     "switch"},
    {ID_CASE,       "case"},
    {ID_DEFAULT,    "default"},
    {ID_TRY,        "try"},
    {ID_THROW,      "throw"},
    {ID_CATCH,      "catch"},
    {ID_FINALLY,    "finally"},
    {ID_TXT_AND,    "and"},
    {ID_TXT_OR,     "or"},
    {ID_TXT_NOT,    "not"},
    {ID_RETURN,     "return"},
    {ID_CLASS,      "class"},
    {ID_EXTENDS,    "extends"},
    {ID_SYNCHO,     "synchronized"},
    {ID_NEW,        "new"},
    {ID_PUBLIC,     "public"},
    {ID_EXTERN,     "extern"},
    {ID_STATIC,     "static"},
    {ID_PROTECTED,  "protected"},
    {ID_PRIVATE,    "private"},
    {ID_INT,        "int"},
    {ID_FLOAT,      "float"},
    {ID_BOOLEAN,    "boolean"},
    {ID_STRING,     "string"},
    {ID_VOID,       "void"},
    {ID_BOOL,       "bool"},
    {ID_TRUE,       "true"},
    {ID_FALSE,      "false"},
    {ID_NULL,       "null"},
    {ID_NAN,        "nan"},
    {ID_OPENPAR,    "("},
    {ID_CLOSEPAR,   ")"},
    {ID_OPBLK,      "{"},
    {ID_CLBLK,      "}"},
    {ID_SEP,        ";"},
    {ID_COMMA,      ","},
    {ID_DOTS,       ":"},
    {ID_DOT,        "."},
    {ID_OPBRK,      "["},
    {ID_CLBRK,      "]"},
    {ID_DBLDOTS,    "::"},
    {ID_LOGIC,      "?"},
    {ID_ADD,        "+"},
    {ID_SUB,        "-"},
    {ID_MUL,        "*"},
    {ID_DIV,        "/"},
    {ID_ASS,        "="},
    {ID_ASSADD,     "+="},
    {ID_ASSSUB,     "-="},
    {ID_ASSMUL,     "*="},
    {ID_ASSDIV,     "/="},
    {ID_ASSOR,      "|="},
    {ID_ASSAND,     "&="},
    {ID_ASSXOR,     "^="},
    {ID_ASSSL,      "<<="},
    {ID_ASSSR,      ">>>="},
    {ID_ASSASR,     ">>="},
    {ID_SL,         "<<"},
    {ID_SR,         ">>"},
    {ID_ASR,        ">>"},
    {ID_INC,        "++"},
    {ID_DEC,        "--"},
    {ID_LO,         "<"},
    {ID_HI,         ">"},
    {ID_LS,         "<="},
    {ID_HS,         ">="},
    {ID_EQ,         "=="},
    {ID_NE,         "!="},
    {ID_AND,        "&"},
    {ID_XOR,        "^"},
    {ID_OR,         "|"},
    {ID_LOG_AND,    "&&"},
    {ID_LOG_OR,     "||"},
    {ID_LOG_NOT,    "!"},
    {ID_NOT,        "~"},
    {ID_MODULO,     "%"},
    {ID_POWER,      "**"},
    {ID_ASSMODULO,  "%="},
    {TX_UNDEF,      "undefined"},
    {TX_NAN,        "not a number"}
};

static const char emptyString[] = "";

const char* LoadString(EID id)
{
    if (s_keywordString.find(id) != s_keywordString.end())
    {
        return s_keywordString.at(id);
    }
    else
    {
        return emptyString;
    }
}