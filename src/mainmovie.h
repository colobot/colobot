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

// mainmovie.h

#ifndef _MAINMOVIE_H_
#define _MAINMOVIE_H_


class CInstanceManager;
class CEvent;
class CD3DEngine;
class CInterface;
class CRobotMain;
class CCamera;
class CSound;
class CObject;




enum MainMovieType
{
    MM_NONE,
    MM_SATCOMopen,
    MM_SATCOMclose,
};



class CMainMovie
{
public:
    CMainMovie(CInstanceManager* iMan);
    ~CMainMovie();

    void            Flush();
    BOOL            Start(MainMovieType type, float time);
    BOOL            Stop();
    BOOL            IsExist();
    BOOL            EventProcess(const Event &event);
    MainMovieType   RetType();
    MainMovieType   RetStopType();

protected:

protected:
    CInstanceManager* m_iMan;
    CEvent*         m_event;
    CD3DEngine*     m_engine;
    CInterface*     m_interface;
    CRobotMain*     m_main;
    CCamera*        m_camera;
    CSound*         m_sound;

    MainMovieType   m_type;
    MainMovieType   m_stopType;
    float           m_speed;
    float           m_progress;
    D3DVECTOR       m_initialEye;
    D3DVECTOR       m_initialLookat;
    D3DVECTOR       m_finalEye[2];
    D3DVECTOR       m_finalLookat[2];
};


#endif //_MAINMOVIE_H_
