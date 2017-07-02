// autobarrel.cpp

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
#include "object/auto/buzzingcars/building/autobarrel.h"



// Constructeur de l'objet.

CAutoBarrel::CAutoBarrel(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoBarrel::~CAutoBarrel()
{
}


// Détruit l'objet.

void CAutoBarrel::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoBarrel::Init()
{
   m_time = 0.0f;

   m_progress = 0.0f;
   m_speed    = 1.0f/0.05f;

   m_type = m_object->GetType();
   m_force = Math::Rand();
}


// Démarre l'objet.

void CAutoBarrel::Start(int param)
{
}


// Gestion d'un événement.

bool CAutoBarrel::EventProcess(const Event &event)
{
   Math::Matrix*  mat;
   Math::Vector   lookat, pos, speed;
   Math::Point      dim;
   float       dist, duration;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   m_progress += event.rTime*m_speed;

   if ( m_progress >= 1.0f )
   {
       m_progress = 0.0f;
       m_speed    = 1.0f/0.05f;

       lookat = m_engine->GetLookatPt();
       pos = m_object->GetPartPosition(0);
       dist = Math::DistanceProjected(lookat, pos);
       if ( dist < 400.0f )
       {
           mat = m_object->GetWorldMatrix(0);

           if ( m_type == OBJECT_BARRELa )
           {
               pos.x = (Math::Rand()-0.5f)*2.0f;
               pos.z = (Math::Rand()-0.5f)*2.0f;
               pos.y = 4.5f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*1.5f;
               speed.z = (Math::Rand()-0.5f)*1.5f;
               speed.y = 2.0f+Math::Rand()*2.0f;
               dim.x = 0.4f+Math::Rand()*0.8f;
               dim.y = dim.x;
               duration = 1.0f+Math::Rand()*2.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEJECT, duration);
           }
           else if ( m_type == OBJECT_BARREL )
           {
               if ( m_force > 0.3f )
               {
                   pos.x = (Math::Rand()-0.5f)*1.0f;
                   pos.z = (Math::Rand()-0.5f)*1.0f;
                   pos.y = 4.5f;
                   pos = Math::Transform(*mat, pos);
                   speed.x = (Math::Rand()-0.5f)*1.0f*m_force;
                   speed.z = (Math::Rand()-0.5f)*1.0f*m_force;
                   speed.y = (1.0f+Math::Rand()*2.0f)*m_force;
                   dim.x = (0.8f+Math::Rand()*1.6f)*m_force;
                   dim.y = dim.x;
                   duration = 1.0f+Math::Rand()*2.0f;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, duration);

                   pos.x = (Math::Rand()-0.5f)*2.0f;
                   pos.z = (Math::Rand()-0.5f)*2.0f;
                   pos.y = 4.5f;
                   pos = Math::Transform(*mat, pos);
                   speed.x = (Math::Rand()-0.5f)*1.0f*m_force;
                   speed.z = (Math::Rand()-0.5f)*1.0f*m_force;
                   speed.y = (4.0f+Math::Rand()*5.0f)*m_force;
                   dim.x = (1.2f+Math::Rand()*2.2f)*m_force;
                   dim.y = dim.x;
                   duration = 2.0f+Math::Rand()*4.0f;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, duration);
               }
           }
           else if ( m_type == OBJECT_CARCASS1 )
           {
               pos.x = 3.0f+(Math::Rand()-0.5f)*1.0f;
               pos.z = 0.0f+(Math::Rand()-0.5f)*2.0f;
               pos.y = 1.5f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*1.0f;
               speed.z = (Math::Rand()-0.5f)*1.0f;
               speed.y = (1.0f+Math::Rand()*2.0f);
               dim.x = (1.2f+Math::Rand()*2.4f);
               dim.y = dim.x;
               duration = 1.0f+Math::Rand()*2.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, duration);

               pos.x = 3.0f+(Math::Rand()-0.5f)*1.0f;
               pos.z = 0.0f+(Math::Rand()-0.5f)*1.0f;
               pos.y = 1.5f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*1.0f;
               speed.z = (Math::Rand()-0.5f)*1.0f;
               speed.y = (4.0f+Math::Rand()*5.0f);
               dim.x = (1.8f+Math::Rand()*3.3f);
               dim.y = dim.x;
               duration = 2.0f+Math::Rand()*4.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, duration);
           }
           else if ( m_type == OBJECT_CARCASS2 )
           {
               pos.x = 3.0f+(Math::Rand()-0.5f)*1.0f;
               pos.z = 1.0f+(Math::Rand()-0.5f)*2.0f;
               pos.y = 1.5f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*1.0f;
               speed.z = (Math::Rand()-0.5f)*1.0f;
               speed.y = (1.0f+Math::Rand()*2.0f);
               dim.x = (1.2f+Math::Rand()*2.4f);
               dim.y = dim.x;
               duration = 1.0f+Math::Rand()*2.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, duration);

               pos.x = 3.0f+(Math::Rand()-0.5f)*1.0f;
               pos.z = 1.0f+(Math::Rand()-0.5f)*1.0f;
               pos.y = 1.5f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*1.0f;
               speed.z = (Math::Rand()-0.5f)*1.0f;
               speed.y = (4.0f+Math::Rand()*5.0f);
               dim.x = (1.8f+Math::Rand()*3.3f);
               dim.y = dim.x;
               duration = 2.0f+Math::Rand()*4.0f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, duration);
           }
       }
   }

   return true;
}

// Stoppe l'automate.

bool CAutoBarrel::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoBarrel::GetError()
{
   return ERR_OK;
}


