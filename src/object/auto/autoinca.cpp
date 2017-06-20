// autoinca.cpp

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
#include "camera.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "motion.h"
#include "motionbot.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "pyro.h"
#include "auto.h"
#include "autoinca.h"





// Constructeur de l'objet.

CAutoInca::CAutoInca(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   CAuto::CAuto(iMan, object);

   Init();
}

// Destructeur de l'objet.

CAutoInca::~CAutoInca()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoInca::DeleteObject(BOOL bAll)
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
   m_bBlitz        = FALSE;
}


// Démarre l'objet.

void CAutoInca::Start(int param)
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   float       duration, mass;
   int         i;

   if ( param == 2 && !m_bBlitz )  // bouton rouge pressé ?
   {
       m_bBlitz = TRUE;

       m_king = SearchObject(OBJECT_EVIL3, m_object->RetPosition(0), 100.0f);
       if ( m_king == 0 )  return;
       m_vehicle = SearchObject(OBJECT_CAR, m_object->RetPosition(0), 100.0f);
       if ( m_vehicle == 0 )  return;
       m_vehicle->SetLock(TRUE);

       CameraBegin();
       SoundManip(2.0f, 1.0f, 1.0f);

       m_object->SetAngleY(1, (Rand()-0.5f)*1.0f);
       m_object->SetAngleZ(1, (Rand()-0.5f)*1.0f);

       mat = m_object->RetWorldMatrix(1);  // bouton rouge
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

       m_phase    = AINP_OPEN;
       m_speed    = 1.0f/2.0f;
       m_progress = 0.0f;
   }
}


// Gestion d'un événement.

BOOL CAutoInca::EventProcess(const Event &event)
{
   D3DVECTOR   pos;
   FPOINT      dim;
   float       progress, angle, delay;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;
   if ( m_main->RetEndingGame() )  return TRUE;

   m_progress += event.rTime*m_speed;
   progress = Norm(m_progress);

   if ( m_phase == AINP_WAIT )
   {
   }

   if ( m_phase == AINP_OPEN )
   {
       angle = progress*60.0f*PI/180.0f;
       m_object->SetAngleX(3,  angle);
       m_object->SetAngleX(4, -angle);

       if ( progress >= 1.0f )
       {
           pos = m_object->RetPosition(0);
           pos.y += 42.0f;
           dim.x = 25.0f;
           dim.y = dim.x;
           m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIGLINTb, 5.0f, 0.0f);

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
       pos.y = 10.0f-Bounce(progress)*15.0f;
       pos.z = 0.0f;
       m_object->SetPosition(2, pos);
       m_object->SetAngleY(2, progress*PI*1.0f);

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
           m_vehicle->SetLock(FALSE);
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
           delay = 2.0f+Rand()*2.0f;
           CreateBlitz(delay);
           m_phase    = AINP_TERM;
           m_speed    = 1.0f/delay;
           m_progress = 0.0f;
           progress = 0.0f;
       }
   }

   return TRUE;
}


// Cherche un objet proche.

CObject* CAutoInca::SearchObject(ObjectType type, D3DVECTOR center, float radius)
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

// Début du cadrage pour la caméra.

void CAutoInca::CameraBegin()
{
   m_cameraType = m_camera->RetType();
   m_camera->SetObject(m_object);
   m_camera->SetType(CAMERA_BACK);
   m_camera->SetSmooth(CS_SOFT);

   m_main->SetStopwatch(FALSE);  // stoppe le chrono
}

// Fin du cadrage pour la caméra.

void CAutoInca::CameraEnd()
{
   m_camera->SetObject(m_vehicle);
   m_camera->SetType(m_cameraType);
   m_camera->SetSmooth(CS_NORM);

   m_main->SetStopwatch(TRUE);  // redémarre le chrono
}

// Crée les éclairs.

void CAutoInca::CreateBlitz(float delay)
{
   CObject*    pObj;
   D3DVECTOR   pos, src, dst, eye;
   FPOINT      dim;
   int         i;

   pos = m_object->RetPosition(0);
   pos.y += 42.0f;
   dim.x = 8.5f;
   dim.y = dim.x;
   m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f),
                                dim, PARTISPHERE1, delay, 0.0f);

   src = m_object->RetPosition(0);
   src.y += 42.0f;
   dst = m_king->RetPosition(0);
   dst.y += 15.0f;
   m_channelRay = m_particule->CreateRay(src, dst, PARTIRAY2, FPOINT(6.0f, 6.0f), delay);

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( !pObj->RetLock() )  continue;
       if ( pObj->RetType() != OBJECT_INCA7 )  continue;
       if ( Rand() > 0.3f )  continue;

       dst = pObj->RetPosition(0);
       dst.y += 42.0f;
       m_particule->CreateRay(src, dst, PARTIRAY2, FPOINT(6.0f, 6.0f), 0.5f);
   }

   pos = m_king->RetPosition(0);
   eye = m_engine->RetEyePt();
   pos = SegmentDist(pos, eye, Length(pos, eye)*0.5f);
   m_sound->Play(SOUND_EVIL1, pos, 1.0f, 1.0f+(Rand()-0.5f)*0.8f);
   SoundBlitz(delay);
}

// Fait évoluer les éclairs.

void CAutoInca::ProgressBlitz(float progress)
{
   D3DVECTOR   src, dst;
   FPOINT      dim;
   float       h;

   h = (Rand()-0.5f)*20.0f;
   src = m_object->RetPosition(0);
   src.y += 42.0f;
   dst = m_king->RetPosition(0);
   dst.y += 15.0f+h;
   dst.x += (Rand()-0.5f)*8.0f;
   dst.z += (Rand()-0.5f)*8.0f;

   dst = SegmentDist(dst, src, 11.0f-h*0.1f);
   m_particule->SetGoal(m_channelRay, dst);

   if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
   {
       m_lastParticule = m_time;

       if ( Rand() < 0.5f )
       {
           dim.x = 6.0f;
           dim.y = dim.x;
           m_particule->CreateParticule(dst, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIGLINT, 1.0f, 0.0f);
       }
       else
       {
           dim.x = 3.0f;
           dim.y = dim.x;
           m_particule->CreateParticule(dst, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIBLITZb, 1.0f, 0.0f);
       }
   }
}

// Casse le méchant roi.

void CAutoInca::BreakDownEvil3()
{
   CObject*    evil;
   CPyro*      pyro;

   evil = SearchObject(OBJECT_EVIL3, m_targetPos, 10.0f);
   if ( evil == 0 )  return;

   pyro = new CPyro(m_iMan);
   pyro->Create(PT_BURNO, evil);  // destruction totale
}

// Fait entendre le son du bras manipulateur.

void CAutoInca::SoundManip(float time, float amplitude, float frequency)
{
   D3DVECTOR   pos;
   int         i;

//?    pos = m_object->RetPosition(0);
   pos = m_engine->RetEyePt();

   i = m_sound->Play(SOUND_MANIP, pos, 0.0f, 0.3f*frequency, TRUE);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}

// Fait entendre le son du rayon.

void CAutoInca::SoundBlitz(float time)
{
   D3DVECTOR   pos, eye;
   int         i;

   pos = m_object->RetPosition(0);
   eye = m_engine->RetEyePt();
   pos = SegmentDist(pos, eye, Length(pos, eye)*0.5f);

   i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, TRUE);
   m_sound->AddEnvelope(i, 1.0f, 1.0f, 0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 1.0f, 1.0f, time-0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.1f, SOPER_STOP);
}


// Stoppe l'automate.

BOOL CAutoInca::Abort()
{
   return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoInca::RetError()
{
   return ERR_OK;
}

