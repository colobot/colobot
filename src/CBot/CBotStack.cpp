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
// gestion de la pile (stack)

#include "CBot.h"


#define ITIMER  100

////////////////////////////////////////////////////////////////////////////
// gestion de la pile d'exécution
////////////////////////////////////////////////////////////////////////////

int             CBotStack::m_initimer = ITIMER;     // init la variable statique
int             CBotStack::m_timer = 0;             // init la variable statique
CBotVar*        CBotStack::m_retvar = NULL;         // init la variable statique
int             CBotStack::m_error = 0;             // init la variable statique
int             CBotStack::m_start = 0;             // init la variable statique
int             CBotStack::m_end   = 0;             // init la variable statique
CBotString      CBotStack::m_labelBreak="";         // init la variable statique
void*           CBotStack::m_pUser = NULL;

#if STACKMEM

CBotStack* CBotStack::FirstStack()
{
    CBotStack*  p;

    long    size = sizeof(CBotStack);
    size    *= (MAXSTACK+10);

    // demande une tranche mémoire pour la pile
    p = (CBotStack*)malloc(size);

    // la vide totalement
    memset(p, 0, size);

    p-> m_bBlock = TRUE;
    m_timer = m_initimer;               // met le timer au début

    CBotStack* pp = p;
    pp += MAXSTACK;
    int i;
    for ( i = 0 ; i< 10 ; i++ )
    {
        pp->m_bOver = TRUE;
        pp ++;
    }
#ifdef  _DEBUG
    int n = 1;
    pp = p;
    for ( i = 0 ; i< MAXSTACK+10 ; i++ )
    {
        pp->m_index = n++;
        pp ++;
    }
#endif

    m_error = 0;    // évite des blocages car m_error est static
    return p;
}

CBotStack::CBotStack(CBotStack* ppapa)
{
    // constructeur doit exister, sinon le destructeur n'est jamais appelé !
    ASM_TRAP();
}

CBotStack::~CBotStack()
{
    ASM_TRAP(); // utiliser Delete() à la place
}

void CBotStack::Delete()
{
    if ( this == NULL || this == EOX ) return;

    m_next->Delete();
    m_next2->Delete();

    if (m_prev != NULL)
    {
        if ( m_prev->m_next == this )
            m_prev->m_next = NULL;      // enlève de la chaîne

        if ( m_prev->m_next2 == this )
            m_prev->m_next2 = NULL;     // enlève de la chaîne
    }

    delete m_var;
    delete m_listVar;

    CBotStack*  p = m_prev;
    BOOL        bOver = m_bOver;
#ifdef  _DEBUG
    int         n = m_index;
#endif

    // efface le bloc libéré
    memset(this, 0, sizeof(CBotStack));
    m_bOver = bOver;
#ifdef  _DEBUG
    m_index = n;
#endif

    if ( p == NULL )
        free( this );
}


// routine optimisée
CBotStack* CBotStack::AddStack(CBotInstr* instr, BOOL bBlock)
{
    if (m_next != NULL)
    {
        return m_next;              // reprise dans une pile existante
    }

#ifdef  _DEBUG
    int     n = 0;
#endif
    CBotStack*  p = this;
    do
    {
        p ++;
#ifdef  _DEBUG
        n ++;
#endif
    }
    while ( p->m_prev != NULL );

    m_next = p;                                 // chaîne l'élément
    p->m_bBlock      = bBlock;
    p->m_instr       = instr;
    p->m_prog        = m_prog;
    p->m_step        = 0;
    p->m_prev        = this;
    p->m_state       = 0;
    p->m_call        = NULL;
    p->m_bFunc       = FALSE;
    return  p;
}

CBotStack* CBotStack::AddStackEOX(CBotCall* instr, BOOL bBlock)
{
    if (m_next != NULL)
    {
        if ( m_next == EOX )
        {
            m_next = NULL;
            return EOX;
        }
        return m_next;              // reprise dans une pile existante
    }
    CBotStack*  p = AddStack(NULL, bBlock);
    p->m_call = instr;
    p->m_bFunc = 2; // spécial
    return  p;
}

CBotStack* CBotStack::AddStack2(BOOL bBlock)
{
    if (m_next2 != NULL)
    {
        m_next2->m_prog = m_prog;       // spécial évite un RestoreStack2
        return m_next2;                 // reprise dans une pile existante
    }

    CBotStack*  p = this;
    do
    {
        p ++;
    }
    while ( p->m_prev != NULL );

    m_next2 = p;                                // chaîne l'élément
    p->m_prev = this;
    p->m_bBlock = bBlock;
    p->m_prog = m_prog;
    p->m_step = 0;
    return  p;
}

BOOL CBotStack::GivBlock()
{
    return  m_bBlock;
}

BOOL CBotStack::Return(CBotStack* pfils)
{
    if ( pfils == this ) return TRUE;   // spécial

    if (m_var != NULL) delete m_var;            // valeur remplacée ?
    m_var = pfils->m_var;                       // résultat transmis
    pfils->m_var = NULL;                        // ne pas détruire la variable

    m_next->Delete();m_next = NULL;             // libère la pile au dessus
    m_next2->Delete();m_next2 = NULL;           // aussi la seconde pile (catch)

    return (m_error == 0);                      // interrompu si erreur
}

BOOL CBotStack::ReturnKeep(CBotStack* pfils)
{
    if ( pfils == this ) return TRUE;   // spécial

    if (m_var != NULL) delete m_var;            // valeur remplacée ?
    m_var = pfils->m_var;                       // résultat transmis
    pfils->m_var = NULL;                        // ne pas détruire la variable

    return (m_error == 0);                      // interrompu si erreur
}

BOOL CBotStack::StackOver()
{
    if (!m_bOver) return FALSE;
    m_error = TX_STACKOVER;
    return TRUE;
}

#else

CBotStack::CBotStack(CBotStack* ppapa)
{
    m_next  = NULL;
    m_next2 = NULL;
    m_prev  = ppapa;

    m_bBlock = (ppapa == NULL) ? TRUE : FALSE;

    m_state = 0;
    m_step = 1;

    if (ppapa == NULL) m_timer = m_initimer;                // met le timer au début

    m_listVar = NULL;
    m_bDontDelete = FALSE;

    m_var     = NULL;
    m_prog    = NULL;
    m_instr   = NULL;
    m_call    = NULL;
    m_bFunc   = FALSE;
}

// destructeur
CBotStack::~CBotStack()
{
    if ( m_next != EOX) delete m_next;
    delete m_next2;
    if (m_prev != NULL && m_prev->m_next == this )
            m_prev->m_next = NULL;      // enlève de la chaîne

    delete m_var;
    if ( !m_bDontDelete ) delete m_listVar;
}

// routine à optimiser
CBotStack* CBotStack::AddStack(CBotInstr* instr, BOOL bBlock)
{
    if (m_next != NULL)
    {
        return m_next;              // reprise dans une pile existante
    }
    CBotStack*  p = new CBotStack(this);
    m_next = p;                                 // chaîne l'élément
    p->m_bBlock = bBlock;
    p->m_instr = instr;
    p->m_prog = m_prog;
    p->m_step = 0;
    return  p;
}

CBotStack* CBotStack::AddStackEOX(CBotCall* instr, BOOL bBlock)
{
    if (m_next != NULL)
    {
        if ( m_next == EOX )
        {
            m_next = NULL;
            return EOX;
        }
        return m_next;              // reprise dans une pile existante
    }
    CBotStack*  p = new CBotStack(this);
    m_next = p;                                 // chaîne l'élément
    p->m_bBlock = bBlock;
    p->m_call = instr;
    p->m_prog = m_prog;
    p->m_step = 0;
    p->m_bFunc = 2; // spécial
    return  p;
}

CBotStack* CBotStack::AddStack2(BOOL bBlock)
{
    if (m_next2 != NULL)
    {
        m_next2->m_prog = m_prog;       // spécial évite un RestoreStack2
        return m_next2;                 // reprise dans une pile existante
    }

    CBotStack*  p = new CBotStack(this);
    m_next2 = p;                                // chaîne l'élément
    p->m_bBlock = bBlock;
    p->m_prog = m_prog;
    p->m_step = 0;

    return  p;
}

BOOL CBotStack::Return(CBotStack* pfils)
{
    if ( pfils == this ) return TRUE;   // spécial

    if (m_var != NULL) delete m_var;            // valeur remplacée ?
    m_var = pfils->m_var;                       // résultat transmis
    pfils->m_var = NULL;                        // ne pas détruite la variable

    if ( m_next != EOX ) delete m_next;         // libère la pile au dessus
    delete m_next2;m_next2 = NULL;              // aussi la seconde pile (catch)

    return (m_error == 0);                      // interrompu si erreur
}

BOOL CBotStack::StackOver()
{
    return FALSE;           // pas de test de débordement dans cette version
}

#endif

void CBotStack::Reset(void* pUser)
{
    m_timer = m_initimer;       // remet le timer
    m_error = 0;
//  m_start = 0;
//  m_end   = 0;
    m_labelBreak.Empty();
    m_pUser = pUser;
}




CBotStack* CBotStack::RestoreStack(CBotInstr* instr)
{
    if (m_next != NULL)
    {
        m_next->m_instr = instr;    // réinit (si reprise après restitution)
        m_next->m_prog = m_prog;
        return m_next;              // reprise dans une pile existante
    }
    return  NULL;
}

CBotStack* CBotStack::RestoreStackEOX(CBotCall* instr)
{
    CBotStack*   p = RestoreStack();
    p->m_call = instr;
    return p;
}



// routine pour l'exécution pas à pas
BOOL CBotStack::IfStep()
{
    if ( m_initimer > 0 || m_step++ > 0 ) return FALSE;
    return TRUE;
}


BOOL CBotStack::BreakReturn(CBotStack* pfils, const char* name)
{
    if ( m_error>=0 ) return FALSE;             // sortie normale
    if ( m_error==-3 ) return FALSE;            // sortie normale (return en cours)

    if (!m_labelBreak.IsEmpty() && (name[0] == 0 || m_labelBreak != name))
        return FALSE;                           // c'est pas pour moi

    m_error = 0;
    m_labelBreak.Empty();
    return Return(pfils);
}

BOOL CBotStack::IfContinue(int state, const char* name)
{
    if ( m_error != -2 ) return FALSE;

    if (!m_labelBreak.IsEmpty() && (name == NULL || m_labelBreak != name))
        return FALSE;                           // c'est pas pour moi

    m_state = state;                            // où reprendre ?
    m_error = 0;
    m_labelBreak.Empty();
    if ( m_next != EOX ) m_next->Delete();          // purge la pile au dessus
    return TRUE;
}

void CBotStack::SetBreak(int val, const char* name)
{
    m_error = -val;                             // réagit comme une Exception
    m_labelBreak = name;
    if (val == 3)   // pour un return
    {
        m_retvar = m_var;
        m_var = NULL;
    }
}

// remet sur la pile la valeur calculée par le dernier CBotReturn

BOOL CBotStack::GivRetVar(BOOL bRet)
{
    if (m_error == -3)
    {
        if ( m_var ) delete m_var;
        m_var       = m_retvar;
        m_retvar    = NULL;
        m_error     = 0;
        return      TRUE;
    }
    return bRet;                        // interrompu par autre chose que return
}

int CBotStack::GivError(int& start, int& end)
{
    start = m_start;
    end   = m_end;
    return m_error;
}


// type d'instruction sur la pile
int CBotStack::GivType(int mode)
{
    if (m_var == NULL) return -1;
    return m_var->GivType(mode);
}

// type d'instruction sur la pile
CBotTypResult CBotStack::GivTypResult(int mode)
{
    if (m_var == NULL) return -1;
    return m_var->GivTypResult(mode);
}

// type d'instruction sur la pile
void CBotStack::SetType(CBotTypResult& type)
{
    if (m_var == NULL) return;
    m_var->SetType( type );
}


// trouve une variable par son token
// ce peut être une variable composée avec un point
CBotVar* CBotStack::FindVar(CBotToken* &pToken, BOOL bUpdate, BOOL bModif)
{
    CBotStack*  p = this;
    CBotString  name = pToken->GivString();

    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (pp->GivName() == name)
            {
                if ( bUpdate )
                    pp->Maj(m_pUser, FALSE);

                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return NULL;
}

CBotVar* CBotStack::FindVar(const char* name)
{
    CBotStack*  p = this;
    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (pp->GivName() == name)
            {
                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return NULL;
}

// retrouve une variable sur la pile selon son numéro d'identification
// ce qui va plus vite que de comparer les noms.

CBotVar* CBotStack::FindVar(long ident, BOOL bUpdate, BOOL bModif)
{
    CBotStack*  p = this;
    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (pp->GivUniqNum() == ident)
            {
                if ( bUpdate )
                    pp->Maj(m_pUser, FALSE);

                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return NULL;
}


CBotVar* CBotStack::FindVar(CBotToken& Token, BOOL bUpdate, BOOL bModif)
{
    CBotToken*  pt = &Token;
    return FindVar(pt, bUpdate, bModif);
}


CBotVar* CBotStack::CopyVar(CBotToken& Token, BOOL bUpdate)
{
    CBotVar*    pVar = FindVar( Token, bUpdate );

    if ( pVar == NULL) return NULL;

    CBotVar*    pCopy = CBotVar::Create(pVar);
    pCopy->Copy(pVar);
    return  pCopy;
}


BOOL CBotStack::SetState(int n, int limite)
{
    m_state = n;

    m_timer--;                                  // décompte les opérations
    return ( m_timer > limite );                    // interrompu si timer passé
}

BOOL CBotStack::IncState(int limite)
{
    m_state++;

    m_timer--;                                  // décompte les opérations
    return ( m_timer > limite );                    // interrompu si timer passé
}


void CBotStack::SetError(int n, CBotToken* token)
{
    if ( n!= 0 && m_error != 0) return; // ne change pas une erreur déjà existante
    m_error = n;
    if (token != NULL)
    {
        m_start = token->GivStart();
        m_end   = token->GivEnd();
    }
}

void CBotStack::ResetError(int n, int start, int end)
{
    m_error = n;
    m_start = start;
    m_end   = end;
}

void CBotStack::SetPosError(CBotToken* token)
{
    m_start = token->GivStart();
    m_end   = token->GivEnd();
}

void CBotStack::SetTimer(int n)
{
    m_initimer = n;
}

BOOL CBotStack::Execute()
{
    CBotCall*       instr = NULL;                       // instruction la plus élevée
    CBotStack*      pile;

    CBotStack*      p = this;

    while (p != NULL)
    {
        if ( p->m_next2 != NULL ) break;
        if ( p->m_call != NULL )
        {
            instr = p->m_call;
            pile  = p->m_prev ;
        }
        p = p->m_next;
    }

    if ( instr == NULL ) return TRUE;               // exécution normale demandée

    if (!instr->Run(pile)) return FALSE;            // exécution à partir de là

#if STACKMEM
    pile->m_next->Delete();
#else
    delete pile->m_next;
#endif

    pile->m_next = EOX;         // spécial pour reprise
    return TRUE;
}

// met sur le stack le pointeur à une variable
void CBotStack::SetVar( CBotVar* var )
{
    if (m_var) delete m_var;    // remplacement d'une variable
    m_var = var;
}

// met sur le stack une copie d'une variable
void CBotStack::SetCopyVar( CBotVar* var )
{
    if (m_var) delete m_var;    // remplacement d'une variable

    m_var = CBotVar::Create("", var->GivTypResult(2));
    m_var->Copy( var );
}

CBotVar* CBotStack::GivVar()
{
    return m_var;
}

CBotVar* CBotStack::GivPtVar()
{
    CBotVar*    p = m_var;
    m_var = NULL;               // ne sera pas détruit donc
    return p;
}

CBotVar* CBotStack::GivCopyVar()
{
    if (m_var == NULL) return NULL;
    CBotVar*    v = CBotVar::Create("", m_var->GivType());
    v->Copy( m_var );
    return v;
}

long CBotStack::GivVal()
{
    if (m_var == NULL) return 0;
    return m_var->GivValInt();
}




void CBotStack::AddVar(CBotVar* pVar)
{
    CBotStack*  p = this;

    // revient sur l'élement père
    while (p != NULL && p->m_bBlock == 0) p = p->m_prev;

    if ( p == NULL ) return;

/// p->m_bDontDelete = bDontDelete;

    CBotVar**   pp = &p->m_listVar;
    while ( *pp != NULL ) pp = &(*pp)->m_next;

    *pp = pVar;                 // ajoute à la suite

#ifdef  _DEBUG
    if ( pVar->GivUniqNum() == 0 ) ASM_TRAP();
#endif
}

/*void CBotStack::RestoreVar(CBotVar* pVar)
{
    if ( !m_bDontDelete ) __asm int 3;
    delete  m_listVar;
    m_listVar = pVar;       // remplace directement
}*/

void CBotStack::SetBotCall(CBotProgram* p)
{
    m_prog  = p;
    m_bFunc = TRUE;
}

CBotProgram*  CBotStack::GivBotCall(BOOL bFirst)
{
    if ( ! bFirst ) return m_prog;
    CBotStack*  p = this;
    while ( p->m_prev != NULL ) p = p->m_prev;
    return p->m_prog;
}

void* CBotStack::GivPUser()
{
    return m_pUser;
}


BOOL CBotStack::ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype)
{
    CBotTypResult       res;

    // cherche d'abord selon l'identificateur

    res = CBotCall::DoCall(nIdent, NULL, ppVar, this, rettype );
    if (res.GivType() >= 0) return res.GivType();

    res = m_prog->GivFunctions()->DoCall(nIdent, NULL, ppVar, this, token );
    if (res.GivType() >= 0) return res.GivType();

    // si pas trouvé (recompilé ?) cherche selon le nom

    nIdent = 0;
    res = CBotCall::DoCall(nIdent, token, ppVar, this, rettype );
    if (res.GivType() >= 0) return res.GivType();

    res = m_prog->GivFunctions()->DoCall(nIdent, token->GivString(), ppVar, this, token );
    if (res.GivType() >= 0) return res.GivType();

    SetError(TX_NOCALL, token);
    return TRUE;
}

void CBotStack::RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar)
{
    if ( m_next == NULL ) return;

    if ( !CBotCall::RestoreCall(nIdent, token, ppVar, this) )
        m_prog->GivFunctions()->RestoreCall(nIdent, token->GivString(), ppVar, this );
}


BOOL SaveVar(FILE* pf, CBotVar* pVar)
{
    while ( TRUE )
    {
        if ( pVar == NULL )
        {
            return WriteWord(pf, 0);                            // met un terminateur
        }

        if ( !pVar->Save0State(pf)) return FALSE;               // entête commune
        if ( !pVar->Save1State(pf) ) return FALSE;              // sauve selon la classe fille

        pVar = pVar->GivNext();
    }
}

void CBotStack::GetRunPos(const char* &FunctionName, int &start, int &end)
{
    CBotProgram*    prog = m_prog;                      // programme courrant

    CBotInstr*      funct = NULL;                       // fonction trouvée
    CBotInstr*      instr = NULL;                       // instruction la plus élevée

    CBotStack*      p = this;

    while (p->m_next != NULL)
    {
        if ( p->m_instr != NULL ) instr = p->m_instr;
        if ( p->m_bFunc == 1 ) funct = p->m_instr;
        if ( p->m_next->m_prog != prog ) break ;

        if (p->m_next2 && p->m_next2->m_state != 0) p = p->m_next2 ;
        else                                        p = p->m_next;
    }

    if ( p->m_instr != NULL ) instr = p->m_instr;
    if ( p->m_bFunc == 1 ) funct = p->m_instr;

    if ( funct == NULL ) return;

    CBotToken* t = funct->GivToken();
    FunctionName = t->GivString();

//  if ( p->m_instr != NULL ) instr = p->m_instr;

    t = instr->GivToken();
    start = t->GivStart();
    end   = t->GivEnd();
}

CBotVar* CBotStack::GivStackVars(const char* &FunctionName, int level)
{
    CBotProgram*    prog = m_prog;                      // programme courrant
    FunctionName    = NULL;

    // remonte la pile dans le module courant
    CBotStack*      p = this;

    while (p->m_next != NULL)
    {
        if ( p->m_next->m_prog != prog ) break ;

        if (p->m_next2 && p->m_next2->m_state != 0) p = p->m_next2 ;
        else                                        p = p->m_next;
    }


    // descend sur les éléments de block
    while ( p != NULL && !p->m_bBlock ) p = p->m_prev;

    while ( p != NULL && level++ < 0 )
    {
        p = p->m_prev;
        while ( p != NULL && !p->m_bBlock ) p = p->m_prev;
    }

    if ( p == NULL ) return NULL;

    // recherche le nom de la fonction courante
    CBotStack* pp = p;
    while ( pp != NULL )
    {
        if ( pp->m_bFunc == 1 ) break;
        pp = pp->m_prev;
    }

    if ( pp == NULL || pp->m_instr == NULL ) return NULL;

    CBotToken* t = pp->m_instr->GivToken();
    FunctionName = t->GivString();

    return p->m_listVar;
}

BOOL CBotStack::SaveState(FILE* pf)
{
    if ( this == NULL )                                 // fin de l'arbre ?
    {
        return WriteWord(pf, 0);                        // met un terminateur
    }

    if ( m_next2 != NULL )
    {
        if (!WriteWord(pf, 2)) return FALSE;                // une marque de poursuite
        if (!m_next2->SaveState(pf)) return FALSE;
    }
    else
    {
        if (!WriteWord(pf, 1)) return FALSE;                // une marque de poursuite
    }
    if (!WriteWord(pf, m_bBlock)) return FALSE;         // est-ce un bloc local
    if (!WriteWord(pf, m_state)) return FALSE;          // dans quel état
    if (!WriteWord(pf, 0)) return FALSE;                // par compatibilité m_bDontDelete
    if (!WriteWord(pf, m_step)) return FALSE;           // dans quel état


    if (!SaveVar(pf, m_var)) return FALSE;          // le résultat courant
    if (!SaveVar(pf, m_listVar)) return FALSE;      // les variables locales

    return m_next->SaveState(pf);                       // enregistre la suite
}


BOOL CBotStack::RestoreState(FILE* pf, CBotStack* &pStack)
{
    WORD    w;

    pStack = NULL;
    if (!ReadWord(pf, w)) return FALSE;
    if ( w == 0 ) return TRUE;

#if STACKMEM
    if ( this == NULL ) pStack = FirstStack();
    else pStack = AddStack();
#else
    pStack = new CBotStack(this);
#endif

    if ( w == 2 )
    {
        if (!pStack->RestoreState(pf, pStack->m_next2)) return FALSE;
    }

    if (!ReadWord(pf, w)) return FALSE;         // est-ce un bloc local
    pStack->m_bBlock = w;

    if (!ReadWord(pf, w)) return FALSE;         // dans quel état j'ère ?
    pStack->SetState((short)w);                 // dans le bon état

    if (!ReadWord(pf, w)) return FALSE;         // dont delete ?
                                                // plus utilisé

    if (!ReadWord(pf, w)) return FALSE;         // pas à pas
    pStack->m_step = w;

    if (!CBotVar::RestoreState(pf, pStack->m_var)) return FALSE;    // la variable temp
    if (!CBotVar::RestoreState(pf, pStack->m_listVar)) return FALSE;// les variables locales

    return pStack->RestoreState(pf, pStack->m_next);
}


BOOL CBotVar::Save0State(FILE* pf)
{
    if (!WriteWord(pf, 100+m_mPrivate))return FALSE;        // variable privée ?
    if (!WriteWord(pf, m_bStatic))return FALSE;             // variable static ?
    if (!WriteWord(pf, m_type.GivType()))return FALSE;      // enregiste le type (toujours non nul)
    if (!WriteWord(pf, m_binit))return FALSE;               // variable définie ?
    return WriteString(pf, m_token->GivString());           // et le nom de la variable
}

BOOL CBotVarInt::Save0State(FILE* pf)
{
    if ( !m_defnum.IsEmpty() )
    {
        if(!WriteWord(pf, 200 )) return FALSE;          // marqueur spécial
        if(!WriteString(pf, m_defnum)) return FALSE;    // nom de la valeur
    }

    return CBotVar::Save0State(pf);
}

BOOL CBotVarInt::Save1State(FILE* pf)
{
    return WriteWord(pf, m_val);                            // la valeur de la variable
}

BOOL CBotVarBoolean::Save1State(FILE* pf)
{
    return WriteWord(pf, m_val);                            // la valeur de la variable
}

BOOL CBotVarFloat::Save1State(FILE* pf)
{
    return WriteFloat(pf, m_val);                           // la valeur de la variable
}

BOOL CBotVarString::Save1State(FILE* pf)
{
    return WriteString(pf, m_val);                          // la valeur de la variable
}



BOOL CBotVarClass::Save1State(FILE* pf)
{
    if ( !WriteType(pf, m_type) ) return FALSE;
    if ( !WriteLong(pf, m_ItemIdent) ) return FALSE;

    return SaveVar(pf, m_pVar);                             // contenu de l'objet
}

BOOL CBotVar::RestoreState(FILE* pf, CBotVar* &pVar)
{
    WORD        w, wi, prv, st;
    float       ww;
    CBotString  name, s;

    delete pVar;

                pVar    = NULL;
    CBotVar*    pNew    = NULL;
    CBotVar*    pPrev   = NULL;

    while ( TRUE )          // recupère toute une liste
    {
        if (!ReadWord(pf, w)) return FALSE;                     // privé ou type ?
        if ( w == 0 ) return TRUE;

        CBotString defnum;
        if ( w == 200 )
        {
            if (!ReadString(pf, defnum)) return FALSE;          // nombre avec un identifiant
            if (!ReadWord(pf, w)) return FALSE;                 // type
        }

        prv = 100; st = 0;
        if ( w >= 100 )
        {
            prv = w;
            if (!ReadWord(pf, st)) return FALSE;                // statique
            if (!ReadWord(pf, w)) return FALSE;                 // type
        }

        if ( w == CBotTypClass ) w = CBotTypIntrinsic;          // forcément intrinsèque

        if (!ReadWord(pf, wi)) return FALSE;                    // init ?

        if (!ReadString(pf, name)) return FALSE;                // nom de la variable

        CBotToken token(name, CBotString());

        switch (w)
        {
        case CBotTypInt:
        case CBotTypBoolean:
            pNew = CBotVar::Create(&token, w);                      // crée une variable
            if (!ReadWord(pf, w)) return FALSE;
            pNew->SetValInt((short)w, defnum);
            break;
        case CBotTypFloat:
            pNew = CBotVar::Create(&token, w);                      // crée une variable
            if (!ReadFloat(pf, ww)) return FALSE;
            pNew->SetValFloat(ww);
            break;
        case CBotTypString:
            pNew = CBotVar::Create(&token, w);                      // crée une variable
            if (!ReadString(pf, s)) return FALSE;
            pNew->SetValString(s);
            break;

        // restitue un objet intrinsic ou un élément d'un array
        case CBotTypIntrinsic:
        case CBotTypArrayBody:
            {
                CBotTypResult   r;
                long            id;
                if (!ReadType(pf, r))  return FALSE;                // type complet
                if (!ReadLong(pf, id) ) return FALSE;

//              if (!ReadString(pf, s)) return FALSE;
                {
                    CBotVar* p = NULL;
                    if ( id ) p = CBotVarClass::Find(id) ;

                    pNew = new CBotVarClass(&token, r);             // crée directement une instance
                                                                    // attention cptuse = 0
                    if ( !RestoreState(pf, ((CBotVarClass*)pNew)->m_pVar)) return FALSE;
                    pNew->SetIdent(id);

                    if ( p != NULL )
                    {
                        delete pNew;
                        pNew = p;           // reprend l'élément connu
                    }
                }
            }
            break;

        case CBotTypPointer:
        case CBotTypNullPointer:
            if (!ReadString(pf, s)) return FALSE;
            {
                pNew = CBotVar::Create(&token, CBotTypResult(w, s));// crée une variable
                CBotVarClass* p = NULL;
                long id;
                ReadLong(pf, id);
//              if ( id ) p = CBotVarClass::Find(id);       // retrouve l'instance ( fait par RestoreInstance )

                // restitue une copie de l'instance d'origine
                CBotVar* pInstance = NULL;
                if ( !CBotVar::RestoreState( pf, pInstance ) ) return FALSE;
                ((CBotVarPointer*)pNew)->SetPointer( pInstance );           // et pointe dessus

//              if ( p != NULL ) ((CBotVarPointer*)pNew)->SetPointer( p );  // plutôt celui-ci !

            }
            break;

        case CBotTypArrayPointer:
            {
                CBotTypResult   r;
                if (!ReadType(pf, r))  return FALSE;

                pNew = CBotVar::Create(&token, r);                      // crée une variable

                // restitue une copie de l'instance d'origine
                CBotVar* pInstance = NULL;
                if ( !CBotVar::RestoreState( pf, pInstance ) ) return FALSE;
                ((CBotVarPointer*)pNew)->SetPointer( pInstance );           // et pointe dessus
            }
            break;
        default:
            ASM_TRAP();
        }

        if ( pPrev != NULL ) pPrev->m_next = pNew;
        if ( pVar == NULL  ) pVar = pNew;

        pNew->m_binit = wi;     //      pNew->SetInit(wi);
        pNew->SetStatic(st);
        pNew->SetPrivate(prv-100);
        pPrev = pNew;
    }
    return TRUE;
}




////////////////////////////////////////////////////////////////////////////
// gestion de la pile à la compilation
////////////////////////////////////////////////////////////////////////////

CBotProgram*    CBotCStack::m_prog    = NULL;           // init la variable statique
int             CBotCStack::m_error   = 0;
int             CBotCStack::m_end     = 0;
CBotTypResult   CBotCStack::m_retTyp  = CBotTypResult(0);
//CBotToken*        CBotCStack::m_retClass= NULL;


CBotCStack::CBotCStack(CBotCStack* ppapa)
{
    m_next = NULL;
    m_prev = ppapa;

    if (ppapa == NULL)
    {
        m_error = 0;
        m_start = 0;
        m_end   = 0;
        m_bBlock = TRUE;
    }
    else
    {
        m_start = ppapa->m_start;
        m_bBlock = FALSE;
    }

    m_listVar = NULL;
    m_var     = NULL;
}

// destructeur
CBotCStack::~CBotCStack()
{
    if (m_next != NULL) delete m_next;
    if (m_prev != NULL) m_prev->m_next = NULL;      // enlève de la chaîne

    delete m_var;
    delete m_listVar;
}

// utilisé uniquement à la compilation
CBotCStack* CBotCStack::TokenStack(CBotToken* pToken, BOOL bBlock)
{
    if (m_next != NULL) return m_next;          // reprise dans une pile existante

    CBotCStack* p = new CBotCStack(this);
    m_next = p;                                 // chaîne l'élément
    p->m_bBlock = bBlock;

    if (pToken != NULL) p->SetStartError(pToken->GivStart());

    return  p;
}


CBotInstr* CBotCStack::Return(CBotInstr* inst, CBotCStack* pfils)
{
    if ( pfils == this ) return inst;

    if (m_var != NULL) delete m_var;            // valeur remplacée ?
    m_var = pfils->m_var;                       // résultat transmis
    pfils->m_var = NULL;                        // ne pas détruire la variable

    if (m_error)
    {
        m_start = pfils->m_start;               // récupère la position de l'erreur
        m_end   = pfils->m_end;
    }

    delete pfils;
    return inst;
}

CBotFunction* CBotCStack::ReturnFunc(CBotFunction* inst, CBotCStack* pfils)
{
    if (m_var != NULL) delete m_var;            // valeur remplacée ?
    m_var = pfils->m_var;                       // résultat transmis
    pfils->m_var = NULL;                        // ne pas détruire la variable

    if (m_error)
    {
        m_start = pfils->m_start;               // récupère la position de l'erreur
        m_end   = pfils->m_end;
    }

    delete pfils;
    return inst;
}

int CBotCStack::GivError(int& start, int& end)
{
    start = m_start;
    end   = m_end;
    return m_error;
}

int CBotCStack::GivError()
{
    return m_error;
}

// type d'instruction sur la pile
CBotTypResult CBotCStack::GivTypResult(int mode)
{
    if (m_var == NULL)
        return CBotTypResult(99);
    return  m_var->GivTypResult(mode);
}

// type d'instruction sur la pile
int CBotCStack::GivType(int mode)
{
    if (m_var == NULL)
        return 99;
    return  m_var->GivType(mode);
}

// pointeur sur la pile est de quelle classe ?
CBotClass* CBotCStack::GivClass()
{
    if ( m_var == NULL )
        return NULL;
    if ( m_var->GivType(1) != CBotTypPointer ) return NULL;

    return m_var->GivClass();
}

// type d'instruction sur la pile
void CBotCStack::SetType(CBotTypResult& type)
{
    if (m_var == NULL) return;
    m_var->SetType( type );
}

// cherche une variable sur la pile
// le token peut être une suite de TokenTypVar (objet d'une classe)
// ou un pointeur dans le source

CBotVar* CBotCStack::FindVar(CBotToken* &pToken)
{
    CBotCStack* p = this;
    CBotString  name = pToken->GivString();

    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (name == pp->GivName())
            {
                return pp;
            }
            pp = pp->m_next;
        }
        p = p->m_prev;
    }
    return NULL;
}

CBotVar* CBotCStack::FindVar(CBotToken& Token)
{
    CBotToken*  pt = &Token;
    return FindVar(pt);
}

CBotVar* CBotCStack::CopyVar(CBotToken& Token)
{
    CBotVar*    pVar = FindVar( Token );

    if ( pVar == NULL) return NULL;

    CBotVar*    pCopy = CBotVar::Create( "", pVar->GivType() );
    pCopy->Copy(pVar);
    return  pCopy;
}

BOOL CBotCStack::IsOk()
{
    return (m_error == 0);
}


void CBotCStack::SetStartError( int pos )
{
    if ( m_error != 0) return;          // ne change pas une erreur déjà existante
    m_start = pos;
}

void CBotCStack::SetError(int n, int pos)
{
    if ( n!= 0 && m_error != 0) return; // ne change pas une erreur déjà existante
    m_error = n;
    m_end   = pos;
}

void CBotCStack::SetError(int n, CBotToken* p)
{
    if (m_error) return;    // ne change pas une erreur déjà existante
    m_error = n;
    m_start = p->GivStart();
    m_end   = p->GivEnd();
}

void CBotCStack::ResetError(int n, int start, int end)
{
    m_error = n;
    m_start = start;
    m_end   = end;
}

BOOL CBotCStack::NextToken(CBotToken* &p)
{
    CBotToken*  pp = p;

    p = p->GivNext();
    if (p!=NULL) return TRUE;

    SetError(TX_ENDOF, pp->GivEnd());
    return FALSE;
}

void CBotCStack::SetBotCall(CBotProgram* p)
{
    m_prog = p;
}

CBotProgram* CBotCStack::GivBotCall()
{
    return m_prog;
}

void CBotCStack::SetRetType(CBotTypResult& type)
{
    m_retTyp = type;
}

CBotTypResult CBotCStack::GivRetType()
{
    return m_retTyp;
}

void CBotCStack::SetVar( CBotVar* var )
{
    if (m_var) delete m_var;    // remplacement d'une variable
    m_var = var;
}

// met sur le stack une copie d'une variable
void CBotCStack::SetCopyVar( CBotVar* var )
{
    if (m_var) delete m_var;    // remplacement d'une variable

    if ( var == NULL ) return;
    m_var = CBotVar::Create("", var->GivTypResult(2));
    m_var->Copy( var );
}

CBotVar* CBotCStack::GivVar()
{
    return m_var;
}

void CBotCStack::AddVar(CBotVar* pVar)
{
    CBotCStack* p = this;

    // revient sur l'élement père
    while (p != NULL && p->m_bBlock == 0) p = p->m_prev;

    if ( p == NULL ) return;

    CBotVar**   pp = &p->m_listVar;
    while ( *pp != NULL ) pp = &(*pp)->m_next;

    *pp = pVar;                 // ajoute à la suite

#ifdef  _DEBUG
    if ( pVar->GivUniqNum() == 0 ) ASM_TRAP();
#endif
}

// test si une variable est déjà définie localement

BOOL CBotCStack::CheckVarLocal(CBotToken* &pToken)
{
    CBotCStack* p = this;
    CBotString  name = pToken->GivString();

    while (p != NULL)
    {
        CBotVar*    pp = p->m_listVar;
        while ( pp != NULL)
        {
            if (name == pp->GivName())
                return TRUE;
            pp = pp->m_next;
        }
        if ( p->m_bBlock ) return FALSE;
        p = p->m_prev;
    }
    return FALSE;
}

CBotTypResult CBotCStack::CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent)
{
    nIdent = 0;
    CBotTypResult val(-1);

    val = CBotCall::CompileCall(p, ppVars, this, nIdent);
    if (val.GivType() < 0)
    {
        val = m_prog->GivFunctions()->CompileCall(p->GivString(), ppVars, nIdent);
        if ( val.GivType() < 0 )
        {
    //      pVar = NULL;                    // l'erreur n'est pas sur un paramètre en particulier
            SetError( -val.GivType(), p );
            val.SetType(-val.GivType());
            return val;
        }
    }
    return val;
}

// test si un nom de procédure est déjà défini quelque part

BOOL CBotCStack::CheckCall(CBotToken* &pToken, CBotDefParam* pParam)
{
    CBotString  name = pToken->GivString();

    if ( CBotCall::CheckCall(name) ) return TRUE;

    CBotFunction*   pp = m_prog->GivFunctions();
    while ( pp != NULL )
    {
        if ( pToken->GivString() == pp->GivName() )
        {
            // les paramètres sont-ils exactement les mêmes ?
            if ( pp->CheckParam( pParam ) )
                return TRUE;
        }
        pp = pp->Next();
    }

    pp = CBotFunction::m_listPublic;
    while ( pp != NULL )
    {
        if ( pToken->GivString() == pp->GivName() )
        {
            // les paramètres sont-ils exactement les mêmes ?
            if ( pp->CheckParam( pParam ) )
                return TRUE;
        }
        pp = pp->m_nextpublic;
    }

    return FALSE;
}

