// autostand.h

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



class CAutoStand : public CAuto
{
public:
   CAutoStand(COldObject* object);
   ~CAutoStand();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        SoundFrame(float rTime);

protected:
   float       m_progress;
   float       m_speed;
   int         m_totalPerso;
   int         m_channelSound;
};
