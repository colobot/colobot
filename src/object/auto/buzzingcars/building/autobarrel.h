// autobarrel.h

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



class CAutoBarrel : public CAuto
{
public:
   CAutoBarrel(COldObject* object);
   ~CAutoBarrel();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:

protected:
   float       m_progress;
   float       m_speed;
   ObjectType  m_type;
   float       m_force;
};
