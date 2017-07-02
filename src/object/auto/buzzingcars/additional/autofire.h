// autofire.h

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


enum AutoFirePhase
{
   AFIP_WAIT       = 1,    // attend cible
   AFIP_TURNIN     = 2,    // tourne vers cible
   AFIP_SPRINKLE   = 3,    // gicle
   AFIP_TURNOUT    = 4,    // tourne vers repos
   AFIP_WAITOUT    = 5,    // attend avant de recommencer
};



class CAutoFire : public CAuto
{
public:
   CAutoFire(COldObject* object);
   ~CAutoFire();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   COldObject*    SearchObject(ObjectType type, Math::Vector center, float radius);
   void        BreakDown();

protected:
   AutoFirePhase m_phase;
   float       m_progress;
   float       m_speed;

   float       m_startAngleH;
   float       m_startAngleV;
   float       m_goalAngleH;
   float       m_goalAngleV;
   float       m_trackMass;
   float       m_lastParticle;
   float       m_lastPlouf;
   float       m_lastSound;
   float       m_lastBreakdown;
   Math::Vector   m_targetPos;
   Gfx::CameraType  m_cameraType;
};
