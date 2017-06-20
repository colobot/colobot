// autobomb.cpp

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
#include "pyro.h"
#include "physics.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "sound.h"
#include "auto.h"
#include "autobomb.h"



// Constructeur de l'objet.

CAutoBomb::CAutoBomb(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   CAuto::CAuto(iMan, object);

   Init();
}

// Destructeur de l'objet.

CAutoBomb::~CAutoBomb()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoBomb::DeleteObject(BOOL bAll)
{
   if ( m_partiSpark != -1 )
   {
       m_particule->DeleteParticule(m_partiSpark);
       m_partiSpark = -1;
   }
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoBomb::Init()
{
   m_time = Rand()*23.0f;

   m_progress = 0.0f;
   m_speed    = 1.0f/2.0f;

   m_actionType = AB_STOP;
   m_bBurn = FALSE;  // mèche éteinte
   m_angle = D3DVECTOR(0.0f, 0.0f, 0.0f);
   m_lastParticule = 0.0f;
   m_fireDelay = NAN;
   m_fireProgress = 0.0f;
   m_partiSpark = -1;
}


// Démarre l'objet.

void CAutoBomb::Start(int param)
{
}


// Démarre une action.

Error CAutoBomb::SetAction(int action, float time)
{
   if ( action == AB_BURNFLIP )
   {
       if ( m_bBurn )  action = AB_BURNOFF;
       else            action = AB_BURNON;
   }

   if ( action == AB_BURNON )
   {
       if ( m_fireDelay == NAN )
       {
           m_fireDelay = m_object->RetCmdLine(0);
           if ( m_fireDelay == NAN )  m_fireDelay = 10.0f;
       }
       m_bBurn = TRUE;  // mèche allumée
       return ERR_OK;
   }

   if ( action == AB_BURNOFF )
   {
       m_bBurn = FALSE;  // mèche éteinte
       return ERR_OK;
   }

   return CAuto::SetAction(action, time);
}



// Gestion d'un événement.

BOOL CAutoBomb::EventProcess(const Event &event)
{
   CObject*    vehicle;
   CPyro*      pyro;
   Character*  character;
   D3DMATRIX*  mat;
   D3DVECTOR   angle, pos, speed;
   FPOINT      dim;
   float       suppl, zoom, duration, factor;
   int         i;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;

   m_progress += event.rTime*m_speed;

   // Calcule l'inclinaison de la mèche.
   if ( m_actionType == AB_STOP )
   {
       factor = 0.2f;
   }
   else
   {
       factor = 1.0f;
   }
   angle.z = sinf(m_time*5.0f)*0.20f*factor;
   angle.x = sinf(m_time*4.7f)*0.16f*factor;
   angle.y = 0.0f;
   SpeedAdapt(angle, event.rTime);
   if ( m_actionType == AB_STOP )
   {
       angle.z += PI*0.2f;
   }
   if ( m_actionType == AB_TRUCK )  suppl = PI*0.5f;
   else                             suppl = 0.0f;

   // Calcule la longueur de la mèche.
   zoom = 1.0f-m_fireProgress;
   if ( zoom < 0.01f )  zoom = 0.01f;
   m_object->SetZoomY(1, zoom);
   angle *= zoom;  // incline moins si petit !

   if ( m_bBurn )  // mèche allumée ?
   {
       if ( !m_object->RetLock() )
       {
           m_fireProgress += event.rTime*(1.0f/m_fireDelay);
           if ( m_fireProgress > 1.0f )  m_fireProgress = 1.0f;
       }

       if ( m_partiSpark == -1 )
       {
           mat = m_object->RetWorldMatrix(4);
           pos = Transform(*mat, D3DVECTOR(0.0f, 2.0f, 0.0f));
           speed = D3DVECTOR(0.0f, 0.f, 0.0f);
           dim.x = 2.0f;
           dim.y = dim.x;
           m_partiSpark = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 10000.0f);
       }

       if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
       {
           m_lastParticule = m_time;

           for ( i=0 ; i<6 ; i++ )
           {
               mat = m_object->RetWorldMatrix(4);
               pos = Transform(*mat, D3DVECTOR(0.0f, 2.0f, 0.0f));
               speed.x = (Rand()-0.5f)*3.0f;
               speed.z = (Rand()-0.5f)*3.0f;
               speed.y = Rand()*10.0f;
               dim.x = 0.2f+Rand()*0.2f;
               dim.y = dim.x;
               duration = 1.0f+Rand()*1.5f;
               m_particule->CreateParticule(pos, speed, dim, PARTIGAS, duration, 20.0f);
           }
       }

       if ( m_fireProgress >= 1.0f )  // boum ?
       {
           vehicle = m_object->RetTruck();
           if ( vehicle != 0 )
           {
               m_object->SetTruck(0);
               vehicle->SetFret(0);
               vehicle->ExploPart(999, 1.0f);  // tout nu en chausettes

               character = vehicle->RetCharacter();
               mat = vehicle->RetWorldMatrix(0);
               pos = Transform(*mat, character->posFret);
               m_object->SetPosition(0, pos);
           }

           pyro = new CPyro(m_iMan);
           pyro->Create(PT_FRAGT, m_object);  // destruction totale
           return FALSE;
       }
   }
   else    // mèche éteinte ?
   {
       if ( m_partiSpark != -1 )
       {
           m_particule->DeleteParticule(m_partiSpark);
           m_partiSpark = -1;
       }
   }

   m_object->SetAngleZ(1, angle.z+suppl);
   m_object->SetAngleZ(2, angle.z);
   m_object->SetAngleZ(3, angle.z);
   m_object->SetAngleZ(4, angle.z);

   m_object->SetAngleX(1, angle.x);
   m_object->SetAngleX(2, angle.x);
   m_object->SetAngleX(3, angle.x);
   m_object->SetAngleX(4, angle.x);

   if ( m_partiSpark != -1 )
   {
       mat = m_object->RetWorldMatrix(4);
       pos = Transform(*mat, D3DVECTOR(0.0f, 2.0f, 0.0f));
       m_particule->SetPosition(m_partiSpark, pos);
   }

   return TRUE;
}

// Stoppe l'automate.

BOOL CAutoBomb::Abort()
{
   return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoBomb::RetError()
{
   return ERR_OK;
}


// Adapte la bombe en fonction de la vitesse du transporteur.

void CAutoBomb::SpeedAdapt(D3DVECTOR &angle, float rTime)
{
   CObject*    vehicle;
   CPhysics*   physics;
   float       lin, cir;

   vehicle = m_object->RetTruck();
   if ( vehicle == 0 )  return;
   physics = vehicle->RetPhysics();
   if ( physics == 0 )  return;

   lin = NormSign(physics->RetLinMotionX(MO_REAACCEL)/50.0f);
   cir = NormSign(physics->RetCirMotionY(MO_REASPEED)/physics->RetCirMotionY(MO_ADVSPEED));

   m_angle.z = Smooth(m_angle.z, lin*0.7f, rTime*1.5f);
   m_angle.x = Smooth(m_angle.x, cir*1.2f, rTime*1.5f);

   angle += m_angle;
}


