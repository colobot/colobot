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
#include "CBot/CBotDefines.h"

#include "CBot/CBotTypResult.h"

#include "CBot/CBotString.h"

// Local include

// Global include

// Forward declaration
class CBotVar;
class CBotClass;
class CBotCallMethode;
class CBotFunction;
class CBotProgram;
class CBotStack;
class CBotDefParam;

/*!
 * \brief The CBotClass class Class to define new classes in the language CBOT
 * for example to define the class CPoint (x, y).
 */
class CBotClass
{
public:
    //! Mark if is set or not
    bool m_IsDef;

    /*!
     * \brief CBotClass Constructor. Once a class is created, it is known around
     * CBot intrinsic mode gives a class that is not managed by pointers.
     * \param name
     * \param pParent
     * \param bIntrinsic
     */
    CBotClass( const char* name,
               CBotClass* pParent,
               bool bIntrinsic = false );

    /*!
     * \brief CBotClass Destructor.
     */
    ~CBotClass( );

    /*!
     * \brief Create
     * \param name
     * \param parent
     * \param intrinsic
     * \return
     */
    static CBotClass* Create(const char* name,
                             CBotClass* parent,
                             bool intrinsic = false);

    /*!
     * \brief AddFunction This call allows to add as external (**) new method
     * used by the objects of this class.
     * \param name
     * \param rExec
     * \param rCompile
     * \return
     */
    bool AddFunction(const char* name,
                     bool rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                     CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar));

    /*!
     * \brief AddUpdateFunc Defines routine to be called to update the elements
     * of the class.
     * \param rMaj
     * \return
     */
    bool AddUpdateFunc( void rMaj ( CBotVar* pThis, void* pUser ) );
    //

    /*!
     * \brief AddItem Adds an element to the class.
     * \param name
     * \param type
     * \param mPrivate
     * \return
     */
    bool AddItem(CBotString name, CBotTypResult type, int mPrivate = PR_PUBLIC);

    /*!
     * \brief AddItem Adds an item by passing the pointer to an instance of a
     * variable the object is taken as is, so do not destroyed.
     * \param pVar
     * \return
     */
    bool AddItem(CBotVar* pVar);

    /*!
     * \brief AddNext
     * \param pClass
     */
    void AddNext(CBotClass* pClass);

    /*!
     * \brief GetName Gives the name of the class.
     * \return
     */
    CBotString GetName();

    /*!
     * \brief GetParent Gives the parent class (or nullptr).
     * \return
     */
    CBotClass* GetParent();

    /*!
     * \brief IsChildOf True if a class is derived (Extends) of another.
     * \param pClass
     * \return true also if the classes are identical
     */
    bool IsChildOf(CBotClass* pClass);

    /*!
     * \brief Find Trouve une classe d'après son nom
     * \param pToken
     * \return A class by it's its name.
     */
    static CBotClass* Find(CBotToken* &pToken);

    /*!
     * \brief Find
     * \param name
     * \return
     */
    static CBotClass* Find(const char* name);

    /*!
     * \brief GetVar Return the list of variables.
     * \return
     */
    CBotVar* GetVar();
    /*!
     * \brief GetItem One of the variables according to its name.
     * \param name
     * \return
     */
    CBotVar* GetItem(const char* name);

    /*!
     * \brief GetItemRef
     * \param nIdent
     * \return
     */
    CBotVar* GetItemRef(int nIdent);

    /*!
     * \brief CompileMethode Compiles a method associated with an instance of
     * class the method can be declared by the user or AddFunction.
     * \param name
     * \param pThis
     * \param ppParams
     * \param pStack
     * \param nIdent
     * \return
     */
    CBotTypResult CompileMethode(const char* name,
                                 CBotVar* pThis,
                                 CBotVar** ppParams,
                                 CBotCStack* pStack,
                                 long& nIdent);

    /*!
     * \brief ExecuteMethode Executes a method.
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppParams
     * \param pResult
     * \param pStack
     * \param pToken
     * \return
     */
    bool ExecuteMethode(long& nIdent,
                        const char* name,
                        CBotVar* pThis,
                        CBotVar** ppParams,
                        CBotVar* &pResult,
                        CBotStack* &pStack,
                        CBotToken* pToken);

    /*!
     * \brief RestoreMethode Restored the execution stack.
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppParams
     * \param pStack
     */
    void RestoreMethode(long& nIdent,
                        const char* name,
                        CBotVar* pThis,
                        CBotVar** ppParams,
                        CBotStack* &pStack);

    /*!
     * \brief Compile Compiles a class declared by the user.
     * \param p
     * \param pStack
     * \return
     */
    static CBotClass* Compile(CBotToken* &p,
                              CBotCStack* pStack);

    /*!
     * \brief Compile1
     * \param p
     * \param pStack
     * \return
     */
    static CBotClass* Compile1(CBotToken* &p,
                               CBotCStack* pStack);

    /*!
     * \brief CompileDefItem
     * \param p
     * \param pStack
     * \param bSecond
     * \return
     */
    bool CompileDefItem(CBotToken* &p,
                        CBotCStack* pStack,
                        bool bSecond);

    /*!
     * \brief IsIntrinsic
     * \return
     */
    bool IsIntrinsic();

    /*!
     * \brief Purge
     */
    void Purge();

    /*!
     * \brief Free
     */
    static void Free();

    /*!
     * \brief SaveStaticState
     * \param pf
     * \return
     */
    static bool SaveStaticState(FILE* pf);

    /*!
     * \brief RestoreStaticState
     * \param pf
     * \return
     */
    static bool RestoreStaticState(FILE* pf);

    /*!
     * \brief Lock
     * \param p
     * \return
     */
    bool Lock(CBotProgram* p);

    /*!
     * \brief Unlock
     */
    void Unlock();

    /*!
     * \brief FreeLock
     * \param p
     */
    static void FreeLock(CBotProgram* p);

    /*!
     * \brief CheckCall Test if a procedure name is already defined somewhere.
     * \param pToken
     * \param pParam
     * \return
     */
    bool CheckCall(CBotToken* &pToken,
                   CBotDefParam* pParam);

private:
    //! List of classes existing at a given time.
    static CBotClass* m_ExClass;
    //! For this general list.
    CBotClass* m_ExNext;
    //! For this general list.
    CBotClass* m_ExPrev;
    //! Parent class.
    CBotClass* m_pParent;
    //! Name of this class.
    CBotString m_name;
    //! Number of variables in the chain.
    int m_nbVar;
    //! Content of the class.
    CBotVar* m_pVar;
    //! Intrinsic class.
    bool m_bIntrinsic;
    //! The string class.
    CBotClass* m_next;
    //! List of methods defined in external.
    CBotCallMethode* m_pCalls;
    //! Compiled list of methods.
    CBotFunction* m_pMethod;
    void (*m_rMaj) ( CBotVar* pThis, void* pUser );
    friend class CBotVarClass;
    //! For Lock / UnLock.
    int m_cptLock;
    //! Lock for reentrancy.
    int m_cptOne;
    //! Processes waiting for sync.
    CBotProgram* m_ProgInLock[5];
};
