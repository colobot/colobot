// autodoor.cpp

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
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autodoor.h"



#define HEIGHT     -26.0f      // abaissement de la porte
#define TIME_CLOSE 1.0f        // durée fermeture
#define TIME_OPEN  5.0f        // durée ouverture



// Constructeur de l'objet.

CAutoDoor::CAutoDoor(CInstanceManager* iMan, CObject* object)
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

CAutoDoor::~CAutoDoor()
{
   CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoDoor::DeleteObject(BOOL bAll)
{
   m_phase = ADOP_WAIT;
   FireStopUpdate();
   CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoDoor::Init()
{
   m_time = 0.0f;
   m_lastParticule = 0.0f;

   m_phase    = ADOP_WAIT;
   m_progress = 0.0f;
   m_speed    = 1.0f/2.0f;

   m_initialDelay = 0.0f;
   m_counter = 0;
   m_rank = -1;
   m_cycleDelay = 15.0f;
   m_programNumber = -1;

   m_type = m_object->RetType();

   if ( m_type == OBJECT_DOOR2 ||
        m_type == OBJECT_DOOR3 )
   {
       m_speed = 1.0f/(m_cycleDelay-TIME_CLOSE-TIME_OPEN);
   }

   m_botPos = m_object->RetPosition(0);
}


// Démarre l'objet.

void CAutoDoor::Start(int param)
{
   D3DMATRIX*  mat;
   CObject*    object;
   D3DVECTOR   pos;
   float       angle, value;
   int         bot, i;

   if ( param == 2 &&  // casse le mécanisme ?
        (m_type == OBJECT_DOOR2 ||
         m_type == OBJECT_DOOR3 ) )
   {
       if ( m_phase == ADOP_BREAK ||
            m_phase == ADOP_STOP  )  return;  // déjà cassé ?

       i = m_sound->Play(SOUND_BREAK1, m_engine->RetEyePt(), 1.0f, 0.5f, TRUE);
       m_sound->AddEnvelope(i, 1.0f, 2.0f, 6.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 0.5f, 0.1f, SOPER_STOP);

       i = m_sound->Play(SOUND_WHEELb, m_engine->RetEyePt(), 0.0f, 0.2f, TRUE);
       m_sound->AddEnvelope(i, 0.5f, 1.0f, 6.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 0.2f, 1.0f, SOPER_STOP);

       m_doorStart = m_doorProgress;

       m_phase    = ADOP_BREAK;
       m_progress = 0.0f;
       m_speed    = 1.0f/6.0f;
       return;
   }

   if ( m_type == OBJECT_DOOR1 ||
        m_type == OBJECT_DOOR4 )
   {
       m_initialDelay = m_object->RetCmdLine(0);
       if ( m_initialDelay == NAN )  m_initialDelay = 0.0f;

       value = m_object->RetCmdLine(1);
       if ( value != NAN )  m_counter = (int)value;

       value = m_object->RetCmdLine(2);
       if ( value != NAN )  m_rank = (int)value;

       if ( param == 3 )  // attend une condition pour s'ouvrir ?
       {
           pos = m_object->RetPosition(1);
           pos.y = HEIGHT;
           m_object->SetPosition(1, pos);  // porte fermée
           UpdateCrashSphere();

           m_phase    = ADOP_13WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.0f;
       }
   }

   if ( m_type == OBJECT_DOOR2 ||
        m_type == OBJECT_DOOR3 )
   {
       m_cycleDelay = m_object->RetCmdLine(0);
       if ( m_cycleDelay == NAN )  m_cycleDelay = 15.0f;
       if ( m_cycleDelay < 7.0f )  m_cycleDelay = 7.0f;

       bot = (int)m_object->RetCmdLine(1);
       if ( bot == NAN )  bot = 0;

       m_programNumber = (int)m_object->RetCmdLine(2);
       if ( m_programNumber == NAN )  m_programNumber = -1;

       m_phase    = ADOP_WAIT;
       m_progress = 0.0f;
       m_speed    = 1.0f/(m_cycleDelay-TIME_CLOSE-TIME_OPEN);

       m_time = 0.0f;
   }

   // Crée la croix (invisible) sous la porte.
   if ( (m_type == OBJECT_DOOR1 || m_type == OBJECT_DOOR4) && param != 3 )
   {
       pos = m_object->RetPosition(0);
       pos.y += 4.0f;
       angle = m_object->RetAngleY(0)+PI/2.0f;
       object = new CObject(m_iMan);
       if ( !object->CreateResource(pos, angle, 1.0f, OBJECT_TARGET, TRUE) )
       {
           delete object;
       }
       else
       {
           object->SetPosition(0, pos);
           object->SetPassCounter(m_counter);
           object->SetRankCounter(m_rank);
       }
   }

   // Crée le robot s'occupant de la porte.
   if ( (m_type == OBJECT_DOOR2 ||
         m_type == OBJECT_DOOR3 ) && bot == 1 )
   {
       if ( m_type == OBJECT_DOOR2 )  pos = D3DVECTOR(26.0f, 0.0f, -49.0f);
       if ( m_type == OBJECT_DOOR3 )  pos = D3DVECTOR(29.0f, 0.0f, -55.0f);
       mat = m_object->RetWorldMatrix(0);
       pos = Transform(*mat, pos);
       m_terrain->MoveOnFloor(pos);
       m_botPos = pos;
       angle = m_object->RetAngleY(0)+PI;
       object = new CObject(m_iMan);
       if ( !object->CreateBot(pos, angle, 1.0f, OBJECT_EVIL1, TRUE) )
       {
           delete object;
       }
       else
       {
           object->SetPosition(0, pos);
           if ( m_programNumber != -1 )
           {
               char    name[100];
               sprintf(name, "auto%.2d.txt", m_programNumber);
               object->ReadProgram(0, name);
               object->RunProgram(0);
           }
       }
   }
}


// Gestion d'un événement.

BOOL CAutoDoor::EventProcess(const Event &event)
{
   D3DMATRIX*  mat;
   CObject     *wp, *vehicle;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   float       D, d, h, a, piston, duration, mass;
   int         i;

   CAuto::EventProcess(event);

   if ( m_engine->RetPause() )  return TRUE;
   if ( !m_main->IsGameTime() )  return TRUE;

   m_progress += event.rTime*m_speed;
   piston = 0.0f;

   if ( m_type == OBJECT_DOOR1 ||
        m_type == OBJECT_DOOR4 )
   {
       if ( m_phase == ADOP_WAIT )
       {
           m_initialDelay -= event.rTime;

           if ( m_initialDelay <= 0.0f )
           {
               m_phase    = ADOP_1DOWN;
               m_progress = 0.0f;
               m_speed    = 1.0f/15.0f;
               m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 0.2f);
           }
       }

       if ( m_phase == ADOP_1DOWN )
       {
           FireStopUpdate();  // clignotte

           pos = m_object->RetPosition(1);
           pos.y = HEIGHT*m_progress;
           m_object->SetPosition(1, pos);
           UpdateCrashSphere();

           if ( m_progress >= 1.0f )
           {
               pos = m_object->RetPosition(1);
               pos.y = HEIGHT;
               m_object->SetPosition(1, pos);
               UpdateCrashSphere();

               wp = SearchTarget();
               if ( wp != 0 )
               {
                   wp->SetEnable(FALSE);
                   m_main->SetSuperLost();  // on a perdu !
               }

               m_phase    = ADOP_1CLOSE;
               m_progress = 0.0f;
               m_speed    = 1.0f/2.0f;

               FireStopUpdate();  // allume toujours
               m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0));
           }
       }

       if ( m_phase == ADOP_1CLOSE )
       {
       }

       if ( m_phase == ADOP_13WAIT )
       {
           FireStopUpdate();  // clignotte

           if ( m_progress >= 1.0f )
           {
               vehicle = SearchVehicle(1000.0f);
               if ( vehicle != 0 && vehicle->RetFret() != 0 )
               {
                   m_phase    = ADOP_13UP;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/5.0f;
                   m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 0.4f);
               }
               else
               {
                   m_phase    = ADOP_13WAIT;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/1.0f;
               }
           }
       }

       if ( m_phase == ADOP_13UP )
       {
           FireStopUpdate();  // clignotte

           if ( m_progress < 1.0f )
           {
               pos = m_object->RetPosition(1);
               pos.y = HEIGHT*(1.0f-m_progress);
               m_object->SetPosition(1, pos);
               UpdateCrashSphere();
           }
           else
           {
               pos = m_object->RetPosition(1);
               pos.y = 0.0f;
               m_object->SetPosition(1, pos);  // porte ouverte
               UpdateCrashSphere();

               m_phase    = ADOP_13OPEN;
               m_progress = 0.0f;
               m_speed    = 1.0f/1.0f;

               FireStopUpdate();  // éteint
               m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0));
           }
       }

       if ( m_phase == ADOP_13OPEN )
       {
       }
   }

   if ( m_type == OBJECT_DOOR2 )
   {
       if ( m_phase == ADOP_WAIT )
       {
           piston = m_progress*1.0f;

           if ( m_progress < 1.0f )
           {
               vehicle = SearchVehicle();
               if ( vehicle != 0 )  m_progress = 1.0f;
           }
           if ( m_progress >= 1.0f )
           {
               m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0));
               m_phase    = ADOP_2CLOSE;
               m_progress = 0.0f;
               m_speed    = 1.0f/TIME_CLOSE;
           }
       }

       if ( m_phase == ADOP_2CLOSE )
       {
           piston = 0.0f;

           if ( m_progress >= 0.3f )
           {
               vehicle = SearchVehicle();
               if ( vehicle != 0 )
               {
                   CrashVehicle(vehicle, FALSE);
               }
           }

           DownParticule(event.rTime, D3DVECTOR(25.0f, 34.0f, -38.0f));
           MoveDoor(Bounce(m_progress));

           if ( m_progress >= 1.0f )
           {
               m_phase    = ADOP_2OPEN;
               m_progress = 0.0f;
               m_speed    = 1.0f/TIME_OPEN;
           }
       }

       if ( m_phase == ADOP_2OPEN )
       {
           piston = 0.0f;
           MoveDoor(1.0f-m_progress);

           if ( m_progress >= 1.0f )
           {
               m_phase    = ADOP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/(m_cycleDelay-TIME_CLOSE-TIME_OPEN);
           }
       }
   }

   if ( m_type == OBJECT_DOOR3 )
   {
       if ( m_phase == ADOP_WAIT )
       {
           piston = m_progress*1.0f;

           if ( m_progress < 1.0f )
           {
               vehicle = SearchVehicle();
               if ( vehicle != 0 )  m_progress = 1.0f;
           }
           if ( m_progress >= 1.0f )
           {
               m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0));
               m_phase    = ADOP_3DOWN;
               m_progress = 0.0f;
               m_speed    = 1.0f/TIME_CLOSE;
           }
       }

       if ( m_phase == ADOP_3DOWN )
       {
           piston = 0.0f;

           if ( m_progress >= 0.3f )
           {
               vehicle = SearchVehicle();
               if ( vehicle != 0 )
               {
                   CrashVehicle(vehicle, TRUE);
               }
           }

           DownParticule(event.rTime, D3DVECTOR(25.0f, 34.0f, -43.0f));
           MoveDoor(Bounce(m_progress));

           if ( m_progress >= 1.0f )
           {
               m_phase    = ADOP_3UP;
               m_progress = 0.0f;
               m_speed    = 1.0f/TIME_OPEN;
           }
       }

       if ( m_phase == ADOP_3UP )
       {
           piston = 0.0f;
           MoveDoor(1.0f-m_progress);

           if ( m_progress >= 1.0f )
           {
               m_phase    = ADOP_WAIT;
               m_progress = 0.0f;
               m_speed    = 1.0f/(m_cycleDelay-TIME_CLOSE-TIME_OPEN);
           }
       }
   }

   if ( m_phase == ADOP_BREAK )
   {
       if ( m_progress < 1.0f )
       {
           a = -m_object->RetAngleX(3);
           piston = a/PI/2.0f;
           piston += event.rTime*3.0f*m_progress;

           MoveDoor(m_doorStart+powf(m_progress, 3.0f)*(1.0f-m_doorStart));

           m_object->SetAngleY(3, sinf(m_progress*15.0f)*0.5f*m_progress);
           m_object->SetAngleZ(3, sinf(m_time    *25.0f)*0.2f*m_progress);
           m_object->SetAngleY(4, sinf(m_progress*14.7f)*0.5f*m_progress);
           m_object->SetAngleY(5, sinf(m_progress*13.9f)*0.5f*m_progress);
           m_object->SetAngleY(6, sinf(m_progress*16.1f)*0.5f*m_progress);
           m_object->SetAngleZ(6, sinf(m_time    *26.1f)*0.2f*m_progress);

           if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;
               mat = m_object->RetWorldMatrix(6);  // petit engrenage
               for ( i=0 ; i<5 ; i++ )
               {
                   pos.x = 0.0f;
                   pos.y = (Rand()-0.5f)*5.0f;
                   pos.z = (Rand()-0.5f)*5.0f;
                   pos = Transform(*mat, pos);
                   speed.x = (Rand()-0.5f)*8.0f;
                   speed.z = (Rand()-0.5f)*8.0f;
                   speed.y = Rand()*5.0f;
                   dim.x = Rand()*0.5f+0.5f;
                   dim.y = dim.x;
                   duration = 0.5f+Rand()*1.0f;
                   m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, duration);
               }

               for ( i=0 ; i<(int)(5*m_progress) ; i++ )
               {
                   pos.x = 0.0f;
                   pos.y = (Rand()-0.5f)*5.0f;
                   pos.z = (Rand()-0.5f)*5.0f;
                   pos = Transform(*mat, pos);
                   speed.x = (Rand()-0.5f)*4.0f;
                   speed.z = (Rand()-0.5f)*4.0f;
                   speed.y = Rand()*5.0f;
                   dim.x = Rand()*3.0f+3.0f;
                   dim.y = dim.x;
                   duration = 2.0f+Rand()*2.0f;
                   m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, duration);
               }
           }
       }
       else
       {
           m_sound->Play(SOUND_BREAK2, m_engine->RetEyePt(), 1.0f, 0.7f);

           mat = m_object->RetWorldMatrix(6);  // petit engrenage
           for ( i=0 ; i<15 ; i++ )
           {
               pos.x = 0.0f;
               pos.y = (Rand()-0.5f)*5.0f;
               pos.z = (Rand()-0.5f)*5.0f;
               pos = Transform(*mat, pos);
               speed.x = (Rand()-0.5f)*30.0f;
               speed.z = (Rand()-0.5f)*30.0f;
               speed.y = Rand()*30.0f;
               dim.x = 1.0f;
               dim.y = dim.x;
               duration = Rand()*3.0f+2.0f;
               mass = Rand()*10.0f+15.0f;
               m_particule->CreateTrack(pos, speed, dim, PARTITRACK1,
                                        duration, mass, Rand()+0.7f, 1.0f);
           }

           // Donne une apparance cassée au mécanisme.
           pos = m_object->RetPosition(3);
           pos.y -= 3.0f;
           m_object->SetPosition(3, pos);
           m_object->SetAngleX(3, 0.0f);
           m_object->SetAngleY(3, (Rand()-0.5f)*(60.0f*PI/180.0f));
           m_object->SetAngleZ(3, (Rand()-0.5f)*(60.0f*PI/180.0f));

           m_object->SetAngleX(4, (Rand()-0.5f)*(30.0f*PI/180.0f));
           m_object->SetAngleY(4, (Rand()-0.5f)*(60.0f*PI/180.0f));
           m_object->SetAngleZ(4, (Rand()-0.5f)*(30.0f*PI/180.0f));

           m_object->SetAngleX(5, (Rand()-0.5f)*(30.0f*PI/180.0f));
           m_object->SetAngleY(5, (Rand()-0.5f)*(60.0f*PI/180.0f));
           m_object->SetAngleZ(5, (Rand()-0.5f)*(30.0f*PI/180.0f));

           m_object->SetAngleX(6, 0.0f);
           m_object->SetAngleY(6, (Rand()-0.5f)*(30.0f*PI/180.0f));
           m_object->SetAngleZ(6, (Rand()-0.5f)*(30.0f*PI/180.0f));

           speed.x = 10.0f+Rand()*10.0f;
           speed.y = 10.0f+Rand()*15.0f;
           speed.z = (Rand()-0.5f)*1.0f;
           DetachPart(6, speed);  // détache le petit engrenage

           m_phase    = ADOP_STOP;
           m_progress = 0.0f;
           m_speed    = 1.0f/0.8f;
       }
   }

   if ( m_phase == ADOP_STOP )
   {
       if ( m_progress < 1.0f )
       {
           MoveDoor(1.0f-m_progress);
       }
       return TRUE;
   }

   if ( m_type == OBJECT_DOOR2 ||
        m_type == OBJECT_DOOR3 )
   {
       // Fait tourner le mécanisme "roue-piston-bielle".
       a = piston*PI*2.0f;
       m_object->SetAngleX(3, -a);  // grande roue (diamètre = 7)
       m_object->SetAngleX(6, a*7.0f/2.0f);  // petite roue (diamètre = 2)

       d = cosf(a)*5.0f;  // R = 5
       h = sinf(a)*5.0f;
       D = sqrtf(powf(14.0f,2.0f)-powf(h,2.0f))+d;  // L = 14

       pos = m_object->RetPosition(4);
       pos.y = 9.0f+D;
       m_object->SetPosition(4, pos);  // piston

       a = atanf(h/(D-d));
       m_object->SetAngleX(5, a);  // bielle
   }

   return TRUE;
}

// Symboles du système "roue-piston-bielle" :
//
//             o <----- bas du piston mobile |
//            /|
//           /b|   (angle de la bielle)
//          /<--------- bielle
//       L /   | D
//        /    |
//       /    a|   (ici angle a = 120 degrés)
//      /  R __o <----- centre de la roue fixe
//     / __--  | d (ici négatif)
//    o--      o
//    <----h--->
//
// Si l'angle a=0, le piston est en position haute (d=R).
// Si l'angle a=180, le piston est en position basse (d=-R).
//     d = cos(a)*R
//     h = sin(a)*R
//     D = sqrt(L**2-h**2)+d  hauteur du piston mobile
//     b = atan(h/(D-d))      angle de la bielle


// Crée un morceau d'objet qui part.

BOOL CAutoDoor::DetachPart(int part, D3DVECTOR speed)
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos, min, max;
   FPOINT      dim;
   int         channel, objRank;

   mat = m_object->RetWorldMatrix(part);
   pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 0.0f));
   speed = Transform(*mat, speed);
   speed -= pos;
   
   if ( !m_object->FlatParent(part) )  return FALSE;

   objRank = m_object->RetObjectRank(part);
   m_engine->GetBBox(objRank, min, max);
   dim.x = Length(min, max)/2.0f;
   dim.y = dim.x;

   channel = m_particule->CreatePart(pos, speed, dim, PARTIPART, 10.0f, 20.0f, 10.0f);
   if ( channel != -1 )
   {
       m_object->SetMasterParticule(part, channel);
   }

   return TRUE;
}

// Stoppe l'automate.

BOOL CAutoDoor::Abort()
{
   return TRUE;
}


// Met à jour la position des portes.

void CAutoDoor::MoveDoor(float progress)
{
   D3DVECTOR   pos;
   float       dist;

   m_doorProgress = progress;

   if ( m_type == OBJECT_DOOR2 )
   {
       dist = 20.0f-progress*18.0f;

       pos = m_object->RetPosition(1);
       pos.z = dist;
       m_object->SetPosition(1, pos);

       pos = m_object->RetPosition(2);
       pos.z = -dist;
       m_object->SetPosition(2, pos);
   }

   if ( m_type == OBJECT_DOOR3 )
   {
       dist = 20.0f-progress*18.0f;

       pos = m_object->RetPosition(1);
       pos.y = dist;
       m_object->SetPosition(1, pos);
   }

   UpdateCrashSphere();
}

// Met à jour les sphères de collision.

void CAutoDoor::UpdateCrashSphere()
{
   D3DVECTOR   pos;
   float       h;

   if ( m_type == OBJECT_DOOR1 )
   {
       pos = m_object->RetPosition(1);
       h = 35.0f+pos.y;

       m_object->MoveCrashSphere(0, D3DVECTOR(0.0f, h,  24.0f), 6.0f);
       m_object->MoveCrashSphere(1, D3DVECTOR(0.0f, h,  12.0f), 6.0f);
       m_object->MoveCrashSphere(2, D3DVECTOR(0.0f, h,   0.0f), 6.0f);
       m_object->MoveCrashSphere(3, D3DVECTOR(0.0f, h, -12.0f), 6.0f);
       m_object->MoveCrashSphere(4, D3DVECTOR(0.0f, h, -24.0f), 6.0f);
   }

   if ( m_type == OBJECT_DOOR2 )
   {
       pos = m_object->RetPosition(1);
       h = 3.0f+pos.z;

       m_object->MoveCrashSphere(0, D3DVECTOR(-15.0f, 5.0f, -h), 4.0f);
       m_object->MoveCrashSphere(1, D3DVECTOR( -8.0f, 5.0f, -h), 4.0f);
       m_object->MoveCrashSphere(2, D3DVECTOR(  0.0f, 5.0f, -h), 4.0f);
       m_object->MoveCrashSphere(3, D3DVECTOR(  8.0f, 5.0f, -h), 4.0f);
       m_object->MoveCrashSphere(4, D3DVECTOR( 15.0f, 5.0f, -h), 4.0f);

       m_object->MoveCrashSphere(5, D3DVECTOR(-15.0f, 5.0f,  h), 4.0f);
       m_object->MoveCrashSphere(6, D3DVECTOR( -8.0f, 5.0f,  h), 4.0f);
       m_object->MoveCrashSphere(7, D3DVECTOR(  0.0f, 5.0f,  h), 4.0f);
       m_object->MoveCrashSphere(8, D3DVECTOR(  8.0f, 5.0f,  h), 4.0f);
       m_object->MoveCrashSphere(9, D3DVECTOR( 15.0f, 5.0f,  h), 4.0f);
   }

   if ( m_type == OBJECT_DOOR3 )
   {
       pos = m_object->RetPosition(1);
       h = 5.0f+pos.y;

       m_object->MoveCrashSphere(0, D3DVECTOR( 14.0f, h,   0.0f), 6.0f);
       m_object->MoveCrashSphere(1, D3DVECTOR(-14.0f, h,   0.0f), 6.0f);
       m_object->MoveCrashSphere(2, D3DVECTOR(  0.0f, h,  14.0f), 6.0f);
       m_object->MoveCrashSphere(3, D3DVECTOR(  0.0f, h, -14.0f), 6.0f);
       m_object->MoveCrashSphere(4, D3DVECTOR( 10.0f, h,  10.0f), 6.0f);
       m_object->MoveCrashSphere(5, D3DVECTOR( 10.0f, h, -10.0f), 6.0f);
       m_object->MoveCrashSphere(6, D3DVECTOR(-10.0f, h,  10.0f), 6.0f);
       m_object->MoveCrashSphere(7, D3DVECTOR(-10.0f, h, -10.0f), 6.0f);
   }
}

// Met à jour les feux de stop.

void CAutoDoor::FireStopUpdate()
{
   D3DMATRIX*  mat;
   D3DVECTOR   pos, speed;
   FPOINT      dim;
   BOOL        bOn;
   int         i;

   if ( m_type != OBJECT_DOOR1 )  return;
   
   static float listpos[3*6] =
   {
        12.0f, 40.0f,  36.0f,
       -12.0f, 40.0f,  36.0f,
        12.0f, 40.0f, -36.0f,
       -12.0f, 40.0f, -36.0f,  // 4 feux au sommet des tours
         4.5f,  9.0f,   0.0f,
        -4.5f,  9.0f,   0.0f,  // 2 feux sur la porte
   };

   if ( m_phase == ADOP_WAIT   ||  // éteint ?
        m_phase == ADOP_13OPEN )
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
       bOn = FALSE;

       if ( m_phase == ADOP_1DOWN )
       {
           if ( i < 4 && Mod(m_time, 0.6f) >= 0.4f )  bOn = TRUE;
       }

       if ( m_phase == ADOP_13WAIT )
       {
           if ( Mod(m_time, 0.6f) >= 0.4f )  bOn = TRUE;
       }

       if ( m_phase == ADOP_1CLOSE )
       {
           bOn = TRUE;
       }

       if ( m_phase == ADOP_13UP )
       {
           bOn = ( i < 4 );
       }

       if ( bOn )
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


// Retourne une erreur liée à l'état de l'automate.

Error CAutoDoor::RetError()
{
   return ERR_OK;
}


// Cherche l'objet cible.

CObject* CAutoDoor::SearchTarget()
{
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->RetPosition(0);

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;
       if ( pObj->RetTruck() != 0 )  continue;
       if ( pObj->RetLock() )  continue;

       oType = pObj->RetType();
       if ( oType != OBJECT_TARGET )  continue;

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= 5.0f )  return pObj;
   }

   return 0;
}

// Cherche le véhicule sous la porte.

CObject* CAutoDoor::SearchVehicle(float radius)
{
   CObject*    pObj;
   D3DVECTOR   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->RetPosition(0);

   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetGhost() )  continue;

       oType = pObj->RetType();
       if ( oType != OBJECT_CAR )  continue;

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist <= radius )  return pObj;
   }

   return 0;
}

// Ecrase un véhicule sous la porte.

void CAutoDoor::CrashVehicle(CObject *vehicle, BOOL bFlat)
{
   CMotion*    motion;

   motion = vehicle->RetMotion();
   if ( motion == 0 )  return;

   if ( bFlat )
   {
       motion->SetAction(MV_FLATY);  // tout plat
   }
   else
   {
       motion->SetAction(MV_FLATZ);  // tout mince
   }
}

// Particule lorsque le piston descend.

void CAutoDoor::DownParticule(float rTime, D3DVECTOR pos)
{
   D3DMATRIX*  mat;
   D3DVECTOR   speed;
   FPOINT      dim;
   float       duration;

   mat = m_object->RetWorldMatrix(0);
   pos.x += (Rand()-0.5f)*6.0f;
   pos.z += (Rand()-0.5f)*6.0f;
   pos = Transform(*mat, pos);
   speed.x = (Rand()-0.5f)*8.0f;
   speed.z = (Rand()-0.5f)*8.0f;
   speed.y = -Rand()*10.0f;
   dim.x = Rand()*3.0f+3.0f;
   dim.y = dim.x;
   duration = 2.0f+Rand()*2.0f;
   m_particule->CreateParticule(pos, speed, dim, PARTICRASH, duration);
}


