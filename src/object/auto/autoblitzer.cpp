// autoblitzer.cpp

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
#include "autoblitzer.h"





// Constructeur de l'objet.

CAutoBlitzer::CAutoBlitzer(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   CAuto::CAuto(iMan, object);

   Init();
}

// Destructeur de l'objet.

CAutoBlitzer::~CAutoBlitzer()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoBlitzer::DeleteObject(BOOL bAll)
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
   m_lastParticule = 0.0f;
   m_targetPos.x   = NAN;
   m_channelSphere = -1;
}


// Démarre l'objet.

void CAutoBlitzer::Start(int param)
{
}


// Gestion d'un événement.

BOOL CAutoBlitzer::EventProcess(const Event &event)
{
   CObject*    target;
   CObject*    car;
   CObject*    inca;
   CPhysics*   physics;
   ObjectType  tType[5];
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed, src, dst;
   FPOINT      dim;
   float       s, angle, goalAngle;
   float       tRadius[5];
   int         i;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;
   if ( m_main->RetEndingGame() )  return TRUE;

   car = m_object->RetTruck();
   if ( car == 0 )  return TRUE;

   m_progress += event.rTime*m_speed;

   if ( m_phase == ABLP_WAIT )
   {
       physics = car->RetPhysics();
       if ( physics == 0 )
       {
           s = 0.0f;
       }
       else
       {
           s = Abs(physics->RetLinMotionX(MO_REASPEED));
       }

       if ( s == 0.0f )
       {
           tType[0] = OBJECT_ALIEN9;  tRadius[0] =  60.0f;
           tType[1] = OBJECT_EVIL1;   tRadius[1] = 120.0f;
           tType[2] = OBJECT_EVIL3;   tRadius[2] =  60.0f;
           target = SearchObject(3, tType, tRadius, car->RetPosition(0));
           if ( target != 0 )
           {
               m_targetType = target->RetType();

               if ( m_targetType == OBJECT_EVIL3 )
               {
                   tType[0] = OBJECT_INCA7;  tRadius[0] = 100.0f;
                   inca = SearchObject(1, tType, tRadius, car->RetPosition(0));
                   if ( inca != 0 )  return TRUE;
               }

               car->SetLock(TRUE);

               m_targetPos = target->RetPosition(0);
               pos = car->RetPosition(0);
               goalAngle = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
               goalAngle -= car->RetAngleY(0);

               m_cameraType = m_camera->RetType();
               m_camera->SetType(CAMERA_BACK);
               angle = NormAngle(goalAngle);
               if ( angle < PI )  angle -= PI*0.15f;
               else               angle += PI*0.15f;
               m_camera->SetBackHoriz(-angle);
               if ( m_targetType == OBJECT_EVIL3 )
               {
                   m_camera->SetBackDist(50.0f);
               }
               m_camera->SetLockRotate(TRUE);
               m_main->SetStopwatch(FALSE);  // stoppe le chrono

               mat = m_object->RetWorldMatrix(0);
               src = Transform(*mat, D3DVECTOR(0.0f, 6.8f, 0.0f));
               mat = target->RetWorldMatrix(0);
               if ( m_targetType == OBJECT_ALIEN9 )
               {
                   dst = Transform(*mat, D3DVECTOR(0.0f, 5.0f, -14.5f));
                   m_delayBlitz = 2.0f;
               }
               if ( m_targetType == OBJECT_EVIL1 )
               {
                   dst = Transform(*mat, D3DVECTOR(0.0f, 4.0f, 0.0f));
                   m_delayBlitz = 2.0f;
               }
               if ( m_targetType == OBJECT_EVIL3 )
               {
                   dst = Transform(*mat, D3DVECTOR(0.0f, 15.0f, 0.0f));
                   m_delayBlitz = 3.0f;
               }
               m_particule->CreateRay(src, dst, PARTIRAY2, FPOINT(6.0f, 6.0f), m_delayBlitz);

               dim.x = 1.9f;
               dim.y = dim.x;
               m_channelSphere = m_particule->CreateParticule(src, D3DVECTOR(0.0f, 0.0f, 0.0f),
                                                dim, PARTISPHERE1, m_delayBlitz+1.5f, 0.0f);

               pos = car->RetPosition(0);
               i = m_sound->Play(SOUND_GGG, pos, 0.0f, 1.0f, TRUE);
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
               m_lastParticule = 0.0f;
           }
       }
   }

   if ( m_channelSphere != -1 )
   {
       mat = m_object->RetWorldMatrix(0);
       pos = Transform(*mat, D3DVECTOR(0.0f, 6.8f, 0.0f));
       if ( !m_particule->SetPosition(m_channelSphere, pos) )
       {
           m_channelSphere = -1;
       }
   }

   if ( m_phase == ABLP_WAIT    ||
        m_phase == ABLP_WAITOUT )
   {
       angle = m_object->RetAngleY(1);
       angle += event.rTime;
       m_object->SetAngleY(1, angle);
   }

   if ( m_phase == ABLP_BLITZ )
   {
       if ( m_progress < m_delayBlitz/m_delayTotal )
       {
           if ( m_lastParticule+m_engine->ParticuleAdapt(0.025f) <= m_progress )
           {
               m_lastParticule = m_progress;

               for ( i=0 ; i<4 ; i++ )
               {
                   mat = m_object->RetWorldMatrix(0);
                   pos = Transform(*mat, D3DVECTOR(0.0f, 6.8f, 0.0f));

                   speed.x = (Rand()-0.5f)*10.0f;
                   speed.z = (Rand()-0.5f)*10.0f;
                   speed.y = 5.0f+Rand()*5.0f;
                   dim.x = 0.4f;
                   dim.y = 0.4f;
                   m_particule->CreateParticule(pos, speed, dim, PARTIBLITZb, 1.0f, 20.0f);
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
           car->SetLock(FALSE);

           m_camera->SetType(m_cameraType);
           m_camera->SetBackHoriz(0.0f);
           m_camera->SetLockRotate(FALSE);
           m_camera->ResetLockRotate();
           m_main->SetStopwatch(TRUE);  // redémarre le chrono

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

   return TRUE;
}


// Cherche un objet proche.

CObject* CAutoBlitzer::SearchObject(int total, ObjectType *type,
                                   float *radius, D3DVECTOR center)
{
   CObject     *pObj, *pBest;
   ObjectType  oType;
   D3DVECTOR   pos;
   float       min, dist;
   int         i, j;

   pBest = 0;
   min = 100000.0f;
   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetExplo() )  continue;
       if ( pObj->RetLock() )  continue;  // déjà stoppé ?

       oType = pObj->RetType();

       for ( j=0 ; j<total ; j++ )
       {
           if ( type[j] == oType )
           {
               pos = pObj->RetPosition(0);
               dist = Length(pos, center);

               if ( dist <= radius[j] && dist < min )
               {
                   min = dist;
                   pBest = pObj;
               }
           }
       }
   }
   return pBest;
}

// Casse la fabrique instrument de torture.

void CAutoBlitzer::BreakDownFactory()
{
   CObject*    home;
   CAuto*      automat;
   ObjectType  tType[2];
   float       tRadius[2];

   tType[0]   = OBJECT_ALIEN9;
   tRadius[0] = 10.0f;
   home = SearchObject(1, tType, tRadius, m_targetPos);
   if ( home == 0 )  return;

   automat = home->RetAuto();
   if ( automat == 0 )  return;

   automat->Start(2);
}

// Casse le méchant vilain pas bô.

void CAutoBlitzer::BreakDownEvil1()
{
   CObject*    evil;
   CBrain*     brain;
   CPyro*      pyro;
   ObjectType  tType[2];
   float       tRadius[2];

   tType[0]   = OBJECT_EVIL1;
   tRadius[0] = 10.0f;
   evil = SearchObject(1, tType, tRadius, m_targetPos);
   if ( evil == 0 )  return;

   brain = evil->RetBrain();
   if ( brain != 0 )
   {
       brain->StopProgram();
   }

   pyro = new CPyro(m_iMan);
   pyro->Create(PT_BURNO, evil);  // destruction totale
}

// Casse le méchant roi.

void CAutoBlitzer::BreakDownEvil3()
{
   CObject*    evil;
   CPyro*      pyro;
   ObjectType  tType[2];
   float       tRadius[2];

   tType[0]   = OBJECT_EVIL3;
   tRadius[0] = 10.0f;
   evil = SearchObject(1, tType, tRadius, m_targetPos);
   if ( evil == 0 )  return;

   evil->DetachPiece(1, 0, D3DVECTOR(0.0f, 0.0f, 0.0f));

   pyro = new CPyro(m_iMan);
   pyro->Create(PT_FRAGO, evil);  // destruction totale
}


// Stoppe l'automate.

BOOL CAutoBlitzer::Abort()
{
   return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoBlitzer::RetError()
{
   return ERR_OK;
}

