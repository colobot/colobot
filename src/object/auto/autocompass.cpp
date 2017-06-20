// autocompass.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "camera.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "motion.h"
#include "motionbot.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autocompass.h"





// Constructeur de l'objet.

CAutoCompass::CAutoCompass(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   CAuto::CAuto(iMan, object);

   Init();
}

// Destructeur de l'objet.

CAutoCompass::~CAutoCompass()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoCompass::DeleteObject(BOOL bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoCompass::Init()
{
   m_time = 0.0f;

   m_progress = 0.0f;
   m_speed    = 1.0f/2.0f;
   m_angle    = 0.0f;
   m_carSpeed = 0.0f;
   m_lastDetect = 0.0f;
   m_targetPos.x = NAN;
}


// Démarre l'objet.

void CAutoCompass::Start(int param)
{
}


// Gestion d'un événement.

BOOL CAutoCompass::EventProcess(const Event &event)
{
   CObject*    car;
   CObject*    target;
   CPhysics*   physics;
   D3DVECTOR   pos;
   float       speed, angle;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;

   m_progress += event.rTime*m_speed;
   m_lastDetect += event.rTime;

   if ( m_lastDetect > 0.5f )
   {
       m_lastDetect = 0.0f;

       target = SearchObject(OBJECT_UFO, m_object->RetPosition(0), 10000.0f);
       if ( target == 0 )
       {
           m_targetPos.x = NAN;
       }
       else
       {
           m_targetPos = target->RetPosition(0);
       }
   }

   car = m_object->RetTruck();
   if ( car == 0 )  // boussole seule ?
   {
       if ( m_targetPos.x == NAN )
       {
           angle = 0.0f;
       }
       else
       {
           pos = m_object->RetPosition(0);
           angle = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
       }

       m_object->SetAngleY(1, angle);
   }
   else    // boussole sur une voiture ?
   {
       physics = car->RetPhysics();
       if ( physics == 0 )
       {
           speed = 0.0f;
       }
       else
       {
           speed = Abs(physics->RetLinMotionX(MO_REASPEED)/physics->RetLinMotionX(MO_ADVSPEED));
       }

       if ( m_targetPos.x == NAN )
       {
           angle = 0.0f;
       }
       else
       {
           pos = car->RetPosition(0);
           angle = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
           angle -= car->RetAngleY(0);
       }

       m_carSpeed = Smooth(m_carSpeed, speed, event.rTime*0.5f);

       m_angle += (10.0f-m_carSpeed*8.0f)*event.rTime;;
       angle += sinf(m_angle)*m_carSpeed*PI*2.0f;
       angle = Smooth(m_object->RetAngleY(1), angle, event.rTime*5.0f);
       m_object->SetAngleY(1, angle);
   }

   return TRUE;
}


// Cherche un objet proche.

CObject* CAutoCompass::SearchObject(ObjectType type, D3DVECTOR center, float radius)
{
   CObject     *pObj, *pBest;
   D3DVECTOR   pos;
   float       min, dist;
   int         i;

   pBest = 0;
   min = 100000.0f;
   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetExplo() )  continue;

       if ( type != pObj->RetType() )  continue;

       pos = pObj->RetPosition(0);
       dist = Length(pos, center);

       if ( dist <= radius && dist < min )
       {
           min = dist;
           pBest = pObj;
       }
   }
   return pBest;
}


// Stoppe l'automate.

BOOL CAutoCompass::Abort()
{
   return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoCompass::RetError()
{
   return ERR_OK;
}

