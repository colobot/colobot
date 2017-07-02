// autohome.h

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


enum AutoHomePhase
{
   AHOP_WAIT       = 1,    // attend véhicule avec robot
   AHOP_STOPCHECK  = 2,    // attend véhicule immobile
   AHOP_OPEN       = 3,    // ouvre le toît
   AHOP_MOVE       = 4,    // déplace le robot
   AHOP_CLOSE      = 5,    // ferme le toît
   AHOP_LIVE1      = 6,    // maison définitivement habitée
   AHOP_LIVE2      = 7,    // 
   AHOP_LIVE3      = 8,    // 
   AHOP_LIVE4      = 9,    // 
   AHOP_LIVE5      = 10,   // 
   AHOP_BREAKDOWN  = 11,   // panne
};



class CAutoHome : public CAuto
{
public:
   CAutoHome(COldObject* object);
   ~CAutoHome();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        FireStopUpdate();
   void        StartingEffect();
   void        MoveBot(float progress, float rTime);
   void        EndingEffect();
   COldObject*    SearchVehicle();
   COldObject*    SearchObject(ObjectType type, Math::Vector center, float radius);
   bool        ProxiSelect(float dist);
   Math::Vector   GetVehiclePoint(COldObject *pObj);
   void        StartVehicleAction(int action);
   void        StartBotAction(int action, float delay=2.0f);
   void        HappyBlupi();

protected:
   AutoHomePhase   m_phase;
   float           m_progress;
   float           m_speed;
   float           m_lastParticule;
   int             m_partiStop[6];
   ObjectType      m_type;
   COldObject*        m_vehicle;
   COldObject*        m_bot;
   Math::Vector       m_vehiclePos;
   Math::Vector       m_startPos;
   Math::Vector       m_goalPos;
   Math::Vector       m_dir;
   float           m_startAngle;
   float           m_goalAngle;
   float           m_beforeClose;
   Gfx::CameraType      m_cameraType;
   bool            m_bZoomIn;
   bool            m_bBreakFinish;
   int             m_breakPhase[4];
   float           m_breakTimeWait[4];
   float           m_breakTimeBzzz[4];
   int             m_lastDir;
   int             m_channelSound;
};
