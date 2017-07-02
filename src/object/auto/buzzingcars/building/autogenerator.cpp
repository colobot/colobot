// autogenerator.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "object/motion/motion.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/building/autogenerator.h"
#include "object/object_manager.h"



// Constructeur de l'objet.

CAutoGenerator::CAutoGenerator(COldObject* object)
                        : CAuto(object)
{
   Init();
}

// Destructeur de l'objet.

CAutoGenerator::~CAutoGenerator()
{
}


// Détruit l'objet.

void CAutoGenerator::DeleteObject(bool bAll)
{
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoGenerator::Init()
{
   m_time = 0.0f;
   m_lastParticule = 0.0f;

   m_totalCreate = 10;
   m_maxCreate = 10;
   m_delay = 10.0f;
   m_programNumber = -1;

   m_phase    = AGEP_WAIT;
   m_progress = 0.0f;
   m_speed    = 1.0f/m_delay;
}


// Démarre l'objet.

void CAutoGenerator::Start(int param)
{
   float       value;

   value = m_object->GetCmdLine(0);
   if ( value == NAN )
   {
       m_totalCreate = 0;
   }
   else
   {
       m_totalCreate = (int)value;
   }

   value = m_object->GetCmdLine(1);
   if ( value == NAN )
   {
       m_maxCreate = 10;
   }
   else
   {
       m_maxCreate = (int)value;
   }

   value = m_object->GetCmdLine(2);
   if ( value == NAN )
   {
       m_delay = 10.0f;
   }
   else
   {
       m_delay = value;
   }

   value = m_object->GetCmdLine(3);
   if ( value == NAN )
   {
       m_programNumber = -1;
   }
   else
   {
       m_programNumber = (int)value;
   }

   m_phase    = AGEP_WAIT;
   m_progress = 0.0f;
   m_speed    = 1.0f/m_delay;
}


// Gestion d'un événement.

bool CAutoGenerator::EventProcess(const Event &event)
{
   Math::Vector   pos, speed;
   Math::Point      dim, c, p;
   COldObject*    pObj;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   m_progress += event.rTime*m_speed;

   if ( m_phase == AGEP_WAIT )
   {
       if ( m_progress >= 1.0f )
       {
           pObj = SearchObject();
           if ( pObj != 0 ||  // position occupée ?
                m_totalCreate == 0 ||  // plus rien à créer ?
                CountObject(OBJECT_EVIL1) >= m_maxCreate ||  // trop d'objets ?
                !m_main->IsGameTime() )  // partie terminée ?
           {
               m_phase    = AGEP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/m_delay;
           }
           else
           {
               if ( !CreateObject(OBJECT_EVIL1) )
               {
                   m_phase    = AGEP_WAIT;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/m_delay;
               }
               else
               {
                   m_totalCreate --;

                   m_phase    = AGEP_CREATE;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/5.0f;
               }
           }
       }
   }

   if ( m_phase == AGEP_CREATE )
   {
       pObj = SearchObject(OBJECT_EVIL1);
       if ( pObj == 0 )
       {
           m_phase    = AGEP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/m_delay;
           return true;
       }

       if ( m_progress < 1.0f )
       {
           pObj->SetPartScale(0, m_progress);
           pObj->SetPartRotationY(0, pObj->GetPartRotationY(0)+event.rTime*5.0f);

           if ( m_lastParticule+m_engine->ParticleAdapt(0.10f) <= m_time )
           {
               m_lastParticule = m_time;

               pos = m_object->GetPartPosition(0);
               c.x = pos.x;
               c.y = pos.z;
               p.x = c.x;
               p.y = c.y+2.0f;
               p = Math::RotatePoint(c, Math::Rand()*Math::PI*2.0f, p);
               pos.x = p.x;
               pos.z = p.y;
               pos.y += 2.5f+Math::Rand()*3.0f;
               speed = Math::Vector(0.0f, 0.0f, 0.0f);
               dim.x = Math::Rand()*2.0f+1.0f;
               dim.y = dim.x;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEXPLOG2, 1.0f, 0.0f);

               pos = m_object->GetPartPosition(0);
//?                pos.y += 3.0f;
               speed.x = (Math::Rand()-0.5f)*30.0f;
               speed.z = (Math::Rand()-0.5f)*30.0f;
               speed.y = Math::Rand()*20.0f+10.0f;
               dim.x = Math::Rand()*0.4f+0.4f;
               dim.y = dim.x;
               m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK4, 2.0f, 50.0f, 1.2f, 1.2f);

               pos = m_object->GetPartPosition(0);
               pos.y += 20.0f;
               speed.x = (Math::Rand()-0.5f)*1.5f;
               speed.z = (Math::Rand()-0.5f)*1.5f;
               speed.y = -20.0f;
               dim.x = Math::Rand()*2.0f+2.0f;
               dim.y = dim.x;
               m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEXPLOG2, 1.0f, 0.0f);

               m_sound->Play(SOUND_ENERGY, m_object->GetPartPosition(0),
                             1.0f, 1.0f+Math::Rand()*1.5f);
           }
       }
       else
       {
           pObj->SetPartScale(0, 1.0f);
           StartAction(pObj, 0);  // attend au repos
           if ( m_programNumber != -1 )
           {
               char    name[100];
               sprintf(name, "auto%.2d.txt", m_programNumber);
               pObj->ReadProgram(0, name);
               pObj->RunProgram(0);
           }

           m_phase    = AGEP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/m_delay;
       }
   }

   return true;
}


// Stoppe l'automate.

bool CAutoGenerator::Abort()
{
   return true;
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoGenerator::GetError()
{
   return ERR_OK;
}


// Cherche un objet sous le générateur.

COldObject* CAutoGenerator::SearchObject()
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  type;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       if ( pObj->GetLock() )  continue;

       type = pObj->GetType();
       if ( type == OBJECT_GENERATOR )  continue;

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);
       assert(pObj->Implements(ObjectInterfaceType::Old));
       if ( dist <= 3.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Cherche un objet sous le générateur.

COldObject* CAutoGenerator::SearchObject(ObjectType type)
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       if ( pObj->GetLock() )  continue;
       if ( pObj->GetType() != type )  continue;

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);
       assert(pObj->Implements(ObjectInterfaceType::Old));
       if ( dist <= 3.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Compte un objet partout.

int CAutoGenerator::CountObject(ObjectType type)
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   int         i, total;

   total = 0;
   cPos = m_object->GetPosition();

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       if ( pObj->GetLock() )  continue;
       if ( pObj->GetType() != type )  continue;

       total ++;
   }

   return total;
}

// Crée un objet sous le générateur.

bool CAutoGenerator::CreateObject(ObjectType type)
{
   Math::Vector       pos;
   float           angle;
   COldObject*        bot;

   pos = m_object->GetPartPosition(0);
   angle = m_object->GetPartRotationY(0);

   bot = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type)); // TODO (krzys_h): bPlumb = false (?)
   bot->SetPartScale(0, 0.0f);  // minuscule
   StartAction(bot, -1);  // zoom manuel (pas géré par CMotion)
   return true;
}

// Démarre une action pour un objet.

void CAutoGenerator::StartAction(COldObject *pObj, int action)
{
   CMotion*    motion;

   motion = pObj->GetMotion();
   if ( motion == 0 )  return;

   motion->SetAction(action);
}

