/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

// Modules inlcude
#include "../CBotDefines.h"

#include "../CBotString.h"

// Local include

// Global include

// Forward declaration
class CBotVarClass;
class CBotInstr;

/*!
 * \brief The CBotVar class Class for managing variables. May be useful to the
 * outside of the module ( it is currently not expected to be able to create
 * these objects in outer ). It never creates an instance of the class mother
 * CBotVar.
 */
class CBotVar
{
public:
    /*!
     * \brief The InitType enum Results of GetInit().
     */
    enum class InitType : int
    {
        UNDEF = 0,
        DEF = 1,
        IS_POINTER = 2,
        IS_NAN = 999
    };

    /*!
     * \brief CBotVar
     */
    CBotVar();

    /*!
     * \brief ~CBotVar Destructor.
     */
    virtual ~CBotVar( );

    /*!
     * \brief Create Creates from a complete type.
     * \param name
     * \param type
     * \return
     */
    static CBotVar* Create( const char* name, CBotTypResult type);

    /*!
     * \brief Create Creates from one instance of a known class.
     * \param name
     * \param pClass
     * \return
     */
    static CBotVar* Create( const char* name, CBotClass* pClass);

    /*!
     * \brief Create Creates a variable depending on its type.
     * \param name
     * \param type
     * \return
     */
    static CBotVar* Create( const CBotToken* name, int type );

    /*!
     * \brief Create
     * \param name
     * \param type
     * \return
     */
    static CBotVar* Create( const CBotToken* name, CBotTypResult type );

    /*!
     * \brief Create
     * \param name
     * \param type
     * \param pClass
     * \return
     */
    static CBotVar* Create( const char* name, int type, CBotClass* pClass);

    /*!
     * \brief Create
     * \param pVar
     * \return
     */
    static CBotVar* Create( CBotVar* pVar );

    /*!
     * \brief Destroy
     * \param var
     */
    static void Destroy(CBotVar* var);

    /*!
     * \brief SetUserPtr Associate a user pointer to an instance.
     * \param pUser
     */
    void SetUserPtr(void* pUser);

    /*!
     * \brief SetIdent Associates a unique identifier to an instance
     * ( it is used to ensure that the id is unique)
     * \param UniqId
     */
    virtual void SetIdent(long UniqId);

    /*!
     * \brief GetUserPtr Makes the pointer associated with the variable.
     * \return
     */
    void* GetUserPtr();

    /*!
     * \brief GetName The name of the variable, if known.
     * \return
     */
    CBotString GetName();

    /*!
     * \brief SetName Changes the name of the variable
     * \param name
     */
    void SetName(const char* name);

    /*!
     * \brief GetType Returns the base type (int) of the variable
     * \param mode
     * \return
     * \todo Check it?
     */
    int GetType(int mode = 0);

    /*!
     * \brief GetTypResult Returns the complete type of the variable.
     * \param mode
     * \return
     */
    CBotTypResult GetTypResult(int mode = 0);

    /*!
     * \brief GetToken
     * \return
     */
    CBotToken* GetToken();

    /*!
     * \brief SetType
     * \param type
     */
    void SetType(CBotTypResult& type);

    /*!
     * \brief SetInit Is the variable in the state UNDEF, DEF, NAN.
     * \param initType
     */
    void SetInit(InitType initType);

    /*!
     * \brief GetInit Gives the state of the variable.
     * \return
     */
    InitType GetInit() const;

    /*!
     * \brief IsUndefined
     * \return
     */
    bool IsUndefined() const { return GetInit() == InitType::UNDEF; }

    /*!
     * \brief IsDefined
     * \return
     */
    bool IsDefined() const { return GetInit() == InitType::DEF; }

    /*!
     * \brief IsNAN
     * \return
     */
    bool IsNAN() const { return GetInit() == InitType::IS_NAN; }

    /*!
     * \brief SetStatic
     * \param bStatic
     */
    void SetStatic(bool bStatic);

    /*!
     * \brief IsStatic
     * \return
     */
    bool IsStatic();

    /*!
     * \brief SetPrivate
     * \param mPrivate
     */
    void SetPrivate(int mPrivate);

    /*!
     * \brief IsPrivate
     * \param mode
     * \return
     */
    bool IsPrivate(int mode = PR_PROTECT);

    /*!
     * \brief GetPrivate
     * \return
     */
    int GetPrivate();

    /*!
     * \brief ConstructorSet
     */
    virtual void ConstructorSet();

    /*!
     * \brief SetVal Set the value.
     * \param var
     */
    void SetVal(CBotVar* var);

    /*!
     * \brief GetItem Returns an element of a class according to its name (*).
     * \param name
     * \return
     */
    virtual CBotVar* GetItem(const char* name);

    /*!
     * \brief GetItemRef
     * \param nIdent
     * \return
     */
    virtual CBotVar* GetItemRef(int nIdent);

    /*!
     * \brief GetItem
     * \param row
     * \param bGrow
     * \return
     */
    virtual CBotVar* GetItem(int row, bool bGrow = false);

    /*!
     * \brief GetItemList Lists the elements.
     * \return
     */
    virtual CBotVar* GetItemList();

    /*!
     * \brief GetStaticVar Makes the pointer to the variable if it is static.
     * \return
     */
    CBotVar* GetStaticVar();

    /*!
     * \brief IsElemOfClass Check if a variable belongs to a given class said if
     * the element belongs to the class "name" makes true if the object is a
     * subclass.
     * \param name
     * \return
     */
    bool IsElemOfClass(const char* name);

    /*!
     * \brief GetNext Next variable in the list (parameters).
     * \return
     */
    CBotVar* GetNext();

    /*!
     * \brief AddNext Added to a list.
     * \param pVar
     */
    void AddNext(CBotVar* pVar);

    /*!
     * \brief Copy Makes a copy of the variable.
     * \param pSrc
     * \param bName
     */
    virtual void Copy(CBotVar* pSrc, bool bName = true);

    /*!
     * \brief SetValInt Initialized with an integer value (#)
     * \param val
     * \param name
     */
    virtual void SetValInt(int val, const char* name = nullptr);

    /*!
     * \brief SetValFloat Initialized with a real value (#).
     * \param val
     */
    virtual void SetValFloat(float val);

    /*!
     * \brief SetValString Initialized with a string value (#).
     * \param p
     */
    virtual void SetValString(const char* p);

    /*!
     * \brief GetValInt Request the full value (#).
     * \return
     */
    virtual int GetValInt();

    /*!
     * \brief GetValFloat Gets real value (#).
     * \return
     */
    virtual float GetValFloat();

    /*!
     * \brief GetValString Request the string value (#).
     * \return
     */
    virtual CBotString GetValString();

    /*!
     * \brief SetClass
     * \param pClass
     */
    virtual void SetClass(CBotClass* pClass);

    /*!
     * \brief GetClass
     * \return
     */
    virtual CBotClass* GetClass();

    /*!
     * \brief SetPointer
     * \param p
     */
    virtual void SetPointer(CBotVar* p);

    /*!
     * \brief GetPointer
     * \return
     */
    virtual CBotVarClass* GetPointer();

    /*!
     * \brief Add Addition
     * \param left
     * \param right
     */
    virtual void Add(CBotVar* left, CBotVar* right);

    /*!
     * \brief Sub Subtraction
     * \param left
     * \param right
     */
    virtual void Sub(CBotVar* left, CBotVar* right);

    /*!
     * \brief Mul Multiplication
     * \param left
     * \param right
     */
    virtual void Mul(CBotVar* left, CBotVar* right);

    /*!
     * \brief Div Division
     * \param left
     * \param right
     * \return
     */
    virtual int Div(CBotVar* left, CBotVar* right);

    /*!
     * \brief Modulo Remainder of division
     * \param left
     * \param right
     * \return
     */
    virtual int Modulo(CBotVar* left, CBotVar* right);

    /*!
     * \brief Power
     * \param left
     * \param right
     */
    virtual void Power(CBotVar* left, CBotVar* right);

    /*!
     * \brief Lo
     * \param left
     * \param right
     * \return
     */
    virtual bool Lo(CBotVar* left, CBotVar* right);

    /*!
     * \brief Hi
     * \param left
     * \param right
     * \return
     */
    virtual bool Hi(CBotVar* left, CBotVar* right);

    /*!
     * \brief Ls
     * \param left
     * \param right
     * \return
     */
    virtual bool Ls(CBotVar* left, CBotVar* right);

    /*!
     * \brief Hs
     * \param left
     * \param right
     * \return
     */
    virtual bool Hs(CBotVar* left, CBotVar* right);

    /*!
     * \brief Eq
     * \param left
     * \param right
     * \return
     */
    virtual bool Eq(CBotVar* left, CBotVar* right);

    /*!
     * \brief Ne
     * \param left
     * \param right
     * \return
     */
    virtual bool Ne(CBotVar* left, CBotVar* right);

    /*!
     * \brief And
     * \param left
     * \param right
     */
    virtual void And(CBotVar* left, CBotVar* right);

    /*!
     * \brief Or
     * \param left
     * \param right
     */
    virtual void Or(CBotVar* left, CBotVar* right);

    /*!
     * \brief XOr
     * \param left
     * \param right
     */
    virtual void XOr(CBotVar* left, CBotVar* right);

    /*!
     * \brief ASR
     * \param left
     * \param right
     */
    virtual void ASR(CBotVar* left, CBotVar* right);

    /*!
     * \brief SR
     * \param left
     * \param right
     */
    virtual void SR(CBotVar* left, CBotVar* right);

    /*!
     * \brief SL
     * \param left
     * \param right
     */
    virtual void SL(CBotVar* left, CBotVar* right);

    /*!
     * \brief Neg
     */
    virtual void Neg();

    /*!
     * \brief Not
     */
    virtual void Not();

    /*!
     * \brief Inc
     */
    virtual void Inc();

    /*!
     * \brief Dec
     */
    virtual void Dec();

    /*!
     * \brief Save0State
     * \param pf
     * \return
     */
    virtual bool Save0State(FILE* pf);

    /*!
     * \brief Save1State
     * \param pf
     * \return
     */
    virtual bool Save1State(FILE* pf);

    /*!
     * \brief RestoreState
     * \param pf
     * \param pVar
     * \return
     */
    static bool RestoreState(FILE* pf, CBotVar* &pVar);

    /*!
     * \brief debug
     */
    void debug();

    /*!
     * \brief Maj
     * \param pUser
     * \param bContinue
     */
    virtual void Maj(void* pUser = nullptr, bool bContinue = true);

    /*!
     * \brief SetUniqNum
     * \param n
     */
    void SetUniqNum(long n);

    /*!
     * \brief GetUniqNum
     * \return
     */
    long GetUniqNum();

    /*!
     * \brief NextUniqNum
     * \return
     */
    static long NextUniqNum();

protected:

    //! The corresponding token.
    CBotToken* m_token;
    //! List of variables.
    CBotVar* m_next;
    //! Type of value.
    CBotTypResult m_type;
    //! Not initialized.
    InitType m_binit;
    //! Corresponding this element.
    CBotVarClass* m_pMyThis;
    //! User data if necessary.
    void* m_pUserPtr;
    //! Static element (in class).
    bool m_bStatic;
    //! Element public, protected or private.
    int m_mPrivate;
    //! Expression for the original content.
    CBotInstr* m_InitExpr;
    //! List of limits for a table.
    CBotInstr* m_LimExpr;
    //! Unique identifier.
    long m_ident;

    //! Counter
    static long m_identcpt;

    friend class CBotStack;
    friend class CBotCStack;
    friend class CBotInstrCall;
    friend class CBotProgram;
    friend class CBotClass;
    friend class CBotVarClass;
    friend class CBotVarPointer;
    friend class CBotVarArray;
};

/* NOTE (#)
    methods    SetValInt() SetValFloat() et SetValString()
    can be called  with objects which are respectively integer, real or string
    Always be sure of the type of the variable before calling these methods

    if ( pVar->GetType() == CBotInt() ) pVar->SetValFloat( 3.3 ); // plante !!

    methods    GetValInt(), GetValFloat() et GetValString()
    use value conversions,
    GetValString() works on numbers (makes the corresponding string)
    but do not make GetValInt () with a string variable!
*/
