/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#include "CBot/CBotInstr/CBotExprLitChar.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include <common/codepoint.h>
#include <common/stringutils.h>

namespace CBot
{

CBotExprLitChar::CBotExprLitChar(char32_t valchar) :
    m_valchar(valchar)
{
}

CBotExprLitChar::~CBotExprLitChar()
{
}

CBotInstr* CBotExprLitChar::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    const auto& s = p->GetString();

    auto it = s.cbegin();
    if (++it != s.cend())
    {
        if (*it != '\'') // not empty quotes ?
        {
            char32_t valchar = 0;
            int pos = p->GetStart() + 1;

            if (*it != '\\')       // not escape sequence ?
            {
                auto cp = StrUtils::ReadUTF8(s.data() + 1);
                valchar = StrUtils::ToUTF32(cp);
                std::advance(it, cp.Size());
            }
            else if (++it != s.cend())
            {
                pStk->SetStartError(pos++);
                const unsigned char c = *(it++);
                if (c == '\"' || c == '\'' || c == '\\') valchar = c;
                else if (c == 'a') valchar = '\a'; // alert bell
                else if (c == 'b') valchar = '\b'; // backspace
                else if (c == 'f') valchar = '\f'; // form feed
                else if (c == 'n') valchar = '\n'; // new line
                else if (c == 'r') valchar = '\r'; // carriage return
                else if (c == 't') valchar = '\t'; // horizontal tab
                else if (c == 'v') valchar = '\v'; // vertical tab
                else if (c == 'u' || c == 'U') // unicode escape
                {
                    if (it != s.cend())
                    {
                        std::string hex = "";
                        size_t maxlen = (c == 'u') ? 4 : 8;

                        for (size_t i = 0; i < maxlen; i++)
                        {
                            if (!CharIsHexNum(*it)) break;
                            ++pos;
                            hex += *it;
                            if (++it == s.cend()) break;
                        }

                        if (maxlen == hex.length()) // unicode character
                        {
                            try
                            {
                                valchar = std::stoi(hex, nullptr, 16);
                                if (0x10FFFF < valchar || (0xD7FF < valchar && valchar < 0xE000))
                                    pStk->SetError(CBotErrUnicodeName, pos + 1);
                            }
                            catch (const std::out_of_range& e)
                            {
                                pStk->SetError(CBotErrHexRange, pos + 1);
                            }
                        }
                        else
                            pStk->SetError(CBotErrHexDigits, pos + 1);
                    }
                    else
                        pStk->SetError(CBotErrHexDigits, pos + 1);
                }
                else
                    pStk->SetError(CBotErrBadEscape, pos + 1);
            }

            if (it == s.cend() || *it != '\'')
                pStk->SetError(CBotErrEndQuote, p);

            if (pStk->IsOk())
            {
                CBotExprLitChar* inst = new CBotExprLitChar(valchar);
                inst->SetToken(p);
                p = p->GetNext();

                CBotVar* var = CBotVar::Create("", CBotTypChar);
                pStk->SetVar(var);

                return pStack->Return(inst, pStk);
            }
        }
        pStk->SetError(CBotErrCharEmpty, p);
    }

    pStk->SetError(CBotErrEndQuote, p);
    return pStack->Return(nullptr, pStk);
}

bool CBotExprLitChar::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar* var = CBotVar::Create("", CBotTypChar);

    var->SetValChar(m_valchar);

    pile->SetVar(var);

    return pj->Return(pile);
}

void CBotExprLitChar::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

std::string CBotExprLitChar::GetDebugData()
{
    return m_token.GetString();
}

} // namespace CBot
