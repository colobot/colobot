// autobomb.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "graphics/engine/pyro.h"
#include "physics/physics.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/building/autobomb.h"
#include "graphics/engine/pyro_manager.h"



// Constructeur de l'objet.

CAutoBomb::CAutoBomb(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoBomb::~CAutoBomb()
{
}


// Détruit l'objet.

void CAutoBomb::DeleteObject(bool bAll)
{
   if ( m_partiSpark != -1 )
   {
       m_particle->DeleteParticle(m_partiSpark);
       m_partiSpark = -1;
   }
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoBomb::Init()
{
   m_time = Math::Rand()*23.0f;

   m_progress = 0.0f;
   m_speed    = 1.0f/2.0f;

   m_actionType = AB_STOP;
   m_bBurn = false;  // mèche éteinte
   m_angle = Math::Vector(0.0f, 0.0f, 0.0f);
   m_lastParticle = 0.0f;
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
       if ( std::isnan(m_fireDelay) )
       {
           m_fireDelay = m_object->GetCmdLine(0);
           if ( std::isnan(m_fireDelay) )  m_fireDelay = 10.0f;
       }
       m_bBurn = true;  // mèche allumée
       return ERR_OK;
   }

   if ( action == AB_BURNOFF )
   {
       m_bBurn = false;  // mèche éteinte
       return ERR_OK;
   }

   return CAuto::SetAction(action, time);
}



// Gestion d'un événement.

bool CAutoBomb::EventProcess(const Event &event)
{
   COldObject*    vehicle;
   Gfx::CPyro*      pyro;
   Character*  character;
   Math::Matrix*  mat;
   Math::Vector   angle, pos, speed;
   Math::Point      dim;
   float       suppl, zoom, duration, factor;
   int         i;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

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
       angle.z += Math::PI*0.2f;
   }
   if ( m_actionType == AB_TRUCK )  suppl = Math::PI*0.5f;
   else                             suppl = 0.0f;

   // Calcule la longueur de la mèche.
   zoom = 1.0f-m_fireProgress;
   if ( zoom < 0.01f )  zoom = 0.01f;
   m_object->SetPartScaleY(1, zoom);
   angle *= zoom;  // incline moins si petit !

   if ( m_bBurn )  // mèche allumée ?
   {
       if ( !m_object->GetLock() )
       {
           m_fireProgress += event.rTime*(1.0f/m_fireDelay);
           if ( m_fireProgress > 1.0f )  m_fireProgress = 1.0f;
       }

       if ( m_partiSpark == -1 )
       {
           mat = m_object->GetWorldMatrix(4);
           pos = Transform(*mat, Math::Vector(0.0f, 2.0f, 0.0f));
           speed = Math::Vector(0.0f, 0.f, 0.0f);
           dim.x = 2.0f;
           dim.y = dim.x;
           m_partiSpark = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISELY, 10000.0f);
       }

       if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
       {
           m_lastParticle = m_time;

           for ( i=0 ; i<6 ; i++ )
           {
               mat = m_object->GetWorldMatrix(4);
               pos = Transform(*mat, Math::Vector(0.0f, 2.0f, 0.0f));
               speed.x = (Math::Rand()-0.5f)*3.0f;
               speed.z = (Math::Rand()-0.5f)*3.0f;
               speed.y = Math::Rand()*10.0f;
               dim.x = 0.2f+Math::Rand()*0.2f;
               dim.y = dim.x;
               duration = 1.0f+Math::Rand()*1.5f;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, duration, 20.0f);
           }
       }

       if ( m_fireProgress >= 1.0f )  // boum ?
       {
           vehicle = dynamic_cast<COldObject*>(m_object->GetTransporter());
           if ( vehicle != 0 )
           {
               m_object->SetTransporter(0);
               vehicle->SetCargo(0);
               vehicle->ExploPart(999, 1.0f);  // tout nu en chausettes

               character = vehicle->GetCharacter();
               mat = vehicle->GetWorldMatrix(0);
               pos = Transform(*mat, character->posFret);
               m_object->SetPosition(pos);
           }

           m_engine->GetPyroManager()->Create(Gfx::PT_FRAGT, m_object);  // destruction totale
           return false;
       }
   }
   else    // mèche éteinte ?
   {
       if ( m_partiSpark != -1 )
       {
           m_particle->DeleteParticle(m_partiSpark);
           m_partiSpark = -1;
       }
   }

   m_object->SetPartRotationZ(1, angle.z+suppl);
   m_object->SetPartRotationZ(2, angle.z);
   m_object->SetPartRotationZ(3, angle.z);
   m_object->SetPartRotationZ(4, angle.z);

   m_object->SetPartRotationX(1, angle.x);
   m_object->SetPartRotationX(2, angle.x);
   m_object->SetPartRotationX(3, angle.x);
   m_object->SetPartRotationX(4, angle.x);

   if ( m_partiSpark != -1 )
   {
       mat = m_object->GetWorldMatrix(4);
       pos = Transform(*mat, Math::Vector(0.0f, 2.0f, 0.0f));
       m_particle->SetPosition(m_partiSpark, pos);
   }

   return true;
}

// Stoppe l'automate.

bool CAutoBomb::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoBomb::GetError()
{
   return ERR_OK;
}


// Adapte la bombe en fonction de la vitesse du transporteur.

void CAutoBomb::SpeedAdapt(Math::Vector &angle, float rTime)
{
   COldObject*    vehicle;
   CPhysics*   physics;
   float       lin, cir;

   vehicle = dynamic_cast<COldObject*>(m_object->GetTransporter());
   if ( vehicle == 0 )  return;
   physics = vehicle->GetPhysics();
   if ( physics == 0 )  return;

   lin = Math::NormSign(physics->GetLinMotionX(MO_REAACCEL)/50.0f);
   cir = Math::NormSign(physics->GetCirMotionY(MO_REASPEED)/physics->GetCirMotionY(MO_ADVSPEED));

   m_angle.z = Math::Smooth(m_angle.z, lin*0.7f, rTime*1.5f);
   m_angle.x = Math::Smooth(m_angle.x, cir*1.2f, rTime*1.5f);

   angle += m_angle;
}


