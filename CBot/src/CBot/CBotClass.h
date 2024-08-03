/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include <deque>
#include <set>
#include <list>

namespace CBot
{

class CBotCallMethode;
class CBotFunction;
class CBotProgram;
class CBotStack;
class CBotDefParam;
class CBotToken;
class CBotCStack;
class CBotExternalCallList;

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
 *  CBotVar* var = CBotVar::Create("variableName", classRobot);
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
class CBotClass
{
public:
    /*!
     * \brief CBotClass Constructor. Once a class is created, it is known around
     * CBot intrinsic mode gives a class that is not managed by pointers.
     * \param name
     * \param parent
     * \param bIntrinsic
     */
    CBotClass(const std::string& name,
              CBotClass* parent,
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
     * \brief Add a function that can be called from CBot
     * \see CBotProgram::AddFunction
     */
    bool AddFunction(const std::string& name,
                     bool rExec(CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception, void* user),
                     CBotTypResult rCompile(CBotVar* pThis, CBotVar*& pVar));

    /*!
     * \brief SetUpdateFunc Defines routine to be called to update the elements
     * of the class.
     * \param rUpdate
     * \return
     */
    bool SetUpdateFunc(void rUpdate(CBotVar* thisVar, void* user));
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
    const std::string& GetName();

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
     * \brief Check whether a variable is already defined in a class
     * \param name Name of the variable
     * \return True if a variable is defined in the class
     */
    bool CheckVar(const std::string &name);

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
    CBotTypResult CompileMethode(CBotToken* name,
                                 CBotVar* pThis,
                                 CBotVar** ppParams,
                                 CBotCStack* pStack,
                                 long &nIdent);

    /*!
     * \brief ExecuteMethode Executes a method.
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppParams
     * \param pResultType
     * \param pStack
     * \param pToken
     * \return
     */
    bool ExecuteMethode(long &nIdent, CBotVar* pThis, CBotVar** ppParams, CBotTypResult pResultType,
                        CBotStack*&pStack, CBotToken* pToken);

    /*!
     * \brief RestoreMethode Restored the execution stack.
     * \param nIdent
     * \param name
     * \param pThis
     * \param ppParams
     * \param pStack
     */
    void RestoreMethode(long &nIdent,
                        CBotToken* name,
                        CBotVar* pThis,
                        CBotVar** ppParams,
                        CBotStack*&pStack);

    /*!
     * \brief Compile Compiles a class declared by the user.
     * \param p
     * \param pStack
     * \return
     */
    static CBotClass* Compile(CBotToken* &p,
                              CBotCStack* pStack);

    /*!
     * \brief Pre-compile a new class
     * \param p[in, out] Pointer to first token of the class, will be updated to point to first token after the class definition
     * \param pStack Compile stack
     *
     * This function is used to find the beginning and end of class definition.
     *
     * If any errors in the code are detected, this function will set the error on compile stack and return nullptr.
     *
     * \return Precompiled class, or nullptr in case of error
     */
    static CBotClass* Compile1(CBotToken* &p,
                               CBotCStack* pStack);

    /*!
     * \brief DefineClasses Calls CompileDefItem for each class in a list
     * of classes, defining fields and pre-compiling methods.
     * \param pClassList List of classes
     * \param pStack
     */
    static void DefineClasses(std::list<CBotClass*> pClassList, CBotCStack* pStack);

    /*!
     * \brief Get the list of user-defined methods in this class.
     * \return List of methods, can be empty.
     */
    const std::list<CBotFunction*>& GetFunctions() const;

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
    static void ClearPublic();

    /*!
     * \brief Save all static variables from each public class
     * \param ostr Output stream
     * \return true on success
     */
    static bool SaveStaticState(std::ostream &ostr);

    /*!
     * \brief Restore all static variables in each public class
     * \param istr Input stream
     * \return true on success
     */
    static bool RestoreStaticState(std::istream &istr);

    /**
     * \brief Request a lock on this class (for "synchronized" keyword)
     * \param prog Program that requests the lock
     * \return true if lock was acquired, false if the lock is already taken
     */
    bool Lock(CBotProgram* prog);

    /**
     * \brief Release the lock acquired in Lock()
     * If you call Lock() multiple times for the same program, you have to call Unlock() multiple times too
     */
    void Unlock();

    /**
     * \brief Release all locks in all classes held by this program
     * \param prog Program to release the locks from
     */
    static void FreeLock(CBotProgram* prog);

    /*!
     * \brief CheckCall Test if a procedure name is already defined somewhere.
     * \param program
     * \param pToken
     * \param pParam
     * \return
     */
    bool CheckCall(CBotProgram* program, CBotDefParam* pParam, CBotToken*& pToken);

    void Update(CBotVar* var, void* user);

private:
    //! List of all public classes
    static std::set<CBotClass*> m_publicClasses;


    //! true if this class is fully compiled, false if only precompiled
    bool m_IsDef;
    //! Name of this class
    std::string m_name;
    //! Parent class
    CBotClass* m_parent;
    //! Number of variables in the chain
    int m_nbVar;
    //! Intrinsic class
    bool m_bIntrinsic;
    //! Linked list of all class fields
    CBotVar* m_pVar;
    //! Linked list of all class external calls
    CBotExternalCallList* m_externalMethods;
    //! List of all class methods
    std::list<CBotFunction*> m_pMethod{};
    void (*m_rUpdate)(CBotVar* thisVar, void* user);

    CBotToken* m_pOpenblk;

    //! How many times the program currently holding the lock called Lock()
    int m_lockCurrentCount = 0;
    //! Programs waiting for lock. m_lockProg[0] is the program currently holding the lock, if any
    std::deque<CBotProgram*> m_lockProg{};
};

} // namespace CBot
