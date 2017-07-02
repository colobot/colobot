// autohome.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motioncar.h"
#include "object/motion/buzzingcars/motionbot.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/autohome.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoHome::CAutoHome(COldObject* object)
                        : CAuto(object)
{
   int     i;

   for ( i=0 ; i<6 ; i++ )
   {
       m_partiStop[i] = -1;
   }
   m_lastParticule = 0.0f;
   m_beforeClose = 0.0f;
   m_channelSound = -1;

   Init();
}

// Destructeur de l'objet.

CAutoHome::~CAutoHome()
{
}


// Détruit l'objet.

void CAutoHome::DeleteObject(bool bAll)
{
   m_phase = AHOP_WAIT;
   FireStopUpdate();
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoHome::Init()
{
   m_time = 0.0f;

   m_phase = AHOP_WAIT;
   m_progress = 0.0f;
   m_speed    = 1.0f/1.0f;
   m_type = m_object->GetType();
}


// Démarre l'objet.

void CAutoHome::Start(int param)
{
   ObjectType  type;
   float       angle;
   int         i;

   if ( param == AHOP_BREAKDOWN )
   {
       i = m_sound->Play(SOUND_NUCLEAR, m_object->GetPartPosition(0), 0.0f, 1.0f, true);
       m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.5f, 1.0f, 3.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.1f, SOPER_STOP);

       if ( m_channelSound == -1 )
       {
           m_channelSound = m_sound->Play(SOUND_HOME1, m_goalPos, 1.0f);
       }
       if ( m_channelSound != -1 )  // la musique ralenti
       {
           m_sound->FlushEnvelope(m_channelSound);
           m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 0.1f, SOPER_CONTINUE);
           m_sound->AddEnvelope(m_channelSound, 1.0f, 0.5f, 3.0f, SOPER_STOP);
       }

       StartBotAction(MB_FEAR, 3.1f);  // peur

       m_phase    = AHOP_BREAKDOWN;
       m_progress = 0.0f;
       m_speed    = 1.0f/3.0f;
       m_bBreakFinish = false;
       for ( i=0 ; i<4 ; i++ )
       {
           m_breakPhase[i] = 0;  // wait
           m_breakTimeWait[i] = Math::Rand()*0.5f;
           m_breakTimeBzzz[i] = m_breakTimeWait[i]+Math::Rand()*(0.9f-m_breakTimeWait[i]);
       }

       m_object->SetLock(true);  // plus détecté par la suite
       return;
   }

   type = OBJECT_NULL;
   if ( m_type == OBJECT_HOME1 )  type = OBJECT_BOT1;
   if ( m_type == OBJECT_HOME2 )  type = OBJECT_BOT2;
   if ( m_type == OBJECT_HOME3 )  type = OBJECT_BOT3;
   if ( m_type == OBJECT_HOME4 )  type = OBJECT_BOT4;
   if ( m_type == OBJECT_HOME5 )  type = OBJECT_BOT5;
   if ( type == OBJECT_NULL )  return;

   m_goalPos = m_object->GetPartPosition(0);
   if ( m_type == OBJECT_HOME1 )  m_goalPos.y += 3.0f;
   if ( m_type == OBJECT_HOME2 )  m_goalPos.y += 6.0f;
   if ( m_type == OBJECT_HOME3 )  m_goalPos.y += 0.0f;
   if ( m_type == OBJECT_HOME4 )  m_goalPos.y += 3.0f;
   if ( m_type == OBJECT_HOME5 )  m_goalPos.y += 3.0f;

   angle = m_object->GetPartRotationY(0);

   m_bot = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->CreateObject(m_goalPos, angle, type)); // TODO (krzys_h): bPlumb = true

   m_goalPos.y += m_bot->GetCharacter()->height;
   m_bot->SetPartPosition(0, m_goalPos);

   m_phase    = AHOP_LIVE1;
   m_progress = 0.0f;
   m_speed    = 1.0f/0.1f;
}


// Gestion d'un événement.

bool CAutoHome::EventProcess(const Event &event)
{
   COldObject*    mark;
   Math::Matrix*  mat;
   Math::Vector   pos, speed, eye;
   Math::Point      dim;
   float       angle, duration, mass;
   int         r, i, j;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   m_progress += event.rTime*m_speed;

   if ( m_phase == AHOP_WAIT )
   {
       if ( m_progress >= 1.0f )
       {
           m_vehicle = SearchVehicle();
           if ( m_vehicle == 0 )
           {
               m_phase    = AHOP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.5f;
           }
           else
           {
               m_vehiclePos = m_vehicle->GetPartPosition(0);
               m_phase    = AHOP_STOPCHECK;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.2f;
           }
       }
   }

   if ( m_phase == AHOP_STOPCHECK )
   {
       m_vehicle = SearchVehicle();
       if ( m_vehicle == 0 )
       {
           m_phase    = AHOP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.5f;
           return true;
       }
       pos = m_vehicle->GetPartPosition(0);
       if ( Math::Distance(pos, m_vehiclePos) > 0.1f )
       {
           m_phase    = AHOP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.2f;
           return true;
       }

       if ( m_progress >= 1.0f )
       {
           m_vehicle->SetLock(true);
           StartVehicleAction(MV_OPEN);
           m_cameraType = m_camera->GetType();
           m_camera->SetControllingObject(m_object);
           m_camera->SetType(Gfx::CAM_TYPE_BACK);
           m_main->SetStopwatch(false);  // stoppe le chrono

           m_phase    = AHOP_OPEN;
           m_progress = 0.0f;
           m_speed    = 1.0f/2.0f;
       }
   }

   if ( m_phase == AHOP_OPEN )
   {
       if ( m_progress >= 1.0f )
       {
           m_bot = dynamic_cast<COldObject*>(m_vehicle->GetCargo());
           m_bot->SetTransporter(0);
           m_vehicle->SetCargo(0);
           m_startPos = GetVehiclePoint(m_vehicle);
           m_startPos.y += m_bot->GetCharacter()->height;
           m_goalPos = m_object->GetPartPosition(0);
           m_goalPos.y += m_bot->GetCharacter()->height;
           if ( m_type == OBJECT_HOME1 )  m_goalPos.y += 3.0f;
           if ( m_type == OBJECT_HOME2 )  m_goalPos.y += 6.0f;
           if ( m_type == OBJECT_HOME3 )  m_goalPos.y += 0.0f;
           if ( m_type == OBJECT_HOME4 )  m_goalPos.y += 3.0f;
           if ( m_type == OBJECT_HOME5 )  m_goalPos.y += 3.0f;
           m_bot->SetPartPosition(0, m_startPos);
           m_bot->SetPartRotation(0, Math::Vector(0.0f, m_vehicle->GetPartRotationY(0), 0.0f));
           StartingEffect();

           m_phase    = AHOP_MOVE;
           m_progress = 0.0f;
           m_speed    = 1.0f/3.0f;
           if ( m_type == OBJECT_HOME3 )  m_speed = 1.0f/5.0f;
       }
   }

   if ( m_phase == AHOP_MOVE )
   {
       MoveBot(m_progress, event.rTime);

       if ( m_progress >= 1.0f )
       {
           StartVehicleAction(MV_CLOSE);
           EndingEffect();
           if ( m_type == OBJECT_HOME1 )  m_beforeClose = 10.0f;
           if ( m_type == OBJECT_HOME2 )  m_beforeClose =  8.0f;
           if ( m_type == OBJECT_HOME3 )  m_beforeClose =  3.0f;
           if ( m_type == OBJECT_HOME4 )  m_beforeClose =  5.0f;
           if ( m_type == OBJECT_HOME5 )  m_beforeClose =  5.0f;
           m_lastDir = -1;

           m_phase    = AHOP_LIVE1;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.1f;
       }
   }

   if ( m_beforeClose > 0.0f )
   {
       m_beforeClose -= event.rTime;
       if ( m_beforeClose <= 0.0f )
       {
           m_beforeClose = 0.0f;
           m_vehicle->SetLock(false);
           m_camera->SetControllingObject(m_vehicle);
           m_camera->SetType(m_cameraType);
           m_main->SetStopwatch(true);  // redémarre le chrono
       }
   }

   if ( m_phase == AHOP_LIVE1 )
   {
       if ( m_type == OBJECT_HOME1 )  // choix direction et attente
       {
           if ( m_progress >= 1.0f )
           {
               do
               {
                   r = rand()%4;
               }
               while ( r == m_lastDir );
               m_lastDir = r;
               if ( r == 0 )  {m_dir = Math::Vector( 4.0f, 6.5f,  0.0f); m_goalAngle=Math::PI*1.0f;}
               if ( r == 1 )  {m_dir = Math::Vector(-4.0f, 6.5f,  0.0f); m_goalAngle=Math::PI*0.0f;}
               if ( r == 2 )  {m_dir = Math::Vector( 0.0f, 6.5f,  4.0f); m_goalAngle=Math::PI*0.5f;}
               if ( r == 3 )  {m_dir = Math::Vector( 0.0f, 6.5f, -4.0f); m_goalAngle=Math::PI*1.5f;}
               m_startAngle = Math::NormAngle(m_bot->GetPartRotationY(0));

               m_phase    = AHOP_LIVE2;
               m_progress = 0.0f;
               m_speed    = 1.0f/(abs(m_startAngle-m_goalAngle)*0.3f+0.1f);
           }
       }
       if ( m_type == OBJECT_HOME2 )  // choix direction et attente
       {
           if ( m_progress >= 1.0f )
           {
               pos = m_object->GetPartPosition(0);
               mark = SearchObject(OBJECT_MARK, pos, 400.0f);
               if ( mark == 0 )
               {
                   m_goalAngle = Math::Rand()*Math::PI*2.0f;
               }
               else
               {
                   m_goalPos = mark->GetPartPosition(0);
                   m_goalAngle = Math::RotateAngle(m_goalPos.x-pos.x, pos.z-m_goalPos.z);
               }
               m_startAngle = Math::NormAngle(m_bot->GetPartRotationY(0));

               m_phase    = AHOP_LIVE2;
               m_progress = 0.0f;
               m_speed    = 1.0f/(abs(m_startAngle-m_goalAngle)*0.3f+0.1f);
           }
       }
       if ( m_type == OBJECT_HOME3 )
       {
           StartBotAction(MB_HOME2);  // assis
       }
   }

   if ( m_phase == AHOP_LIVE2 )
   {
       if ( m_type == OBJECT_HOME1 )  // rotation
       {
           angle = m_startAngle+(m_goalAngle-m_startAngle)*Math::Norm(m_progress);
           m_bot->SetPartRotationY(0, angle);

           if ( m_progress >= 1.0f )
           {
               m_phase    = AHOP_LIVE3;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.6f;
           }
       }
       if ( m_type == OBJECT_HOME2 )  // rotation
       {
           angle = m_startAngle+(m_goalAngle-m_startAngle)*Math::Norm(m_progress);
           m_bot->SetPartRotationY(0, angle);

           if ( m_progress >= 1.0f )
           {
               if ( ProxiSelect(200.0f) )
               {
                   StartBotAction(MB_HOME2);  // tir
               }
               m_phase    = AHOP_LIVE3;
               m_progress = 0.0f;
               m_speed    = 1.0f/3.0f;
           }
       }
   }

   if ( m_phase == AHOP_LIVE3 )
   {
       if ( m_type == OBJECT_HOME1 )  // avance vers console
       {
           mat = m_object->GetWorldMatrix(0);
           pos = Math::Transform(*mat, m_dir);
           pos = m_goalPos+(pos-m_goalPos)*Math::Norm(m_progress);
           m_bot->SetPartPosition(0, pos);

           if ( m_progress >= 1.0f )
           {
               StartBotAction(MB_HOME2);
               if ( ProxiSelect(200.0f) )
               {
                   m_channelSound = m_sound->Play(SOUND_HOME1, m_goalPos, 1.0f);
                   m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 8.0f, SOPER_CONTINUE);
                   m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 0.1f, SOPER_STOP);
               }
               m_phase    = AHOP_LIVE4;
               m_progress = 0.0f;
               m_speed    = 1.0f/7.5f;
           }
       }
       if ( m_type == OBJECT_HOME2 )  // tir
       {
           if ( m_progress >= 1.0f )
           {
               StartBotAction(MB_HOME1);
               m_phase    = AHOP_LIVE1;
               m_progress = 0.0f;
               m_speed    = 1.0f/3.5f;
           }
       }
   }

   if ( m_phase == AHOP_LIVE4 )
   {
       if ( m_type == OBJECT_HOME1 )  // pianote
       {
           m_object->SetPartScaleX(1, 0.95f+Math::Rand()*0.1f);  // hp vibrent
           m_object->SetPartScaleX(2, 0.95f+Math::Rand()*0.1f);
           m_object->SetPartScaleX(3, 0.95f+Math::Rand()*0.1f);
           m_object->SetPartScaleX(4, 0.95f+Math::Rand()*0.1f);

           if ( m_progress >= 1.0f )
           {
               m_object->SetPartScaleX(1, 1.0f);
               m_object->SetPartScaleX(2, 1.0f);
               m_object->SetPartScaleX(3, 1.0f);
               m_object->SetPartScaleX(4, 1.0f);
               StartBotAction(MB_HOME1);

               m_phase    = AHOP_LIVE5;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.6f;
           }
       }
   }

   if ( m_phase == AHOP_LIVE5 )
   {
       if ( m_type == OBJECT_HOME1 )  // recule au centre
       {
           mat = m_object->GetWorldMatrix(0);
           pos = Math::Transform(*mat, m_dir);
           pos = m_goalPos+(pos-m_goalPos)*Math::Norm(1.0f-m_progress);
           m_bot->SetPartPosition(0, pos);

           if ( m_progress >= 1.0f )
           {
               m_phase    = AHOP_LIVE1;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.1f;
           }
       }
   }

   if ( m_phase == AHOP_BREAKDOWN )
   {
       if ( m_type == OBJECT_HOME1 )
       {
           mat = m_object->GetWorldMatrix(0);

           if ( m_progress < 1.0f )
           {
               for ( j=0 ; j<4 ; j++ )
               {
                   if ( m_breakPhase[j] == 0 )  // wait ?
                   {
                       if ( m_progress >= m_breakTimeWait[j] )
                       {
                           m_breakPhase[j] = 1;
                       }
                   }
                   if ( m_breakPhase[j] == 1 )  // bzzz ?
                   {
                       if ( m_progress >= m_breakTimeBzzz[j] )
                       {
                           m_object->DetachPart(j+1);  // explosion du haut-parleur
                           m_sound->Play(SOUND_EXPLOlp, m_object->GetPartPosition(0));
                           for ( i=0 ; i<10 ; i++ )
                           {
                               if ( j == 0 )  pos = Math::Vector( 8.5f, 14.0f,  8.5f);
                               if ( j == 1 )  pos = Math::Vector(-8.5f, 14.0f,  8.5f);
                               if ( j == 2 )  pos = Math::Vector( 8.5f, 14.0f, -8.5f);
                               if ( j == 3 )  pos = Math::Vector(-8.5f, 14.0f, -8.5f);
                               pos = Math::Transform(*mat, pos);
                               pos.x += (Math::Rand()-0.5f)*2.0f;
                               pos.y += (Math::Rand()-0.5f)*4.0f;
                               pos.z += (Math::Rand()-0.5f)*2.0f;
                               speed.x = (Math::Rand()-0.5f)*15.0f;
                               speed.z = (Math::Rand()-0.5f)*15.0f;
                               speed.y = Math::Rand()*20.0f;
                               dim.x = 1.0f;
                               dim.y = dim.x;
                               duration = Math::Rand()*3.0f+2.0f;
                               mass = Math::Rand()*10.0f+15.0f;
                               m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK1,
                                                        duration, mass, Math::Rand()+0.7f, 1.0f);

                               speed = Math::Vector(0.0f, 0.0f, 0.0f);
                               dim.x = Math::Rand()*5.0f+1.0f;
                               dim.y = dim.x;
                               duration = Math::Rand()*6.0f+2.0f;
                               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, duration);
                           }
                           m_breakPhase[j] = 2;
                       }
                   }
               }

               if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
               {
                   m_lastParticule = m_time;

                   for ( j=0 ; j<4 ; j++ )
                   {
                       if ( m_breakPhase[j] != 1 )  continue;

                       for ( i=0 ; i<5 ; i++ )
                       {
                           if ( j == 0 )  pos = Math::Vector( 8.5f, 14.0f,  8.5f);
                           if ( j == 1 )  pos = Math::Vector(-8.5f, 14.0f,  8.5f);
                           if ( j == 2 )  pos = Math::Vector( 8.5f, 14.0f, -8.5f);
                           if ( j == 3 )  pos = Math::Vector(-8.5f, 14.0f, -8.5f);
                           pos = Math::Transform(*mat, pos);
                           pos.x += (Math::Rand()-0.5f)*2.0f;
                           pos.y += (Math::Rand()-0.5f)*4.0f;
                           pos.z += (Math::Rand()-0.5f)*2.0f;
                           speed.x = (Math::Rand()-0.5f)*20.0f;
                           speed.z = (Math::Rand()-0.5f)*20.0f;
                           speed.y = 5.0f+Math::Rand()*10.0f;
                           dim.x = 0.8f;
                           dim.y = 0.8f;
                           m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 40.0f);
                       }
                   }
               }
           }

           if ( m_progress >= 1.0f && !m_bBreakFinish )
           {
               m_bBreakFinish = true;
               m_bot->SetDying(DeathType::Exploding);  // comme si plus là !
               HappyBlupi();  // les blupis sont heureux
           }
       }
   }

   if ( m_channelSound != -1 )
   {
       eye = m_engine->GetEyePt();
       pos = m_object->GetPartPosition(0);
       pos = pos+(eye-pos)*0.8f;
       m_sound->Position(m_channelSound, pos);

       /* TODO (krzys_h):
       if ( m_sound->GetAmplitude(m_channelSound) == 0.0f )
       {
           m_channelSound = -1;
       }
       */
   }

   return true;
}

// Stoppe l'automate.

bool CAutoHome::Abort()
{
   return true;
}


// Met à jour les feux de stop.

void CAutoHome::FireStopUpdate()
{
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   bool        bOn;
   int         i;

   static float listpos[3*6] =
   {
        8.0f, 26.0f,  24.0f,
       -8.0f, 26.0f,  24.0f,
        8.0f, 26.0f, -24.0f,
       -8.0f, 26.0f, -24.0f,  // 4 feux au sommet des tours
        3.0f,  6.0f,   0.0f,
       -3.0f,  6.0f,   0.0f,  // 2 feux sur la porte
   };

   if ( m_phase == AHOP_WAIT )  // éteint ?
   {
       for ( i=0 ; i<6 ; i++ )
       {
           if ( m_partiStop[i] != -1 )
           {
               m_particle->DeleteParticle(m_partiStop[i]);
               m_partiStop[i] = -1;
           }
       }
       return;
   }

   mat = m_object->GetWorldMatrix(0);

   speed = Math::Vector(0.0f, 0.0f, 0.0f);
   dim.x = 3.0f;
   dim.y = dim.x;

   for ( i=0 ; i<6 ; i++ )
   {
       bOn = false;

       if ( bOn )
       {
           if ( m_partiStop[i] == -1 )
           {
               pos.x = listpos[i*3+0];
               pos.y = listpos[i*3+1];
               pos.z = listpos[i*3+2];
               pos = Math::Transform(*mat, pos);
               m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                             dim, Gfx::PARTISELR,
                                                             1.0f, 0.0f);
           }
       }
       else
       {
           if ( m_partiStop[i] != -1 )
           {
               m_particle->DeleteParticle(m_partiStop[i]);
               m_partiStop[i] = -1;
           }
       }
   }
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoHome::GetError()
{
   return ERR_OK;
}


// Génère les particules suite au décollage.

void CAutoHome::StartingEffect()
{
   int     i;

   StartBotAction(MB_GOHOME);

   if ( m_type == OBJECT_HOME1 ||
        m_type == OBJECT_HOME2 )
   {
       i = m_sound->Play(SOUND_FLY, m_goalPos, 0.0f, 1.0f, true);
       m_sound->AddEnvelope(i, 1.0f, 1.5f, 1.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 1.0f, 1.5f, 2.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.5f, SOPER_STOP);
   }

   if ( m_type == OBJECT_HOME3 )
   {
       m_sound->Play(SOUND_POWEROFF, m_startPos, 1.0f);
       m_bZoomIn = false;
   }
}

// Déplace le robot du véhicule à la maison.

void CAutoHome::MoveBot(float progress, float rTime)
{
   Math::Vector   src, dst, pos, speed;
   Math::Point      dim;
   float       duration;
   int         i;

   progress = Math::Norm(progress);

   if ( m_type == OBJECT_HOME1 ||
        m_type == OBJECT_HOME2 )
   {
       pos = m_startPos+(m_goalPos-m_startPos)*progress;
       pos.y += sinf(progress*Math::PI)*20.0f;
       m_bot->SetPartPosition(0, pos);

       pos.y -= m_bot->GetCharacter()->height;

       if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
       {
           m_lastParticule = m_time;

           for ( i=0 ; i<4 ; i++ )
           {
               speed.x = (Math::Rand()-0.5f)*2.0f;
               speed.z = (Math::Rand()-0.5f)*2.0f;
               speed.y = -Math::Rand()*5.0f;
               dim.x = Math::Rand()*1.0f+1.0f;
               dim.y = dim.x;
               duration = Math::Rand()*1.0f+1.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, duration);
           }
       }
   }

   if ( m_type == OBJECT_HOME3 )
   {
       if ( progress < 0.5f )
       {
           progress = progress/0.5f;

           if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<4 ; i++ )
               {
                   pos = m_startPos;
                   speed = pos;
                   pos.x += (Math::Rand()-0.5f)*30.0f;
                   pos.y += (Math::Rand()-0.5f)*30.0f;
                   pos.z += (Math::Rand()-0.5f)*30.0f;
                   speed = (speed-pos)*1.0f;
                   dim.x = 0.6f;
                   dim.y = dim.x;
                   duration = Math::Rand()*0.5f+0.5f;
                   m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                            duration, 0.0f,
                                            duration*0.9f, 0.7f);
               }
           }

           m_bot->SetPartScale(0, 1.0f-progress);
           m_bot->SetPartRotationY(0, m_bot->GetPartRotationY(0)+rTime*20.0f);
       }
       else
       {
           progress = (progress-0.5f)/0.5f;

           if ( !m_bZoomIn )
           {
               m_bZoomIn = true;
               m_sound->Play(SOUND_POWERON, m_goalPos, 1.0f);
           }

           if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<4 ; i++ )
               {
                   pos = m_goalPos;
                   speed.x = (Math::Rand()-0.5f)*30.0f;
                   speed.z = (Math::Rand()-0.5f)*30.0f;
                   speed.y = (Math::Rand()-0.5f)*30.0f;
                   dim.x = 0.6f;
                   dim.y = dim.x;
                   duration = Math::Rand()*0.5f+0.5f;
                   m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                            duration, 0.0f,
                                            duration*0.9f, 0.7f);
               }
           }

           m_bot->SetPartPosition(0, m_goalPos);
           m_bot->SetPartScale(0, progress);
           m_bot->SetPartRotationY(0, m_bot->GetPartRotationY(0)+rTime*20.0f);
       }
   }
}

// Génère les particules suite à l'atterrissage.

void CAutoHome::EndingEffect()
{
   Math::Vector   pos, ppos, speed;
   Math::Point      dim;
   float       len;
   int         i, max;

   StartBotAction(MB_HOME1);

   if ( m_type == OBJECT_HOME1 ||
        m_type == OBJECT_HOME2 )
   {
       pos = m_bot->GetPartPosition(0);
       max = (int)(10.0f*m_engine->GetParticleDensity());

       for ( i=0 ; i<max ; i++ )
       {
           ppos.x = pos.x + (Math::Rand()-0.5f)*5.0f;
           ppos.z = pos.z + (Math::Rand()-0.5f)*5.0f;
           ppos.y = pos.y + Math::Rand()*4.0f;
           len = 1.0f-(Math::Distance(ppos, pos)/(15.0f+5.0f));
           if ( len <= 0.0f )  continue;
           speed.x = (ppos.x-pos.x)*0.1f;
           speed.z = (ppos.z-pos.z)*0.1f;
           speed.y = -2.0f;
           dim.x = 2.0f+5.0f*len;
           dim.y = dim.x;
           m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTICRASH, 2.0f);
       }

       m_sound->Play(SOUND_BOUMv, m_goalPos, 1.0f);
   }
}

// Cherche l'objet véhicule.

COldObject* CAutoHome::SearchVehicle()
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType, fType;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
// TODO (krzys_h):       if ( pObj->GetGhost() )  continue;

       oType = pObj->GetType();
       if ( oType != OBJECT_CAR )  continue;
       assert(pObj->Implements(ObjectInterfaceType::Old));

       if ( dynamic_cast<COldObject*>(pObj)->GetCargo() == 0 )  continue;
       fType = dynamic_cast<COldObject*>(pObj)->GetCargo()->GetType();

       if ( (m_type != OBJECT_HOME1 || fType != OBJECT_BOT1) &&
            (m_type != OBJECT_HOME2 || fType != OBJECT_BOT2) &&
            (m_type != OBJECT_HOME3 || fType != OBJECT_BOT3) &&
            (m_type != OBJECT_HOME4 || fType != OBJECT_BOT4) &&
            (m_type != OBJECT_HOME5 || fType != OBJECT_BOT5) )  continue;

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       if ( dist <= 30.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Cherche un objet proche.

COldObject* CAutoHome::SearchObject(ObjectType type, Math::Vector center, float radius)
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

// Vérifie si l'objet sélectionné est proche.

bool CAutoHome::ProxiSelect(float dist)
{
#if 0
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {

       if ( !pObj->GetSelect() )  continue;

       oPos = pObj->GetPartPosition(0);
       if ( Math::Distance(oPos, cPos) <= dist )  return true;
   }

   return false;
#else
   Math::Vector   cPos, oPos;

   cPos = m_object->GetPosition();
   oPos = m_engine->GetLookatPt();
   if ( Math::Distance(oPos, cPos) <= dist )  return true;
   return false;
#endif
}

// Cherche le point où mettre du fret sur un véhicule.

Math::Vector CAutoHome::GetVehiclePoint(COldObject *pObj)
{
   Character*  character;
   Math::Matrix*  mat;
   Math::Vector   pos;

   character = pObj->GetCharacter();
   mat = pObj->GetWorldMatrix(0);
   pos = Math::Transform(*mat, character->posFret);

   return pos;
}

// Démarre une action pour le véhicule.

void CAutoHome::StartVehicleAction(int action)
{
   CMotion*    motion;
   float       delay;

   motion = m_vehicle->GetMotion();
   if ( motion == 0 )  return;

   delay = 1.0f;
   if ( action == MV_CLOSE )  delay = 2.0f;
   motion->SetAction(action, delay);
}

// Démarre une action "soulevé" pour l'objet pris par la grue.

void CAutoHome::StartBotAction(int action, float delay)
{
   CMotion*    motion;

   motion = m_bot->GetMotion();
   if ( motion == 0 )  return;

   motion->SetAction(action, delay);
}

// Rend heureux tous les blupi avoisinnants.

void CAutoHome::HappyBlupi()
{
   COldObject*    pObj;
   CMotion*    motion;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
//TODO (krzys_h):       if ( pObj->GetGhost() )  continue;

       oType = pObj->GetType();
       if ( oType != OBJECT_BOT3 )  continue;

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       if ( dist <= 200.0f )
       {
           assert(pObj->Implements(ObjectInterfaceType::Movable));
           motion = dynamic_cast<CMovableObject*>(pObj)->GetMotion();
           if ( motion != 0 )
           {
               motion->SetAction(MB_WAIT, 1.0f);
               motion->SetAction(MB_HAPPY, 1.0f);
           }
       }
   }
}

