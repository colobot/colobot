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

#include "CBot/CBotEnums.h"

#include <string>
#include <memory>

namespace CBot
{

class CBotClass;

/**
 * \class CBotTypResult
 * \brief Class to define the complete type of a variable
 *
 * Examples:
 * \code
 * // Return a simple "float" variable
 * return CBotTypResult( CBotTypFloat );
 *
 * // Return an array of "string" variables
 * return CBotTypResult( CBotTypArrayPointer, CBotTypResult( CBotTypString ) );
 *
 * // Return an array of "point" class
 * return CBotTypResult( CBotTypArrayPointer, CBotTypResult( CBotTypArrayPointer, CBotTypResult( CBotTypIntrinsic, "point" ) ) );
 * \endcode
 */
class CBotTypResult
{
public:
    /**
     * \brief Constructor for simple types (::CBotTypInt to ::CBotTypString)
     * \param type type of created result, see ::CBotType. This can also sometimes be a value from ::CBotError.
     */
    CBotTypResult(int type);

    /**
     * \brief Constructor for pointer types and intrinsic classes
     *
     * This is equivalent to calling:
     * \code
     * CBotTypResult(type, CBotClass::Find(name))
     * \endcode
     *
     * \param type type of created result, see ::CBotType
     * \param name name of the class
     */
    CBotTypResult(int type, const std::string& name);

    /**
     * \brief Constructor for instance of a class
     * \param type type of created result, see ::CBotType
     * \param pClass class type
     */
    CBotTypResult(int type, CBotClass* pClass);

    /**
     * \brief Constructor for arrays
     * \param type type of created result, see ::CBotType
     * \param elem type of array elements
     */
    CBotTypResult(int type, const CBotTypResult& elem);

    /**
     * \brief Copy constructor
     */
    CBotTypResult(const CBotTypResult& typ);

    /**
     * \brief Default constructor
     */
    CBotTypResult() = default;

    /**
     * \brief Mode for GetType()
     */
    enum class GetTypeMode
    {
        NORMAL = 0,
        NULL_AS_POINTER = 3,
    };

    /**
     * \brief Returns ::CBotType or ::CBotError stored in this object
     * \param mode Mode, see GetTypeMode enum
     */
    int         GetType(GetTypeMode mode = GetTypeMode::NORMAL) const;

    /**
     * \brief Changes ::CBotType or ::CBotError stored in this object
     * \param n new value
     */
    void        SetType(int n);

    /**
     * \brief Returns CBotClass pointer (for ::CBotTypClass, ::CBotTypPointer)
     */
    CBotClass*  GetClass() const;

    /**
     * \brief Get size limit of an array (for ::CBotTypArrayBody or ::CBotTypArrayPointer)
     */
    int         GetLimite() const;

    /**
     * \brief Set size limit of an array (for ::CBotTypArrayBody or ::CBotTypArrayPointer)
     * \param n new value
     */
    void        SetLimite(int n);

    /**
     * \brief Set size limit of an multidimensional array
     * \param max Array of limit values, the array size has to match the number of dimensions of this array
     */
    void        SetArray(int max[]);

    /**
     * \brief Get type of array elements (for ::CBotTypArrayBody or ::CBotTypArrayPointer)
     */
    const CBotTypResult& GetTypElem() const;


    /**
     * \brief Compares whether the types are compatible
     *
     * This compares the whole object with another
     */
    bool        Compare(const CBotTypResult& typ) const;

    /**
     * \brief Compare type only
     *
     * This compares the general "type" part of this object, without checking the additional parameters
     */
    bool        Eq(int type) const;

    /**
     * \brief Copy
     */
    CBotTypResult& operator=(const CBotTypResult& src);

    /**
     * \brief Get this type name as string
     * \returns This type name as string
     */
    std::string ToString() const;

private:
    int m_type = 0;   //!< type, see ::CBotType and ::CBotError
    std::unique_ptr<CBotTypResult> m_elementType;   //!< type of array element
    CBotClass* m_class = nullptr;  //!< class type
    int m_limite = -1;  //!< array limit
    friend class CBotVarClass;
    friend class CBotVarPointer;
};

} // namespace CBot
