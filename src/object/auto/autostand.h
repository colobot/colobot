// autostand.h

#ifndef _AUTOSTAND_H_
#define    _AUTOSTAND_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



class CAutoStand : public CAuto
{
public:
   CAutoStand(CInstanceManager* iMan, CObject* object);
   ~CAutoStand();

   void        DeleteObject(BOOL bAll=FALSE);

   void        Init();
   void        Start(int param);
   BOOL        EventProcess(const Event &event);
   BOOL        Abort();
   Error       RetError();

protected:
   void        SoundFrame(float rTime);

protected:
   float       m_progress;
   float       m_speed;
   int         m_totalPerso;
   int         m_channelSound;
};


#endif //_AUTOSTAND_H_
