// autobomb.h

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



#define AB_STOP        0
#define AB_TRUCK   1
#define AB_BURNON  2
#define AB_BURNOFF 3
#define AB_BURNFLIP    4



class CAutoBomb : public CAuto
{
public:
   CAutoBomb(COldObject* object);
   ~CAutoBomb();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   Error       SetAction(int action, float time);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        SpeedAdapt(Math::Vector &angle, float rTime);

protected:
   float       m_progress;
   float       m_speed;
   Math::Vector   m_angle;
   bool        m_bBurn;
   float       m_fireDelay;
   float       m_fireProgress;
   float       m_lastParticle;
   int         m_partiSpark;
};
