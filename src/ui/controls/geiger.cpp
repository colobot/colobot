// geiger.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "object.h"
#include "motion.h"
#include "motionbot.h"
#include "sound.h"
#include "geiger.h"



#define TYPE_ATOMIC        1
#define TYPE_WALKER        2
#define TYPE_UFO       3
#define TYPE_BOT1      4



// Constructeur de l'objet.

CGeiger::CGeiger(CInstanceManager* iMan) : CControl(iMan)
{
   CControl::CControl(iMan);
   m_type = OBJECT_BARRELa;
   m_radius = 300.0f;
   m_proxi = 0.0f;
   m_lastDetect = 0.0f;
   m_phase = 0.0f;
   m_bLightOn = FALSE;
}

// Destructeur de l'objet.

CGeiger::~CGeiger()
{
   CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CGeiger::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
   if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

   CControl::Create(pos, dim, icon, eventMsg);

   if ( m_icon == TYPE_ATOMIC )
   {
       m_type = OBJECT_BARRELa;
       m_radius = 300.0f;
   }

   if ( m_icon == TYPE_WALKER )
   {
       m_type = OBJECT_WALKER;
       m_radius = 600.0f;
   }

   if ( m_icon == TYPE_UFO )
   {
       m_type = OBJECT_UFO;
       m_radius = 600.0f;
   }

   if ( m_icon == TYPE_BOT1 )
   {
       m_type = OBJECT_BOT1;
       m_radius = 100.0f;
   }

   return TRUE;
}


// Gestion d'un événement.

BOOL CGeiger::EventProcess(const Event &event)
{
   if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
   if ( m_state & STATE_DEAD )  return TRUE;

   CControl::EventProcess(event);

   if ( event.event == EVENT_FRAME )
   {
       if ( !m_engine->RetPause()   &&
            m_state & STATE_VISIBLE )
       {
           m_lastDetect += event.rTime;

           if ( m_lastDetect > 0.4f )
           {
               m_lastDetect = 0.0f;
               m_proxi = DetectObject();
           }

           m_phase += event.rTime*m_proxi*6.0f;
       }
   }

   if ( event.event == EVENT_LBUTTONDOWN )
   {
       if ( CControl::Detect(event.pos) )
       {
           Event newEvent = event;
           newEvent.event = m_eventMsg;
           m_event->AddEvent(newEvent);
           return FALSE;
       }
   }

   return TRUE;
}


// Dessine le bouton.

void CGeiger::Draw()
{
   FPOINT  pos, dim, corner, uv1, uv2;
   float   dp;

   if ( (m_state & STATE_VISIBLE) == 0 )  return;

   dp = 0.5f/256.0f;

   if ( m_state & STATE_SHADOW )
   {
       DrawShadow(m_pos, m_dim);
   }

   m_engine->SetTexture("button1.tga");
   m_engine->SetState(D3DSTATENORMAL);

   pos = m_pos;
   dim = m_dim;
   uv1.x =  64.0f/256.0f;
   uv1.y =   0.0f/256.0f;
   uv2.x =  96.0f/256.0f;
   uv2.y =  32.0f/256.0f;
   uv1.x += dp;
   uv1.y += dp;
   uv2.x -= dp;
   uv2.y -= dp;
   corner.x = 6.0f/640.0f;
   corner.y = 6.0f/480.0f;
   DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);

   if ( m_proxi != 0.0f && Mod(m_phase, 1.0f) < 0.3f )
   {
       pos.x += 3.0f/640.0f;
       pos.y += 3.0f/480.0f;
       dim.x -= 6.0f/640.0f;
       dim.y -= 6.0f/480.0f;
       uv1.x =  64.0f/256.0f;  // vert
       uv1.y =  64.0f/256.0f;
       uv2.x =  96.0f/256.0f;
       uv2.y =  96.0f/256.0f;
       uv1.x += dp;
       uv1.y += dp;
       uv2.x -= dp;
       uv2.y -= dp;
       corner.x = 2.0f/640.0f;
       corner.y = 2.0f/480.0f;
       DrawIcon(pos, dim, uv1, uv2, corner, 3.0f/256.0f);

       m_engine->SetState(D3DSTATETTw);
       if ( m_icon == TYPE_ATOMIC )
       {
           uv1.x =   0.0f/256.0f;  // radioactif
           uv1.y = 224.0f/256.0f;
           uv2.x =  32.0f/256.0f;
           uv2.y = 256.0f/256.0f;
       }
       else if ( m_icon == TYPE_WALKER )
       {
           uv1.x =  64.0f/256.0f;  // robot
           uv1.y = 224.0f/256.0f;
           uv2.x =  96.0f/256.0f;
           uv2.y = 256.0f/256.0f;
       }
       else if ( m_icon == TYPE_UFO )
       {
           uv1.x =  32.0f/256.0f;  // radar
           uv1.y = 224.0f/256.0f;
           uv2.x =  64.0f/256.0f;
           uv2.y = 256.0f/256.0f;
       }
       else
       {
           uv1.x =  32.0f/256.0f;  // radar
           uv1.y = 224.0f/256.0f;
           uv2.x =  64.0f/256.0f;
           uv2.y = 256.0f/256.0f;
       }
       uv1.x += dp;
       uv1.y += dp;
       uv2.x -= dp;
       uv2.y -= dp;
       DrawIcon(pos, dim, uv1, uv2);

       if ( !m_bLightOn )
       {
           m_sound->Play(SOUND_MESSAGE, m_engine->RetEyePt(), 0.5f, 0.5f+2.0f*m_proxi);
       }
       m_bLightOn = TRUE;
   }
   else
   {
       m_bLightOn = FALSE;
   }
}


// Détecte l'objet le plus proche et retourne la "distance" (0..1).

float CGeiger::DetectObject()
{
   CObject*    pObj;
   CMotion*    motion;
   D3DVECTOR   cPos, oPos;
   float       min, dist;
   int         i;

   cPos = m_engine->RetLookatPt();

   min = 100000.0f;
   for ( i=0 ; i<1000000 ; i++ )
   {
       pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
       if ( pObj == 0 )  break;

       if ( pObj->RetType() != m_type )  continue;
       if ( pObj->RetTruck() )  continue;

       if ( m_icon == TYPE_WALKER )
       {
           if ( !pObj->IsProgram() )  continue;
       }

       if ( m_icon == TYPE_BOT1 )
       {
           if ( pObj->RetLock() )  continue;

           motion = pObj->RetMotion();
           if ( motion == 0 )  continue;
           if ( motion->RetAction() != MB_WALK1 )  continue;
       }

       oPos = pObj->RetPosition(0);
       dist = Length(oPos, cPos);

       if ( dist < min )
       {
           min = dist;
       }
   }

   if ( min > m_radius )  return 0.0f;
   return 1.0f-(min/m_radius);
}

