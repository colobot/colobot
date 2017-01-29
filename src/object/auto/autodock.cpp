// autodock.cpp

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
#include "motion.h"
#include "motionvehicle.h"
#include "motionbot.h"
#include "auto.h"
#include "autobomb.h"
#include "physics.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autodock.h"



#define HIGHPOS        16.0f       // position haute de la grue
#define MOVESPEEDv 16.0f       // vitesse de la grue à vide
#define MOVESPEEDc 8.0f        // vitesse de la grue chargée



// Constructeur de l'objet.

CAutoDock::CAutoDock(CInstanceManager* iMan, CObject* object)
                        : CAuto(iMan, object)
{
   int     i;

   CAuto::CAuto(iMan, object);

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
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoDock::DeleteObject(BOOL bAll)
{
   FireStopUpdate(FALSE);
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoDock::Init()
{
   m_time = 0.0f;

   m_phase = ADCP_WAIT;
   m_progress = 0.0f;
   m_speed    = 1.0f/0.5f;
   m_center = m_object->RetPosition(0);
   m_currentPos = D3DVECTOR(0.0f, HIGHPOS, 0.0f);
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

BOOL CAutoDock::EventProcess(const Event &event)
{
   CObject*    fret;
   CObject*    evil;
   Character*  character;
   D3DVECTOR   pos;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;

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
               m_vehiclePos = m_vehicle->RetPosition(0);
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
           return TRUE;
       }
       pos = m_vehicle->RetPosition(0);
       if ( Length(pos, m_vehiclePos) > 0.1f )
       {
           m_phase    = ADCP_WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.2f;
           return TRUE;
       }

       if ( m_progress >= 1.0f )
       {
           if ( m_vehicle->RetFret() == 0 )
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
                   m_vehicle->SetLock(TRUE);
                   StartVehicleAction(MV_OPEN);
                   CameraBegin();
                   pos = fret->RetPosition(0);
                   m_startPos = m_currentPos;
                   m_goalPos  = pos-m_center;
                   m_goalPos.y = HIGHPOS;
                   m_heightFret = pos.y+RetObjectHeight(fret)-m_center.y;
                   m_phase    = ADCP_OUTMOVE1;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/(Length2d(m_startPos, m_goalPos)/MOVESPEEDv+0.1f);
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
                   m_vehicle->SetLock(TRUE);
                   StartVehicleAction(MV_OPEN);
                   CameraBegin();
                   pos = RetVehiclePoint(m_vehicle);
                   pos.y += m_vehicle->RetFret()->RetCharacter()->height;
                   m_heightVehicle = pos.y-m_center.y;
                   m_heightFret = RetObjectHeight(m_vehicle->RetFret());
                   m_startPos = m_currentPos;
                   m_goalPos  = pos-m_center;
                   m_goalPos.y = HIGHPOS;
                   m_phase    = ADCP_INMOVE1;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/(Length2d(m_startPos, m_goalPos)/MOVESPEEDv+0.1f);
                   SoundManip(1.0f/m_speed, 1.0f, 0.5f);
               }
           }
       }
   }

   if ( m_phase == ADCP_OUTMOVE1 )  // va vers stock ?
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightFret;
           m_phase    = ADCP_OUTDOWN1;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0));
       }
   }

   if ( m_phase == ADCP_OUTDOWN1 )
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret = SearchStockOut();
           if ( m_fret == 0 )
           {
               FireStopUpdate(FALSE);  // éteint
               m_phase    = ADCP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/0.5f;
           }
           else
           {
               m_fret->SetLock(TRUE);
               TruckObject(m_fret, TRUE);
               ArmObject(m_fret, TRUE);
//-                m_fret->SetAngle(0, D3DVECTOR(0.0f, m_vehicle->RetAngleY(0) ,0.0f));
               m_fretPos = m_fret->RetPosition(0);
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
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           pos = RetVehiclePoint(m_vehicle);
           pos.y += m_fret->RetCharacter()->height;
           m_heightVehicle = pos.y-m_center.y;
           m_startPos = m_currentPos;
           m_goalPos  = pos-m_center;
           m_goalPos.y = HIGHPOS;
           m_startAngle = NormAngle(m_fret->RetAngleY(0));
           m_goalAngle = NormAngle(m_vehicle->RetAngleY(0));
           m_phase    = ADCP_OUTMOVE2;
           m_progress = 0.0f;
           m_speed    = 1.0f/(Length2d(m_startPos, m_goalPos)/MOVESPEEDc+0.1f);
           SoundManip(1.0f/m_speed, 1.0f, 0.5f);
       }
   }

   if ( m_phase == ADCP_OUTMOVE2 )  // va vers véhicule ?
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret->SetTruck(m_vehicle);  // bot4 bras croisés
           TruckObject(m_fret, FALSE);
           m_fret->SetTruck(0);  // vraiment fait phase suivante
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightVehicle+RetObjectHeight(m_fret);
           m_phase    = ADCP_OUTDOWN2;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0));
       }
   }

   if ( m_phase == ADCP_OUTDOWN2 )
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           character = m_vehicle->RetCharacter();
           ArmObject(m_fret, FALSE);
           m_fret->SetLock(FALSE);
           m_fret->SetTruck(m_vehicle);
           m_vehicle->SetFret(m_fret);
           pos = character->posFret;
           pos.y += m_fret->RetCharacter()->height;
           m_fret->SetPosition(0, pos);
           m_fret->SetAngle(0, character->angleFret);
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
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_vehicle->SetLock(FALSE);
           StartVehicleAction(MV_CLOSE);
           CameraEnd();
           FireStopUpdate(FALSE);  // éteint
           m_phase    = ADCP_START;
           m_progress = 0.0f;
           m_speed    = 1.0f/10.0f;
       }
   }
   
   if ( m_phase == ADCP_INMOVE1 )  // va vers véhicule ?
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightVehicle+m_heightFret;
           m_phase    = ADCP_INDOWN1;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0));
       }
   }

   if ( m_phase == ADCP_INDOWN1 )
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret = m_vehicle->RetFret();
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
               m_fret->SetLock(TRUE);
               m_fret->SetTruck(0);
               m_vehicle->SetFret(0);
               TruckObject(m_fret, FALSE);
               ArmObject(m_fret, TRUE);
               m_fret->SetAngle(0, D3DVECTOR(0.0f, m_vehicle->RetAngleY(0) ,0.0f));
               m_fretPos = m_currentPos+m_center;
               m_fretPos.y -= m_heightFret;
               m_fret->SetPosition(0, m_fretPos);
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
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           if ( m_fret == 0 )
           {
               m_vehicle->SetLock(FALSE);
               StartVehicleAction(MV_CLOSE);
               CameraEnd();
               FireStopUpdate(FALSE);  // éteint
               m_phase    = ADCP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/5.0f;
           }
           else
           {
               TruckObject(m_fret, TRUE);
               SearchFreePos(pos);
               m_startPos = m_currentPos;
               m_goalPos  = pos-m_center;
               m_goalPos.y = HIGHPOS;
               m_startAngle = NormAngle(m_vehicle->RetAngleY(0));
               m_goalAngle = PI*0.5f;  // dépose de face
               m_phase    = ADCP_INMOVE2;
               m_progress = 0.0f;
               m_speed    = 1.0f/(Length2d(m_startPos, m_goalPos)/MOVESPEEDc+0.1f);
               SoundManip(1.0f/m_speed, 1.0f, 0.5f);
           }
       }
   }

   if ( m_phase == ADCP_INMOVE2 )  // va vers stock ?
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           SearchFreePos(pos);
           pos.y += m_fret->RetCharacter()->height;
           m_heightFret = pos.y+RetObjectHeight(m_fret)-m_center.y;
           m_startPos = m_currentPos;
           m_goalPos  = m_currentPos;
           m_goalPos.y = m_heightFret;
           m_phase    = ADCP_INDOWN2;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.5f;
           m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0));
       }
   }

   if ( m_phase == ADCP_INDOWN2 )
   {
       MoveDock();
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_fret->SetLock(FALSE);
           TruckObject(m_fret, FALSE);
           ArmObject(m_fret, FALSE);
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
       FireStopUpdate(TRUE);  // clignotte
       if ( m_progress >= 1.0f )
       {
           m_vehicle->SetLock(FALSE);
           StartVehicleAction(MV_CLOSE);
           CameraEnd();
           FireStopUpdate(FALSE);  // éteint
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
   
   ParticuleFrame(event.rTime);
   return TRUE;
}

// Stoppe l'automate.

BOOL CAutoDock::Abort()
{
   return TRUE;
}


// Met à jour les feux de stop.

void CAutoDock::FireStopUpdate(BOOL bLightOn)
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
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
               m_particule->DeleteParticule(m_partiStop[i]);
               m_partiStop[i] = -1;
           }
       }
       return;
   }

   mat = m_object->RetWorldMatrix(0);

   speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
   dim.x = 3.0f;
   dim.y = dim.x;

   for ( i=0 ; i<6 ; i++ )
   {
//?        if ( Mod(m_time+i*0.15f, 0.9f) > 0.2f )
       if ( Mod(m_time+i*0.15f, 0.45f) > 0.10f )
       {
           if ( m_partiStop[i] != -1 )
           {
               m_particule->DeleteParticule(m_partiStop[i]);
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
               pos = Transform(*mat, pos);
               m_partiStop[i] = m_particule->CreateParticule(pos, speed,
                                                             dim, PARTISELR,
                                                             1.0f, 0.0f);
           }
       }
   }
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoDock::RetError()
{
   return ERR_OK;
}


// Déplace la grue.

void CAutoDock::MoveDock()
{
   D3DVECTOR   pos;
   float       progress, bounce, angle;

   bounce = progress = Norm(m_progress);

   if ( m_goalPos.y < m_startPos.y )  // descend ?
   {
       bounce = Bounce(Norm(progress*1.2f));
   }
   m_currentPos.y = m_startPos.y+(m_goalPos.y-m_startPos.y)*bounce;

   pos.x =  0.0f;
   pos.y = 20.0f;
   pos.z = m_startPos.z+(m_goalPos.z-m_startPos.z)*progress;
   m_object->SetPosition(1, pos);
   m_currentPos.z = pos.z;

   pos.x = m_startPos.x+(m_goalPos.x-m_startPos.x)*progress;
   pos.y = 0.0f;
   pos.z = 0.0f;
   m_object->SetPosition(2, pos);
   m_currentPos.x = pos.x;

   pos.x = 0.0f;
   pos.y = (m_currentPos.y-14.0f)*3.0f/10.0f;
   pos.z = 0.0f;
   m_object->SetPosition(3, pos);

   pos.x = 0.0f;
   pos.y = (m_currentPos.y-14.0f)*6.0f/10.0f;
   pos.z = 0.0f;
   m_object->SetPosition(4, pos);

   pos.x = 0.0f;
   pos.y = m_currentPos.y-14.0f;
   pos.z = 0.0f;
   m_object->SetPosition(5, pos);

   if ( m_fret != 0 )
   {
       m_fret->SetPosition(0, m_fretPos+m_currentPos-m_fretOffset);

       if ( m_phase == ADCP_OUTMOVE2 ||
            m_phase == ADCP_INMOVE2  )
       {
           angle = m_startAngle+(m_goalAngle-m_startAngle)*m_progress;
           m_fret->SetAngleY(0, angle);
           m_object->SetAngleY(2, angle);
       }
   }
}

// Cherche un méchant proche.

CObject* CAutoDock::SearchEvil()
{
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_center;

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       oType = pObj->RetType();

       if ( oType != OBJECT_EVIL1 &&
            oType != OBJECT_EVIL2 &&
            oType != OBJECT_EVIL3 &&
            oType != OBJECT_EVIL4 &&
            oType != OBJECT_EVIL5 )  continue;

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 40.0f )  return pObj;
   }

   return 0;
}

// Cherche un véhicule sur le quai.

CObject* CAutoDock::SearchVehicle()
{
   CObject*    pObj;
   CPhysics*   physics;
   D3DVECTOR   cPos, oPos, speed, zoom;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_center;

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetGhost() )  continue;

       oType = pObj->RetType();

       if ( oType != OBJECT_CAR )  continue;

       zoom = pObj->RetZoom(0);
       if ( zoom.x != 1.0f ||
            zoom.y != 1.0f ||
            zoom.z != 1.0f )  continue;

       physics = pObj->RetPhysics();
       if ( physics != 0 )
       {
           speed = physics->RetLinMotion(MO_REASPEED);
           if ( speed.x > 0.01f )  continue;  // véhicule en mouvement ?
       }

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 8.0f )  return pObj;
   }

   return 0;
}

// Cherche un objet dans le stock pour le sortir.

CObject* CAutoDock::SearchStockOut()
{
   D3DMATRIX*  mat;
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   mat = m_object->RetWorldMatrix(0);
   cPos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 27.0f));

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       oType = pObj->RetType();

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

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 16.0f )  return pObj;
   }

   return 0;
}

// Cherche la hauteur d'un objet.

float CAutoDock::RetObjectHeight(CObject *pObj)
{
   ObjectType  oType;

   oType = pObj->RetType();
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

D3DVECTOR CAutoDock::RetVehiclePoint(CObject *pObj)
{
   Character*  character;
   D3DMATRIX*  mat;
   D3DVECTOR   pos;

   character = pObj->RetCharacter();
   mat = pObj->RetWorldMatrix(0);
   pos = Transform(*mat, character->posFret);

   return pos;
}

// Cherche une position libre ou déposer un objet.

BOOL CAutoDock::SearchFreePos(D3DVECTOR &pos)
{
   D3DMATRIX*  mat;
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType;
   BOOL        bFree[4*4];
   int         i, x, y;

   for ( i=0 ; i<4*4 ; i++ )  bFree[i] = TRUE;

   mat = m_object->RetWorldMatrix(0);
   cPos = Transform(*mat, D3DVECTOR(-13.0f, 0.0f, 14.0f));

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       oType = pObj->RetType();

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

       oPos = pObj->RetPosition(0)-cPos;

       if ( oPos.x < 0.0f || oPos.x >= 6.5f*4.0f ||
            oPos.z < 0.0f || oPos.z >= 6.5f*4.0f )  continue;

       x = (int)(oPos.x/6.5f);
       y = (int)(oPos.z/6.5f);
       bFree[x+y*4] = FALSE;
   }

   for ( i=0 ; i<4*4 ; i++ )
   {
       if ( bFree[i] )
       {
           cPos.x += 6.5f*(i%4)+6.5f*0.5f;
           cPos.z += 6.5f*(i/4)+6.5f*0.5f;
           pos = cPos;
           return TRUE;
       }
   }

   return FALSE;
}

// Fait évoluer les particules.

void CAutoDock::ParticuleFrame(float rTime)
{
   CPhysics*   physics;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   int         i;

   // Choc lorsque la grue arrive dans la voiture.
   if ( (m_phase == ADCP_OUTDOWN2 ||
         m_phase == ADCP_INDOWN1  ) && m_progress >= 0.3f/1.2f )
   {
       if ( m_lastEffect+5.0f <= m_time )
       {
           m_lastEffect = m_time;

           physics = m_vehicle->RetPhysics();
           if ( physics != 0 )
           {
               physics->SuspForce(2, -1.0f, -1.0f, 100.0f);
               physics->FFBCrash(1.0f, 0.5f, 1.0f);
           }

           m_camera->StartEffect(CE_SHOT, m_center, 1.0f);
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

   if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) > m_time )  return;
   m_lastParticule = m_time;

   if ( m_phase == ADCP_OUTUP1   ||
        m_phase == ADCP_OUTMOVE2 ||
        m_phase == ADCP_OUTDOWN2 ||
        m_phase == ADCP_INUP1    ||
        m_phase == ADCP_INMOVE2  ||
        m_phase == ADCP_INDOWN2  )
   {
       pos  = m_object->RetPosition(0);
       pos += m_object->RetPosition(1);
       pos += m_object->RetPosition(2);
       pos += m_object->RetPosition(5);
       pos.y -= 7.0f;

       for ( i=0 ; i<5 ; i++ )
       {
           speed.x = (Rand()-0.5f)*20.0f;
           speed.z = (Rand()-0.5f)*20.0f;
           speed.y = 5.0f+Rand()*10.0f;
           dim.x = 0.8f;
           dim.y = 0.8f;
           m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 1.0f, 40.0f);
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
       pos  = m_object->RetPosition(0);
       pos += m_object->RetPosition(1);
       pos += m_object->RetPosition(2);
       pos += m_object->RetPosition(5);
       pos.y -= 7.0f;
       pos.x += (Rand()-0.5f)*3.0f;
       pos.z += (Rand()-0.5f)*3.0f;
       speed.x = (Rand()-0.5f)*6.0f;
       speed.z = (Rand()-0.5f)*6.0f;
       speed.y = 2.5f+Rand()*5.0f;
       dim.x = Rand()*2.0f+1.2f;
       dim.y = dim.x;
       m_particule->CreateParticule(pos, speed, dim, PARTIVAPOR, 3.0f);
   }
}

// Fait entendre le son du bras manipulateur.

void CAutoDock::SoundManip(float time, float amplitude, float frequency)
{
   int     i;

   i = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.3f*frequency, TRUE);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
   m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}

// Fait entendre le son de l'électro-aimant.

void CAutoDock::StartBzzz()
{
   if ( m_channelSound != -1 )  return;
   m_channelSound = m_sound->Play(SOUND_NUCLEAR, m_center, 0.5f, 1.0f, TRUE);
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

   motion = m_vehicle->RetMotion();
   if ( motion == 0 )  return;

   delay = 2.0f;
   if ( action == MV_CLOSE )  delay = 2.0f;
   motion->SetAction(action, delay);
}

// Démarre une action "soulevé" pour l'objet pris par la grue.

void CAutoDock::TruckObject(CObject *pObj, BOOL bTake)
{
   ObjectType  type;
   CMotion*    motion;
   CAuto*      automat;

   type = pObj->RetType();

   if ( type == OBJECT_BOT1   ||
        type == OBJECT_BOT2   ||
        type == OBJECT_BOT3   ||
        type == OBJECT_BOT4   ||
        type == OBJECT_BOT5   ||
        type == OBJECT_WALKER )
   {
       motion = pObj->RetMotion();
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

   if ( type == OBJECT_BOMB )
   {
       automat = pObj->RetAuto();
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
}

// Modifie l'objet lorsque le bras de la grue arrive dessus.

void CAutoDock::ArmObject(CObject *pObj, BOOL bTake)
{
   ObjectType  type;
   float       angle;

   type = pObj->RetType();

   if ( type == OBJECT_FIRE )
   {
       if ( bTake )  angle = 0.0f;  // canon horizontal
       else          angle = 20.0f*PI/180.0f;
       pObj->SetAngleZ(2, angle);
   }
}


// Début du cadrage pour la caméra.

void CAutoDock::CameraBegin()
{
#if 0
   D3DMATRIX*  mat;
   D3DVECTOR   pos, vPos;
   float       angle;

   mat = m_object->RetWorldMatrix(0);
   pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 30.0f));
   vPos = m_vehicle->RetPosition(0);
   angle = RotateAngle(pos.x-vPos.x, vPos.z-pos.z);
   angle -= m_vehicle->RetAngleY(0);
   angle = NormAngle(angle);
   if ( angle < PI )  angle -= PI*0.15f;
   else               angle += PI*0.15f;
   m_camera->SetBackHoriz(-angle);
   m_camera->SetLockRotate(TRUE);
#else
   m_cameraType = m_camera->RetType();
   m_camera->SetObject(m_object);
   m_camera->SetType(CAMERA_BACK);
   m_camera->SetSmooth(CS_SOFT);
#endif

   m_main->SetStopwatch(FALSE);  // stoppe le chrono
}

// Fin du cadrage pour la caméra.

void CAutoDock::CameraEnd()
{
#if 0
   m_camera->SetBackHoriz(0.0f);
   m_camera->SetLockRotate(FALSE);
   m_camera->ResetLockRotate();
#else
   m_camera->SetObject(m_vehicle);
   m_camera->SetType(m_cameraType);
   m_camera->SetSmooth(CS_NORM);
#endif

   m_main->SetStopwatch(TRUE);  // redémarre le chrono
}

