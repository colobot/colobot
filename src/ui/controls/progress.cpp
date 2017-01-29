// progress.cpp

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
#include "particule.h"
#include "iman.h"
#include "progress.h"




// Constructeur de l'objet.

CProgress::CProgress(CInstanceManager* iMan) : CControl(iMan)
{
   CControl::CControl(iMan);

   m_total = 0;
   m_progress = 0;
}

// Destructeur de l'objet.

CProgress::~CProgress()
{
   CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CProgress::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
   if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

   CControl::Create(pos, dim, icon, eventMsg);
   return TRUE;
}


// Gestion d'un événement.

BOOL CProgress::EventProcess(const Event &event)
{
   CControl::EventProcess(event);

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


// Dessine la jauge.

void CProgress::Draw()
{
   FPOINT      uv1,uv2, corner, pos, dim;
   float       dp;
   int         i;

   if ( (m_state & STATE_VISIBLE) == 0 )  return;

   if ( m_state & STATE_SHADOW )
   {
       DrawShadow(m_pos, m_dim);
   }

   dp = 0.5f/256.0f;

   m_engine->SetTexture("button1.tga");
   m_engine->SetState(D3DSTATENORMAL);
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
   DrawIcon(m_pos, m_dim, uv1, uv2, corner, 8.0f/256.0f);

   if ( m_total != 0 )
   {
       pos.x = m_pos.x+3.0f/640.0f;
       pos.y = m_pos.y+3.0f/480.0f;
       dim.x = m_dim.x-6.0f/640.0f;
       dim.y = m_dim.y-6.0f/480.0f;
       dim.x /= m_total;

       for ( i=0 ; i<m_total ; i++ )
       {
           uv1.x =  32.0f/256.0f;
           uv1.y =  64.0f/256.0f;
           uv2.x =  64.0f/256.0f;
           uv2.y =  96.0f/256.0f;
           if ( i < m_progress )
           {
               uv1.x += 32.0f/256.0f;
               uv2.x += 32.0f/256.0f;
           }
           uv1.x += dp;
           uv1.y += dp;
           uv2.x -= dp;
           uv2.y -= dp;
           corner.x = 2.0f/640.0f;
           corner.y = 2.0f/480.0f;
           DrawIcon(pos, dim, uv1, uv2, corner, 3.0f/256.0f);
           pos.x += dim.x;
       }
   }
}


// Gestion du nombre total d'étapes.

void CProgress::SetTotal(int total)
{
   if ( total < 0 )  total = 0;
   m_total = total;
}

int CProgress::RetTotal()
{
   return m_total;
}

// Gestion la progression.

void CProgress::SetProgress(int progress)
{
   D3DVECTOR   pos, speed;
   FPOINT      ppos, dim;

   if ( progress > m_total )  progress = m_total;
   if ( progress == m_progress )  return;
   m_progress = progress;

   ppos.x = m_pos.x+3.0f/640.0f;
   ppos.y = m_pos.y+3.0f/480.0f;
   dim.x = m_dim.x-6.0f/640.0f;
   dim.y = m_dim.y-6.0f/480.0f;
   dim.x /= m_total;

   pos.x = ppos.x+dim.x*(m_progress-0.5f);
   pos.y = ppos.y+dim.y*0.5f;
   pos.z = 0.0f;
   speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
   dim.x = ((50.0f+Rand()*20.0f)/640.0f);
   dim.y = dim.x/0.75f;
   m_particule->CreateParticule(pos, speed, dim, PARTIGLINT,
                                0.5f, 0.0f, SH_INTERFACE);
}

int CProgress::RetProgress()
{
   return m_progress;
}


