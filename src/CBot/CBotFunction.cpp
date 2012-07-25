// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.///////////////////////////////////////////////////////////////////////
// compilation des diverses fonctions d�clar�es par l'utilisateur
//

#include "CBot.h"

// les divers constructeurs / destructeurs
// pour lib�rer tout selon l'arbre �tabli
CBotFunction::CBotFunction()
{
	m_Param		 = NULL;			// liste des param�tres vide
	m_Block		 = NULL;			// le bloc d'instructions
	m_next		 = NULL;			// les fonctions peuvent �tre cha�n�es
	m_bPublic	 = false;			// fonction non publique
	m_bExtern	 = false;			// fonction non externe
	m_nextpublic = NULL;
	m_prevpublic = NULL;
	m_pProg		 = NULL;
//	m_nThisIdent = 0;
	m_nFuncIdent = 0;
	m_bSynchro	  = false;
}

CBotFunction* CBotFunction::m_listPublic = NULL;

CBotFunction::~CBotFunction()
{
	delete	m_Param;				// liste des param�tres vide
	delete	m_Block;				// le bloc d'instructions
	delete	m_next;

	// enl�ve de la liste publique s'il y a lieu
	if ( m_bPublic )
	{
		if ( m_nextpublic != NULL )
		{
			m_nextpublic->m_prevpublic = m_prevpublic;
		}
		if ( m_prevpublic != NULL)
		{
			m_prevpublic->m_nextpublic = m_nextpublic;
		}
		else
		{
			// si prev = next = null peut ne pas �tre dans la liste !
			if ( m_listPublic == this ) m_listPublic = m_nextpublic;
		}
	}
}

bool CBotFunction::IsPublic()
{
	return m_bPublic;
}

bool CBotFunction::IsExtern()
{
	return m_bExtern;
}

bool CBotFunction::GetPosition(int& start, int& stop, CBotGet modestart, CBotGet modestop)
{
	start = m_extern.GivStart();
	stop = m_closeblk.GivEnd();

	if (modestart == GetPosExtern)
	{
		start = m_extern.GivStart();
	}
	if (modestop == GetPosExtern)
	{
		stop = m_extern.GivEnd();
	}
	if (modestart == GetPosNom)
	{
		start = m_token.GivStart();
	}
	if (modestop == GetPosNom)
	{
		stop = m_token.GivEnd();
	}
	if (modestart == GetPosParam)
	{
		start = m_openpar.GivStart();
	}
	if (modestop == GetPosParam)
	{
		stop = m_closepar.GivEnd();
	}
	if (modestart == GetPosBloc)
	{
		start = m_openblk.GivStart();
	}
	if (modestop == GetPosBloc)
	{
		stop = m_closeblk.GivEnd();
	}

	return true;
}


CBotTypResult	ArrayType(CBotToken* &p, CBotCStack* pile, CBotTypResult type)
{
	while ( IsOfType( p, ID_OPBRK ) )
	{
		if ( !IsOfType( p, ID_CLBRK ) )
		{
			pile->SetError(TX_CLBRK, p->GivStart());
			return CBotTypResult( -1 );
		}
		type = CBotTypResult( CBotTypArrayPointer, type );
	}
	return type;
}

CBotTypResult	TypeParam(CBotToken* &p, CBotCStack* pile)
{
	CBotClass*	pClass = NULL;

	switch (p->GivType())
	{
	case ID_INT:
		p = p->GivNext();
		return ArrayType(p, pile, CBotTypResult( CBotTypInt ));
	case ID_FLOAT:
		p = p->GivNext();
		return ArrayType(p, pile, CBotTypResult( CBotTypFloat ));
	case ID_BOOLEAN:
	case ID_BOOL:
		p = p->GivNext();
		return ArrayType(p, pile, CBotTypResult( CBotTypBoolean ));
	case ID_STRING:
		p = p->GivNext();
		return ArrayType(p, pile, CBotTypResult( CBotTypString ));
	case ID_VOID:
		p = p->GivNext();
		return CBotTypResult( 0 );

	case TokenTypVar:
		pClass = CBotClass::Find(p);
		if ( pClass != NULL)
		{
			p = p->GivNext();
			return ArrayType(p, pile, 
							 pClass->IsIntrinsic() ? 
							 CBotTypResult( CBotTypIntrinsic, pClass ) : 
							 CBotTypResult( CBotTypPointer,   pClass ) );
		}
	}
	return CBotTypResult( -1 );
}

// compile une nouvelle fonction
// bLocal permet de mettre la d�claration des param�tres au m�me niveau
// que le �l�ments appartenant � la classe pour les m�thodes
CBotFunction* CBotFunction::Compile(CBotToken* &p, CBotCStack* pStack, CBotFunction* finput, bool bLocal)
{
	CBotToken*		pp;
 	CBotFunction* func = finput;
	if ( func == NULL ) func = new CBotFunction();

	CBotCStack*	pStk = pStack->TokenStack(p, bLocal);

//	func->m_nFuncIdent = CBotVar::NextUniqNum();

	while (true)
	{
		if ( IsOfType(p, ID_PUBLIC) )
		{
			func->m_bPublic = true;
			continue;
		}
		pp = p;
		if ( IsOfType(p, ID_EXTERN) )
		{
			func->m_extern = pp;		// pour la position du mot "extern"
			func->m_bExtern = true;
//			func->m_bPublic = true;		// donc aussi publique!
			continue;
		}
		break;
	}

	func->m_retToken = *p;
//	CBotClass*	pClass;
	func->m_retTyp = TypeParam(p, pStk);		// type du r�sultat

	if (func->m_retTyp.GivType() >= 0)
	{
		CBotToken*	pp = p;
		func->m_token = *p;

		if ( IsOfType(p, ID_NOT) )
		{
			CBotToken d("~" + p->GivString());
			func->m_token = d;
		}

		// un nom de fonction est-il l� ?
		if (IsOfType(p, TokenTypVar))
		{
			if ( IsOfType( p, ID_DBLDOTS ) )		// m�thode pour une classe
			{
				func->m_MasterClass = pp->GivString();
				CBotClass* pClass = CBotClass::Find(pp);
				if ( pClass == NULL ) goto bad;

//				pp = p;
				func->m_token = *p;
				if (!IsOfType(p, TokenTypVar)) goto bad;

			}
			func->m_openpar = p;
			func->m_Param = CBotDefParam::Compile( p, pStk );
			func->m_closepar = p->GivPrev();
			if (pStk->IsOk())
			{
				pStk->SetRetType(func->m_retTyp);	// pour savoir de quel type les return

				if (!func->m_MasterClass.IsEmpty())
				{
					// rend "this" connu
					CBotVar* pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, func->m_MasterClass ));
					pThis->SetInit(2);
//					pThis->SetUniqNum(func->m_nThisIdent = -2); //CBotVar::NextUniqNum() va pas
					pThis->SetUniqNum(-2);
					pStk->AddVar(pThis);

					// initialise les variables selon This
					// n'enregistre que le pointeur � la premi�re, 
					// le reste est chain�
					CBotVar* pv = pThis->GivItemList();
//					int num = 1;
					while (pv != NULL)
					{
						CBotVar* pcopy = CBotVar::Create(pv);
//						pcopy->SetInit(2);
						pcopy->Copy(pv);
						pcopy->SetPrivate(pv->GivPrivate());
//						pcopy->SetUniqNum(pv->GivUniqNum()); //num++);
						pStk->AddVar(pcopy);
						pv = pv->GivNext();
					}
				}

				// et compile le bloc d'instruction qui suit
				func->m_openblk = p;
				func->m_Block	= CBotBlock::Compile(p, pStk, false);
				func->m_closeblk = p->GivPrev();
				if ( pStk->IsOk() )
				{
					if ( func->m_bPublic )	// fonction publique, la rend connue pour tous
					{
						CBotFunction::AddPublic(func);
					}
					return pStack->ReturnFunc(func, pStk);
				}
			}
		}
bad:
		pStk->SetError(TX_NOFONC, p);
	}
	pStk->SetError(TX_NOTYP, p);
	if ( finput == NULL ) delete func;
	return pStack->ReturnFunc(NULL, pStk);
}

// pr�-compile une nouvelle fonction
CBotFunction* CBotFunction::Compile1(CBotToken* &p, CBotCStack* pStack, CBotClass*	pClass)
{
 	CBotFunction* func = new CBotFunction();
	func->m_nFuncIdent = CBotVar::NextUniqNum();
	
	CBotCStack*	pStk = pStack->TokenStack(p, true);

	while (true)
	{
		if ( IsOfType(p, ID_PUBLIC) )
		{
		//	func->m_bPublic = true;		// sera fait en passe 2
			continue;
		}
		if ( IsOfType(p, ID_EXTERN) )
		{
			func->m_bExtern = true;
			continue;
		}
		break;
	}

	func->m_retToken = *p;
	func->m_retTyp = TypeParam(p, pStack);		// type du r�sultat

	if (func->m_retTyp.GivType() >= 0)
	{
		CBotToken*	pp = p;
		func->m_token = *p;
		// un nom de fonction est-il l� ?
		if (IsOfType(p, TokenTypVar))
		{
			if ( IsOfType( p, ID_DBLDOTS ) )		// m�thode pour une classe
			{
				func->m_MasterClass = pp->GivString();
				CBotClass* pClass = CBotClass::Find(pp);
				if ( pClass == NULL )
				{
					pStk->SetError(TX_NOCLASS, pp);
					goto bad;
				}

				pp = p;
				func->m_token = *p;
				if (!IsOfType(p, TokenTypVar)) goto bad;

			}
			func->m_Param = CBotDefParam::Compile( p, pStk );
			if (pStk->IsOk())
			{
				// regarde si la fonction existe ailleurs
				if (( pClass != NULL || !pStack->CheckCall(pp, func->m_Param)) &&
					( pClass == NULL || !pClass->CheckCall(pp, func->m_Param)) )
				{
					if (IsOfType(p, ID_OPBLK))
					{
						int level = 1;
						// et saute le bloc d'instructions qui suit
						do
						{
							int type = p->GivType();
							p = p->GivNext();
							if (type == ID_OPBLK) level++;
							if (type == ID_CLBLK) level--;
						}
						while (level > 0 && p != NULL);
					
						return pStack->ReturnFunc(func, pStk);
					}
					pStk->SetError(TX_OPENBLK, p);
				}
			}
			pStk->SetError(TX_REDEF, pp);
		}
bad:
		pStk->SetError(TX_NOFONC, p);
	}
	pStk->SetError(TX_NOTYP, p);
	delete func;
	return pStack->ReturnFunc(NULL, pStk);
}

#ifdef	_DEBUG
static int xx = 0;
#endif

bool CBotFunction::Execute(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
	CBotStack*	pile = pj->AddStack(this, 2);				// un bout de pile local � cette fonction
//	if ( pile == EOX ) return true;

	pile->SetBotCall(m_pProg);								// bases pour les routines

	if ( pile->GivState() == 0 )
	{
		if ( !m_Param->Execute(ppVars, pile) ) return false;	// d�fini les param�tres
		pile->IncState();
	}

	if ( pile->GivState() == 1 && !m_MasterClass.IsEmpty() )
	{
		// rend "this" connu
		CBotVar* pThis ;
		if ( pInstance == NULL )
		{
			pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, m_MasterClass ));
			pThis->SetInit(2);
		}
		else
		{
			pThis = CBotVar::Create("this", CBotTypResult( CBotTypPointer, m_MasterClass ));
			pThis->SetPointer(pInstance);
			pThis->SetInit(2);
		}

//		pThis->SetUniqNum(m_nThisIdent);
		pThis->SetUniqNum(-2);
		pile->AddVar(pThis);

		pile->IncState();
	}

	if ( pile->IfStep() ) return false;

	if ( !m_Block->Execute(pile) )
	{
		if ( pile->GivError() < 0 )
			pile->SetError( 0 );
		else
			return false;
	}

	return pj->Return(pile);
}


void CBotFunction::RestoreState(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance)
{
	CBotStack*	pile = pj->RestoreStack(this);			// un bout de pile local � cette fonction
	if ( pile == NULL ) return;
	CBotStack*	pile2 = pile;

	pile->SetBotCall(m_pProg);							// bases pour les routines

	if ( pile->GivBlock() < 2 )
	{
		CBotStack*	pile2 = pile->RestoreStack(NULL);		// un bout de pile local � cette fonction
		if ( pile2 == NULL ) return;
		pile->SetState(pile->GivState() + pile2->GivState());
		pile2->Delete();
	}

	m_Param->RestoreState(pile2, true);					// les param�tres

	if ( !m_MasterClass.IsEmpty() )
	{
		CBotVar* pThis = pile->FindVar("this");
		pThis->SetInit(2);
		pThis->SetUniqNum(-2);
	}

	m_Block->RestoreState(pile2, true);
}

void CBotFunction::AddNext(CBotFunction* p)
{
	CBotFunction*	pp = this;
	while (pp->m_next != NULL) pp = pp->m_next;

	pp->m_next = p;
}


CBotTypResult CBotFunction::CompileCall(const char* name, CBotVar** ppVars, long& nIdent)
{
	nIdent = 0;
	CBotTypResult	type;

	CBotFunction*	pt = FindLocalOrPublic(nIdent, name, ppVars, type);
	return type;
}


// trouve une fonction selon son identificateur unique
// si l'identificateur n'est pas trouv�, cherche selon le nom et les param�tres

CBotFunction* CBotFunction::FindLocalOrPublic(long& nIdent, const char* name, CBotVar** ppVars, CBotTypResult& TypeOrError, bool bPublic)
{
	TypeOrError.SetType(TX_UNDEFCALL);		// pas de routine de ce nom
	CBotFunction*	pt;

	if ( nIdent )
	{
		if ( this != NULL ) for ( pt = this ; pt != NULL ; pt = pt->m_next )
		{
			if ( pt->m_nFuncIdent == nIdent )
			{
				TypeOrError = pt->m_retTyp;
				return pt;
			}
		}

		// recherche dans la liste des fonctions publiques

		for ( pt = m_listPublic ; pt != NULL ; pt = pt->m_nextpublic )
		{
			if ( pt->m_nFuncIdent == nIdent )
			{
				TypeOrError = pt->m_retTyp;
				return pt;
			}
		}
	}

	if ( name == NULL ) return NULL;

	int		delta	= 99999;				// cherche la signature la plus faible
	CBotFunction*	pFunc = NULL;			// la meilleure fonction trouv�e

	if ( this != NULL )
	{
		for ( pt = this ; pt != NULL ; pt = pt->m_next )
		{
			if ( pt->m_token.GivString() == name )
			{
				int	i = 0;
				int	alpha = 0;							// signature des param�tres
				// les param�tres sont-ils compatibles ?
				CBotDefParam* pv = pt->m_Param;			// liste des param�tres attendus
				CBotVar* pw = ppVars[i++];				// liste des param�tres fournis
				while ( pv != NULL && pw != NULL)
				{
					if (!TypesCompatibles(pv->GivTypResult(), pw->GivTypResult()))
					{
						if ( pFunc == NULL ) TypeOrError = TX_BADPARAM;
						break;
					}
					int d = pv->GivType() - pw->GivType(2);
					alpha += d>0 ? d : -10*d;		// perte de qualit�, 10 fois plus cher !!

					pv = pv->GivNext();
					pw = ppVars[i++];
				}
				if ( pw != NULL ) 
				{
					if ( pFunc != NULL ) continue;
					if ( TypeOrError.Eq(TX_LOWPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
					if ( TypeOrError.Eq(TX_UNDEFCALL)) TypeOrError.SetType(TX_OVERPARAM);
					continue;					// trop de param�tres
				}
				if ( pv != NULL )
				{
					if ( pFunc != NULL ) continue;
					if ( TypeOrError.Eq(TX_OVERPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
					if ( TypeOrError.Eq(TX_UNDEFCALL) ) TypeOrError.SetType(TX_LOWPARAM);
					continue;					// pas assez de param�tres
				}

				if (alpha == 0)					// signature parfaite
				{
					nIdent = pt->m_nFuncIdent;
					TypeOrError = pt->m_retTyp;
					return pt;
				}

				if ( alpha < delta )			// une meilleur signature ?
				{
					pFunc = pt;
					delta = alpha;
				}
			}
		}
	}

	if ( bPublic )
	{
		for ( pt = m_listPublic ; pt != NULL ; pt = pt->m_nextpublic )
		{
			if ( pt->m_token.GivString() == name )
			{
				int	i = 0;
				int	alpha = 0;							// signature des param�tres
				// les param�tres sont-ils compatibles ?
				CBotDefParam* pv = pt->m_Param;			// liste des param�tres attendus
				CBotVar* pw = ppVars[i++];				// liste des param�tres fournis
				while ( pv != NULL && pw != NULL)
				{
					if (!TypesCompatibles(pv->GivTypResult(), pw->GivTypResult()))
					{
						if ( pFunc == NULL ) TypeOrError = TX_BADPARAM;
						break;
					}
					int d = pv->GivType() - pw->GivType(2);
					alpha += d>0 ? d : -10*d;		// perte de qualit�, 10 fois plus cher !!

					pv = pv->GivNext();
					pw = ppVars[i++];
				}
				if ( pw != NULL ) 
				{
					if ( pFunc != NULL ) continue;
					if ( TypeOrError.Eq(TX_LOWPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
					if ( TypeOrError.Eq(TX_UNDEFCALL)) TypeOrError.SetType(TX_OVERPARAM);
					continue;					// trop de param�tres
				}
				if ( pv != NULL )
				{
					if ( pFunc != NULL ) continue;
					if ( TypeOrError.Eq(TX_OVERPARAM) ) TypeOrError.SetType(TX_NUMPARAM);
					if ( TypeOrError.Eq(TX_UNDEFCALL) ) TypeOrError.SetType(TX_LOWPARAM);
					continue;					// pas assez de param�tres
				}

				if (alpha == 0)					// signature parfaite
				{
					nIdent = pt->m_nFuncIdent;
					TypeOrError = pt->m_retTyp;
					return pt;
				}

				if ( alpha < delta )			// une meilleur signature ?
				{
					pFunc = pt;
					delta = alpha;
				}
			}
		}
	}

	if ( pFunc != NULL )
	{
		nIdent = pFunc->m_nFuncIdent;
		TypeOrError = pFunc->m_retTyp;
		return pFunc;
	}
	return NULL;
}


// fait un appel � une fonction

int CBotFunction::DoCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken)
{
	CBotTypResult	type;
	CBotFunction*	pt = NULL;
	
	pt = FindLocalOrPublic(nIdent, name, ppVars, type);

	if ( pt != NULL )
	{
		CBotStack*	pStk1 = pStack->AddStack(pt, 2);	// pour mettre "this"
//		if ( pStk1 == EOX ) return true;

		pStk1->SetBotCall(pt->m_pProg);					// on a peut-�tre chang� de module

		if ( pStk1->IfStep() ) return false;

		CBotStack*	pStk3 = pStk1->AddStack(NULL, true);	// param�tres

		// pr�pare les param�tres sur la pile

		if ( pStk1->GivState() == 0 )
		{
			if ( !pt->m_MasterClass.IsEmpty() )
			{
				CBotVar* pInstance = m_pProg->m_pInstance;
				// rend "this" connu
				CBotVar* pThis ;
				if ( pInstance == NULL )
				{
					pThis = CBotVar::Create("this", CBotTypResult( CBotTypClass, pt->m_MasterClass ));
					pThis->SetInit(2);
				}
				else
				{
					pThis = CBotVar::Create("this", CBotTypResult( CBotTypPointer, pt->m_MasterClass ));
					pThis->SetPointer(pInstance);
					pThis->SetInit(2);
				}

				pThis->SetUniqNum(-2);
				pStk1->AddVar(pThis);

			}

			// initialise les variables selon param�tres
			pt->m_Param->Execute(ppVars, pStk3);			// ne peut pas �tre interrompu

			pStk1->IncState();
		}

		// finalement ex�cute la fonction trouv�e

		if ( !pStk3->GivRetVar(						// remet le r�sultat sur la pile
			pt->m_Block->Execute(pStk3) ))			// GivRetVar dit si c'est interrompu
		{
			if ( !pStk3->IsOk() && pt->m_pProg != m_pProg )
			{
#ifdef _DEBUG
				if ( m_pProg->GivFunctions()->GivName() == "LaCommande" ) return false;
#endif
				pStk3->SetPosError(pToken);			// indique l'erreur sur l'appel de proc�dure
			}
			return false;	// interrompu !
		}

		return pStack->Return( pStk3 );
	}
	return -1;
}

void CBotFunction::RestoreCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack)
{
	CBotTypResult	type;
	CBotFunction*	pt = NULL;
	CBotStack*		pStk1;
	CBotStack*		pStk3;

	// recherche la fonction pour remettre l'identificateur ok

	pt = FindLocalOrPublic(nIdent, name, ppVars, type);

	if ( pt != NULL )
	{
		pStk1 = pStack->RestoreStack(pt);
		if ( pStk1 == NULL ) return;

		pStk1->SetBotCall(pt->m_pProg);					// on a peut-�tre chang� de module

		if ( pStk1->GivBlock() < 2 )
		{
			CBotStack* pStk2 = pStk1->RestoreStack(NULL); // plus utilis�
			if ( pStk2 == NULL ) return;
			pStk3 = pStk2->RestoreStack(NULL);
			if ( pStk3 == NULL ) return;
		}
		else
		{
			pStk3 = pStk1->RestoreStack(NULL);
			if ( pStk3 == NULL ) return;
		}

		// pr�pare les param�tres sur la pile

		{
			if ( !pt->m_MasterClass.IsEmpty() )
			{
				CBotVar* pInstance = m_pProg->m_pInstance;
				// rend "this" connu
				CBotVar* pThis = pStk1->FindVar("this");
				pThis->SetInit(2);
				pThis->SetUniqNum(-2);
			}
		}

		if ( pStk1->GivState() == 0 )
		{
			pt->m_Param->RestoreState(pStk3, true);
			return;
		}

		// initialise les variables selon param�tres
		pt->m_Param->RestoreState(pStk3, false);
		pt->m_Block->RestoreState(pStk3, true);
	}
}



// fait un appel d'une m�thode
// note : this est d�j� sur la pile, le pointeur pThis est juste l� pour simplifier

int CBotFunction::DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass)
{
	CBotTypResult	type;
	CBotProgram*	pProgCurrent = pStack->GivBotCall();

	CBotFunction*	pt = FindLocalOrPublic(nIdent, name, ppVars, type, false);

	if ( pt != NULL )
	{
//		DEBUG( "CBotFunction::DoCall" + pt->GivName(), 0, pStack);

		CBotStack*	pStk = pStack->AddStack(pt, 2);
//		if ( pStk == EOX ) return true;

		pStk->SetBotCall(pt->m_pProg);					// on a peut-�tre chang� de module
		CBotStack*	pStk3 = pStk->AddStack(NULL, true); // pour mettre les param�tres pass�s

		// pr�pare les param�tres sur la pile

		if ( pStk->GivState() == 0 )
		{
			// met la variable "this" sur la pile
			CBotVar* pthis = CBotVar::Create("this", CBotTypNullPointer);
			pthis->Copy(pThis, false);
			pthis->SetUniqNum(-2);		// valeur sp�ciale
			pStk->AddVar(pthis);

			CBotClass*	pClass = pThis->GivClass()->GivParent();
			if ( pClass )
			{
				// met la variable "super" sur la pile
				CBotVar* psuper = CBotVar::Create("super", CBotTypNullPointer);
				psuper->Copy(pThis, false);	// en fait identique � "this"
				psuper->SetUniqNum(-3);		// valeur sp�ciale
				pStk->AddVar(psuper);
			}
			// initialise les variables selon param�tres
			pt->m_Param->Execute(ppVars, pStk3);			// ne peut pas �tre interrompu
			pStk->IncState();
		}

		if ( pStk->GivState() == 1 )
		{
			if ( pt->m_bSynchro )
			{
				CBotProgram* pProgBase = pStk->GivBotCall(true);
				if ( !pClass->Lock(pProgBase) ) return false;		// attend de pouvoir
			}
			pStk->IncState();
		}
		// finalement appelle la fonction trouv�e

		if ( !pStk3->GivRetVar(							// remet le r�sultat sur la pile
			pt->m_Block->Execute(pStk3) ))			// GivRetVar dit si c'est interrompu
		{
			if ( !pStk3->IsOk() )
			{
				if ( pt->m_bSynchro )
				{
					pClass->Unlock();					// lib�re la fonction
				}

				if ( pt->m_pProg != pProgCurrent )
				{
					pStk3->SetPosError(pToken);			// indique l'erreur sur l'appel de proc�dure
				}
			}
			return false;	// interrompu !
		}

		if ( pt->m_bSynchro )
		{
			pClass->Unlock();							// lib�re la fonction
		}

		return pStack->Return( pStk3 );
	}
	return -1;
}

void CBotFunction::RestoreCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass)
{
	CBotTypResult	type;
	CBotFunction*	pt = FindLocalOrPublic(nIdent, name, ppVars, type);

	if ( pt != NULL )
	{
		CBotStack*	pStk = pStack->RestoreStack(pt);
		if ( pStk == NULL ) return;
		pStk->SetBotCall(pt->m_pProg);					// on a peut-�tre chang� de module

		CBotVar*	pthis = pStk->FindVar("this");
		pthis->SetUniqNum(-2);

		CBotStack*	pStk3 = pStk->RestoreStack(NULL);	// pour mettre les param�tres pass�s
		if ( pStk3 == NULL ) return;

		pt->m_Param->RestoreState(pStk3, true);					// les param�tres

		if ( pStk->GivState() > 1 &&						// v�rouillage est effectif ?
			 pt->m_bSynchro )
			{
				CBotProgram* pProgBase = pStk->GivBotCall(true);
				pClass->Lock(pProgBase);					// v�rouille la classe
			}

		// finalement appelle la fonction trouv�e

		pt->m_Block->RestoreState(pStk3, true);					// interrompu !
	}
}

// regarde si la "signature" des param�tres est identique
bool CBotFunction::CheckParam(CBotDefParam* pParam)
{
	CBotDefParam*	pp = m_Param;
	while ( pp != NULL && pParam != NULL )
	{
		CBotTypResult type1 = pp->GivType();
		CBotTypResult type2 = pParam->GivType();
		if ( !type1.Compare(type2) ) return false;
		pp = pp->GivNext();
		pParam = pParam->GivNext();
	}
	return ( pp == NULL && pParam == NULL );
}

CBotString CBotFunction::GivName()
{
	return	m_token.GivString();
}

CBotString CBotFunction::GivParams()
{
	if ( m_Param == NULL ) return CBotString("()");

	CBotString		params = "( ";
	CBotDefParam*	p = m_Param;		// liste des param�tres

	while (p != NULL)
	{
		params += p->GivParamString();
		p = p->GivNext();
		if ( p != NULL ) params += ", ";
	}

	params += " )";
	return params;
}

CBotFunction* CBotFunction::Next()
{
	return	m_next;
}

void CBotFunction::AddPublic(CBotFunction* func)
{
	if ( m_listPublic != NULL )
	{
		func->m_nextpublic = m_listPublic;
		m_listPublic->m_prevpublic = func;
	}
	m_listPublic = func;
}



/////////////////////////////////////////////////////////////////////////
// gestion des param�tres


CBotDefParam::CBotDefParam()
{
	m_next	 = NULL;
	m_nIdent = 0;
}

CBotDefParam::~CBotDefParam()
{
	delete	m_next;
}


// compile une liste de param�tres
CBotDefParam* CBotDefParam::Compile(CBotToken* &p, CBotCStack* pStack)
{
	// surtout pas de pStack->TokenStack ici
	// les variables d�clar�es doivent rester visibles par la suite

	pStack->SetStartError(p->GivStart());

	if (IsOfType(p, ID_OPENPAR))
	{
		CBotDefParam* list = NULL;
	
		while (!IsOfType(p, ID_CLOSEPAR))
		{
		 	CBotDefParam* param = new CBotDefParam();
			if (list == NULL) list = param;
			else list->AddNext(param);			// ajoute � la liste

			CBotClass*	pClass = NULL;//= CBotClass::Find(p);
			param->m_typename = p->GivString();
			CBotTypResult type = param->m_type = TypeParam(p, pStack);
//			if ( type == CBotTypPointer ) type = CBotTypClass;			// il faut cr�er un nouvel objet

			if (param->m_type.GivType() > 0)
			{
				CBotToken*	pp = p;
				param->m_token = *p;
				if (pStack->IsOk() && IsOfType(p, TokenTypVar) )
				{

					// variable d�j� d�clar�e ?
					if (pStack->CheckVarLocal(pp))
					{
						pStack->SetError(TX_REDEFVAR, pp);
						break;
					}

					if ( type.Eq(CBotTypArrayPointer) ) type.SetType(CBotTypArrayBody);
					CBotVar*	var = CBotVar::Create(pp->GivString(), type);		// cr�e la variable 
//					if ( pClass ) var->SetClass(pClass);
					var->SetInit(2);									// la marque initialis�e
					param->m_nIdent = CBotVar::NextUniqNum();
					var->SetUniqNum(param->m_nIdent);
					pStack->AddVar(var);								// la place sur la pile

					if (IsOfType(p, ID_COMMA) || p->GivType() == ID_CLOSEPAR)
						continue;
				}
				pStack->SetError(TX_CLOSEPAR, p->GivStart());
			}
			pStack->SetError(TX_NOTYP, p);
			delete list;
			return NULL;
		}
		return list;
	}
	pStack->SetError(TX_OPENPAR, p->GivStart());
	return NULL;
}

void CBotDefParam::AddNext(CBotDefParam* p)
{
	CBotDefParam*	pp = this;
	while (pp->m_next != NULL) pp = pp->m_next;

	pp->m_next = p;
}


bool CBotDefParam::Execute(CBotVar** ppVars, CBotStack* &pj)
{
	int				i = 0;
	CBotDefParam*	p = this;

	while ( p != NULL )
	{
		// cr�e une variable locale sur la pile
		CBotVar*	newvar = CBotVar::Create(p->m_token.GivString(), p->m_type);

		// proc�de ainsi pour faire la transformation des types :
		if ( ppVars != NULL && ppVars[i] != NULL )
		{
			switch (p->m_type.GivType())
			{
			case CBotTypInt:
				newvar->SetValInt(ppVars[i]->GivValInt());
				break;
			case CBotTypFloat:
				newvar->SetValFloat(ppVars[i]->GivValFloat());
				break;
			case CBotTypString:
				newvar->SetValString(ppVars[i]->GivValString());
				break;
			case CBotTypBoolean:
				newvar->SetValInt(ppVars[i]->GivValInt());
				break;
			case CBotTypIntrinsic:
				((CBotVarClass*)newvar)->Copy(ppVars[i], false);
				break;
			case CBotTypPointer:
			case CBotTypArrayPointer:
				{
					newvar->SetPointer(ppVars[i]->GivPointer());
				}
				break;
			default:
				ASM_TRAP();
			}
		}
		newvar->SetUniqNum(p->m_nIdent);
		pj->AddVar(newvar);		// place la variable
		p = p->m_next;
		i++;
	}

	return true;
}

void CBotDefParam::RestoreState(CBotStack* &pj, bool bMain)
{
	int				i = 0;
	CBotDefParam*	p = this;

	while ( p != NULL )
	{
		// cr�e une variable locale sur la pile
		CBotVar*	var = pj->FindVar(p->m_token.GivString());
		var->SetUniqNum(p->m_nIdent);
		p = p->m_next;
	}
}

int CBotDefParam::GivType()
{
	return	m_type.GivType();
}

CBotTypResult CBotDefParam::GivTypResult()
{
	return	m_type;
}

CBotDefParam* CBotDefParam::GivNext()
{
	return	m_next;
}

CBotString CBotDefParam::GivParamString()
{
	CBotString	param;
	
	param = m_typename;
	param += ' ';

	param += m_token.GivString();
	return param;
}



//////////////////////////////////////////////////////////////////////////
// retour des param�tres 

CBotReturn::CBotReturn()
{
	m_Instr	= NULL;
	name = "CBotReturn";		// debug
}

CBotReturn::~CBotReturn()
{
	delete	m_Instr;
}

CBotInstr* CBotReturn::Compile(CBotToken* &p, CBotCStack* pStack)
{
	CBotToken*	pp = p;

	if (!IsOfType(p, ID_RETURN)) return NULL;	// ne devrait jamais arriver

	CBotReturn*	inst = new CBotReturn();		// cr�e l'objet
	inst->SetToken( pp );

	CBotTypResult	type = pStack->GivRetType();

	if ( type.GivType() == 0 )					// retourne void ?
	{
		if ( IsOfType( p, ID_SEP ) ) return inst;
		pStack->SetError( TX_BADTYPE, pp );
		return NULL;
	}

	inst->m_Instr = CBotExpression::Compile(p, pStack);
	if ( pStack->IsOk() )
	{
		CBotTypResult	retType = pStack->GivTypResult(2);
		if (TypeCompatible(retType, type, ID_ASS)) 
		{
			if ( IsOfType( p, ID_SEP ) )
				return inst;

			pStack->SetError(TX_ENDOF, p->GivStart());
		}
		pStack->SetError(TX_BADTYPE, p->GivStart());
	}

	delete inst;
	return NULL;							// pas d'objet, l'erreur est sur la pile
}

bool CBotReturn::Execute(CBotStack* &pj)
{
	CBotStack*	pile = pj->AddStack(this);
//	if ( pile == EOX ) return true;

	if ( pile->GivState() == 0 )
	{
		if ( m_Instr != NULL && !m_Instr->Execute(pile) ) return false;	// �value le r�sultat
		// le r�sultat est sur la pile 
		pile->IncState();
	}

	if ( pile->IfStep() ) return false;

	pile->SetBreak(3, CBotString());
	return pj->Return(pile);
}

void CBotReturn::RestoreState(CBotStack* &pj, bool bMain)
{
	if ( !bMain ) return;
	CBotStack*	pile = pj->RestoreStack(this);
	if ( pile == NULL ) return;

	if ( pile->GivState() == 0 )
	{
		if ( m_Instr != NULL ) m_Instr->RestoreState(pile, bMain);	// �value le r�sultat
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Les appels � ces fonctions

CBotInstrCall::CBotInstrCall()
{
	m_Parameters = NULL;
	m_nFuncIdent = 0;
	name = "CBotInstrCall";
}

CBotInstrCall::~CBotInstrCall()
{
	delete	m_Parameters;
}

CBotInstr* CBotInstrCall::Compile(CBotToken* &p, CBotCStack* pStack)
{
	CBotVar*	ppVars[1000];

	int			i = 0;

	CBotToken*	pp = p; 
	p = p->GivNext();

	pStack->SetStartError(p->GivStart());
	CBotCStack* pile = pStack;

	if ( IsOfType(p, ID_OPENPAR) )
	{
		int	start, end;
		CBotInstrCall* inst = new CBotInstrCall();
		inst->SetToken(pp);

		// compile la liste des param�tres
		if (!IsOfType(p, ID_CLOSEPAR)) while (true)
		{
			start = p->GivStart();
			pile = pile->TokenStack();						// garde les r�sultats sur la pile

			CBotInstr*	param = CBotExpression::Compile(p, pile);
			end	  = p->GivStart();
			if ( inst->m_Parameters == NULL ) inst->m_Parameters = param;
			else inst->m_Parameters->AddNext(param);			// construit la liste

			if ( !pile->IsOk() )
			{
				delete inst;
				return pStack->Return(NULL, pile);
			}

			if ( param != NULL )
			{
				if ( pile->GivTypResult().Eq(99) )
				{
					delete pStack->TokenStack();
					pStack->SetError(TX_VOID, p->GivStart());
					delete inst;
					return NULL;
				}
				ppVars[i] = pile->GivVar();
				ppVars[i]->GivToken()->SetPos(start, end);
				i++;

				if (IsOfType(p, ID_COMMA)) continue;			// saute la virgule
				if (IsOfType(p, ID_CLOSEPAR)) break;
			}

			pStack->SetError(TX_CLOSEPAR, p->GivStart());
			delete pStack->TokenStack();
			delete inst;
			return NULL;
		}
		ppVars[i] = NULL;

		// la routine est-elle connue ?
//		CBotClass*	pClass = NULL;
		inst->m_typRes = pStack->CompileCall(pp, ppVars, inst->m_nFuncIdent);
		if ( inst->m_typRes.GivType() >= 20 )
		{
//			if (pVar2!=NULL) pp = pVar2->RetToken();
			pStack->SetError( inst->m_typRes.GivType(), pp );
			delete pStack->TokenStack();
			delete inst;
			return NULL;
		}

		delete pStack->TokenStack();
		if ( inst->m_typRes.GivType() > 0 )
		{
			CBotVar* pRes = CBotVar::Create("", inst->m_typRes);
			pStack->SetVar(pRes);	// pour conna�tre le type du r�sultat
		}
		else pStack->SetVar(NULL);			// routine retourne void

		return inst;
	}
	p = pp;
	delete pStack->TokenStack();
	return NULL;
}

bool CBotInstrCall::Execute(CBotStack* &pj)
{
	CBotVar*	ppVars[1000];
	CBotStack*	pile  = pj->AddStack(this);
	if ( pile->StackOver() ) return pj->Return( pile );

	CBotStack*	pile1 = pile;

	int		i = 0;

	CBotInstr*	p = m_Parameters;
	// �value les param�tres
	// et place les valeurs sur la pile
	// pour pouvoir �tre interrompu n'importe quand
	if ( p != NULL) while ( true )
	{
		pile = pile->AddStack();						// de la place sur la pile pour les r�sultats
		if ( pile->GivState() == 0 )
		{
			if (!p->Execute(pile)) return false;		// interrompu ici ?
			pile->SetState(1);							// marque sp�ciale pour reconna�re les param�tres
		}
		ppVars[i++] = pile->GivVar();
		p = p->GivNext();
		if ( p == NULL) break;
	}
	ppVars[i] = NULL;

	CBotStack* pile2 = pile->AddStack();
	if ( pile2->IfStep() ) return false;

	if ( !pile2->ExecuteCall(m_nFuncIdent, GivToken(), ppVars, m_typRes)) return false;	// interrompu

	return pj->Return(pile2);	// lib�re toute la pile
}

void CBotInstrCall::RestoreState(CBotStack* &pj, bool bMain)
{
	if ( !bMain ) return;

	CBotStack*	pile  = pj->RestoreStack(this);
	if ( pile == NULL ) return;

	CBotStack*	pile1 = pile;

	int			i = 0;
	CBotVar*	ppVars[1000];
	CBotInstr*	p = m_Parameters;
	// �value les param�tres
	// et place les valeurs sur la pile
	// pour pouvoir �tre interrompu n'importe quand
	if ( p != NULL) while ( true )
	{
		pile = pile->RestoreStack();						// de la place sur la pile pour les r�sultats
		if ( pile == NULL ) return;
		if ( pile->GivState() == 0 )
		{
			p->RestoreState(pile, bMain);					// interrompu ici !
			return;
		}
		ppVars[i++] = pile->GivVar();				// construit la liste des param�tres
		p = p->GivNext();
		if ( p == NULL) break;
	}
	ppVars[i] = NULL;

	CBotStack* pile2 = pile->RestoreStack();
	if ( pile2 == NULL ) return;

	pile2->RestoreCall(m_nFuncIdent, GivToken(), ppVars);
}

//////////////////////////////////////////////////////////////////////////////
// d�claration des classes par l'utilisateur

// pr�-compile une nouvelle class
// l'analyse est compl�te � l'execption du corps des routines

CBotClass* CBotClass::Compile1(CBotToken* &p, CBotCStack* pStack)
{
	if ( !IsOfType(p, ID_PUBLIC) )
	{
		pStack->SetError(TX_NOPUBLIC, p);
		return NULL;
	}
	
	if ( !IsOfType(p, ID_CLASS) ) return NULL;

	CBotString name = p->GivString();

	CBotClass* pOld = CBotClass::Find(name);
	if ( pOld != NULL && pOld->m_IsDef )
	{
		pStack->SetError( TX_REDEFCLASS, p );
		return NULL;
	}

	// un nom pour la classe est-il l� ?
	if (IsOfType(p, TokenTypVar))
	{
		CBotClass* pPapa = NULL;
		if ( IsOfType( p, ID_EXTENDS ) )
		{
			CBotString name = p->GivString();
			pPapa = CBotClass::Find(name);

			if (!IsOfType(p, TokenTypVar) || pPapa == NULL )
			{
				pStack->SetError( TX_NOCLASS, p );
				return NULL;
			}
		}
		CBotClass* classe = (pOld == NULL) ? new CBotClass(name, pPapa) : pOld;
		classe->Purge();							// vide les anciennes d�finitions
		classe->m_IsDef = false;					// d�finition en cours

		if ( !IsOfType( p, ID_OPBLK) )
		{
			pStack->SetError(TX_OPENBLK, p);
			return NULL;
		}

		while ( pStack->IsOk() && !IsOfType( p, ID_CLBLK ) )
		{
			classe->CompileDefItem(p, pStack, false);
		}

		if (pStack->IsOk()) return classe;
	}
	pStack->SetError(TX_ENDOF, p);
	return NULL;
}

bool CBotClass::CompileDefItem(CBotToken* &p, CBotCStack* pStack, bool bSecond)
{
	bool	bStatic = false;
	int		mProtect = PR_PUBLIC;
	bool	bSynchro = false;

	while (IsOfType(p, ID_SEP)) ;

	CBotTypResult	type( -1 );

	if ( IsOfType(p, ID_SYNCHO) ) bSynchro = true;
	CBotToken*		pBase = p;

	if ( IsOfType(p, ID_STATIC) ) bStatic = true;
	if ( IsOfType(p, ID_PUBLIC) ) mProtect = PR_PUBLIC;
	if ( IsOfType(p, ID_PRIVATE) ) mProtect = PR_PRIVATE;
	if ( IsOfType(p, ID_PROTECTED) ) mProtect = PR_PROTECT;
	if ( IsOfType(p, ID_STATIC) ) bStatic = true;

//	CBotClass* pClass = NULL;
	type = TypeParam(p, pStack);		// type du r�sultat

	if ( type.Eq(-1) )
	{
		pStack->SetError(TX_NOTYP, p);
		return false;
	}

	while (pStack->IsOk()) 
	{
		CBotToken*	pp = p;
		IsOfType(p, ID_NOT);	// saute le ~ �ventuel (destructeur)

		if (IsOfType(p, TokenTypVar))
		{
			CBotInstr* limites = NULL;
			while ( IsOfType( p, ID_OPBRK ) )	// un tableau ?
			{
				CBotInstr* i = NULL;

				if ( p->GivType() != ID_CLBRK )
					i = CBotExpression::Compile( p, pStack );			// expression pour la valeur
				else
					i = new CBotEmpty();							// sp�cial si pas de formule

				type = CBotTypResult(CBotTypArrayPointer, type);

				if (!pStack->IsOk() || !IsOfType( p, ID_CLBRK ) )
				{
					pStack->SetError(TX_CLBRK, p->GivStart());
					return false;
				}

/*				CBotVar* pv = pStack->GivVar();
				if ( pv->GivType()>= CBotTypBoolean )
				{
					pStack->SetError(TX_BADTYPE, p->GivStart());
					return false;
				}*/

				if (limites == NULL) limites = i;
				else limites->AddNext3(i);
			}

			if ( p->GivType() == ID_OPENPAR )
			{
				if ( !bSecond )
				{
					p = pBase;
					CBotFunction* f = 
					CBotFunction::Compile1(p, pStack, this);

					if ( f == NULL ) return false;

					if (m_pMethod == NULL) m_pMethod = f;
					else m_pMethod->AddNext(f);
				}
				else
				{
					// retrouve la m�thode pr�compil�e en passe 1
					CBotFunction*	pf = m_pMethod;
					CBotFunction*	prev = NULL;
					while ( pf != NULL ) 
					{
						if (pf->GivName() == pp->GivString()) break;
						prev = pf;
						pf = pf->Next();
					}

					bool bConstructor = (pp->GivString() == GivName());
					CBotCStack* pile = pStack->TokenStack(NULL, true);

					// rend "this" connu
					CBotToken TokenThis(CBotString("this"), CBotString());
					CBotVar* pThis = CBotVar::Create(&TokenThis, CBotTypResult( CBotTypClass, this ) );
					pThis->SetUniqNum(-2);
					pile->AddVar(pThis);

					if ( m_pParent )
					{
						// rend "super" connu
						CBotToken TokenSuper(CBotString("super"), CBotString());
						CBotVar* pThis = CBotVar::Create(&TokenSuper, CBotTypResult( CBotTypClass, m_pParent ) );
						pThis->SetUniqNum(-3);
						pile->AddVar(pThis);
					}

//					int num = 1;
					CBotClass*	my = this;
					while (my != NULL)
					{
						// place une copie des varibles de la classe (this) sur la pile
						CBotVar* pv = my->m_pVar;
						while (pv != NULL)
						{
							CBotVar* pcopy = CBotVar::Create(pv);
							pcopy->SetInit(!bConstructor || pv->IsStatic());
							pcopy->SetUniqNum(pv->GivUniqNum());
							pile->AddVar(pcopy);
							pv = pv->GivNext();
						}
						my = my->m_pParent;
					}

					// compile une m�thode
					p = pBase;
					CBotFunction* f = 
					CBotFunction::Compile(p, pile, NULL/*, false*/);

					if ( f != NULL )
					{
						f->m_pProg = pStack->GivBotCall();
						f->m_bSynchro = bSynchro;
						// remplace l'�l�ment dans la cha�ne
						f->m_next = pf->m_next;
						pf->m_next = NULL;
						delete pf;
						if (prev == NULL) m_pMethod = f;
						else prev->m_next = f;
					}
					pStack->Return(NULL, pile);
				}

				return pStack->IsOk();
			}

			// d�finition d'un �l�ment
			if (type.Eq(0))
			{
				pStack->SetError(TX_ENDOF, p);
				return false;
			}

			CBotInstr* i = NULL;
			if ( IsOfType(p, ID_ASS ) )
			{
				if ( type.Eq(CBotTypArrayPointer) )
				{
					i = CBotListArray::Compile(p, pStack, type.GivTypElem());
				}
				else
				{
					// il y a une assignation � calculer
					i = CBotTwoOpExpr::Compile(p, pStack);
				}
				if ( !pStack->IsOk() ) return false;
			}


			if ( !bSecond )
			{
				CBotVar*	pv = CBotVar::Create(pp->GivString(), type);
				pv -> SetStatic( bStatic );
				pv -> SetPrivate( mProtect );

				AddItem( pv );

				pv->m_InitExpr = i;
				pv->m_LimExpr = limites;


				if ( pv->IsStatic() && pv->m_InitExpr != NULL )
				{
					CBotStack* pile = CBotStack::FirstStack();				// une pile ind�pendante
					while(pile->IsOk() && !pv->m_InitExpr->Execute(pile));	// �value l'expression sans timer
					pv->SetVal( pile->GivVar() ) ;
					pile->Delete();
				}
			}
			else
				delete i;

			if ( IsOfType(p, ID_COMMA) ) continue;
			if ( IsOfType(p, ID_SEP) ) break;
		}
		pStack->SetError(TX_ENDOF, p);
	}
	return pStack->IsOk();
}


CBotClass* CBotClass::Compile(CBotToken* &p, CBotCStack* pStack)
{
	if ( !IsOfType(p, ID_PUBLIC) ) return NULL;
	if ( !IsOfType(p, ID_CLASS) ) return NULL;

	CBotString name = p->GivString();

	// un nom pour la classe est-il l� ?
	if (IsOfType(p, TokenTypVar))
	{
		// la classe � �t� cr��e par Compile1
		CBotClass* pOld = CBotClass::Find(name);

		if ( IsOfType( p, ID_EXTENDS ) )
		{
			IsOfType(p, TokenTypVar); // forc�ment
		}
		IsOfType( p, ID_OPBLK);	// forc�ment

		while ( pStack->IsOk() && !IsOfType( p, ID_CLBLK ) )
		{
			pOld->CompileDefItem(p, pStack, true);
		}

		pOld->m_IsDef = true;			// d�finition termin�e
		if (pStack->IsOk())	return pOld;
	}
	pStack->SetError(TX_ENDOF, p);
	return NULL;
}
