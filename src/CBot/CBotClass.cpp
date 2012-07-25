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
// Gestion des variables de type classe
//

#include "CBot.h"


CBotClass* CBotClass::m_ExClass = NULL;

CBotClass::CBotClass(const char* name, CBotClass* pPapa, bool bIntrinsic)
{
	m_pParent	= pPapa;
	m_name		= name;
	m_pVar		= NULL;
	m_next		= NULL;
	m_pCalls	= NULL;
	m_pMethod	= NULL;
	m_rMaj		= NULL;
	m_IsDef		= true;
	m_bIntrinsic= bIntrinsic;
	m_cptLock	= 0;
	m_cptOne	= 0;
	m_nbVar		= m_pParent == NULL ? 0 : m_pParent->m_nbVar;

	for ( int j= 0; j< 5 ; j++ )
	{
		m_ProgInLock[j] = NULL;
	}


	// se place tout seul dans la liste
	if (m_ExClass) m_ExClass->m_ExPrev = this;
	m_ExNext  = m_ExClass;
	m_ExPrev  = NULL;
	m_ExClass = this;

}

CBotClass::~CBotClass()
{
	// retire la classe de la liste
	if ( m_ExPrev ) m_ExPrev->m_ExNext = m_ExNext;
	else m_ExClass = m_ExNext;

	if ( m_ExNext ) m_ExNext->m_ExPrev = m_ExPrev;
	m_ExPrev = NULL;
	m_ExNext = NULL;

	delete	m_pVar;
	delete	m_pCalls;
	delete	m_pMethod;

	delete	m_next;			// lib�re toutes celle de ce niveau
}


void CBotClass::Free()
{
	while ( m_ExClass != NULL )
	{
		delete m_ExClass;
	}
}

void CBotClass::Purge()
{
	if ( this == NULL ) return;

	delete		m_pVar;
	m_pVar		= NULL;
	delete		m_pCalls;
	m_pCalls	= NULL;
	delete		m_pMethod;
	m_pMethod	= NULL;
	m_IsDef		= false;

	m_nbVar		= m_pParent == NULL ? 0 : m_pParent->m_nbVar;

	m_next->Purge();
	m_next = NULL;			// n'appartient plus � cette cha�ne
}

bool CBotClass::Lock(CBotProgram* p)
{
	int	i = m_cptLock++;

	if ( i == 0 )
	{
		m_cptOne = 1;
		m_ProgInLock[0] = p;
		return true;
	}
	if ( p == m_ProgInLock[0] ) 
	{
		m_cptOne++;
		m_cptLock--;								// a d�j� �t� compt�
		return true;
	}

	for ( int j = 1 ; j <= i ; j++)
	{
		if ( p == m_ProgInLock[j] )
		{
			m_cptLock--;
			return false;	// d�j� en attente
		}
	}

	if ( i < 5 )	// maxi 5 en attente
	{
		m_ProgInLock[i] = p;						// se place dans la queue
	}
	else
		m_cptLock--;

	return false;
}

void CBotClass::Unlock()
{
	if ( --m_cptOne > 0 ) return ;

	int i = --m_cptLock;
	if ( i<0 )
	{
		m_cptLock = 0;
		return;
	}

	for ( int j= 0; j< i ; j++ )
	{
		m_ProgInLock[j] = m_ProgInLock[j+1];
	}
	m_ProgInLock[i] = 0;
}

void CBotClass::FreeLock(CBotProgram* p)
{
	CBotClass* pClass = m_ExClass;

	while ( pClass != NULL )
	{
		if ( p == pClass->m_ProgInLock[0] ) 
		{
			pClass->m_cptLock -= pClass->m_cptOne;
			pClass->m_cptOne = 0;
		}

		for ( int j = 1; j < 5 ; j++ )
			if ( p == pClass->m_ProgInLock[j] ) 
				pClass->m_cptLock--;

		pClass = pClass->m_ExNext;
	}
}



bool CBotClass::AddItem(CBotString name, CBotTypResult type, int mPrivate)
{
	CBotToken	token(name, CBotString());
	CBotClass*	pClass = type.GivClass();

	CBotVar*	pVar = CBotVar::Create( name, type );
///	pVar->SetUniqNum(CBotVar::NextUniqNum());
	pVar->SetPrivate( mPrivate );

	if ( pClass != NULL )
	{
//		pVar->SetClass(pClass);
		if ( type.Eq(CBotTypClass) )
		{
			// ajoute une instruction new pour initialiser l'object
			pVar->m_InitExpr = new CBotNew() ;
			CBotToken nom( pClass->GivName() );
			pVar->m_InitExpr->SetToken(&nom);
		}
	}
	return AddItem( pVar );
}


bool CBotClass::AddItem(CBotVar* pVar)
{
	pVar->SetUniqNum(++m_nbVar);

	if ( m_pVar == NULL ) m_pVar = pVar;
	else m_pVar->AddNext(pVar);

	return true;
}

void CBotClass::AddNext(CBotClass* pClass)
{
	CBotClass*		p = this;
	while (p->m_next != NULL) p = p->m_next;

	p->m_next = pClass;
}

CBotString  CBotClass::GivName()
{
	return m_name;
}

CBotClass*  CBotClass::GivParent()
{
	if ( this == NULL ) return NULL;
	return m_pParent;
}

bool  CBotClass::IsChildOf(CBotClass* pClass)
{
	CBotClass* p = this;
	while ( p != NULL )
	{
		if ( p == pClass ) return true;
		p = p->m_pParent;
	}
	return false;
}


CBotVar* CBotClass::GivVar()
{
	return	m_pVar;
}

CBotVar* CBotClass::GivItem(const char* name)
{
	CBotVar*	p = m_pVar;

	while ( p != NULL )
	{
		if ( p->GivName() == name ) return p;
		p = p->GivNext();
	}
	if ( m_pParent != NULL ) return m_pParent->GivItem(name);
	return NULL;
}

CBotVar* CBotClass::GivItemRef(int nIdent)
{
	CBotVar*	p = m_pVar;

	while ( p != NULL )
	{
		if ( p->GivUniqNum() == nIdent ) return p;
		p = p->GivNext();
	}
	if ( m_pParent != NULL ) return m_pParent->GivItemRef(nIdent);
	return NULL;
}

bool CBotClass::IsIntrinsic()
{
	return	m_bIntrinsic;
}

CBotClass* CBotClass::Find(CBotToken* &pToken)
{
	return Find(pToken->GivString());
}

CBotClass* CBotClass::Find(const char* name)
{
	CBotClass*	p = m_ExClass;

	while ( p != NULL )
	{
		if ( p->GivName() == name ) return p;
		p = p->m_ExNext;
	}

	return NULL;
}

bool CBotClass::AddFunction(const char* name, 
								bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception), 
								CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar))
{
	// m�morise les pointeurs aux deux fonctions
	CBotCallMethode*	p = m_pCalls;
	CBotCallMethode*	pp = NULL;

	while ( p != NULL )
	{
		if ( name == p->GivName() )
		{
			if ( pp == NULL ) m_pCalls = p->m_next;
			else			  pp->m_next = p->m_next;
			delete p;
			break;
		}
		pp = p;
		p = p->m_next;
	}

	p = new CBotCallMethode(name, rExec, rCompile);
	
	if (m_pCalls == NULL) m_pCalls = p;
	else	m_pCalls->AddNext(p);				// ajoute � la liste

	return true;
}

bool CBotClass::AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) )
{
	m_rMaj = rMaj;
	return true;
}

// compile une m�thode associ�e � une instance de classe
// la m�thode peut �tre d�clar�e par AddFunction ou par l'utilisateur

CBotTypResult CBotClass::CompileMethode(const char* name, 
										CBotVar* pThis, CBotVar** ppParams, 
										CBotCStack* pStack, long& nIdent)
{
	nIdent = 0;	// oublie le pr�c�dent s'il y a lieu

	// recherche dans les m�thodes d�clar�es par AddFunction

	CBotTypResult r = m_pCalls->CompileCall(name, pThis, ppParams, pStack, nIdent);
	if ( r.GivType() >= 0) return r;

	// recherche dans les m�thodes d�clar�es par l'utilisateur

	r = m_pMethod->CompileCall(name, ppParams, nIdent);
	if ( r.Eq(TX_UNDEFCALL) && m_pParent != NULL )
		return m_pParent->m_pMethod->CompileCall(name, ppParams, nIdent);
	return r;
}

// ex�cute une m�thode

bool CBotClass::ExecuteMethode(long& nIdent, const char* name, 
							   CBotVar* pThis, CBotVar** ppParams, 
							   CBotVar* &pResult, CBotStack* &pStack,
							   CBotToken* pToken)
{
	int ret = m_pCalls->DoCall(nIdent, name, pThis, ppParams, pResult, pStack, pToken);
	if (ret>=0) return ret;

	ret = m_pMethod->DoCall(nIdent, name, pThis, ppParams, pStack, pToken, this);
	return ret;
}

// r�tabli la pile d'ex�cution

void CBotClass::RestoreMethode(long& nIdent, const char* name, CBotVar* pThis,
							   CBotVar** ppParams, CBotStack* &pStack)
{
	m_pMethod->RestoreCall(nIdent, name, pThis, ppParams, pStack, this);
}




bool CBotClass::SaveStaticState(FILE* pf)
{
	if (!WriteWord( pf, CBOTVERSION*2)) return false;

	// sauve l'�tat des variables statiques dans les classes
	CBotClass*	p = m_ExClass;

	while ( p != NULL )
	{
		if (!WriteWord( pf, 1)) return false;
		// enregistre le nom de la classe
		if (!WriteString( pf, p->GivName() )) return false;

		CBotVar*	pv = p->GivVar();
		while( pv != NULL )
		{
			if ( pv->IsStatic() )
			{
				if (!WriteWord( pf, 1)) return false;
				if (!WriteString( pf, pv->GivName() )) return false;

				if ( !pv->Save0State(pf)) return false;				// ent�te commune
				if ( !pv->Save1State(pf) ) return false;				// sauve selon la classe fille
				if ( !WriteWord( pf, 0)) return false;
			}
			pv = pv->GivNext();
		}

		if (!WriteWord( pf, 0)) return false;
		p = p->m_ExNext;
	}

	if (!WriteWord( pf, 0)) return false;
	return true;
}

bool CBotClass::RestoreStaticState(FILE* pf)
{
	CBotString		ClassName, VarName;
	CBotClass*		pClass;
	unsigned short	w;

	if (!ReadWord( pf, w )) return false;
	if ( w != CBOTVERSION*2 ) return false;

	while (true)
	{
		if (!ReadWord( pf, w )) return false;
		if ( w == 0 ) return true;

		if (!ReadString( pf, ClassName )) return false;
		pClass = Find(ClassName);

		while (true)
		{
			if (!ReadWord( pf, w )) return false;
			if ( w == 0 ) break;

			CBotVar*	pVar = NULL;
			CBotVar*	pv = NULL;

			if (!ReadString( pf, VarName )) return false;
			if ( pClass != NULL ) pVar = pClass->GivItem(VarName);

			if (!CBotVar::RestoreState(pf, pv)) return false;	// la variable temp

			if ( pVar != NULL ) pVar->Copy(pv);
			delete pv;
		}
	}
	return true;
}


/////////////////////////////////////////////////////////////////////

CBotClassInst::CBotClassInst()
{
	m_next			= NULL;
	m_var			= NULL;
	m_Parameters	= NULL;
	m_expr			= NULL;
	m_hasParams		= false;
	m_nMethodeIdent = 0;
	name = "CBotClassInst";
}

CBotClassInst::~CBotClassInst()
{
	delete m_var;
//	delete m_next;			// fait par le destructeur de la classe de base ~CBotInstr()
}

// d�finition de pointeur(s) � un objet
// du style
// CPoint A, B ;

CBotInstr* CBotClassInst::Compile(CBotToken* &p, CBotCStack* pStack, CBotClass* pClass)
{
	// cherche la classe correspondante
	if ( pClass == NULL )
	{
		pStack->SetStartError(p->GivStart());
		pClass = CBotClass::Find(p);
		if ( pClass == NULL )
		{
			// pas trouv� ? c'est bizare
			pStack->SetError(TX_NOCLASS, p);
			return NULL;
		}
		p = p->GivNext();
	}

	bool		bIntrinsic = pClass->IsIntrinsic();
	CBotTypResult type = CBotTypResult( bIntrinsic ? CBotTypIntrinsic : CBotTypPointer, pClass );
	CBotClassInst*	inst = (CBotClassInst*)CompileArray(p, pStack, type);
	if ( inst != NULL || !pStack->IsOk() ) return inst;

	CBotCStack* pStk = pStack->TokenStack();

	inst = new CBotClassInst();
    /// \TODO Need to be revised and fixed after adding unit tests
    CBotToken token(pClass->GivName(), CBotString(), p->GivStart(), p->GivEnd());
	inst->SetToken(&token);
	CBotToken*	vartoken = p;

	if ( NULL != (inst->m_var = CBotLeftExprVar::Compile( p, pStk )) )
	{
		((CBotLeftExprVar*)inst->m_var)->m_typevar = type;
		if (pStk->CheckVarLocal(vartoken))					// red�finition de la variable
		{
			pStk->SetStartError(vartoken->GivStart());
			pStk->SetError(TX_REDEFVAR, vartoken->GivEnd());
			goto error;
		}

		if (IsOfType(p,  ID_OPBRK))							// avec des indices ?
		{
			delete inst;									// n'est pas de type CBotInt
			p = vartoken;									// revient sur le nom de la variable

			// compile une d�claration de tableau

			inst = (CBotClassInst*)CBotInstArray::Compile( p, pStk, type );

			if (!pStk->IsOk() )
			{
				pStk->SetError(TX_CLBRK, p->GivStart());
				goto error;
			}
			goto suite;			// pas d'assignation, variable d�j� cr��e
		}


		CBotVar*	var;
		var = CBotVar::Create(vartoken->GivString(), type);	// cr�e l'instance
//		var->SetClass(pClass);
		var->SetUniqNum(
			((CBotLeftExprVar*)inst->m_var)->m_nIdent = CBotVar::NextUniqNum());
															// lui attribut un num�ro unique
		pStack->AddVar(var);								// la place sur la pile

		// regarde s'il y a des param�tres
		inst->m_hasParams = (p->GivType() == ID_OPENPAR);

		CBotVar*	ppVars[1000];
		inst->m_Parameters = CompileParams(p, pStk, ppVars);
		if ( !pStk->IsOk() ) goto error;

		// s'il y a des param�tres, fait l'�quivalent de l'instruction new
		// CPoint A ( 0, 0 ) est �quivalent �
		// CPoint A = new CPoint( 0, 0 )

//		if ( NULL != inst->m_Parameters )
		if ( inst->m_hasParams )
		{
			// le constructeur existe-il ?
//			CBotString	noname;
			CBotTypResult r = pClass->CompileMethode(pClass->GivName(), var, ppVars, pStk, inst->m_nMethodeIdent);
			delete pStk->TokenStack();							// lib�re le suppl�ment de pile
			int typ = r.GivType();

			if (typ == TX_UNDEFCALL)
			{
				// si le constructeur n'existe pas
				if (inst->m_Parameters != NULL)					// avec des param�tres
				{
					pStk->SetError(TX_NOCONST, vartoken);
					goto error;
				}
				typ = 0;
			}

			if (typ>20)
			{
				pStk->SetError(typ, vartoken->GivEnd());
				goto error;
			}

		}

		if (IsOfType(p,  ID_ASS))							// avec une assignation ?
		{
			if (inst->m_hasParams)
			{
				pStk->SetError(TX_ENDOF, p->GivStart());
				goto error;
			}

			if ( NULL == ( inst->m_expr = CBotTwoOpExpr::Compile( p, pStk )) )
			{
				goto error;
			}
			CBotClass* result = pStk->GivClass();
			if ( !pStk->GivTypResult(1).Eq(CBotTypNullPointer) &&
			   ( !pStk->GivTypResult(1).Eq(CBotTypPointer) ||
				 ( result != NULL && !pClass->IsChildOf(result) )))		// type compatible ?
			{
				pStk->SetError(TX_BADTYPE, p->GivStart());
				goto error;
			}
//			if ( !bIntrinsic ) var->SetPointer(pStk->GivVar()->GivPointer());
			if ( !bIntrinsic ) 
			{
				// n'utilise pas le r�sultat sur la pile, pour imposer la classe
				CBotVar* pvar = CBotVar::Create("", pClass);
				var->SetPointer( pvar );					// var d�j� d�clar�e pointe l'instance
				delete pvar;								// supprime le second pointeur
			}
			var->SetInit(true);							// marque le pointeur comme init
		}
		else if (inst->m_hasParams)
		{
			// cr�e l'objet sur le "tas"
			// avec un pointeur sur cet objet
			if ( !bIntrinsic ) 
			{
				CBotVar* pvar = CBotVar::Create("", pClass);
				var->SetPointer( pvar );					// var d�j� d�clar�e pointe l'instance
				delete pvar;								// supprime le second pointeur
			}
			var->SetInit(2);							// marque le pointeur comme init
		}
suite:
		if (IsOfType(p,  ID_COMMA))							// plusieurs d�finitions encha�n�es
		{
			if ( NULL != ( inst->m_next = CBotClassInst::Compile(p, pStk, pClass) ))	// compile la suivante
			{
				return pStack->Return(inst, pStk);
			}
		}
		
		if (IsOfType(p,  ID_SEP))							// instruction termin�e
		{
			return pStack->Return(inst, pStk);
		}

		pStk->SetError(TX_ENDOF, p->GivStart());
	}

error:
	delete inst;
	return pStack->Return(NULL, pStk);
}

// d�claration de l'instance d'une classe, par exemple:
//	CPoint A, B;

bool CBotClassInst::Execute(CBotStack* &pj)
{
	CBotVar*	pThis = NULL;

 	CBotStack*	pile = pj->AddStack(this);//indispensable pour SetState()
//	if ( pile == EOX ) return true;

	CBotToken*	pt = &m_token;
	CBotClass*	pClass = CBotClass::Find(pt);

	bool bIntrincic = pClass->IsIntrinsic();

	// cr�e la variable de type pointeur � l'objet

	if ( pile->GivState()==0)
	{
		CBotString	name = m_var->m_token.GivString();
		if ( bIntrincic )
		{
			pThis = CBotVar::Create(name, CBotTypResult( CBotTypIntrinsic, pClass ));
		}
		else
		{
			pThis = CBotVar::Create(name, CBotTypResult( CBotTypPointer, pClass ));
		}

		pThis->SetUniqNum(((CBotLeftExprVar*)m_var)->m_nIdent);	// lui attribut un num�ro unique
		pile->AddVar(pThis);									// la place sur la pile
		pile->IncState();
	}

	if ( pThis == NULL ) pThis = pile->FindVar(((CBotLeftExprVar*)m_var)->m_nIdent);

	if ( pile->GivState()<3)
	{
		// y a-t-il une assignation ou des param�tres (constructeur)

//		CBotVarClass* pInstance = NULL;

		if ( m_expr != NULL )
		{
			// �value l'expression pour l'assignation
			if (!m_expr->Execute(pile)) return false;
	
			if ( bIntrincic )
			{
				CBotVar*	pv = pile->GivVar();
				if ( pv == NULL || pv->GivPointer() == NULL )
				{
					pile->SetError(TX_NULLPT, &m_token);
					return pj->Return(pile);
				}
				pThis->Copy(pile->GivVar(), false);
			}
			else
			{
				CBotVarClass* pInstance;
				pInstance = ((CBotVarPointer*)pile->GivVar())->GivPointer();	// valeur pour l'assignation
				pThis->SetPointer(pInstance);
			}
			pThis->SetInit(true);
		}

		else if ( m_hasParams )
		{
			// �value le constructeur d'une instance

			if ( !bIntrincic && pile->GivState() == 1)
			{
				CBotToken*	pt = &m_token;
				CBotClass* pClass = CBotClass::Find(pt);

				// cr�e une instance de la classe demand�e

				CBotVarClass* pInstance;
				pInstance = (CBotVarClass*)CBotVar::Create("", pClass);
				pThis->SetPointer(pInstance);
				delete pInstance;

				pile->IncState();
			}

			CBotVar*	ppVars[1000];
			CBotStack*	pile2 = pile;

			int		i = 0;

			CBotInstr*	p = m_Parameters;
			// �value les param�tres
			// et place les valeurs sur la pile
			// pour pouvoir �tre interrompu n'importe quand

			if ( p != NULL) while ( true )
			{
				pile2 = pile2->AddStack();						// de la place sur la pile pour les r�sultats
				if ( pile2->GivState() == 0 )
				{
					if (!p->Execute(pile2)) return false;		// interrompu ici ?
					pile2->SetState(1);
				}
				ppVars[i++] = pile2->GivVar();
				p = p->GivNext();
				if ( p == NULL) break;
			}
			ppVars[i] = NULL;

			// cr�e une variable pour le r�sultat
			CBotVar*	pResult = NULL;		// constructeurs toujours void

			if ( !pClass->ExecuteMethode(m_nMethodeIdent, pClass->GivName(), 
										 pThis, ppVars, 
										 pResult, pile2, GivToken())) return false;	// interrompu

			pThis->SetInit(true);
			pThis->ConstructorSet();		// signale que le constructeur a �t� appel�
			pile->Return(pile2);								// lib�re un bout de pile

//			pInstance = pThis->GivPointer();

		}

//		if ( !bIntrincic ) pThis->SetPointer(pInstance);		// le fait pointer l'instance

		pile->SetState(3);									// fini cette partie
	}

	if ( pile->IfStep() ) return false;

	if ( m_next2b != NULL &&
		!m_next2b->Execute(pile)) return false;				// autre(s) d�finition(s)

	return pj->Return( pile );								// transmet en dessous
}



void CBotClassInst::RestoreState(CBotStack* &pj, bool bMain)
{
	CBotVar*	pThis = NULL;

	CBotStack*	pile = pj;
	if ( bMain ) pile = pj->RestoreStack(this);
	if ( pile == NULL ) return;

	// cr�e la variable de type pointeur � l'objet
	{
		CBotString	name = m_var->m_token.GivString();
		pThis = pile->FindVar(name);
		pThis->SetUniqNum(((CBotLeftExprVar*)m_var)->m_nIdent);	// lui attribut un num�ro unique
	}

	CBotToken*	pt = &m_token;
	CBotClass*	pClass = CBotClass::Find(pt);
	bool bIntrincic = pClass->IsIntrinsic();

	if ( bMain && pile->GivState()<3)
	{
		// y a-t-il une assignation ou des param�tres (constructeur)

//		CBotVarClass* pInstance = NULL;

		if ( m_expr != NULL )
		{
			// �value l'expression pour l'assignation
			m_expr->RestoreState(pile, bMain);
			return;
		}

		else if ( m_hasParams )
		{
			// �value le constructeur d'une instance

			if ( !bIntrincic && pile->GivState() == 1)
			{
				return;
			}

			CBotVar*	ppVars[1000];
			CBotStack*	pile2 = pile;

			int		i = 0;

			CBotInstr*	p = m_Parameters;
			// �value les param�tres
			// et place les valeurs sur la pile
			// pour pouvoir �tre interrompu n'importe quand

			if ( p != NULL) while ( true )
			{
				pile2 = pile2->RestoreStack();						// de la place sur la pile pour les r�sultats
				if ( pile2 == NULL ) return;

				if ( pile2->GivState() == 0 )
				{
					p->RestoreState(pile2, bMain);		// interrompu ici ?
					return;
				}
				ppVars[i++] = pile2->GivVar();
				p = p->GivNext();
				if ( p == NULL) break;
			}
			ppVars[i] = NULL;

			// cr�e une variable pour le r�sultat
			CBotVar*	pResult = NULL;		// constructeurs toujours void

			pClass->RestoreMethode(m_nMethodeIdent, pClass->GivName(), pThis, ppVars, pile2);
			return;
		}
	}

	if ( m_next2b != NULL )
		 m_next2b->RestoreState(pile, bMain);					// autre(s) d�finition(s)
}


// test si un nom de proc�dure est d�j� d�fini quelque part

bool CBotClass::CheckCall(CBotToken* &pToken, CBotDefParam* pParam)
{
	CBotString	name = pToken->GivString();

	if ( CBotCall::CheckCall(name) ) return true;

	CBotFunction*	pp = m_pMethod;
	while ( pp != NULL )
	{
		if ( pToken->GivString() == pp->GivName() )
		{
			// les param�tres sont-ils exactement les m�mes ?
			if ( pp->CheckParam( pParam ) )
				return true;
		}
		pp = pp->Next();
	}

	return false;
}

