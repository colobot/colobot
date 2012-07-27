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
// * along with this program. If not, see  http://www.gnu.org/licenses/.////////////////////////////////////////////////////////////////////
/**
 * \file CBot.h
 * \brief Interpreter of the language CBot for COLOBOT game
 */



#include "resource.h"
#include "CBotDll.h"                    // public definitions
#include "CBotToken.h"                  // token management

#define    STACKRUN    true             /// \def return execution directly on a suspended routine
#define    STACKMEM    true             /// \def preserve memory for the execution stack
#define    MAXSTACK    990              /// \def stack size reserved

#define    EOX         (CBotStack*)-1   /// \def tag special condition


// fix for MSVC instruction __asm int 3 (setting a trap)
#if defined(__MINGW32__) || defined(__GNUC__)
#define ASM_TRAP()    asm("int $3");
#else
#define ASM_TRAP()    __asm int 3;
#endif

/////////////////////////////////////////////////////////////////////
// forward declaration

class CBotCompExpr; // an expression like
                    // () <= ()
class CBotAddExpr;  // an expression like
                    // () + ()
class CBotParExpr;  // basic type or instruction in parenthesis
                    // Toto.truc
                    // 12.5
                    // "string"
                    // ( expression )
class CBotExprVar;  // a variable name as
                    // Toto
                    // chose.truc.machin
class CBotWhile;    // while (...) {...};
class CBotIf;       // if (...) {...} else {...}
class CBotDefParam; // paramerer list of a function
class CBotRepeat;   // repeat (nb) {...}



////////////////////////////////////////////////////////////////////////
// Management of the execution stack
////////////////////////////////////////////////////////////////////////

// actually, externally, the only thing he can do
// is to create an instance of a stack
// to use for routine CBotProgram :: Execute (CBotStack)


/**\class CBotStack 
 * \brief Management of the execution stack.
 * \brief Actually the only thing it can do is to create an instance of a stack
 * \brief to use for routine CBotProgram :: Execute(CBotStack)*/
class CBotStack
{
public:
#if    STACKMEM
    /**
     * \brief FirstStack Allocate first stack
     * \return pointer to created stack
     */
    static CBotStack * FirstStack();

    /** \brief Delete Remove current stack */
    void Delete();
#endif

    /**
     * \brief CBotStack Constructor of the stack
     * \param ppapa Not used.
     */
    CBotStack(CBotStack* ppapa);


    /** \brief ~CBotStack Destructor */
    ~CBotStack();

    /**
     * \brief StackOver Check if end of stack is reached
     * \return true if end of stack
     */
    bool StackOver();

    /**
     * \brief GivError Get error number of the stack
     * \param [out] start beginning of the stack
     * \param [out] end end of stack
     * \return error number
     */
    int GivError(int& start, int& end);

    /**
     * \brief GivError Get error number
     * \return eror number
     */
    int GivError();// rend le numéro d'erreur retourné

    /**
     * \brief Reset Reset error at and set user
     * \param [in] pUser User of stack
     */
    void Reset(void* pUser);

    /**
     * \brief SetType Determines the type.
     * \param type Type of instruction on the stack.
     */
    void SetType(CBotTypResult& type);

    /**
     * \brief GivType Get the type of value on the stack.
     * \param [in] mode Used when getting class type (1 gives pointer, 2 gives intrinsic).
     * \return Type number.
     */
    int GivType(int mode = 0);

    /**
     * \brief Gives the type of complete value on the stack.
     * \param [in] mode Used when getting class type (1 gives pointer, 2 gives intrinsic).
     * \return  Type of an element.
     */
    CBotTypResult GivTypResult(int mode = 0);

    /**
     * \brief Adds a local variable.
     * \param [in] p Variable to be added.
     */
    void AddVar(CBotVar* p);

    /**
     * \brief Fetch a variable by its token.
     * \brief This may be a composite variable
     * \param [in] pToken Token upon which search is performed
     * \param [in] bUpdate Not used. Probably need to be removed
     * \param [in] bModif Not used. Probably need to be removed
     * \return Found variable
     */
    CBotVar* FindVar(CBotToken* &pToken, bool bUpdate = false,
                                           bool bModif  = false);

    /**
     * \brief Fetch a variable by its token.
     * \brief This may be a composite variable
     * \param [in] pToken Token upon which search is performed
     * \param [in] bUpdate Not used. Probably need to be removed
     * \param [in] bModif Not used. Probably need to be removed
     * \return Found variable
     */
    CBotVar* FindVar(CBotToken& Token, bool bUpdate = false,
                                              bool bModif  = false);

    /**
     * \brief Fetch variable by its name
     * \param [in] name Name of variable to find
     * \return Found variable
     */
    CBotVar* FindVar(const char* name);

    /**
     * \brief Fetch a variable on the stack according to its identification number
     * \brief This is faster than comparing names
     * \param [in] ident Identifier of a variable
     * \param [in] bUpdate Not used. Probably need to be removed
     * \param [in] bModif Not used. Probably need to be removed
     * \return Found variable
     */
    CBotVar* FindVar(long ident, bool bUpdate = false,
                                        bool bModif  = false);

    /**
     * \brief Find variable by its token and returns a copy of it.
     * \param Token Token upon which search is performed
     * \param bUpdate Not used.
     * \return Found variable, NULL if not found
     */
    CBotVar*        CopyVar(CBotToken& Token, bool bUpdate = false);


    CBotStack*        AddStack(CBotInstr* instr = NULL, bool bBlock = false);    // étend le stack
    CBotStack*        AddStackEOX(CBotCall* instr = NULL, bool bBlock = false);    // étend le stack
    CBotStack*        RestoreStack(CBotInstr* instr = NULL);
    CBotStack*        RestoreStackEOX(CBotCall* instr = NULL);

    CBotStack*        AddStack2(bool bBlock = false);                        // étend le stack
    bool            Return(CBotStack* pFils);                            // transmet le résultat au dessus
    bool            ReturnKeep(CBotStack* pFils);                        // transmet le résultat sans réduire la pile
    bool            BreakReturn(CBotStack* pfils, const char* name = NULL);
                                                                        // en cas de break éventuel
    bool            IfContinue(int state, const char* name);
                                                                        // ou de "continue"
    
    bool            IsOk();

    bool            SetState(int n, int lim = -10);                        // sélectionne un état
    int                GivState();                                            // dans quel état j'ère ?
    bool            IncState(int lim = -10);                            // passe à l'état suivant
    bool            IfStep();                                            // faire du pas à pas ?
    bool            Execute(); 

    void            SetVar( CBotVar* var );
    void            SetCopyVar( CBotVar* var );
    CBotVar*        GivVar();
    CBotVar*        GivCopyVar();
    CBotVar*        GivPtVar();
    bool            GivRetVar(bool bRet);
    long            GivVal();

    void            SetStartError(int pos);
    void            SetError(int n, CBotToken* token = NULL);
    void            SetPosError(CBotToken* token);
    void            ResetError(int n, int start, int end);
    void            SetBreak(int val, const char* name);

    void            SetBotCall(CBotProgram* p);
    CBotProgram*    GivBotCall(bool bFirst = false);
    void*            GivPUser();
    bool            GivBlock();


    bool            ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype);
    void            RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar);

    bool            SaveState(FILE* pf);
    bool            RestoreState(FILE* pf, CBotStack* &pStack);

    static
    void            SetTimer(int n);

    void            GetRunPos(const char* &FunctionName, int &start, int &end);
    CBotVar*        GivStackVars(const char* &FunctionName, int level);

    int                m_temp;

private:
    CBotStack*        m_next;
    CBotStack*        m_next2;
    CBotStack*        m_prev;
    friend class CBotInstArray;

#ifdef    _DEBUG 
    int                m_index;
#endif
    int                m_state;
    int                m_step;
    static int        m_error;
    static int        m_start;
    static int        m_end;
    static
    CBotVar*        m_retvar;                    // résultat d'un return

    CBotVar*        m_var;                        // résultat des opérations
    CBotVar*        m_listVar;                    // les variables déclarées à ce niveau

    bool            m_bBlock;                    // fait partie d'un bloc (variables sont locales à ce bloc)
    bool            m_bOver;                    // limites de la pile ?
//    bool            m_bDontDelete;                // spécial, ne pas détruire les variables au delete
    CBotProgram*    m_prog;                        // les fonctions définies par user

    static
    int                m_initimer;
    static
    int                m_timer;
    static
    CBotString        m_labelBreak;
    static
    void*            m_pUser;

    CBotInstr*        m_instr;                    // l'instruction correspondante
    bool            m_bFunc;                    // une entrée d'une fonction ?
    CBotCall*        m_call;                        // point de reprise dans un call extern
    friend class    CBotTry;
};

// les routines inline doivent être déclarées dans le fichier .h

inline bool CBotStack::IsOk()
{
    return (m_error == 0);
}

inline int CBotStack::GivState()
{
    return m_state;
}

inline int CBotStack::GivError()
{
    return m_error;
}

////////////////////////////////////////////////////////////////////////
// Gestion de la pile de compilation
////////////////////////////////////////////////////////////////////////


class CBotCStack
{
private:
    CBotCStack*        m_next;
    CBotCStack*        m_prev;

    static
    int                m_error;
    static
    int                m_end;
    int                m_start;

    CBotVar*        m_var;                        // résultat des opérations

    bool            m_bBlock;                    // fait partie d'un bloc (variables sont locales à ce bloc)
    CBotVar*        m_listVar;

    static
    CBotProgram*    m_prog;                        // liste des fonctions compilées
    static
    CBotTypResult    m_retTyp;
//    static
//    CBotToken*        m_retClass;

public:
                    CBotCStack(CBotCStack* ppapa);
                    ~CBotCStack();

    bool            IsOk();
    int                GivError();
    int                GivError(int& start, int& end);
                                                // rend le numéro d'erreur retourné

    void            SetType(CBotTypResult& type);// détermine le type
    CBotTypResult    GivTypResult(int mode = 0);    // donne le type de valeur sur le stack
    int                GivType(int mode = 0);        // donne le type de valeur sur le stack
    CBotClass*        GivClass();                    // donne la classe de la valeur sur le stack

    void            AddVar(CBotVar* p);            // ajoute une variable locale
    CBotVar*        FindVar(CBotToken* &p);        // trouve une variable
    CBotVar*        FindVar(CBotToken& Token);
    bool            CheckVarLocal(CBotToken* &pToken);
    CBotVar*        CopyVar(CBotToken& Token);    // trouve et rend une copie

    CBotCStack*        TokenStack(CBotToken* pToken = NULL, bool bBlock = false);
    CBotInstr*        Return(CBotInstr* p, CBotCStack* pParent);    // transmet le résultat au dessus
    CBotFunction*    ReturnFunc(CBotFunction* p, CBotCStack* pParent);    // transmet le résultat au dessus
    
    void            SetVar( CBotVar* var );
    void            SetCopyVar( CBotVar* var );
    CBotVar*        GivVar();

    void            SetStartError(int pos);
    void            SetError(int n, int pos);
    void            SetError(int n, CBotToken* p);
    void            ResetError(int n, int start, int end);

    void            SetRetType(CBotTypResult& type);
    CBotTypResult    GivRetType();

//    void            SetBotCall(CBotFunction* &pFunc);
    void            SetBotCall(CBotProgram* p);
    CBotProgram*    GivBotCall();
    CBotTypResult    CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent);
    bool            CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

    bool            NextToken(CBotToken* &p);
};


extern bool SaveVar(FILE* pf, CBotVar* pVar);


/////////////////////////////////////////////////////////////////////
// classes defining an instruction
class CBotInstr
{
private:
    static
    CBotStringArray
                m_labelLvl;
protected:
    CBotToken    m_token;                // keeps the token
    CBotString    name;                    // debug
    CBotInstr*    m_next;                    // linked command
    CBotInstr*    m_next2b;                // second list definition chain
    CBotInstr*    m_next3;                // third list for indices and fields
    CBotInstr*    m_next3b;                // necessary for reporting tables
/*
    for example, the following program
    int        x[]; x[1] = 4;
    int        y[x[1]][10], z;
    is generated
    CBotInstrArray
    m_next3b-> CBotEmpty
    m_next->
    CBotExpression ....
    m_next->
    CBotInstrArray
    m_next3b-> CBotExpression ('x') ( m_next3-> CBotIndexExpr ('1') )
    m_next3b-> CBotExpression ('10')
    m_next2-> 'z'
    m_next->...

*/

    static
    int                m_LoopLvl;
    friend class    CBotClassInst;
    friend class    CBotInt;
    friend class    CBotListArray;

public:
                CBotInstr();
                virtual
                ~CBotInstr();

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    static
    CBotInstr*    CompileArray(CBotToken* &p, CBotCStack* pStack, CBotTypResult type, bool first = true);

    virtual
    bool        Execute(CBotStack* &pj);
    virtual
    bool        Execute(CBotStack* &pj, CBotVar* pVar);
    virtual
    void        RestoreState(CBotStack* &pj, bool bMain);

    virtual
    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
    virtual
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend);
    virtual
    void        RestoreStateVar(CBotStack* &pile, bool bMain);

    virtual
    bool        CompCase(CBotStack* &pj, int val);

    void        SetToken(CBotToken* p);
    int            GivTokenType();
    CBotToken*    GivToken();

    void        AddNext(CBotInstr* n);
    CBotInstr*    GivNext();
    void        AddNext3(CBotInstr* n);
    CBotInstr*    GivNext3();
    void        AddNext3b(CBotInstr* n);
    CBotInstr*    GivNext3b();

    static
    void        IncLvl(CBotString& label);
    static
    void        IncLvl();
    static
    void        DecLvl();
    static
    bool        ChkLvl(const CBotString& label, int type);

    bool        IsOfClass(CBotString name);
};

class CBotWhile : public CBotInstr
{
private:
    CBotInstr*    m_Condition;        // la condition
    CBotInstr*    m_Block;            // les instructions
    CBotString    m_label;            // une étiquette s'il y a

public:
                CBotWhile();
                ~CBotWhile();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotRepeat : public CBotInstr
{
private:
    /// Number of iterations
    CBotInstr*    m_NbIter;

    /// Instructions
    CBotInstr*    m_Block;

    /// Label
    CBotString    m_label;            // une étiquette s'il y a

public:
    CBotRepeat();
    ~CBotRepeat();

    /// Static method used for compilation
    static CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);

    /// Execute
    bool Execute(CBotStack* &pj);

    /// Restore state
    void RestoreState(CBotStack* &pj, bool bMain);
};

class CBotDo : public CBotInstr
{
private:
    CBotInstr*    m_Block;            // les instructions
    CBotInstr*    m_Condition;        // la condition
    CBotString    m_label;            // une étiquette s'il y a

public:
                CBotDo();
                ~CBotDo();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotFor : public CBotInstr
{
private:
    CBotInstr*    m_Init;                // intruction initiale
    CBotInstr*    m_Test;                // la condition de test
    CBotInstr*    m_Incr;                // instruction pour l'incrément
    CBotInstr*    m_Block;            // les instructions
    CBotString    m_label;            // une étiquette s'il y a

public:
                CBotFor();
                ~CBotFor();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotBreak : public CBotInstr
{
private:
    CBotString    m_label;            // une étiquette s'il y a

public:
                CBotBreak();
                ~CBotBreak();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotReturn : public CBotInstr
{
private:
    CBotInstr*    m_Instr;            // le paramètre à retourner

public:
                CBotReturn();
                ~CBotReturn();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotSwitch : public CBotInstr
{
private:
    CBotInstr*    m_Value;            // value à chercher
    CBotInstr*    m_Block;            // les instructions

public:
                CBotSwitch();
                ~CBotSwitch();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotCase : public CBotInstr
{
private:
    CBotInstr*    m_Value;            // valeur à comparer

public:
                CBotCase();
                ~CBotCase();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
    bool        CompCase(CBotStack* &pj, int val);
};

class CBotCatch : public CBotInstr
{
private:
    CBotInstr*    m_Block;            // les instructions
    CBotInstr*    m_Cond;                // la condition
    CBotCatch*    m_next;                // le catch suivant
    friend class CBotTry;

public:
                CBotCatch();
                ~CBotCatch();
    static
    CBotCatch*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        TestCatch(CBotStack* &pj, int val);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
    void        RestoreCondState(CBotStack* &pj, bool bMain);
};

class CBotTry : public CBotInstr
{
private:
    CBotInstr*    m_Block;            // les instructions
    CBotCatch*    m_ListCatch;        // les catches
    CBotInstr*    m_FinalInst;        // instruction finale

public:
                CBotTry();
                ~CBotTry();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotThrow : public CBotInstr
{
private:
    CBotInstr*    m_Value;            // la valeur à envoyer

public:
                CBotThrow();
                ~CBotThrow();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotStartDebugDD : public CBotInstr
{
private:

public:
                CBotStartDebugDD();
                ~CBotStartDebugDD();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
};


class CBotIf : public CBotInstr
{
private:
    CBotInstr*    m_Condition;        // la condition
    CBotInstr*    m_Block;            // les instructions
    CBotInstr*    m_BlockElse;        // les instructions

public:
                CBotIf();
                ~CBotIf();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


// définition d'un nombre entier

class CBotInt : public CBotInstr
{
private:
    CBotInstr*    m_var;                // la variable à initialiser
    CBotInstr*    m_expr;                // la valeur à mettre, s'il y a
///    CBotInstr*    m_next;                // plusieurs définitions enchaînées

public:
                CBotInt();
                ~CBotInt();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip = false);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

// définition d'un tableau

class CBotInstArray : public CBotInstr
{
private:
    CBotInstr*    m_var;                // la variable à initialiser
    CBotInstr*    m_listass;            // liste d'assignations pour le tableau
    CBotTypResult
                m_typevar;            // type d'éléments
//    CBotString    m_ClassName;

public:
                CBotInstArray();
                ~CBotInstArray();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


// définition d'une liste d'assignation pour un tableau
// int [ ] a [ ] = ( ( 1, 2, 3 ) , ( 3, 2, 1 ) ) ;

class CBotListArray : public CBotInstr
{
private:
    CBotInstr*    m_expr;                // expression pour un élément
                                    // les autres sont chaînés avec CBotInstr::m_next3;
public:
                CBotListArray();
                ~CBotListArray();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type);
    bool        Execute(CBotStack* &pj, CBotVar* pVar);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotEmpty : public CBotInstr
{
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

// définition d'un booléen

class CBotBoolean : public CBotInstr
{
private:
    CBotInstr*    m_var;                // la variable à initialiser
    CBotInstr*    m_expr;                // la valeur à mettre, s'il y a

public:
                CBotBoolean();
                ~CBotBoolean();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip=false);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


// définition d'un nombre réel

class CBotFloat : public CBotInstr
{
private:
    CBotInstr*    m_var;                // la variable à initialiser
    CBotInstr*    m_expr;                // la valeur à mettre, s'il y a

public:
                CBotFloat();
                ~CBotFloat();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip=false);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

// définition d'un elément string

class CBotIString : public CBotInstr
{
private:
    CBotInstr*    m_var;                // la variable à initialiser
    CBotInstr*    m_expr;                // la valeur à mettre, s'il y a

public:
                CBotIString();
                ~CBotIString();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool cont = false, bool noskip=false);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

// définition d'un elément dans une classe quelconque

class CBotClassInst : public CBotInstr
{
private:
    CBotInstr*    m_var;                // la variable à initialiser
    CBotClass*    m_pClass;            // référence à la classe
    CBotInstr*    m_Parameters;        // les paramètres à évaluer pour le constructeur
    CBotInstr*    m_expr;                // la valeur à mettre, s'il y a
    bool        m_hasParams;        // il y a des paramètres ?
    long        m_nMethodeIdent;

public:
                CBotClassInst();
                ~CBotClassInst();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotClass* pClass = NULL);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotCondition : public CBotInstr
{
private:

public:

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
};


// left opérande
// n'accepte que les expressions pouvant être à gauche d'une assignation

class CBotLeftExpr : public CBotInstr
{
private:
    long        m_nIdent;

public:
                CBotLeftExpr();
                ~CBotLeftExpr();
    static
    CBotLeftExpr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack, CBotStack* array);

    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep);
    void        RestoreStateVar(CBotStack* &pile, bool bMain);
};


// gestion des champs d'une instance

class CBotFieldExpr : public CBotInstr
{
private:
    friend class CBotExpression;
    int            m_nIdent;

public:
                CBotFieldExpr();
                ~CBotFieldExpr();
    void        SetUniqNum(int num);
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend);
    void        RestoreStateVar(CBotStack* &pj, bool bMain);
};

// gestion des index dans les tableaux

class CBotIndexExpr : public CBotInstr
{
private:
    CBotInstr*     m_expr;                    // expression pour le calcul de l'index
    friend class CBotLeftExpr;
    friend class CBotExprVar;

public:
                CBotIndexExpr();
                ~CBotIndexExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend);
    void        RestoreStateVar(CBotStack* &pj, bool bMain);
};

// une expression du genre
// x = a;
// x * y + 3;

class CBotExpression : public CBotInstr
{
private:
    CBotLeftExpr*    m_leftop;            // élément de gauche
    CBotInstr*        m_rightop;            // élément de droite

public:
                CBotExpression();
                ~CBotExpression();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotListExpression : public CBotInstr
{
private:
    CBotInstr*    m_Expr;                // la 1ère expression à évaluer

public:
                CBotListExpression();
                ~CBotListExpression();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotLogicExpr : public CBotInstr
{
private:
    CBotInstr*    m_condition;        // test à évaluer
    CBotInstr*    m_op1;                // élément de gauche
    CBotInstr*    m_op2;                // élément de droite
    friend class CBotTwoOpExpr;

public:
                CBotLogicExpr();
                ~CBotLogicExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack);
    void        RestoreState(CBotStack* &pj, bool bMain);
};



class CBotBoolExpr : public CBotInstr
{
private:

public:
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
};



// une expression éventuellement entre parenthèses ( ... )
// il n'y a jamais d'instance de cette classe
// l'objet retourné étant le contenu de la parenthése
class CBotParExpr : public CBotInstr
{
private:

public:
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
};

// expression unaire 
class CBotExprUnaire : public CBotInstr
{
private:
    CBotInstr*    m_Expr;                // l'expression à évaluer
public:
                CBotExprUnaire();
                ~CBotExprUnaire();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pStack);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

// toutes les opérations à 2 opérandes

class CBotTwoOpExpr : public CBotInstr
{
private:
    CBotInstr*    m_leftop;            // élément de gauche
    CBotInstr*    m_rightop;            // élément de droite
public:
                CBotTwoOpExpr();
                ~CBotTwoOpExpr();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations = NULL);
    bool        Execute(CBotStack* &pStack);
    void        RestoreState(CBotStack* &pj, bool bMain);
};




// un bloc d'instructions { .... }
class CBotBlock : public CBotInstr
{
private:

public:
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal = true);
    static
    CBotInstr*    CompileBlkOrInst(CBotToken* &p, CBotCStack* pStack, bool bLocal = false);
};


// le contenu d'un bloc d'instructions ... ; ... ; ... ; ... ;
class CBotListInstr : public CBotInstr
{
private:
    CBotInstr*    m_Instr;            // les instructions à faire

public:
                CBotListInstr();
                ~CBotListInstr();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal = true);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotInstrCall : public CBotInstr
{
private:
    CBotInstr*    m_Parameters;        // les paramètres à évaluer
//    int            m_typeRes;            // type du résultat
//    CBotString    m_RetClassName;        // class du résultat
    CBotTypResult
                m_typRes;            // type complet du résultat
    long        m_nFuncIdent;        // id de la fonction

public:
                CBotInstrCall();
                ~CBotInstrCall();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

// un appel d'une méthode

class CBotInstrMethode : public CBotInstr
{
private:
    CBotInstr*    m_Parameters;        // les paramètres à évaluer
//    int            m_typeRes;            // type du résultat
//    CBotString    m_RetClassName;        // class du résultat
    CBotTypResult
                m_typRes;            // type complet du résultat

    CBotString    m_NomMethod;        // nom de la méthode
    long        m_MethodeIdent;        // identificateur de la méthode
//    long        m_nThisIdent;        // identificateur pour "this"
    CBotString    m_ClassName;        // nom de la classe

public:
                CBotInstrMethode();
                ~CBotInstrMethode();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* pVar);
    bool        Execute(CBotStack* &pj);
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep, bool bExtend);
    void        RestoreStateVar(CBotStack* &pj, bool bMain);
};

// expression for the variable name

class CBotExprVar : public CBotInstr
{
private:
    long        m_nIdent;
    friend class CBotPostIncExpr;
    friend class CBotPreIncExpr;

public:
                CBotExprVar();
                ~CBotExprVar();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack, int privat=PR_PROTECT);
    static
    CBotInstr*    CompileMethode(CBotToken* &p, CBotCStack* pStack);

    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
    bool        ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep);
    bool        Execute2Var(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep);
    void        RestoreStateVar(CBotStack* &pj, bool bMain);
};

class CBotPostIncExpr : public CBotInstr
{
private:
    CBotInstr*    m_Instr;
    friend class CBotParExpr;

public:
                CBotPostIncExpr();
                ~CBotPostIncExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotPreIncExpr : public CBotInstr
{
private:
    CBotInstr*    m_Instr;
    friend class CBotParExpr;

public:
                CBotPreIncExpr();
                ~CBotPreIncExpr();
//    static
//    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotLeftExprVar : public CBotInstr
{
private:
public:
    CBotTypResult
                m_typevar;            // type de variable déclarée
    long        m_nIdent;            // identificateur unique pour cette variable

public:
                CBotLeftExprVar();
                ~CBotLeftExprVar();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotExprBool : public CBotInstr
{
private:

public:
                CBotExprBool();
                ~CBotExprBool();

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


class CBotExprNull : public CBotInstr
{
private:

public:
                CBotExprNull();
                ~CBotExprNull();

    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotExprNan : public CBotInstr
{
private:

public:
                CBotExprNan();
                ~CBotExprNan();

    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

class CBotNew : public CBotInstr
{
private:
    CBotInstr*    m_Parameters;        // les paramètres à évaluer
    long        m_nMethodeIdent;
//    long        m_nThisIdent;
    CBotToken    m_vartoken;

public:
                CBotNew();
                ~CBotNew();

    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};

// expression représentant un nombre

class CBotExprNum : public CBotInstr
{
private:
    int            m_numtype;                    // et le type de nombre
    long        m_valint;                    // valeur pour un int
    float        m_valfloat;                    // valeur pour un float

public:
                CBotExprNum();
                ~CBotExprNum();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};



// expression représentant une chaine de caractères

class CBotExprAlpha : public CBotInstr
{
private:

public:
                CBotExprAlpha();
                ~CBotExprAlpha();
    static
    CBotInstr*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool        Execute(CBotStack* &pj);
    void        RestoreState(CBotStack* &pj, bool bMain);
};


#define    MAX(a,b)    ((a>b) ? a : b)


// classe pour la gestion des nombres entier (int)
class CBotVarInt : public CBotVar
{
private:
    int            m_val;            // la valeur
    CBotString    m_defnum;        // le nom si donné par DefineNum
    friend class CBotVar;

public:
                CBotVarInt( const CBotToken* name );
//                ~CBotVarInt();

    void        SetValInt(int val, const char* s = NULL);
    void        SetValFloat(float val);
    int            GivValInt();
    float        GivValFloat();
    CBotString    GivValString();

    void        Copy(CBotVar* pSrc, bool bName=true);


    void        Add(CBotVar* left, CBotVar* right);    // addition
    void        Sub(CBotVar* left, CBotVar* right);    // soustraction
    void        Mul(CBotVar* left, CBotVar* right);    // multiplication
    int            Div(CBotVar* left, CBotVar* right);    // division
    int            Modulo(CBotVar* left, CBotVar* right);    // reste de division
    void        Power(CBotVar* left, CBotVar* right);    // puissance

    bool        Lo(CBotVar* left, CBotVar* right);
    bool        Hi(CBotVar* left, CBotVar* right);
    bool        Ls(CBotVar* left, CBotVar* right);
    bool        Hs(CBotVar* left, CBotVar* right);
    bool        Eq(CBotVar* left, CBotVar* right);
    bool        Ne(CBotVar* left, CBotVar* right);

    void        XOr(CBotVar* left, CBotVar* right);
    void        Or(CBotVar* left, CBotVar* right);
    void        And(CBotVar* left, CBotVar* right);

    void        SL(CBotVar* left, CBotVar* right);
    void        SR(CBotVar* left, CBotVar* right);
    void        ASR(CBotVar* left, CBotVar* right);

    void        Neg();
    void        Not();
    void        Inc();
    void        Dec();

    bool        Save0State(FILE* pf);
    bool        Save1State(FILE* pf);

};

// classe pour la gestion des nombres réels (float)
class CBotVarFloat : public CBotVar
{
private:
    float        m_val;        // la valeur

public:
                CBotVarFloat( const CBotToken* name );
//                ~CBotVarFloat();

    void        SetValInt(int val, const char* s = NULL);
    void        SetValFloat(float val);
    int            GivValInt();
    float        GivValFloat();
    CBotString    GivValString();

    void        Copy(CBotVar* pSrc, bool bName=true);


    void        Add(CBotVar* left, CBotVar* right);    // addition
    void        Sub(CBotVar* left, CBotVar* right);    // soustraction
    void        Mul(CBotVar* left, CBotVar* right);    // multiplication
    int         Div(CBotVar* left, CBotVar* right);    // division
    int            Modulo(CBotVar* left, CBotVar* right);    // reste de division
    void        Power(CBotVar* left, CBotVar* right);    // puissance

    bool        Lo(CBotVar* left, CBotVar* right);
    bool        Hi(CBotVar* left, CBotVar* right);
    bool        Ls(CBotVar* left, CBotVar* right);
    bool        Hs(CBotVar* left, CBotVar* right);
    bool        Eq(CBotVar* left, CBotVar* right);
    bool        Ne(CBotVar* left, CBotVar* right);

    void        Neg();
    void        Inc();
    void        Dec();

    bool        Save1State(FILE* pf);
};


// classe pour la gestion des chaînes (String)
class CBotVarString : public CBotVar
{
private:
    CBotString    m_val;        // la valeur

public:
                CBotVarString( const CBotToken* name );
//                ~CBotVarString();

    void        SetValString(const char* p);
    CBotString    GivValString();

    void        Copy(CBotVar* pSrc, bool bName=true);

    void        Add(CBotVar* left, CBotVar* right);    // addition

    bool        Lo(CBotVar* left, CBotVar* right);
    bool        Hi(CBotVar* left, CBotVar* right);
    bool        Ls(CBotVar* left, CBotVar* right);
    bool        Hs(CBotVar* left, CBotVar* right);
    bool        Eq(CBotVar* left, CBotVar* right);
    bool        Ne(CBotVar* left, CBotVar* right);

    bool        Save1State(FILE* pf);
};

// classe pour la gestion des boolean
class CBotVarBoolean : public CBotVar
{
private:
    bool        m_val;        // la valeur

public:
                CBotVarBoolean( const CBotToken* name );
//                ~CBotVarBoolean();

    void        SetValInt(int val, const char* s = NULL);
    void        SetValFloat(float val);
    int            GivValInt();
    float        GivValFloat();
    CBotString    GivValString();

    void        Copy(CBotVar* pSrc, bool bName=true);

    void        And(CBotVar* left, CBotVar* right);
    void        Or(CBotVar* left, CBotVar* right);
    void        XOr(CBotVar* left, CBotVar* right);
    void        Not();
    bool        Eq(CBotVar* left, CBotVar* right);
    bool        Ne(CBotVar* left, CBotVar* right);

    bool        Save1State(FILE* pf);
};


// classe pour la gestion des instances de classe
class CBotVarClass : public CBotVar
{
private:
    static
    CBotVarClass*    m_ExClass;        // liste des instances existantes à un moment donné
    CBotVarClass*    m_ExNext;        // pour cette liste générale
    CBotVarClass*    m_ExPrev;        // pour cette liste générale

private:
    CBotClass*        m_pClass;        // la définition de la classe
    CBotVarClass*    m_pParent;        // l'instance dans la classe parent
    CBotVar*        m_pVar;            // contenu
    friend class    CBotVar;        // mon papa est un copain
    friend class    CBotVarPointer;    // et le pointeur aussi
    int                m_CptUse;        // compteur d'utilisation
    long            m_ItemIdent;    // identificateur (unique) de l'instance
    bool            m_bConstructor;    // set si un constructeur a été appelé

public:
                CBotVarClass( const CBotToken* name, const CBotTypResult& type );
//                CBotVarClass( const CBotToken* name, CBotTypResult& type, int &nIdent );
                ~CBotVarClass();
//    void        InitCBotVarClass( const CBotToken* name, CBotTypResult& type, int &nIdent );

    void        Copy(CBotVar* pSrc, bool bName=true);
    void        SetClass(CBotClass* pClass); //, int &nIdent);
    CBotClass*    GivClass();
    CBotVar*    GivItem(const char* name);    // rend un élément d'une classe selon son nom (*)
    CBotVar*    GivItemRef(int nIdent);

    CBotVar*    GivItem(int n, bool bExtend);
    CBotVar*    GivItemList();

    CBotString    GivValString();

    bool        Save1State(FILE* pf);
    void        Maj(void* pUser, bool bContinue);

    void        IncrementUse();                // une référence en plus
    void        DecrementUse();                // une référence en moins

    CBotVarClass* 
                GivPointer();
    void        SetItemList(CBotVar* pVar);

    void        SetIdent(long n);
    
    static CBotVarClass* Find(long id);


//    CBotVar*    GivMyThis();

    bool        Eq(CBotVar* left, CBotVar* right);
    bool        Ne(CBotVar* left, CBotVar* right);

    void        ConstructorSet();
};


// classe pour la gestion des pointeurs à une instances de classe
class CBotVarPointer : public CBotVar
{
private:
    CBotVarClass*
                m_pVarClass;        // contenu
    CBotClass*    m_pClass;            // la classe prévue pour ce pointeur
    friend class CBotVar;            // mon papa est un copain

public:
                CBotVarPointer( const CBotToken* name, CBotTypResult& type );
                ~CBotVarPointer();

    void        Copy(CBotVar* pSrc, bool bName=true);
    void        SetClass(CBotClass* pClass);
    CBotClass*    GivClass();
    CBotVar*    GivItem(const char* name);    // rend un élément d'une classe selon son nom (*)
    CBotVar*    GivItemRef(int nIdent);
    CBotVar*    GivItemList();

    CBotString    GivValString();
    void        SetPointer(CBotVar* p);
    CBotVarClass*
                GivPointer();

    void        SetIdent(long n);            // associe un numéro d'identification (unique)
    long        GivIdent();                    // donne le numéro d'identification associé
    void        ConstructorSet();

    bool        Save1State(FILE* pf);
    void        Maj(void* pUser, bool bContinue);

    bool        Eq(CBotVar* left, CBotVar* right);
    bool        Ne(CBotVar* left, CBotVar* right);
};


// classe pour les tableaux

#define    MAXARRAYSIZE    9999

class CBotVarArray : public CBotVar
{
private:
    CBotVarClass*
                m_pInstance;                // instance gérant le tableau

    friend class CBotVar;                    // papa est un copain

public:
                CBotVarArray( const CBotToken* name, CBotTypResult& type );
                ~CBotVarArray();

    void        SetPointer(CBotVar* p);
    CBotVarClass*
                GivPointer();
    
    void        Copy(CBotVar* pSrc, bool bName=true);
    CBotVar*    GivItem(int n, bool bGrow=false);    // rend un élément selon son index numérique
                                                // agrandi le tableau si nécessaire si bExtend
//    CBotVar*    GivItem(const char* name);        // rend un élément selon son index litéral
    CBotVar*    GivItemList();                    // donne le premier élément de la liste

    CBotString    GivValString();                    // donne le contenu du tableau dans une chaîne

    bool        Save1State(FILE* pf);
};


extern CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars);

extern bool TypeCompatible( CBotTypResult& type1, CBotTypResult& type2, int op = 0 );
extern bool TypesCompatibles( const CBotTypResult& type1, const CBotTypResult& type2 );

extern bool WriteWord(FILE* pf, unsigned short w);
extern bool ReadWord(FILE* pf, unsigned short& w);
extern bool ReadLong(FILE* pf, long& w);
extern bool WriteFloat(FILE* pf, float w);
extern bool WriteLong(FILE* pf, long w);
extern bool ReadFloat(FILE* pf, float& w);
extern bool WriteString(FILE* pf, CBotString s);
extern bool ReadString(FILE* pf, CBotString& s);
extern bool WriteType(FILE* pf, CBotTypResult type);
extern bool ReadType(FILE* pf, CBotTypResult& type);

extern float GivNumFloat( const char* p );

#if    false
extern void DEBUG( const char* text, int val, CBotStack* pile );
#endif

///////////////////////////////////////////
// classe pour les appels de routines (externes)

class CBotCall
{
private:
    static
    CBotCall*    m_ListCalls;
    static
    void*        m_pUser;
    long        m_nFuncIdent;

private:
    CBotString    m_name;
    bool        (*m_rExec) (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser) ;
    CBotTypResult
                (*m_rComp) (CBotVar* &pVar, void* pUser)    ;
    CBotCall*    m_next;

public:
                CBotCall(const char* name, 
                         bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser), 
                         CBotTypResult rCompile (CBotVar* &pVar, void* pUser));
                ~CBotCall();

    static
    bool        AddFunction(const char* name, 
                            bool rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser), 
                            CBotTypResult rCompile (CBotVar* &pVar, void* pUser));

    static
    CBotTypResult
                CompileCall(CBotToken* &p, CBotVar** ppVars, CBotCStack* pStack, long& nIdent);
    static
    bool        CheckCall(const char* name);

//    static
//    int            DoCall(CBotToken* &p, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);
    static
    int            DoCall(long& nIdent, CBotToken* token, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);
#if    STACKRUN
    bool        Run(CBotStack* pStack);
    static
    bool        RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack);
#endif

    CBotString    GivName();
    CBotCall*    Next();
    
    static void    SetPUser(void* pUser);
    static void    Free();
};

// classe gérant les méthodes déclarées par AddFunction sur une classe

class CBotCallMethode
{
private:
    CBotString    m_name;
    bool        (*m_rExec) (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception);
    CBotTypResult
                (*m_rComp) (CBotVar* pThis, CBotVar* &pVar);
    CBotCallMethode*    m_next;
    friend class CBotClass;
    long        m_nFuncIdent;

public:
                CBotCallMethode(const char* name, 
                         bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception), 
                         CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar));
                ~CBotCallMethode();

    CBotTypResult
                CompileCall(const char* name, CBotVar* pThis, 
                            CBotVar** ppVars, CBotCStack* pStack,
                            long& nIdent);

    int            DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotVar* &pResult, CBotStack* pStack, CBotToken* pFunc);

    CBotString    GivName();
    CBotCallMethode*    Next();
    void        AddNext(CBotCallMethode* p);
    
};

// une liste de paramètres

class CBotDefParam
{
private:
    CBotToken        m_token;        // nom du paramètre
    CBotString        m_typename;        // nom du type
    CBotTypResult    m_type;            // type de paramètre
    CBotDefParam*    m_next;            // paramètre suivant
    long            m_nIdent;

public:
                    CBotDefParam();
                    ~CBotDefParam();
    static
    CBotDefParam*    Compile(CBotToken* &p, CBotCStack* pStack);
    bool            Execute(CBotVar** ppVars, CBotStack* &pj);
    void            RestoreState(CBotStack* &pj, bool bMain);

    void            AddNext(CBotDefParam* p);
    int                GivType();
    CBotTypResult    GivTypResult();
    CBotDefParam*    GivNext();

    CBotString        GivParamString();
};


// une déclaration de fonction

class CBotFunction : CBotInstr
{
private:
    // gestion d'une liste (static) de fonctions publiques
    static
    CBotFunction*    m_listPublic;
    CBotFunction*    m_nextpublic;
    CBotFunction*    m_prevpublic;
    friend class    CBotCStack;
//    long            m_nThisIdent;
    long            m_nFuncIdent;
    bool            m_bSynchro;        // méthode synchronisée ?

private:
    CBotDefParam*    m_Param;        // liste des paramètres
    CBotInstr*        m_Block;        // le bloc d'instructions
    CBotFunction*    m_next;
    CBotToken        m_retToken;        // si retourne un CBotTypClass
    CBotTypResult    m_retTyp;        // type complet du résultat

    bool            m_bPublic;        // fonction publique
    bool            m_bExtern;        // fonction extern
    CBotString        m_MasterClass;    // nom de la classe qu'on dérive
    CBotProgram*    m_pProg;
    friend class CBotProgram;
    friend class CBotClass;

    CBotToken        m_extern;        // pour la position du mot "extern"
    CBotToken        m_openpar;
    CBotToken        m_closepar;
    CBotToken        m_openblk;
    CBotToken        m_closeblk;
public:
                    CBotFunction();
                    ~CBotFunction();
    static
    CBotFunction*    Compile(CBotToken* &p, CBotCStack* pStack, CBotFunction* pFunc, bool bLocal = true);
    static
    CBotFunction*    Compile1(CBotToken* &p, CBotCStack* pStack, CBotClass*    pClass);
    bool            Execute(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance = NULL);
    void            RestoreState(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance = NULL);

    void            AddNext(CBotFunction* p);
    CBotTypResult    CompileCall(const char* name, CBotVar** ppVars, long& nIdent);
    CBotFunction*    FindLocalOrPublic(long& nIdent, const char* name, CBotVar** ppVars, CBotTypResult& TypeOrError, bool bPublic = true);

    int                DoCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken);
    void            RestoreCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack);
    int                DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass);
    void            RestoreCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass);
    bool            CheckParam(CBotDefParam* pParam);

    static
    void            AddPublic(CBotFunction* pfunc);

    CBotString        GivName();
    CBotString        GivParams();
    bool            IsPublic();
    bool            IsExtern();
    CBotFunction*    Next();

    bool            GetPosition(int& start, int& stop, CBotGet modestart, CBotGet modestop);
};


