// autocomputer.h

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



class CAutoComputer : public CAuto
{
public:
   CAutoComputer(COldObject* object);
   ~CAutoComputer();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        StopObject(ObjectType type, Math::Vector center, float radius);
   void        FireStopUpdate();

protected:
   Math::Vector   m_pos;
   float       m_progress;
   float       m_speed;
   int         m_phase;
   int         m_flagStop;
   int         m_partiStop[9];
};
