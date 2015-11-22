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

////////////////////////////////////////////////////////////////////////

/**
 * \file CBotDll.h
 * \brief Library for interpretation of CBOT language
 */

#pragma once

// Modules inlcude
#include "resource.h"
#include "CBotEnums.h"

// Local include

// Global include
#include <stdio.h>
#include <map>
#include <cstring>

////////////////////////////////////////////////////////////////////////
// forward declaration of needed classes

class CBotToken;        // program turned into "tokens
class CBotStack;        // for the execution stack
class CBotClass;        // class of object
class CBotInstr;        // instruction to be executed
class CBotFunction;     // user functions
class CBotVar;          // variables
class CBotVarClass;     // instance of class
class CBotVarPointer;   // pointer to an instance of class
class CBotCall;         // functions
class CBotCallMethode;  // methods
class CBotDefParam;     // parameter list
class CBotCStack;       // stack


////////////////////////////////////////////////////////////////////////
// Variables management
////////////////////////////////////////////////////////////////////////
//n = not implemented yet

// other values ​​may be returned
// for example exceptions returned by external routines
// and " throw " with any number.


#if 0
/*
(**) Note:
     To define an external function, proceed as follows:

    a) define a routine for compilation
        this routine receive list of parameters (no values)
        and either returns a result type (CBotTyp... or 0 = void)
        or an error number
    b) define a routine for the execution
        this routine receive list of parameters (with valeurs),
        a variable to store the result (according to the given type at compile time)

    For example, a routine which calculates the mean of a parameter list */

int    cMean(CBotVar* &pVar, CBotString& ClassName)
{
    if ( pVar == nullptr ) return 6001;    // there is no parameter!

    while ( pVar != nullptr )
    {
        if ( pVar->GetType() > CBotTypDouble ) return 6002;        // this is not a number
        pVar = pVar -> GetNext();
    }

    return CBotTypFloat;        // the type of the result may depend on the parameters!
}


bool rMean(CBotVar* pVar, CBotVar* pResult, int& Exception)
{
    float total = 0;
    int   nb      = 0;
    while (pVar != nullptr)
    {
        total += pVar->GetValFloat();
        pVar = pVar->GetNext();
        nb++;
    }
    pResult->SetValFloat(total/nb);                // returns the mean value

    return true;                                // operation fully completed
}

#endif

/*
////////////////////////////////////////////////////////////////////////
// Examples of use
// Definition classes and functions


// define the global class CPoint
// --------------------------------
    m_pClassPoint    = new CBotClass("CPoint", nullptr);
    // adds the component ".x"
    m_pClassPoint->AddItem("x", CBotTypResult(CBotTypFloat));
    // adds the component ".y"
    m_pClassPoint->AddItem("y", CBotTypResult(CBotTypFloat));
    // the player can then use the instructions
    // CPoint position; position.x = 12; position.y = -13.6

// define class CColobotObject
// --------------------------------
// This class manages all the objects in the world of COLOBOT
// the  "main" user program belongs to this class
    m_pClassObject    = new CBotClass("CColobotObject", m_pClassBase);
    // adds the component ".position"
    m_pClassObject->AddItem("position", m_pClassPoint);
    // adds the component ".type"
    m_pClassObject->AddItem("type", CBotTypResult(CBotTypShort));
    // adds a definition of constant
    m_pClassObject->AddConst("ROBOT", CBotTypShort, 1);            // ROBOT equivalent to the value 1
    // adds the FIND routine
    m_pClassObject->AddFunction( rCompFind, rDoFind );
    // the player can now use the instructions
    // CColobotObject chose; chose = FIND( ROBOT )



// define class CColobotRobot derived from CColobotObject
// ---------------------------------------------------------
// programs "main" associated with robots as a part of this class
    m_pClassRobot    = new CBotClass("CColobotRobot", m_pClassObject);
    // add routine GOTO
    m_pClassRobot->AddFunction( rCompGoto, rDoGoto );
    // the player can now use
    // GOTO( FIND ( ROBOT ) );


// creates an instance of the class Robot
// ------------------------------------
// for example a new robot which has just been manufactured
    CBotVar*    m_pMonRobot = new CBotVar("MonRobot", m_pClassRobot);

// compiles the program by hand for this robot
// ------------------------------------------
    CString LeProgramme( "void main() {GOTO(0, 0); return 0;}" );
    if ( !m_pMonRobot->Compile( LeProgramme ) ) {error handling ...};

// build a stack for interpreter
// --------------------------------------
    CBotStack*    pStack = new CBotStack(nullptr);

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

    m_PosToGo.Copy( pVar );                // keeps the target position (object type CBotVar)

    // or so
    CBotVar*    temp;
    temp = pVar->GetItem("x");            // is necessary for the object of type CPoint
    ASSERT (temp != nullptr && temp->GetType() == CBotTypFloat);
    m_PosToGo.x = temp->GetValFloat();

    temp = pVar->GetItem("y");            // is necessary for the object of type CPoint
    ASSERT (temp != nullptr && temp->GetType() == CBotTypFloat);
    m_PosToGo.y = temp->GetValFloat();

    return (m_CurentPos == m_PosToGo);    // makes true if the position is reached
                                        // returns false if one had wait yet
}

*/
