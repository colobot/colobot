// autohelico.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "physics/physics.h"
#include "graphics/engine/terrain.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motionbot.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/additional/autohelico.h"





// Constructeur de l'objet.

CAutoHelico::CAutoHelico(COldObject* object)
                        : CAuto(object)
{
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
}


// Détruit l'objet.

void CAutoHelico::DeleteObject(bool bAll)
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

bool CAutoHelico::EventProcess(const Event &event)
{
   COldObject*    car;
   CPhysics*   physics;
   float       hopeSpeed;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   car = dynamic_cast<COldObject*>(m_object->GetTransporter());
   if ( car == 0 )
   {
       if ( m_rotorSound != -1 )
       {
           m_sound->FlushEnvelope(m_rotorSound);
           m_sound->AddEnvelope(m_rotorSound, 0.0f, 1.0f, 0.3f, SOPER_STOP);
           m_rotorSound = -1;
       }
       UpdateRotorMapping(0.0f);
       return true;
   }

   hopeSpeed = 4.0f;  // vitesse au sol
   physics = car->GetPhysics();
   if ( physics != 0 && physics->GetFloorHeight() > 0.0f )
   {
       hopeSpeed = 20.0f;  // vitesse en l'air
   }
   m_rotorSpeed = Math::Smooth(m_rotorSpeed, hopeSpeed, event.rTime*2.0f);
   UpdateRotorMapping(m_rotorSpeed);

   m_rotorAngle += event.rTime*m_rotorSpeed*2.0f;  // fait tourner l'hélice +/- vite
   m_object->SetPartRotationY(1, m_rotorAngle);

   if ( m_rotorSound == -1 )
   {
       m_rotorSound = m_sound->Play(SOUND_HELICO, car->GetPartPosition(0), 1.0f, 1.0f, true);
   }
   if ( m_rotorSound != -1 )
   {
       m_sound->Position(m_rotorSound, car->GetPartPosition(0));
       m_sound->Frequency(m_rotorSound, m_rotorSpeed/10.0f);
       m_sound->Amplitude(m_rotorSound, 1.0f);
   }

   return true;
}


// Stoppe l'automate.

bool CAutoHelico::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoHelico::GetError()
{
   return ERR_OK;
}


// Met à jour le mapping du rotor.

void CAutoHelico::UpdateRotorMapping(float speed)
{
   Gfx::Material    mat;
   float           au, bu, x1, x2;
   int             rotor, i, s;

   speed = abs(speed);
        if ( speed <  4.0f )  s = 0;
   else if ( speed <  8.0f )  s = 1;
   else if ( speed < 14.0f )  s = 2;
   else                       s = 3;

   if ( s == m_lastRotorSpeed )  return;
   m_lastRotorSpeed = s;

   mat.diffuse.r = 1.0f;
   mat.diffuse.g = 1.0f;
   mat.diffuse.b = 1.0f;  // blanc
   mat.ambient.r = 0.5f;
   mat.ambient.g = 0.5f;
   mat.ambient.b = 0.5f;

   rotor = m_object->GetObjectRank(1);  // helico2.mod

   x1 = (119.0f+26.0f*s)/256.0f;
   x2 = x1+26.0f/256.0f;

   au = (x2-x1)/(2.0f*7.0f);
   bu = (x2+x1)/2.0f;
   for ( i=0 ; i<1 ; i++ )
   {
       m_engine->ChangeTextureMapping(rotor,
                                      mat, Gfx::ENG_RSTATE_TTEXTURE_WHITE|Gfx::ENG_RSTATE_2FACE,
                                      "search.tga", "",
                                      Gfx::ENG_TEX_MAPPING_1X,
                                      au, bu, 1.0f, 0.0f);
   }
}

