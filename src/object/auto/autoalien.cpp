// autoalien.cpp

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
#include "pyro.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "motion.h"
#include "motionbot.h"
#include "auto.h"
#include "autoalien.h"





// Constructeur de l'objet.

CAutoAlien::CAutoAlien(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   int     i;

   CAuto::CAuto(iMan, object);

   for ( i=0 ; i<12 ; i++ )
   {
       m_partiStop[i] = -1;
   }

   Init();
}

// Destructeur de l'objet.

CAutoAlien::~CAutoAlien()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoAlien::DeleteObject(BOOL bAll)
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
   m_bSynchro = FALSE;
   m_bStop = FALSE;
   m_bFear = FALSE;

   m_type = m_object->RetType();
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
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   float       duration, mass, angle, value;
   int         i;

   if ( m_type == OBJECT_ALIEN5 )  // marteau ?
   {
       if ( param == 2 )  // synchronisé avec générateur ?
       {
           m_bSynchro = TRUE;
       }
       if ( param == 3 && m_bSynchro )  // synchro venant du générateur ?
       {
           m_progress = 0.0f;
           m_speed    = 1.0f/8.0f;
           m_phase    = 0;
       }
       if ( param == 4 && !m_bStop )  // stoppe ?
       {
           m_bStop = TRUE;
       }
   }

   if ( m_type == OBJECT_ALIEN8 )  // générateur ?
   {
       if ( param == 2 && !m_bStop )  // bouton rouge pressé ?
       {
           m_bStop = TRUE;

           m_object->SetAngleX(4, (Rand()-0.5f)*2.0f);
           m_object->SetAngleY(4, (Rand()-0.5f)*2.0f);

           mat = m_object->RetWorldMatrix(4);  // bouton rouge
           for ( i=0 ; i<15 ; i++ )
           {
               pos.x = 0.0f;
               pos.y = (Rand()-0.5f)*5.0f;
               pos.z = (Rand()-0.5f)*5.0f;
               pos = Transform(*mat, pos);
               speed.x = (Rand()-0.5f)*30.0f;
               speed.z = (Rand()-0.5f)*30.0f;
               speed.y = Rand()*30.0f;
               dim.x = 1.0f;
               dim.y = dim.x;
               duration = Rand()*3.0f+2.0f;
               mass = Rand()*10.0f+15.0f;
               m_particule->CreateTrack(pos, speed, dim, PARTITRACK1,
                                        duration, mass, Rand()+0.7f, 1.0f);
           }

           m_sound->Play(SOUND_BREAK2, m_engine->RetEyePt(), 1.0f, 0.7f);

           m_object->SetLock(TRUE);  // il ne sera plus compté !
           SynchroHammer(4);  // stoppe
       }
   }

   if ( m_type == OBJECT_ALIEN9 )  // torture ?
   {
       if ( param == 1 )  // démarre ?
       {
           value = m_object->RetCmdLine(0);
           if ( value == NAN || value == 0.0f )
           {
               m_speed = 1.0f/10.0f;
           }
           else
           {
               m_speed = 1.0f/value;
           }

           mat = m_object->RetWorldMatrix(0);
           pos = Transform(*mat, D3DVECTOR(0.0f, 5.6f, -0.5f));
           angle = m_object->RetAngleY(0)+PI*0.5f;

           m_bot = new CObject(m_iMan);
           if ( !m_bot->CreateBot(pos, angle, 1.0f, OBJECT_BOT3, TRUE) )
           {
               delete m_bot;
               m_bot = 0;
           }
           else
           {
               pos.y += m_bot->RetCharacter()->height;
               m_bot->SetPosition(0, pos);
               BotAction(m_bot, MB_HOME2, 1.0f);  // blupi assis
               BotAction(m_bot, MB_ANGRY, 1.0f);  // blupi triste
           }
       }

       if ( param == 2 && m_phase == 0 && m_bot != 0 )  // stoppe ?
       {
//?            m_sound->Play(SOUND_BREAK2, m_engine->RetEyePt(), 1.0f, 1.4f);

           m_progress = 0.0f;
           m_speed    = 1.0f/2.0f;
           m_phase    = 2;  // court-circuit
       }
   }
}


// Gestion d'un événement.

BOOL CAutoAlien::EventProcess(const Event &event)
{
   CPyro*      pyro;
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed, src, dst, eye;
   FPOINT      dim, rot, c, p;
   ObjectType  type;
   float       progress, duration, delay, mass, angle, dir, factor, skip;
   int         i, r;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;

   m_progress += event.rTime*m_speed;
   progress = Norm(m_progress);

   if ( m_type == OBJECT_ALIEN2 )  // tour ?
   {
       if ( m_phase == 0 )  // up ?
       {
           m_object->SetPosition(1, D3DVECTOR(0.0f, 30.0f+progress*40.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               delay = 0.5f+Rand()*1.0f;

               pos = m_object->RetPosition(0);
               eye = m_engine->RetEyePt();
               pos = eye+(pos-eye)*0.6f;
               i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, TRUE);
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
           m_object->SetPosition(1, D3DVECTOR(0.0f, 30.0f+(1.0f-progress)*40.0f, 0.0f));

           pos = m_object->RetPosition(0);
           if ( Length(pos, m_engine->RetLookatPt()) < 500.0f &&
                m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<10 ; i++ )
               {
                   rot = RotatePoint(Rand()*PI*2.0f, 35.0f);
                   src = m_object->RetPosition(1);
                   src.x = pos.x+rot.x;
                   src.z = pos.z+rot.y;
                   src.y += pos.y+5.0f;
                   speed = D3DVECTOR(0.0f, 10.0f+Rand()*10.0f, 0.0f);
                   dim.x = Rand()*2.0f+2.0f;
                   dim.y = dim.x;
                   duration = 1.0f+Rand()*1.0f;
                   m_particule->CreateParticule(src, speed, dim, PARTIBLITZ, duration);
               }
           }

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Rand()*0.5f);
               m_phase    = 2;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 2 )  // wait ?
       {
           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Rand()*4.0f);
               m_phase    = 0;
               progress   = 0.0f;
           }
       }
   }

   if ( m_type == OBJECT_ALIEN3 )  // tour haute ?
   {
       if ( m_phase == 0 )  // up ?
       {
           m_object->SetPosition(1, D3DVECTOR(0.0f,  30.0f+progress*60.0f, 0.0f));
           m_object->SetPosition(2, D3DVECTOR(0.0f, 100.0f+(1.0f-progress)*40.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               delay = 0.5f+Rand()*1.0f;

               pos = m_object->RetPosition(0);
               eye = m_engine->RetEyePt();
               pos = eye+(pos-eye)*0.6f;
               i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, TRUE);
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
           m_object->SetPosition(1, D3DVECTOR(0.0f,  30.0f+(1.0f-progress)*60.0f, 0.0f));
           m_object->SetPosition(2, D3DVECTOR(0.0f, 100.0f+progress*40.0f, 0.0f));

           pos = m_object->RetPosition(0);
           if ( Length(pos, m_engine->RetLookatPt()) < 500.0f &&
                m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<10 ; i++ )
               {
                   rot = RotatePoint(Rand()*PI*2.0f, 35.0f);
                   src = m_object->RetPosition(1);
                   src.x = pos.x+rot.x;
                   src.z = pos.z+rot.y;
                   src.y += pos.y+5.0f;
                   speed = D3DVECTOR(0.0f, 20.0f+Rand()*20.0f, 0.0f);
                   dim.x = Rand()*2.0f+2.0f;
                   dim.y = dim.x;
                   duration = 1.0f+Rand()*1.0f;
                   m_particule->CreateParticule(src, speed, dim, PARTIBLITZ, duration);
               }
           }

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Rand()*0.5f);
               m_phase    = 2;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 2 )  // wait ?
       {
           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Rand()*4.0f);
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

           m_object->SetAngleX(1, progress*PI*0.25f);

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
                   else               m_speed = 1.0f/(0.5f+Rand()*1.0f);
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
           m_object->SetAngleX(1, (1.0f-progress)*PI*0.25f);

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
                   pos = m_object->RetPosition(0);
                   if ( Length(pos, m_engine->RetLookatPt()) < 500.0f )
                   {
                       mat = m_object->RetWorldMatrix(0);
                       for ( i=0 ; i<10 ; i++ )
                       {
                           pos.x =   0.0f+(Rand()-0.5f)*25.0f;
                           pos.z = -18.0f+(Rand()-0.5f)*25.0f;
                           pos.y =  10.0f;
                           pos = Transform(*mat, pos);
                           speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                           dim.x = Rand()*20.0f+20.0f;
                           dim.y = dim.x;
                           duration = 2.0f+Rand()*2.0f;
                           m_particule->CreateParticule(pos, speed, dim, PARTICRASH, duration);
                       }
                       for ( i=0 ; i<10 ; i++ )
                       {
                           angle = Rand()*PI*2.0f;
                           rot = RotatePoint(angle, 13.0f);
                           pos.x = rot.x;
                           pos.z = rot.y-18.0f;
                           pos.y = 10.0f;
                           pos = Transform(*mat, pos);
                           rot = RotatePoint(angle, 25.0f+Rand()*25.0f);
                           speed.x = rot.x;
                           speed.z = rot.y-18.0f;
                           speed.y = 10.0f+Rand()*30.0f;
                           speed = Transform(*mat, speed);
                           speed -= pos;
                           dim.x = 1.0f;
                           dim.y = dim.x;
                           duration = Rand()*3.0f+2.0f;
                           mass = Rand()*20.0f+30.0f;
                           m_particule->CreateTrack(pos, speed, dim, PARTITRACK1,
                                                    duration, mass, Rand()+0.7f, 1.0f);
                       }
                       eye = m_engine->RetEyePt();
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
                   else               m_speed = 1.0f/(2.0f+Rand()*4.0f);
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
           m_object->SetAngleZ(1,  progress*PI*0.15f);
           m_object->SetAngleZ(2, -progress*PI*0.15f);

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(0.5f+Rand()*1.0f);
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
           m_object->SetAngleZ(1,  (1.0f-progress)*PI*0.15f);
           m_object->SetAngleZ(2, -(1.0f-progress)*PI*0.15f);

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
           m_object->SetPosition(1, D3DVECTOR(-(26.0f-progress*5.0f), 13.0f, 0.0f));
           m_object->SetPosition(2, D3DVECTOR( (26.0f-progress*5.0f), 13.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(1.0f+Rand()*1.0f);
               m_phase    = 4;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 4 )  // open ?
       {
           m_object->SetPosition(1, D3DVECTOR(-(26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));
           m_object->SetPosition(2, D3DVECTOR( (26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));

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
           m_object->SetPosition(1, D3DVECTOR(-(26.0f-progress*5.0f), 13.0f, 0.0f));
           m_object->SetPosition(2, D3DVECTOR( (26.0f-progress*5.0f), 13.0f, 0.0f));

           if ( progress >= 1.0f )
           {
               m_progress = 0.0f;
               m_speed    = 1.0f/(1.0f+Rand()*1.0f);
               m_phase    = 6;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 6 )  // open ?
       {
           m_object->SetPosition(1, D3DVECTOR(-(26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));
           m_object->SetPosition(2, D3DVECTOR( (26.0f-(1.0f-progress)*5.0f), 13.0f, 0.0f));

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
           pos = m_object->RetPosition(0);
           if ( Length(pos, m_engine->RetLookatPt()) < 300.0f )
           {
               mat = m_object->RetWorldMatrix(0);
               src = Transform(*mat, D3DVECTOR(-22.0f, 13.0f, 0.0f));
               dst = Transform(*mat, D3DVECTOR( 22.0f, 13.0f, 0.0f));
               duration = 1.0f+Rand()*2.0f;
               m_particule->CreateRay(src, dst, PARTIRAY2, FPOINT(6.0f, 6.0f), duration);

               dim.x = 8.5f;
               dim.y = dim.x;
               m_particule->CreateParticule(src, D3DVECTOR(0.0f, 0.0f, 0.0f),
                                            dim, PARTISPHERE1, duration, 0.0f);
               m_particule->CreateParticule(dst, D3DVECTOR(0.0f, 0.0f, 0.0f),
                                            dim, PARTISPHERE1, duration, 0.0f);
           }

           m_progress = 0.0f;
           m_speed    = 1.0f/(4.0f+Rand()*4.0f);
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
               ArmPosition(pos, dir, 50.0f*PI/180.0f, 0.0f);
               m_bot->SetPosition(0, pos);
               m_bot->SetAngleY(0, dir+progress*PI*10.0f);
               m_bot->SetZoom(0, progress);

               if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time &&
                    Length(m_object->RetPosition(0), m_engine->RetLookatPt()) < 500.0f )
               {
                   m_lastParticule = m_time;

                   mat = m_object->RetWorldMatrix(0);
                   src = Transform(*mat, D3DVECTOR(-14.0f, 10.0f, -18.0f));

                   pos = src;
                   c.x = pos.x;
                   c.y = pos.z;
                   p.x = c.x;
                   p.y = c.y;
                   p = RotatePoint(c, Rand()*PI*2.0f, p);
                   pos.x = p.x;
                   pos.z = p.y;
                   pos.y += 2.0f+Rand()*3.0f;
                   speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                   dim.x = Rand()*3.0f+1.5f;
                   dim.y = dim.x;
                   m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 1.0f, 0.0f);

                   pos = src;
                   speed.x = (Rand()-0.5f)*30.0f;
                   speed.z = (Rand()-0.5f)*30.0f;
                   speed.y = Rand()*20.0f+10.0f;
                   dim.x = Rand()*0.4f+0.4f;
                   dim.y = dim.x;
                   m_particule->CreateTrack(pos, speed, dim, PARTITRACK2, 2.0f, 50.0f, 1.2f, 1.2f);

                   pos = src;
                   pos.y += 10.0f;
                   speed.x = (Rand()-0.5f)*1.5f;
                   speed.z = (Rand()-0.5f)*1.5f;
                   speed.y = -8.0f;
                   dim.x = Rand()*2.0f+2.0f;
                   dim.y = dim.x;
                   m_particule->CreateParticule(pos, speed, dim, PARTIFIREZ, 1.0f, 0.0f);

                   m_sound->Play(SOUND_ENERGY, m_object->RetPosition(0),
                                 1.0f, 1.0f+Rand()*1.5f);
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
           m_object->SetAngleY(2,  (45.0f*(1.0f-progress))*PI/180.0f);
           m_object->SetAngleY(3, -(45.0f*(1.0f-progress))*PI/180.0f);

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
           angle = (50.0f-100.0f*Soft(progress, 2))*PI/180.0f;
           m_object->SetAngleY(1, angle);

           if ( m_bot != 0 )
           {
               ArmPosition(pos, dir, angle, 0.0f);
               m_bot->SetPosition(0, pos);
               m_bot->SetAngleY(0, dir);
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
           factor = Norm(progress*2.0f);
           m_object->SetAngleY(2,  (45.0f*factor)*PI/180.0f);
           m_object->SetAngleY(3, -(45.0f*factor)*PI/180.0f);

           if ( m_bot != 0 )
           {
               ArmPosition(pos, dir, -50.0f*PI/180.0f, progress*15.0f);
               m_bot->SetPosition(0, pos);
               m_bot->SetAngleY(0, dir);
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
           m_object->SetAngleY(1, (50.0f-100.0f*Soft(1.0f-progress, 2))*PI/180.0f);

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
                       pyro = new CPyro(m_iMan);
                       pyro->Create(PT_EXPLOT, m_bot);
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
           if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time &&
                Length(m_object->RetPosition(0), m_engine->RetLookatPt()) < 500.0f )
           {
               m_lastParticule = m_time;

               mat = m_object->RetWorldMatrix(0);
               src = Transform(*mat, D3DVECTOR(-14.0f, 10.0f, -18.0f));

               pos = src;
               pos.y += 10.0f;
               speed.x = (Rand()-0.5f)*1.5f;
               speed.z = (Rand()-0.5f)*1.5f;
               speed.y = -8.0f;
               dim.x = Rand()*2.0f+2.0f;
               dim.y = dim.x;
               m_particule->CreateParticule(pos, speed, dim, PARTIFIREZ, 1.0f, 0.0f);
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
                   if ( Length(m_object->RetPosition(0), m_engine->RetLookatPt()) < 500.0f )
                   {
                       ArmPosition(pos, dir, 50.0f*PI/180.0f, 0.0f);

                       for ( i=0 ; i<20 ; i++ )
                       {
                           r = rand()%3;
                           if ( r == 0 )  type = OBJECT_BOT1;
                           if ( r == 1 )  type = OBJECT_BOT2;
                           if ( r == 2 )  type = OBJECT_BOT4;
                           if ( type != m_lastType )  break;
                       }
                       m_lastType = type;

                       m_bot = new CObject(m_iMan);
                       m_bot->CreateBot(pos, dir, 1.0f, type, TRUE);
                       m_bot->SetZoom(0, 0.0f);
                       m_bot->SetLock(TRUE);
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
               m_object->SetAngleY(3, m_rotAngle);

               m_progress -= event.rTime*m_speed;
               return TRUE;
           }

           pos = D3DVECTOR(0.0f, 34.0f, 0.0f);
           pos.y -= progress*3.5f;
           m_object->SetPosition(1, pos);

           pos = D3DVECTOR(0.0f, -1.0f, 0.0f);
           pos.y -= progress*3.5f;
           m_object->SetPosition(2, pos);

           pos = D3DVECTOR(0.0f, 1.0f, 0.0f);
           pos.y -= progress*3.5f;
           m_object->SetPosition(3, pos);

           m_rotAngle += m_rotSpeed*event.rTime;
           m_object->SetAngleY(3, m_rotAngle);

           if ( m_bot != 0 && progress >= 1.0f-1.0f*m_speed && !m_bFear )
           {
               m_bFear = TRUE;
               BotAction(m_bot, MB_FEAR, 3.0f);  // blupi a peur
           }

           if ( progress >= 1.0f )
           {
               m_object->SetLock(TRUE);  // plus détecté par la suite
               m_main->SetSuperLost(m_object->RetPosition(0));

               if ( m_bot != 0 )
               {
                   pyro = new CPyro(m_iMan);
                   pyro->Create(PT_FRAGT, m_bot);  // mort de blupi
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
           m_object->SetAngleY(3, m_rotAngle);
           m_object->SetAngleX(3, progress*0.2f);

           if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               mat = m_object->RetWorldMatrix(0);
               pos = D3DVECTOR(0.0f, 5.0f, -14.5f);
               pos = Transform(*mat, pos);

               for ( i=0 ; i<4 ; i++ )
               {
                   speed.x = (Rand()-0.5f)*20.0f;
                   speed.z = (Rand()-0.5f)*20.0f;
                   speed.y = 5.0f+Rand()*10.0f;
                   dim.x = 0.8f;
                   dim.y = 0.8f;
                   m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 1.0f, 40.0f);
               }

               pos.x = (Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Rand()-0.5f)*2.0f;
               pos = Transform(*mat, pos);
               speed.x = (Rand()-0.5f)*1.0f;
               speed.z = (Rand()-0.5f)*1.0f;
               speed.y = 1.0f+Rand()*2.0f;
               dim.x = Rand()*1.2f+2.4f;
               dim.y = dim.x;
               duration = 1.0f+Rand()*2.0f;
               m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, duration);

               pos.x = (Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Rand()-0.5f)*2.0f;
               pos = Transform(*mat, pos);
               speed.x = 0.0f;
               speed.z = 0.0f;
               speed.y = 4.0f+Rand()*5.0f;
               dim.x = Rand()*1.8f+3.3f;
               dim.y = dim.x;
               duration = 2.0f+Rand()*4.0f;
               m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, duration);
           }

           if ( progress >= 1.0f )
           {
               mat = m_object->RetWorldMatrix(3);
               for ( i=0 ; i<15 ; i++ )
               {
                   pos.x = (Rand()-0.5f)*5.0f;
                   pos.z = (Rand()-0.5f)*5.0f;
                   pos.y = 1.0f;
                   pos = Transform(*mat, pos);
                   speed.x = (Rand()-0.5f)*50.0f;
                   speed.z = (Rand()-0.5f)*50.0f;
                   speed.y = Rand()*15.0f;
                   dim.x = 2.0f;
                   dim.y = dim.x;
                   duration = Rand()*3.0f+2.0f;
                   mass = Rand()*10.0f+15.0f;
                   m_particule->CreateTrack(pos, speed, dim, PARTITRACK2,
                                            duration, mass, Rand()+0.7f, 2.0f);
               }

               m_sound->Play(SOUND_BOUMm, m_engine->RetEyePt(), 1.0f, 0.7f);

               speed.x = (Rand()-0.5f)*20.0f;
               speed.z = (Rand()-0.5f)*20.0f;
               speed.y = 10.0f+Rand()*10.0f;
//?                m_object->DetachPart(3, speed);  // détache le piston
               m_object->ExploPiece(3);

               speed.x = (Rand()-0.5f)*20.0f;
               speed.z = (Rand()-0.5f)*20.0f;
               speed.y = 10.0f+Rand()*10.0f;
               m_object->DetachPart(2, speed);  // détache le piston
//?                m_object->ExploPiece(2);

               speed.x = (Rand()-0.5f)*20.0f;
               speed.z = (Rand()-0.5f)*20.0f;
               speed.y = 10.0f+Rand()*10.0f;
               m_object->DetachPart(1, speed);  // détache le piston
//?                m_object->ExploPiece(1);

               BotAction(m_bot, MB_GOHOME, 1.0f);  // blupi debout
               BotAction(m_bot, MB_HAPPY, 1.0f);  // blupi heureux
               m_object->SetLock(TRUE);  // plus détecté par la suite

               m_progress = 0.0f;
               m_speed    = 1.0f/5.0f;
               m_phase    = 3;
               progress   = 0.0f;
           }
       }

       if ( m_phase == 3 )  // smoke ?
       {
           mat = m_object->RetWorldMatrix(0);
           skip = Norm(progress*6.0f);
           angle = m_object->RetAngleY(0)+PI*0.5f;
           angle += PI*2.25f*skip;
           m_bot->SetAngleY(0, angle);
           if ( skip < 0.5f )
           {
               src = Transform(*mat, D3DVECTOR(0.0f, 5.6f, -0.5f));
               dst = Transform(*mat, D3DVECTOR(4.0f, 7.0f, -3.5f));
               skip = powf(skip, 0.5f);
           }
           else
           {
               src = Transform(*mat, D3DVECTOR(4.0f, 7.0f, -2.5f));
               dst = Transform(*mat, D3DVECTOR(8.0f, 2.0f, -7.0f));
               skip = powf(skip, 2.0f);
           }
           pos = src+(dst-src)*skip;
           pos.y += m_bot->RetCharacter()->height;
           m_bot->SetPosition(0, pos);  // déplace blupi
           if ( skip == 1.0f )
           {
               BotAction(m_bot, MB_WAIT, 1.0f);  // blupi attend
           }

           if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               mat = m_object->RetWorldMatrix(0);

               pos.x = (Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Rand()-0.5f)*2.0f;
               pos = Transform(*mat, pos);
               speed.x = (Rand()-0.5f)*1.0f;
               speed.z = (Rand()-0.5f)*1.0f;
               speed.y = 1.0f+Rand()*2.0f;
               dim.x = Rand()*1.2f+2.4f;
               dim.y = dim.x;
               duration = 1.0f+Rand()*2.0f;
               m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, duration);

               pos.x = (Rand()-0.5f)*4.0f;
               pos.y = 4.0f;
               pos.z = -14.5f+(Rand()-0.5f)*2.0f;
               pos = Transform(*mat, pos);
               speed.x = 0.0f;
               speed.z = 0.0f;
               speed.y = 4.0f+Rand()*5.0f;
               dim.x = Rand()*1.8f+3.3f;
               dim.y = dim.x;
               duration = 2.0f+Rand()*4.0f;
               m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, duration);
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

   return TRUE;
}

// Calcule la position et l'orientation du robot au bout du bras
// selon l'angle (50..-50), pour le générateur.

void CAutoAlien::ArmPosition(D3DVECTOR &pos, float &dir, float angle, float lin)
{
   D3DMATRIX*  mat;
   FPOINT      rot;

   rot = RotatePoint(PI*1.5f-angle, 51.0f);
   rot.x += 25.0f+lin;
   rot.y += 15.0f;

   mat = m_object->RetWorldMatrix(0);
   pos = Transform(*mat, D3DVECTOR(rot.x, 10.0f, rot.y));

   if ( m_bot != 0 )
   {
       pos.y += m_bot->RetCharacter()->height*m_bot->RetZoomY(0);
   }

   dir = angle + m_object->RetAngleY(0) + PI/2.0f;
}

// Démarre une action pour le robot.

void CAutoAlien::BotAction(CObject *bot, int action, float delay)
{
   CMotion*    motion;

   if ( bot == 0 )  return;

   motion = bot->RetMotion();
   if ( motion == 0 )  return;

   motion->SetAction(action, delay);
}

// Cherche l'objet marteau à droite pour le synchroniser.

void CAutoAlien::SynchroHammer(int action)
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos;
   CObject*    hammer;
   CAuto*      automat;

   mat = m_object->RetWorldMatrix(0);
   pos = Transform(*mat, D3DVECTOR(80.0f, 0.0f, 0.0f));
   hammer = SearchObject(OBJECT_ALIEN5, pos, 40.0f);
   if ( hammer == 0 )  return;

   automat = hammer->RetAuto();
   if ( automat == 0 )  return;

   automat->Start(action);
}

// Cherche un objet proche.

CObject* CAutoAlien::SearchObject(ObjectType type, D3DVECTOR center, float radius)
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

// Met à jour les feux de stop.

void CAutoAlien::FireStopUpdate()
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   BOOL        bOn;
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
               m_particule->DeleteParticule(m_partiStop[i]);
               m_partiStop[i] = -1;
           }
       }
       return;
   }

   mat = m_object->RetWorldMatrix(0);

   speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
   dim.x = 3.0f;
   dim.y = dim.x;

   for ( i=0 ; i<12 ; i++ )
   {
       bOn = FALSE;

       if ( Mod(m_time+0.1f*i, 0.6f) >= 0.4f )  bOn = TRUE;

       if ( bOn )
       {
           if ( m_partiStop[i] == -1 )
           {
               pos.x = listpos[i*3+0];
               pos.y = listpos[i*3+1];
               pos.z = listpos[i*3+2];
               pos = Transform(*mat, pos);
               m_partiStop[i] = m_particule->CreateParticule(pos, speed,
                                                             dim, PARTISELY,
                                                             1.0f, 0.0f);
           }
       }
       else
       {
           if ( m_partiStop[i] != -1 )
           {
               m_particule->DeleteParticule(m_partiStop[i]);
               m_partiStop[i] = -1;
           }
       }
   }
}


// Stoppe l'automate.

BOOL CAutoAlien::Abort()
{
   return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoAlien::RetError()
{
   return ERR_OK;
}


// Met à jour le mapping de la texture des chenilles.

void CAutoAlien::UpdateTrackMapping(float progress)
{
   D3DMATERIAL7    mat;
   float           limit[2];
   int             rank;

   if ( Length(m_object->RetPosition(0), m_engine->RetLookatPt()) > 500.0f )  return;

   ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
   mat.diffuse.r = 1.0f;
   mat.diffuse.g = 1.0f;
   mat.diffuse.b = 1.0f;  // blanc
   mat.ambient.r = 0.5f;
   mat.ambient.g = 0.5f;
   mat.ambient.b = 0.5f;

   rank = m_object->RetObjectRank(0);

   limit[0] = 0.0f;
   limit[1] = 1000000.0f;

   m_engine->TrackTextureMapping(rank, mat, D3DSTATEPART1, "alien1.tga", "",
                                 limit[0], limit[1], D3DMAPPINGZ,
                                 progress, 3.0f, 8.0f, 128.0f, 256.0f);
}

