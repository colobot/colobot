
#pragma once

#include "CBot/CBotInstr/CBotInstr.h"

namespace CBot
{

/**
 * \brief Access a member/element of the variable on the stack
 *
 *
 *
 */
class CBotExprRetVar : public CBotInstr
{
public:
    CBotExprRetVar();
    ~CBotExprRetVar();

    static CBotInstr* Compile(CBotToken*& p, CBotCStack* pStack);

    /*!
     * \brief Execute
     * \param pj
     * \return
     */
    bool Execute(CBotStack* &pj) override;

    /*!
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    void RestoreState(CBotStack* &pj, bool bMain) override;

protected:
    virtual const std::string GetDebugName() override { return "CBotExprRetVar"; }
    virtual std::string GetDebugData() override;

private:

};

} // namespace CBot
