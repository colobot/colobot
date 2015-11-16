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


#define    CBOTVERSION    104

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

// for SetUserPtr when deleting an object
// \TODO define own types to distinct between different states of objects
#define OBJECTDELETED (reinterpret_cast<void*>(-1))
// value set before initialization
#define OBJECTCREATED (reinterpret_cast<void*>(-2))


/** \brief CBotTypResult class to define the complete type of a result*/
class CBotTypResult
{
public:
    /**
     * \brief CBotTypResult constructor  for simple types (CBotTypInt to CBotTypString)
     * \param type type of created result, see CBotType
     */
    CBotTypResult(int type);
    // for simple types (CBotTypInt à CBotTypString)


    CBotTypResult(int type, const char* name);
    // for pointer types and intrinsic classes

    CBotTypResult(int type, CBotClass* pClass);
    // for the instance of a class

    CBotTypResult(int type, CBotTypResult elem);
    // for arrays of variables

    CBotTypResult(const CBotTypResult& typ);
    // for assignments

    CBotTypResult();
    // for default

    ~CBotTypResult();

    int            GetType(int mode = 0) const;
    // returns type CBotType* as a result

    void        SetType(int n);
    // modifies a type

    CBotClass*    GetClass() const;
    // makes the pointer to the class (for CBotTypClass, CBotTypPointer)

    int            GetLimite() const;
    // returns limit size of table (CBotTypArray)

    void        SetLimite(int n);
    // set limit to the table

    void        SetArray(int* max );
    // set limits for a list of dimensions (arrays of arrays)

    CBotTypResult& GetTypElem() const;
    // returns type of array elements (CBotTypArray)
    // rend le type des éléments du tableau (CBotTypArray)

    bool        Compare(const CBotTypResult& typ) const;
    // compares whether the types are compatible
    bool        Eq(int type) const;
    // compare type

    CBotTypResult& operator=(const CBotTypResult& src);
    // copy a complete type in another

private:
    int                m_type;
    CBotTypResult*    m_pNext;    // for the types of type
    CBotClass*        m_pClass;    // for the derivatives of class
    int                m_limite;    // limits of tables
    friend class    CBotVarClass;
    friend class    CBotVarPointer;
};

/*
// to define a result as output, using for example

    // to return a simple Float
    return CBotTypResult( CBotTypFloat );


    // to return a string array
    return CBotTypResult( CBotTypArray, CBotTypResult( CBotTypString ) );

    // to return un array of array of "point" class
    CBotTypResult    typPoint( CBotTypIntrinsic, "point" );
    CBotTypResult    arrPoint( CBotTypArray, typPoint );
    return    CBotTypResult( CBotTypArray, arrPoint );
*/


////////////////////////////////////////////////////////////////////////
// Error Handling of compilation and execution
////////////////////////////////////////////////////////////////////////

// Here are the list of errors that can be returned by the module
// for compilation

#define    CBotErrOpenPar            5000    // missing the opening parenthesis
#define    CBotErrClosePar            5001    // missing the closing parenthesis
#define    CBotErrNotBoolean        5002    // expression must be a boolean
#define    CBotErrUndefVar            5003    // undeclared variable
#define    CBotErrBadLeft            5004    // assignment impossible ( 5 = ... )
#define    CBotErrNoTerminator        5005    // semicolon expected
#define    CBotErrCaseOut            5006    // case outside a switch
//    CBotErrNoTerm            5007, plus utile
#define    CBotErrCloseBlock        5008    // missing " } "
#define    CBotErrElseWhitoutIf    5009    // else without matching if
#define    CBotErrOpenBlock        5010    // missing " { "
#define    CBotErrBadType1            5011    // wrong type for the assignment
#define    CBotErrRedefVar            5012    // redefinition of the variable
#define    CBotErrBadType2            5013    // Two operands are incompatible
#define    CBotErrUndefCall        5014    // routine undefined
#define    CBotErrNoDoubleDots        5015    // " : " expected
//    CBotErrWhile            5016, plus utile
#define    CBotErrBreakOutside        5017    // break outside of a loop
#define    CBotErrUndefLabel        5019    // label udnefined
#define    CBotErrLabel            5018    // label ne peut se mettre ici (label can not get here)
#define    CBotErrNoCase            5020    // missing " case "
#define    CBotErrBadNum            5021    // expected number
#define    CBotErrVoid                5022    // " void " not possible here
#define    CBotErrNoType            5023    // type declaration expected
#define    CBotErrNoVar            5024    // variable name expected
#define    CBotErrNoFunc            5025    // expected function name
#define    CBotErrOverParam        5026    // too many parameters
#define    CBotErrRedefFunc        5027    // this function already exists
#define    CBotErrLowParam            5028    // not enough parameters
#define    CBotErrBadParam            5029    // wrong types of parameters
#define    CBotErrNbParam            5030    // wrong number of parameters
#define    CBotErrUndefItem        5031    // element does not exist in the class
#define    CBotErrUndefClass        5032    // variable is not a class
#define    CBotErrNoConstruct        5033    // no appropriate constructor
#define    CBotErrRedefClass        5034    // class already exists
#define    CBotErrCloseIndex        5035    // " ] " expected
#define    CBotErrReserved            5036    // reserved word (for a DefineNum)
#define CBotErrBadNew            5037    // wrong setting for new
#define CBotErrOpenIndex        5038    // " [ " expected
#define CBotErrBadString        5039    // expected string
#define CBotErrBadIndex            5040    // wrong index type "[ false ]"
#define CBotErrPrivate            5041    // protected item
#define CBotErrNoPublic            5042    // missing word "public"

// here is the list of errors that can be returned by the module
// for the execution

#define    CBotErrZeroDiv            6000    // division by zero
#define    CBotErrNotInit            6001    // uninitialized variable
#define    CBotErrBadThrow            6002    // throw a negative value
#define    CBotErrNoRetVal            6003    // function did not return results
#define    CBotErrNoRun            6004    // Run() without active function
#define    CBotErrUndefFunc        6005    // calling a function that no longer exists
#define CBotErrNotClass            6006    // this class does not exist
#define CBotErrNull                6007    // null pointer
#define CBotErrNan                6008    // calculation with a NAN
#define CBotErrOutArray            6009    // index out of array
#define CBotErrStackOver        6010    // stack overflow
#define CBotErrDeletedPtr        6011    // pointer to an object destroyed

#define CBotErrFileOpen            6012    // cannot open the file
#define CBotErrNotOpen            6013    // channel not open
#define CBotErrRead                6014    // error while reading
#define CBotErrWrite            6015    // writing error


// other values ​​may be returned
// for example exceptions returned by external routines
// and " throw " with any number.

///////////////////////////////////////////////////////////////////////////////
// routines for file management  (* FILE)
    FILE*        fOpen(const char* name, const char* mode);
    int            fClose(FILE* filehandle);
    size_t        fWrite(const void *buffer, size_t elemsize, size_t length, FILE* filehandle);
    size_t        fRead(void *buffer, size_t elemsize, size_t length, FILE* filehandle);


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
