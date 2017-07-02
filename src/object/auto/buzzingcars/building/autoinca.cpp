// autoinca.cpp

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
#include "object/auto/buzzingcars/building/autoinca.h"
#include "graphics/engine/pyro_manager.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoInca::CAutoInca(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoInca::~CAutoInca()
{
}


// Détruit l'objet.

void CAutoInca::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoInca::Init()
{
   m_time          = 0.0f;
   m_progress      = 0.0f;
   m_phase         = AINP_WAIT;
   m_speed         = 1.0f/1.0f;
   m_lastParticule = 0.0f;
   m_targetPos.x   = NAN;
   m_channelRay    = -1;
   m_vehicle       = 0;
   m_bBlitz        = false;
}


// Démarre l'objet.

void CAutoInca::Start(int param)
{
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   float       duration, mass;
   int         i;

   if ( param == 2 && !m_bBlitz )  // bouton rouge pressé ?
   {
       m_bBlitz = true;

       m_king = SearchObject(OBJECT_EVIL3, m_object->GetPartPosition(0), 100.0f);
       if ( m_king == 0 )  return;
       m_vehicle = SearchObject(OBJECT_CAR, m_object->GetPartPosition(0), 100.0f);
       if ( m_vehicle == 0 )  return;
       m_vehicle->SetLock(true);

       CameraBegin();
       SoundManip(2.0f, 1.0f, 1.0f);

       m_object->SetPartRotationY(1, (Math::Rand()-0.5f)*1.0f);
       m_object->SetPartRotationZ(1, (Math::Rand()-0.5f)*1.0f);

       mat = m_object->GetWorldMatrix(1);  // bouton rouge
       for ( i=0 ; i<15 ; i++ )
       {
           pos.x = 0.0f;
           pos.y = (Math::Rand()-0.5f)*5.0f;
           pos.z = (Math::Rand()-0.5f)*5.0f;
           pos = Math::Transform(*mat, pos);
           speed.x = (Math::Rand()-0.5f)*30.0f;
           speed.z = (Math::Rand()-0.5f)*30.0f;
           speed.y = Math::Rand()*30.0f;
           dim.x = 1.0f;
           dim.y = dim.x;
           duration = Math::Rand()*3.0f+2.0f;
           mass = Math::Rand()*10.0f+15.0f;
           m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK1,
                                    duration, mass, Math::Rand()+0.7f, 1.0f);
       }
       m_sound->Play(SOUND_BREAK2, m_engine->GetEyePt(), 1.0f, 0.7f);

       m_object->SetLock(true);  // il ne sera plus compté !

       m_phase    = AINP_OPEN;
       m_speed    = 1.0f/2.0f;
       m_progress = 0.0f;
   }
}


// Gestion d'un événement.

bool CAutoInca::EventProcess(const Event &event)
{
   Math::Vector   pos;
   Math::Point      dim;
   float       progress, angle, delay;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;
   if ( m_main->GetEndingGame() )  return true;

   m_progress += event.rTime*m_speed;
   progress = Math::Norm(m_progress);

   if ( m_phase == AINP_WAIT )
   {
   }

   if ( m_phase == AINP_OPEN )
   {
       angle = progress*60.0f*Math::PI/180.0f;
       m_object->SetPartRotationX(3,  angle);
       m_object->SetPartRotationX(4, -angle);

       if ( progress >= 1.0f )
       {
           pos = m_object->GetPartPosition(0);
           pos.y += 42.0f;
           dim.x = 25.0f;
           dim.y = dim.x;
           m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, Gfx::PARTIGLINTb, 5.0f, 0.0f);

           SoundManip(2.0f, 0.5f, 0.8f);
           m_phase    = AINP_DOWN;
           m_speed    = 1.0f/2.0f;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   if ( m_phase == AINP_DOWN )
   {
       pos.x = 0.0f;
       pos.y = 10.0f-Math::Bounce(progress)*15.0f;
       pos.z = 0.0f;
       m_object->SetPartPosition(2, pos);
       m_object->SetPartRotationY(2, progress*Math::PI*1.0f);

       if ( progress >= 1.0f )
       {
           CreateBlitz(2.0f);
           m_phase    = AINP_BLITZ;
           m_speed    = 1.0f/2.0f;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   if ( m_phase == AINP_BLITZ )
   {
       ProgressBlitz(progress);

       if ( progress >= 1.0f )
       {
           m_vehicle->SetLock(false);
           CameraEnd();
           CreateBlitz(2.0f);
           m_phase    = AINP_TERM;
           m_speed    = 1.0f/2.0f;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   if ( m_phase == AINP_TERM )
   {
       ProgressBlitz(progress);

       if ( progress >= 1.0f )
       {
           delay = 2.0f+Math::Rand()*2.0f;
           CreateBlitz(delay);
           m_phase    = AINP_TERM;
           m_speed    = 1.0f/delay;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   return true;
}


// Cherche un objet proche.

COldObject* CAutoInca::SearchObject(ObjectType type, Math::Vector center, float radius)
{
   COldObject     *pBest;
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

// Début du cadrage pour la caméra.

void CAutoInca::CameraBegin()
{
   m_cameraType = m_camera->GetType();
   m_camera->SetControllingObject(m_object);
   m_camera->SetType(Gfx::CAM_TYPE_BACK);
   m_camera->SetSmooth(Gfx::CAM_SMOOTH_SOFT);

   m_main->SetStopwatch(false);  // stoppe le chrono
}

// Fin du cadrage pour la caméra.

void CAutoInca::CameraEnd()
{
   m_camera->SetControllingObject(m_vehicle);
   m_camera->SetType(m_cameraType);
   m_camera->SetSmooth(Gfx::CAM_SMOOTH_NORM);

   m_main->SetStopwatch(true);  // redémarre le chrono
}

// Crée les éclairs.

void CAutoInca::CreateBlitz(float delay)
{
   Math::Vector   pos, src, dst, eye;
   Math::Point      dim;

   pos = m_object->GetPartPosition(0);
   pos.y += 42.0f;
   dim.x = 8.5f;
   dim.y = dim.x;
   m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f),
                                dim, Gfx::PARTISPHERE1, delay, 0.0f);

   src = m_object->GetPartPosition(0);
   src.y += 42.0f;
   dst = m_king->GetPartPosition(0);
   dst.y += 15.0f;
   m_channelRay = m_particle->CreateRay(src, dst, Gfx::PARTIRAY2, Math::Point(6.0f, 6.0f), delay);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {

       if ( !pObj->GetLock() )  continue;
       if ( pObj->GetType() != OBJECT_INCA7 )  continue;
       if ( Math::Rand() > 0.3f )  continue;

       dst = pObj->GetPosition();
       dst.y += 42.0f;
       m_particle->CreateRay(src, dst, Gfx::PARTIRAY2, Math::Point(6.0f, 6.0f), 0.5f);
   }

   pos = m_king->GetPartPosition(0);
   eye = m_engine->GetEyePt();
   pos = Math::SegmentPoint(pos, eye, Math::Distance(pos, eye)*0.5f);
   m_sound->Play(SOUND_EVIL1, pos, 1.0f, 1.0f+(Math::Rand()-0.5f)*0.8f);
   SoundBlitz(delay);
}

// Fait évoluer les éclairs.

void CAutoInca::ProgressBlitz(float progress)
{
   Math::Vector   src, dst;
   Math::Point      dim;
   float       h;

   h = (Math::Rand()-0.5f)*20.0f;
   src = m_object->GetPartPosition(0);
   src.y += 42.0f;
   dst = m_king->GetPartPosition(0);
   dst.y += 15.0f+h;
   dst.x += (Math::Rand()-0.5f)*8.0f;
   dst.z += (Math::Rand()-0.5f)*8.0f;

   dst = Math::SegmentPoint(dst, src, 11.0f-h*0.1f);
   m_particle->SetGoal(m_channelRay, dst);

   if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
   {
       m_lastParticule = m_time;

       if ( Math::Rand() < 0.5f )
       {
           dim.x = 6.0f;
           dim.y = dim.x;
           m_particle->CreateParticle(dst, Math::Vector(0.0f, 0.0f, 0.0f), dim, Gfx::PARTIGLINT, 1.0f, 0.0f);
       }
       else
       {
           dim.x = 3.0f;
           dim.y = dim.x;
           m_particle->CreateParticle(dst, Math::Vector(0.0f, 0.0f, 0.0f), dim, Gfx::PARTIBLITZb, 1.0f, 0.0f);
       }
   }
}

// Casse le méchant roi.

void CAutoInca::BreakDownEvil3()
{
   COldObject*    evil;
   Gfx::CPyro*      pyro;

   evil = SearchObject(OBJECT_EVIL3, m_targetPos, 10.0f);
   if ( evil == 0 )  return;

   m_engine->GetPyroManager()->Create(Gfx::PT_BURNO, evil);  // destruction totale
}

// Fait entendre le son du bras manipulateur.

void CAutoInca::SoundManip(float time, float amplitude, float frequency)
{
   Math::Vector   pos;
   int         i;

//?    pos = m_object->GetPartPosition(0);
   pos = m_engine->GetEyePt();

   i = m_sound->Play(SOUND_MANIP, pos, 0.0f, 0.3f*frequency, true);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}

// Fait entendre le son du rayon.

void CAutoInca::SoundBlitz(float time)
{
   Math::Vector   pos, eye;
   int         i;

   pos = m_object->GetPartPosition(0);
   eye = m_engine->GetEyePt();
   pos = Math::SegmentPoint(pos, eye, Math::Distance(pos, eye)*0.5f);

   i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, true);
   m_sound->AddEnvelope(i, 1.0f, 1.0f, 0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 1.0f, 1.0f, time-0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.1f, SOPER_STOP);
}


// Stoppe l'automate.

bool CAutoInca::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoInca::GetError()
{
   return ERR_OK;
}

