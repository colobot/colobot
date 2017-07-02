// autohook.h

#pragma once

#include "object/auto/auto.h"

#include "graphics/engine/camera.h"


namespace Gfx
{
class CEngine;
class CParticle;
class CTerrain;
class CCamera;
}
class COldObject;



enum AutoHookPhase
{
   AHKP_WAIT       = 1,    // attend cible
   AHKP_GOSTART    = 2,    // 
   AHKP_DOWN       = 3,    // 
   AHKP_UP         = 4,    // 
   AHKP_GOEND      = 5,    // 
};


class CAutoHook : public CAuto
{
public:
   CAutoHook(COldObject* object);
   ~CAutoHook();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   COldObject*    SearchObject(ObjectType type, Math::Vector center, float minRadius, float maxRadius);
   void        StartAction(int action, float delay);
   void        SoundManip(float time, float amplitude, float frequency);

protected:
   AutoHookPhase m_phase;
   float       m_progress;
   float       m_speed;

   float       m_lastParticle;
   Math::Vector   m_targetPos;
   Gfx::CameraType  m_cameraType;
   float       m_startAngle;
   float       m_goalAngle;
   float       m_startDist;
   float       m_goalDist;
   float       m_startHeight;
   float       m_goalHeight;
   float       m_startRot;
   float       m_goalRot;
   bool        m_bGrab;
   COldObject*    m_target;
   COldObject*    m_load;
};

