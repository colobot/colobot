
#include <sstream>
#include "CBot/CBotInstr/CBotExprRetVar.h"

#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotInstrMethode.h"
#include "CBot/CBotInstr/CBotIndexExpr.h"
#include "CBot/CBotInstr/CBotFieldExpr.h"

#include "CBot/CBotStack.h"

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotExprRetVar::CBotExprRetVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotExprRetVar::~CBotExprRetVar()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotInstr* CBotExprRetVar::Compile(CBotToken*& p, CBotCStack* pStack)
{
    if (p->GetType() == ID_DOT)
    {
        CBotVar*     var = pStack->GetVar();

        if (var == nullptr)
        {
            pStack->SetError(CBotErrNoTerminator, p->GetStart());
            return nullptr;
        }

        CBotCStack* pStk = pStack->TokenStack();
        CBotInstr* inst = new CBotExprRetVar();

        while (true)
        {
            pStk->SetStartError(p->GetStart());
            if (var->GetType() == CBotTypArrayPointer)
            {
                if (IsOfType( p, ID_OPBRK ))
                {
                    CBotIndexExpr* i = new CBotIndexExpr();
                    i->m_expr = CBotExpression::Compile(p, pStk);
                    inst->AddNext3(i);

                    var = var->GetItem(0,true);

                    if (i->m_expr == nullptr || pStk->GetType() != CBotTypInt)
                    {
                        pStk->SetError(CBotErrBadIndex, p->GetStart());
                        goto err;
                    }
                    if (!pStk->IsOk() || !IsOfType( p, ID_CLBRK ))
                    {
                        pStk->SetError(CBotErrCloseIndex, p->GetStart());
                        goto err;
                    }
                    continue;
                }
            }
            if (var->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypPointer)
            {
                if (IsOfType(p, ID_DOT))
                {
                    CBotToken* pp = p;

                    if (p->GetType() == TokenTypVar)
                    {
                        if (p->GetNext()->GetType() == ID_OPENPAR)
                        {
                            CBotInstr* i = CBotInstrMethode::Compile(p, pStk, var);
                            if (!pStk->IsOk()) goto err;
                            inst->AddNext3(i);
                            return pStack->Return(inst, pStk);
                        }
                        else
                        {
                            CBotFieldExpr* i = new CBotFieldExpr();
                            i->SetToken(pp);
                            inst->AddNext3(i);
                            var = var->GetItem(p->GetString());
                            if (var != nullptr)
                            {
                                i->SetUniqNum(var->GetUniqNum());
                                if ( var->IsPrivate() &&
                                 !pStk->GetProgram()->m_bCompileClass)
                                {
                                    pStk->SetError(CBotErrPrivate, pp);
                                    goto err;
                                }
                            }
                        }

                        if (var != nullptr)
                        {
                            p = p->GetNext();
                            continue;
                        }
                        pStk->SetError(CBotErrUndefItem, p);
                        goto err;
                    }
                    pStk->SetError(CBotErrUndefClass, p);
                    goto err;
                }
            }
            break;
        }

        pStk->SetCopyVar(var);
        if (pStk->IsOk()) return pStack->Return(inst, pStk);

        pStk->SetError(CBotErrUndefVar, p);
err:
        delete inst;
        return pStack->Return(nullptr, pStk);
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotExprRetVar::Execute(CBotStack* &pj)
{

    CBotStack* pile = pj->AddStack();
    CBotStack* pile1 = pile;
    CBotVar* pVar;

    if (pile1->GetState() == 0)
    {
        pVar = pj->GetVar();
        pVar->Update(pj->GetUserPtr());
        if (pVar->GetType(CBotVar::GetTypeMode::CLASS_AS_POINTER) == CBotTypNullPointer)
        {
            pile1->SetError(CBotErrNull, &m_token);
            return pj->Return(pile1);
        }

        if ( !m_next3->ExecuteVar(pVar, pile, &m_token, true, false) )
            return false;

        if (pVar)
            pile1->SetCopyVar(pVar);
        else
            return pj->Return(pile1);

        pile1->IncState();
    }
    pVar = pile1->GetVar();

    if (pVar == nullptr)
    {
        return pj->Return(pile1);
    }

    if (pVar->IsUndefined())
    {
        pile1->SetError(CBotErrNotInit, &m_token);
        return pj->Return(pile1);
    }
    return pj->Return(pile1);
}

////////////////////////////////////////////////////////////////////////////////
void CBotExprRetVar::RestoreState(CBotStack* &pj, bool bMain)
{
    if (!bMain) return;

    CBotStack* pile = pj->RestoreStack();
    if ( pile == nullptr ) return;

    if (pile->GetState() == 0)
        m_next3->RestoreStateVar(pile, bMain);
}

std::string CBotExprRetVar::GetDebugData()
{
    std::stringstream ss;
    ss << m_token.GetString() << "func(...).something" << std::endl;
    return ss.str();
}

} // namespace CBot
