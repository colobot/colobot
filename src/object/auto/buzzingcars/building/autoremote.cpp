// autoremote.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "common/restext.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "physics/physics.h"
#include "object/motion/motion.h"
#include "object/motion/motionhuman.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "ui/displaytext.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/building/autoremote.h"
#include "object/object_manager.h"




// Constructeur de l'objet.

CAutoRemote::CAutoRemote(COldObject* object)
                        : CAuto(object)
{
   int     i;

   for ( i=0 ; i<6 ; i++ )
   {
       m_partiStop[i] = -1;
   }
   m_lastParticule = 0.0f;
   m_lastSound = -10.0f;
   m_bDisplayText = false;
   m_bStartDemoWheel = false;
   m_bForceSlow = false;
   m_vehicle = 0;
   m_human = 0;
   m_param = 0;

   Init();
}

// Destructeur de l'objet.

CAutoRemote::~CAutoRemote()
{
}


// Détruit l'objet.

void CAutoRemote::DeleteObject(bool bAll)
{
   m_phase = ARMP_WAIT;
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoRemote::Init()
{
   m_time = 0.0f;

   m_phase = ARMP_WAIT;
   m_progress = 0.0f;
   m_speed    = 1.0f/1.0f;

   m_main->SetStopwatch(false);  // stoppe le chrono
//TODO (krzys_h):   m_main->SetMovieAuto(this);
}


// Démarre l'objet.

void CAutoRemote::Start(int param)
{
   if ( param == 99 )  // stoppe le film ?
   {
       if ( m_vehicle == 0 )
       {
           m_vehicle = SearchVehicle();
       }

       m_phase    = ARMP_CAMERA5;
       m_progress = 1.0f;
       m_speed    = 1.0f/1.0f;
       return;
   }

   m_param = param;
}


// Gestion d'un événement.

bool CAutoRemote::EventProcess(const Event &event)
{
   Math::Matrix*  mat;
   CMotion*    motion;
   SoundType       sound;
   int         i;
   float       angle;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;
   if ( m_param == 0 )  return true;

   m_progress += event.rTime*m_speed;

   if ( m_phase == ARMP_WAIT )
   {
       if ( !m_bDisplayText )
       {
           m_bDisplayText = true;
           std::string text;
           GetResource(RES_TEXT, RT_START_REMOTE, text);
           m_main->GetDisplayText()->DisplayText(text.c_str(), 8.0f);
       }

       if ( m_progress >= 1.0f )
       {
           m_human = SearchHuman();
           m_vehicle = SearchVehicle();
           if ( m_human == 0 || m_vehicle == 0 )
           {
               m_phase    = ARMP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/1.0f;
           }
           else
           {
//?                human->SetLock(true);
               m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);

               m_eyeStart = m_engine->GetEyePt();
               m_lookatStart = m_engine->GetLookatPt();

               // Derrière l'écran.
               mat = m_object->GetWorldMatrix(0);
//?                m_eyeGoal = Math::Transform(*mat, Math::Vector(0.0f, 8.0f, 0.0f));
//?                m_lookatGoal = Math::Transform(*mat, Math::Vector(2.0f, 7.0f, 0.0f));
               m_eyeGoal = Math::Transform(*mat, Math::Vector(2.0f, 7.0f, 5.0f));
               m_lookatGoal = Math::Transform(*mat, Math::Vector(4.0f, 6.0f, 0.0f));

               i = m_sound->Play(SOUND_RESEARCH, m_object->GetPartPosition(0), 0.0f, 1.0f, true);
               m_sound->AddEnvelope(i, 0.4f, 1.0f, 1.0f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 0.4f, 1.0f, 6.0f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 0.0f, 1.0f, 1.0f, SOPER_STOP);

               m_phase    = ARMP_CAMERA1;
               m_progress = 0.0f;
               m_speed    = 1.0f/6.0f;
           }
       }
   }

   if ( m_human != 0 )  // homme aux commandes ?
   {
       motion = m_human->GetMotion();
       if ( motion != 0 )
       {
           angle = 0.0f;
           /* TODO (krzys_h):
            * NOTE: This is part of changed motionhuman
           if ( motion->GetAction() == MHS_DRIVE1 )
           {
               angle = motion->GetActionProgress()*0.5f;
           }
           if ( motion->GetAction() == MHS_DRIVE2 )
           {
               angle = -motion->GetActionProgress()*0.5f;
           }
           */
           angle = Math::Smooth(m_object->GetPartRotationY(1), angle, event.rTime*2.0f);
           m_object->SetPartRotationY(1, angle);  // tourne le volant
       }
   }

   if ( m_phase == ARMP_CAMERA1 )
   {
       UpdateCamera(m_progress*3.8f);

       if ( m_progress >= 1.0f )
       {
           m_eyeStart = m_eyeGoal;
           m_lookatStart = m_lookatGoal;

           // Remote de 3/4.
           mat = m_object->GetWorldMatrix(0);
           m_eyeGoal = Math::Transform(*mat, Math::Vector(-8.0f, 6.0f, 8.0f));
           m_lookatGoal = Math::Transform(*mat, Math::Vector(10.0f, 6.0f, 8.0f));

           EmitParticle(true);
           m_sound->Play(SOUND_RADAR, m_object->GetPartPosition(0), 0.8f, 0.5f);

           m_phase    = ARMP_CAMERA2;
           m_progress = 0.0f;
           m_speed    = 1.0f/2.0f;
       }
   }

   if ( m_phase == ARMP_CAMERA2 )
   {
       UpdateCamera(m_progress*1.2f);
       EmitParticle(false);
       DemoWheel(false);

       if ( m_progress >= 1.0f )
       {
           m_eyeStart = m_eyeGoal;
           m_lookatStart = m_lookatGoal;

           // A gauche de la voiture sur le récepteur.
           mat = m_vehicle->GetWorldMatrix(0);
//?            m_eyeGoal = Math::Transform(*mat, Math::Vector(0.0f, 6.0f, 6.0f));
//?            m_lookatGoal = Math::Transform(*mat, Math::Vector(0.0f, 4.0f, 0.0f));
//?            m_eyeGoal = Math::Transform(*mat, Math::Vector(0.0f, 7.0f, 9.0f));
//?            m_lookatGoal = Math::Transform(*mat, Math::Vector(0.0f, 4.0f, 0.0f));
           m_eyeGoal = Math::Transform(*mat, Math::Vector(3.0f, 1.5f, 10.0f));
           m_lookatGoal = Math::Transform(*mat, Math::Vector(3.0f, 2.5f, 0.0f));

           m_sound->Play(SOUND_RADAR, m_object->GetPartPosition(0), 0.8f, 0.5f);

           i = m_sound->Play(SOUND_RESEARCH, m_vehicle->GetPartPosition(0), 0.0f, 1.0f, true);
           m_sound->AddEnvelope(i, 0.2f, 1.0f, 1.0f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.2f, 1.0f, 2.0f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.0f, 1.0f, 1.0f, SOPER_STOP);

           m_phase    = ARMP_CAMERA4;
           m_progress = 0.0f;
           m_speed    = 1.0f/5.0f;
       }
   }

   if ( m_phase == ARMP_CAMERA4 )
   {
       UpdateCamera(m_progress*3.0f);
       EmitParticle(false);
       DemoWheel(false);

       if ( m_progress >= 1.0f )
       {
           m_eyeStart = m_eyeGoal;
           m_lookatStart = m_lookatGoal;

           // Derrière la voiture.
           mat = m_vehicle->GetWorldMatrix(0);
           m_eyeGoal = Math::Transform(*mat, Math::Vector(-20.0f, 6.0f, 5.0f));
           m_lookatGoal = Math::Transform(*mat, Math::Vector(0.0f, 4.0f, 0.0f));

           m_phase    = ARMP_CAMERA5;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.0f;
       }
   }

   if ( m_phase == ARMP_CAMERA5 )
   {
       UpdateCamera(m_progress*1.0f);
       EmitParticle(false);

       if ( m_progress >= 1.0f )
       {
           DemoWheel(true);  // remet au neutre
           m_main->SelectObject(m_vehicle);

           std::string text;
           GetResource(RES_TEXT, RT_START_GO, text);
           m_main->GetDisplayText()->DisplayText(text.c_str(), 3.0f, Ui::TT_START);
           sound = SOUND_STARTGO;
//TODO (krzys_h):           if ( !m_sound->GetComments() )  sound = SOUND_MESSAGE;
           m_sound->Play(sound, m_engine->GetLookatPt(), 1.0f, 1.0f);
           
           m_main->SetStopwatch(true);  // démarre le chrono
//TODO (krzys_h):           m_main->SetMovieAuto(0);

           m_phase    = ARMP_TERM;
           m_progress = 0.0f;
           m_speed    = 1.0f/100.0f;
       }
   }

   if ( m_phase == ARMP_TERM )
   {
       EmitParticle(false);
   }

   return true;
}

// Stoppe l'automate.

bool CAutoRemote::Abort()
{
   return true;
}


// Met à jour la caméra.

void CAutoRemote::UpdateCamera(float progress)
{
   progress = Math::Norm(progress);  // 0..1
   m_camera->SetScriptCamera(
       m_eyeStart+(m_eyeGoal-m_eyeStart)*progress,
       m_lookatStart+(m_lookatGoal-m_lookatStart)*progress);
   m_engine->SetFocus(1.0f);
}

// Emission de particule de l'émetteur vers la voiture.

void CAutoRemote::EmitParticle(bool bFirst)
{
   Math::Matrix*  mat;
   Character*  character;
   Math::Vector   src, dst, pos, speed;
   Math::Point      dim;
   float       duration;
   int         i;

   if ( m_vehicle == 0 )  return;

   mat = m_object->GetWorldMatrix(0);
   src = Math::Transform(*mat, Math::Vector(4.0f, 8.9f, 4.0f));

   character = m_vehicle->GetCharacter();
   mat = m_vehicle->GetWorldMatrix(0);
   dst = Math::Transform(*mat, character->antenna);

   if ( bFirst )
   {
       speed = Math::Vector(0.0f, 0.0f, 0.0f);
       dim.x = 10.0f;
       dim.y = dim.x;
       m_particle->CreateParticle(src, speed, dim, Gfx::PARTISPHERE4, 1.5f, 0.0f);

       m_particle->CreateRay(src, dst, Gfx::PARTIRAY3, Math::Point(1.5f, 1.5f), 3.5f);

       m_distance = Math::Distance(src, dst);
   }

   if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
   {
       m_lastParticule = m_time;

       for ( i=0 ; i<4 ; i++ )
       {
           pos = src;
           speed.x = (Math::Rand()-0.5f)*30.0f;
           speed.z = (Math::Rand()-0.5f)*30.0f;
           speed.y = (Math::Rand()-0.5f)*30.0f;
           dim.x = 0.4f;
           dim.y = dim.x;
           duration = Math::Rand()*0.5f+0.5f;
           m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                    duration, 0.0f,
                                    duration*0.9f, 0.7f);
       }

       if ( Math::Distance(src, dst) < m_distance+10.0f )  // pas trop loin ?
       {
           pos = dst;
           speed.x = (Math::Rand()-0.5f)*0.5f;
           speed.z = (Math::Rand()-0.5f)*0.5f;
           speed.y = Math::Rand()*2.0f;
           dim.x = 0.2f;
           dim.y = dim.x;
           duration = Math::Rand()*1.0f+0.5f;
           m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINTr, duration);
       }
   }
}

// Petite démo du volant, même si le véhicule est inactif (SetLock).

void CAutoRemote::DemoWheel(bool bStop)
{
   CPhysics*   physics;
   Character*  character;
   Math::Matrix*  mat;
   Math::Vector   src, dst, pos, speed;
   float       time;
   int         i;

   if ( m_vehicle == 0 )  return;

   physics = m_vehicle->GetPhysics();
   if ( physics == 0 )  return;

   mat = m_object->GetWorldMatrix(0);
   src = Math::Transform(*mat, Math::Vector(4.0f, 8.9f, 4.0f));

   character = m_vehicle->GetCharacter();
   mat = m_vehicle->GetWorldMatrix(0);
   dst = Math::Transform(*mat, character->antenna);

   if ( !m_bStartDemoWheel )
   {
       m_startTime = m_time;
       m_bStartDemoWheel = true;

       m_particle->CreateRay(dst, src, Gfx::PARTIRAY3, Math::Point(1.5f, 0.5f), 11.0f);

       i = m_sound->Play(SOUND_MANIP, m_vehicle->GetPartPosition(0), 0.0f, 0.5f, true);
       m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.2f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.3f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 0.5f, 0.5f, SOPER_STOP);
   }

   if ( bStop )
   {
       physics->ForceMotorSpeedZ(0.0f);
       physics->SetForceSlow(false);
       return;
   }

   time = (m_time-m_startTime)*2.0f;  // temps relatif depuis début démo
   if ( time < 13.0f )
   {
       physics->ForceMotorSpeedZ(sinf(time));

       if ( Math::Mod(time+Math::PI/2.0f, Math::PI) < Math::PI*0.5f && m_lastSound+2.0f <= time )
       {
           m_lastSound = time;

           i = m_sound->Play(SOUND_MANIP, m_vehicle->GetPartPosition(0), 0.0f, 0.5f, true);
           m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.5f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.8f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.0f, 0.5f, 0.5f, SOPER_STOP);
       }
   }
   if ( time > 10.0f && !m_bForceSlow )
   {
       m_bForceSlow = true;
       physics->SetForceSlow(true);  // démarre le moteur au ralenti
   }
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoRemote::GetError()
{
   return ERR_OK;
}


// Cherche le cosmonaute.

COldObject* CAutoRemote::SearchHuman()
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {

       oType = pObj->GetType();
       if ( oType != OBJECT_HUMAN )  continue;
       assert(pObj->Implements(ObjectInterfaceType::Old));

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       if ( dist <= 6.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Cherche la voiture.

COldObject* CAutoRemote::SearchVehicle()
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
//TODO (krzys_h):       if ( pObj->GetGhost() )  continue;

       oType = pObj->GetType();
       if ( oType != OBJECT_CAR )  continue;
       assert(pObj->Implements(ObjectInterfaceType::Old));

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       if ( dist <= 400.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

