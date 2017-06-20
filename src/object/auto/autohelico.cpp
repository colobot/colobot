// autohelico.cpp

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
#include "terrain.h"
#include "motion.h"
#include "motionbot.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autohelico.h"





// Constructeur de l'objet.

CAutoHelico::CAutoHelico(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   CAuto::CAuto(iMan, object);

   Init();
}

// Destructeur de l'objet.

CAutoHelico::~CAutoHelico()
{
   if ( m_rotorSound != -1 )
   {
       m_sound->FlushEnvelope(m_rotorSound);
       m_sound->AddEnvelope(m_rotorSound, 0.0f, 1.0f, 0.3f, SOPER_STOP);
       m_rotorSound = -1;
   }
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoHelico::DeleteObject(BOOL bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoHelico::Init()
{
   m_time       = 0.0f;
   m_progress   = 0.0f;
   m_speed      = 1.0f/2.0f;
   m_rotorSpeed = 4.0f;
   m_lastRotorSpeed = 0;
   m_rotorAngle = 0.0f;
   m_rotorSound = -1;

}


// Démarre l'objet.

void CAutoHelico::Start(int param)
{
}


// Gestion d'un événement.

BOOL CAutoHelico::EventProcess(const Event &event)
{
   CObject*    car;
   CPhysics*   physics;
   float       hopeSpeed;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;

   car = m_object->RetTruck();
   if ( car == 0 )
   {
       if ( m_rotorSound != -1 )
       {
           m_sound->FlushEnvelope(m_rotorSound);
           m_sound->AddEnvelope(m_rotorSound, 0.0f, 1.0f, 0.3f, SOPER_STOP);
           m_rotorSound = -1;
       }
       UpdateRotorMapping(0.0f);
       return TRUE;
   }

   hopeSpeed = 4.0f;  // vitesse au sol
   physics = car->RetPhysics();
   if ( physics != 0 && physics->RetFloorHeight() > 0.0f )
   {
       hopeSpeed = 20.0f;  // vitesse en l'air
   }
   m_rotorSpeed = Smooth(m_rotorSpeed, hopeSpeed, event.rTime*2.0f);
   UpdateRotorMapping(m_rotorSpeed);

   m_rotorAngle += event.rTime*m_rotorSpeed*2.0f;  // fait tourner l'hélice +/- vite
   m_object->SetAngleY(1, m_rotorAngle);

   if ( m_rotorSound == -1 )
   {
       m_rotorSound = m_sound->Play(SOUND_HELICO, car->RetPosition(0), 1.0f, 1.0f, TRUE);
   }
   if ( m_rotorSound != -1 )
   {
       m_sound->Position(m_rotorSound, car->RetPosition(0));
       m_sound->Frequency(m_rotorSound, m_rotorSpeed/10.0f);
       m_sound->Amplitude(m_rotorSound, 1.0f);
   }

   return TRUE;
}


// Stoppe l'automate.

BOOL CAutoHelico::Abort()
{
   return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoHelico::RetError()
{
   return ERR_OK;
}


// Met à jour le mapping du rotor.

void CAutoHelico::UpdateRotorMapping(float speed)
{
   D3DMATERIAL7    mat;
   float           limit[4], au, bu, x1, x2;
   int             rotor, i, s;

   speed = Abs(speed);
        if ( speed <  4.0f )  s = 0;
   else if ( speed <  8.0f )  s = 1;
   else if ( speed < 14.0f )  s = 2;
   else                       s = 3;

   if ( s == m_lastRotorSpeed )  return;
   m_lastRotorSpeed = s;

   ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
   mat.diffuse.r = 1.0f;
   mat.diffuse.g = 1.0f;
   mat.diffuse.b = 1.0f;  // blanc
   mat.ambient.r = 0.5f;
   mat.ambient.g = 0.5f;
   mat.ambient.b = 0.5f;

   rotor = m_object->RetObjectRank(1);  // helico2.mod

//?    limit[0] = 0.0f;
//?    limit[1] = m_engine->RetLimitLOD(0);
//?    limit[2] = limit[1];
//?    limit[3] = m_engine->RetLimitLOD(1);
   limit[0] = 0.0f;
   limit[1] = 1000000.0f;
   limit[2] = 0.0f;
   limit[3] = 1000000.0f;

   x1 = (119.0f+26.0f*s)/256.0f;
   x2 = x1+26.0f/256.0f;

   au = (x2-x1)/(2.0f*7.0f);
   bu = (x2+x1)/2.0f;
   for ( i=0 ; i<1 ; i++ )
   {
       m_engine->ChangeTextureMapping(rotor,
                                      mat, D3DSTATETTw|D3DSTATE2FACE,
                                      "search.tga", "",
                                      limit[i*2+0], limit[i*2+1], D3DMAPPING1X,
                                      au, bu, 1.0f, 0.0f);
   }
}

