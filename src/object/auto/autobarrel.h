// autobarrel.h

#ifndef _AUTOBARREL_H_
#define    _AUTOBARREL_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



class CAutoBarrel : public CAuto
{
public:
   CAutoBarrel(CInstanceManager* iMan, CObject* object);
   ~CAutoBarrel();

   void        DeleteObject(BOOL bAll=FALSE);

   void        Init();
   void        Start(int param);
   BOOL        EventProcess(const Event &event);
   BOOL        Abort();
   Error       RetError();

protected:

protected:
   float       m_progress;
   float       m_speed;
   ObjectType  m_type;
   float       m_force;
};


#endif //_AUTOBARREL_H_
