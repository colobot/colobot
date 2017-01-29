// array.cpp

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
#include "button.h"
#include "scroll.h"
#include "text.h"
#include "array.h"



#define MARGING        4.0f



// Constructeur de l'objet.

CArray::CArray(CInstanceManager* iMan) : CControl(iMan)
{
   int     i;

   CControl::CControl(iMan);

   for ( i=0 ; i<ARRAYMAXDISPLAY ; i++ )
   {
       m_button[i] = 0;
   }
   m_scroll = 0;

   m_head[0] = 0;
   for ( i=0 ; i<ARRAYMAXTOTAL ; i++ )
   {
       m_text[i][0] = 0;
       m_enable[i] = TRUE;
   }

   for ( i=0 ; i<10 ; i++ )
   {
       m_tabs[i] = 0.0f;
       m_justifs[i] = 1;
       m_font[i] = FONT_COLOBOT;
   }

   m_totalLine = 0;
   m_displayLine = 0;
   m_selectLine = -1;
   m_firstLine = 0;
   m_multiple = 1;
   m_bBlink = FALSE;
   m_bSelectCap = TRUE;
   m_blinkTime = 0.0f;
}

// Destructeur de l'objet.

CArray::~CArray()
{
   int     i;

   for ( i=0 ; i<ARRAYMAXDISPLAY ; i++ )
   {
       delete m_button[i];
   }
   delete m_scroll;

   CControl::~CControl();
}


// Crée une nouvelle liste.

BOOL CArray::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg,
                  float expand)
{
   m_expand = expand;

   if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();
   CControl::Create(pos, dim, icon, eventMsg);

   m_scroll = new CScroll(m_iMan);
   m_scroll->Create(pos, dim, 0, EVENT_NULL);
   m_eventScroll = m_scroll->RetEventMsg();

   return MoveAdjust();
}

// Ajuste après un changement de dimensions.

BOOL CArray::MoveAdjust()
{
   FPOINT      ipos, idim, ppos, ddim;
   float       marging, h;
   int         i;

   for ( i=0 ; i<ARRAYMAXDISPLAY ; i++ )
   {
       delete m_button[i];
       m_button[i] = 0;
   }

   if ( m_icon == 0 )  marging = MARGING;
   else                marging = 0.0f;

   ipos.x = m_pos.x+marging/640.f;
   ipos.y = m_pos.y+marging/480.f;
   idim.x = m_dim.x-marging*2.0f/640.f;
   idim.y = m_dim.y-marging*2.0f/480.f;

   h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType)*m_expand;

   m_displayLine = (int)(idim.y/h-1);
   if ( m_displayLine == 0 )  return FALSE;
   if ( m_displayLine > ARRAYMAXDISPLAY )  m_displayLine = ARRAYMAXDISPLAY;
   idim.y = h*(m_displayLine+1);
   m_dim.y = idim.y+marging*2.0f/480.f;

   ppos.x = ipos.x;
   ppos.y = ipos.y+idim.y-h;
   ddim.x = idim.x-SCROLL_WIDTH;
   ddim.y = h;
   h_headPos = ppos;
   h_headDim = ddim;
   ppos.y -= h;

   for ( i=0 ; i<m_displayLine ; i++ )
   {
       m_button[i] = new CButton(m_iMan);
       m_button[i]->Create(ppos, ddim, -1, EVENT_NULL);
       m_button[i]->SetJustif(1);
       m_button[i]->SetState(STATE_SIMPLY);
       m_button[i]->SetFontType(m_fontType);
       m_button[i]->SetFontSize(m_fontSize);
       ppos.y -= h;

       m_eventButton[i] = m_button[i]->RetEventMsg();
   }

   if ( m_scroll != 0 )
   {
       ppos.x = ipos.x+idim.x-SCROLL_WIDTH;
       ppos.y = ipos.y;
       ddim.x = SCROLL_WIDTH;
       ddim.y = idim.y;
       m_scroll->SetPos(ppos);
       m_scroll->SetDim(ddim);
   }

   UpdateScroll();
   UpdateButton();
   return TRUE;
}


// Retourne le message d'un bouton.

EventMsg CArray::RetEventMsgButton(int i)
{
   if ( i < 0 || i >= m_displayLine )  return EVENT_NULL;
   if ( m_button[i] == 0 )  return EVENT_NULL;
   return m_button[i]->RetEventMsg();
}

// Retourne le message de l'ascenseur.

EventMsg CArray::RetEventMsgScroll()
{
   if ( m_scroll == 0 )  return EVENT_NULL;
   return m_scroll->RetEventMsg();
}


void CArray::SetPos(FPOINT pos)
{
   CControl::SetPos(pos);
}

void CArray::SetDim(FPOINT dim)
{
   m_dim = dim;
   MoveAdjust();
   CControl::SetDim(dim);
}


BOOL CArray::SetState(int state, BOOL bState)
{
   int     i;

   if ( state & STATE_ENABLE )
   {
       for ( i=0 ; i<m_displayLine ; i++ )
       {
           if ( m_button[i] != 0 )  m_button[i]->SetState(state, bState);
       }
       if ( m_scroll != 0 )  m_scroll->SetState(state, bState);
   }

   return CControl::SetState(state, bState);
}

BOOL CArray::SetState(int state)
{
   int     i;

   if ( state & STATE_ENABLE )
   {
       for ( i=0 ; i<m_displayLine ; i++ )
       {
           if ( m_button[i] != 0 )  m_button[i]->SetState(state);
       }
       if ( m_scroll != 0 )  m_scroll->SetState(state);
   }

   return CControl::SetState(state);
}

BOOL CArray::ClearState(int state)
{
   int     i;

   if ( state & STATE_ENABLE )
   {
       for ( i=0 ; i<m_displayLine ; i++ )
       {
           if ( m_button[i] != 0 )  m_button[i]->ClearState(state);
       }
       if ( m_scroll != 0 )  m_scroll->ClearState(state);
   }

   return CControl::ClearState(state);
}


// Gestion d'un événement.

BOOL CArray::EventProcess(const Event &event)
{
   int     i;

   if ( m_bBlink                   &&  // clignotte ?
        event.event == EVENT_FRAME )
   {
       i = m_selectLine-m_firstLine;

       if ( i >= 0 && i < 4  &&
            m_button[i] != 0 )
       {
           m_blinkTime += event.rTime;
           if ( Mod(m_blinkTime, 0.7f) < 0.3f )
           {
               m_button[i]->ClearState(STATE_ENABLE);
               m_button[i]->ClearState(STATE_CHECK);
           }
           else
           {
               m_button[i]->SetState(STATE_ENABLE);
               m_button[i]->SetState(STATE_CHECK);
           }
       }
   }

   if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
   if ( (m_state & STATE_ENABLE) == 0 )  return TRUE;

   if ( event.event == EVENT_KEYDOWN &&
        event.param == VK_WHEELUP    &&
        Detect(event.pos)            )
   {
       m_firstLine -= m_multiple;
       if ( m_firstLine < 0 )  m_firstLine = 0;
       UpdateScroll();
       UpdateButton();
       return TRUE;
   }
   if ( event.event == EVENT_KEYDOWN &&
        event.param == VK_WHEELDOWN  &&
        Detect(event.pos)            )
   {
       m_firstLine += m_multiple;
       if ( m_firstLine > m_totalLine-m_displayLine )  m_firstLine = m_totalLine-m_displayLine;
       if ( m_firstLine < 0 )  m_firstLine = 0;
       UpdateScroll();
       UpdateButton();
       return TRUE;
   }

   if ( m_bFocus &&
        (m_state & STATE_VISIBLE) &&
        (m_state & STATE_ENABLE)  &&
        event.event == EVENT_KEYDOWN &&
        (event.param == VK_UP ||
         event.param == VK_JUP) )
   {
       m_selectLine --;
       if ( m_selectLine < 0 )  m_selectLine = 0;
       if ( m_selectLine < m_firstLine ||
            m_selectLine >= m_firstLine+m_displayLine )
       {
           ShowSelect();
       }
       UpdateScroll();
       UpdateButton();
       Event newEvent = event;
       newEvent.event = m_eventMsg;
       m_event->AddEvent(newEvent);  // ligne sélectionnée changée
       return TRUE;
   }
   if ( m_bFocus &&
        (m_state & STATE_VISIBLE) &&
        (m_state & STATE_ENABLE)  &&
        event.event == EVENT_KEYDOWN &&
        (event.param == VK_DOWN ||
         event.param == VK_JDOWN) )
   {
       m_selectLine ++;
       if ( m_selectLine >= m_totalLine )  m_selectLine = m_totalLine-1;
       if ( m_selectLine < m_firstLine ||
            m_selectLine >= m_firstLine+m_displayLine )
       {
           ShowSelect();
       }
       UpdateScroll();
       UpdateButton();
       Event newEvent = event;
       newEvent.event = m_eventMsg;
       m_event->AddEvent(newEvent);  // ligne sélectionnée changée
       return TRUE;
   }

   if ( m_bFocus &&
        (m_state & STATE_VISIBLE) &&
        (m_state & STATE_ENABLE)  &&
        (m_state & STATE_DEFAULT) &&
        event.event == EVENT_KEYDOWN &&
        (event.param == VK_RETURN ||
         event.param == VK_BUTTON1) )
   {
       Event newEvent = event;
       newEvent.event = EVENT_KEYDOWN;
       newEvent.param = VK_DEFAULT;
       m_event->AddEvent(newEvent);  // ligne sélectionnée changée
       return TRUE;
   }

   CControl::EventProcess(event);

   if ( event.event == EVENT_MOUSEMOVE && Detect(event.pos) )
   {
       m_engine->SetMouseType(D3DMOUSENORM);
       if ( m_bSelectCap )
       {
           for ( i=0 ; i<m_displayLine ; i++ )
           {
               if ( i+m_firstLine >= m_totalLine )  break;
               if ( m_button[i] != 0 )
               {
                   m_button[i]->EventProcess(event);
               }
           }
       }
   }

   if ( m_bSelectCap )
   {
       for ( i=0 ; i<m_displayLine ; i++ )
       {
           if ( i+m_firstLine >= m_totalLine )  break;
           if ( m_button[i] != 0 )
           {
               if ( !m_button[i]->EventProcess(event) )  return FALSE;

               if ( event.event == m_eventButton[i] )
               {
                   SetSelect(m_firstLine+i);

                   Event newEvent = event;
                   newEvent.event = m_eventMsg;
                   m_event->AddEvent(newEvent);  // ligne sélectionnée changée
               }
           }
       }
   }

   if ( m_scroll != 0 )
   {
       if ( !m_scroll->EventProcess(event) )  return FALSE;

       if ( event.event == m_eventScroll )
       {
           MoveScroll();
           UpdateButton();
       }
   }

   return TRUE;
}


// Dessine la liste.

void CArray::Draw()
{
   FPOINT  uv1, uv2, corner, pos, dim;
   float   dp;
   int     i;

   if ( (m_state & STATE_VISIBLE) == 0 )  return;

   if ( m_state & STATE_SHADOW )
   {
       DrawShadow(m_pos, m_dim);
   }
   if ( m_bFocus )
   {
       DrawFocus(m_pos, m_dim);
   }

   dp = 0.5f/256.0f;

   if ( m_icon != -1 )
   {
       dim = m_dim;

       if ( m_icon == 0 )
       {
           m_engine->SetTexture("button1.tga");
           m_engine->SetState(D3DSTATENORMAL);

           uv1.x =  64.0f/256.0f;
           uv1.y =   0.0f/256.0f;  // u-v texture
           uv2.x =  96.0f/256.0f;
           uv2.y =  32.0f/256.0f;
       }
       else
       {
           m_engine->SetTexture("button1.tga");
           m_engine->SetState(D3DSTATENORMAL);

           uv1.x =  64.0f/256.0f;
           uv1.y =   0.0f/256.0f;  // u-v texture
           uv2.x =  96.0f/256.0f;
           uv2.y =  32.0f/256.0f;

           if ( m_button[0] != 0 )
           {
               dim = m_button[0]->RetDim();
               dim.y *= m_displayLine+1;  // fond pile-poil derrière
           }
       }

       uv1.x += dp;
       uv1.y += dp;
       uv2.x -= dp;
       uv2.y -= dp;

       corner.x = 10.0f/640.0f;
       corner.y = 10.0f/480.0f;
       DrawIcon(m_pos, dim, uv1, uv2, corner, 8.0f/256.0f);
   }

   if ( m_totalLine < m_displayLine )  // boutons pas jusqu'en bas ?
   {
       i = m_totalLine;
       if ( m_button[i] != 0 )
       {
           pos = m_button[i]->RetPos();
           dim = m_button[i]->RetDim();
           pos.y += dim.y*1.1f;
           dim.y *= 0.4f;
           pos.y -= dim.y;

           m_engine->SetTexture("button1.tga");
           m_engine->SetState(D3DSTATETTw);
           uv1.x =  24.0f/256.0f;
           uv1.y =  48.0f/256.0f;
           uv2.x =  32.0f/256.0f;
           uv2.y =  32.0f/256.0f;
           uv1.x += dp;
           uv1.y -= dp;
           uv2.x -= dp;
           uv2.y += dp;
           DrawIcon(pos, dim, uv1, uv2);  // ch'tite ombre mignonne
       }
   }

   DrawLine(h_headPos, h_headDim, m_head, FALSE);  // dessine l'en-tête
   
   for ( i=0 ; i<m_displayLine ; i++ )
   {
       if ( i+m_firstLine >= m_totalLine )  break;

       if ( m_button[i] != 0 )
       {
           if ( !m_bBlink && i+m_firstLine < m_totalLine )
           {
               m_button[i]->SetState(STATE_ENABLE, m_enable[i+m_firstLine] && (m_state & STATE_ENABLE) );
           }
           m_button[i]->SetState(STATE_SIMPLY);
           m_button[i]->Draw();  // dessine une case sans texte

           // dessine le texte dans la case
           pos = m_button[i]->RetPos();
           dim = m_button[i]->RetDim();
           DrawLine(pos, dim, m_text[i+m_firstLine], TRUE);

           if ( !m_enable[i+m_firstLine] )
           {
               m_engine->SetTexture("button1.tga");
               m_engine->SetState(D3DSTATETTb);
               DrawPart(pos, dim, 7, 1.0f, 8.0f/256.0f);  // blanc par-dessus
           }
       }
   }

   if ( m_scroll != 0 )
   {
       m_scroll->Draw();  // dessine l'ascenseur
   }
}

// Dessine tous les textes d'une ligne.

void CArray::DrawLine(FPOINT pos, FPOINT dim, char *line, BOOL bMultiFont)
{
   FPOINT  ppos, ddim;
   int     j;
   char    text[200];
   char    *pb, *pe;

   if ( m_tabs[0] == 0.0f )
   {
       ppos.x = pos.x+dim.y*0.5f;
       ppos.y = pos.y+dim.y*0.5f;
       ppos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
       ddim.x = dim.x-dim.y;
       DrawCase(line, ppos, ddim.x, 1, m_fontType);
   }
   else
   {
       ppos.x = pos.x+dim.y*0.5f;
       ppos.y = pos.y+dim.y*0.5f;
       ppos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
       pb = line;
       for ( j=0 ; j<10 ; j++ )
       {
           pe = strchr(pb, '\t');
           if ( pe == 0 )
           {
               strcpy(text, pb);
           }
           else
           {
               strncpy(text, pb, pe-pb);
               text[pe-pb] = 0;
           }

           if ( bMultiFont )
           {
               DrawCase(text, ppos, m_tabs[j], m_justifs[j], m_font[j]);
           }
           else
           {
               DrawCase(text, ppos, m_tabs[j], m_justifs[j], m_fontType);
           }

           if ( pe == 0 )  break;
           ppos.x += m_tabs[j];
           pb = pe+1;
       }
   }
}

// Affiche un texte dans une case.

void CArray::DrawCase(char *text, FPOINT pos, float width, int justif, FontType font)
{
   if ( justif == 1 )
   {
       m_engine->RetText()->DrawText(text, pos, width, 1, m_fontSize, m_fontStretch, font, 0);
   }
   else if ( justif == 0 )
   {
       pos.x += width/2.0f;
       m_engine->RetText()->DrawText(text, pos, width, 0, m_fontSize, m_fontStretch, font, 0);
   }
   else
   {
       pos.x += width;
       m_engine->RetText()->DrawText(text, pos, width, -1, m_fontSize, m_fontStretch, font, 0);
   }
}


// Vide complètement la liste.

void CArray::Flush()
{
   m_totalLine = 0;
   m_selectLine = -1;
   m_firstLine = 0;
   UpdateButton();
   UpdateScroll();
}


// Spécifie le nombre total de lignes.

void CArray::SetTotal(int i)
{
   m_totalLine = i;
}

// Retourne le nombre total de lignes.

int CArray::RetTotal()
{
   return m_totalLine;
}


// Spécifie le nombre multiple pour la 1ère ligne.

void CArray::SetMultiple(int i)
{
   m_multiple = i;
}

// Retourne le nombre multiple pour la 1ère ligne.

int CArray::RetMultiple()
{
   return m_multiple;
}


// Sélectionne une ligne.

void CArray::SetSelect(int i)
{
   m_selectLine = i;
   UpdateButton();
}

// Retourne la ligne sélectionnée.

int CArray::RetSelect()
{
   return m_selectLine;
}


// Gestion de la capacité à sélectionner une case.

void CArray::SetSelectCap(BOOL bEnable)
{
   m_bSelectCap = bEnable;
}

BOOL CArray::RetSelectCap()
{
   return m_bSelectCap;
}


// Spécifie le bit "enable" pour une case.

void CArray::SetEnable(int i, BOOL bMode)
{
   if ( i < 0 || i >= m_totalLine )  return;

   m_enable[i] = bMode;
}

// Retourne le bit "enable" pour une case.

BOOL CArray::RetEnable(int i)
{
   if ( i < 0 || i >= m_totalLine )  return FALSE;

   return m_enable[i];
}


// Fait cligontter une ligne.

void CArray::SetBlink(BOOL bEnable)
{
   int     i;

   m_bBlink = bEnable;
   m_blinkTime = 0.0f;

   i = m_selectLine-m_firstLine;

   if ( i >= 0 && i < 4  &&
        m_button[i] != 0 )
   {
       if ( !bEnable )
       {
           m_button[i]->SetState(STATE_CHECK);
           m_button[i]->ClearState(STATE_ENABLE);
       }
   }
}

BOOL CArray::RetBlink()
{
   return m_bBlink;
}


// Spécifie le texte d'une ligne.

void CArray::SetName(int i, char* name)
{
   if ( i == -1 )
   {
       strcpy(m_head, name);
       return;
   }

   if ( i < 0 || i >= ARRAYMAXTOTAL )  return;

   if ( i >= m_totalLine )
   {
       m_totalLine = i+1;  // allonge la liste
   }

   if ( name[0] == 0 )
   {
       strcpy(m_text[i], " ");
   }
   else
   {
       strcpy(m_text[i], name);
   }
   UpdateButton();
   UpdateScroll();
}

// Retourne le texte d'une ligne.

char* CArray::RetName(int i)
{
   if ( i < 0 || i >= m_totalLine )  return 0;

   return m_text[i];
}


// Gestion de la position des tabulateurs.

void CArray::SetTabs(int i, float pos, int justif, FontType font)
{
   if ( i < 0 || i >= 10 )  return;
   m_tabs[i] = pos;
   m_justifs[i] = justif;
   m_font[i] = font;
}

float  CArray::RetTabs(int i)
{
   if ( i < 0 || i >= 10 )  return 0.0f;
   return m_tabs[i];
}


// Déplace l'ascenseur de la liste pour voir la ligne sélectionnée.

void CArray::ShowSelect()
{
   int     sel;

//?    if ( m_selectLine >= m_firstLine &&
//?         m_selectLine <  m_firstLine+m_displayLine )  return;  // tout bon

   sel = m_selectLine;

   if ( m_multiple == 1 )
   {
       // Descend de 1/2*h.
       sel += m_displayLine/2;
       if ( sel > m_totalLine-1 )  sel = m_totalLine-1;

       // Remonte de h-1.
       sel -= m_displayLine-1;
       if ( sel < 0 )  sel = 0;
   }
   else
   {
       sel = (sel/m_multiple)*m_multiple;
   }

   m_firstLine = sel;

   UpdateButton();
   UpdateScroll();
}


// Met à jour tous les noms des boutons.

void CArray::UpdateButton()
{
   int     state, i, j;

   state = CControl::RetState();

   j = m_firstLine;
   for ( i=0 ; i<m_displayLine ; i++ )
   {
       if ( m_button[i] == 0 )  continue;

       m_button[i]->SetState(STATE_CHECK, (j == m_selectLine));

       if ( j < m_totalLine )
       {
//?            m_button[i]->SetName(m_text[j]);
           m_button[i]->SetName(" ");  // bouton vide
           m_button[i]->SetState(STATE_ENABLE, (state & STATE_ENABLE));
       }
       else
       {
           m_button[i]->SetName(" ");  // bouton vide
           m_button[i]->ClearState(STATE_ENABLE);
       }
       j ++;
   }
}

// Met à jour l'ascenseur.

void CArray::UpdateScroll()
{
   float   ratio, value, step;
   int     displayLine, totalLine, firstLine;

   if ( m_scroll == 0 )  return;

   if ( m_totalLine <= m_displayLine )
   {
       ratio = 1.0f;
       value = 0.0f;
       step = 0.0f;
   }
   else
   {
       displayLine = m_displayLine/m_multiple;
       totalLine   = m_totalLine/m_multiple;
       firstLine   = m_firstLine/m_multiple;

       ratio = (float)displayLine/totalLine;
       if ( ratio > 1.0f )  ratio = 1.0f;

       value = (float)firstLine/(totalLine-displayLine);
       if ( value < 0.0f )  value = 0.0f;
       if ( value > 1.0f )  value = 1.0f;

       step = (float)1.0f/(totalLine-displayLine);
       if ( step < 0.0f )  step = 0.0f;
   }

   m_scroll->SetVisibleRatio(ratio);
   m_scroll->SetVisibleValue(value);
   m_scroll->SetArrowStep(step);
}

// Mise à jour lorsque l'ascenseur a été bougé.

void CArray::MoveScroll()
{
   float   pos;
   int     n;

   if ( m_scroll == 0 )  return;

   n = m_totalLine-m_displayLine;
   pos = m_scroll->RetVisibleValue();
   pos += m_scroll->RetArrowStep()/2.0f;  // c'est magique !
   m_firstLine = (int)(pos*n);
   m_firstLine = (m_firstLine/m_multiple)*m_multiple;
   if ( m_firstLine < 0 )  m_firstLine = 0;
   if ( m_firstLine > n )  m_firstLine = n;
}


