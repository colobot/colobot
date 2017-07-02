// autodock.h

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


enum AutoDockPhase
{
   ADCP_WAIT       = 1,    // attend véhicule
   ADCP_STOPCHECK  = 2,    // attend véhicule immobile

   ADCP_OUTMOVE1   = 10,   // va vers stock
   ADCP_OUTDOWN1   = 11,   // 
   ADCP_OUTUP1     = 12,   // 
   ADCP_OUTMOVE2   = 13,   // va vers véhicule
   ADCP_OUTDOWN2   = 14,   // 
   ADCP_OUTUP2     = 15,   // 

   ADCP_INMOVE1    = 20,   // va vers véhicule
   ADCP_INDOWN1    = 21,   // 
   ADCP_INUP1      = 22,   // 
   ADCP_INMOVE2    = 23,   // va vers stock
   ADCP_INDOWN2    = 24,   // 
   ADCP_INUP2      = 25,   // 

   ADCP_START      = 30,   // attend le départ
};



class CAutoDock : public CAuto
{
public:
   CAutoDock(COldObject* object);
   ~CAutoDock();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   void        FireStopUpdate(bool bLightOn);
   void        MoveDock();
   COldObject*    SearchEvil();
   COldObject*    SearchVehicle();
   COldObject*    SearchStockOut();
   float       GetObjectHeight(COldObject *pObj);
   Math::Vector   GetVehiclePoint(COldObject *pObj);
   bool        SearchFreePos(Math::Vector &os);
   void        ParticleFrame(float rTime);
   void        SoundManip(float time, float amplitude=1.0f, float frequency=1.0f);
   void        StartBzzz();
   void        StopBzzz();
   void        StartVehicleAction(int action);
   void        TruckObject(COldObject *pObj, bool bTake);
   void        ArmObject(COldObject *pObj, bool bTake);
   void        CameraBegin();
   void        CameraEnd();

protected:
   AutoDockPhase   m_phase;
   float           m_progress;
   float           m_speed;
   int             m_partiStop[6];
   Math::Vector       m_center;
   Math::Vector       m_currentPos;
   Math::Vector       m_startPos;
   Math::Vector       m_goalPos;
   float           m_startAngle;
   float           m_goalAngle;
   float           m_heightFret;
   float           m_heightVehicle;
   COldObject*        m_vehicle;
   COldObject*        m_fret;
   Math::Vector       m_fretPos;
   Math::Vector       m_fretOffset;
   Math::Vector       m_vehiclePos;
   float           m_lastParticule;
   float           m_lastEffect;
   int             m_channelSound;
   Gfx::CameraType      m_cameraType;
};
