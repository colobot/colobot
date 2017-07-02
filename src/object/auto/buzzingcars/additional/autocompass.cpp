// autocompass.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "physics/physics.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motionbot.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/additional/autocompass.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoCompass::CAutoCompass(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoCompass::~CAutoCompass()
{
}


// Détruit l'objet.

void CAutoCompass::DeleteObject(bool bAll)
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

bool CAutoCompass::EventProcess(const Event &event)
{
   COldObject*    car;
   COldObject*    target;
   CPhysics*   physics;
   Math::Vector   pos;
   float       speed, angle;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   m_progress += event.rTime*m_speed;
   m_lastDetect += event.rTime;

   if ( m_lastDetect > 0.5f )
   {
       m_lastDetect = 0.0f;

       target = SearchObject(OBJECT_UFO, m_object->GetPartPosition(0), 10000.0f);
       if ( target == 0 )
       {
           m_targetPos.x = NAN;
       }
       else
       {
           m_targetPos = target->GetPartPosition(0);
       }
   }

   car = dynamic_cast<COldObject*>(m_object->GetTransporter());
   if ( car == 0 )  // boussole seule ?
   {
       if ( m_targetPos.x == NAN )
       {
           angle = 0.0f;
       }
       else
       {
           pos = m_object->GetPartPosition(0);
           angle = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
       }

       m_object->SetPartRotationY(1, angle);
   }
   else    // boussole sur une voiture ?
   {
       physics = car->GetPhysics();
       if ( physics == 0 )
       {
           speed = 0.0f;
       }
       else
       {
           speed = abs(physics->GetLinMotionX(MO_REASPEED)/physics->GetLinMotionX(MO_ADVSPEED));
       }

       if ( m_targetPos.x == NAN )
       {
           angle = 0.0f;
       }
       else
       {
           pos = car->GetPartPosition(0);
           angle = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
           angle -= car->GetPartRotationY(0);
       }

       m_carSpeed = Math::Smooth(m_carSpeed, speed, event.rTime*0.5f);

       m_angle += (10.0f-m_carSpeed*8.0f)*event.rTime;;
       angle += sinf(m_angle)*m_carSpeed*Math::PI*2.0f;
       angle = Math::Smooth(m_object->GetPartRotationY(1), angle, event.rTime*5.0f);
       m_object->SetPartRotationY(1, angle);
   }

   return true;
}


// Cherche un objet proche.

COldObject* CAutoCompass::SearchObject(ObjectType type, Math::Vector center, float radius)
{
   COldObject     *pObj, *pBest;
   Math::Vector   pos;
   float       min, dist;
   int         i;

   pBest = 0;
   min = 100000.0f;
   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       if ( type != pObj->GetType() )  continue;
       assert(pObj->Implements(ObjectInterfaceType::Old));
       if ( dynamic_cast<COldObject*>(pObj)->IsDying() )  continue;

       pos = pObj->GetPosition();
       dist = Math::Distance(pos, center);

       if ( dist <= radius && dist < min )
       {
           min = dist;
           pBest = dynamic_cast<COldObject*>(pObj);
       }
   }
   return pBest;
}


// Stoppe l'automate.

bool CAutoCompass::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoCompass::GetError()
{
   return ERR_OK;
}

