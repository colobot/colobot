// autoblitzer.cpp

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
#include "graphics/engine/pyro.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/additional/autoblitzer.h"
#include "graphics/engine/pyro_manager.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoBlitzer::CAutoBlitzer(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoBlitzer::~CAutoBlitzer()
{
}


// Détruit l'objet.

void CAutoBlitzer::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoBlitzer::Init()
{
   m_time          = 0.0f;
   m_progress      = 0.0f;
   m_phase         = ABLP_WAIT;
   m_speed         = 1.0f/1.0f;
   m_lastParticle = 0.0f;
   m_targetPos.x   = NAN;
   m_channelSphere = -1;
}


// Démarre l'objet.

void CAutoBlitzer::Start(int param)
{
}


// Gestion d'un événement.

bool CAutoBlitzer::EventProcess(const Event &event)
{
   COldObject*    target;
   COldObject*    car;
   COldObject*    inca;
   CPhysics*   physics;
   ObjectType  tType[5];
   Math::Matrix*  mat;
   Math::Vector   pos, speed, src, dst;
   Math::Point      dim;
   float       s, angle, goalAngle;
   float       tRadius[5];
   int         i;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;
   if ( m_main->GetEndingGame() )  return true;

   car = dynamic_cast<COldObject*>(m_object->GetTransporter());
   if ( car == 0 )  return true;

   m_progress += event.rTime*m_speed;

   if ( m_phase == ABLP_WAIT )
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
           tType[0] = OBJECT_ALIEN9;  tRadius[0] =  60.0f;
           tType[1] = OBJECT_EVIL1;   tRadius[1] = 120.0f;
           tType[2] = OBJECT_EVIL3;   tRadius[2] =  60.0f;
           target = SearchObject(3, tType, tRadius, car->GetPartPosition(0));
           if ( target != 0 )
           {
               m_targetType = target->GetType();

               if ( m_targetType == OBJECT_EVIL3 )
               {
                   tType[0] = OBJECT_INCA7;  tRadius[0] = 100.0f;
                   inca = SearchObject(1, tType, tRadius, car->GetPartPosition(0));
                   if ( inca != 0 )  return true;
               }

               car->SetLock(true);

               m_targetPos = target->GetPartPosition(0);
               pos = car->GetPartPosition(0);
               goalAngle = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
               goalAngle -= car->GetPartRotationY(0);

               m_cameraType = m_camera->GetType();
               m_camera->SetType(Gfx::CAM_TYPE_BACK);
               angle = Math::NormAngle(goalAngle);
               if ( angle < Math::PI )  angle -= Math::PI*0.15f;
               else               angle += Math::PI*0.15f;
               m_camera->SetBackHoriz(-angle);
               if ( m_targetType == OBJECT_EVIL3 )
               {
                   m_camera->SetBackDist(50.0f);
               }
               m_camera->SetLockRotate(true);
               m_main->SetStopwatch(false);  // stoppe le chrono

               mat = m_object->GetWorldMatrix(0);
               src = Transform(*mat, Math::Vector(0.0f, 6.8f, 0.0f));
               mat = target->GetWorldMatrix(0);
               if ( m_targetType == OBJECT_ALIEN9 )
               {
                   dst = Transform(*mat, Math::Vector(0.0f, 5.0f, -14.5f));
                   m_delayBlitz = 2.0f;
               }
               if ( m_targetType == OBJECT_EVIL1 )
               {
                   dst = Transform(*mat, Math::Vector(0.0f, 4.0f, 0.0f));
                   m_delayBlitz = 2.0f;
               }
               if ( m_targetType == OBJECT_EVIL3 )
               {
                   dst = Transform(*mat, Math::Vector(0.0f, 15.0f, 0.0f));
                   m_delayBlitz = 3.0f;
               }
               m_particle->CreateRay(src, dst, Gfx::PARTIRAY2, Math::Point(6.0f, 6.0f), m_delayBlitz);

               dim.x = 1.9f;
               dim.y = dim.x;
               m_channelSphere = m_particle->CreateParticle(src, Math::Vector(0.0f, 0.0f, 0.0f),
                                                dim, Gfx::PARTISPHERE1, m_delayBlitz+1.5f, 0.0f);

               pos = car->GetPartPosition(0);
               i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, true);
               m_sound->AddEnvelope(i, 1.0f, 1.0f, 0.1f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 1.0f, 1.0f, m_delayBlitz-0.1f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.1f, SOPER_STOP);

               if ( m_targetType == OBJECT_ALIEN9 )
               {
                   BreakDownFactory();
                   m_delayTotal = 5.0f;
               }
               if ( m_targetType == OBJECT_EVIL1 )
               {
                   BreakDownEvil1();
                   m_delayTotal = 2.0f;
               }
               if ( m_targetType == OBJECT_EVIL3 )
               {
                   m_delayTotal = 10.0f;
               }
               m_phase = ABLP_BLITZ;
               m_speed = 1.0f/m_delayTotal;
               m_progress = 0.0f;
               m_lastParticle = 0.0f;
           }
       }
   }

   if ( m_channelSphere != -1 )
   {
       mat = m_object->GetWorldMatrix(0);
       pos = Transform(*mat, Math::Vector(0.0f, 6.8f, 0.0f));
       m_particle->SetPosition(m_channelSphere, pos);
       /* TODO (krzys_h): original code
       if ( !m_particle->SetPosition(m_channelSphere, pos) )
       {
           m_channelSphere = -1;
       }*/
   }

   if ( m_phase == ABLP_WAIT    ||
        m_phase == ABLP_WAITOUT )
   {
       angle = m_object->GetPartRotationY(1);
       angle += event.rTime;
       m_object->SetPartRotationY(1, angle);
   }

   if ( m_phase == ABLP_BLITZ )
   {
       if ( m_progress < m_delayBlitz/m_delayTotal )
       {
           if ( m_lastParticle+m_engine->ParticleAdapt(0.025f) <= m_progress )
           {
               m_lastParticle = m_progress;

               for ( i=0 ; i<4 ; i++ )
               {
                   mat = m_object->GetWorldMatrix(0);
                   pos = Transform(*mat, Math::Vector(0.0f, 6.8f, 0.0f));

                   speed.x = (Math::Rand()-0.5f)*10.0f;
                   speed.z = (Math::Rand()-0.5f)*10.0f;
                   speed.y = 5.0f+Math::Rand()*5.0f;
                   dim.x = 0.4f;
                   dim.y = 0.4f;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZb, 1.0f, 20.0f);
               }
           }
       }
       else
       {
           if ( m_targetType == OBJECT_EVIL3 )
           {
               BreakDownEvil3();
           }
       }
       if ( m_progress >= 1.0f )
       {
           car->SetLock(false);

           m_camera->SetType(m_cameraType);
           m_camera->SetBackHoriz(0.0f);
           m_camera->SetLockRotate(false);
           m_camera->ResetLockRotate();
           m_main->SetStopwatch(true);  // redémarre le chrono

           m_phase = ABLP_WAITOUT;
           m_speed = 1.0f/1.0f;
           m_progress = 0.0f;
       }
   }

   if ( m_phase == ABLP_WAITOUT )
   {
       if ( m_progress >= 1.0f )
       {
           m_phase = ABLP_WAIT;
           m_speed = 1.0f/1.0f;
           m_progress = 0.0f;
       }
   }

   return true;
}


// Cherche un objet proche.

COldObject* CAutoBlitzer::SearchObject(int total, ObjectType *type,
                                   float *radius, Math::Vector center)
{
   COldObject     *pObj, *pBest;
   ObjectType  oType;
   Math::Vector   pos;
   float       min, dist;
   int         i, j;

   pBest = 0;
   min = 100000.0f;
   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       if ( !pObj->Implements(ObjectInterfaceType::Old) ) continue;
       if ( dynamic_cast<COldObject*>(pObj)->IsDying() )  continue;
       if ( pObj->GetLock() )  continue;  // déjà stoppé ?

       oType = pObj->GetType();

       for ( j=0 ; j<total ; j++ )
       {
           if ( type[j] == oType )
           {
               pos = pObj->GetPosition();
               dist = Math::Distance(pos, center);

               if ( dist <= radius[j] && dist < min )
               {
                   min = dist;
                   pBest = dynamic_cast<COldObject*>(pObj);
               }
           }
       }
   }
   return pBest;
}

// Casse la fabrique instrument de torture.

void CAutoBlitzer::BreakDownFactory()
{
   COldObject*    home;
   CAuto*      automat;
   ObjectType  tType[2];
   float       tRadius[2];

   tType[0]   = OBJECT_ALIEN9;
   tRadius[0] = 10.0f;
   home = SearchObject(1, tType, tRadius, m_targetPos);
   if ( home == 0 )  return;

   automat = home->GetAuto();
   if ( automat == 0 )  return;

   automat->Start(2);
}

// Casse le méchant vilain pas bô.

void CAutoBlitzer::BreakDownEvil1()
{
   COldObject*    evil;
   Gfx::CPyro*      pyro;
   ObjectType  tType[2];
   float       tRadius[2];

   tType[0]   = OBJECT_EVIL1;
   tRadius[0] = 10.0f;
   evil = SearchObject(1, tType, tRadius, m_targetPos);
   if ( evil == 0 )  return;

   evil->StopProgram();

   m_engine->GetPyroManager()->Create(Gfx::PT_BURNO, evil);  // destruction totale
}

// Casse le méchant roi.

void CAutoBlitzer::BreakDownEvil3()
{
   COldObject*    evil;
   Gfx::CPyro*      pyro;
   ObjectType  tType[2];
   float       tRadius[2];

   tType[0]   = OBJECT_EVIL3;
   tRadius[0] = 10.0f;
   evil = SearchObject(1, tType, tRadius, m_targetPos);
   if ( evil == 0 )  return;

   evil->DetachPiece(1, 0, Math::Vector(0.0f, 0.0f, 0.0f));

   m_engine->GetPyroManager()->Create(Gfx::PT_FRAGO, evil);  // destruction totale
}


// Stoppe l'automate.

bool CAutoBlitzer::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoBlitzer::GetError()
{
   return ERR_OK;
}

