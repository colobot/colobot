// autoremote.h

#ifndef _AUTOREMOTE_H_
#define    _AUTOREMOTE_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


enum AutoRemotePhase
{
   ARMP_WAIT       = 1,    // attend cosmonaute
   ARMP_CAMERA1    = 2,    // derrière l'écran
   ARMP_CAMERA2    = 3,    // remote de 3/4
   ARMP_CAMERA3    = 4,    // sur le récepteur
   ARMP_CAMERA4    = 5,    // récepteur + roue avant
   ARMP_CAMERA5    = 6,    // derrière voiture
   ARMP_TERM       = 7,    // terminé
};



class CAutoRemote : public CAuto
{
public:
   CAutoRemote(CInstanceManager* iMan, CObject* object);
   ~CAutoRemote();

   void        DeleteObject(BOOL bAll=FALSE);

   void        Init();
   void        Start(int param);
   BOOL        EventProcess(const Event &event);
   BOOL        Abort();
   Error       RetError();

protected:
   void        UpdateCamera(float progress);
   void        EmitParticule(BOOL bFirst);
   void        DemoWheel(BOOL bStop);
   CObject*    SearchHuman();
   CObject*    SearchVehicle();

protected:
   int             m_param;
   AutoRemotePhase m_phase;
   float           m_progress;
   float           m_speed;
   int             m_partiStop[6];
   float           m_lastParticule;
   float           m_lastSound;
   BOOL            m_bDisplayText;
   BOOL            m_bStartDemoWheel;
   BOOL            m_bForceSlow;
   float           m_startTime;
   CObject*        m_vehicle;
   CObject*        m_human;
   D3DVECTOR       m_eyeStart;
   D3DVECTOR       m_lookatStart;
   D3DVECTOR       m_eyeGoal;
   D3DVECTOR       m_lookatGoal;
   float           m_distance;
};


#endif //_AUTOREMOTE_H_
