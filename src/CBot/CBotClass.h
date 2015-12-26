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

#include "CBot/CBotDefines.h"
#include "CBot/CBotTypResult.h"
#include "CBot/CBotVar/CBotVar.h"

#include <string>

namespace CBot
{

class CBotCallMethode;
class CBotFunction;
class CBotProgram;
class CBotStack;
class CBotDefParam;
class CBotToken;
class CBotCStack;

/**
 * \brief A CBot class definition
 *
 * \section Examples Usage examples
 *
 * Define class "point":
 *
 * \code
 *  CBotClass* classPoint = new CBotClass("CPoint", nullptr);
 *  classPoint->AddItem("x", CBotTypResult(CBotTypFloat)); // add ".x"
 *  classPoint->AddItem("y", CBotTypResult(CBotTypFloat)); // add ".y"
 *
 *  // This class can be used in CBot like so:
 *  // point position;
 *  // position.x = 12;
 *  // position.y = -13.6
 *  \endcode
 *
 *  Define readonly class "object" with members of type "point" and some methods:
 *  \code
 *  CBotClass* classObject = new CBotClass("object", nullptr);
 *  classObject->AddItem("category", CBotTypResult(CBotTypInt), CBotVar::ProtectionType::ReadOnly);
 *  classObject->AddItem("position", CBotTypResult(CBotTypClass, classPoint), CBotVar::ProtectionType::ReadOnly);
 *  classObject->AddFunction("func", rFunc, cFunc); // TODO: Document function format for class methods (different from standard CBotProgram::AddFunction()!)
 *
 *  // This class can be used in CBot like so:
 *  // object item = radar(Me);
 *  // goto(item.position);
 *  // item.func();
 *  \endcode
 *
 *  Define class "robot" derrived from "object":
 *  \code
 *  CBotClass* classRobot = new CBotClass("object", classObject);
 *  classRobot->AddItem("velocity", CBotTypResult(CBotTypClass, classPoint), CBotVar::ProtectionType::ReadOnly);
 *  classRobot->AddFunction("func2", rFunc2, cFunc2);
 *
 *  // This class can be used in CBot like so:
 *  // robot item = something();
 *  // goto(item.position); // can still access base class
 *  // item.func(); // can still call base class
 *  // item.func2(); // but also the derrived class
 *  \endcode
 *
 *  Create instance of the "robot" class:
 *  \code
 *  CBotVar* var = new CBotVar("variableName", classRobot);
 *  \endcode
 *
 *  Access members of the "point" class:
 *  \code
 *  CBotVar* varX = classInstance->GetItem("x");
 *  float x = varX->GetValFloat();
 *  CBotVar* varY = classInstance->GetItem("y");
 *  float y = varX->GetValFloat();
 *
 *  // OR
 *
 *  CBotVar* var = classInstance->GetItemList();
 *  float x = var->GetValFloat();
 *  var->GetNext();
 *  float y = var->GetValFloat();
 *  \endcode
 */
class CBotClass : public CBotLinkedList<CBotClass>
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
    CBotClass(const std::string& name,
              CBotClass* pParent,
              bool bIntrinsic = false);

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
    static CBotClass* Create(const std::string& name,
                             CBotClass* parent,
                             bool intrinsic = false);

    /*!
     * \brief AddFunction This call allows to add as external new method
     * used by the objects of this class. See (**) at end of this file for
     * more details.
     * \param name
     * \param rExec
     * \param rCompile
     * \return
     */
    bool AddFunction(const std::string& name,
                     bool rExec(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                     CBotTypResult rCompile(CBotVar* pThis, CBotVar*& pVar));

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
    bool AddItem(std::string name, CBotTypResult type,
                 CBotVar::ProtectionLevel mPrivate = CBotVar::ProtectionLevel::Public);

    /*!
     * \brief AddItem Adds an item by passing the pointer to an instance of a
     * variable the object is taken as is, so do not destroyed.
     * \param pVar
     * \return
     */
    bool AddItem(CBotVar* pVar);

    /*!
     * \brief GetName Gives the name of the class.
     * \return
     */
    std::string GetName();

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
    static CBotClass* Find(const std::string& name);

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
    CBotVar* GetItem(const std::string& name);

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
    CBotTypResult CompileMethode(const std::string& name,
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
                        const std::string& name,
                        CBotVar* pThis,
                        CBotVar** ppParams,
                        CBotVar*& pResult,
                        CBotStack*& pStack,
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
                        const std::string& name,
                        CBotVar* pThis,
                        CBotVar** ppParams,
                        CBotStack*& pStack);

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
     * \param program
     * \param pToken
     * \param pParam
     * \return
     */
    bool CheckCall(CBotProgram* program, CBotDefParam* pParam, CBotToken*& pToken);

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
    std::string m_name;
    //! Number of variables in the chain.
    int m_nbVar;
    //! Content of the class.
    CBotVar* m_pVar;
    //! Intrinsic class.
    bool m_bIntrinsic;
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

} // namespace CBot
