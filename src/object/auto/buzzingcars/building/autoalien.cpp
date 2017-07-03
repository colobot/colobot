// autoalien.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/pyro.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motionbot.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/building/autoalien.h"
#include "graphics/engine/pyro_manager.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoAlien::CAutoAlien(COldObject* object)
                        : CAuto(object)
{
   int     i;

   for ( i=0 ; i<12 ; i++ )
   {
       m_partiStop[i] = -1;
   }

   Init();
}

// Destructeur de l'objet.

CAutoAlien::~CAutoAlien()
{
}


// Détruit l'objet.

void CAutoAlien::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoAlien::Init()
{
   m_time = 0.0f;
   m_lastParticule = 0.0f;
   m_trackProgress = 0.0f;
   m_rotAngle = 0.0f;
   m_rotSpeed = 2.0f;
   m_bot = 0;
   m_lastType = OBJECT_NULL;
   m_bSynchro = false;
   m_bStop = false;
   m_bFear = false;

   m_type = m_object->GetType();
   m_progress = 0.0f;
   m_speed    = 1.0f/2.0f;
   m_phase    = 0;

   if ( m_type == OBJECT_ALIEN5 )  // marteau ?
   {
       m_speed = 1.0f/8.0f;
   }
   if ( m_type == OBJECT_ALIEN8 )  // générateur ?
   {
       m_speed = 1.0f/3.0f;
   }
}


// Démarre l'objet.

void CAutoAlien::Start(int param)
{
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   float       duration, mass, angle, value;
   int         i;

   if ( m_type == OBJECT_ALIEN5 )  // marteau ?
   {
       if ( param == 2 )  // synchronisé avec générateur ?
       {
           m_bSynchro = true;
       }
       if ( param == 3 && m_bSynchro )  // synchro venant du générateur ?
       {
           m_progress = 0.0f;
           m_speed    = 1.0f/8.0f;
           m_phase    = 0;
       }
       if ( param == 4 && !m_bStop )  // stoppe ?
       {
           m_bStop = true;
       }
   }

   if ( m_type == OBJECT_ALIEN8 )  // générateur ?
   {
       if ( param == 2 && !m_bStop )  // bouton rouge pressé ?
       {
           m_bStop = true;

           m_object->SetPartRotationX(4, (Math::Rand()-0.5f)*2.0f);
           m_object->SetPartRotationY(4, (Math::Rand()-0.5f)*2.0f);

           mat = m_object->GetWorldMatrix(4);  // bouton rouge
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
           SynchroHammer(4);  // stoppe
       }
   }

   if ( m_type == OBJECT_ALIEN9 )  // torture ?
   {
       if ( param == 1 )  // démarre ?
       {
           value = m_object->GetCmdLine(0);
           if ( std::isnan(value) || value == 0.0f )
           {
               m_speed = 1.0f/10.0f;
           }
           else
           {
               m_speed = 1.0f/value;
           }

           mat = m_object->GetWorldMatrix(0);
           pos = Math::Transform(*mat, Math::Vector(0.0f, 5.6f, -0.5f));
           angle = m_object->GetPartRotationY(0)+Math::PI*0.5f;

           m_bot = static_cast<COldObject*>(CObjectManager::GetInstancePointer()->CreateObject(pos, angle, OBJECT_BOT3)); // TODO(krzys_h): bPlumb=true?

           pos.y += m_bot->GetCharacter()->height;
           m_bot->SetPartPosition(0, pos);
           BotAction(m_bot, MB_HOME2, 1.0f);  // blupi assis
           BotAction(m_bot, MB_ANGRY, 1.0f);  // blupi triste
       }

       if ( param == 2 && m_phase == 0 && m_bot != 0 )  // stoppe ?
       {
//?            m_sound->Play(SOUND_BREAK2, m_engine->GetEyePt(), 1.0f, 1.4f);

           m_progress = 0.0f;
           m_speed    = 1.0f/2.0f;
           m_phase    = 2;  // court-circuit
       }
   }
}


// Gestion d'un événement.

bool CAutoAlien::EventProcess(const Event &event)
{
   Gfx::CPyro*      pyro;
   Math::Matrix*  mat;
   Math::Vector   pos, speed, src, dst, eye;
   Math::Point      dim, rot, c, p;
   ObjectType  type;
   float       progress, duration, delay, mass, angle, dir, factor, skip;
   int         i, r;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   m_progress += event.rTime*m_speed;
   progress = Math::Norm(m_progress);

   if ( m_type == OBJECT_ALIEN2 )  // tour ?
   {
       if ( m_phase == 0 )  // up ?
       {
           m_object->SetPartPosition(1, Math::Vector(0.0f, 30.0f+progress*40.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               delay = 0.5f+Math::Rand()*1.0f;

               pos = m_object->GetPartPosition(0);
               eye = m_engine->GetEyePt();
               pos = eye+(pos-eye)*0.6f;
               i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, true);
               m_sound->AddEnvelope(i, 1.0f, 1.0f, 0.1f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 1.0f, 1.0f, delay-0.1f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.1f, SOPER_STOP);

               m_progress = 0.0f;
               m_speed    = 1.0f/delay;
               m_phase    = 1;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 1 )  // down ?
       {
           m_object->SetPartPosition(1, Math::Vector(0.0f, 30.0f+(1.0f-progress)*40.0f, 0.0f));

           pos = m_object->GetPartPosition(0);
           if ( Math::Distance(pos, m_engine->GetLookatPt()) < 500.0f &&
                m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<10 ; i++ )
               {
                   rot = Math::RotatePoint(Math::Rand()*Math::PI*2.0f, 35.0f);
                   src = m_object->GetPartPosition(1);
                   src.x = pos.x+rot.x;
                   src.z = pos.z+rot.y;
                   src.y += pos.y+5.0f;
                   speed = Math::Vector(0.0f, 10.0f+Math::Rand()*10.0f, 0.0f);
                   dim.x = Math::Rand()*2.0f+2.0f;
                   dim.y = dim.x;
                   duration = 1.0f+Math::Rand()*1.0f;
                   m_particle->CreateParticle(src, speed, dim, Gfx::PARTIBLITZ, duration);
               }
           }

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Math::Rand()*0.5f);
               m_phase    = 2;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 2 )  // wait ?
       {
           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Math::Rand()*4.0f);
               m_phase    = 0;
               progress   = 0.0f;
           }
       }
   }

   if ( m_type == OBJECT_ALIEN3 )  // tour haute ?
   {
       if ( m_phase == 0 )  // up ?
       {
           m_object->SetPartPosition(1, Math::Vector(0.0f,  30.0f+progress*60.0f, 0.0f));
           m_object->SetPartPosition(2, Math::Vector(0.0f, 100.0f+(1.0f-progress)*40.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               delay = 0.5f+Math::Rand()*1.0f;

               pos = m_object->GetPartPosition(0);
               eye = m_engine->GetEyePt();
               pos = eye+(pos-eye)*0.6f;
               i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, true);
               m_sound->AddEnvelope(i, 1.0f, 1.0f, 0.1f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 1.0f, 1.0f, delay-0.1f, SOPER_CONTINUE);
               m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.1f, SOPER_STOP);

               m_progress = 0.0f;
               m_speed    = 1.0f/delay;
               m_phase    = 1;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 1 )  // down ?
       {
           m_object->SetPartPosition(1, Math::Vector(0.0f,  30.0f+(1.0f-progress)*60.0f, 0.0f));
           m_object->SetPartPosition(2, Math::Vector(0.0f, 100.0f+progress*40.0f, 0.0f));

           pos = m_object->GetPartPosition(0);
           if ( Math::Distance(pos, m_engine->GetLookatPt()) < 500.0f &&
                m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<10 ; i++ )
               {
                   rot = Math::RotatePoint(Math::Rand()*Math::PI*2.0f, 35.0f);
                   src = m_object->GetPartPosition(1);
                   src.x = pos.x+rot.x;
                   src.z = pos.z+rot.y;
                   src.y += pos.y+5.0f;
                   speed = Math::Vector(0.0f, 20.0f+Math::Rand()*20.0f, 0.0f);
                   dim.x = Math::Rand()*2.0f+2.0f;
                   dim.y = dim.x;
                   duration = 1.0f+Math::Rand()*1.0f;
                   m_particle->CreateParticle(src, speed, dim, Gfx::PARTIBLITZ, duration);
               }
           }

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Math::Rand()*0.5f);
               m_phase    = 2;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 2 )  // wait ?
       {
           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Math::Rand()*4.0f);
               m_phase    = 0;
               progress   = 0.0f;
           }
       }
   }

   if ( m_type == OBJECT_ALIEN5 )  // marteau ?
   {
       if ( m_phase == 0 )  // up ?
       {
           if ( progress > 0.2f )
           {
               m_trackProgress += event.rTime*2.2f;
               UpdateTrackMapping(m_trackProgress);
           }

           m_object->SetPartRotationX(1, progress*Math::PI*0.25f);

           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   m_progress = 0.0f;
                   if ( m_bSynchro )  m_speed = 1.0f/1.8f;
                   else               m_speed = 1.0f/(0.5f+Math::Rand()*1.0f);
                   m_phase    = 1;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 1 )  // wait ?
       {
           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/0.2f;
                   m_phase    = 2;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 2 )  // down ?
       {
           m_object->SetPartRotationX(1, (1.0f-progress)*Math::PI*0.25f);

           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   pos = m_object->GetPartPosition(0);
                   if ( Math::Distance(pos, m_engine->GetLookatPt()) < 500.0f )
                   {
                       mat = m_object->GetWorldMatrix(0);
                       for ( i=0 ; i<10 ; i++ )
                       {
                           pos.x =   0.0f+(Math::Rand()-0.5f)*25.0f;
                           pos.z = -18.0f+(Math::Rand()-0.5f)*25.0f;
                           pos.y =  10.0f;
                           pos = Math::Transform(*mat, pos);
                           speed = Math::Vector(0.0f, 0.0f, 0.0f);
                           dim.x = Math::Rand()*20.0f+20.0f;
                           dim.y = dim.x;
                           duration = 2.0f+Math::Rand()*2.0f;
                           m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, duration);
                       }
                       for ( i=0 ; i<10 ; i++ )
                       {
                           angle = Math::Rand()*Math::PI*2.0f;
                           rot = Math::RotatePoint(angle, 13.0f);
                           pos.x = rot.x;
                           pos.z = rot.y-18.0f;
                           pos.y = 10.0f;
                           pos = Math::Transform(*mat, pos);
                           rot = Math::RotatePoint(angle, 25.0f+Math::Rand()*25.0f);
                           speed.x = rot.x;
                           speed.z = rot.y-18.0f;
                           speed.y = 10.0f+Math::Rand()*30.0f;
                           speed = Math::Transform(*mat, speed);
                           speed -= pos;
                           dim.x = 1.0f;
                           dim.y = dim.x;
                           duration = Math::Rand()*3.0f+2.0f;
                           mass = Math::Rand()*20.0f+30.0f;
                           m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK1,
                                                    duration, mass, Math::Rand()+0.7f, 1.0f);
                       }
                       eye = m_engine->GetEyePt();
                       pos = eye+(pos-eye)*0.2f;
                       m_sound->Play(SOUND_BOUMm, pos);
                   }

                   m_progress = 0.0f;
                   if ( m_bSynchro )  m_speed = 1.0f/10000.0f;  // attend synchro
                   else               m_speed = 1.0f/2.5f;
                   m_phase    = 3;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 3 )  // wait ?
       {
           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   m_progress = 0.0f;
                   if ( m_bSynchro )  m_speed = 1.0f/8.0f;
                   else               m_speed = 1.0f/(2.0f+Math::Rand()*4.0f);
                   m_phase    = 0;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 6 )  // dead ?
       {
       }
   }

   if ( m_type == OBJECT_ALIEN6 )  // écraseur ?
   {
       if ( m_phase == 0 )  // up ?
       {
           m_object->SetPartRotationZ(1,  progress*Math::PI*0.15f);
           m_object->SetPartRotationZ(2, -progress*Math::PI*0.15f);

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Math::Rand()*1.0f);
               m_phase    = 1;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 1 )  // wait ?
       {
           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/2.0f;
               m_phase    = 2;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 2 )  // down ?
       {
           m_object->SetPartRotationZ(1,  (1.0f-progress)*Math::PI*0.15f);
           m_object->SetPartRotationZ(2, -(1.0f-progress)*Math::PI*0.15f);

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/0.2f;
               m_phase    = 3;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 3 )  // close ?
       {
           m_object->SetPartPosition(1, Math::Vector(-(26.0f-progress*5.0f), 13.0f, 0.0f));
           m_object->SetPartPosition(2, Math::Vector( (26.0f-progress*5.0f), 13.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(1.0f+Math::Rand()*1.0f);
               m_phase    = 4;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 4 )  // open ?
       {
           m_object->SetPartPosition(1, Math::Vector(-(26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));
           m_object->SetPartPosition(2, Math::Vector( (26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/0.2f;
               m_phase    = 5;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 5 )  // close ?
       {
           m_object->SetPartPosition(1, Math::Vector(-(26.0f-progress*5.0f), 13.0f, 0.0f));
           m_object->SetPartPosition(2, Math::Vector( (26.0f-progress*5.0f), 13.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(1.0f+Math::Rand()*1.0f);
               m_phase    = 6;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 6 )  // open ?
       {
           m_object->SetPartPosition(1, Math::Vector(-(26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));
           m_object->SetPartPosition(2, Math::Vector( (26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/2.0f;
               m_phase    = 0;
               progress   = 0.0f;
           }
       }
   }

   if ( m_type == OBJECT_ALIEN7 )  // électrocuteur ?
   {
       if ( progress >= 1.0f )
       {
           pos = m_object->GetPartPosition(0);
           if ( Math::Distance(pos, m_engine->GetLookatPt()) < 300.0f )
           {
               mat = m_object->GetWorldMatrix(0);
               src = Math::Transform(*mat, Math::Vector(-22.0f, 13.0f, 0.0f));
               dst = Math::Transform(*mat, Math::Vector( 22.0f, 13.0f, 0.0f));
               duration = 1.0f+Math::Rand()*2.0f;
               m_particle->CreateRay(src, dst, Gfx::PARTIRAY2, Math::Point(6.0f, 6.0f), duration);

               dim.x = 8.5f;
               dim.y = dim.x;
               m_particle->CreateParticle(src, Math::Vector(0.0f, 0.0f, 0.0f),
                                            dim, Gfx::PARTISPHERE1, duration, 0.0f);
               m_particle->CreateParticle(dst, Math::Vector(0.0f, 0.0f, 0.0f),
                                            dim, Gfx::PARTISPHERE1, duration, 0.0f);
           }

           m_progress = 0.0f;
           m_speed    = 1.0f/(4.0f+Math::Rand()*4.0f);
           m_phase    = 0;
           progress   = 0.0f;
       }
   }


   if ( m_type == OBJECT_ALIEN8 )  // générateur ?
   {
       FireStopUpdate();

       if ( m_phase == 0 )  // create ?
       {
           if ( m_bot != 0 )
           {
               ArmPosition(pos, dir, 50.0f*Math::PI/180.0f, 0.0f);
               m_bot->SetPartPosition(0, pos);
               m_bot->SetPartRotationY(0, dir+progress*Math::PI*10.0f);
               m_bot->SetPartScale(0, progress);

               if ( m_lastParticule+m_engine->ParticleAdapt(0.10f) <= m_time &&
                    Math::Distance(m_object->GetPartPosition(0), m_engine->GetLookatPt()) < 500.0f )
               {
                   m_lastParticule = m_time;

                   mat = m_object->GetWorldMatrix(0);
                   src = Math::Transform(*mat, Math::Vector(-14.0f, 10.0f, -18.0f));

                   pos = src;
                   c.x = pos.x;
                   c.y = pos.z;
                   p.x = c.x;
                   p.y = c.y;
                   p = Math::RotatePoint(c, Math::Rand()*Math::PI*2.0f, p);
                   pos.x = p.x;
                   pos.z = p.y;
                   pos.y += 2.0f+Math::Rand()*3.0f;
                   speed = Math::Vector(0.0f, 0.0f, 0.0f);
                   dim.x = Math::Rand()*3.0f+1.5f;
                   dim.y = dim.x;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, 1.0f, 0.0f);

                   pos = src;
                   speed.x = (Math::Rand()-0.5f)*30.0f;
                   speed.z = (Math::Rand()-0.5f)*30.0f;
                   speed.y = Math::Rand()*20.0f+10.0f;
                   dim.x = Math::Rand()*0.4f+0.4f;
                   dim.y = dim.x;
                   m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK2, 2.0f, 50.0f, 1.2f, 1.2f);

                   pos = src;
                   pos.y += 10.0f;
                   speed.x = (Math::Rand()-0.5f)*1.5f;
                   speed.z = (Math::Rand()-0.5f)*1.5f;
                   speed.y = -8.0f;
                   dim.x = Math::Rand()*2.0f+2.0f;
                   dim.y = dim.x;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFIREZ, 1.0f, 0.0f);

                   m_sound->Play(SOUND_ENERGY, m_object->GetPartPosition(0),
                                 1.0f, 1.0f+Math::Rand()*1.5f);
               }
           }

           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 1;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 1 )  // close ?
       {
           m_object->SetPartRotationY(2,  (45.0f*(1.0f-progress))*Math::PI/180.0f);
           m_object->SetPartRotationY(3, -(45.0f*(1.0f-progress))*Math::PI/180.0f);

           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   BotAction(m_bot, MB_TRUCK, 1.0f);

                   m_progress = 0.0f;
                   m_speed    = 1.0f/2.0f;
                   m_phase    = 2;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 2 )  // ccw ?
       {
           angle = (50.0f-100.0f*Math::Soft(progress, 2))*Math::PI/180.0f;
           m_object->SetPartRotationY(1, angle);

           if ( m_bot != 0 )
           {
               ArmPosition(pos, dir, angle, 0.0f);
               m_bot->SetPartPosition(0, pos);
               m_bot->SetPartRotationY(0, dir);
           }

           if ( progress >= 1.0f )
           {
               BotAction(m_bot, MB_FEAR, 3.3f);

               m_progress = 0.0f;
               m_speed    = 1.0f/2.0f;
               m_phase    = 3;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 3 )  // open ?
       {
           factor = Math::Norm(progress*2.0f);
           m_object->SetPartRotationY(2,  (45.0f*factor)*Math::PI/180.0f);
           m_object->SetPartRotationY(3, -(45.0f*factor)*Math::PI/180.0f);

           if ( m_bot != 0 )
           {
               ArmPosition(pos, dir, -50.0f*Math::PI/180.0f, progress*15.0f);
               m_bot->SetPartPosition(0, pos);
               m_bot->SetPartRotationY(0, dir);
           }

           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/2.0f;
                   m_phase    = 4;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 4 )  // cw ?
       {
           m_object->SetPartRotationY(1, (50.0f-100.0f*Math::Soft(1.0f-progress, 2))*Math::PI/180.0f);

           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   if ( m_bot != 0 )
                   {
                       m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOT, m_bot);
                       m_bot = 0;
                   }

                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 5;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 5 )  // wait ?
       {
           if ( m_lastParticule+m_engine->ParticleAdapt(0.10f) <= m_time &&
                Math::Distance(m_object->GetPartPosition(0), m_engine->GetLookatPt()) < 500.0f )
           {
               m_lastParticule = m_time;

               mat = m_object->GetWorldMatrix(0);
               src = Math::Transform(*mat, Math::Vector(-14.0f, 10.0f, -18.0f));

               pos = src;
               pos.y += 10.0f;
               speed.x = (Math::Rand()-0.5f)*1.5f;
               speed.z = (Math::Rand()-0.5f)*1.5f;
               speed.y = -8.0f;
               dim.x = Math::Rand()*2.0f+2.0f;
               dim.y = dim.x;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFIREZ, 1.0f, 0.0f);
           }

           if ( progress >= 1.0f )
           {
               if ( m_bStop )  // bouton rouge pressé ?
               {
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
                   m_phase    = 6;
                   progress   = 0.0f;
               }
               else
               {
                   if ( Math::Distance(m_object->GetPartPosition(0), m_engine->GetLookatPt()) < 500.0f )
                   {
                       ArmPosition(pos, dir, 50.0f*Math::PI/180.0f, 0.0f);

                       for ( i=0 ; i<20 ; i++ )
                       {
                           r = rand()%3;
                           if ( r == 0 )  type = OBJECT_BOT1;
                           if ( r == 1 )  type = OBJECT_BOT2;
                           if ( r == 2 )  type = OBJECT_BOT4;
                           if ( type != m_lastType )  break;
                       }
                       m_lastType = type;

                       m_bot = static_cast<COldObject*>(CObjectManager::GetInstancePointer()->CreateObject(pos, dir, type)); // TODO (krzys_h): bPlumb=true (?)
                       m_bot->SetScale(0.0f);
                       m_bot->SetLock(true);
                   }

                   SynchroHammer(3);  // synchro

                   m_progress = 0.0f;
                   m_speed    = 1.0f/3.0f;
                   m_phase    = 0;
                   progress   = 0.0f;
               }
           }
       }

       if ( m_phase == 6 )  // dead ?
       {
       }
   }

   if ( m_type == OBJECT_ALIEN9 )  // torture ?
   {
       if ( m_phase == 0 )  // down ?
       {
           if ( !m_main->IsGameTime() )
           {
               m_rotAngle += m_rotSpeed*event.rTime;
               m_object->SetPartRotationY(3, m_rotAngle);

               m_progress -= event.rTime*m_speed;
               return true;
           }

           pos = Math::Vector(0.0f, 34.0f, 0.0f);
           pos.y -= progress*3.5f;
           m_object->SetPartPosition(1, pos);

           pos = Math::Vector(0.0f, -1.0f, 0.0f);
           pos.y -= progress*3.5f;
           m_object->SetPartPosition(2, pos);

           pos = Math::Vector(0.0f, 1.0f, 0.0f);
           pos.y -= progress*3.5f;
           m_object->SetPartPosition(3, pos);

           m_rotAngle += m_rotSpeed*event.rTime;
           m_object->SetPartRotationY(3, m_rotAngle);

           if ( m_bot != 0 && progress >= 1.0f-1.0f*m_speed && !m_bFear )
           {
               m_bFear = true;
               BotAction(m_bot, MB_FEAR, 3.0f);  // blupi a peur
           }

           if ( progress >= 1.0f )
           {
               m_object->SetLock(true);  // plus détecté par la suite
               m_main->SetMissionResult(INFO_LOST, m_object->GetPosition());

               if ( m_bot != 0 )
               {
                   m_engine->GetPyroManager()->Create(Gfx::PT_FRAGT, m_bot);  // mort de blupi
                   m_bot = 0;
               }

               m_progress = 0.0f;
               m_speed    = 1.0f/10.0f;
               m_phase    = 1;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 1 )  // ended ?
       {
       }

       if ( m_phase == 2 )  // court-circuit ?
       {
           m_rotSpeed = 2.0f+progress*12.0f;
           m_rotAngle += m_rotSpeed*event.rTime;
           m_object->SetPartRotationY(3, m_rotAngle);
           m_object->SetPartRotationX(3, progress*0.2f);

           if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               mat = m_object->GetWorldMatrix(0);
               pos = Math::Vector(0.0f, 5.0f, -14.5f);
               pos = Math::Transform(*mat, pos);

               for ( i=0 ; i<4 ; i++ )
               {
                   speed.x = (Math::Rand()-0.5f)*20.0f;
                   speed.z = (Math::Rand()-0.5f)*20.0f;
                   speed.y = 5.0f+Math::Rand()*10.0f;
                   dim.x = 0.8f;
                   dim.y = 0.8f;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 40.0f);
               }

               pos.x = (Math::Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Math::Rand()-0.5f)*2.0f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*1.0f;
               speed.z = (Math::Rand()-0.5f)*1.0f;
               speed.y = 1.0f+Math::Rand()*2.0f;
               dim.x = Math::Rand()*1.2f+2.4f;
               dim.y = dim.x;
               duration = 1.0f+Math::Rand()*2.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, duration);

               pos.x = (Math::Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Math::Rand()-0.5f)*2.0f;
               pos = Math::Transform(*mat, pos);
               speed.x = 0.0f;
               speed.z = 0.0f;
               speed.y = 4.0f+Math::Rand()*5.0f;
               dim.x = Math::Rand()*1.8f+3.3f;
               dim.y = dim.x;
               duration = 2.0f+Math::Rand()*4.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, duration);
           }

           if ( progress >= 1.0f )
           {
               mat = m_object->GetWorldMatrix(3);
               for ( i=0 ; i<15 ; i++ )
               {
                   pos.x = (Math::Rand()-0.5f)*5.0f;
                   pos.z = (Math::Rand()-0.5f)*5.0f;
                   pos.y = 1.0f;
                   pos = Math::Transform(*mat, pos);
                   speed.x = (Math::Rand()-0.5f)*50.0f;
                   speed.z = (Math::Rand()-0.5f)*50.0f;
                   speed.y = Math::Rand()*15.0f;
                   dim.x = 2.0f;
                   dim.y = dim.x;
                   duration = Math::Rand()*3.0f+2.0f;
                   mass = Math::Rand()*10.0f+15.0f;
                   m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK2,
                                            duration, mass, Math::Rand()+0.7f, 2.0f);
               }

               m_sound->Play(SOUND_BOUMm, m_engine->GetEyePt(), 1.0f, 0.7f);

               speed.x = (Math::Rand()-0.5f)*20.0f;
               speed.z = (Math::Rand()-0.5f)*20.0f;
               speed.y = 10.0f+Math::Rand()*10.0f;
//?                m_object->DetachPart(3, speed);  // détache le piston
               m_object->ExploPiece(3);

               speed.x = (Math::Rand()-0.5f)*20.0f;
               speed.z = (Math::Rand()-0.5f)*20.0f;
               speed.y = 10.0f+Math::Rand()*10.0f;
               m_object->DetachPart(2, speed);  // détache le piston
//?                m_object->ExploPiece(2);

               speed.x = (Math::Rand()-0.5f)*20.0f;
               speed.z = (Math::Rand()-0.5f)*20.0f;
               speed.y = 10.0f+Math::Rand()*10.0f;
               m_object->DetachPart(1, speed);  // détache le piston
//?                m_object->ExploPiece(1);

               BotAction(m_bot, MB_GOHOME, 1.0f);  // blupi debout
               BotAction(m_bot, MB_HAPPY, 1.0f);  // blupi heureux
               m_object->SetLock(true);  // plus détecté par la suite

               m_progress = 0.0f;
               m_speed    = 1.0f/5.0f;
               m_phase    = 3;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 3 )  // smoke ?
       {
           mat = m_object->GetWorldMatrix(0);
           skip = Math::Norm(progress*6.0f);
           angle = m_object->GetPartRotationY(0)+Math::PI*0.5f;
           angle += Math::PI*2.25f*skip;
           m_bot->SetPartRotationY(0, angle);
           if ( skip < 0.5f )
           {
               src = Math::Transform(*mat, Math::Vector(0.0f, 5.6f, -0.5f));
               dst = Math::Transform(*mat, Math::Vector(4.0f, 7.0f, -3.5f));
               skip = powf(skip, 0.5f);
           }
           else
           {
               src = Math::Transform(*mat, Math::Vector(4.0f, 7.0f, -2.5f));
               dst = Math::Transform(*mat, Math::Vector(8.0f, 2.0f, -7.0f));
               skip = powf(skip, 2.0f);
           }
           pos = src+(dst-src)*skip;
           pos.y += m_bot->GetCharacter()->height;
           m_bot->SetPartPosition(0, pos);  // déplace blupi
           if ( skip == 1.0f )
           {
               BotAction(m_bot, MB_WAIT, 1.0f);  // blupi attend
           }

           if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               mat = m_object->GetWorldMatrix(0);

               pos.x = (Math::Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Math::Rand()-0.5f)*2.0f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*1.0f;
               speed.z = (Math::Rand()-0.5f)*1.0f;
               speed.y = 1.0f+Math::Rand()*2.0f;
               dim.x = Math::Rand()*1.2f+2.4f;
               dim.y = dim.x;
               duration = 1.0f+Math::Rand()*2.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, duration);

               pos.x = (Math::Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Math::Rand()-0.5f)*2.0f;
               pos = Math::Transform(*mat, pos);
               speed.x = 0.0f;
               speed.z = 0.0f;
               speed.y = 4.0f+Math::Rand()*5.0f;
               dim.x = Math::Rand()*1.8f+3.3f;
               dim.y = dim.x;
               duration = 2.0f+Math::Rand()*4.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, duration);
           }

           if ( progress >= 1.0f )
           {
               BotAction(m_bot, MB_WAIT, 1.0f);  // blupi attend

               m_progress = 0.0f;
               m_speed    = 1.0f/10.0f;
               m_phase    = 4;
               progress   = 0.0f;
               m_lastParticule = 0.0f;
           }
       }

       if ( m_phase == 4 )  // saved ?
       {
       }
   }

   return true;
}

// Calcule la position et l'orientation du robot au bout du bras
// selon l'angle (50..-50), pour le générateur.

void CAutoAlien::ArmPosition(Math::Vector &pos, float &dir, float angle, float lin)
{
   Math::Matrix*  mat;
   Math::Point      rot;

   rot = Math::RotatePoint(Math::PI*1.5f-angle, 51.0f);
   rot.x += 25.0f+lin;
   rot.y += 15.0f;

   mat = m_object->GetWorldMatrix(0);
   pos = Math::Transform(*mat, Math::Vector(rot.x, 10.0f, rot.y));

   if ( m_bot != 0 )
   {
       pos.y += m_bot->GetCharacter()->height*m_bot->GetPartScaleY(0);
   }

   dir = angle + m_object->GetPartRotationY(0) + Math::PI/2.0f;
}

// Démarre une action pour le robot.

void CAutoAlien::BotAction(COldObject *bot, int action, float delay)
{
   CMotion*    motion;

   if ( bot == 0 )  return;

   motion = bot->GetMotion();
   if ( motion == 0 )  return;

   motion->SetAction(action, delay);
}

// Cherche l'objet marteau à droite pour le synchroniser.

void CAutoAlien::SynchroHammer(int action)
{
   Math::Matrix*  mat;
   Math::Vector   pos;
   COldObject*    hammer;
   CAuto*      automat;

   mat = m_object->GetWorldMatrix(0);
   pos = Math::Transform(*mat, Math::Vector(80.0f, 0.0f, 0.0f));
   hammer = SearchObject(OBJECT_ALIEN5, pos, 40.0f);
   if ( hammer == 0 )  return;

   automat = hammer->GetAuto();
   if ( automat == 0 )  return;

   automat->Start(action);
}

// Cherche un objet proche.

COldObject* CAutoAlien::SearchObject(ObjectType type, Math::Vector center, float radius)
{
   COldObject     *pBest;
   Math::Vector   pos;
   float       min, dist;
   int         i;

   pBest = 0;
   min = 100000.0f;
   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       if ( !pObj->Implements(ObjectInterfaceType::Old) ) continue;
       if ( dynamic_cast<COldObject*>(pObj)->IsDying() )  continue;

       if ( type != pObj->GetType() )  continue;

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

// Met à jour les feux de stop.

void CAutoAlien::FireStopUpdate()
{
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   bool        bOn;
   int         i;

   if ( m_type != OBJECT_ALIEN8 )  return;
   
   static float listpos[3*12] =
   {
       -28.0f, 33.0f,  -4.0f,
       -33.5f, 33.0f, -13.0f,
       -33.5f, 33.0f, -23.0f,
       -28.0f, 33.0f, -32.0f,
       -19.0f, 33.0f, -37.5f,
        -9.0f, 33.0f, -37.5f,
         0.0f, 33.0f, -32.5f,
         5.5f, 33.0f, -23.5f,
         5.5f, 33.0f, -13.5f,
         0.0f, 33.0f,  -4.5f,
        -9.0f, 33.0f,   1.5f,
       -19.0f, 33.0f,   1.5f,
   };

   if ( m_bStop )  // éteint ?
   {
       for ( i=0 ; i<12 ; i++ )
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

   for ( i=0 ; i<12 ; i++ )
   {
       bOn = false;

       if ( Math::Mod(m_time+0.1f*i, 0.6f) >= 0.4f )  bOn = true;

       if ( bOn )
       {
           if ( m_partiStop[i] == -1 )
           {
               pos.x = listpos[i*3+0];
               pos.y = listpos[i*3+1];
               pos.z = listpos[i*3+2];
               pos = Math::Transform(*mat, pos);
               m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                             dim, Gfx::PARTISELY,
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


// Stoppe l'automate.

bool CAutoAlien::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoAlien::GetError()
{
   return ERR_OK;
}


// Met à jour le mapping de la texture des chenilles.

void CAutoAlien::UpdateTrackMapping(float progress)
{
   Gfx::Material    mat;
   int             rank;

   if ( Math::Distance(m_object->GetPartPosition(0), m_engine->GetLookatPt()) > 500.0f )  return;

   mat.diffuse.r = 1.0f;
   mat.diffuse.g = 1.0f;
   mat.diffuse.b = 1.0f;  // blanc
   mat.ambient.r = 0.5f;
   mat.ambient.g = 0.5f;
   mat.ambient.b = 0.5f;

   rank = m_object->GetObjectRank(0);

   m_engine->TrackTextureMapping(rank, mat, Gfx::ENG_RSTATE_PART1, "alien1.tga", "",
                                 Gfx::ENG_TEX_MAPPING_Z,
                                 progress, 3.0f, 8.0f, 128.0f, 256.0f);
}

