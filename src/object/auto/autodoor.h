// autodoor.h

#ifndef _AUTODOOR_H_
#define    _AUTODOOR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;


enum AutoDoorPhase
{
   ADOP_WAIT   = 1,    // attend porte ouverte
   ADOP_BREAK  = 2,    // mécanisme porte cassé
   ADOP_STOP   = 3,    // mécanisme porte stoppé
                       // DOOR1:
   ADOP_1DOWN  = 10,   // ferme la porte
   ADOP_1CLOSE = 11,   // porte définitivement fermée

   ADOP_13WAIT = 12,   // attend condition d'ouverture
   ADOP_13UP   = 13,   // ouvre
   ADOP_13OPEN = 14,   // porte définitivement ouverte
                       // DOOR2:
   ADOP_2CLOSE = 20,   // ferme les portes
   ADOP_2OPEN  = 21,   // ouvre les portes
                       // DOOR3:
   ADOP_3DOWN  = 30,   // descend la porte
   ADOP_3UP    = 31,   // monte la porte
};



class CAutoDoor : public CAuto
{
public:
   CAutoDoor(CInstanceManager* iMan, CObject* object);
   ~CAutoDoor();

   void        DeleteObject(BOOL bAll=FALSE);

   void        Init();
   void        Start(int param);
   BOOL        EventProcess(const Event &event);
   BOOL        Abort();
   Error       RetError();

protected:
   void        MoveDoor(float progress);
   BOOL        DetachPart(int part, D3DVECTOR speed);
   void        UpdateCrashSphere();
   void        FireStopUpdate();
   CObject*    SearchTarget();
   CObject*    SearchVehicle(float radius=20.0f);
   void        CrashVehicle(CObject *vehicle, BOOL bFlat);
   void        DownParticule(float rTime, D3DVECTOR pos);

protected:
   ObjectType      m_type;     // OBJECT_DOORn
   AutoDoorPhase   m_phase;
   float           m_progress;
   float           m_speed;
   float           m_lastParticule;
   float           m_initialDelay;
   int             m_counter;
   int             m_rank;
   int             m_programNumber;
   float           m_cycleDelay;
   float           m_doorProgress;
   float           m_doorStart;
   int             m_partiStop[6];
   D3DVECTOR       m_botPos;
};


#endif //_AUTODOOR_H_
