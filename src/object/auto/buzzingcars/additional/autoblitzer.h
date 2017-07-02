// autoblitzer.h

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



enum AutoBlitzerPhase
{
   ABLP_WAIT       = 1,    // attend cible
   ABLP_BLITZ      = 2,    // électrocute
   ABLP_WAITOUT    = 3,    // attend avant de recommencer
};


class CAutoBlitzer : public CAuto
{
public:
   CAutoBlitzer(COldObject* object);
   ~CAutoBlitzer();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   COldObject*    SearchObject(int total, ObjectType *type, float *radius, Math::Vector center);
   void        BreakDownFactory();
   void        BreakDownEvil1();
   void        BreakDownEvil3();

protected:
   AutoBlitzerPhase m_phase;
   float       m_progress;
   float       m_speed;

   float       m_lastParticle;
   Math::Vector   m_targetPos;
   Gfx::CameraType  m_cameraType;
   ObjectType  m_targetType;
   float       m_delayBlitz;
   float       m_delayTotal;
   int         m_channelSphere;
};
