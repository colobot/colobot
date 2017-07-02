// autodock.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motioncar.h"
#include "object/motion/buzzingcars/motionbot.h"
#include "object/auto/auto.h"
//#include "object/auto/buzzingcars/building/autobomb.h"
#include "physics/physics.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/building/autodock.h"
#include "object/object_manager.h"



#define HIGHPOS        16.0f       // position haute de la grue
#define MOVESPEEDv 16.0f       // vitesse de la grue à vide
#define MOVESPEEDc 8.0f        // vitesse de la grue chargée



// Constructeur de l'objet.

CAutoDock::CAutoDock(COldObject* object)
                        : CAuto(object)
{
   int     i;

   for ( i=0 ; i<6 ; i++ )
   {
       m_partiStop[i] = -1;
   }

   Init();
}

// Destructeur de l'objet.

CAutoDock::~CAutoDock()
{
   StopBzzz();
}


// Détruit l'objet.

void CAutoDock::DeleteObject(bool bAll)
{
   FireStopUpdate(false);
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoDock::Init()
{
   m_time = 0.0f;

   m_phase = ADCP_WAIT;
   m_progress = 0.0f;
   m_speed    = 1.0f/0.5f;
   m_center = m_object->GetPartPosition(0);
   m_currentPos = Math::Vector(0.0f, HIGHPOS, 0.0f);
   m_fret = 0;
   m_lastParticule = 0.0f;
   m_lastEffect = 0.0f;
   m_channelSound = -1;
}


// Démarre l'objet.

void CAutoDock::Start(int param)
{
}


// Gestion d'un événement.

bool CAutoDock::EventProcess(const Event &event)
{
   COldObject*    fret;
   COldObject*    evil;
   Character*  character;
   Math::Vector   pos;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;

   m_progress += event.rTime*m_speed;

   if ( m_phase == ADCP_WAIT )
   {
       if ( m_progress >= 1.0f )
       {
           evil = SearchEvil();
           m_vehicle = SearchVehicle();
           if ( evil != 0 || m_vehicle == 0 )
           {
               m_phase    = ADCP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.5f;
           }
           else
           {
               m_vehiclePos = m_vehicle->GetPartPosition(0);
               m_phase    = ADCP_STOPCHECK;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.2f;
           }
       }
   }

   if ( m_phase == ADCP_STOPCHECK )
   {
       m_vehicle = SearchVehicle();
       if ( m_vehicle == 0 )
       {
           m_phase    = ADCP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.5f;
           return true;
       }
       pos = m_vehicle->GetPartPosition(0);
       if ( Math::Distance(pos, m_vehiclePos) > 0.1f )
       {
           m_phase    = ADCP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.2f;
           return true;
       }

       if ( m_progress >= 1.0f )
       {
           if ( m_vehicle->GetCargo() == 0 )
           {
               fret = SearchStockOut();
               if ( fret == 0 )
               {
                   m_phase    = ADCP_WAIT;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/2.0f;
               }
               else
               {
                   m_vehicle->SetLock(true);
                   StartVehicleAction(MV_OPEN);
                   CameraBegin();
                   pos = fret->GetPartPosition(0);
                   m_startPos = m_currentPos;
                   m_goalPos  = pos-m_center;
                   m_goalPos.y = HIGHPOS;
                   m_heightFret = pos.y+GetObjectHeight(fret)-m_center.y;
                   m_phase    = ADCP_OUTMOVE1;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/(Math::DistanceProjected(m_startPos, m_goalPos)/MOVESPEEDv+0.1f);
                   SoundManip(1.0f/m_speed, 1.0f, 0.5f);
               }
           }
           else
           {
               if ( !SearchFreePos(pos) )
               {
                   m_phase    = ADCP_WAIT;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/2.0f;
               }
               else
               {
                   m_vehicle->SetLock(true);
                   StartVehicleAction(MV_OPEN);
                   CameraBegin();
                   pos = GetVehiclePoint(m_vehicle);
                   pos.y += m_vehicle->GetCargo()->GetCharacter()->height;
                   m_heightVehicle = pos.y-m_center.y;
                   m_heightFret = GetObjectHeight(dynamic_cast<COldObject*>(m_vehicle->GetCargo()));
                   m_startPos = m_currentPos;
                   m_goalPos  = pos-m_center;
                   m_goalPos.y = HIGHPOS;
                   m_phase    = ADCP_INMOVE1;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/(Math::DistanceProjected(m_startPos, m_goalPos)/MOVESPEEDv+0.1f);
                   SoundManip(1.0f/m_speed, 1.0f, 0.5f);
               }
           }
       }
   }

   if ( m_phase == ADCP_OUTMOVE1 )  // va vers stock ?
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightFret;
           m_phase    = ADCP_OUTDOWN1;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->GetPartPosition(0));
       }
   }

   if ( m_phase == ADCP_OUTDOWN1 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret = SearchStockOut();
           if ( m_fret == 0 )
           {
               FireStopUpdate(false);  // éteint
               m_phase    = ADCP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.5f;
           }
           else
           {
               m_fret->SetLock(true);
               TruckObject(m_fret, true);
               ArmObject(m_fret, true);
//-                m_fret->SetPartRotation(0, Math::Vector(0.0f, m_vehicle->GetPartRotationY(0) ,0.0f));
               m_fretPos = m_fret->GetPartPosition(0);
               m_fretOffset = m_currentPos;
               m_startPos = m_currentPos;
               m_goalPos  = m_currentPos;
               m_goalPos.y = HIGHPOS;
               m_phase    = ADCP_OUTUP1;
               m_progress = 0.0f;
               m_speed    = 1.0f/1.0f;
               SoundManip(1.0f, 1.0f, 1.0f);
           }
       }
   }

   if ( m_phase == ADCP_OUTUP1 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           pos = GetVehiclePoint(m_vehicle);
           pos.y += m_fret->GetCharacter()->height;
           m_heightVehicle = pos.y-m_center.y;
           m_startPos = m_currentPos;
           m_goalPos  = pos-m_center;
           m_goalPos.y = HIGHPOS;
           m_startAngle = Math::NormAngle(m_fret->GetPartRotationY(0));
           m_goalAngle = Math::NormAngle(m_vehicle->GetPartRotationY(0));
           m_phase    = ADCP_OUTMOVE2;
           m_progress = 0.0f;
           m_speed    = 1.0f/(Math::DistanceProjected(m_startPos, m_goalPos)/MOVESPEEDc+0.1f);
           SoundManip(1.0f/m_speed, 1.0f, 0.5f);
       }
   }

   if ( m_phase == ADCP_OUTMOVE2 )  // va vers véhicule ?
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret->SetTransporter(m_vehicle);  // bot4 bras croisés
           TruckObject(m_fret, false);
           m_fret->SetTransporter(0);  // vraiment fait phase suivante
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightVehicle+GetObjectHeight(m_fret);
           m_phase    = ADCP_OUTDOWN2;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->GetPartPosition(0));
       }
   }

   if ( m_phase == ADCP_OUTDOWN2 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           character = m_vehicle->GetCharacter();
           ArmObject(m_fret, false);
           m_fret->SetLock(false);
           m_fret->SetTransporter(m_vehicle);
           m_vehicle->SetCargo(m_fret);
           pos = character->posFret;
           pos.y += m_fret->GetCharacter()->height;
           m_fret->SetPartPosition(0, pos);
           m_fret->SetPartRotation(0, character->angleFret);
           m_fret = 0;
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = HIGHPOS;
           m_phase    = ADCP_OUTUP2;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.0f;
           SoundManip(1.0f, 1.0f, 1.0f);
       }
   }

   if ( m_phase == ADCP_OUTUP2 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_vehicle->SetLock(false);
           StartVehicleAction(MV_CLOSE);
           CameraEnd();
           FireStopUpdate(false);  // éteint
           m_phase    = ADCP_START;
           m_progress = 0.0f;
           m_speed    = 1.0f/10.0f;
       }
   }
   
   if ( m_phase == ADCP_INMOVE1 )  // va vers véhicule ?
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightVehicle+m_heightFret;
           m_phase    = ADCP_INDOWN1;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->GetPartPosition(0));
       }
   }

   if ( m_phase == ADCP_INDOWN1 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret = dynamic_cast<COldObject*>(m_vehicle->GetCargo());
           if ( m_fret == 0 )
           {
               m_startPos = m_currentPos;
               m_goalPos  = m_currentPos;
               m_goalPos.y = HIGHPOS;
               m_phase    = ADCP_INUP1;
               m_progress = 0.0f;
               m_speed    = 1.0f/1.0f;
               SoundManip(1.0f, 1.0f, 1.0f);
           }
           else
           {
               m_fret->SetLock(true);
               m_fret->SetTransporter(0);
               m_vehicle->SetCargo(0);
               TruckObject(m_fret, false);
               ArmObject(m_fret, true);
               m_fret->SetPartRotation(0, Math::Vector(0.0f, m_vehicle->GetPartRotationY(0) ,0.0f));
               m_fretPos = m_currentPos+m_center;
               m_fretPos.y -= m_heightFret;
               m_fret->SetPartPosition(0, m_fretPos);
               m_fretOffset = m_currentPos;
               m_startPos = m_currentPos;
               m_goalPos  = m_currentPos;
               m_goalPos.y = HIGHPOS;
               m_phase    = ADCP_INUP1;
               m_progress = 0.0f;
               m_speed    = 1.0f/1.0f;
               SoundManip(1.0f, 1.0f, 1.0f);
           }
       }
   }

   if ( m_phase == ADCP_INUP1 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           if ( m_fret == 0 )
           {
               m_vehicle->SetLock(false);
               StartVehicleAction(MV_CLOSE);
               CameraEnd();
               FireStopUpdate(false);  // éteint
               m_phase    = ADCP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/5.0f;
           }
           else
           {
               TruckObject(m_fret, true);
               SearchFreePos(pos);
               m_startPos = m_currentPos;
               m_goalPos  = pos-m_center;
               m_goalPos.y = HIGHPOS;
               m_startAngle = Math::NormAngle(m_vehicle->GetPartRotationY(0));
               m_goalAngle = Math::PI*0.5f;  // dépose de face
               m_phase    = ADCP_INMOVE2;
               m_progress = 0.0f;
               m_speed    = 1.0f/(Math::DistanceProjected(m_startPos, m_goalPos)/MOVESPEEDc+0.1f);
               SoundManip(1.0f/m_speed, 1.0f, 0.5f);
           }
       }
   }

   if ( m_phase == ADCP_INMOVE2 )  // va vers stock ?
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           SearchFreePos(pos);
           pos.y += m_fret->GetCharacter()->height;
           m_heightFret = pos.y+GetObjectHeight(m_fret)-m_center.y;
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightFret;
           m_phase    = ADCP_INDOWN2;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->GetPartPosition(0));
       }
   }

   if ( m_phase == ADCP_INDOWN2 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret->SetLock(false);
           TruckObject(m_fret, false);
           ArmObject(m_fret, false);
           m_fret = 0;
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = HIGHPOS;
           m_phase    = ADCP_INUP2;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.0f;
           SoundManip(1.0f, 1.0f, 1.0f);
       }
   }

   if ( m_phase == ADCP_INUP2 )
   {
       MoveDock();
       FireStopUpdate(true);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_vehicle->SetLock(false);
           StartVehicleAction(MV_CLOSE);
           CameraEnd();
           FireStopUpdate(false);  // éteint
           m_phase    = ADCP_START;
           m_progress = 0.0f;
           m_speed    = 1.0f/10.0f;
       }
   }

   if ( m_phase == ADCP_START )
   {
       if ( SearchVehicle() == 0 || m_progress >= 1.0f )
       {
           m_phase    = ADCP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.5f;
       }
   }
   
   ParticleFrame(event.rTime);
   return true;
}

// Stoppe l'automate.

bool CAutoDock::Abort()
{
   return true;
}


// Met à jour les feux de stop.

void CAutoDock::FireStopUpdate(bool bLightOn)
{
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   int         i;

   static float listpos[3*6] =
   {
       -15.0f, 10.0f, -14.0f,
       -15.0f, 10.0f,  10.0f,
       -15.0f, 10.0f,  40.0f,
        15.0f, 10.0f,  40.0f,
        15.0f, 10.0f,  10.0f,
        15.0f, 10.0f, -14.0f,
   };

   if ( !bLightOn )  // éteint ?
   {
       for ( i=0 ; i<6 ; i++ )
       {
           if ( m_partiStop[i] != -1 )
           {
               m_particle->DeleteParticle(m_partiStop[i]);
               m_partiStop[i] = -1;
           }
       }
       return;
   }

   mat = m_object->GetWorldMatrix(0);

   speed = Math::Vector(0.0f, 0.0f, 0.0f);
   dim.x = 3.0f;
   dim.y = dim.x;

   for ( i=0 ; i<6 ; i++ )
   {
//?        if ( Math::Mod(m_time+i*0.15f, 0.9f) > 0.2f )
       if ( Math::Mod(m_time+i*0.15f, 0.45f) > 0.10f )
       {
           if ( m_partiStop[i] != -1 )
           {
               m_particle->DeleteParticle(m_partiStop[i]);
               m_partiStop[i] = -1;
           }
       }
       else
       {
           if ( m_partiStop[i] == -1 )
           {
               pos.x = listpos[i*3+0];
               pos.y = listpos[i*3+1];
               pos.z = listpos[i*3+2];
               pos = Math::Transform(*mat, pos);
               m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                             dim, Gfx::PARTISELR,
                                                             1.0f, 0.0f);
           }
       }
   }
}


// Getourne une erreur liée à l'état de l'automate.

Error CAutoDock::GetError()
{
   return ERR_OK;
}


// Déplace la grue.

void CAutoDock::MoveDock()
{
   Math::Vector   pos;
   float       progress, bounce, angle;

   bounce = progress = Math::Norm(m_progress);

   if ( m_goalPos.y < m_startPos.y )  // descend ?
   {
       bounce = Math::Bounce(Math::Norm(progress*1.2f));
   }
   m_currentPos.y = m_startPos.y+(m_goalPos.y-m_startPos.y)*bounce;

   pos.x =  0.0f;
   pos.y = 20.0f;
   pos.z = m_startPos.z+(m_goalPos.z-m_startPos.z)*progress;
   m_object->SetPartPosition(1, pos);
   m_currentPos.z = pos.z;

   pos.x = m_startPos.x+(m_goalPos.x-m_startPos.x)*progress;
   pos.y = 0.0f;
   pos.z = 0.0f;
   m_object->SetPartPosition(2, pos);
   m_currentPos.x = pos.x;

   pos.x = 0.0f;
   pos.y = (m_currentPos.y-14.0f)*3.0f/10.0f;
   pos.z = 0.0f;
   m_object->SetPartPosition(3, pos);

   pos.x = 0.0f;
   pos.y = (m_currentPos.y-14.0f)*6.0f/10.0f;
   pos.z = 0.0f;
   m_object->SetPartPosition(4, pos);

   pos.x = 0.0f;
   pos.y = m_currentPos.y-14.0f;
   pos.z = 0.0f;
   m_object->SetPartPosition(5, pos);

   if ( m_fret != 0 )
   {
       m_fret->SetPartPosition(0, m_fretPos+m_currentPos-m_fretOffset);

       if ( m_phase == ADCP_OUTMOVE2 ||
            m_phase == ADCP_INMOVE2  )
       {
           angle = m_startAngle+(m_goalAngle-m_startAngle)*m_progress;
           m_fret->SetPartRotationY(0, angle);
           m_object->SetPartRotationY(2, angle);
       }
   }
}

// Cherche un méchant proche.

COldObject* CAutoDock::SearchEvil()
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_center;

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       oType = pObj->GetType();

       if ( oType != OBJECT_EVIL1 &&
            oType != OBJECT_EVIL2 &&
            oType != OBJECT_EVIL3 &&
            oType != OBJECT_EVIL4 &&
            oType != OBJECT_EVIL5 )  continue;

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       assert(pObj->Implements(ObjectInterfaceType::Old));
       if ( dist <= 40.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Cherche un véhicule sur le quai.

COldObject* CAutoDock::SearchVehicle()
{
   COldObject*    pObj;
   CPhysics*   physics;
   Math::Vector   cPos, oPos, speed, zoom;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_center;

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
//TODO (krzys_h):       if ( pObj->GetGhost() )  continue;

       oType = pObj->GetType();

       if ( oType != OBJECT_CAR )  continue;
       assert(pObj->Implements(ObjectInterfaceType::Old));

       zoom = pObj->GetScale();
       if ( zoom.x != 1.0f ||
            zoom.y != 1.0f ||
            zoom.z != 1.0f )  continue;

       physics = dynamic_cast<COldObject*>(pObj)->GetPhysics();
       assert ( physics != 0 );
       speed = physics->GetLinMotion(MO_REASPEED);
       if ( speed.x > 0.01f )  continue;  // véhicule en mouvement ?

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       if ( dist <= 8.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Cherche un objet dans le stock pour le sortir.

COldObject* CAutoDock::SearchStockOut()
{
   Math::Matrix*  mat;
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   mat = m_object->GetWorldMatrix(0);
   cPos = Math::Transform(*mat, Math::Vector(0.0f, 0.0f, 27.0f));

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       oType = pObj->GetType();

       if ( oType != OBJECT_STONE   &&
            oType != OBJECT_URANIUM &&
            oType != OBJECT_METAL   &&
            oType != OBJECT_BARREL  &&
            oType != OBJECT_BARRELa &&
            oType != OBJECT_ATOMIC  &&
            oType != OBJECT_BULLET  &&
            oType != OBJECT_BBOX    &&
            oType != OBJECT_TNT     &&
            oType != OBJECT_BOMB    &&
            oType != OBJECT_BOT1    &&
            oType != OBJECT_BOT2    &&
            oType != OBJECT_BOT3    &&
            oType != OBJECT_BOT4    &&
            oType != OBJECT_BOT5    &&
            oType != OBJECT_WALKER  &&
            oType != OBJECT_FIRE    &&
            oType != OBJECT_HELICO  &&
            oType != OBJECT_COMPASS &&
            oType != OBJECT_BLITZER &&
            oType != OBJECT_HOOK    &&
            oType != OBJECT_AQUA    )  continue;

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       assert(pObj->Implements(ObjectInterfaceType::Old));
       if ( dist <= 16.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Cherche la hauteur d'un objet.

float CAutoDock::GetObjectHeight(COldObject *pObj)
{
   ObjectType  oType;

   oType = pObj->GetType();
   if ( oType == OBJECT_BOT1    )  return 3.2f;
   if ( oType == OBJECT_BOT2    )  return 5.7f;
   if ( oType == OBJECT_BOT3    )  return 3.8f;
   if ( oType == OBJECT_BOT4    )  return 4.0f;
   if ( oType == OBJECT_BOT5    )  return 6.0f;
   if ( oType == OBJECT_WALKER  )  return 4.0f;
   if ( oType == OBJECT_BARREL  )  return 5.0f;
   if ( oType == OBJECT_BARRELa )  return 5.0f;
   if ( oType == OBJECT_BOMB    )  return 4.3f;
   if ( oType == OBJECT_FIRE    )  return 7.1f;
   if ( oType == OBJECT_HELICO  )  return 7.0f;
   if ( oType == OBJECT_COMPASS )  return 6.0f;
   if ( oType == OBJECT_BLITZER )  return 8.5f;
   if ( oType == OBJECT_HOOK    )  return 6.4f;
   if ( oType == OBJECT_AQUA    )  return 4.0f;
   return 2.0f;
}

// Cherche le point où mettre du fret sur un véhicule.

Math::Vector CAutoDock::GetVehiclePoint(COldObject *pObj)
{
   Character*  character;
   Math::Matrix*  mat;
   Math::Vector   pos;

   character = pObj->GetCharacter();
   mat = pObj->GetWorldMatrix(0);
   pos = Math::Transform(*mat, character->posFret);

   return pos;
}

// Cherche une position libre ou déposer un objet.

bool CAutoDock::SearchFreePos(Math::Vector &pos)
{
   Math::Matrix*  mat;
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   bool        bFree[4*4];
   int         i, x, y;

   for ( i=0 ; i<4*4 ; i++ )  bFree[i] = true;

   mat = m_object->GetWorldMatrix(0);
   cPos = Math::Transform(*mat, Math::Vector(-13.0f, 0.0f, 14.0f));

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {

       oType = pObj->GetType();

       if ( oType != OBJECT_STONE   &&
            oType != OBJECT_URANIUM &&
            oType != OBJECT_METAL   &&
            oType != OBJECT_BARREL  &&
            oType != OBJECT_BARRELa &&
            oType != OBJECT_ATOMIC  &&
            oType != OBJECT_BULLET  &&
            oType != OBJECT_BBOX    &&
            oType != OBJECT_TNT     &&
            oType != OBJECT_BOT1    &&
            oType != OBJECT_BOT2    &&
            oType != OBJECT_BOT3    &&
            oType != OBJECT_BOT4    &&
            oType != OBJECT_BOT5    &&
            oType != OBJECT_WALKER  &&
            oType != OBJECT_FIRE    &&
            oType != OBJECT_HELICO  &&
            oType != OBJECT_COMPASS &&
            oType != OBJECT_BLITZER &&
            oType != OBJECT_HOOK    &&
            oType != OBJECT_AQUA    )  continue;

       oPos = pObj->GetPosition()-cPos;

       if ( oPos.x < 0.0f || oPos.x >= 6.5f*4.0f ||
            oPos.z < 0.0f || oPos.z >= 6.5f*4.0f )  continue;

       x = (int)(oPos.x/6.5f);
       y = (int)(oPos.z/6.5f);
       bFree[x+y*4] = false;
   }

   for ( i=0 ; i<4*4 ; i++ )
   {
       if ( bFree[i] )
       {
           cPos.x += 6.5f*(i%4)+6.5f*0.5f;
           cPos.z += 6.5f*(i/4)+6.5f*0.5f;
           pos = cPos;
           return true;
       }
   }

   return false;
}

// Fait évoluer les particules.

void CAutoDock::ParticleFrame(float rTime)
{
   CPhysics*   physics;
   Math::Vector   pos, speed;
   Math::Point      dim;
   int         i;

   // Choc lorsque la grue arrive dans la voiture.
   if ( (m_phase == ADCP_OUTDOWN2 ||
         m_phase == ADCP_INDOWN1  ) && m_progress >= 0.3f/1.2f )
   {
       if ( m_lastEffect+5.0f <= m_time )
       {
           m_lastEffect = m_time;

           physics = m_vehicle->GetPhysics();
           if ( physics != 0 )
           {
               physics->SuspForce(2, -1.0f, -1.0f, 100.0f);
               physics->FFBCrash(1.0f, 0.5f, 1.0f);
           }

           m_camera->StartEffect(Gfx::CAM_EFFECT_SHOT, m_center, 1.0f);
           m_sound->Play(SOUND_TAKE, m_center);
       }
   }

   if ( (m_phase == ADCP_OUTDOWN1 ||
         m_phase == ADCP_INDOWN2  ) && m_progress >= 0.2f )
   {
       if ( m_lastEffect+5.0f <= m_time )
       {
           m_lastEffect = m_time;
           m_sound->Play(SOUND_TAKE, m_center);
       }
   }

   if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) > m_time )  return;
   m_lastParticule = m_time;

   if ( m_phase == ADCP_OUTUP1   ||
        m_phase == ADCP_OUTMOVE2 ||
        m_phase == ADCP_OUTDOWN2 ||
        m_phase == ADCP_INUP1    ||
        m_phase == ADCP_INMOVE2  ||
        m_phase == ADCP_INDOWN2  )
   {
       pos  = m_object->GetPartPosition(0);
       pos += m_object->GetPartPosition(1);
       pos += m_object->GetPartPosition(2);
       pos += m_object->GetPartPosition(5);
       pos.y -= 7.0f;

       for ( i=0 ; i<5 ; i++ )
       {
           speed.x = (Math::Rand()-0.5f)*20.0f;
           speed.z = (Math::Rand()-0.5f)*20.0f;
           speed.y = 5.0f+Math::Rand()*10.0f;
           dim.x = 0.8f;
           dim.y = 0.8f;
           m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 40.0f);
       }

       StartBzzz();

       if ( m_channelSound != -1 )
       {
           m_sound->Position(m_channelSound, pos);
       }
   }
   else
   {
       StopBzzz();
   }

   if ( m_progress < 0.5f &&
       (m_phase == ADCP_OUTDOWN1 ||
        m_phase == ADCP_OUTDOWN2 ||
        m_phase == ADCP_INDOWN1  ||
        m_phase == ADCP_INDOWN2  ) )
   {
       pos  = m_object->GetPartPosition(0);
       pos += m_object->GetPartPosition(1);
       pos += m_object->GetPartPosition(2);
       pos += m_object->GetPartPosition(5);
       pos.y -= 7.0f;
       pos.x += (Math::Rand()-0.5f)*3.0f;
       pos.z += (Math::Rand()-0.5f)*3.0f;
       speed.x = (Math::Rand()-0.5f)*6.0f;
       speed.z = (Math::Rand()-0.5f)*6.0f;
       speed.y = 2.5f+Math::Rand()*5.0f;
       dim.x = Math::Rand()*2.0f+1.2f;
       dim.y = dim.x;
       m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIVAPOR, 3.0f);
   }
}

// Fait entendre le son du bras manipulateur.

void CAutoDock::SoundManip(float time, float amplitude, float frequency)
{
   int     i;

   i = m_sound->Play(SOUND_MANIP, m_object->GetPartPosition(0), 0.0f, 0.3f*frequency, true);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}

// Fait entendre le son de l'électro-aimant.

void CAutoDock::StartBzzz()
{
   if ( m_channelSound != -1 )  return;
   m_channelSound = m_sound->Play(SOUND_NUCLEAR, m_center, 0.5f, 1.0f, true);
   m_sound->AddEnvelope(m_channelSound, 0.5f, 1.0f, 1.0f, SOPER_LOOP);
}

// Stoppe le son de l'électro-aimant.

void CAutoDock::StopBzzz()
{
   if ( m_channelSound == -1 )  return;
   m_sound->FlushEnvelope(m_channelSound);
   m_sound->AddEnvelope(m_channelSound, 0.5f, 1.0f, 0.1f, SOPER_STOP);
   m_channelSound = -1;
}

// Démarre une action pour le véhicule.

void CAutoDock::StartVehicleAction(int action)
{
   CMotion*    motion;
   float       delay;

   motion = m_vehicle->GetMotion();
   if ( motion == 0 )  return;

   delay = 2.0f;
   if ( action == MV_CLOSE )  delay = 2.0f;
   motion->SetAction(action, delay);
}

// Démarre une action "soulevé" pour l'objet pris par la grue.

void CAutoDock::TruckObject(COldObject *pObj, bool bTake)
{
   ObjectType  type;
   CMotion*    motion;
   CAuto*      automat;

   type = pObj->GetType();

   if ( type == OBJECT_BOT1   ||
        type == OBJECT_BOT2   ||
        type == OBJECT_BOT3   ||
        type == OBJECT_BOT4   ||
        type == OBJECT_BOT5   ||
        type == OBJECT_WALKER )
   {
       motion = pObj->GetMotion();
       if ( motion == 0 )  return;

       if ( bTake )
       {
           motion->SetAction(MB_TRUCK, 1.0f);
       }
       else
       {
           motion->SetAction(MB_WAIT, 1.0f);
       }
   }

   /* TODO (krzys_h)
   if ( type == OBJECT_BOMB )
   {
       automat = pObj->GetAuto();
       if ( automat == 0 )  return;

       if ( bTake )
       {
           automat->SetAction(AB_TRUCK, 1.0f);
           automat->SetAction(AB_BURNFLIP, 1.0f);
       }
       else
       {
           automat->SetAction(AB_STOP, 1.0f);
       }
   }
   */
}

// Math::Modifie l'objet lorsque le bras de la grue arrive dessus.

void CAutoDock::ArmObject(COldObject *pObj, bool bTake)
{
   ObjectType  type;
   float       angle;

   type = pObj->GetType();

   if ( type == OBJECT_FIRE )
   {
       if ( bTake )  angle = 0.0f;  // canon horizontal
       else          angle = 20.0f*Math::PI/180.0f;
       pObj->SetPartRotationZ(2, angle);
   }
}


// Début du cadrage pour la caméra.

void CAutoDock::CameraBegin()
{
#if 0
   Math::Matrix*  mat;
   Math::Vector   pos, vPos;
   float       angle;

   mat = m_object->GetWorldMatrix(0);
   pos = Math::Transform(*mat, Math::Vector(0.0f, 0.0f, 30.0f));
   vPos = m_vehicle->GetPartPosition(0);
   angle = Math::RotateAngle(pos.x-vPos.x, vPos.z-pos.z);
   angle -= m_vehicle->GetPartRotationY(0);
   angle = Math::NormAngle(angle);
   if ( angle < Math::PI )  angle -= Math::PI*0.15f;
   else               angle += Math::PI*0.15f;
   m_camera->SetBackHoriz(-angle);
   m_camera->SetLockRotate(true);
#else
   m_cameraType = m_camera->GetType();
   m_camera->SetControllingObject(m_object);
   m_camera->SetType(Gfx::CAM_TYPE_BACK);
   m_camera->SetSmooth(Gfx::CAM_SMOOTH_SOFT);
#endif

   m_main->SetStopwatch(false);  // stoppe le chrono
}

// Fin du cadrage pour la caméra.

void CAutoDock::CameraEnd()
{
#if 0
   m_camera->SetBackHoriz(0.0f);
   m_camera->SetLockRotate(false);
   m_camera->ResetLockRotate();
#else
   m_camera->SetControllingObject(m_vehicle);
   m_camera->SetType(m_cameraType);
   m_camera->SetSmooth(Gfx::CAM_SMOOTH_NORM);
#endif

   m_main->SetStopwatch(true);  // redémarre le chrono
}

