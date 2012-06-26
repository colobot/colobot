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

// mainmap.h

#ifndef _MAINMAP_H_
#define _MAINMAP_H_


class CInstanceManager;
class CEvent;
class CD3DEngine;
class CInterface;
class CObject;



class CMainMap
{
public:
    CMainMap(CInstanceManager* iMan);
    ~CMainMap();

    void        UpdateMap();
    void        CreateMap();
    void        SetFixImage(char *filename);
    void        FloorColorMap(D3DCOLORVALUE floor, D3DCOLORVALUE water);
    void        ShowMap(BOOL bShow);
    void        DimMap();
    float       RetZoomMap();
    void        ZoomMap(float zoom);
    void        ZoomMap();
    void        MapEnable(BOOL bEnable);
    BOOL        RetShowMap();
    BOOL        RetFixImage();
    CObject*    DetectMap(FPOINT pos, BOOL &bInMap);
    void        SetHilite(CObject* pObj);
    void        SetToy(BOOL bToy);
    void        SetFixParam(float zoom, float ox, float oy, float angle, int mode, BOOL bDebug);

protected:
    void        CenterMap();

protected:
    CInstanceManager* m_iMan;
    CEvent*         m_event;
    CD3DEngine*     m_engine;
    CInterface*     m_interface;

    int             m_mapMode;
    BOOL            m_bFixImage;
};


#endif //_MAINMAP_H_
