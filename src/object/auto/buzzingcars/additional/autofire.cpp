// autofire.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "physics/physics.h"
#include "graphics/engine/terrain.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motionbot.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/building/autohome.h"
#include "object/auto/buzzingcars/additional/autofire.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoFire::CAutoFire(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoFire::~CAutoFire()
{
}


// Détruit l'objet.

void CAutoFire::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoFire::Init()
{
   m_time          = 0.0f;
   m_progress      = 0.0f;
   m_phase         = AFIP_WAIT;
   m_speed         = 1.0f/1.0f;
   m_progress      = 0.0f;
   m_lastParticle = 0.0f;
   m_lastPlouf     = 0.0f;
}


// Démarre l'objet.

void CAutoFire::Start(int param)
{
}


// Gestion d'un événement.

bool CAutoFire::EventProcess(const Event &event)
{
   COldObject*    target;
   COldObject*    car;
   CPhysics*   physics;
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   float       s, angle, dist;
   int         i, channel;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   car = dynamic_cast<COldObject*>(m_object->GetTransporter());
   if ( car == 0 )  return true;

   m_progress += event.rTime*m_speed;

   if ( m_phase == AFIP_WAIT )
   {
       physics = car->GetPhysics();
       if ( physics == 0 )
       {
           s = 0.0f;
       }
       else
       {
           s = abs(physics->GetLinMotionX(MO_REASPEED));
       }

       if ( s == 0.0f )
       {
           target = SearchObject(OBJECT_HOME1, car->GetPartPosition(0), 80.0f);
           if ( target != 0 )
           {
               car->SetLock(true);

               m_targetPos = target->GetPartPosition(0);
               pos = car->GetPartPosition(0);
               m_startAngleH = Math::NormAngle(m_object->GetPartRotationY(1));
               m_goalAngleH = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
               m_goalAngleH -= car->GetPartRotationY(0);
               m_goalAngleH += Math::PI*4.0f;  // +2 tour = rigolo
               m_startAngleV = 20.0f*Math::PI/180.0f;
               m_goalAngleV = 40.0f*Math::PI/180.0f;

               dist = Math::DistanceProjected(pos, m_targetPos);
               m_trackMass = 1500.0f*(3.0f-dist/40.0f);

               i = m_sound->Play(SOUND_MANIP, car->GetPartPosition(0), 0.0f, 0.3f, true);
               m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.8f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

               m_cameraType = m_camera->GetType();
               m_camera->SetType(Gfx::CAM_TYPE_BACK);
               angle = Math::NormAngle(m_goalAngleH);
               if ( angle < Math::PI )  angle -= Math::PI*0.15f;
               else               angle += Math::PI*0.15f;
               m_camera->SetBackHoriz(-angle);
               m_camera->SetLockRotate(true);
               m_main->SetStopwatch(false);  // stoppe le chrono

               m_phase = AFIP_TURNIN;
               m_speed = 1.0f/1.0f;
               m_progress = 0.0f;
           }
       }
   }

   if ( m_phase == AFIP_WAIT    ||
        m_phase == AFIP_WAITOUT )
   {
       angle = m_object->GetPartRotationY(1);
       angle += event.rTime;
       m_object->SetPartRotationY(1, angle);
   }

   if ( m_phase == AFIP_TURNIN )
   {
       if ( m_progress < 1.0f )
       {
           angle = m_startAngleH+(m_goalAngleH-m_startAngleH)*m_progress;
           m_object->SetPartRotationY(1, angle);

           angle = m_startAngleV+(m_goalAngleV-m_startAngleV)*m_progress;
           m_object->SetPartRotationZ(2, angle);
       }
       else
       {
           m_object->SetPartRotationY(1, m_goalAngleH);
           m_object->SetPartRotationZ(2, m_goalAngleV);

           i = m_sound->Play(SOUND_SPRINKLE, car->GetPartPosition(0), 0.0f, 0.3f, true);
           m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.5f, 1.0f, 3.8f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

           m_phase = AFIP_SPRINKLE;
           m_speed = 1.0f/4.0f;
           m_progress = 0.0f;
           m_lastParticle = 0.0f;
           m_lastPlouf = 0.1f;  // pas tout de suite
           m_lastSound = 0.1f;
           m_lastBreakdown = 0.2f;
       }
   }

   if ( m_phase == AFIP_SPRINKLE )
   {
       if ( m_progress < 1.0f )
       {
           if ( m_lastParticle+m_engine->ParticleAdapt(0.025f) <= m_progress )
           {
               m_lastParticle = m_progress;

               for ( i=0 ; i<4 ; i++ )
               {
                   mat = m_object->GetWorldMatrix(2);
                   pos = Math::Vector(2.0f, 0.0f, 0.0f);
                   pos = Transform(*mat, pos);

                   speed = Math::Vector(200.0f, 0.0f, 0.0f);
                   speed.x += (Math::Rand()-0.5f)*24.0f;
                   speed.y += (Math::Rand()-0.5f)*48.0f;
                   speed.z += (Math::Rand()-0.5f)*48.0f;
                   speed = Transform(*mat, speed);
                   speed -= pos;

                   dim.x = 1.0f;
                   dim.y = dim.x;
                   channel = m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK13,
                                                      2.0f, m_trackMass, 0.1f, 2.0f);
                   m_particle->SetObjectFather(channel, m_object);
               }
           }

           if ( m_lastPlouf+m_engine->ParticleAdapt(0.05f) <= m_progress )
           {
               m_lastPlouf = m_progress;

               pos = m_targetPos;
               pos.x += (Math::Rand()-0.5f)*15.0f;
               pos.z += (Math::Rand()-0.5f)*15.0f;
               m_terrain->AdjustToFloor(pos);
               dim.x = 5.0f+5.0f*Math::Rand();  // hauteur
               dim.y = 3.0f+3.0f*Math::Rand();  // diamètre
               m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, Gfx::PARTIPLOUF0, 1.4f, 0.0f);
           }

           if ( m_lastSound <= m_progress )
           {
               m_lastSound = 10.0f;  // plus jamais
               m_sound->Play(SOUND_PLOUF, m_targetPos, 1.0f);
           }

           if ( m_lastBreakdown < m_progress )
           {
               m_lastBreakdown = 10.0f;  // plus jamais
               BreakDown();
           }
       }
       else
       {
           i = m_sound->Play(SOUND_MANIP, car->GetPartPosition(0), 0.0f, 0.3f, true);
           m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.8f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

           m_phase = AFIP_TURNOUT;
           m_speed = 1.0f/1.0f;
           m_progress = 0.0f;
       }
   }

   if ( m_phase == AFIP_TURNOUT )
   {
       if ( m_progress < 1.0f )
       {
           angle = m_startAngleH+(m_goalAngleH-m_startAngleH)*(1.0f-m_progress);
           m_object->SetPartRotationY(1, angle);

           angle = m_startAngleV+(m_goalAngleV-m_startAngleV)*(1.0f-m_progress);
           m_object->SetPartRotationZ(2, angle);
       }
       else
       {
           car->SetLock(false);
           m_object->SetPartRotationY(1, m_startAngleH);
           m_object->SetPartRotationZ(2, m_startAngleV);

           m_camera->SetType(m_cameraType);
           m_camera->SetBackHoriz(0.0f);
           m_camera->SetLockRotate(false);
           m_camera->ResetLockRotate();
           m_main->SetStopwatch(true);  // redémarre le chrono

           m_phase = AFIP_WAITOUT;
           m_speed = 1.0f/4.0f;
           m_progress = 0.0f;
       }
   }

   if ( m_phase == AFIP_WAITOUT )
   {
       if ( m_progress >= 1.0f )
       {
           m_phase = AFIP_WAIT;
           m_speed = 1.0f/1.0f;
           m_progress = 0.0f;
       }
   }

   return true;
}


// Cherche un objet proche.

COldObject* CAutoFire::SearchObject(ObjectType type, Math::Vector center, float radius)
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
       if ( pObj->GetLock() )  continue;  // déjà stoppé ?

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


// Casse la maison.

void CAutoFire::BreakDown()
{
   COldObject*    home;
   CAuto*      automat;

   home = SearchObject(OBJECT_HOME1, m_targetPos, 10.0f);
   if ( home == 0 )  return;

   automat = home->GetAuto();
   if ( automat == 0 )  return;

   automat->Start(AHOP_BREAKDOWN);
}


// Stoppe l'automate.

bool CAutoFire::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoFire::GetError()
{
   return ERR_OK;
}

