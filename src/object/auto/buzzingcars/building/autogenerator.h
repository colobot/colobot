// autogenerator.h

#pragma once

#include "object/auto/auto.h"


namespace Gfx
{
class CEngine;
class CParticle;
class CTerrain;
class CCamera;
}
class COldObject;



enum AutoGeneratorPhase
{
   AGEP_WAIT       = 1,    // attend
   AGEP_CREATE     = 2,    // crée
};


class CAutoGenerator : public CAuto
{
public:
   CAutoGenerator(COldObject* object);
   ~CAutoGenerator();

   void        DeleteObject(bool bAll=false);

   void        Init();
   void        Start(int param);
   bool        EventProcess(const Event &event);
   bool        Abort();
   Error       GetError();

protected:
   COldObject*    SearchObject();
   COldObject*    SearchObject(ObjectType type);
   int         CountObject(ObjectType type);
   bool        CreateObject(ObjectType type);
   void        StartAction(COldObject *pObj, int action);

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
