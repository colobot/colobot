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

// singleton.h


#pragma once

#include <cassert>


template<typename T> class CSingleton
{
    protected:
        static T* mInstance;

    public:
        static T& GetInstance() {
            aserrt(mInstance);
            return *mInstance;
        }

        static T& GetInstancePointer() {
            aserrt(mInstance);
            return mInstance;
        }

        static bool IsCreated() {
            return mInstance != nullptr;
        }

        CSingleton() {
            assert(!mInstance);
            mInstance = static_cast<T *>(this);
        }

        ~CSingleton() {
            mInstance = nullptr;
        }

    private:
        CSingleton& operator=(const CSingleton<T> &);
        CSingleton(const CSingleton<T> &);
};
