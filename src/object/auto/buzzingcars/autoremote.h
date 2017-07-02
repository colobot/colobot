// autoremote.h

#pragma once

#include "object/auto/auto.h"


namespace Gfx
{
class CEngine;
class CParticle;
class CTerrain;
class CCamera;
}
class COldObject;


enum AutoRemotePhase
{
   ARMP_WAIT       = 1,    // attend cosmonaute
   ARMP_CAMERA1    = 2,    // derrière l'écran
   ARMP_CAMERA2    = 3,    // remote de 3/4
   ARMP_CAMERA3    = 4,    // sur le récepteur
   ARMP_CAMERA4    = 5,    // récepteur + roue avant
   ARMP_CAMERA5    = 6,    // derrière voiture
   ARMP_TERM       = 7,    // terminé
};



class CAutoRemote : public CAuto
{
public:
   CAutoRemote(COldObject* object);
   ~CAutoRemote();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        UpdateCamera(float progress);
   void        EmitParticle(bool bFirst);
   void        DemoWheel(bool bStop);
   COldObject*    SearchHuman();
   COldObject*    SearchVehicle();

protected:
   int             m_param;
   AutoRemotePhase m_phase;
   float           m_progress;
   float           m_speed;
   int             m_partiStop[6];
   float           m_lastParticule;
   float           m_lastSound;
   bool            m_bDisplayText;
   bool            m_bStartDemoWheel;
   bool            m_bForceSlow;
   float           m_startTime;
   COldObject*        m_vehicle;
   COldObject*        m_human;
   Math::Vector       m_eyeStart;
   Math::Vector       m_lookatStart;
   Math::Vector       m_eyeGoal;
   Math::Vector       m_lookatGoal;
   float           m_distance;
};
