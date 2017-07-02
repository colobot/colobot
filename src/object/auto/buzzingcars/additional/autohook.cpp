// autohook.cpp

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
#include "object/auto/buzzingcars/additional/autohook.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoHook::CAutoHook(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoHook::~CAutoHook()
{
}


// Détruit l'objet.

void CAutoHook::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoHook::Init()
{
   m_time          = 0.0f;
   m_progress      = 0.0f;
   m_phase         = AHKP_WAIT;
   m_speed         = 1.0f/1.0f;
   m_lastParticle = 0.0f;
   m_targetPos.x   = NAN;
   m_load          = 0;
}


// Démarre l'objet.

void CAutoHook::Start(int param)
{
}


// Gestion d'un événement.

bool CAutoHook::EventProcess(const Event &event)
{
   COldObject*    car;
   CPhysics*   physics;
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   float       s, angle, goalAngle, progress, dist, height;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;
   if ( m_main->GetEndingGame() )  return true;

   car = dynamic_cast<COldObject*>(m_object->GetTransporter());
   if ( car == 0 )  return true;

   if ( m_load != 0 )
   {
       if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
       {
           m_lastParticle = m_time;

           mat = m_object->GetWorldMatrix(3);
           pos = Transform(*mat, Math::Vector(0.0f, 0.0f, 0.0f));
           speed.x = (Math::Rand()-0.5f)*10.0f;
           speed.z = (Math::Rand()-0.5f)*10.0f;
           speed.y = 2.0f+Math::Rand()*5.0f;
           dim.x = 0.2f;
           dim.y = 0.2f;
           m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 10.0f);
       }
   }

   m_progress += event.rTime*m_speed;
   progress = Math::Norm(m_progress);

   if ( m_phase == AHKP_WAIT )
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
           if ( m_load == 0 )  // prend ?
           {
               m_target = SearchObject(OBJECT_BOT1, car->GetPartPosition(0), 0.0f, 14.0f);
               if ( m_target != 0 )
               {
                   car->SetLock(true);  // voiture stoppée
                   m_target->SetLock(true);  // robot pris

                   m_targetPos = m_target->GetPartPosition(0);
                   pos = car->GetPartPosition(0);
                   goalAngle = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
                   goalAngle -= car->GetPartRotationY(0);

                   m_cameraType = m_camera->GetType();
                   m_camera->SetType(Gfx::CAM_TYPE_BACK);
                   angle = Math::NormAngle(goalAngle);
                   if ( angle < Math::PI )  angle =  Math::PI*0.1f;
                   else               angle = -Math::PI*0.1f;
//TODO (krzys_h):                   m_camera->SetBackHoriz(angle);
//TODO (krzys_h):                   m_camera->SetLockRotate(true);
                   m_main->SetStopwatch(false);  // stoppe le chrono

                   pos = car->GetCharacter()->posFret;
                   mat = car->GetWorldMatrix(0);
                   pos = Transform(*mat, pos);
                   m_goalAngle = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
                   m_goalAngle -= car->GetPartRotationY(0);
                   m_goalAngle += Math::PI;
                   m_goalAngle = Math::NormAngle(m_goalAngle);
                   if ( m_goalAngle < Math::PI )
                   {
                       m_startAngle = m_object->GetPartRotationY(1);
                   }
                   else
                   {
                       m_startAngle = -m_object->GetPartRotationY(1);
                       m_goalAngle -= Math::PI*2.0f;
                   }

                   m_goalDist = -Math::DistanceProjected(pos, m_targetPos);
                   pos = m_object->GetPartPosition(2);
                   m_startDist = pos.x;

                   SoundManip(1.0f, 0.8f, 1.0f);

                   m_bGrab = true;
                   m_phase = AHKP_GOSTART;
                   m_speed = 1.0f/1.0f;
                   m_progress = 0.0f;
                   progress = 0.0f;
               }
           }
           else    // dépose ?
           {
               m_target = SearchObject(OBJECT_REPAIR, car->GetPartPosition(0), 5.0f, 14.0f);
               if ( m_target != 0 )
               {
                   car->SetLock(true);  // voiture stoppée

                   m_targetPos = m_target->GetPartPosition(0);
                   pos = car->GetPartPosition(0);
                   goalAngle = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
                   goalAngle -= car->GetPartRotationY(0);

                   m_cameraType = m_camera->GetType();
                   m_camera->SetType(Gfx::CAM_TYPE_BACK);
                   angle = Math::NormAngle(goalAngle);
                   if ( angle < Math::PI )  angle =  Math::PI*0.1f;
                   else               angle = -Math::PI*0.1f;
//TODO (krzys_h):                   m_camera->SetBackHoriz(angle);
//TODO (krzys_h):                   m_camera->SetLockRotate(true);
                   m_main->SetStopwatch(false);  // stoppe le chrono

                   pos = car->GetCharacter()->posFret;
                   mat = car->GetWorldMatrix(0);
                   pos = Transform(*mat, pos);
                   m_goalAngle = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
                   m_goalAngle -= car->GetPartRotationY(0);
                   m_goalAngle += Math::PI;
                   m_goalAngle = Math::NormAngle(m_goalAngle);
                   if ( m_goalAngle < Math::PI )
                   {
                       m_startAngle = m_object->GetPartRotationY(1);
                   }
                   else
                   {
                       m_startAngle = -m_object->GetPartRotationY(1);
                       m_goalAngle -= Math::PI*2.0f;
                   }

                   m_goalDist = -Math::DistanceProjected(pos, m_targetPos);
                   pos = m_object->GetPartPosition(2);
                   m_startDist = pos.x;

                   SoundManip(1.0f, 0.8f, 1.0f);

                   m_bGrab = false;
                   m_phase = AHKP_GOSTART;
                   m_speed = 1.0f/1.0f;
                   m_progress = 0.0f;
                   progress = 0.0f;
               }
           }
       }
   }

   if ( m_phase == AHKP_GOSTART )
   {
       angle = m_startAngle+(m_goalAngle-m_startAngle)*progress;
       m_object->SetPartRotationY(1, angle);

       dist = m_startDist+(m_goalDist-m_startDist)*progress;
       m_object->SetPartPosition(2, Math::Vector(dist, 0.9f, 0.0f));

       if ( m_progress >= 1.0f )
       {
           if ( m_bGrab )  // prend ?
           {
               mat = m_object->GetWorldMatrix(3);
               pos = Transform(*mat, Math::Vector(0.0f, 1.2f, 0.0f));
               m_goalHeight = m_targetPos.y-pos.y-2.0f;
               m_startHeight = -2.1f;
           }
           else    // dépose ?
           {
               mat = m_object->GetWorldMatrix(3);
               pos = Transform(*mat, Math::Vector(0.0f, 1.2f, 0.0f));
               m_goalHeight = m_targetPos.y-pos.y+2.2f;
               m_startHeight = -2.1f;
               StartAction(MB_WALK1, 1.0f);  // cassé au sol
           }

           SoundManip(0.5f, 0.8f, 1.5f);

           m_phase = AHKP_DOWN;
           m_speed = 1.0f/1.0f;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   if ( m_phase == AHKP_DOWN )
   {
       height = m_startHeight+(m_goalHeight-m_startHeight)*Math::Bounce(progress);
       m_object->SetPartPosition(3, Math::Vector(0.0f, height, 0.0f));

       if ( m_progress >= 1.0f )
       {
           car = dynamic_cast<COldObject*>(m_object->GetTransporter());

           if ( m_bGrab )  // prend ?
           {
               m_load = m_target;
               m_load->SetTransporter(m_object);
               m_load->SetTransporterPart(3);  // prend avec le bout du bras
               m_load->SetPartPosition(0, Math::Vector(0.0f, 1.2f, 0.0f));
               m_goalRot = Math::PI*0.5f;
               m_startRot = m_load->GetPartRotationY(0)-car->GetPartRotationY(0)-m_object->GetPartRotationY(1);
               m_startRot = Math::NormAngle(m_startRot);
               if ( m_startRot > Math::PI )  m_startRot -= Math::PI*2.0f;
               m_load->SetPartRotationY(0, m_startRot);
               StartAction(MB_WALK2, 1.0f);  // cassé transporté
           }
           else    // dépose ?
           {
               mat = m_object->GetWorldMatrix(3);
               pos = Transform(*mat, Math::Vector(0.0f, 1.2f, 0.0f));
               m_terrain->AdjustToFloor(pos);
               pos.y += m_load->GetCharacter()->height;
               m_load->SetTransporter(0);
               m_load->SetPartPosition(0, pos);
               m_load->SetPartRotationY(0, car->GetPartRotationY(0)+m_object->GetPartRotationY(1)+m_load->GetPartRotationY(0));
               m_load = 0;
           }

           SoundManip(0.5f, 0.8f, 1.5f);

           m_phase = AHKP_UP;
           m_speed = 1.0f/1.0f;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   if ( m_phase == AHKP_UP )
   {
       height = m_startHeight+(m_goalHeight-m_startHeight)*(1.0f-Math::Bounce(progress));
       m_object->SetPartPosition(3, Math::Vector(0.0f, height, 0.0f));

       if ( m_bGrab )  // prend ?
       {
           angle = m_startRot+(m_goalRot-m_startRot)*progress;
           m_load->SetPartRotationY(0, angle);
       }

       if ( m_progress >= 1.0f )
       {
           car = dynamic_cast<COldObject*>(m_object->GetTransporter());

           if ( m_bGrab )  // prend ?
           {
               m_startAngle = m_goalAngle;
               m_goalAngle = 0.0f;

               m_startDist = m_goalDist;
               m_goalDist = car->GetCharacter()->hookDist;

               m_startHeight = m_goalHeight;
               m_goalHeight = -2.1f;
           }
           else    // dépose ?
           {
               m_startAngle = m_goalAngle;
               if ( m_startAngle < 0.0f )  m_goalAngle = -Math::PI;
               else                        m_goalAngle =  Math::PI;

               m_startDist = m_goalDist;
               m_goalDist = -8.0f;

               m_startHeight = m_goalHeight;
               m_goalHeight = -2.1f;
           }

           SoundManip(1.0f, 0.8f, 1.0f);

           m_phase = AHKP_GOEND;
           m_speed = 1.0f/1.0f;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   if ( m_phase == AHKP_GOEND )
   {
       angle = m_startAngle+(m_goalAngle-m_startAngle)*progress;
       m_object->SetPartRotationY(1, angle);

       dist = m_startDist+(m_goalDist-m_startDist)*progress;
       m_object->SetPartPosition(2, Math::Vector(dist, 0.9f, 0.0f));

       if ( m_progress >= 1.0f )
       {
           car->SetLock(false);  // voiture de nouveau mobile

           m_camera->SetType(m_cameraType);
//TODO (krzys_h):           m_camera->SetBackHoriz(0.0f);
//TODO (krzys_h):           m_camera->SetLockRotate(false);
//TODO (krzys_h):           m_camera->ResetLockRotate();
           m_main->SetStopwatch(true);  // redémarre le chrono

           m_phase = AHKP_WAIT;
           m_speed = 1.0f/4.0f;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   return true;
}


// Cherche un objet proche.

COldObject* CAutoHook::SearchObject(ObjectType type, Math::Vector center,
                                float minRadius, float maxRadius)
{
   COldObject     *pObj, *pBest;
   CMotion*    motion;
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
       if ( pObj->GetLock() )  continue;  // déjà pris ?

       if ( type == OBJECT_BOT1 )
       {
           motion = dynamic_cast<COldObject*>(pObj)->GetMotion();
           if ( motion == 0 )  continue;
           if ( motion->GetAction() != MB_WALK1 )  continue;
       }

       pos = pObj->GetPosition();
       dist = Math::DistanceProjected(pos, center);

       if ( dist >= minRadius && dist <= maxRadius && dist < min )
       {
           min = dist;
           pBest = dynamic_cast<COldObject*>(pObj);
       }
   }
   return pBest;
}

// Démarre une action pour le robot transporté.

void CAutoHook::StartAction(int action, float delay)
{
   CMotion*    motion;

   if ( m_load == 0 )  return;

   motion = m_load->GetMotion();
   if ( motion == 0 )  return;

   motion->SetAction(action, delay);
}

// Fait entendre le son du bras manipulateur.

void CAutoHook::SoundManip(float time, float amplitude, float frequency)
{
   COldObject*    car;
   int         i;

   car = dynamic_cast<COldObject*>(m_object->GetTransporter());
   if ( car == 0 )  return;

   i = m_sound->Play(SOUND_MANIP, car->GetPartPosition(0), 0.0f, 0.3f*frequency, true);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}


// Stoppe l'automate.

bool CAutoHook::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoHook::GetError()
{
   return ERR_OK;
}

