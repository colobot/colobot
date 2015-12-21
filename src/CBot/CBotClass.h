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
#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include
#include <string>

// Forward declaration
class CBotClass;
class CBotCallMethode;
class CBotFunction;
class CBotProgram;
class CBotStack;
class CBotDefParam;
class CBotToken;
class CBotCStack;

/*!
 * \brief The CBotClass class Class to define new classes in the language CBOT
 * For example to define the class CPoint (x, y) see comments at end of this
 * file.
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
     * \brief AddNext
     * \param pClass
     */
    void AddNext(CBotClass* pClass);

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
    std::string m_name;
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

/*
(**) Note:

    To define an external function, proceed as follows:

    a) define a routine for compilation this routine receive list of parameters
        (no values) and either returns a result type (CBotTyp... or 0 = void)
        or an error number.

    b) define a routine for the execution this routine receive list of
        parameters (with valeurs), a variable to store the result
        (according to the given type at compile time)

    For example, a routine which calculates the mean of a parameter list

int cMean(CBotVar* &pVar, std::string& ClassName)
{
    if ( pVar == nullptr ) return 6001; // there is no parameter!
    while ( pVar != nullptr )
    {
        if ( pVar->GetType() > CBotTypDouble ) return 6002; // this is not a number
        pVar = pVar -> GetNext();
    }
    return CBotTypFloat; // the type of the result may depend on the parameters!
}


bool rMean(CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    float total = 0;
    int nb = 0;
    while (pVar != nullptr)
    {
        total += pVar->GetValFloat();
        pVar = pVar->GetNext();
        nb++;
    }
    pResult->SetValFloat(total/nb); // returns the mean value
    return true; // operation fully completed
}

////////////////////////////////////////////////////////////////////////////////
// Examples of use
// Definition classes and functions


// define the global class CPoint
// --------------------------------
    m_pClassPoint = new CBotClass("CPoint", nullptr);
    // adds the component ".x"
    m_pClassPoint->AddItem("x", CBotTypResult(CBotTypFloat));
    // adds the component ".y"
    m_pClassPoint->AddItem("y", CBotTypResult(CBotTypFloat));
    // the player can then use the instructions
    // CPoint position; position.x = 12; position.y = -13.6

// define class CColobotObject
// --------------------------------
    // This class manages all the objects in the world of COLOBOT
    // the "main" user program belongs to this class
    m_pClassObject = new CBotClass("CColobotObject", m_pClassBase);
    // adds the component ".position"
    m_pClassObject->AddItem("position", m_pClassPoint);
    // adds the component ".type"
    m_pClassObject->AddItem("type", CBotTypResult(CBotTypShort));
    // adds a definition of constant
    m_pClassObject->AddConst("ROBOT", CBotTypShort, 1); // ROBOT equivalent to the value 1
    // adds the FIND routine
    m_pClassObject->AddFunction( rCompFind, rDoFind );
    // the player can now use the instructions
    // CColobotObject chose; chose = FIND( ROBOT )

// define class CColobotRobot derived from CColobotObject
// ---------------------------------------------------------
    // programs "main" associated with robots as a part of this class
    m_pClassRobot = new CBotClass("CColobotRobot", m_pClassObject);
    // add routine GOTO
    m_pClassRobot->AddFunction( rCompGoto, rDoGoto );
    // the player can now use
    // GOTO( FIND ( ROBOT ) );


// creates an instance of the class Robot
// ------------------------------------
    // for example a new robot which has just been manufactured
    CBotVar* m_pMonRobot = new CBotVar("MonRobot", m_pClassRobot);


// compiles the program by hand for this robot
// ------------------------------------------
    CString LeProgramme( "void main() {GOTO(0, 0); return 0;}" );
    if ( !m_pMonRobot->Compile( LeProgramme ) ) {error handling ...};

// build a stack for interpreter
// --------------------------------------
    CBotStack* pStack = new CBotStack(nullptr);

// executes the main program
// -------------------------
    while( false = m_pMonRobot->Execute( "main", pStack ))
    {
        // program suspended
        // could be pass a handle to another (safeguarding pstack for the robot one)
    };
    // programme "main" finished !

// routine that implements the GOTO (CPoint pos)
bool rDoGoto( CBotVar* pVar, CBotVar* pResult, int& exception )
{
    if (pVar->GetType() != CBotTypeClass ||
            pVar->IsElemOfClas("CPoint") ) { exception = 6522; return false; )
    // the parameter is not the right class?
    // in fact the control is done to the routine of compilation

    m_PosToGo.Copy( pVar ); // keeps the target position (object type CBotVar)

    // or so
    CBotVar* temp;
    temp = pVar->GetItem("x"); // is necessary for the object of type CPoint
    ASSERT (temp != nullptr && temp->GetType() == CBotTypFloat);
    m_PosToGo.x = temp->GetValFloat();

    temp = pVar->GetItem("y"); // is necessary for the object of type CPoint
    ASSERT (temp != nullptr && temp->GetType() == CBotTypFloat);
    m_PosToGo.y = temp->GetValFloat();

    return (m_CurentPos == m_PosToGo); // makes true if the position is reached
    // returns false if one had wait yet
}

*/
