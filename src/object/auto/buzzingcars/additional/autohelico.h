// autohelico.h

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



class CAutoHelico : public CAuto
{
public:
   CAutoHelico(COldObject* object);
   ~CAutoHelico();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        UpdateRotorMapping(float speed);

protected:
   float       m_progress;
   float       m_speed;
   float       m_rotorSpeed;
   float       m_rotorAngle;
   int         m_rotorSound;
   int         m_lastRotorSpeed;
};
