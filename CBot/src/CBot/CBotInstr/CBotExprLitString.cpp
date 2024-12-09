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

#include "CBot/CBotInstr/CBotExprLitString.h"

#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include <common/codepoint.h>
#include <common/stringutils.h>

#include <stdexcept>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprLitString::CBotExprLitString()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotExprLitString::~CBotExprLitString()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprLitString::Compile(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    const auto& s = p->GetString();

    auto it = s.cbegin();
    if (++it != s.cend())
    {
        int pos = p->GetStart();
        std::string valstring = "";
        while (it != s.cend() && *it != '\"')
        {
            ++pos;
            if (*it != '\\') // not escape sequence ?
            {
                valstring += *(it++);
                continue;
            }

            if (++it == s.cend()) break;
            pStk->SetStartError(pos);

            if (CharIsOctalNum(*it))                  // octal
            {
                std::string octal = "";

                for (int i = 0; i < 3; i++)
                {
                    if (!CharIsOctalNum(*it)) break;
                    ++pos;
                    octal += *it;
                    if (++it == s.cend()) break;
                }

                unsigned int val = std::stoi(octal, nullptr, 8);
                if (val <= 255)
                {
                    valstring.push_back(val);
                    continue;
                }
                pStk->SetError(CBotErrOctalRange, pos + 1);
            }
            else
            {
                ++pos;
                unsigned char c = *(it++);
                if (c == '\"' || c == '\'' || c == '\\') valstring += c;
                else if (c == 'a') valstring += '\a'; // alert bell
                else if (c == 'b') valstring += '\b'; // backspace
                else if (c == 'f') valstring += '\f'; // form feed
                else if (c == 'n') valstring += '\n'; // new line
                else if (c == 'r') valstring += '\r'; // carriage return
                else if (c == 't') valstring += '\t'; // horizontal tab
                else if (c == 'v') valstring += '\v'; // vertical tab
                else if (c == 'x' || c == 'u' || c == 'U') // hex or unicode
                {
                    if (it != s.cend())
                    {
                        std::string hex = "";
                        bool isHexCode = (c == 'x');
                        size_t maxlen = (c == 'u') ? 4 : 8;

                        for (size_t i = 0; isHexCode || i < maxlen; i++)
                        {
                            if (!CharIsHexNum(*it)) break;
                            ++pos;
                            hex += *it;
                            if (++it == s.cend()) break;
                        }

                        if (!hex.empty())
                        {
                            unsigned int val = 0;
                            try
                            {
                                val = std::stoi(hex, nullptr, 16);
                            }
                            catch (const std::out_of_range& e)
                            {
                                pStk->SetError(CBotErrHexRange, pos + 1);
                            }

                            if (pStk->IsOk())
                            {
                                if (isHexCode)        // hexadecimal
                                {
                                    if (val <= 255)
                                    {
                                        valstring.push_back(val);
                                        continue;
                                    }
                                    pStk->SetError(CBotErrHexRange, pos + 1);
                                }
                                else if (maxlen == hex.length()) // unicode character
                                {
                                    if (val < 0xD800 || (0xDFFF < val && val < 0x110000))
                                    {
                                        valstring += StrUtils::ToUTF8(val).ToStringView();
                                        continue;
                                    }
                                    pStk->SetError(CBotErrUnicodeName, pos + 1);
                                }
                            }
                        }
                    }

                    pStk->SetError(CBotErrHexDigits, pos + 1);
                }
                else
                    pStk->SetError(CBotErrBadEscape, pos + 1);   // unknown escape code
            }

            if (!pStk->IsOk()) break;
        }

        if (it == s.cend() || *it != '\"')
            pStk->SetError(CBotErrEndQuote, p);

        if (pStk->IsOk())
        {
            CBotExprLitString* inst = new CBotExprLitString();
            inst->m_valstring.swap(valstring);
            inst->SetToken(p);
            p = p->GetNext();

            CBotVar* var = CBotVar::Create("", CBotTypString);
            pStk->SetVar(var);

            return pStack->Return(inst, pStk);
        }
    }

    pStk->SetError(CBotErrEndQuote, p);
    return pStack->Return(nullptr, pStk);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprLitString::Execute(CBotStack* &pj)
{
    CBotStack*    pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar*    var = CBotVar::Create("", CBotTypString);

    var->SetValString(m_valstring);

    pile->SetVar(var);                            // put on the stack

    return pj->Return(pile);
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprLitString::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

std::string CBotExprLitString::GetDebugData()
{
    return m_token.GetString();
}

} // namespace CBot
