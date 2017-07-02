// autodoor.cpp

#include "graphics/engine/engine.h"
#include "math/all.h"
#include "common/event.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/camera.h"
#include "object/old_object.h"
#include "object/motion/motion.h"
#include "object/motion/buzzingcars/motioncar.h"
#include "ui/controls/interface.h"
#include "ui/controls/button.h"
#include "ui/controls/window.h"
#include "level/robotmain.h"
#include "sound/sound.h"
#include "object/auto/auto.h"
#include "object/auto/buzzingcars/autodoor.h"
#include "object/object_manager.h"



#define HEIGHT     -26.0f      // abaissement de la porte
#define TIME_CLOSE 1.0f        // durée fermeture
#define TIME_OPEN  5.0f        // durée ouverture



// Constructeur de l'objet.

CAutoDoor::CAutoDoor(COldObject* object)
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

CAutoDoor::~CAutoDoor()
{
}


// Détruit l'objet.

void CAutoDoor::DeleteObject(bool bAll)
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

   m_type = m_object->GetType();

   if ( m_type == OBJECT_DOOR2 ||
        m_type == OBJECT_DOOR3 )
   {
       m_speed = 1.0f/(m_cycleDelay-TIME_CLOSE-TIME_OPEN);
   }

   m_botPos = m_object->GetPartPosition(0);
}


// Démarre l'objet.

void CAutoDoor::Start(int param)
{
   Math::Matrix*  mat;
   COldObject*    object;
   Math::Vector   pos;
   float       angle, value;
   int         bot, i;

   if ( param == 2 &&  // casse le mécanisme ?
        (m_type == OBJECT_DOOR2 ||
         m_type == OBJECT_DOOR3 ) )
   {
       if ( m_phase == ADOP_BREAK ||
            m_phase == ADOP_STOP  )  return;  // déjà cassé ?

       i = m_sound->Play(SOUND_BREAK1, m_engine->GetEyePt(), 1.0f, 0.5f, true);
       m_sound->AddEnvelope(i, 1.0f, 2.0f, 6.0f, SOPER_CONTINUE);
       m_sound->AddEnvelope(i, 0.0f, 0.5f, 0.1f, SOPER_STOP);

       i = m_sound->Play(SOUND_WHEELb, m_engine->GetEyePt(), 0.0f, 0.2f, true);
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
       m_initialDelay = m_object->GetCmdLine(0);
       if ( m_initialDelay == NAN )  m_initialDelay = 0.0f;

       value = m_object->GetCmdLine(1);
       if ( value != NAN )  m_counter = (int)value;

       value = m_object->GetCmdLine(2);
       if ( value != NAN )  m_rank = (int)value;

       if ( param == 3 )  // attend une condition pour s'ouvrir ?
       {
           pos = m_object->GetPartPosition(1);
           pos.y = HEIGHT;
           m_object->SetPartPosition(1, pos);  // porte fermée
           UpdateCrashSphere();

           m_phase    = ADOP_13WAIT;
           m_progress = 0.0f;
           m_speed    = 1.0f/1.0f;
       }
   }

   if ( m_type == OBJECT_DOOR2 ||
        m_type == OBJECT_DOOR3 )
   {
       m_cycleDelay = m_object->GetCmdLine(0);
       if ( m_cycleDelay == NAN )  m_cycleDelay = 15.0f;
       if ( m_cycleDelay < 7.0f )  m_cycleDelay = 7.0f;

       bot = (int)m_object->GetCmdLine(1);
       if ( bot == NAN )  bot = 0;

       m_programNumber = (int)m_object->GetCmdLine(2);
       if ( m_programNumber == NAN )  m_programNumber = -1;

       m_phase    = ADOP_WAIT;
       m_progress = 0.0f;
       m_speed    = 1.0f/(m_cycleDelay-TIME_CLOSE-TIME_OPEN);

       m_time = 0.0f;
   }

   // Crée la croix (invisible) sous la porte.
   if ( (m_type == OBJECT_DOOR1 || m_type == OBJECT_DOOR4) && param != 3 )
   {
       pos = m_object->GetPartPosition(0);
       pos.y += 4.0f;
       angle = m_object->GetPartRotationY(0)+Math::PI/2.0f;
       object = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->CreateObject(pos, angle, OBJECT_TARGET)); // TODO (krzys_h): bPlumb = true (?)
       object->SetPosition(pos);
//TODO(krzys_h):       object->SetPassCounter(m_counter);
//TODO(krzys_h):       object->SetRankCounter(m_rank);
   }

   // Crée le robot s'occupant de la porte.
   if ( (m_type == OBJECT_DOOR2 ||
         m_type == OBJECT_DOOR3 ) && bot == 1 )
   {
       if ( m_type == OBJECT_DOOR2 )  pos = Math::Vector(26.0f, 0.0f, -49.0f);
       if ( m_type == OBJECT_DOOR3 )  pos = Math::Vector(29.0f, 0.0f, -55.0f);
       mat = m_object->GetWorldMatrix(0);
       pos = Math::Transform(*mat, pos);
       m_terrain->AdjustToFloor(pos);
       m_botPos = pos;
       angle = m_object->GetRotationY()+Math::PI;
       object = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->CreateObject(pos, angle, OBJECT_EVIL1)); // TODO (krzys_h): bPlumb = true (?)

       object->SetPosition(pos);
       if ( m_programNumber != -1 )
       {
           char    name[100];
           sprintf(name, "auto%.2d.txt", m_programNumber);
           object->ReadProgram(0, name);
           object->RunProgram(0);
       }
   }
}


// Gestion d'un événement.

bool CAutoDoor::EventProcess(const Event &event)
{
   Math::Matrix*  mat;
   COldObject     *wp, *vehicle;
   Math::Vector   pos, speed;
   Math::Point      dim;
   float       D, d, h, a, piston, duration, mass;
   int         i;

   CAuto::EventProcess(event);

   if ( m_engine->GetPause() )  return true;
   if ( !m_main->IsGameTime() )  return true;

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
               m_sound->Play(SOUND_OPEN, m_object->GetPartPosition(0), 1.0f, 0.2f);
           }
       }

       if ( m_phase == ADOP_1DOWN )
       {
           FireStopUpdate();  // clignotte

           pos = m_object->GetPartPosition(1);
           pos.y = HEIGHT*m_progress;
           m_object->SetPartPosition(1, pos);
           UpdateCrashSphere();

           if ( m_progress >= 1.0f )
           {
               pos = m_object->GetPartPosition(1);
               pos.y = HEIGHT;
               m_object->SetPartPosition(1, pos);
               UpdateCrashSphere();

               wp = SearchTarget();
               if ( wp != 0 )
               {
                   wp->SetLock(true); // TODO (krzys_h): ???? wp->SetEnable(false);
                   m_main->SetMissionResult(INFO_LOST);  // on a perdu !
               }

               m_phase    = ADOP_1CLOSE;
               m_progress = 0.0f;
               m_speed    = 1.0f/2.0f;

               FireStopUpdate();  // allume toujours
               m_sound->Play(SOUND_CLOSE, m_object->GetPartPosition(0));
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
               if ( vehicle != 0 && vehicle->GetCargo() != 0 )
               {
                   m_phase    = ADOP_13UP;
                   m_progress = 0.0f;
                   m_speed    = 1.0f/5.0f;
                   m_sound->Play(SOUND_OPEN, m_object->GetPartPosition(0), 1.0f, 0.4f);
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
               pos = m_object->GetPartPosition(1);
               pos.y = HEIGHT*(1.0f-m_progress);
               m_object->SetPartPosition(1, pos);
               UpdateCrashSphere();
           }
           else
           {
               pos = m_object->GetPartPosition(1);
               pos.y = 0.0f;
               m_object->SetPartPosition(1, pos);  // porte ouverte
               UpdateCrashSphere();

               m_phase    = ADOP_13OPEN;
               m_progress = 0.0f;
               m_speed    = 1.0f/1.0f;

               FireStopUpdate();  // éteint
               m_sound->Play(SOUND_CLOSE, m_object->GetPartPosition(0));
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
               m_sound->Play(SOUND_PSHHH, m_object->GetPartPosition(0));
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
                   CrashVehicle(vehicle, false);
               }
           }

           DownParticule(event.rTime, Math::Vector(25.0f, 34.0f, -38.0f));
           MoveDoor(Math::Bounce(m_progress));

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
               m_sound->Play(SOUND_PSHHH, m_object->GetPartPosition(0));
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
                   CrashVehicle(vehicle, true);
               }
           }

           DownParticule(event.rTime, Math::Vector(25.0f, 34.0f, -43.0f));
           MoveDoor(Math::Bounce(m_progress));

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
           a = -m_object->GetPartRotationX(3);
           piston = a/Math::PI/2.0f;
           piston += event.rTime*3.0f*m_progress;

           MoveDoor(m_doorStart+powf(m_progress, 3.0f)*(1.0f-m_doorStart));

           m_object->SetPartRotationY(3, sinf(m_progress*15.0f)*0.5f*m_progress);
           m_object->SetPartRotationZ(3, sinf(m_time    *25.0f)*0.2f*m_progress);
           m_object->SetPartRotationY(4, sinf(m_progress*14.7f)*0.5f*m_progress);
           m_object->SetPartRotationY(5, sinf(m_progress*13.9f)*0.5f*m_progress);
           m_object->SetPartRotationY(6, sinf(m_progress*16.1f)*0.5f*m_progress);
           m_object->SetPartRotationZ(6, sinf(m_time    *26.1f)*0.2f*m_progress);

           if ( m_lastParticule+m_engine->ParticleAdapt(0.05f) <= m_time )
           {
               m_lastParticule = m_time;
               mat = m_object->GetWorldMatrix(6);  // petit engrenage
               for ( i=0 ; i<5 ; i++ )
               {
                   pos.x = 0.0f;
                   pos.y = (Math::Rand()-0.5f)*5.0f;
                   pos.z = (Math::Rand()-0.5f)*5.0f;
                   pos = Math::Transform(*mat, pos);
                   speed.x = (Math::Rand()-0.5f)*8.0f;
                   speed.z = (Math::Rand()-0.5f)*8.0f;
                   speed.y = Math::Rand()*5.0f;
                   dim.x = Math::Rand()*0.5f+0.5f;
                   dim.y = dim.x;
                   duration = 0.5f+Math::Rand()*1.0f;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, duration);
               }

               for ( i=0 ; i<(int)(5*m_progress) ; i++ )
               {
                   pos.x = 0.0f;
                   pos.y = (Math::Rand()-0.5f)*5.0f;
                   pos.z = (Math::Rand()-0.5f)*5.0f;
                   pos = Math::Transform(*mat, pos);
                   speed.x = (Math::Rand()-0.5f)*4.0f;
                   speed.z = (Math::Rand()-0.5f)*4.0f;
                   speed.y = Math::Rand()*5.0f;
                   dim.x = Math::Rand()*3.0f+3.0f;
                   dim.y = dim.x;
                   duration = 2.0f+Math::Rand()*2.0f;
                   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, duration);
               }
           }
       }
       else
       {
           m_sound->Play(SOUND_BREAK2, m_engine->GetEyePt(), 1.0f, 0.7f);

           mat = m_object->GetWorldMatrix(6);  // petit engrenage
           for ( i=0 ; i<15 ; i++ )
           {
               pos.x = 0.0f;
               pos.y = (Math::Rand()-0.5f)*5.0f;
               pos.z = (Math::Rand()-0.5f)*5.0f;
               pos = Math::Transform(*mat, pos);
               speed.x = (Math::Rand()-0.5f)*30.0f;
               speed.z = (Math::Rand()-0.5f)*30.0f;
               speed.y = Math::Rand()*30.0f;
               dim.x = 1.0f;
               dim.y = dim.x;
               duration = Math::Rand()*3.0f+2.0f;
               mass = Math::Rand()*10.0f+15.0f;
               m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK1,
                                        duration, mass, Math::Rand()+0.7f, 1.0f);
           }

           // Donne une apparance cassée au mécanisme.
           pos = m_object->GetPartPosition(3);
           pos.y -= 3.0f;
           m_object->SetPartPosition(3, pos);
           m_object->SetPartRotationX(3, 0.0f);
           m_object->SetPartRotationY(3, (Math::Rand()-0.5f)*(60.0f*Math::PI/180.0f));
           m_object->SetPartRotationZ(3, (Math::Rand()-0.5f)*(60.0f*Math::PI/180.0f));

           m_object->SetPartRotationX(4, (Math::Rand()-0.5f)*(30.0f*Math::PI/180.0f));
           m_object->SetPartRotationY(4, (Math::Rand()-0.5f)*(60.0f*Math::PI/180.0f));
           m_object->SetPartRotationZ(4, (Math::Rand()-0.5f)*(30.0f*Math::PI/180.0f));

           m_object->SetPartRotationX(5, (Math::Rand()-0.5f)*(30.0f*Math::PI/180.0f));
           m_object->SetPartRotationY(5, (Math::Rand()-0.5f)*(60.0f*Math::PI/180.0f));
           m_object->SetPartRotationZ(5, (Math::Rand()-0.5f)*(30.0f*Math::PI/180.0f));

           m_object->SetPartRotationX(6, 0.0f);
           m_object->SetPartRotationY(6, (Math::Rand()-0.5f)*(30.0f*Math::PI/180.0f));
           m_object->SetPartRotationZ(6, (Math::Rand()-0.5f)*(30.0f*Math::PI/180.0f));

           speed.x = 10.0f+Math::Rand()*10.0f;
           speed.y = 10.0f+Math::Rand()*15.0f;
           speed.z = (Math::Rand()-0.5f)*1.0f;
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
       return true;
   }

   if ( m_type == OBJECT_DOOR2 ||
        m_type == OBJECT_DOOR3 )
   {
       // Fait tourner le mécanisme "roue-piston-bielle".
       a = piston*Math::PI*2.0f;
       m_object->SetPartRotationX(3, -a);  // grande roue (diamètre = 7)
       m_object->SetPartRotationX(6, a*7.0f/2.0f);  // petite roue (diamètre = 2)

       d = cosf(a)*5.0f;  // R = 5
       h = sinf(a)*5.0f;
       D = sqrtf(powf(14.0f,2.0f)-powf(h,2.0f))+d;  // L = 14

       pos = m_object->GetPartPosition(4);
       pos.y = 9.0f+D;
       m_object->SetPartPosition(4, pos);  // piston

       a = atanf(h/(D-d));
       m_object->SetPartRotationX(5, a);  // bielle
   }

   return true;
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

bool CAutoDoor::DetachPart(int part, Math::Vector speed)
{
   Math::Matrix*  mat;
   Math::Vector   pos, min, max;
   Math::Point      dim;
   int         channel, objRank;

   mat = m_object->GetWorldMatrix(part);
   pos = Math::Transform(*mat, Math::Vector(0.0f, 0.0f, 0.0f));
   speed = Math::Transform(*mat, speed);
   speed -= pos;
   
   if ( !m_object->FlatParent(part) )  return false;

   objRank = m_object->GetObjectRank(part);
   m_engine->GetObjectBBox(objRank, min, max);
   dim.x = Math::Distance(min, max)/2.0f;
   dim.y = dim.x;

   channel = m_particle->CreatePart(pos, speed, dim, Gfx::PARTIPART, 10.0f, 20.0f, 10.0f, 0);
   if ( channel != -1 )
   {
       m_object->SetMasterParticle(part, channel);
   }

   return true;
}

// Stoppe l'automate.

bool CAutoDoor::Abort()
{
   return true;
}


// Met à jour la position des portes.

void CAutoDoor::MoveDoor(float progress)
{
   Math::Vector   pos;
   float       dist;

   m_doorProgress = progress;

   if ( m_type == OBJECT_DOOR2 )
   {
       dist = 20.0f-progress*18.0f;

       pos = m_object->GetPartPosition(1);
       pos.z = dist;
       m_object->SetPartPosition(1, pos);

       pos = m_object->GetPartPosition(2);
       pos.z = -dist;
       m_object->SetPartPosition(2, pos);
   }

   if ( m_type == OBJECT_DOOR3 )
   {
       dist = 20.0f-progress*18.0f;

       pos = m_object->GetPartPosition(1);
       pos.y = dist;
       m_object->SetPartPosition(1, pos);
   }

   UpdateCrashSphere();
}

// Met à jour les sphères de collision.

void CAutoDoor::UpdateCrashSphere()
{
   Math::Vector   pos;
   float       h;

   if ( m_type == OBJECT_DOOR1 )
   {
       pos = m_object->GetPartPosition(1);
       h = 35.0f+pos.y;

       m_object->MoveCrashSphere(0, Math::Sphere(Math::Vector(0.0f, h,  24.0f), 6.0f));
       m_object->MoveCrashSphere(1, Math::Sphere(Math::Vector(0.0f, h,  12.0f), 6.0f));
       m_object->MoveCrashSphere(2, Math::Sphere(Math::Vector(0.0f, h,   0.0f), 6.0f));
       m_object->MoveCrashSphere(3, Math::Sphere(Math::Vector(0.0f, h, -12.0f), 6.0f));
       m_object->MoveCrashSphere(4, Math::Sphere(Math::Vector(0.0f, h, -24.0f), 6.0f));
   }

   if ( m_type == OBJECT_DOOR2 )
   {
       pos = m_object->GetPartPosition(1);
       h = 3.0f+pos.z;

       m_object->MoveCrashSphere(0, Math::Sphere(Math::Vector(-15.0f, 5.0f, -h), 4.0f));
       m_object->MoveCrashSphere(1, Math::Sphere(Math::Vector( -8.0f, 5.0f, -h), 4.0f));
       m_object->MoveCrashSphere(2, Math::Sphere(Math::Vector(  0.0f, 5.0f, -h), 4.0f));
       m_object->MoveCrashSphere(3, Math::Sphere(Math::Vector(  8.0f, 5.0f, -h), 4.0f));
       m_object->MoveCrashSphere(4, Math::Sphere(Math::Vector( 15.0f, 5.0f, -h), 4.0f));

       m_object->MoveCrashSphere(5, Math::Sphere(Math::Vector(-15.0f, 5.0f,  h), 4.0f));
       m_object->MoveCrashSphere(6, Math::Sphere(Math::Vector( -8.0f, 5.0f,  h), 4.0f));
       m_object->MoveCrashSphere(7, Math::Sphere(Math::Vector(  0.0f, 5.0f,  h), 4.0f));
       m_object->MoveCrashSphere(8, Math::Sphere(Math::Vector(  8.0f, 5.0f,  h), 4.0f));
       m_object->MoveCrashSphere(9, Math::Sphere(Math::Vector( 15.0f, 5.0f,  h), 4.0f));
   }

   if ( m_type == OBJECT_DOOR3 )
   {
       pos = m_object->GetPartPosition(1);
       h = 5.0f+pos.y;

       m_object->MoveCrashSphere(0, Math::Sphere(Math::Vector( 14.0f, h,   0.0f), 6.0f));
       m_object->MoveCrashSphere(1, Math::Sphere(Math::Vector(-14.0f, h,   0.0f), 6.0f));
       m_object->MoveCrashSphere(2, Math::Sphere(Math::Vector(  0.0f, h,  14.0f), 6.0f));
       m_object->MoveCrashSphere(3, Math::Sphere(Math::Vector(  0.0f, h, -14.0f), 6.0f));
       m_object->MoveCrashSphere(4, Math::Sphere(Math::Vector( 10.0f, h,  10.0f), 6.0f));
       m_object->MoveCrashSphere(5, Math::Sphere(Math::Vector( 10.0f, h, -10.0f), 6.0f));
       m_object->MoveCrashSphere(6, Math::Sphere(Math::Vector(-10.0f, h,  10.0f), 6.0f));
       m_object->MoveCrashSphere(7, Math::Sphere(Math::Vector(-10.0f, h, -10.0f), 6.0f));
   }
}

// Met à jour les feux de stop.

void CAutoDoor::FireStopUpdate()
{
   Math::Matrix*  mat;
   Math::Vector   pos, speed;
   Math::Point      dim;
   bool        bOn;
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
       bOn = false;

       if ( m_phase == ADOP_1DOWN )
       {
           if ( i < 4 && Math::Mod(m_time, 0.6f) >= 0.4f )  bOn = true;
       }

       if ( m_phase == ADOP_13WAIT )
       {
           if ( Math::Mod(m_time, 0.6f) >= 0.4f )  bOn = true;
       }

       if ( m_phase == ADOP_1CLOSE )
       {
           bOn = true;
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
               pos = Math::Transform(*mat, pos);
               m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                             dim, Gfx::PARTISELR,
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


// Getourne une erreur liée à l'état de l'automate.

Error CAutoDoor::GetError()
{
   return ERR_OK;
}


// Cherche l'objet cible.

COldObject* CAutoDoor::SearchTarget()
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
       oType = pObj->GetType();
       if ( oType != OBJECT_TARGET )  continue;

       assert(pObj->Implements(ObjectInterfaceType::Old));
       if ( dynamic_cast<COldObject*>(pObj)->GetTransporter() != 0 )  continue;
       if ( pObj->GetLock() )  continue;

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       if ( dist <= 5.0f )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Cherche le véhicule sous la porte.

COldObject* CAutoDoor::SearchVehicle(float radius)
{
   COldObject*    pObj;
   Math::Vector   cPos, oPos;
   ObjectType  oType;
   float       dist;
   int         i;

   cPos = m_object->GetPartPosition(0);

   for ( auto pObj : CObjectManager::GetInstancePointer()->GetAllObjects() )
   {
//TODO (krzys_h):       if ( pObj->GetGhost() )  continue;

       oType = pObj->GetType();
       if ( oType != OBJECT_CAR )  continue;
       assert(pObj->Implements(ObjectInterfaceType::Old));

       oPos = pObj->GetPosition();
       dist = Math::Distance(oPos, cPos);

       if ( dist <= radius )  return dynamic_cast<COldObject*>(pObj);
   }

   return 0;
}

// Ecrase un véhicule sous la porte.

void CAutoDoor::CrashVehicle(COldObject *vehicle, bool bFlat)
{
   CMotion*    motion;

   motion = vehicle->GetMotion();
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

void CAutoDoor::DownParticule(float rTime, Math::Vector pos)
{
   Math::Matrix*  mat;
   Math::Vector   speed;
   Math::Point      dim;
   float       duration;

   mat = m_object->GetWorldMatrix(0);
   pos.x += (Math::Rand()-0.5f)*6.0f;
   pos.z += (Math::Rand()-0.5f)*6.0f;
   pos = Math::Transform(*mat, pos);
   speed.x = (Math::Rand()-0.5f)*8.0f;
   speed.z = (Math::Rand()-0.5f)*8.0f;
   speed.y = -Math::Rand()*10.0f;
   dim.x = Math::Rand()*3.0f+3.0f;
   dim.y = dim.x;
   duration = 2.0f+Math::Rand()*2.0f;
   m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, duration);
}


