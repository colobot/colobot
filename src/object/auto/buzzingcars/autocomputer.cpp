// autocomputer.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motionbot.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/autocomputer.h"
#include "object/object_manager.h"





// Constructeur de l'objet.

CAutoComputer::CAutoComputer(COldObject* object)
                        : CAuto(object)
{
   int     i;

   for ( i=0 ; i<9 ; i++ )
   {
       m_partiStop[i] = -1;
   }

   Init();
}

// Destructeur de l'objet.

CAutoComputer::~CAutoComputer()
{
}


// Détruit l'objet.

void CAutoComputer::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoComputer::Init()
{
   m_pos = m_object->GetPartPosition(0);
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
       m_object->SetPartScaleX(rank, 0.4f);

       total = 0;
       if ( m_flagStop & (1<<0) )  total ++;
       if ( m_flagStop & (1<<1) )  total ++;
       if ( m_flagStop & (1<<2) )  total ++;
       m_sound->Play(SOUND_RESEARCH, m_engine->GetEyePt(), 1.0f, 0.3f+0.5f*total);

       if ( total == 3 )
       {
           StopObject(OBJECT_CRAZY, m_object->GetPartPosition(0), 400.0f);
           m_sound->Play(SOUND_WAYPOINT, m_engine->GetEyePt(), 1.0f, 2.0f);
       }

       if ( m_flagStop == ((1<<0)|(1<<1)|(1<<2)) )  // éteint ?
       {
           m_object->SetLock(true);  // il ne sera plus compté !
       }
   }
}


// Gestion d'un événement.

bool CAutoComputer::EventProcess(const Event &event)
{
   Math::Vector   pos;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   FireStopUpdate();
   m_progress += event.rTime*m_speed;

   if ( m_flagStop == ((1<<0)|(1<<1)|(1<<2)) )  // éteint ?
   {
       pos = m_pos;
       pos.y += sinf(m_time*40.0f)*0.1f;
       m_object->SetPartPosition(0, pos);
   }

   return true;
}

// Cherche un objet proche.

void CAutoComputer::StopObject(ObjectType type, Math::Vector center, float radius)
{
   COldObject*    pObj;
   CMotion*    motion;
   Math::Vector   pos;
   float       dist;
   int         i;

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       if ( !pObj->Implements(ObjectInterfaceType::Old) ) continue;
       if ( dynamic_cast<COldObject*>(pObj)->IsDying() )  continue;

       if ( type != pObj->GetType() )  continue;

       pos = pObj->GetPosition();
       dist = Math::Distance(pos, center);

       if ( dist <= radius )
       {
           if (pObj->Implements(ObjectInterfaceType::Programmable))
           {
               dynamic_cast<CProgrammableObject*>(pObj)->StopProgram();
           }

           assert(pObj->Implements(ObjectInterfaceType::Movable));
           motion = dynamic_cast<CMovableObject*>(pObj)->GetMotion();
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
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   bool        bOn;
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

   mat = m_object->GetWorldMatrix(0);

   speed = Math::Vector(0.0f, 0.0f, 0.0f);
   dim.x = 3.0f;
   dim.y = dim.x;

   for ( i=0 ; i<9 ; i++ )
   {
       bOn = false;

       if ( i < 6 )
       {
           if ( m_flagStop != ((1<<0)|(1<<1)|(1<<2)) )
           {
               if ( Math::Mod(m_time+0.1f*i, 0.6f) >= 0.4f )  bOn = true;
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
               pos = Math::Transform(*mat, pos);
               m_partiStop[i] = m_particle->CreateParticle(pos, speed, dim,
                                                             (i<6)?Gfx::PARTISELR:Gfx::PARTISELY,
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

bool CAutoComputer::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoComputer::GetError()
{
   return ERR_OK;
}

