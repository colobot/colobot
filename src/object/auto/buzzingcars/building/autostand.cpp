// autostand.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/building/autostand.h"
#include "object/object_manager.h"



// Constructeur de l'objet.

CAutoStand::CAutoStand(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoStand::~CAutoStand()
{
}


// Détruit l'objet.

void CAutoStand::DeleteObject(bool bAll)
{
   if ( m_channelSound != -1 )
   {
       m_sound->FlushEnvelope(m_channelSound);
       m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 2.0f, SOPER_STOP);
       m_channelSound = -1;
   }

   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoStand::Init()
{
   m_time = 0.0f;

   m_progress = 0.0f;
   m_speed    = 1.0f/2.0f;
   m_channelSound = -1;
   m_totalPerso = 0;
}


// Démarre l'objet.

void CAutoStand::Start(int param)
{
   Math::Matrix*  mat;
   CObject*    object;
   ObjectType  type;
   Math::Vector   pos;
   bool        occupied[8*4];
   float       value, angle, percent;
   int         i, r, x, y;

   value = m_object->GetCmdLine(0);
   if ( value == NAN )
   {
       m_totalPerso = 0;
   }
   else
   {
       m_totalPerso = (int)value;
//       percent = m_engine->GetGadgetQuantity();
       percent = 1.0f; // TODO (krzys_h): cleanup
       m_totalPerso = (int)(percent*m_totalPerso);
   }
   if ( m_totalPerso > 8*4 )  m_totalPerso = 8*4;

   for ( i=0 ; i<8*4 ; i++ )
   {
       occupied[i] = false;
   }

   mat = m_object->GetWorldMatrix(0);
   angle = m_object->GetPartRotationY(0);
   for ( i=0 ; i<m_totalPerso ; i++ )
   {
       r = rand()%4;
       if ( r == 0 )  type = OBJECT_BOT1;
       if ( r == 1 )  type = OBJECT_BOT2;
       if ( r == 2 )  type = OBJECT_BOT3;
       if ( r == 3 )  type = OBJECT_BOT4;

       while ( true )
       {
           int rank[16] = {0,0,0,0,0,0,0,0,1,1,1,1,1,2,2,3};
           x = rank[rand()%16];  // rempli mieux les premiers rangs
           pos.x = 15.0f-x*10.0f;
           pos.y = 4.0f+x*4.0f;

           y = rand()%8;
           pos.z = y*10.0f-40.0f+(Math::Rand()-0.5f)*6.0f;

           if ( occupied[x*8+y] == false )
           {
               occupied[x*8+y] = true;
               break;
           }
       }
       pos = Math::Transform(*mat, pos);

       object = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type); // TODO (krzys_h): bPlumb = true
       pos.y += object->GetCharacter()->height;
       object->SetPosition(pos);
   }
}


// Gestion d'un événement.

bool CAutoStand::EventProcess(const Event &event)
{
   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   m_progress += event.rTime*m_speed;
   SoundFrame(event.rTime);

   return true;
}

// Gestion du son.

void CAutoStand::SoundFrame(float rTime)
{
   Math::Vector   pos, look;
   float       dist;

   if ( m_totalPerso == 0 )  return;

   pos = m_object->GetPartPosition(0);
   look = m_engine->GetLookatPt();
   dist = Math::Distance(pos, look);

   if ( dist >= 400.0f )
   {
       if ( m_channelSound != -1 )
       {
           m_sound->FlushEnvelope(m_channelSound);
           m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 2.0f, SOPER_STOP);
           m_channelSound = -1;
       }
   }
   else
   {
       if ( m_channelSound == -1 )
       {
           m_channelSound = m_sound->Play(SOUND_RESEARCH, pos, 0.0f, 1.0f, true);
           m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 2.0f, SOPER_CONTINUE);
           m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
       }
   }
}



// Stoppe l'automate.

bool CAutoStand::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoStand::GetError()
{
   return ERR_OK;
}


