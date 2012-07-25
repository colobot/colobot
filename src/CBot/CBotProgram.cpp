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
// * along with this program. If not, see  http://www.gnu.org/licenses/.//////////////////////////////////////////////////////////////////////
// gestion de base d'un programme CBot

#include "CBot.h"
#include <stdio.h>

CBotProgram::CBotProgram()
{
	m_Prog		= NULL;
	m_pRun		= NULL;
	m_pClass	= NULL;
	m_pStack	= NULL;
	m_pInstance = NULL;

	m_ErrorCode = 0;
	m_Ident		= 0;
	m_bDebugDD	= 0;
}

CBotProgram::CBotProgram(CBotVar* pInstance)
{
	m_Prog		= NULL;
	m_pRun		= NULL;
	m_pClass	= NULL;
	m_pStack	= NULL;
	m_pInstance = pInstance;

	m_ErrorCode = 0;
	m_Ident		= 0;
	m_bDebugDD	= 0;
}


CBotProgram::~CBotProgram()
{
//	delete	m_pClass;
	m_pClass->Purge();
	m_pClass	= NULL;

	CBotClass::FreeLock(this);

	delete	m_Prog;
#if	STACKMEM
	m_pStack->Delete();
#else
	delete	m_pStack;
#endif
}


bool CBotProgram::Compile( const char* program, CBotStringArray& ListFonctions, void* pUser )
{
	int			error = 0;
	Stop();

//	delete		m_pClass;
	m_pClass->Purge();		// purge les anciennes d�finitions des classes
							// mais sans d�truire l'object
	m_pClass	= NULL;
	delete		m_Prog;		m_Prog= NULL;

	ListFonctions.SetSize(0);
	m_ErrorCode = 0;

	if (m_pInstance != NULL && m_pInstance->m_pUserPtr != NULL)
		pUser = m_pInstance->m_pUserPtr;

	// transforme le programme en Tokens
	CBotToken*	pBaseToken = CBotToken::CompileTokens(program, error);
	if ( pBaseToken == NULL ) return false;


	CBotCStack*	pStack		= new CBotCStack(NULL);
	CBotToken*	p  = pBaseToken->GivNext();					// saute le 1er token (s�parateur)

	pStack->SetBotCall(this);								// d�fini les routines utilisables
	CBotCall::SetPUser(pUser);

	// fait une premi�re passe rapide juste pour prendre les ent�tes de routines et de classes
	while ( pStack->IsOk() && p != NULL && p->GivType() != 0)
	{
		if ( IsOfType(p, ID_SEP) ) continue;				// des point-virgules qui trainent

		if ( p->GivType() == ID_CLASS || 
			( p->GivType() == ID_PUBLIC && p->GivNext()->GivType() == ID_CLASS ))
		{
			CBotClass*	nxt = CBotClass::Compile1(p, pStack);
			if (m_pClass == NULL ) m_pClass = nxt;
			else m_pClass->AddNext(nxt);
		}
		else
		{
			CBotFunction*	next = CBotFunction::Compile1(p, pStack, NULL);
			if (m_Prog == NULL ) m_Prog = next;
			else m_Prog->AddNext(next);
		}
	}
	if ( !pStack->IsOk() )
	{
		m_ErrorCode = pStack->GivError(m_ErrorStart, m_ErrorEnd); 
		delete m_Prog;
		m_Prog = NULL;
		delete pBaseToken;
		return false;
	}

//	CBotFunction*	temp = NULL;
	CBotFunction*	next = m_Prog;		// reprend la liste

	p  = pBaseToken->GivNext();								// revient au d�but

	while ( pStack->IsOk() && p != NULL && p->GivType() != 0 )
	{
		if ( IsOfType(p, ID_SEP) ) continue;				// des point-virgules qui trainent

		if ( p->GivType() == ID_CLASS || 
			( p->GivType() == ID_PUBLIC && p->GivNext()->GivType() == ID_CLASS ))
		{
			m_bCompileClass = true;
			CBotClass::Compile(p, pStack);					// compl�te la d�finition de la classe
		}
		else
		{
			m_bCompileClass = false;
			CBotFunction::Compile(p, pStack, next);
			if (next->IsExtern()) ListFonctions.Add(next->GivName()/* + next->GivParams()*/);
			next->m_pProg = this;							// garde le pointeur au module
			next = next->Next();
		}
	}

//	delete m_Prog;			// la liste de la 1�re passe
//	m_Prog = temp;			// la liste de la seconde passe

	if ( !pStack->IsOk() )
	{
		m_ErrorCode = pStack->GivError(m_ErrorStart, m_ErrorEnd); 
		delete m_Prog;
		m_Prog = NULL;
	}

	delete pBaseToken;
	delete pStack;

	return (m_Prog != NULL);
}


bool CBotProgram::Start(const char* name)
{
#if	STACKMEM
	m_pStack->Delete();
#else
	delete m_pStack;
#endif
	m_pStack = NULL;

	m_pRun = m_Prog;
	while (m_pRun != NULL)
	{
		if ( m_pRun->GivName() == name ) break;
		m_pRun = m_pRun->m_next;
	}

	if ( m_pRun == NULL )
	{
		m_ErrorCode = TX_NORUN;
		return false;
	}

#if	STACKMEM
	m_pStack = CBotStack::FirstStack();
#else
	m_pStack = new CBotStack(NULL);					// cr�e une pile d'ex�cution
#endif

	m_pStack->SetBotCall(this);						// bases pour les routines

	return true;									// on est pr�t pour un Run()
}

bool CBotProgram::GetPosition(const char* name, int& start, int& stop, CBotGet modestart, CBotGet modestop)
{
	CBotFunction* p = m_Prog;
	while (p != NULL)
	{
		if ( p->GivName() == name ) break;
		p = p->m_next;
	}

	if ( p == NULL ) return false;

	p->GetPosition(start, stop, modestart, modestop);
	return true;
}

bool CBotProgram::Run(void* pUser, int timer)
{
	bool	ok;

	if (m_pStack == NULL || m_pRun == NULL) goto error;

	m_ErrorCode = 0;
	if (m_pInstance != NULL && m_pInstance->m_pUserPtr != NULL)
		pUser = m_pInstance->m_pUserPtr;

	m_pStack->Reset(pUser);							// vide l'�ventuelle erreur pr�c�dente, et remet le timer
	if ( timer >= 0 ) m_pStack->SetTimer(timer);

	m_pStack->SetBotCall(this);						// bases pour les routines

#if	STACKRUN
	// reprend l'ex�cution sur le haut de la pile
	ok = m_pStack->Execute();
	if ( ok ) 
	{
#ifdef	_DEBUG
		CBotVar*	ppVar[3];
		ppVar[0] = CBotVar::Create("aa", CBotTypInt);
		ppVar[1] = CBotVar::Create("bb", CBotTypInt);
		ppVar[2] = NULL;
		ok = m_pRun->Execute(ppVar, m_pStack, m_pInstance);
#else
		// revient sur l'ex�cution normale
		ok = m_pRun->Execute(NULL, m_pStack, m_pInstance);
#endif
	}
#else
	ok = m_pRun->Execute(NULL, m_pStack, m_pInstance);
#endif

	// termin� sur une erreur ?
	if (!ok && !m_pStack->IsOk())
	{
		m_ErrorCode = m_pStack->GivError(m_ErrorStart, m_ErrorEnd);
#if	STACKMEM
		m_pStack->Delete();
#else
		delete m_pStack;
#endif
		m_pStack = NULL;
		return true;								// ex�cution termin�e !!
	}

	if ( ok ) m_pRun = NULL;						// plus de fonction en ex�cution
	return ok;

error:
	m_ErrorCode = TX_NORUN;
	return true;
}

void CBotProgram::Stop()
{
#if	STACKMEM
	m_pStack->Delete();
#else
	delete m_pStack;
#endif
	m_pStack = NULL;
	m_pRun = NULL;
}



bool CBotProgram::GetRunPos(const char* &FunctionName, int &start, int &end)
{
	FunctionName = NULL;
	start = end = 0;
	if (m_pStack == NULL) return false;

	m_pStack->GetRunPos(FunctionName, start, end);
	return true;
}

CBotVar* CBotProgram::GivStackVars(const char* &FunctionName, int level)
{
	FunctionName = NULL;
	if (m_pStack == NULL) return NULL;

	return m_pStack->GivStackVars(FunctionName, level);
}







void CBotProgram::SetTimer(int n)
{
	CBotStack::SetTimer( n );
}

int CBotProgram::GivError()
{
	return m_ErrorCode;
}

void CBotProgram::SetIdent(long n)
{
	m_Ident = n;
}

long CBotProgram::GivIdent()
{
	return m_Ident;
}

bool CBotProgram::GetError(int& code, int& start, int& end)
{
	code  = m_ErrorCode;
	start = m_ErrorStart;
	end   = m_ErrorEnd;
	return code > 0;
}

bool CBotProgram::GetError(int& code, int& start, int& end, CBotProgram* &pProg)
{
	code	= m_ErrorCode;
	start	= m_ErrorStart;
	end		= m_ErrorEnd;
	pProg	= this;
	return code > 0;
}

CBotString CBotProgram::GivErrorText(int code)
{
	CBotString TextError;

	TextError.LoadString( code );
	if (TextError.IsEmpty())
	{
		char	buf[100];
		sprintf(buf, "Exception num�ro %d.", code);
		TextError = buf;
	}
	return TextError;
}


CBotFunction* CBotProgram::GivFunctions()
{
	return	m_Prog;
}

bool CBotProgram::AddFunction(const char* name, 
							  bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser), 
							  CBotTypResult rCompile (CBotVar* &pVar, void* pUser))
{
	// m�morise les pointeurs aux deux fonctions
	return CBotCall::AddFunction(name, rExec, rCompile);
}


bool WriteWord(FILE* pf, unsigned short w)
{
	size_t	lg;

	lg = fwrite(&w, sizeof( unsigned short ), 1, pf );

	return (lg == 1);     
}

bool ReadWord(FILE* pf, unsigned short& w)
{
	size_t	lg;

	lg = fread(&w, sizeof( unsigned short ), 1, pf );

	return (lg == 1);
}

bool WriteFloat(FILE* pf, float w)
{
	size_t	lg;

	lg = fwrite(&w, sizeof( float ), 1, pf );

	return (lg == 1);     
}

bool ReadFloat(FILE* pf, float& w)
{
	size_t	lg;

	lg = fread(&w, sizeof( float ), 1, pf );

	return (lg == 1);
}

bool WriteLong(FILE* pf, long w)
{
	size_t	lg;

	lg = fwrite(&w, sizeof( long ), 1, pf );

	return (lg == 1);     
}

bool ReadLong(FILE* pf, long& w)
{
	size_t	lg;

	lg = fread(&w, sizeof( long ), 1, pf );

	return (lg == 1);
}

bool WriteString(FILE* pf, CBotString s)
{
	size_t	lg1, lg2;

	lg1 = s.GivLength();
	if (!WriteWord(pf, lg1)) return false;

	lg2 = fwrite(s, 1, lg1, pf );
	return (lg1 == lg2);     
}

bool ReadString(FILE* pf, CBotString& s)
{
	unsigned short	w;
	char	buf[1000];
	size_t	lg1, lg2;

	if (!ReadWord(pf, w)) return false;
	lg1 = w;
	lg2 = fread(buf, 1, lg1, pf );
	buf[lg2] = 0;

	s = buf;
	return (lg1 == lg2);
}

bool WriteType(FILE* pf, CBotTypResult type)
{
	int	typ = type.GivType();
	if ( typ == CBotTypIntrinsic ) typ = CBotTypClass;
	if ( !WriteWord(pf, typ) ) return false;
	if ( typ == CBotTypClass )
	{
		CBotClass* p = type.GivClass();
		if ( !WriteString(pf, p->GivName()) ) return false;
	}
	if ( type.Eq( CBotTypArrayBody ) ||
		 type.Eq( CBotTypArrayPointer ) )
	{
		if ( !WriteWord(pf, type.GivLimite()) ) return false;
		if ( !WriteType(pf, type.GivTypElem()) ) return false;
	}
	return true;
}

bool ReadType(FILE* pf, CBotTypResult& type)
{
	unsigned short	w, ww;
	if ( !ReadWord(pf, w) ) return false;
	type.SetType(w);

	if ( type.Eq( CBotTypIntrinsic ) )
	{
		type = CBotTypResult( w, "point" );
	}

	if ( type.Eq( CBotTypClass ) )
	{
		CBotString	s;
		if ( !ReadString(pf, s) ) return false;
		type = CBotTypResult( w, s );
	}

	if ( type.Eq( CBotTypArrayPointer ) ||
		 type.Eq( CBotTypArrayBody ) )
	{
		CBotTypResult	r;
		if ( !ReadWord(pf, ww) ) return false;
		if ( !ReadType(pf, r) ) return false;
		type = CBotTypResult( w, r );
		type.SetLimite((short)ww);
	}
	return true;
}


bool CBotProgram::DefineNum(const char* name, long val)
{
	return CBotToken::DefineNum(name, val);
}


bool CBotProgram::SaveState(FILE* pf)
{
	if (!WriteWord( pf, CBOTVERSION)) return false;


	if ( m_pStack != NULL )
	{
		if (!WriteWord( pf, 1)) return false;
		if (!WriteString( pf, m_pRun->GivName() )) return false;
		if (!m_pStack->SaveState(pf)) return false;
	}
	else 
	{
		if (!WriteWord( pf, 0)) return false;
	}
	return true;
}


bool CBotProgram::RestoreState(FILE* pf)
{
	unsigned short  w;
	CBotString      s;

	Stop();

	if (!ReadWord( pf, w )) return false;
	if ( w != CBOTVERSION ) return false;

	if (!ReadWord( pf, w )) return false;
	if ( w == 0 ) return true;

	if (!ReadString( pf, s )) return false;
	Start(s);		// point de reprise

#if	STACKMEM
	m_pStack->Delete();
#else
	delete m_pStack;
#endif
	m_pStack = NULL;

	// r�cup�re la pile depuis l'enregistrement
	// utilise un pointeur NULL (m_pStack) mais c'est ok comme �a
	if (!m_pStack->RestoreState(pf, m_pStack)) return false;
	m_pStack->SetBotCall(this);						// bases pour les routines

	// r�tabli certains �tats dans la pile selon la structure
	m_pRun->RestoreState(NULL, m_pStack, m_pInstance);
	return true;
}

int CBotProgram::GivVersion()
{
	return	CBOTVERSION;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

CBotCall* CBotCall::m_ListCalls = NULL;
	
CBotCall::CBotCall(const char* name, 
				   bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser), 
				   CBotTypResult rCompile (CBotVar* &pVar, void* pUser))
{
	m_name		 = name;
	m_rExec		 = rExec;
	m_rComp		 = rCompile;
	m_next		 = NULL;
	m_nFuncIdent = CBotVar::NextUniqNum();
}

CBotCall::~CBotCall()
{
	if (m_next) delete m_next;
	m_next = NULL;
}

void CBotCall::Free()
{
	delete CBotCall::m_ListCalls;
}

bool CBotCall::AddFunction(const char* name, 
						   bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser), 
						   CBotTypResult rCompile (CBotVar* &pVar, void* pUser))
{
	CBotCall*	p = m_ListCalls;
	CBotCall*	pp = NULL;

	if ( p != NULL ) while ( p->m_next != NULL )
	{
		if ( p->GivName() == name )
		{
			// lib�re une fonction qu'on red�fini
			if ( pp ) pp->m_next = p->m_next;
			else	  m_ListCalls = p->m_next;
			pp = p;
			p = p->m_next;
			pp->m_next = NULL;	// ne pas d�truire la suite de la liste
			delete pp;
			continue;
		}
		pp = p;				// pointeur pr�c�dent
		p = p->m_next;
	}

	pp = new CBotCall(name, rExec, rCompile);
	
	if (p) p->m_next = pp;
	else m_ListCalls = pp;

	return true;
}


// transforme le tableau de pointeurs aux variables
// en une liste de variables cha�n�es
CBotVar* MakeListVars(CBotVar** ppVars, bool bSetVal=false)
{
	int		i = 0;
	CBotVar*	pVar = NULL;

	while( true )
	{
		ppVars[i];
		if ( ppVars[i] == NULL ) break;

		CBotVar*	pp = CBotVar::Create(ppVars[i]);
		if (bSetVal) pp->Copy(ppVars[i]);
		else
			if ( ppVars[i]->GivType() == CBotTypPointer )
				pp->SetClass( ppVars[i]->GivClass());
// copier le pointeur selon indirection		
		if (pVar == NULL) pVar = pp;
		else pVar->AddNext(pp);
		i++;
	}
	return pVar;
}

// trouve un appel acceptable selon le nom de la proc�dure
// et les param�tres donn�s

CBotTypResult CBotCall::CompileCall(CBotToken* &p, CBotVar** ppVar, CBotCStack* pStack, long& nIdent)
{
	nIdent = 0;
	CBotCall*	pt = m_ListCalls;
	CBotString	name = p->GivString();

	while ( pt != NULL )
	{
		if ( pt->m_name == name )
		{
			CBotVar*	pVar = MakeListVars(ppVar);
			CBotVar*	pVar2 = pVar;
			CBotTypResult r = pt->m_rComp(pVar2, m_pUser);
			int ret = r.GivType();
			
			// si une classe est retourn�e, c'est en fait un pointeur
			if ( ret == CBotTypClass ) r.SetType( ret = CBotTypPointer );

			if ( ret > 20 )
			{
				if (pVar2) pStack->SetError(ret, p /*pVar2->GivToken()*/ );
			}
			delete pVar;
			nIdent = pt->m_nFuncIdent;
			return r;
		}
		pt = pt->m_next;
	}
	return -1;
}

void* CBotCall::m_pUser = NULL;

void CBotCall::SetPUser(void* pUser)
{
	m_pUser = pUser;
}

bool CBotCall::CheckCall(const char* name)
{
	CBotCall* p = m_ListCalls;

	while ( p != NULL )
	{
		if ( name == p->GivName() ) return true;
		p = p->m_next;
	}
	return false;
}



CBotString CBotCall::GivName()
{
	return	m_name;
}

CBotCall* CBotCall::Next()
{
	return	m_next;
}


int CBotCall::DoCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack, CBotTypResult& rettype)
{
	CBotCall*	pt = m_ListCalls;

	if ( nIdent ) while ( pt != NULL )
	{
		if ( pt->m_nFuncIdent == nIdent )
		{
			goto fund;
		}
		pt = pt->m_next;
	}

	pt = m_ListCalls;

	if ( token != NULL )
	{
		CBotString name = token->GivString();
		while ( pt != NULL )
		{
			if ( pt->m_name == name )
			{
				nIdent = pt->m_nFuncIdent;
				goto fund;
			}
			pt = pt->m_next;
		}
	}

	return -1;

fund:
#if !STACKRUN
	// fait la liste des param�tres selon le contenu de la pile (pStackVar)

	CBotVar*	pVar = MakeListVars(ppVar, true);
	CBotVar*	pVarToDelete = pVar;

	// cr�e une variable pour le r�sultat
	CBotVar*	pResult = rettype.Eq(0) ? NULL : CBotVar::Create("", rettype);

	CBotVar*	pRes = pResult;
	int			Exception = 0;
	int res = pt->m_rExec(pVar, pResult, Exception, pStack->GivPUser());

	if ( pResult != pRes ) delete pRes;	// si r�sultat diff�rent rendu
	delete pVarToDelete;

	if (res == false)
	{
		if (Exception!=0)
		{
			pStack->SetError(Exception, token);
		}
		delete pResult;
		return false;
	}
	pStack->SetVar(pResult);

	if ( rettype.GivType() > 0 && pResult == NULL )
	{
		pStack->SetError(TX_NORETVAL, token);
	}
	nIdent = pt->m_nFuncIdent;
	return true;

#else

	CBotStack*	pile = pStack->AddStackEOX(pt);
	if ( pile == EOX ) return true;

	// fait la liste des param�tres selon le contenu de la pile (pStackVar)

	CBotVar*	pVar = MakeListVars(ppVar, true);
	CBotVar*	pVarToDelete = pVar;

	// cr�e une variable pour le r�sultat
	CBotVar*	pResult = rettype.Eq(0) ? NULL : CBotVar::Create("", rettype);

	pile->SetVar( pVar );

	CBotStack*	pile2 = pile->AddStack();
	pile2->SetVar( pResult );

	pile->SetError(0, token);			// pour la position en cas d'erreur + loin
	return pt->Run( pStack );

#endif

}

#if	STACKRUN

bool CBotCall::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack)
{
	CBotCall*	pt = m_ListCalls;

	{
		CBotString name = token->GivString();
		while ( pt != NULL )
		{
			if ( pt->m_name == name )
			{
				nIdent = pt->m_nFuncIdent;

				CBotStack*	pile = pStack->RestoreStackEOX(pt);
				if ( pile == NULL ) return true;

				CBotStack*	pile2 = pile->RestoreStack();
				return true;
			}
			pt = pt->m_next;
		}
	}

	return false;
}

bool CBotCall::Run(CBotStack* pStack)
{
	CBotStack*	pile = pStack->AddStackEOX(this);
	if ( pile == EOX ) return true;
	CBotVar*	pVar = pile->GivVar();

	CBotStack*	pile2 = pile->AddStack();
	CBotVar*	pResult = pile2->GivVar();
	CBotVar*	pRes = pResult;

	int			Exception = 0;
	int res = m_rExec(pVar, pResult, Exception, pStack->GivPUser());

	if (res == false)
	{
		if (Exception!=0)
		{
			pStack->SetError(Exception);
		}
		if ( pResult != pRes ) delete pResult;	// si r�sultat diff�rent rendu
		return false;
	}

	if ( pResult != NULL ) pStack->SetCopyVar( pResult );
	if ( pResult != pRes ) delete pResult;	// si r�sultat diff�rent rendu

	return true;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////

CBotCallMethode::CBotCallMethode(const char* name, 
				   bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception), 
				   CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar))
{
	m_name		 = name;
	m_rExec		 = rExec;
	m_rComp		 = rCompile;
	m_next		 = NULL;
	m_nFuncIdent = CBotVar::NextUniqNum();
}

CBotCallMethode::~CBotCallMethode()
{
	delete m_next;
	m_next = NULL;
}

// trouve un appel acceptable selon le nom de la proc�dure
// et les param�tres donn�s

CBotTypResult CBotCallMethode::CompileCall(const char* name, CBotVar* pThis, 
										   CBotVar** ppVar, CBotCStack* pStack,
										   long& nIdent)
{
	CBotCallMethode*	pt = this;
	nIdent = 0;

	while ( pt != NULL )
	{
		if ( pt->m_name == name )
		{
			CBotVar*	pVar = MakeListVars(ppVar, true);
			CBotVar*	pVar2 = pVar;
			CBotTypResult r = pt->m_rComp(pThis, pVar2);
			int	ret = r.GivType();
			if ( ret > 20 )
			{
				if (pVar2) pStack->SetError(ret, pVar2->GivToken());
			}
			delete pVar;
			nIdent = pt->m_nFuncIdent;
			return r;
		}
		pt = pt->m_next;
	}
	return CBotTypResult(-1);
}


CBotString CBotCallMethode::GivName()
{
	return	m_name;
}

CBotCallMethode* CBotCallMethode::Next()
{
	return	m_next;
}

void CBotCallMethode::AddNext(CBotCallMethode* pt)
{
	CBotCallMethode* p = this;
	while ( p->m_next != NULL ) p = p->m_next;

	p->m_next = pt;
}


int CBotCallMethode::DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotVar* &pResult, CBotStack* pStack, CBotToken* pToken)
{
	CBotCallMethode*	pt = this;

	// recherche selon l'identificateur

	if ( nIdent ) while ( pt != NULL )
	{
		if ( pt->m_nFuncIdent == nIdent )
		{
			// fait la liste des param�tres selon le contenu de la pile (pStackVar)

			CBotVar*	pVar = MakeListVars(ppVars, true);
			CBotVar*	pVarToDelete = pVar;

			// puis appelle la routine externe au module

			int			Exception = 0;
			int res = pt->m_rExec(pThis, pVar, pResult, Exception);
			pStack->SetVar(pResult);

			if (res == false)
			{
				if (Exception!=0)
				{
//					pStack->SetError(Exception, pVar->GivToken());
					pStack->SetError(Exception, pToken);
				}
				delete pVarToDelete;
				return false;
			}
			delete pVarToDelete;
			return true;
		}
		pt = pt->m_next;
	}

	// recherche selon le nom

	while ( pt != NULL )
	{
		if ( pt->m_name == name )
		{
			// fait la liste des param�tres selon le contenu de la pile (pStackVar)

			CBotVar*	pVar = MakeListVars(ppVars, true);
			CBotVar*	pVarToDelete = pVar;

			int			Exception = 0;
			int res = pt->m_rExec(pThis, pVar, pResult, Exception);
			pStack->SetVar(pResult);

			if (res == false)
			{
				if (Exception!=0)
				{
//					pStack->SetError(Exception, pVar->GivToken());
					pStack->SetError(Exception, pToken);
				}
				delete pVarToDelete;
				return false;
			}
			delete pVarToDelete;
			nIdent = pt->m_nFuncIdent;
			return true;
		}
		pt = pt->m_next;
	}

	return -1;
}

bool rSizeOf( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	if ( pVar == NULL ) return TX_LOWPARAM;

	int	i = 0;
	pVar = pVar->GivItemList();

	while ( pVar != NULL )
	{
		i++; 
		pVar = pVar->GivNext();
	}

	pResult->SetValInt(i);
	return true;
}

CBotTypResult cSizeOf( CBotVar* &pVar, void* pUser )
{
	if ( pVar == NULL ) return CBotTypResult( TX_LOWPARAM );
	if ( pVar->GivType() != CBotTypArrayPointer )
						return CBotTypResult( TX_BADPARAM );
	return CBotTypResult( CBotTypInt );
}


CBotString CBotProgram::m_DebugVarStr = "";

bool rCBotDebug( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
	pResult->SetValString( CBotProgram::m_DebugVarStr );

	return true;
}

CBotTypResult cCBotDebug( CBotVar* &pVar, void* pUser )
{
	// pas de param�tre
	if ( pVar != NULL ) return CBotTypResult( TX_OVERPARAM );

	// la fonction retourne un r�sultat "string"
	return CBotTypResult( CBotTypString );
}


#include "StringFunctions.cpp"

void CBotProgram::Init()
{
	CBotToken::DefineNum( "CBotErrOpenPar", 5000) ;		// manque la parenth�se ouvrante
	CBotToken::DefineNum( "CBotErrClosePar", 5001) ;	// manque la parenth�se fermante
	CBotToken::DefineNum( "CBotErrNotBoolean", 5002) ;	// l'expression doit �tre un boolean
	CBotToken::DefineNum( "CBotErrUndefVar", 5003) ;	// variable non d�clar�e
	CBotToken::DefineNum( "CBotErrBadLeft", 5004) ;		// assignation impossible ( 5 = ... )
	CBotToken::DefineNum( "CBotErrNoTerminator", 5005) ;// point-virgule attendu
	CBotToken::DefineNum( "CBotErrCaseOut", 5006) ;		// case en dehors d'un switch
	CBotToken::DefineNum( "CBotErrCloseBlock", 5008) ;	// manque " } "
	CBotToken::DefineNum( "CBotErrElseWhitoutIf", 5009) ;// else sans if correspondant
	CBotToken::DefineNum( "CBotErrOpenBlock", 5010) ;	// manque " { "
	CBotToken::DefineNum( "CBotErrBadType1", 5011) ;	// mauvais type pour l'assignation
	CBotToken::DefineNum( "CBotErrRedefVar", 5012) ;	// red�finition de la variable
	CBotToken::DefineNum( "CBotErrBadType2", 5013) ;	// 2 op�randes de type incompatibles
	CBotToken::DefineNum( "CBotErrUndefCall", 5014) ;	// routine inconnue
	CBotToken::DefineNum( "CBotErrNoDoubleDots", 5015) ;// " : " attendu
	CBotToken::DefineNum( "CBotErrBreakOutside", 5017) ;// break en dehors d'une boucle
	CBotToken::DefineNum( "CBotErrUndefLabel", 5019) ;	// label inconnu
	CBotToken::DefineNum( "CBotErrLabel", 5018) ;		// label ne peut se mettre ici
	CBotToken::DefineNum( "CBotErrNoCase", 5020) ;		// manque " case "
	CBotToken::DefineNum( "CBotErrBadNum", 5021) ;		// nombre attendu
	CBotToken::DefineNum( "CBotErrVoid", 5022) ;		// " void " pas possible ici
	CBotToken::DefineNum( "CBotErrNoType", 5023) ;		// d�claration de type attendue
	CBotToken::DefineNum( "CBotErrNoVar", 5024) ;		// nom de variable attendu
	CBotToken::DefineNum( "CBotErrNoFunc", 5025) ;		// nom de fonction attendu
	CBotToken::DefineNum( "CBotErrOverParam", 5026) ;	// trop de param�tres
	CBotToken::DefineNum( "CBotErrRedefFunc", 5027) ;	// cette fonction existe d�j�
	CBotToken::DefineNum( "CBotErrLowParam", 5028) ;	// pas assez de param�tres
	CBotToken::DefineNum( "CBotErrBadParam", 5029) ;	// mauvais types de param�tres
	CBotToken::DefineNum( "CBotErrNbParam", 5030) ;		// mauvais nombre de param�tres
	CBotToken::DefineNum( "CBotErrUndefItem", 5031) ;	// �l�ment n'existe pas dans la classe
	CBotToken::DefineNum( "CBotErrUndefClass", 5032) ;	// variable n'est pas une classe
	CBotToken::DefineNum( "CBotErrNoConstruct", 5033) ;	// pas de constructeur appropri�
	CBotToken::DefineNum( "CBotErrRedefClass", 5034) ;	// classe existe d�j�
	CBotToken::DefineNum( "CBotErrCloseIndex", 5035) ;	// " ] " attendu
	CBotToken::DefineNum( "CBotErrReserved", 5036) ;	// mot r�serv� (par un DefineNum)

// voici la liste des erreurs pouvant �tre retourn�es par le module
// pour l'ex�cution

	CBotToken::DefineNum( "CBotErrZeroDiv", 6000) ;		// division par z�ro
	CBotToken::DefineNum( "CBotErrNotInit", 6001) ;		// variable non initialis�e
	CBotToken::DefineNum( "CBotErrBadThrow", 6002) ;	// throw d'une valeur n�gative
	CBotToken::DefineNum( "CBotErrNoRetVal", 6003) ;	// fonction n'a pas retourn� de r�sultat
	CBotToken::DefineNum( "CBotErrNoRun", 6004) ;		// Run() sans fonction active
	CBotToken::DefineNum( "CBotErrUndefFunc", 6005) ;	// appel d'une fonction qui n'existe plus

	CBotProgram::AddFunction("sizeof", rSizeOf, cSizeOf );

	InitStringFunctions();

	// une fonction juste pour les debug divers
	CBotProgram::AddFunction("CBOTDEBUGDD", rCBotDebug, cCBotDebug);
    //TODO implement this deletion
    // DeleteFile("CbotDebug.txt");

}

void CBotProgram::Free()
{
	CBotToken::Free() ; 
	CBotCall ::Free() ;
	CBotClass::Free() ;
}

