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

// cloud.h

#ifndef _CLOUD_H_
#define _CLOUD_H_


#include "struct.h"


class CInstanceManager;
class CD3DEngine;
class CTerrain;



#define MAXCLOUDLINE    100

typedef struct
{
    short       x, y;       // beginning
    short       len;        // in length x
    float       px1, px2, pz;
}
CloudLine;


class CCloud
{
public:
    CCloud(CInstanceManager* iMan, CD3DEngine* engine);
    ~CCloud();

    BOOL        EventProcess(const Event &event);
    void        Flush();
    BOOL        Create(const char *filename, D3DCOLORVALUE diffuse, D3DCOLORVALUE ambient, float level);
    void        Draw();

    BOOL        SetLevel(float level);
    float       RetLevel();

    void        SetEnable(BOOL bEnable);
    BOOL        RetEnable();

protected:
    BOOL        EventFrame(const Event &event);
    void        AdjustLevel(D3DVECTOR &pos, D3DVECTOR &eye, float deep, FPOINT &uv1, FPOINT &uv2);
    BOOL        CreateLine(int x, int y, int len);

protected:
    CInstanceManager*   m_iMan;
    CD3DEngine*     m_engine;
    CTerrain*       m_terrain;

    char            m_filename[100];
    float           m_level;        // overall level
    FPOINT          m_speed;        // feedrate (wind)
    D3DCOLORVALUE       m_diffuse;      // diffuse color
    D3DCOLORVALUE       m_ambient;      // ambient color
    float           m_time;
    float           m_lastTest;
    int             m_subdiv;

    D3DVECTOR       m_wind;         // wind speed
    int         m_brick;        // brick mosaic
    float           m_size;         // size of a brick element

    int         m_lineUsed;
    CloudLine       m_line[MAXCLOUDLINE];

    BOOL            m_bEnable;
};


#endif //_CLOUD_H_
