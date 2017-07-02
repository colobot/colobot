// autocompass.h

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



class CAutoCompass : public CAuto
{
public:
   CAutoCompass(COldObject* object);
   ~CAutoCompass();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   COldObject*    SearchObject(ObjectType type, Math::Vector center, float radius);

protected:
   float       m_progress;
   float       m_speed;
   float       m_angle;
   float       m_carSpeed;
   float       m_lastDetect;
   Math::Vector   m_targetPos;
};
