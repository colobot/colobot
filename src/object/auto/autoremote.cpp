// autoremote.cpp

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
#include "restext.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "physics.h"
#include "motion.h"
#include "motionhuman.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "displaytext.h"
#include "auto.h"
#include "autoremote.h"




// Constructeur de l'objet.

CAutoRemote::CAutoRemote(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   int     i;

   CAuto::CAuto(iMan, object);

   for ( i=0 ; i<6 ; i++ )
   {
       m_partiStop[i] = -1;
   }
   m_lastParticule = 0.0f;
   m_lastSound = -10.0f;
   m_bDisplayText = FALSE;
   m_bStartDemoWheel = FALSE;
   m_bForceSlow = FALSE;
   m_vehicle = 0;
   m_human = 0;
   m_param = 0;

   Init();
}

// Destructeur de l'objet.

CAutoRemote::~CAutoRemote()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoRemote::DeleteObject(BOOL bAll)
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

   m_main->SetStopwatch(FALSE);  // stoppe le chrono
   m_main->SetMovieAuto(this);
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

BOOL CAutoRemote::EventProcess(const Event &event)
{
   D3DMATRIX*  mat;
   CMotion*    motion;
   Sound       sound;
   int         i;
   char        text[100];
   float       angle;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;
   if ( m_param == 0 )  return TRUE;

   m_progress += event.rTime*m_speed;

   if ( m_phase == ARMP_WAIT )
   {
       if ( !m_bDisplayText )
       {
           m_bDisplayText = TRUE;
           GetResource(RES_TEXT, RT_START_REMOTE, text);
           m_displayText->DisplayText(text, 8.0f);
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
//?                human->SetLock(TRUE);
               m_camera->SetType(CAMERA_SCRIPT);

               m_eyeStart = m_engine->RetEyePt();
               m_lookatStart = m_engine->RetLookatPt();

               // Derrière l'écran.
               mat = m_object->RetWorldMatrix(0);
//?                m_eyeGoal = Transform(*mat, D3DVECTOR(0.0f, 8.0f, 0.0f));
//?                m_lookatGoal = Transform(*mat, D3DVECTOR(2.0f, 7.0f, 0.0f));
               m_eyeGoal = Transform(*mat, D3DVECTOR(2.0f, 7.0f, 5.0f));
               m_lookatGoal = Transform(*mat, D3DVECTOR(4.0f, 6.0f, 0.0f));

               i = m_sound->Play(SOUND_RESEARCH, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
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
       motion = m_human->RetMotion();
       if ( motion != 0 )
       {
           angle = 0.0f;
           if ( motion->RetAction() == MHS_DRIVE1 )
           {
               angle = motion->RetActionProgress()*0.5f;
           }
           if ( motion->RetAction() == MHS_DRIVE2 )
           {
               angle = -motion->RetActionProgress()*0.5f;
           }
           angle = Smooth(m_object->RetAngleY(1), angle, event.rTime*2.0f);
           m_object->SetAngleY(1, angle);  // tourne le volant
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
           mat = m_object->RetWorldMatrix(0);
           m_eyeGoal = Transform(*mat, D3DVECTOR(-8.0f, 6.0f, 8.0f));
           m_lookatGoal = Transform(*mat, D3DVECTOR(10.0f, 6.0f, 8.0f));

           EmitParticule(TRUE);
           m_sound->Play(SOUND_RADAR, m_object->RetPosition(0), 0.8f, 0.5f);

           m_phase    = ARMP_CAMERA2;
           m_progress = 0.0f;
           m_speed    = 1.0f/2.0f;
       }
   }

   if ( m_phase == ARMP_CAMERA2 )
   {
       UpdateCamera(m_progress*1.2f);
       EmitParticule(FALSE);
       DemoWheel(FALSE);

       if ( m_progress >= 1.0f )
       {
           m_eyeStart = m_eyeGoal;
           m_lookatStart = m_lookatGoal;

           // A gauche de la voiture sur le récepteur.
           mat = m_vehicle->RetWorldMatrix(0);
//?            m_eyeGoal = Transform(*mat, D3DVECTOR(0.0f, 6.0f, 6.0f));
//?            m_lookatGoal = Transform(*mat, D3DVECTOR(0.0f, 4.0f, 0.0f));
//?            m_eyeGoal = Transform(*mat, D3DVECTOR(0.0f, 7.0f, 9.0f));
//?            m_lookatGoal = Transform(*mat, D3DVECTOR(0.0f, 4.0f, 0.0f));
           m_eyeGoal = Transform(*mat, D3DVECTOR(3.0f, 1.5f, 10.0f));
           m_lookatGoal = Transform(*mat, D3DVECTOR(3.0f, 2.5f, 0.0f));

           m_sound->Play(SOUND_RADAR, m_object->RetPosition(0), 0.8f, 0.5f);

           i = m_sound->Play(SOUND_RESEARCH, m_vehicle->RetPosition(0), 0.0f, 1.0f, TRUE);
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
       EmitParticule(FALSE);
       DemoWheel(FALSE);

       if ( m_progress >= 1.0f )
       {
           m_eyeStart = m_eyeGoal;
           m_lookatStart = m_lookatGoal;

           // Derrière la voiture.
           mat = m_vehicle->RetWorldMatrix(0);
           m_eyeGoal = Transform(*mat, D3DVECTOR(-20.0f, 6.0f, 5.0f));
           m_lookatGoal = Transform(*mat, D3DVECTOR(0.0f, 4.0f, 0.0f));

           m_phase    = ARMP_CAMERA5;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.0f;
       }
   }

   if ( m_phase == ARMP_CAMERA5 )
   {
       UpdateCamera(m_progress*1.0f);
       EmitParticule(FALSE);

       if ( m_progress >= 1.0f )
       {
           DemoWheel(TRUE);  // remet au neutre
           m_main->SelectObject(m_vehicle);
           
           GetResource(RES_TEXT, RT_START_GO, text);
           m_displayText->DisplayText(text, 3.0f, 20.0f, TT_START);
           sound = SOUND_STARTGO;
           if ( !m_sound->RetComments() )  sound = SOUND_MESSAGE;
           m_sound->Play(sound, m_engine->RetLookatPt(), 1.0f, 1.0f);
           
           m_main->SetStopwatch(TRUE);  // démarre le chrono
           m_main->SetMovieAuto(0);

           m_phase    = ARMP_TERM;
           m_progress = 0.0f;
           m_speed    = 1.0f/100.0f;
       }
   }

   if ( m_phase == ARMP_TERM )
   {
       EmitParticule(FALSE);
   }

   return TRUE;
}

// Stoppe l'automate.

BOOL CAutoRemote::Abort()
{
   return TRUE;
}


// Met à jour la caméra.

void CAutoRemote::UpdateCamera(float progress)
{
   progress = Norm(progress);  // 0..1
   m_camera->SetScriptEye(m_eyeStart+(m_eyeGoal-m_eyeStart)*progress);
   m_camera->SetScriptLookat(m_lookatStart+(m_lookatGoal-m_lookatStart)*progress);
   m_camera->FixCamera();
   m_engine->SetFocus(1.0f);
}

// Emission de particule de l'émetteur vers la voiture.

void CAutoRemote::EmitParticule(BOOL bFirst)
{
   D3DMATRIX*  mat;
   Character*  character;
   D3DVECTOR   src, dst, pos, speed;
   FPOINT      dim;
   float       duration;
   int         i;

   if ( m_vehicle == 0 )  return;

   mat = m_object->RetWorldMatrix(0);
   src = Transform(*mat, D3DVECTOR(4.0f, 8.9f, 4.0f));

   character = m_vehicle->RetCharacter();
   mat = m_vehicle->RetWorldMatrix(0);
   dst = Transform(*mat, character->antenna);

   if ( bFirst )
   {
       speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
       dim.x = 10.0f;
       dim.y = dim.x;
       m_particule->CreateParticule(src, speed, dim, PARTISPHERE4, 1.5f, 0.0f);

       m_particule->CreateRay(src, dst, PARTIRAY3, FPOINT(1.5f, 1.5f), 3.5f);

       m_distance = Length(src, dst);
   }

   if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
   {
       m_lastParticule = m_time;

       for ( i=0 ; i<4 ; i++ )
       {
           pos = src;
           speed.x = (Rand()-0.5f)*30.0f;
           speed.z = (Rand()-0.5f)*30.0f;
           speed.y = (Rand()-0.5f)*30.0f;
           dim.x = 0.4f;
           dim.y = dim.x;
           duration = Rand()*0.5f+0.5f;
           m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
                                    duration, 0.0f,
                                    duration*0.9f, 0.7f);
       }

       if ( Length(src, dst) < m_distance+10.0f )  // pas trop loin ?
       {
           pos = dst;
           speed.x = (Rand()-0.5f)*0.5f;
           speed.z = (Rand()-0.5f)*0.5f;
           speed.y = Rand()*2.0f;
           dim.x = 0.2f;
           dim.y = dim.x;
           duration = Rand()*1.0f+0.5f;
           m_particule->CreateParticule(pos, speed, dim, PARTIGLINTr, duration);
       }
   }
}

// Petite démo du volant, même si le véhicule est inactif (SetLock).

void CAutoRemote::DemoWheel(BOOL bStop)
{
   CPhysics*   physics;
   Character*  character;
   D3DMATRIX*  mat;
   D3DVECTOR   src, dst, pos, speed;
   float       time;
   int         i;

   if ( m_vehicle == 0 )  return;

   physics = m_vehicle->RetPhysics();
   if ( physics == 0 )  return;

   mat = m_object->RetWorldMatrix(0);
   src = Transform(*mat, D3DVECTOR(4.0f, 8.9f, 4.0f));

   character = m_vehicle->RetCharacter();
   mat = m_vehicle->RetWorldMatrix(0);
   dst = Transform(*mat, character->antenna);

   if ( !m_bStartDemoWheel )
   {
       m_startTime = m_time;
       m_bStartDemoWheel = TRUE;

       m_particule->CreateRay(dst, src, PARTIRAY3, FPOINT(1.5f, 0.5f), 11.0f);

       i = m_sound->Play(SOUND_MANIP, m_vehicle->RetPosition(0), 0.0f, 0.5f, TRUE);
       m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.2f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.3f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 0.5f, 0.5f, SOPER_STOP);
   }

   if ( bStop )
   {
       physics->ForceMotorSpeedZ(0.0f);
       physics->SetForceSlow(FALSE);
       return;
   }

   time = (m_time-m_startTime)*2.0f;  // temps relatif depuis début démo
   if ( time < 13.0f )
   {
       physics->ForceMotorSpeedZ(sinf(time));

       if ( Mod(time+PI/2.0f, PI) < PI*0.5f && m_lastSound+2.0f <= time )
       {
           m_lastSound = time;

           i = m_sound->Play(SOUND_MANIP, m_vehicle->RetPosition(0), 0.0f, 0.5f, TRUE);
           m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.5f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.3f, 1.0f, 0.8f, SOPER_CONTINUE);
           m_sound->AddEnvelope(i, 0.0f, 0.5f, 0.5f, SOPER_STOP);
       }
   }
   if ( time > 10.0f && !m_bForceSlow )
   {
       m_bForceSlow = TRUE;
       physics->SetForceSlow(TRUE);  // démarre le moteur au ralenti
   }
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoRemote::RetError()
{
   return ERR_OK;
}


// Cherche le cosmonaute.

CObject* CAutoRemote::SearchHuman()
{
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->RetPosition(0);

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       oType = pObj->RetType();
       if ( oType != OBJECT_HUMAN )  continue;

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 6.0f )  return pObj;
   }

   return 0;
}

// Cherche la voiture.

CObject* CAutoRemote::SearchVehicle()
{
   CObject*    pObj;
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
       if ( oType != OBJECT_CAR )  continue;

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 400.0f )  return pObj;
   }

   return 0;
}

