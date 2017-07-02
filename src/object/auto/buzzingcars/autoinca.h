// autoinca.h

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



enum AutoIncaPhase
{
   AINP_WAIT       = 1,    // attend qu'on presse le bouton
   AINP_OPEN       = 2,    // ouvre les 2 portes
   AINP_DOWN       = 3,    // descend la colonne
   AINP_BLITZ      = 4,    // démarre les éclairs
   AINP_TERM       = 5,    // attente finale
};


class CAutoInca : public CAuto
{
public:
   CAutoInca(COldObject* object);
   ~CAutoInca();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   COldObject*    SearchObject(ObjectType type, Math::Vector center, float radius);
   void        CameraBegin();
   void        CameraEnd();
   void        CreateBlitz(float delay);
   void        ProgressBlitz(float progress);
   void        BreakDownEvil3();
   void        SoundManip(float time, float amplitude, float frequency);
   void        SoundBlitz(float time);

protected:
   AutoIncaPhase m_phase;
   float       m_progress;
   float       m_speed;

   COldObject*    m_vehicle;
   COldObject*    m_king;
   float       m_lastParticule;
   Math::Vector   m_targetPos;
   Gfx::CameraType  m_cameraType;
   int         m_channelRay;
   bool        m_bBlitz;
};
