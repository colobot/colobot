///////////////////////////////////////////////////
// expression du genre Opérande1 > Opérande2
//					   Opérande1 != Opérande2
// etc.

#include "CBot.h"

// divers constructeurs

CBotCompExpr::CBotCompExpr()
{
	m_leftop	=
	m_rightop	= NULL;
	name = "CBotCompExpr";
}

CBotCompExpr::~CBotCompExpr()
{
	delete	m_leftop;
	delete	m_rightop;
}


// compile une instruction de type A < B

CBotInstr* CBotCompExpr::Compile(CBotToken* &p, CBotCStack* pStack)
{
	CBotCStack* pStk = pStack->TokenStack();

	CBotInstr*	left = CBotAddExpr::Compile( p, pStk );		// expression A + B à gauche
	if (left == NULL) return pStack->Return(NULL, pStk);	// erreur

	if ( p->GetType() == ID_HI ||
		 p->GetType() == ID_LO ||
		 p->GetType() == ID_HS ||
		 p->GetType() == ID_LS ||
		 p->GetType() == ID_EQ ||
		 p->GetType() == ID_NE)								// les diverses comparaisons
	{
		CBotCompExpr* inst = new CBotCompExpr();			// élément pour opération
		inst->SetToken(p);									// mémorise l'opération

		int			 type1, type2;
		type1 = pStk->GetType(CBotTypChar);

		p = p->Next();
		if ( NULL != (inst->m_rightop = CBotAddExpr::Compile( p, pStk )) )	// expression A + B à droite
		{
			type2 = pStk->GetType(CBotTypChar);
			// les résultats sont-ils compatibles
			if ( type1 == type2 && type1 != CBotTypBoolean && type1 != CBotTypClass)
			{
				inst->m_leftop = left;
				pStk->SetVar(new CBotVar(NULL, CBotTypBoolean));
															// le résultat est un boolean
				return pStack->Return(inst, pStk);
			}
			pStk->SetError(TX_BAD2TYPE, &inst->m_token);
		}

		delete left;
		delete inst;
		return pStack->Return(NULL, pStk);
	}

	return pStack->Return(left, pStk);
}


// fait l'opération

BOOL CBotCompExpr::Execute(CBotStack* &pStack)
{
	CBotStack* pStk1 = pStack->AddStack();

	if ( pStk1->GetState() == 0 && !m_leftop->Execute(pStk1) ) return FALSE; // interrompu ici ?

	pStk1->SetState(1);		// opération terminée

	// demande un peu plus de stack pour ne pas toucher le résultat de gauche
	CBotStack* pStk2 = pStk1->AddStack();

	if ( !m_rightop->Execute(pStk2) ) return FALSE; // interrompu ici ?

	int		type1 = pStk1->GetType();
	int		type2 = pStk2->GetType();

	CBotVar*	temp   = CBotVar::Create( NULL, MAX(type1, type2) );
	CBotVar*	result = CBotVar::Create( NULL, CBotTypBoolean );

	switch (GetTokenType())
	{
	case ID_LO:
		temp->Lo(pStk1->GetVar(), pStk2->GetVar());		// inférieur
		break;
	case ID_HI:
		temp->Hi(pStk1->GetVar(), pStk2->GetVar());		// supérieur
		break;
	case ID_LS:
		temp->Ls(pStk1->GetVar(), pStk2->GetVar());		// inférieur ou égal
		break;
	case ID_HS:
		temp->Hs(pStk1->GetVar(), pStk2->GetVar());		// supérieur ou égal
		break;
	case ID_EQ:
		temp->Eq(pStk1->GetVar(), pStk2->GetVar());		// égal
		break;
	case ID_NE:
		temp->Ne(pStk1->GetVar(), pStk2->GetVar());		// différent
		break;
	}
	result->SetValInt(temp->GetValInt());				// converti le résultat
	delete temp;

	pStk2->SetVar(result);				// met le résultat sur la pile

	pStk1->Return(pStk2);				// libère la pile
	return pStack->Return(pStk1);		// transmet le résultat
}

