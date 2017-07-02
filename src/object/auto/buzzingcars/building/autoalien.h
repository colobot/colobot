// autoalien.h

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



class CAutoAlien : public CAuto
{
public:
   CAutoAlien(COldObject* object);
   ~CAutoAlien();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        ArmPosition(Math::Vector &pos, float &dir, float angle, float lin);
   void        BotAction(COldObject *bot, int action, float delay);
   void        SynchroHammer(int action);
   COldObject*    SearchObject(ObjectType type, Math::Vector center, float radius);
   void        FireStopUpdate();
   void        UpdateTrackMapping(float progress);

protected:
   ObjectType  m_type;
   float       m_progress;
   float       m_speed;
   int         m_phase;
   float       m_lastParticule;
   float       m_trackProgress;
   float       m_rotAngle;
   float       m_rotSpeed;
   COldObject*    m_bot;
   ObjectType  m_lastType;
   bool        m_bSynchro;
   bool        m_bStop;
   bool        m_bFear;
   int         m_partiStop[12];
};
