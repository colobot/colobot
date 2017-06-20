// autogenerator.h

#ifndef _AUTOGENERATOR_H_
#define    _AUTOGENERATOR_H_


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;

enum ParticuleType;



enum AutoGeneratorPhase
{
   AGEP_WAIT       = 1,    // attend
   AGEP_CREATE     = 2,    // crée
};


class CAutoGenerator : public CAuto
{
public:
   CAutoGenerator(CInstanceManager* iMan, CObject* object);
   ~CAutoGenerator();

   void        DeleteObject(BOOL bAll=FALSE);

   void        Init();
   void        Start(int param);
   BOOL        EventProcess(const Event &event);
   BOOL        Abort();
   Error       RetError();

protected:
   CObject*    SearchObject();
   CObject*    SearchObject(ObjectType type);
   int         CountObject(ObjectType type);
   BOOL        CreateObject(ObjectType type);
   void        StartAction(CObject *pObj, int action);

protected:
   AutoGeneratorPhase m_phase;
   float       m_progress;
   float       m_speed;
   int         m_totalCreate;
   int         m_maxCreate;
   float       m_delay;
   int         m_programNumber;
   float       m_lastParticule;
};


#endif //_AUTOGENERATOR_H_
