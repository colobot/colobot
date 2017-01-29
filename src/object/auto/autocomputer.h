// autocomputer.h

#ifndef _AUTOCOMPUTER_H_
#define    _AUTOCOMPUTER_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



class CAutoComputer : public CAuto
{
public:
   CAutoComputer(CInstanceManager* iMan, CObject* object);
   ~CAutoComputer();

   void        DeleteObject(BOOL bAll=FALSE);

   void        Init();
   void        Start(int param);
   BOOL        EventProcess(const Event &event);
   BOOL        Abort();
   Error       RetError();

protected:
   void        StopObject(ObjectType type, D3DVECTOR center, float radius);
   void        FireStopUpdate();

protected:
   D3DVECTOR   m_pos;
   float       m_progress;
   float       m_speed;
   int         m_phase;
   int         m_flagStop;
   int         m_partiStop[9];
};


#endif //_AUTOCOMPUTER_H_
