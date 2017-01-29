// autohelico.h

#ifndef _AUTOHELICO_H_
#define    _AUTOHELICO_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



class CAutoHelico : public CAuto
{
public:
   CAutoHelico(CInstanceManager* iMan, CObject* object);
   ~CAutoHelico();

   void        DeleteObject(BOOL bAll=FALSE);

   void        Init();
   void        Start(int param);
   BOOL        EventProcess(const Event &event);
   BOOL        Abort();
   Error       RetError();

protected:
   void        UpdateRotorMapping(float speed);

protected:
   float       m_progress;
   float       m_speed;
   float       m_rotorSpeed;
   float       m_rotorAngle;
   int         m_rotorSound;
   int         m_lastRotorSpeed;
};


#endif //_AUTOHELICO_H_
