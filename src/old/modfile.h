// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// modfile.h

#pragma once


#include "math/vector.h"
#include "old/d3dengine.h"


class CInstanceManager;




struct OldModelTriangle1
{
    char            bUsed;      // true -> using
    char            bSelect;    // true -> selected
    D3DVERTEX       p1;
    D3DVERTEX       p2;
    D3DVERTEX       p3;
    D3DMATERIAL7    material;
    char            texName[20];
    float           min;
    float           max;
};      // length = 196 bytes

struct OldModelTriangle2
{
    char            bUsed;      // true -> used
    char            bSelect;    // true -> selected
    D3DVERTEX       p1;
    D3DVERTEX       p2;
    D3DVERTEX       p3;
    D3DMATERIAL7    material;
    char            texName[20];
    float           min;
    float           max;
    long            state;
    short           reserve1;
    short           reserve2;
    short           reserve3;
    short           reserve4;
};

struct ModelTriangle
{
    char            bUsed;      // true -> used
    char            bSelect;    // true -> selected
    D3DVERTEX2      p1;
    D3DVERTEX2      p2;
    D3DVERTEX2      p3;
    D3DMATERIAL7    material;
    char            texName[20];
    float           min;
    float           max;
    long            state;
    short           texNum2;
    short           reserve2;
    short           reserve3;
    short           reserve4;
};      // length = 208 bytes




class CModFile
{
public:
    CModFile(CInstanceManager* iMan);
    ~CModFile();

    bool            ReadDXF(char *filename, float min, float max);
    bool            AddModel(char *filename, int first, bool bEdit=false, bool bMeta=true);
    bool            ReadModel(char *filename, bool bEdit=false, bool bMeta=true);
    bool            WriteModel(char *filename);

    bool            CreateEngineObject(int objRank, int addState=0);
    void            Mirror();

    void            SetTriangleUsed(int total);
    int             RetTriangleUsed();
    int             RetTriangleMax();
    ModelTriangle*  RetTriangleList();

    float           RetHeight(Math::Vector pos);

protected:
    bool            CreateTriangle(Math::Vector p1, Math::Vector p2, Math::Vector p3, float min, float max);

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;

    ModelTriangle*  m_triangleTable;
    int             m_triangleUsed;
};


