// autohome.cpp

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
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "motion.h"
#include "motionvehicle.h"
#include "motionbot.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autohome.h"





// Constructeur de l'objet.

CAutoHome::CAutoHome(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   int     i;

   CAuto::CAuto(iMan, object);

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
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoHome::DeleteObject(BOOL bAll)
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
   m_type = m_object->RetType();
}


// Démarre l'objet.

void CAutoHome::Start(int param)
{
   ObjectType  type;
   float       angle;
   int         i;

   if ( param == AHOP_BREAKDOWN )
   {
       i = m_sound->Play(SOUND_NUCLEAR, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
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
       m_bBreakFinish = FALSE;
       for ( i=0 ; i<4 ; i++ )
       {
           m_breakPhase[i] = 0;  // wait
           m_breakTimeWait[i] = Rand()*0.5f;
           m_breakTimeBzzz[i] = m_breakTimeWait[i]+Rand()*(0.9f-m_breakTimeWait[i]);
       }

       m_object->SetLock(TRUE);  // plus détecté par la suite
       return;
   }

   type = OBJECT_NULL;
   if ( m_type == OBJECT_HOME1 )  type = OBJECT_BOT1;
   if ( m_type == OBJECT_HOME2 )  type = OBJECT_BOT2;
   if ( m_type == OBJECT_HOME3 )  type = OBJECT_BOT3;
   if ( m_type == OBJECT_HOME4 )  type = OBJECT_BOT4;
   if ( m_type == OBJECT_HOME5 )  type = OBJECT_BOT5;
   if ( type == OBJECT_NULL )  return;

   m_goalPos = m_object->RetPosition(0);
   if ( m_type == OBJECT_HOME1 )  m_goalPos.y += 3.0f;
   if ( m_type == OBJECT_HOME2 )  m_goalPos.y += 6.0f;
   if ( m_type == OBJECT_HOME3 )  m_goalPos.y += 0.0f;
   if ( m_type == OBJECT_HOME4 )  m_goalPos.y += 3.0f;
   if ( m_type == OBJECT_HOME5 )  m_goalPos.y += 3.0f;

   angle = m_object->RetAngleY(0);

   m_bot = new CObject(m_iMan);
   if ( !m_bot->CreateBot(m_goalPos, angle, 1.0f, type, TRUE) )
   {
       delete m_bot;
       m_bot = 0;
   }
   else
   {
       m_goalPos.y += m_bot->RetCharacter()->height;
       m_bot->SetPosition(0, m_goalPos);

       m_phase    = AHOP_LIVE1;
       m_progress = 0.0f;
       m_speed    = 1.0f/0.1f;
   }
}


// Gestion d'un événement.

BOOL CAutoHome::EventProcess(const Event &event)
{
   CObject*    mark;
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed, eye;
   FPOINT      dim;
   float       angle, duration, mass;
   int         r, i, j;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;

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
               m_vehiclePos = m_vehicle->RetPosition(0);
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
           return TRUE;
       }
       pos = m_vehicle->RetPosition(0);
       if ( Length(pos, m_vehiclePos) > 0.1f )
       {
           m_phase    = AHOP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.2f;
           return TRUE;
       }

       if ( m_progress >= 1.0f )
       {
           m_vehicle->SetLock(TRUE);
           StartVehicleAction(MV_OPEN);
           m_cameraType = m_camera->RetType();
           m_camera->SetObject(m_object);
           m_camera->SetType(CAMERA_BACK);
           m_main->SetStopwatch(FALSE);  // stoppe le chrono

           m_phase    = AHOP_OPEN;
           m_progress = 0.0f;
           m_speed    = 1.0f/2.0f;
       }
   }

   if ( m_phase == AHOP_OPEN )
   {
       if ( m_progress >= 1.0f )
       {
           m_bot = m_vehicle->RetFret();
           m_bot->SetTruck(0);
           m_vehicle->SetFret(0);
           m_startPos = RetVehiclePoint(m_vehicle);
           m_startPos.y += m_bot->RetCharacter()->height;
           m_goalPos = m_object->RetPosition(0);
           m_goalPos.y += m_bot->RetCharacter()->height;
           if ( m_type == OBJECT_HOME1 )  m_goalPos.y += 3.0f;
           if ( m_type == OBJECT_HOME2 )  m_goalPos.y += 6.0f;
           if ( m_type == OBJECT_HOME3 )  m_goalPos.y += 0.0f;
           if ( m_type == OBJECT_HOME4 )  m_goalPos.y += 3.0f;
           if ( m_type == OBJECT_HOME5 )  m_goalPos.y += 3.0f;
           m_bot->SetPosition(0, m_startPos);
           m_bot->SetAngle(0, D3DVECTOR(0.0f, m_vehicle->RetAngleY(0), 0.0f));
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
           m_vehicle->SetLock(FALSE);
           m_camera->SetObject(m_vehicle);
           m_camera->SetType(m_cameraType);
           m_main->SetStopwatch(TRUE);  // redémarre le chrono
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
               if ( r == 0 )  {m_dir = D3DVECTOR( 4.0f, 6.5f,  0.0f); m_goalAngle=PI*1.0f;}
               if ( r == 1 )  {m_dir = D3DVECTOR(-4.0f, 6.5f,  0.0f); m_goalAngle=PI*0.0f;}
               if ( r == 2 )  {m_dir = D3DVECTOR( 0.0f, 6.5f,  4.0f); m_goalAngle=PI*0.5f;}
               if ( r == 3 )  {m_dir = D3DVECTOR( 0.0f, 6.5f, -4.0f); m_goalAngle=PI*1.5f;}
               m_startAngle = NormAngle(m_bot->RetAngleY(0));

               m_phase    = AHOP_LIVE2;
               m_progress = 0.0f;
               m_speed    = 1.0f/(Abs(m_startAngle-m_goalAngle)*0.3f+0.1f);
           }
       }
       if ( m_type == OBJECT_HOME2 )  // choix direction et attente
       {
           if ( m_progress >= 1.0f )
           {
               pos = m_object->RetPosition(0);
               mark = SearchObject(OBJECT_MARK, pos, 400.0f);
               if ( mark == 0 )
               {
                   m_goalAngle = Rand()*PI*2.0f;
               }
               else
               {
                   m_goalPos = mark->RetPosition(0);
                   m_goalAngle = RotateAngle(m_goalPos.x-pos.x, pos.z-m_goalPos.z);
               }
               m_startAngle = NormAngle(m_bot->RetAngleY(0));

               m_phase    = AHOP_LIVE2;
               m_progress = 0.0f;
               m_speed    = 1.0f/(Abs(m_startAngle-m_goalAngle)*0.3f+0.1f);
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
           angle = m_startAngle+(m_goalAngle-m_startAngle)*Norm(m_progress);
           m_bot->SetAngleY(0, angle);

           if ( m_progress >= 1.0f )
           {
               m_phase    = AHOP_LIVE3;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.6f;
           }
       }
       if ( m_type == OBJECT_HOME2 )  // rotation
       {
           angle = m_startAngle+(m_goalAngle-m_startAngle)*Norm(m_progress);
           m_bot->SetAngleY(0, angle);

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
           mat = m_object->RetWorldMatrix(0);
           pos = Transform(*mat, m_dir);
           pos = m_goalPos+(pos-m_goalPos)*Norm(m_progress);
           m_bot->SetPosition(0, pos);

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
           m_object->SetZoomX(1, 0.95f+Rand()*0.1f);  // hp vibrent
           m_object->SetZoomX(2, 0.95f+Rand()*0.1f);
           m_object->SetZoomX(3, 0.95f+Rand()*0.1f);
           m_object->SetZoomX(4, 0.95f+Rand()*0.1f);

           if ( m_progress >= 1.0f )
           {
               m_object->SetZoomX(1, 1.0f);
               m_object->SetZoomX(2, 1.0f);
               m_object->SetZoomX(3, 1.0f);
               m_object->SetZoomX(4, 1.0f);
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
           mat = m_object->RetWorldMatrix(0);
           pos = Transform(*mat, m_dir);
           pos = m_goalPos+(pos-m_goalPos)*Norm(1.0f-m_progress);
           m_bot->SetPosition(0, pos);

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
           mat = m_object->RetWorldMatrix(0);

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
                           m_sound->Play(SOUND_EXPLOlp, m_object->RetPosition(0));
                           for ( i=0 ; i<10 ; i++ )
                           {
                               if ( j == 0 )  pos = D3DVECTOR( 8.5f, 14.0f,  8.5f);
                               if ( j == 1 )  pos = D3DVECTOR(-8.5f, 14.0f,  8.5f);
                               if ( j == 2 )  pos = D3DVECTOR( 8.5f, 14.0f, -8.5f);
                               if ( j == 3 )  pos = D3DVECTOR(-8.5f, 14.0f, -8.5f);
                               pos = Transform(*mat, pos);
                               pos.x += (Rand()-0.5f)*2.0f;
                               pos.y += (Rand()-0.5f)*4.0f;
                               pos.z += (Rand()-0.5f)*2.0f;
                               speed.x = (Rand()-0.5f)*15.0f;
                               speed.z = (Rand()-0.5f)*15.0f;
                               speed.y = Rand()*20.0f;
                               dim.x = 1.0f;
                               dim.y = dim.x;
                               duration = Rand()*3.0f+2.0f;
                               mass = Rand()*10.0f+15.0f;
                               m_particule->CreateTrack(pos, speed, dim, PARTITRACK1,
                                                        duration, mass, Rand()+0.7f, 1.0f);

                               speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                               dim.x = Rand()*5.0f+1.0f;
                               dim.y = dim.x;
                               duration = Rand()*6.0f+2.0f;
                               m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, duration);
                           }
                           m_breakPhase[j] = 2;
                       }
                   }
               }

               if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
               {
                   m_lastParticule = m_time;

                   for ( j=0 ; j<4 ; j++ )
                   {
                       if ( m_breakPhase[j] != 1 )  continue;

                       for ( i=0 ; i<5 ; i++ )
                       {
                           if ( j == 0 )  pos = D3DVECTOR( 8.5f, 14.0f,  8.5f);
                           if ( j == 1 )  pos = D3DVECTOR(-8.5f, 14.0f,  8.5f);
                           if ( j == 2 )  pos = D3DVECTOR( 8.5f, 14.0f, -8.5f);
                           if ( j == 3 )  pos = D3DVECTOR(-8.5f, 14.0f, -8.5f);
                           pos = Transform(*mat, pos);
                           pos.x += (Rand()-0.5f)*2.0f;
                           pos.y += (Rand()-0.5f)*4.0f;
                           pos.z += (Rand()-0.5f)*2.0f;
                           speed.x = (Rand()-0.5f)*20.0f;
                           speed.z = (Rand()-0.5f)*20.0f;
                           speed.y = 5.0f+Rand()*10.0f;
                           dim.x = 0.8f;
                           dim.y = 0.8f;
                           m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 1.0f, 40.0f);
                       }
                   }
               }
           }

           if ( m_progress >= 1.0f && !m_bBreakFinish )
           {
               m_bBreakFinish = TRUE;
               m_bot->SetExplo(TRUE);  // comme si plus là !
               HappyBlupi();  // les blupis sont heureux
           }
       }
   }

   if ( m_channelSound != -1 )
   {
       eye = m_engine->RetEyePt();
       pos = m_object->RetPosition(0);
       pos = pos+(eye-pos)*0.8f;
       m_sound->Position(m_channelSound, pos);

       if ( m_sound->RetAmplitude(m_channelSound) == 0.0f )
       {
           m_channelSound = -1;
       }
   }

   return TRUE;
}

// Stoppe l'automate.

BOOL CAutoHome::Abort()
{
   return TRUE;
}


// Met à jour les feux de stop.

void CAutoHome::FireStopUpdate()
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   BOOL        bOn;
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

   for ( i=0 ; i<6 ; i++ )
   {
       bOn = FALSE;

       if ( bOn )
       {
           if ( m_partiStop[i] == -1 )
           {
               pos.x = listpos[i*3+0];
               pos.y = listpos[i*3+1];
               pos.z = listpos[i*3+2];
               pos = Transform(*mat, pos);
               m_partiStop[i] = m_particule->CreateParticule(pos, speed,
                                                             dim, PARTISELR,
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


// Retourne une erreur liée à l'état de l'automate.

Error CAutoHome::RetError()
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
       i = m_sound->Play(SOUND_FLY, m_goalPos, 0.0f, 1.0f, TRUE);
       m_sound->AddEnvelope(i, 1.0f, 1.5f, 1.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 1.0f, 1.5f, 2.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.5f, SOPER_STOP);
   }

   if ( m_type == OBJECT_HOME3 )
   {
       m_sound->Play(SOUND_POWEROFF, m_startPos, 1.0f);
       m_bZoomIn = FALSE;
   }
}

// Déplace le robot du véhicule à la maison.

void CAutoHome::MoveBot(float progress, float rTime)
{
   D3DVECTOR   src, dst, pos, speed;
   FPOINT      dim;
   float       duration;
   int         i;

   progress = Norm(progress);

   if ( m_type == OBJECT_HOME1 ||
        m_type == OBJECT_HOME2 )
   {
       pos = m_startPos+(m_goalPos-m_startPos)*progress;
       pos.y += sinf(progress*PI)*20.0f;
       m_bot->SetPosition(0, pos);

       pos.y -= m_bot->RetCharacter()->height;

       if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
       {
           m_lastParticule = m_time;

           for ( i=0 ; i<4 ; i++ )
           {
               speed.x = (Rand()-0.5f)*2.0f;
               speed.z = (Rand()-0.5f)*2.0f;
               speed.y = -Rand()*5.0f;
               dim.x = Rand()*1.0f+1.0f;
               dim.y = dim.x;
               duration = Rand()*1.0f+1.0f;
               m_particule->CreateParticule(pos, speed, dim, PARTIGAS, duration);
           }
       }
   }

   if ( m_type == OBJECT_HOME3 )
   {
       if ( progress < 0.5f )
       {
           progress = progress/0.5f;

           if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<4 ; i++ )
               {
                   pos = m_startPos;
                   speed = pos;
                   pos.x += (Rand()-0.5f)*30.0f;
                   pos.y += (Rand()-0.5f)*30.0f;
                   pos.z += (Rand()-0.5f)*30.0f;
                   speed = (speed-pos)*1.0f;
                   dim.x = 0.6f;
                   dim.y = dim.x;
                   duration = Rand()*0.5f+0.5f;
                   m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
                                            duration, 0.0f,
                                            duration*0.9f, 0.7f);
               }
           }

           m_bot->SetZoom(0, 1.0f-progress);
           m_bot->SetAngleY(0, m_bot->RetAngleY(0)+rTime*20.0f);
       }
       else
       {
           progress = (progress-0.5f)/0.5f;

           if ( !m_bZoomIn )
           {
               m_bZoomIn = TRUE;
               m_sound->Play(SOUND_POWERON, m_goalPos, 1.0f);
           }

           if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;

               for ( i=0 ; i<4 ; i++ )
               {
                   pos = m_goalPos;
                   speed.x = (Rand()-0.5f)*30.0f;
                   speed.z = (Rand()-0.5f)*30.0f;
                   speed.y = (Rand()-0.5f)*30.0f;
                   dim.x = 0.6f;
                   dim.y = dim.x;
                   duration = Rand()*0.5f+0.5f;
                   m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
                                            duration, 0.0f,
                                            duration*0.9f, 0.7f);
               }
           }

           m_bot->SetPosition(0, m_goalPos);
           m_bot->SetZoom(0, progress);
           m_bot->SetAngleY(0, m_bot->RetAngleY(0)+rTime*20.0f);
       }
   }
}

// Génère les particules suite à l'atterrissage.

void CAutoHome::EndingEffect()
{
   D3DVECTOR   pos, ppos, speed;
   FPOINT      dim;
   float       len;
   int         i, max;

   StartBotAction(MB_HOME1);

   if ( m_type == OBJECT_HOME1 ||
        m_type == OBJECT_HOME2 )
   {
       pos = m_bot->RetPosition(0);
       max = (int)(10.0f*m_engine->RetParticuleDensity());

       for ( i=0 ; i<max ; i++ )
       {
           ppos.x = pos.x + (Rand()-0.5f)*5.0f;
           ppos.z = pos.z + (Rand()-0.5f)*5.0f;
           ppos.y = pos.y + Rand()*4.0f;
           len = 1.0f-(Length(ppos, pos)/(15.0f+5.0f));
           if ( len <= 0.0f )  continue;
           speed.x = (ppos.x-pos.x)*0.1f;
           speed.z = (ppos.z-pos.z)*0.1f;
           speed.y = -2.0f;
           dim.x = 2.0f+5.0f*len;
           dim.y = dim.x;
           m_particule->CreateParticule(ppos, speed, dim, PARTICRASH, 2.0f);
       }

       m_sound->Play(SOUND_BOUMv, m_goalPos, 1.0f);
   }
}

// Cherche l'objet véhicule.

CObject* CAutoHome::SearchVehicle()
{
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType, fType;
   float       dist;
   int         i;

   cPos = m_object->RetPosition(0);

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetGhost() )  continue;

       oType = pObj->RetType();
       if ( oType != OBJECT_CAR )  continue;

       if ( pObj->RetFret() == 0 )  continue;
       fType = pObj->RetFret()->RetType();

       if ( (m_type != OBJECT_HOME1 || fType != OBJECT_BOT1) &&
            (m_type != OBJECT_HOME2 || fType != OBJECT_BOT2) &&
            (m_type != OBJECT_HOME3 || fType != OBJECT_BOT3) &&
            (m_type != OBJECT_HOME4 || fType != OBJECT_BOT4) &&
            (m_type != OBJECT_HOME5 || fType != OBJECT_BOT5) )  continue;

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 30.0f )  return pObj;
   }

   return 0;
}

// Cherche un objet proche.

CObject* CAutoHome::SearchObject(ObjectType type, D3DVECTOR center, float radius)
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
       if ( pObj->RetLock() )  continue;  // déjà stoppé ?

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

// Vérifie si l'objet sélectionné est proche.

BOOL CAutoHome::ProxiSelect(float dist)
{
#if 0
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   int         i;

   cPos = m_object->RetPosition(0);

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( !pObj->RetSelect() )  continue;

       oPos = pObj->RetPosition(0);
       if ( Length(oPos, cPos) <= dist )  return TRUE;
   }

   return FALSE;
#else
   D3DVECTOR   cPos, oPos;

   cPos = m_object->RetPosition(0);
   oPos = m_engine->RetLookatPt();
   if ( Length(oPos, cPos) <= dist )  return TRUE;
   return FALSE;
#endif
}

// Cherche le point où mettre du fret sur un véhicule.

D3DVECTOR CAutoHome::RetVehiclePoint(CObject *pObj)
{
   Character*  character;
   D3DMATRIX*  mat;
   D3DVECTOR   pos;

   character = pObj->RetCharacter();
   mat = pObj->RetWorldMatrix(0);
   pos = Transform(*mat, character->posFret);

   return pos;
}

// Démarre une action pour le véhicule.

void CAutoHome::StartVehicleAction(int action)
{
   CMotion*    motion;
   float       delay;

   motion = m_vehicle->RetMotion();
   if ( motion == 0 )  return;

   delay = 1.0f;
   if ( action == MV_CLOSE )  delay = 2.0f;
   motion->SetAction(action, delay);
}

// Démarre une action "soulevé" pour l'objet pris par la grue.

void CAutoHome::StartBotAction(int action, float delay)
{
   CMotion*    motion;

   motion = m_bot->RetMotion();
   if ( motion == 0 )  return;

   motion->SetAction(action, delay);
}

// Rend heureux tous les blupi avoisinnants.

void CAutoHome::HappyBlupi()
{
   CObject*    pObj;
   CMotion*    motion;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->RetPosition(0);

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetGhost() )  continue;

       oType = pObj->RetType();
       if ( oType != OBJECT_BOT3 )  continue;

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 200.0f )
       {
           motion = pObj->RetMotion();
           if ( motion != 0 )
           {
               motion->SetAction(MB_WAIT, 1.0f);
               motion->SetAction(MB_HAPPY, 1.0f);
           }
       }
   }
}

