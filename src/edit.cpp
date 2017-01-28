// edit.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "scroll.h"
#include "text.h"
#include "edit.h"


#define MARGX			(5.0f/640.0f)
#define MARGY			(5.0f/480.0f)
#define MARGYS			(4.0f/480.0f)
#define MARGY1			(1.0f/480.0f)
#define DELAY_DBCLICK	0.3f			// délai pour double-clic
#define DELAY_SCROLL	0.1f			// délai pour défilement
#define BIG_FONT		1.6f			// agrandissement pour \b;




// Indique si un caractère est un espace.

BOOL IsSpace(int character)
{
	return ( character == ' '  ||
			 character == '\t' ||
			 character == '\n' );
}

// Indique si un caractère fait partie d'un mot.

BOOL IsWord(int character)
{
	char	c;

	c = tolower(RetNoAccent(character));

	return ( (c >= 'a' && c <= 'z') ||
			 (c >= '0' && c <= '9') ||
			 c == '_' );
}

// Indique si un caractère est un séparateur de mot.

BOOL IsSep(int character)
{
	if ( IsSpace(character) )  return FALSE;
	return !IsWord(character);
}





// Constructeur de l'objet.

CEdit::CEdit(CInstanceManager* iMan) : CControl(iMan)
{
	FPOINT	pos;
	int		i;

	CControl::CControl(iMan);

	m_maxChar = 100;
	m_text = (char*)malloc(sizeof(char)*(m_maxChar+1));
	m_format = 0;
	m_len = 0;

	m_fontType = FONT_COLOBOT;
	m_scroll        = 0;
	m_bEdit         = TRUE;
	m_bFilename     = FALSE;
	m_bHilite       = TRUE;
	m_bInsideScroll = TRUE;
	m_bCapture      = FALSE;
	m_bDisplaySpec  = FALSE;
	m_bSoluce       = FALSE;
	m_bGeneric      = FALSE;
	m_bAutoIndent   = FALSE;
	m_cursor1       = 0;
	m_cursor2       = 0;
	m_column        = 0;
	m_imageTotal    = 0;

	for ( i=0 ; i<EDITUNDOMAX ; i++ )
	{
		m_undo[i].text = 0;
	}
	m_bUndoForce = TRUE;
	m_undoOper = OPERUNDO_SPEC;
}

// Destructeur de l'objet.

CEdit::~CEdit()
{
	int		i;

	FreeImage();

	for ( i=0 ; i<EDITUNDOMAX ; i++ )
	{
		delete m_undo[i].text;
	}

	delete m_text;
	delete m_format;
	delete m_scroll;
	CControl::~CControl();
}


// Crée une nouvelle ligne éditable.

BOOL CEdit::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	CScroll*	pc;
	FPOINT		start, end;

	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();
	CControl::Create(pos, dim, icon, eventMsg);

	m_len = 0;
	m_lineFirst = 0;
	m_time = 0.0f;
	m_timeBlink = 0.0f;
	m_timeLastClick = 0.0f;
	m_timeLastScroll = 0.0f;

	m_bMulti = FALSE;
	MoveAdjust();
//?	if ( m_lineVisible <= 1 )
	if ( dim.y <= 32.0f/480.0f )
	{
		m_bMulti = FALSE;
	}
	else
	{
		m_bMulti = TRUE;
		MoveAdjust();  // réajuste en mode multi-lignes
		m_scroll = new CScroll(m_iMan);
		pc = (CScroll*)m_scroll;
		pc->Create(pos, dim, -1, EVENT_NULL);
		pc->SetState(STATE_SHADOW);
		MoveAdjust();
	}

	return TRUE;
}


void CEdit::SetPos(FPOINT pos)
{
	CControl::SetPos(pos);
	MoveAdjust();
}

void CEdit::SetDim(FPOINT dim)
{
	CControl::SetDim(dim);
	MoveAdjust();
}

void CEdit::MoveAdjust()
{
	FPOINT		pos, dim;
	float		height;

	m_lineDescent = m_engine->RetText()->RetDescent(m_fontSize, m_fontType);
	m_lineAscent  = m_engine->RetText()->RetAscent(m_fontSize, m_fontType);
	m_lineHeight  = m_engine->RetText()->RetHeight(m_fontSize, m_fontType);

	height = m_dim.y-(m_bMulti?MARGY*2.0f:MARGY1);
	m_lineVisible = (int)(height/m_lineHeight);

	if ( m_scroll != 0 )
	{
		if ( m_bInsideScroll )
		{
			pos.x = m_pos.x+m_dim.x-MARGX-SCROLL_WIDTH;
			pos.y = m_pos.y+MARGYS;
			dim.x = SCROLL_WIDTH;
			dim.y = m_dim.y-MARGYS*2.0f;
		}
		else
		{
			pos.x = m_pos.x+m_dim.x-SCROLL_WIDTH;
			pos.y = m_pos.y;
			dim.x = SCROLL_WIDTH;
			dim.y = m_dim.y;
		}
		m_scroll->SetPos(pos);
		m_scroll->SetDim(dim);
	}

	Justif();

	if ( m_lineFirst > m_lineTotal-m_lineVisible )
	{
		m_lineFirst = m_lineTotal-m_lineVisible;
		if ( m_lineFirst < 0 )  m_lineFirst = 0;
	}

	pos.x = m_pos.x+m_dim.x-(m_bMulti?SCROLL_WIDTH:0.0f);
	pos.y = m_pos.y;
	GlintCreate(pos, FALSE, FALSE);
}


// Gestion d'un événement.

BOOL CEdit::EventProcess(const Event &event)
{
	BOOL	bShift, bControl;

	if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;

	if ( event.event == EVENT_KEYDOWN &&
		 event.param == VK_WHEELUP    &&
		 Detect(event.pos)            )
	{
		Scroll(m_lineFirst-3, TRUE);
		return TRUE;
	}
	if ( event.event == EVENT_KEYDOWN &&
		 event.param == VK_WHEELDOWN  &&
		 Detect(event.pos)            )
	{
		Scroll(m_lineFirst+3, TRUE);
		return TRUE;
	}

	CControl::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		m_time += event.rTime;
		m_timeBlink += event.rTime;
	}

	if ( event.event == EVENT_MOUSEMOVE )
	{
		if ( Detect(event.pos) &&
			 event.pos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
		{
			if ( m_bEdit )
			{
				m_engine->SetMouseType(D3DMOUSEEDIT);
			}
			else
			{
				m_engine->SetMouseType(D3DMOUSENORM);
			}
		}
	}

	if ( m_scroll != 0 && !m_bGeneric )
	{
		m_scroll->EventProcess(event);

		if ( event.event == m_scroll->RetEventMsg() )
		{
			Scroll();
			return TRUE;
		}
	}

	if ( event.event == EVENT_KEYDOWN && m_bFocus )
	{
		bShift   = (event.keyState&KS_SHIFT);
		bControl = (event.keyState&KS_CONTROL);

		if ( (event.param == 'X'       && !bShift &&  bControl) ||
			 (event.param == VK_DELETE &&  bShift && !bControl) )
		{
			Cut();
			return TRUE;
		}
		if ( (event.param == 'C'       && !bShift &&  bControl) ||
			 (event.param == VK_INSERT && !bShift &&  bControl) )
		{
			Copy();
			return TRUE;
		}
		if ( (event.param == 'V'       && !bShift &&  bControl) ||
			 (event.param == VK_INSERT &&  bShift && !bControl) )
		{
			Paste();
			return TRUE;
		}

		if ( event.param == 'A' && !bShift && bControl )
		{
			SetCursor(999999, 0);
			return TRUE;
		}

		if ( event.param == 'Z' && !bShift && bControl )
		{
			Undo();
			return TRUE;
		}

		if ( event.param == 'U' && !bShift && bControl )
		{
			if ( MinMaj(FALSE) )  return TRUE;
		}
		if ( event.param == 'U' && bShift && bControl )
		{
			if ( MinMaj(TRUE) )  return TRUE;
		}

		if ( m_bEdit )
		{
			if ( event.param == VK_LEFT )
			{
				MoveChar(-1, bControl, bShift);
				return TRUE;
			}
			if ( event.param == VK_RIGHT )
			{
				MoveChar(1, bControl, bShift);
				return TRUE;
			}
			if ( event.param == VK_UP )
			{
				MoveLine(-1, bControl, bShift);
				return TRUE;
			}
			if ( event.param == VK_DOWN )
			{
				MoveLine(1, bControl, bShift);
				return TRUE;
			}

			if ( event.param == VK_PRIOR )  // PageUp ?
			{
				MoveLine(-(m_lineVisible-1), bControl, bShift);
				return TRUE;
			}
			if ( event.param == VK_NEXT )  // PageDown ?
			{
				MoveLine(m_lineVisible-1, bControl, bShift);
				return TRUE;
			}
		}
		else
		{
			if ( event.param == VK_LEFT ||
				 event.param == VK_UP   )
			{
				Scroll(m_lineFirst-1, TRUE);
				return TRUE;
			}
			if ( event.param == VK_RIGHT ||
				 event.param == VK_DOWN  )
			{
				Scroll(m_lineFirst+1, TRUE);
				return TRUE;
			}

			if ( event.param == VK_PRIOR )  // PageUp ?
			{
				Scroll(m_lineFirst-(m_lineVisible-1), TRUE);
				return TRUE;
			}
			if ( event.param == VK_NEXT )  // PageDown ?
			{
				Scroll(m_lineFirst+(m_lineVisible-1), TRUE);
				return TRUE;
			}
		}

		if ( event.param == VK_HOME )
		{
			MoveHome(bControl, bShift);
			return TRUE;
		}
		if ( event.param == VK_END )
		{
			MoveEnd(bControl, bShift);
			return TRUE;
		}

		if ( event.param == VK_BACK )  // backspace ( <- ) ?
		{
			Delete(-1);
			SendModifEvent();
			return TRUE;
		}
		if ( event.param == VK_DELETE )
		{
			Delete(1);
			SendModifEvent();
			return TRUE;
		}

		if ( event.param == VK_RETURN )
		{
			Insert('\n');
			SendModifEvent();
			return TRUE;
		}
		if ( event.param == VK_TAB )
		{
//?			Insert('\t');
//?			SendModifEvent();
			return TRUE;
		}
	}

	if ( event.event == EVENT_CHAR && m_bFocus && m_bEdit )
	{
		if ( event.param >= ' ' && event.param <= 255 )
		{
			Insert((char)event.param);
			SendModifEvent();
			return TRUE;
		}
	}

	if ( event.event == EVENT_FOCUS )
	{
		if ( event.param == m_eventMsg )
		{
			m_bFocus = TRUE;
		}
		else
		{
			m_bFocus = FALSE;
		}
	}

	if ( event.event == EVENT_LBUTTONDOWN )
	{
		m_mouseFirstPos = event.pos;
		m_mouseLastPos  = event.pos;
		if ( Detect(event.pos) )
		{
			if ( event.pos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
			{
				MouseClick(event.pos);
				if ( m_bEdit || m_bHilite )  m_bCapture = TRUE;
			}
			m_bFocus = TRUE;
		}
		else
		{
			m_bFocus = FALSE;
		}
	}

	if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
	{
		m_mouseLastPos = event.pos;
		MouseMove(event.pos);
	}

	if ( event.event == EVENT_FRAME && m_bCapture )
	{
		MouseMove(m_mouseLastPos);
	}

	if ( event.event == EVENT_LBUTTONUP )
	{
		if ( Detect(event.pos) )
		{
			if ( event.pos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
			{
				MouseRelease(m_mouseFirstPos);
			}
		}
		if ( m_bCapture )
		{
			if ( m_timeLastClick+DELAY_DBCLICK > m_time )  // double-clic ?
			{
				MouseDoubleClick(event.pos);
			}
			m_timeLastClick = m_time;
			m_bCapture = FALSE;
		}
	}

	return TRUE;
}


// Envoie un événement pour indiquer que le texte a été modifié.

void CEdit::SendModifEvent()
{
	Event	newEvent;

	m_event->MakeEvent(newEvent, m_eventMsg);
	m_event->AddEvent(newEvent);
}


// Positionne le curseur suite à un double-clic.

void CEdit::MouseDoubleClick(FPOINT mouse)
{
	int		i, character;

	if ( m_bMulti )  // multi-lignes ?
	{
		i = MouseDetect(mouse);
		if ( i == -1 )  return;

		while ( i > 0 )
		{
			character = (unsigned char)m_text[i-1];
			if ( !IsWord(character) )  break;
			i --;
		}
		m_cursor2 = i;

		while ( i < m_len )
		{
			character = (unsigned char)m_text[i];
			if ( !IsWord(character) )  break;
			i ++;
		}
		m_cursor1 = i;
	}
	else	// mono-ligne ?
	{
		m_cursor2 = 0;
		m_cursor1 = m_len;  // sélectionne tout
	}

	m_bUndoForce = TRUE;

	Justif();
	ColumnFix();
}

// Positionne le curseur suite à un clic.

void CEdit::MouseClick(FPOINT mouse)
{
	int		i;

	i = MouseDetect(mouse);
	if ( i == -1 )  return;

	if ( m_bEdit || m_bHilite )
	{
		m_cursor1 = i;
		m_cursor2 = i;
		m_bUndoForce = TRUE;
		m_timeBlink = 0.0f;  // allume le curseur immédiatement
		ColumnFix();
	}
}

// Positionne le curseur suite à un clic relâché.

void CEdit::MouseRelease(FPOINT mouse)
{
}

// Positionne le curseur suite à un déplacement.

void CEdit::MouseMove(FPOINT mouse)
{
	int		i;

	if ( m_bMulti &&
		 m_timeLastScroll+DELAY_SCROLL <= m_time )
	{
		if ( mouse.y > m_pos.y+m_dim.y )  // plus haut ?
		{
			Scroll(m_lineFirst-1, FALSE);
			mouse.y = m_pos.y+m_dim.y-MARGY-m_lineHeight/2.0f;
		}
		if ( mouse.y < m_pos.y )  // plus bas ?
		{
			Scroll(m_lineFirst+1, FALSE);
			mouse.y = m_pos.y+m_dim.y-MARGY-m_lineVisible*m_lineHeight+m_lineHeight/2.0f;
		}
		m_timeLastScroll = m_time;
	}

	i = MouseDetect(mouse);
	if ( i != -1 )
	{
		m_cursor1 = i;
		m_bUndoForce = TRUE;
		m_timeBlink = 0.0f;  // allume le curseur immédiatement
		ColumnFix();
	}
}

// Positionne le curseur suite à un clic.

int CEdit::MouseDetect(FPOINT mouse)
{
	FPOINT	pos;
	float	indentLength, offset, size;
	int		i, len, c;
	BOOL	bTitle;

	if ( m_bAutoIndent )
	{
		indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
						* m_engine->RetEditIndentValue();
	}

	pos.y = m_pos.y+m_dim.y-m_lineHeight-(m_bMulti?MARGY:MARGY1);
	for ( i=m_lineFirst ; i<m_lineTotal ; i++ )
	{
		bTitle = ( m_format != 0 && (m_format[m_lineOffset[i]]&TITLE_MASK) == TITLE_BIG );

		if ( i >= m_lineFirst+m_lineVisible )  break;

		pos.x = m_pos.x+(10.0f/640.0f);
		if ( m_bAutoIndent )
		{
			pos.x += indentLength*m_lineIndent[i];
		}
		offset = mouse.x-pos.x;

		if ( bTitle )  pos.y -= m_lineHeight;

		if ( mouse.y > pos.y )
		{
			len = m_lineOffset[i+1] - m_lineOffset[i];

			if ( m_format == 0 )
			{
				c = m_engine->RetText()->Detect(m_text+m_lineOffset[i],
												len, offset, m_fontSize,
												m_fontStretch, m_fontType);
			}
			else
			{
				size = m_fontSize;
				if ( bTitle )  size *= BIG_FONT;

				c = m_engine->RetText()->Detect(m_text+m_lineOffset[i],
												m_format+m_lineOffset[i],
												len, offset, size,
												m_fontStretch);
			}
			return m_lineOffset[i]+c;
		}

		if ( bTitle )  i ++;
		pos.y -= m_lineHeight;
	}
	return -1;
}


// Dessine la ligne éditable.

void CEdit::Draw()
{
	FPOINT		pos, ppos, dim, start, end;
	float		size, indentLength;
	int			i, j, beg, len, c1, c2, o1, o2, eol, iIndex, line;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	if ( m_state & STATE_SHADOW )
	{
		DrawShadow(m_pos, m_dim);
	}

	pos.x = m_pos.x;
	pos.y = m_pos.y;
	dim.x = m_dim.x;
	if ( !m_bInsideScroll )  dim.x -= m_bMulti?SCROLL_WIDTH:0.0f;
	dim.y = m_dim.y;
	DrawBack(pos, dim);  // fond

	// Affiche toutes les lignes.
	c1 = m_cursor1;
	c2 = m_cursor2;
	if ( c1 > c2 )  Swap(c1, c2);  // toujours c1 <= c2

	if ( m_bInsideScroll )
	{
		dim.x -= m_bMulti?SCROLL_WIDTH:0.0f + (1.0f/640.0f);
	}

	if ( m_bAutoIndent )
	{
		indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
						* m_engine->RetEditIndentValue();
	}

	pos.y = m_pos.y+m_dim.y-m_lineHeight-(m_bMulti?MARGY:MARGY1);
	for ( i=m_lineFirst ; i<m_lineTotal ; i++ )
	{
		if ( i == m_lineFirst && i < m_lineTotal-1 &&
			 m_lineOffset[i] == m_lineOffset[i+1] )
		{
			pos.y -= m_lineHeight;  // saute double ligne \b;
			i ++;
		}

		if ( i >= m_lineFirst+m_lineVisible )  break;

		pos.x = m_pos.x+(10.0f/640.0f);
		if ( m_bAutoIndent )
		{
			for ( j=0 ; j<m_lineIndent[i] ; j++ )
			{
				char s = '\t';  // ligne | pointillée
				m_engine->RetText()->DrawText(&s, 1, pos, 1.0f, 1, m_fontSize, m_fontStretch, m_fontType, 0);
				pos.x += indentLength;
			}
		}

		beg = m_lineOffset[i];
		len = m_lineOffset[i+1] - m_lineOffset[i];

		ppos = pos;
		size = m_fontSize;

		// Grand titre \b; ?
		if ( beg+len < m_len && m_format != 0 &&
			 (m_format[beg]&TITLE_MASK) == TITLE_BIG )
		{
			start.x = ppos.x-MARGX;
			end.x   = dim.x-MARGX*2.0f;
			start.y = ppos.y-(m_bMulti?0.0f:MARGY1)-m_lineHeight*(BIG_FONT-1.0f);
			end.y   = m_lineHeight*BIG_FONT;
			DrawPart(start, end, 2);  // fond bleu dégradé ->

			size *= BIG_FONT;
			ppos.y -= m_lineHeight*(BIG_FONT-1.0f);
		}

		// Titre \t; ?
		if ( beg+len < m_len && m_format != 0 &&
			 (m_format[beg]&TITLE_MASK) == TITLE_NORM )
		{
			start.x = ppos.x-MARGX;
			end.x   = dim.x-MARGX*2.0f;
			start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
			end.y   = m_lineHeight;
			DrawPart(start, end, 2);  // fond bleu dégradé ->
		}

		// Sous-titre \s; ?
		if ( beg+len < m_len && m_format != 0 &&
			 (m_format[beg]&TITLE_MASK) == TITLE_LITTLE )
		{
			start.x = ppos.x-MARGX;
			end.x   = dim.x-MARGX*2.0f;
			start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
			end.y   = m_lineHeight;
			DrawPart(start, end, 3);  // fond jaune dégradé ->
		}

		// Tableau \tab; ?
		if ( beg+len < m_len && m_format != 0 &&
			 (m_format[beg]&COLOR_MASK) == COLOR_TABLE )
		{
			start.x = ppos.x-MARGX;
			end.x   = dim.x-MARGX*2.0f;
			start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
			end.y   = m_lineHeight;
			DrawPart(start, end, 11);  // fond orange dégradé ->
		}

		// Image \image; ?
		if ( beg+len < m_len && m_format != 0 &&
			 (m_format[beg]&IMAGE_MASK) != 0 )
		{
			line = 1;
			while ( true )  // regroupe les tranches d'image
			{
				if ( i+line >= m_lineTotal                ||
					 i+line >= m_lineFirst+m_lineVisible  ||
					 (m_format[beg+line]&IMAGE_MASK) == 0 )  break;
				line ++;
			}

			iIndex = m_text[beg];  // caractère = index dans m_image
			pos.y -= m_lineHeight*(line-1);
			DrawImage(pos, m_image[iIndex].name,
					  m_image[iIndex].width*(m_fontSize/SMALLFONT),
					  m_image[iIndex].offset, m_image[iIndex].height*line, line);
			pos.y -= m_lineHeight;
			i += line-1;
			continue;
		}

		if ( ((m_bEdit && m_bFocus && m_bHilite) ||
			  (!m_bEdit && m_bHilite)            ) &&
			 c1 != c2 && beg <= c2 && beg+len >= c1 )  // zone sélectionnée ?
		{
			o1 = c1;  if ( o1 < beg     )  o1 = beg;
			o2 = c2;  if ( o2 > beg+len )  o2 = beg+len;

			if ( m_format == 0 )
			{
				start.x = ppos.x+m_engine->RetText()->RetStringWidth(m_text+beg, o1-beg, size, m_fontStretch, m_fontType);
				end.x   = m_engine->RetText()->RetStringWidth(m_text+o1, o2-o1, size, m_fontStretch, m_fontType);
			}
			else
			{
				start.x = ppos.x+m_engine->RetText()->RetStringWidth(m_text+beg, m_format+beg, o1-beg, size, m_fontStretch);
				end.x   = m_engine->RetText()->RetStringWidth(m_text+o1, m_format+o1, o2-o1, size, m_fontStretch);
			}

			start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
			end.y   = m_lineHeight;
			if ( m_format != 0 && (m_format[beg]&TITLE_MASK) == TITLE_BIG )  end.y *= BIG_FONT;
			DrawPart(start, end, 1);  // fond jaune uni
		}

		eol = 16;  // >
		if ( len > 0 && m_text[beg+len-1] == '\n' )
		{
			len --;  // n'affiche pas le '\n'
			eol = 0;  // rien
		}
		if ( beg+len >= m_len )
		{
			eol = 2;  // carré (eot)
		}
		if ( !m_bMulti || !m_bDisplaySpec )  eol = 0;
		if ( m_format == 0 )
		{
			m_engine->RetText()->DrawText(m_text+beg, len, ppos, m_dim.x, 1, size, m_fontStretch, m_fontType, eol);
		}
		else
		{
			m_engine->RetText()->DrawText(m_text+beg, m_format+beg, len, ppos, m_dim.x, 1, size, m_fontStretch, eol);
		}

		pos.y -= m_lineHeight;

		if ( i < m_lineTotal-2 && m_lineOffset[i+1] == m_lineOffset[i+2] )
		{
			pos.y -= m_lineHeight;  // saute double ligne \b;
			i ++;
		}
	}

	// Affiche le curseur.
	if ( (m_bEdit && m_bFocus && m_bHilite && Mod(m_timeBlink, 1.0f) <= 0.5f) )  // ça clignotte
	{
		pos.y = m_pos.y+m_dim.y-m_lineHeight-(m_bMulti?MARGY:MARGY1*2.0f);
		for ( i=m_lineFirst ; i<m_lineTotal ; i++ )
		{
			if ( i == m_lineTotal-1 || m_cursor1 < m_lineOffset[i+1] )
			{
				pos.x = m_pos.x+(10.0f/640.0f);
				if ( m_bAutoIndent )
				{
					pos.x += indentLength*m_lineIndent[i];
				}

				len = m_cursor1 - m_lineOffset[i];

				if ( m_format == 0 )
				{
					m_engine->RetText()->DimText(m_text+m_lineOffset[i], len,
												 pos, 1, size,
												 m_fontStretch, m_fontType,
												 start, end);
				}
				else
				{
					m_engine->RetText()->DimText(m_text+m_lineOffset[i],
												 m_format+m_lineOffset[i],
												 len, pos, 1, size,
												 m_fontStretch,
												 start, end);
				}

				pos.x = end.x;
				break;
			}
			pos.y -= m_lineHeight;
		}
		pos.x -= 1.0f/640.0f;
		dim.x = 2.0f/640.0f;
		dim.y = m_lineHeight;
		DrawPart(pos, dim, 0);  // rouge
	}

	if ( m_scroll != 0 && !m_bGeneric )
	{
		m_scroll->Draw();
	}
}

// Dessine une partie d'image.

void CEdit::DrawImage(FPOINT pos, char *name, float width,
					  float offset, float height, int nbLine)
{
	FPOINT		uv1, uv2, dim;
	float		dp;
	char		filename[100];

//?	sprintf(filename, "diagram\\%s.bmp", name);
	UserDir(filename, name, "diagram");
	strcat(filename, ".bmp");

	m_engine->SetTexture(filename);
	m_engine->SetState(D3DSTATENORMAL);

	uv1.x = 0.0f;
	uv2.x = 1.0f;
	uv1.y = offset;
	uv2.y = offset+height;

	dp = 0.5f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	dim.x = width;
	dim.y = m_lineHeight*nbLine;
	DrawIcon(pos, dim, uv1, uv2);
}

// Dessine le fond.

void CEdit::DrawBack(FPOINT pos, FPOINT dim)
{
	FPOINT		uv1,uv2, corner;
	float		dp;

	if ( m_bGeneric )  return;

	m_engine->SetTexture("button1.tga");
//?	m_engine->SetState(D3DSTATENORMAL);
	m_engine->SetState(D3DSTATETTb);

	if ( m_state & STATE_CHECK )
	{
		uv1.x =  64.0f/256.0f;
		uv1.y =  32.0f/256.0f;
		uv2.x =  96.0f/256.0f;
		uv2.y =  64.0f/256.0f;
	}
	else
	{
		uv1.x = 224.0f/256.0f;
		uv1.y =  96.0f/256.0f;
		uv2.x = 256.0f/256.0f;
		uv2.y = 128.0f/256.0f;
	}

	dp = 0.5f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	if ( m_bMulti )
	{
		corner.x = 10.0f/640.0f;
		corner.y = 10.0f/480.0f;
		DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
	}
	else
	{
		DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
	}
}

// Dessine une icône de fond.

void CEdit::DrawPart(FPOINT pos, FPOINT dim, int icon)
{
	FPOINT		uv1, uv2;
	float		dp;

	m_engine->SetTexture("text.tga");
	m_engine->SetState(D3DSTATENORMAL);

	uv1.x = (64.0f/256.0f)+(16.0f/256.0f)*(icon%16);
	uv1.y = (196.0f/256.0f);
	uv2.x = (16.0f/256.0f)+uv1.x;
	uv2.y = (10.0f/256.0f)+uv1.y;

	dp = 0.5f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	DrawIcon(pos, dim, uv1, uv2);
}


// Donne le texte à éditer.

void CEdit::SetText(char *text, BOOL bNew)
{
	KeyRank		key;
	char		iName[50];
	int			i, j, font, n, res[2];
	BOOL		bBOL;

	if ( !bNew )  UndoMemorize(OPERUNDO_SPEC);

	m_len = strlen(text);
	if ( m_len > m_maxChar )  m_len = m_maxChar;

	if ( m_format == 0 )
	{
		if ( m_bAutoIndent )
		{
			j = 0;
			bBOL = TRUE;
			for ( i=0 ; i<m_len ; i++ )
			{
				if ( text[i] == '\t' )
				{
					if ( !bBOL )  m_text[j++] = ' ';
					continue;  // enlève les tabulateurs
				}
				bBOL = ( text[i] == '\n' );

				m_text[j++] = text[i];
			}
			m_len = j;
		}
		else
		{
			strncpy(m_text, text, m_len);
		}
	}
	else
	{
		font = m_fontType;
		j = 0;
		bBOL = TRUE;
		for ( i=0 ; i<m_len ; i++ )
		{
			if ( m_bAutoIndent )
			{
				if ( text[i] == '\t' )
				{
					if ( !bBOL )
					{
						m_text[j] = ' ';
						m_format[j] = font;
						j ++;
					}
					continue;  // enlève les tabulateurs
				}
				bBOL = ( text[i] == '\n' );
			}

			if ( text[i] == '\\' && text[i+2] == ';' )
			{
				if ( text[i+1] == 'n' )  // normal ?
				{
					font &= ~FONT_MASK;
					font |= FONT_COLOBOT;
					i += 2;
				}
				else if ( text[i+1] == 'c' )  // cbot ?
				{
					font &= ~FONT_MASK;
					font |= FONT_COLOBOT;
					i += 2;
				}
				else if ( text[i+1] == 'b' )  // big title ?
				{
					font &= ~TITLE_MASK;
					font |= TITLE_BIG;
					i += 2;
				}
				else if ( text[i+1] == 't' )  // title ?
				{
					font &= ~TITLE_MASK;
					font |= TITLE_NORM;
					i += 2;
				}
				else if ( text[i+1] == 's' )  // sbttl ?
				{
					font &= ~TITLE_MASK;
					font |= TITLE_LITTLE;
					i += 2;
				}
			}
			else if ( text[i+0] == '\\' &&  // \key name; ?
					  text[i+1] == 'k'  &&
					  text[i+2] == 'e'  &&
					  text[i+3] == 'y'  &&
					  text[i+4] == ' '  )
			{
				if ( SearchKey(text+i+5, key) )
				{
					RetKeys(key, res);
					if ( res[0] != 0 )
					{
						if ( GetResource(RES_KEY, res[0], iName) )
						{
							n = 0;
							while ( iName[n] != 0 )
							{
								m_text[j] = iName[n++];
								m_format[j] = font;
								j ++;
							}

							if ( res[1] != 0 )
							{
								if ( GetResource(RES_KEY, res[1], iName) )
								{
									GetResource(RES_TEXT, RT_KEY_OR, text);
									n = 0;
									while ( text[n] != 0 )
									{
										m_text[j] = text[n++];
										m_format[j] = font&~COLOR_MASK;
										j ++;
									}
									n = 0;
									while ( iName[n] != 0 )
									{
										m_text[j] = iName[n++];
										m_format[j] = font;
										j ++;
									}
								}
							}
							while ( text[i++] != ';' );
							i--;
							continue;
						}
					}
				}
				m_text[j] = '?';
				m_format[j] = font;
				j ++;
				while ( text[i++] != ';' );
				i --;
			}
			else
			{
				m_text[j] = text[i];
				m_format[j] = font;
				j ++;

				font &= ~TITLE_MASK;  // reset title
			}
		}
		m_len = j;
	}

	if ( bNew )  UndoFlush();

	m_cursor1 = 0;
	m_cursor2 = 0;  // curseur au début
	Justif();
	ColumnFix();
}

// Retourne un pointeur au texte édité.

char* CEdit::RetText()
{
	m_text[m_len] = 0;
	return m_text;
}

// Retourne le texte édité.

void CEdit::GetText(char *buffer, int max)
{
	if ( m_len < max )  max = m_len;
	if ( m_len > max )  max = max-1;

	strncpy(buffer, m_text, max);
	buffer[max] = 0;
}

// Retourne la longueur du texte.

int CEdit::RetTextLength()
{
	return m_len;
}



// Retourne un nom dans une commande.
// \x nom1 nom2 nom3;

void GetNameParam(char *cmd, int rank, char *buffer)
{
	int		i;

	for ( i=0 ; i<rank ; i++ )
	{
		while ( *cmd != ' ' && *cmd != ';' )
		{
			cmd ++;
		}
		if ( *cmd != ';' )  cmd ++;
	}

	while ( *cmd != ' ' && *cmd != ';' )
	{
		*buffer++ = *cmd++;
	}
	*buffer = 0;
}

// Retourne un nombre dans une commande.
// \x nom n1 n2;

int RetValueParam(char *cmd, int rank)
{
	int		n, i;

	for ( i=0 ; i<rank ; i++ )
	{
		while ( *cmd != ' ' && *cmd != ';' )
		{
			cmd ++;
		}
		if ( *cmd != ';' )  cmd ++;
	}

	sscanf(cmd, "%d", &n);
	return n;
}

// Libère toutes les images.

void CEdit::FreeImage()
{
	char	filename[100];
	int		i;

	for ( i=0 ; i<m_imageTotal ; i++ )
	{
//?		sprintf(filename, "diagram\\%s.bmp", m_image[i].name);
		UserDir(filename, m_image[i].name, "diagram");
		strcat(filename, ".bmp");
		m_engine->FreeTexture(filename);
	}
}

// Lit la texture d'une image.

void CEdit::LoadImage(char *name)
{
	char	filename[100];

//?	sprintf(filename, "diagram\\%s.bmp", name);
	UserDir(filename, name, "diagram");
	strcat(filename, ".bmp");
	m_engine->LoadTexture(filename);
}

// Lit le contenu d'un fichier texte.

BOOL CEdit::ReadText(char *filename, int addSize)
{
	FILE		*file = NULL;
	char		*buffer;
	int			len, i, j, n, font, iIndex, iLines, iCount, iLink, res[2];
	char		iName[50];
	char		text[50];
	float		iWidth;
	KeyRank		key;
	BOOL		bInSoluce, bBOL;

	if ( filename[0] == 0 )  return FALSE;
	file = fopen(filename, "rb");
	if ( file == NULL )  return FALSE;

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	fseek(file, 0, SEEK_SET);

	m_maxChar = len+addSize+100;
	m_len = len;
	m_cursor1 = 0;
	m_cursor2 = 0;

	FreeImage();
	delete m_text;
	m_text = (char*)malloc(sizeof(char)*(m_maxChar+1));
	buffer = (char*)malloc(sizeof(char)*(m_maxChar+1));
	fread(buffer, 1, len, file);

	if ( m_format != 0 )
	{
		delete m_format;
		m_format = (char*)malloc(sizeof(char)*m_maxChar);
	}

	fclose(file);

	bInSoluce = FALSE;
	font = m_fontType;
	iIndex = 0;
	iLink = 0;
	m_imageTotal = 0;
	i = j = 0;
	bBOL = TRUE;
	while ( i < m_len )
	{
		if ( m_bAutoIndent )
		{
			if ( buffer[i] == '\t' )
			{
				if ( !bBOL )
				{
					m_text[j] = buffer[i];
					if ( m_format != 0 )  m_format[j] = font;
					j ++;
				}
				i ++;
				continue;  // enlève les tabulateurs
			}
			bBOL = ( buffer[i] == '\n' || buffer[i] == '\r' );
		}

		if ( buffer[i] == '\r' )  // supprime les \r
		{
			i ++;
		}
		else if ( m_format != 0 && buffer[i] == '\\' && buffer[i+2] == ';' )
		{
			if ( buffer[i+1] == 'n' )  // normal ?
			{
				if ( m_bSoluce || !bInSoluce )
				{
					font &= ~FONT_MASK;
					font |= FONT_COLOBOT;
				}
				i += 3;
			}
			else if ( buffer[i+1] == 'c' )  // cbot ?
			{
				if ( m_bSoluce || !bInSoluce )
				{
					font &= ~FONT_MASK;
					font |= FONT_COLOBOT;
				}
				i += 3;
			}
			else if ( buffer[i+1] == 'b' )  // big title ?
			{
				if ( m_bSoluce || !bInSoluce )
				{
					font &= ~TITLE_MASK;
					font |= TITLE_BIG;
				}
				i += 3;
			}
			else if ( buffer[i+1] == 't' )  // title ?
			{
				if ( m_bSoluce || !bInSoluce )
				{
					font &= ~TITLE_MASK;
					font |= TITLE_NORM;
				}
				i += 3;
			}
			else if ( buffer[i+1] == 's' )  // sbttl ?
			{
				if ( m_bSoluce || !bInSoluce )
				{
					font &= ~TITLE_MASK;
					font |= TITLE_LITTLE;
				}
				i += 3;
			}
			else if ( buffer[i+1] == 'l' )  // link ?
			{
				if ( m_bSoluce || !bInSoluce )
				{
					font &= ~COLOR_MASK;
					font |= COLOR_LINK;
				}
				i += 3;
			}
			else
			{
				i += 3;
			}
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \image nom lx ly; ?
				  buffer[i+1] == 'i'  &&
				  buffer[i+2] == 'm'  &&
				  buffer[i+3] == 'a'  &&
				  buffer[i+4] == 'g'  &&
				  buffer[i+5] == 'e'  &&
				  buffer[i+6] == ' '  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
#if _DEMO | _SE
				strcpy(iName, "demo");
#else
				GetNameParam(buffer+i+7, 0, iName);
#endif
//?				iWidth = m_lineHeight*RetValueParam(buffer+i+7, 1);
				iWidth = (float)RetValueParam(buffer+i+7, 1);
				iWidth *= m_engine->RetText()->RetHeight(SMALLFONT, FONT_COLOBOT);
				iLines = RetValueParam(buffer+i+7, 2);
				LoadImage(iName);

				// Une tranche d'image par ligne de texte.
				for ( iCount=0 ; iCount<iLines ; iCount++ )
				{
					strcpy(m_image[iIndex].name, iName);
					m_image[iIndex].offset = (float)iCount/iLines;
					m_image[iIndex].height = 1.0f/iLines;
					m_image[iIndex].width = iWidth*0.75f;

					m_text[j] = (char)(iIndex++);  // un car index dans m_image
					m_format[j] = (unsigned char)IMAGE_MASK;
					j ++;
				}
			}
			i += strchr(buffer+i, ';')-(buffer+i)+1;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \button; ?
				  buffer[i+1] == 'b'  &&
				  buffer[i+2] == 'u'  &&
				  buffer[i+3] == 't'  &&
				  buffer[i+4] == 't'  &&
				  buffer[i+5] == 'o'  &&
				  buffer[i+6] == 'n'  &&
				  buffer[i+7] == ' '  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				m_text[j] = RetValueParam(buffer+i+8, 0);
				m_format[j] = font|FONT_BUTTON;
				j ++;
			}
			i += strchr(buffer+i, ';')-(buffer+i)+1;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \token; ?
				  buffer[i+1] == 't'  &&
				  buffer[i+2] == 'o'  &&
				  buffer[i+3] == 'k'  &&
				  buffer[i+4] == 'e'  &&
				  buffer[i+5] == 'n'  &&
				  buffer[i+6] == ';'  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				font &= ~COLOR_MASK;
				font |= COLOR_TOKEN;
			}
			i += 7;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \type; ?
				  buffer[i+1] == 't'  &&
				  buffer[i+2] == 'y'  &&
				  buffer[i+3] == 'p'  &&
				  buffer[i+4] == 'e'  &&
				  buffer[i+5] == ';'  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				font &= ~COLOR_MASK;
				font |= COLOR_TYPE;
			}
			i += 6;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \const; ?
				  buffer[i+1] == 'c'  &&
				  buffer[i+2] == 'o'  &&
				  buffer[i+3] == 'n'  &&
				  buffer[i+4] == 's'  &&
				  buffer[i+5] == 't'  &&
				  buffer[i+6] == ';'  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				font &= ~COLOR_MASK;
				font |= COLOR_CONST;
			}
			i += 7;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \key; ?
				  buffer[i+1] == 'k'  &&
				  buffer[i+2] == 'e'  &&
				  buffer[i+3] == 'y'  &&
				  buffer[i+4] == ';'  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				font &= ~COLOR_MASK;
				font |= COLOR_KEY;
			}
			i += 5;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \tab; ?
				  buffer[i+1] == 't'  &&
				  buffer[i+2] == 'a'  &&
				  buffer[i+3] == 'b'  &&
				  buffer[i+4] == ';'  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				font |= COLOR_TABLE;
			}
			i += 5;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \norm; ?
				  buffer[i+1] == 'n'  &&
				  buffer[i+2] == 'o'  &&
				  buffer[i+3] == 'r'  &&
				  buffer[i+4] == 'm'  &&
				  buffer[i+5] == ';'  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				font &= ~COLOR_MASK;
			}
			i += 6;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \begin soluce; ?
				  buffer[i+1] == 'b'  &&
				  buffer[i+2] == 's'  &&
				  buffer[i+3] == ';'  )
		{
			bInSoluce = TRUE;
			i += 4;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \end soluce; ?
				  buffer[i+1] == 'e'  &&
				  buffer[i+2] == 's'  &&
				  buffer[i+3] == ';'  )
		{
			bInSoluce = FALSE;
			i += 4;
		}
		else if ( m_format != 0       &&
				  buffer[i+0] == '\\' &&  // \key name; ?
				  buffer[i+1] == 'k'  &&
				  buffer[i+2] == 'e'  &&
				  buffer[i+3] == 'y'  &&
				  buffer[i+4] == ' '  )
		{
			if ( m_bSoluce || !bInSoluce )
			{
				if ( SearchKey(buffer+i+5, key) )
				{
					RetKeys(key, res);
					if ( res[0] != 0 )
					{
						if ( GetResource(RES_KEY, res[0], iName) )
						{
							m_text[j] = ' ';
							m_format[j] = font;
							j ++;
							n = 0;
							while ( iName[n] != 0 )
							{
								m_text[j] = iName[n++];
								m_format[j] = font;
								j ++;
							}
							m_text[j] = ' ';
							m_format[j] = font;
							j ++;

							if ( res[1] != 0 )
							{
								if ( GetResource(RES_KEY, res[1], iName) )
								{
									GetResource(RES_TEXT, RT_KEY_OR, text);
									n = 0;
									while ( text[n] != 0 )
									{
										m_text[j] = text[n++];
										m_format[j] = font&~COLOR_MASK;
										j ++;
									}
									n = 0;
									while ( iName[n] != 0 )
									{
										m_text[j] = iName[n++];
										m_format[j] = font;
										j ++;
									}
									m_text[j] = ' ';
									m_format[j] = font;
									j ++;
								}
							}
							while ( buffer[i++] != ';' );
							continue;
						}
					}
				}
				m_text[j] = '?';
				m_format[j] = font;
				j ++;
			}
			while ( buffer[i++] != ';' );
		}
		else
		{
			if ( m_bSoluce || !bInSoluce )
			{
				m_text[j] = buffer[i];
				if ( m_format != 0 )  m_format[j] = font;
				j ++;
			}
			i ++;

			font &= ~TITLE_MASK;  // reset title

			if ( (font&COLOR_MASK) == COLOR_TABLE )
			{
				font &= ~COLOR_TABLE;
			}
		}
	}
	m_len = j;
	m_imageTotal = iIndex;

	delete buffer;

	Justif();
	ColumnFix();
	return TRUE;
}

// Ecrit tout le texte dans un fichier.

BOOL CEdit::WriteText(char *filename)
{
	FILE*		file;
	char		buffer[1000+20];
	int			i, j, k, n;
	float		iDim;

	if ( filename[0] == 0 )  return FALSE;
	file = fopen(filename, "wb");
	if ( file == NULL )  return FALSE;

	if ( m_bAutoIndent )
	{
		iDim = m_dim.x;
		m_dim.x = 1000.0f;  // met une largeur infinie !
		Justif();
	}

	i = j = k = 0;
	while ( m_text[i] != 0 && i < m_len )
	{
		if ( m_bAutoIndent && i == m_lineOffset[k] )
		{
			for ( n=0 ; n<m_lineIndent[k] ; n++ )
			{
				buffer[j++] = '\t';
			}
			k ++;
		}

		buffer[j++] = m_text[i];

		if ( m_text[i] == '\n' )
		{
			buffer[j-1] = '\r';
			buffer[j++] = '\n';  // \r\n (0x0D, 0x0A)
		}

		if ( j >= 1000-1 )
		{
			fwrite(buffer, 1, j, file);
			j = 0;
		}

		i ++;
	}
	if ( j > 0 )
	{
		fwrite(buffer, 1, j, file);
	}

	fclose(file);

	if ( m_bAutoIndent )
	{
		m_dim.x = iDim;  // remet la largeur initiale
		Justif();
	}

	return TRUE;
}


// Gestion du nombre max de caractères éditables.

void CEdit::SetMaxChar(int max)
{
	m_maxChar = max;
	FreeImage();
	delete m_text;
	m_text = (char*)malloc(sizeof(char)*(m_maxChar+1));

	if ( m_format != 0 )
	{
		delete m_format;
		m_format = (char*)malloc(sizeof(char)*m_maxChar);
	}

	m_len = 0;
	m_cursor1 = 0;
	m_cursor2 = 0;
	Justif();
	UndoFlush();
}

int CEdit::RetMaxChar()
{
	return m_maxChar;
}


// Gestion du mode "éditable".

void CEdit::SetEditCap(BOOL bMode)
{
	m_bEdit = bMode;
}

BOOL CEdit::RetEditCap()
{
	return m_bEdit;
}

// Gestion du mode "nom de fichier".

void CEdit::SetFilenameCap(BOOL bMode)
{
	m_bFilename = bMode;
}

BOOL CEdit::RetFilenameCap()
{
	return m_bFilename;
}

// Gestion du mode "hilitable" (ça c'est du franch).

void CEdit::SetHiliteCap(BOOL bEnable)
{
	m_bHilite = bEnable;
}

BOOL CEdit::RetHiliteCap()
{
	return m_bHilite;
}

// Ascenseur dans/hors cadre.

void CEdit::SetInsideScroll(BOOL bInside)
{
	m_bInsideScroll = bInside;
}

BOOL CEdit::RetInsideScroll()
{
	return m_bInsideScroll;
}

// Indique s'il faut afficher les liens montrant la solution.

void CEdit::SetSoluceMode(BOOL bSoluce)
{
	m_bSoluce = bSoluce;
}

BOOL CEdit::RetSoluceMode()
{
	return m_bSoluce;
}

// Indique si le texte est un générique qui défile.

void CEdit::SetGenericMode(BOOL bGeneric)
{
	m_bGeneric = bGeneric;
}

BOOL CEdit::RetGenericMode()
{
	return m_bGeneric;
}


// Gestion du mode d'indentation automatique avec { }.

void CEdit::SetAutoIndent(BOOL bMode)
{
	m_bAutoIndent = bMode;
}

BOOL CEdit::RetAutoIndent()
{
	return m_bAutoIndent;
}



// Déplace les curseurs.

void CEdit::SetCursor(int cursor1, int cursor2)
{
	if ( cursor1 > m_len )  cursor1 = m_len;
	if ( cursor2 > m_len )  cursor2 = m_len;

	m_cursor1 = cursor1;
	m_cursor2 = cursor2;
	m_bUndoForce = TRUE;
	ColumnFix();
}

// Retourne les curseurs.

void CEdit::GetCursor(int &cursor1, int &cursor2)
{
	cursor1 = m_cursor1;
	cursor2 = m_cursor2;
}


// Modifie la première ligne affichée.

void CEdit::SetFirstLine(int rank)
{
	Scroll(rank, TRUE);
}

// Retourne la première ligne affichée.

int CEdit::RetFirstLine()
{
	return m_lineFirst;
}


// Montre la zone sélectionnée.

void CEdit::ShowSelect()
{
	int		cursor1, cursor2, line;

	if ( m_cursor1 < m_cursor2 )
	{
		cursor1 = m_cursor1;
		cursor2 = m_cursor2;
	}
	else
	{
		cursor1 = m_cursor2;
		cursor2 = m_cursor1;
	}

	line = RetCursorLine(cursor2);
	if ( line >= m_lineFirst+m_lineVisible )
	{
		line -= m_lineVisible-1;
		if ( line < 0 )  line = 0;
		Scroll(line, FALSE);
	}

	line = RetCursorLine(cursor1);
	if ( line < m_lineFirst )
	{
		Scroll(line, FALSE);
	}
}


// Gestion du mode d'affichage des caractères spéciaux.

void CEdit::SetDisplaySpec(BOOL bDisplay)
{
	m_bDisplaySpec = bDisplay;
}

BOOL CEdit::RetDisplaySpec()
{
	return m_bDisplaySpec;
}


// Gestion du mode multi-fontes.

void CEdit::SetMultiFont(BOOL bMulti)
{
	if ( bMulti )
	{
		delete m_format;
		m_format = (char*)malloc(sizeof(char)*m_maxChar);
		memset(m_format, 0, m_maxChar);
	}
	else
	{
		delete m_format;
		m_format = 0;
	}
}

BOOL CEdit::RetMultiFont()
{
	return ( m_format != 0 );
}


// Gestion de la taille des caractères.

void CEdit::SetFontSize(float size)
{
	CControl::SetFontSize(size);

	MoveAdjust();
}


// Déplace la partie visible selon l'ascenseur.

void CEdit::Scroll()
{
	float	value;

	if ( m_scroll != 0 )
	{
		value = m_scroll->RetVisibleValue();
		value *= m_lineTotal-m_lineVisible;
		Scroll((int)(value+0.5f), TRUE);
	}
}

// Déplace la partie visible selon l'ascenseur.

void CEdit::Scroll(int pos, BOOL bAdjustCursor)
{
	int		max, line;

	m_lineFirst = pos;

	if ( m_lineFirst < 0 )  m_lineFirst = 0;

	max = m_lineTotal-m_lineVisible;
	if ( max < 0 )  max = 0;
	if ( m_lineFirst > max )  m_lineFirst = max;

	line = RetCursorLine(m_cursor1);

	if ( bAdjustCursor && m_bEdit )
	{
		// Curseur trop haut ?
		if ( line < m_lineFirst )
		{
			MoveLine(m_lineFirst-line, FALSE, FALSE);
			return;
		}

		// Curseur trop bas ?
		if ( line >= m_lineFirst+m_lineVisible )
		{
			MoveLine(m_lineFirst+m_lineVisible-line-1, FALSE, FALSE);
			return;
		}
	}
	
	Justif();
}

// Déplace le curseur au début de la ligne.

void CEdit::MoveHome(BOOL bWord, BOOL bSelect)
{
	int		begin, tab;

	if ( bWord )
	{
		m_cursor1 = 0;
	}
	else
	{
		begin = m_cursor1;
		while ( begin > 0 && m_text[begin-1] != '\n' )
		{
			begin --;
		}

		tab = begin;
		while ( tab < m_len && (m_text[tab] == '\t' || m_text[tab] == ' ') )
		{
			tab ++;
		}

		if ( m_cursor1 == tab )
		{
			m_cursor1 = begin;
		}
		else
		{
			m_cursor1 = tab;
		}
	}
	if ( !bSelect )  m_cursor2 = m_cursor1;

	m_bUndoForce = TRUE;
	Justif();
	ColumnFix();
}

// Déplace le curseur à la fin de la ligne.

void CEdit::MoveEnd(BOOL bWord, BOOL bSelect)
{
	if ( bWord )
	{
		m_cursor1 = m_len;
	}
	else
	{
		while ( m_cursor1 < m_len && m_text[m_cursor1] != '\n' )
		{
			m_cursor1 ++;
		}
	}
	if ( !bSelect )  m_cursor2 = m_cursor1;

	m_bUndoForce = TRUE;
	Justif();
	ColumnFix();
}

// Déplace le curseur par caractères.

void CEdit::MoveChar(int move, BOOL bWord, BOOL bSelect)
{
	int		character;

	if ( move == -1 )  // recule ?
	{
		if ( bWord )
		{
			while ( m_cursor1 > 0 )
			{
				character = (unsigned char)m_text[m_cursor1-1];
				if ( !IsSpace(character) )  break;
				m_cursor1 --;
			}

			if ( m_cursor1 > 0 )
			{
				character = (unsigned char)m_text[m_cursor1-1];
				if ( IsSpace(character) )
				{
					while ( m_cursor1 > 0 )
					{
						character = (unsigned char)m_text[m_cursor1-1];
						if ( !IsSpace(character) )  break;
						m_cursor1 --;
					}
				}
				else if ( IsWord(character) )
				{
					while ( m_cursor1 > 0 )
					{
						character = (unsigned char)m_text[m_cursor1-1];
						if ( !IsWord(character) )  break;
						m_cursor1 --;
					}
				}
				else if ( IsSep(character) )
				{
					while ( m_cursor1 > 0 )
					{
						character = (unsigned char)m_text[m_cursor1-1];
						if ( !IsSep(character) )  break;
						m_cursor1 --;
					}
				}
			}
		}
		else
		{
			m_cursor1 --;
			if ( m_cursor1 < 0 )  m_cursor1 = 0;
		}
	}

	if ( move == 1 )  // avance ?
	{
		if ( bWord )
		{
			if ( m_cursor1 < m_len )
			{
				character = (unsigned char)m_text[m_cursor1];
				if ( IsSpace(character) )
				{
					while ( m_cursor1 < m_len )
					{
						character = (unsigned char)m_text[m_cursor1];
						if ( !IsSpace(character) )  break;
						m_cursor1 ++;
					}
				}
				else if ( IsWord(character) )
				{
					while ( m_cursor1 < m_len )
					{
						character = (unsigned char)m_text[m_cursor1];
						if ( !IsWord(character) )  break;
						m_cursor1 ++;
					}
				}
				else if ( IsSep(character) )
				{
					while ( m_cursor1 < m_len )
					{
						character = (unsigned char)m_text[m_cursor1];
						if ( !IsSep(character) )  break;
						m_cursor1 ++;
					}
				}
			}

			while ( m_cursor1 < m_len )
			{
				character = (unsigned char)m_text[m_cursor1];
				if ( !IsSpace(character) )  break;
				m_cursor1 ++;
			}
		}
		else
		{
			m_cursor1 ++;
			if ( m_cursor1 > m_len )  m_cursor1 = m_len;
		}
	}

	if ( !bSelect )  m_cursor2 = m_cursor1;

	m_bUndoForce = TRUE;
	Justif();
	ColumnFix();
}

// Déplace le curseur par lignes.

void CEdit::MoveLine(int move, BOOL bWord, BOOL bSelect)
{
	float	column, indentLength;
	int		i, line, c;

	if ( move == 0 )  return;

	for ( i=0 ; i>move ; i-- )  // recule ?
	{
		while ( m_cursor1 > 0 && m_text[m_cursor1-1] != '\n' )
		{
			m_cursor1 --;
		}
		if ( m_cursor1 != 0 )
		{
			m_cursor1 --;
			while ( m_cursor1 > 0 )
			{
				if ( m_text[--m_cursor1] == '\n' )
				{
					m_cursor1 ++;
					break;
				}
			}
		}
	}

	for ( i=0 ; i<move ; i++ )  // avance ?
	{
		while ( m_cursor1 < m_len )
		{
			if ( m_text[m_cursor1++] == '\n' )
			{
				break;
			}
		}
	}

	line = RetCursorLine(m_cursor1);

	column = m_column;
	if ( m_bAutoIndent )
	{
		indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
						* m_engine->RetEditIndentValue();
		column -= indentLength*m_lineIndent[line];
	}

	if ( m_format == 0 )
	{
		c = m_engine->RetText()->Detect(m_text+m_lineOffset[line],
										m_lineOffset[line+1]-m_lineOffset[line],
										column, m_fontSize,
										m_fontStretch, m_fontType);
	}
	else
	{
		c = m_engine->RetText()->Detect(m_text+m_lineOffset[line],
										m_format+m_lineOffset[line],
										m_lineOffset[line+1]-m_lineOffset[line],
										column, m_fontSize, m_fontStretch);
	}

	m_cursor1 = m_lineOffset[line]+c;
	if ( !bSelect )  m_cursor2 = m_cursor1;

	m_bUndoForce = TRUE;
	Justif();
}

// Fixe la position horizontale.

void CEdit::ColumnFix()
{
	float	indentLength;
	int		line;

	line = RetCursorLine(m_cursor1);

	if ( m_format == 0 )
	{
		m_column = m_engine->RetText()->RetStringWidth
							(
								m_text+m_lineOffset[line],
								m_cursor1-m_lineOffset[line],
								m_fontSize, m_fontStretch, m_fontType
							);
	}
	else
	{
		m_column = m_engine->RetText()->RetStringWidth
							(
								m_text+m_lineOffset[line],
								m_format+m_lineOffset[line],
								m_cursor1-m_lineOffset[line],
								m_fontSize, m_fontStretch
							);
	}

	if ( m_bAutoIndent )
	{
		indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
						* m_engine->RetEditIndentValue();
		m_column += indentLength*m_lineIndent[line];
	}
}


// Coupe les caractères sélectionnés, ou toute la ligne.

BOOL CEdit::Cut()
{
	HGLOBAL	hg;
	char*	text;
	char	c;
	int		c1, c2, start, len, i, j;

	if ( !m_bEdit )  return FALSE;

	c1 = m_cursor1;
	c2 = m_cursor2;
	if ( c1 > c2 )  Swap(c1, c2);  // toujours c1 <= c2

	if ( c1 == c2 )
	{
		while ( c1 > 0 )
		{
			if ( m_text[c1-1] == '\n' )  break;
			c1 --;
		}
		while ( c2 < m_len )
		{
			c2 ++;
			if ( m_text[c2-1] == '\n' )  break;
		}
	}

	if ( c1 == c2 )  return FALSE;

	start = c1;
	len   = c2-c1;

	if ( !(hg = GlobalAlloc(GMEM_DDESHARE, len*2+1)) )
	{
		return FALSE;
	}
	if ( !(text = (char*)GlobalLock(hg)) )
	{
		GlobalFree(hg);
		return FALSE;
	}

	j = 0;
	for ( i=start ; i<start+len ; i++ )
	{
		c = m_text[i];
		if ( c == '\n' )  text[j++] = '\r';
		text[j++] = c;
	}
	text[j] = 0;
	GlobalUnlock(hg);

	if ( !OpenClipboard(NULL) )
	{
		GlobalFree(hg);
		return FALSE;
	}
	if ( !EmptyClipboard() )
	{
		GlobalFree(hg);
		return FALSE;
	}
	if ( !SetClipboardData(CF_TEXT, hg) )
	{
		GlobalFree(hg);
		return FALSE;
	}
	CloseClipboard();

	UndoMemorize(OPERUNDO_SPEC);
	m_cursor1 = c1;
	m_cursor2 = c2;
	DeleteOne(0);  // supprime les caractères sélectionnés
	Justif();
	ColumnFix();
	SendModifEvent();
	return TRUE;
}

// Copie les caractères sélectionnés, ou toute la ligne.

BOOL CEdit::Copy()
{
	HGLOBAL	hg;
	char*	text;
	char	c;
	int		c1, c2, start, len, i, j;

	c1 = m_cursor1;
	c2 = m_cursor2;
	if ( c1 > c2 )  Swap(c1, c2);  // toujours c1 <= c2

	if ( c1 == c2 )
	{
		while ( c1 > 0 )
		{
			if ( m_text[c1-1] == '\n' )  break;
			c1 --;
		}
		while ( c2 < m_len )
		{
			c2 ++;
			if ( m_text[c2-1] == '\n' )  break;
		}
	}

	if ( c1 == c2 )  return FALSE;

	start = c1;
	len   = c2-c1;

	if ( !(hg = GlobalAlloc(GMEM_DDESHARE, len*2+1)) )
	{
		return FALSE;
	}
	if ( !(text = (char*)GlobalLock(hg)) )
	{
		GlobalFree(hg);
		return FALSE;
	}

	j = 0;
	for ( i=start ; i<start+len ; i++ )
	{
		c = m_text[i];
		if ( c == '\n' )  text[j++] = '\r';
		text[j++] = c;
	}
	text[j] = 0;
	GlobalUnlock(hg);

	if ( !OpenClipboard(NULL) )
	{
		GlobalFree(hg);
		return FALSE;
	}
	if ( !EmptyClipboard() )
	{
		GlobalFree(hg);
		return FALSE;
	}
	if ( !SetClipboardData(CF_TEXT, hg) )
	{
		GlobalFree(hg);
		return FALSE;
	}
	CloseClipboard();

	return TRUE;
}

// Colle le contenu du bloc-notes.

BOOL CEdit::Paste()
{
	HANDLE	h;
	char	c;
	char*	p;

	if ( !m_bEdit )  return FALSE;

	if ( !OpenClipboard(NULL) )
	{
		return FALSE;
	}

	if ( !(h = GetClipboardData(CF_TEXT)) )
	{
		CloseClipboard();
		return FALSE;
	}

	if ( !(p = (char*)GlobalLock(h)) )
	{
		CloseClipboard();
		return FALSE;
	}

	UndoMemorize(OPERUNDO_SPEC);

	while ( *p != 0 )
	{
		c = *p++;
		if ( c == '\r' )  continue;
		if ( c == '\t' && m_bAutoIndent )  continue;
		InsertOne(c);
	}

	GlobalUnlock(h);
	CloseClipboard();

	Justif();
	ColumnFix();
	SendModifEvent();
	return TRUE;
}


// Annule la dernière action.

BOOL CEdit::Undo()
{
	if ( !m_bEdit )  return FALSE;

	return UndoRecall();
}


// Insère un caractère.

void CEdit::Insert(char character)
{
	int		i, level, tab;

	if ( !m_bEdit )  return;

	if ( !m_bMulti )  // mono-ligne ?
	{
		if ( character == '\n' ||
			 character == '\t' )  return;
	}

	UndoMemorize(OPERUNDO_INSERT);

	if ( m_bMulti && !m_bAutoIndent )
	{
		if ( character == '\n' )
		{
			InsertOne(character);
			level = IndentCompute();
			for ( i=0 ; i<level ; i++ )
			{
				InsertOne('\t');
			}
		}
		else if ( character == '{' )
		{
			tab = IndentTabCount();
			if ( tab != -1 )
			{
				level = IndentCompute();
				IndentTabAdjust(level-tab);
			}
			InsertOne(character);
		}
		else if ( character == '}' )
		{
			tab = IndentTabCount();
			if ( tab != -1 )
			{
				level = IndentCompute()-1;
				IndentTabAdjust(level-tab);
			}
			InsertOne(character);
		}
		else
		{
			InsertOne(character);
		}
	}
	else if ( m_bAutoIndent )
	{
		if ( character == '{' )
		{
			InsertOne(character);
			InsertOne('\n');
			InsertOne('\n');
			InsertOne('}');
			MoveChar(-1, FALSE, FALSE);
			MoveChar(-1, FALSE, FALSE);
		}
#if 0
		else if ( character == '(' )
		{
			InsertOne(character);
			InsertOne(')');
			MoveChar(-1, FALSE, FALSE);
		}
		else if ( character == '[' )
		{
			InsertOne(character);
			InsertOne(']');
			MoveChar(-1, FALSE, FALSE);
		}
#endif
		else if ( character == '\t' )
		{
			for ( i=0 ; i<m_engine->RetEditIndentValue() ; i++ )
			{
				InsertOne(' ');
			}
		}
		else
		{
			InsertOne(character);
		}
	}
	else
	{
		InsertOne(character);
	}

	Justif();
	ColumnFix();
}

// Insère un caractère brut.

void CEdit::InsertOne(char character)
{
	int		i;

	if ( !m_bEdit )  return;
	if ( !m_bMulti && character == '\n' )  return;

	if ( m_bFilename )
	{
		if ( character == '*'  ||
			 character == '/'  ||
			 character == '\\' ||
			 character == '\"' ||
			 character == '<'  ||
			 character == '>'  ||
			 character == '?'  ||
			 character == ':'  ||
			 character == '|'  )  return;
	}

	if ( m_cursor1 != m_cursor2 )
	{
		DeleteOne(0);  // supprime les caractères sélectionnés
	}

	if ( m_len >= m_maxChar )  return;

	for ( i=m_len ; i>=m_cursor1 ; i-- )
	{
		m_text[i] = m_text[i-1];  // pousse

		if ( m_format != 0 )
		{
			m_format[i] = m_format[i-1];  // pousse
		}
	}

	m_len ++;

	m_text[m_cursor1] = character;

	if ( m_format != 0 )
	{
		m_format[m_cursor1] = 0;
	}

	m_cursor1++;
	m_cursor2 = m_cursor1;
}

// Supprime le caractère à gauche du curseur ou tous les
// caractères sélectionnés.

void CEdit::Delete(int dir)
{
	if ( !m_bEdit )  return;

	UndoMemorize(OPERUNDO_DELETE);
	DeleteOne(dir);

	Justif();
	ColumnFix();
}

// Supprime le caractère à gauche du curseur ou tous les
// caractères sélectionnés brut.

void CEdit::DeleteOne(int dir)
{
	int		i, end, hole;

	if ( !m_bEdit )  return;

	if ( m_cursor1 == m_cursor2 )
	{
		if ( dir < 0 )
		{
			if ( m_cursor1 == 0 )  return;
			m_cursor1 --;
		}
		else
		{
			if ( m_cursor2 == m_len )  return;
			m_cursor2 ++;
		}
	}

	if ( m_cursor1 > m_cursor2 )  Swap(m_cursor1, m_cursor2);
	hole = m_cursor2-m_cursor1;
	end = m_len-hole;
	for ( i=m_cursor1 ; i<end ; i++ )
	{
		m_text[i] = m_text[i+hole];

		if ( m_format != 0 )
		{
			m_format[i] = m_format[i+hole];
		}
	}
	m_len -= hole;
	m_cursor2 = m_cursor1;
}


// Calcule le niveau d'indentation des crochets { et }.

int CEdit::IndentCompute()
{
	int		i, level;

	level = 0;
	for ( i=0 ; i<m_cursor1 ; i++ )
	{
		if ( m_text[i] == '{' )  level ++;
		if ( m_text[i] == '}' )  level --;
	}

	if ( level < 0 )  level = 0;
	return level;
}

// Compte le nombre de tabulateurs avant le curseur.
// Retourne -1 s'il y a autre chose.

int CEdit::IndentTabCount()
{
	int		i, nb;

	if ( m_cursor1 != m_cursor2 )  return -1;

	i = m_cursor1;
	nb = 0;
	while ( i > 0 )
	{
		if ( m_text[i-1] == '\n' )  return nb;
		if ( m_text[i-1] != '\t' )  return -1;
		nb ++;
		i --;
	}
	return nb;
}

// Ajoute ou supprime qq tabulateurs.

void CEdit::IndentTabAdjust(int number)
{
	int		i;

	for ( i=0 ; i<number ; i++ )  // ajoute ?
	{
		InsertOne('\t');
	}

	for ( i=0 ; i>number ; i-- )  // supprime ?
	{
		DeleteOne(-1);
	}
}


// Indente à gauche ou à droite toute la sélection.

BOOL CEdit::Shift(BOOL bLeft)
{
	BOOL	bInvert = FALSE;
	int		c1, c2, i;

	if ( m_cursor1 == m_cursor2 )  return FALSE;

	UndoMemorize(OPERUNDO_SPEC);

	c1 = m_cursor1;
	c2 = m_cursor2;
	if ( c1 > c2 )
	{
		Swap(c1, c2);  // toujours c1 <= c2
		bInvert = TRUE;
	}

	if ( c1 > 0 )
	{
		if ( m_text[c1-1] != '\n' )  return FALSE;
	}
	if ( c2 < m_len )
	{
		if ( m_text[c2-1] != '\n' )  return FALSE;
	}

	if ( bLeft )  // décale à gauche ?
	{
		i = c1;
		while ( i < c2 )
		{
			if ( m_text[i] == '\t' )
			{
				m_cursor1 = i;
				m_cursor2 = i+1;
				DeleteOne(0);
				c2 --;
			}
			while ( i < c2 && m_text[i++] != '\n' );
		}
	}
	else	// décale à droite ?
	{
		i = c1;
		while ( i < c2 )
		{
			m_cursor1 = m_cursor2 = i;
			InsertOne('\t');
			c2 ++;
			while ( i < c2 && m_text[i++] != '\n' );
		}
	}

	if ( bInvert )  Swap(c1, c2);
	m_cursor1 = c1;
	m_cursor2 = c2;

	Justif();
	ColumnFix();
	SendModifEvent();
	return TRUE;
}

// Conversion min <-> maj de la sélection.

BOOL CEdit::MinMaj(BOOL bMaj)
{
	int		c1, c2, i, character;

	if ( m_cursor1 == m_cursor2 )  return FALSE;

	UndoMemorize(OPERUNDO_SPEC);

	c1 = m_cursor1;
	c2 = m_cursor2;
	if ( c1 > c2 )  Swap(c1, c2);  // toujours c1 <= c2

	for ( i=c1 ; i<c2 ; i++ )
	{
		character = (unsigned char)m_text[i];
		if ( bMaj )  character = RetToUpper(character);
		else         character = RetToLower(character);
		m_text[i] = character;
	}

	Justif();
	ColumnFix();
	SendModifEvent();
	return TRUE;
}


// Coupe tout le texte en lignes.

void CEdit::Justif()
{
	float	width, value, size, indentLength;
	int		i, j, line, indent;
	BOOL	bDual, bString, bRem;

	indent = 0;
	m_lineTotal = 0;
	m_lineOffset[m_lineTotal] = 0;
	m_lineIndent[m_lineTotal] = indent;
	m_lineTotal ++;

	if ( m_bAutoIndent )
	{
		indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
						* m_engine->RetEditIndentValue();
	}

	bString = bRem = FALSE;
	i = 0;
	while ( TRUE )
	{
		bDual = FALSE;

		width = m_dim.x-(10.0f/640.0f)*2.0f-(m_bMulti?MARGX*2.0f+SCROLL_WIDTH:0.0f);
		if ( m_bAutoIndent )
		{
			width -= indentLength*m_lineIndent[m_lineTotal-1];
		}

		if ( m_format == 0 )
		{
			i += m_engine->RetText()->Justif(m_text+i, m_len-i, width,
											 m_fontSize, m_fontStretch,
											 m_fontType);
		}
		else
		{
			size = m_fontSize;

			if ( (m_format[i]&TITLE_MASK) == TITLE_BIG )  // grand titre ?
			{
				size *= BIG_FONT;
				bDual = TRUE;
			}

			if ( (m_format[i]&IMAGE_MASK) != 0 )  // tranche d'image ?
			{
				i ++;  // saute juste un caractère (index dans m_image)
			}
			else
			{
				i += m_engine->RetText()->Justif(m_text+i, m_format+i,
												 m_len-i, width,
												 size, m_fontStretch);
			}
		}

		if ( i >= m_len )  break;

		if ( m_bAutoIndent )
		{
			for ( j=m_lineOffset[m_lineTotal-1] ; j<i ; j++ )
			{
				if ( !bRem && m_text[j] == '\"' )  bString = !bString;
				if ( !bString &&
					 m_text[j] == '/' &&
					 m_text[j+1] == '/' )  bRem = TRUE;
				if ( m_text[j] == '\n' )  bString = bRem = FALSE;
				if ( m_text[j] == '{' && !bString && !bRem )  indent ++;
				if ( m_text[j] == '}' && !bString && !bRem )  indent --;
			}
			if ( indent < 0 )  indent = 0;
		}

		m_lineOffset[m_lineTotal] = i;
		m_lineIndent[m_lineTotal] = indent;
		m_lineTotal ++;
		if ( bDual )
		{
			m_lineOffset[m_lineTotal] = i;
			m_lineIndent[m_lineTotal] = indent;
			m_lineTotal ++;
		}
		if ( m_lineTotal >= EDITLINEMAX-2 )  break;
	}
	if ( m_len > 0 && m_text[m_len-1] == '\n' )
	{
		m_lineOffset[m_lineTotal] = m_len;
		m_lineIndent[m_lineTotal] = 0;
		m_lineTotal ++;
	}
	m_lineOffset[m_lineTotal] = m_len;
	m_lineIndent[m_lineTotal] = 0;

	if ( m_bAutoIndent )
	{
		for ( i=0 ; i<=m_lineTotal ; i++ )
		{
			if ( m_text[m_lineOffset[i]] == '}' )
			{
				if ( m_lineIndent[i] > 0 )  m_lineIndent[i] --;
			}
		}
	}

	if ( m_bMulti )
	{
		if ( m_bEdit )
		{
			line = RetCursorLine(m_cursor1);
			if ( line < m_lineFirst )
			{
				m_lineFirst = line;
			}
			if ( line >= m_lineFirst+m_lineVisible )
			{
				m_lineFirst = line-m_lineVisible+1;
			}
		}
	}
	else
	{
		m_lineFirst = 0;
	}

	if ( m_scroll != 0 )
	{
		if ( m_lineTotal <= m_lineVisible )
		{
			m_scroll->SetVisibleRatio(1.0f);
			m_scroll->SetVisibleValue(0.0f);
			m_scroll->SetArrowStep(0.0f);
		}
		else
		{
			value = (float)m_lineVisible/m_lineTotal;
			m_scroll->SetVisibleRatio(value);

			value = (float)m_lineFirst/(m_lineTotal-m_lineVisible);
			m_scroll->SetVisibleValue(value);

			value = (float)1.0f/(m_lineTotal-m_lineVisible);
			m_scroll->SetArrowStep(value);
		}
	}

	m_timeBlink = 0.0f;  // allume le curseur immédiatement
}

// Retourne le rang de la ligne dans laquelle se trouve le curseur.

int CEdit::RetCursorLine(int cursor)
{
	int		line, i;

	line = 0;
	for ( i=0 ; i<m_lineTotal ; i++ )
	{
		if ( cursor >= m_lineOffset[i] )
		{
			line = i;
		}
	}
	return line;
}


// Vide le buffer undo.

void CEdit::UndoFlush()
{
	int		i;

	for ( i=0 ; i<EDITUNDOMAX ; i++ )
	{
		delete m_undo[i].text;
		m_undo[i].text = 0;
	}

	m_bUndoForce = TRUE;
	m_undoOper = OPERUNDO_SPEC;
}

// Mémorise l'état actuel avant une modification.

void CEdit::UndoMemorize(OperUndo oper)
{
	int		i, len;

	if ( !m_bUndoForce               &&
		 oper       != OPERUNDO_SPEC &&
		 m_undoOper != OPERUNDO_SPEC &&
		 oper == m_undoOper          )  return;

	m_bUndoForce = FALSE;
	m_undoOper = oper;

	delete m_undo[EDITUNDOMAX-1].text;

	for ( i=EDITUNDOMAX-1 ; i>=1 ; i-- )
	{
		m_undo[i] = m_undo[i-1];
	}

	len = m_len;
	if ( len == 0 )  len ++;
	m_undo[0].text = (char*)malloc(sizeof(char)*(len+1));
	memcpy(m_undo[0].text, m_text, m_len);
	m_undo[0].len = m_len;

	m_undo[0].cursor1 = m_cursor1;
	m_undo[0].cursor2 = m_cursor2;
	m_undo[0].lineFirst = m_lineFirst;
}

// Revient à l'état précédent.

BOOL CEdit::UndoRecall()
{
	int		i;

	if ( m_undo[0].text == 0 )  return FALSE;

	m_len = m_undo[0].len;
	memcpy(m_text, m_undo[0].text, m_len);

	m_cursor1 = m_undo[0].cursor1;
	m_cursor2 = m_undo[0].cursor2;
	m_lineFirst = m_undo[0].lineFirst;

	for ( i=0 ; i<EDITUNDOMAX-1 ; i++ )
	{
		m_undo[i] = m_undo[i+1];
	}
	m_undo[EDITUNDOMAX-1].text = 0;

	m_bUndoForce = TRUE;
	Justif();
	ColumnFix();
	SendModifEvent();
	return TRUE;
}


// Efface le format de tous les caractères.

BOOL CEdit::ClearFormat()
{
	if ( m_format == 0 )
	{
		SetMultiFont(TRUE);
	}
	memset(m_format, m_fontType, m_len);

	return TRUE;
}

// Modifie le format d'une suite de caractères.

BOOL CEdit::SetFormat(int cursor1, int cursor2, int format)
{
	int		i;

	if ( m_format == 0 )  return FALSE;

	for ( i=cursor1 ; i<cursor2 ; i++ )
	{
		m_format[i] |= format;
	}

	return TRUE;
}


// Retourne les touches uniquement "clavier".

void CEdit::RetKeys(int keyRank, int res[])
{
	res[0] = m_engine->RetKey(keyRank, 0);
	res[1] = m_engine->RetKey(keyRank, 1);

	if ( m_engine->IsKeyMouse(res[1])    ||
		 m_engine->IsKeyJoystick(res[1]) )
	{
		res[1] = 0;
	}

	if ( m_engine->IsKeyMouse(res[0])    ||
		 m_engine->IsKeyJoystick(res[0]) )
	{
		res[0] = res[1];
		res[1] = 0;
	}
}

