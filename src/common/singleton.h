// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file common/singleton.h
 * \brief CSingleton base class for singletons
 */

#pragma once

#include <cassert>


template<typename T> class CSingleton
{
    protected:
        static T* mInstance;

    public:
        static T& GetInstance() {
            assert(mInstance != nullptr);
            return *mInstance;
        }

        static T* GetInstancePointer() {
            assert(mInstance != nullptr);
            return mInstance;
        }

        static bool IsCreated() {
            return mInstance != nullptr;
        }

        CSingleton() {
            assert(mInstance == nullptr);
            mInstance = static_cast<T *>(this);
        }

        virtual ~CSingleton() {
            mInstance = nullptr;
        }

    private:
        CSingleton& operator=(const CSingleton<T> &);
        CSingleton(const CSingleton<T> &);
};
