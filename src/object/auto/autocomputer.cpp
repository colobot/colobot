// autocomputer.cpp

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
#include "brain.h"
#include "motion.h"
#include "motionbot.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autocomputer.h"





// Constructeur de l'objet.

CAutoComputer::CAutoComputer(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   int     i;

   CAuto::CAuto(iMan, object);

   for ( i=0 ; i<9 ; i++ )
   {
       m_partiStop[i] = -1;
   }

   Init();
}

// Destructeur de l'objet.

CAutoComputer::~CAutoComputer()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoComputer::DeleteObject(BOOL bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoComputer::Init()
{
   m_pos = m_object->RetPosition(0);
   m_time = 0.0f;
   m_flagStop = 0;

   m_progress = 0.0f;
   m_speed    = 1.0f/2.0f;
   m_phase    = 0;
   m_speed = 1.0f/8.0f;
}


// Démarre l'objet.

void CAutoComputer::Start(int param)
{
   int     rank, total;

   if ( param >= 2 && param <= 4 &&  // bouton rouge pressé ?
        (m_flagStop & (1<<(param-2))) == 0 )
   {
       m_flagStop |= (1<<(param-2));

       if ( param == 2 )  rank = 1;
       if ( param == 3 )  rank = 3;
       if ( param == 4 )  rank = 2;
       m_object->SetZoomX(rank, 0.4f);

       total = 0;
       if ( m_flagStop & (1<<0) )  total ++;
       if ( m_flagStop & (1<<1) )  total ++;
       if ( m_flagStop & (1<<2) )  total ++;
       m_sound->Play(SOUND_RESEARCH, m_engine->RetEyePt(), 1.0f, 0.3f+0.5f*total);

       if ( total == 3 )
       {
           StopObject(OBJECT_CRAZY, m_object->RetPosition(0), 400.0f);
           m_sound->Play(SOUND_WAYPOINT, m_engine->RetEyePt(), 1.0f, 2.0f);
       }

       if ( m_flagStop == ((1<<0)|(1<<1)|(1<<2)) )  // éteint ?
       {
           m_object->SetLock(TRUE);  // il ne sera plus compté !
       }
   }
}


// Gestion d'un événement.

BOOL CAutoComputer::EventProcess(const Event &event)
{
   D3DVECTOR   pos;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;

   FireStopUpdate();
   m_progress += event.rTime*m_speed;

   if ( m_flagStop == ((1<<0)|(1<<1)|(1<<2)) )  // éteint ?
   {
       pos = m_pos;
       pos.y += sinf(m_time*40.0f)*0.1f;
       m_object->SetPosition(0, pos);
   }

   return TRUE;
}

// Cherche un objet proche.

void CAutoComputer::StopObject(ObjectType type, D3DVECTOR center, float radius)
{
   CObject*    pObj;
   CBrain*     brain;
   CMotion*    motion;
   D3DVECTOR   pos;
   float       dist;
   int         i;

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetExplo() )  continue;

       if ( type != pObj->RetType() )  continue;

       pos = pObj->RetPosition(0);
       dist = Length(pos, center);

       if ( dist <= radius )
       {
           brain = pObj->RetBrain();
           if ( brain != 0 )
           {
               brain->StopProgram();
           }

           motion = pObj->RetMotion();
           if ( motion != 0 )
           {
               motion->SetAction(MB_TRUCK);
           }
       }
   }
}

// Met à jour les feux de stop.

void CAutoComputer::FireStopUpdate()
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   BOOL        bOn;
   int         i;

   static float listpos[3*9] =
   {
       -9.5f, 13.5f,  0.0f,  // leds rouges en haut
       -4.6f, 13.5f,  8.3f,
        4.6f, 13.5f,  8.3f,
        9.5f, 13.5f,  0.0f,
        4.6f, 13.5f, -8.3f,
       -4.6f, 13.5f, -8.3f,

       -7.0f,  9.0f,  0.0f,  // leds jaunes dessus les boutons
        3.5f,  9.0f,  6.2f,
        3.5f,  9.0f, -6.2f,
   };

   mat = m_object->RetWorldMatrix(0);

   speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
   dim.x = 3.0f;
   dim.y = dim.x;

   for ( i=0 ; i<9 ; i++ )
   {
       bOn = FALSE;

       if ( i < 6 )
       {
           if ( m_flagStop != ((1<<0)|(1<<1)|(1<<2)) )
           {
               if ( Mod(m_time+0.1f*i, 0.6f) >= 0.4f )  bOn = TRUE;
           }
       }
       else
       {
           bOn = m_flagStop&(1<<(i-6));
       }

       if ( bOn )
       {
           if ( m_partiStop[i] == -1 )
           {
               pos.x = listpos[i*3+0];
               pos.y = listpos[i*3+1];
               pos.z = listpos[i*3+2];
               pos = Transform(*mat, pos);
               m_partiStop[i] = m_particule->CreateParticule(pos, speed, dim,
                                                             (i<6)?PARTISELR:PARTISELY,
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

BOOL CAutoComputer::Abort()
{
   return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoComputer::RetError()
{
   return ERR_OK;
}

