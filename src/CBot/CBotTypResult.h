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

// Local include

// Global include

class CBotClass;

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
