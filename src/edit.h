// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

#ifndef _EDIT_H_
#define	_EDIT_H_


#include "control.h"


class CD3DEngine;
class CScroll;



#define EDITSTUDIOMAX	20000		// nb max de caract�res pour �diter CBOT
#define EDITLINEMAX		1000		// nb max total de lignes
#define EDITIMAGEMAX	50			// nb max total de lignes avec images
#define EDITLINKMAX		100			// nb max de liens
#define EDITHISTORYMAX	50			// nb max de niveaux concerv�s

#define EDITUNDOMAX		20			// nb max de undo successifs

typedef struct
{
	char*	text;			// texte original
	int		len;			// longueur du texte
	int		cursor1;		// offset curseur
	int		cursor2;		// offset curseur
	int		lineFirst;		// premi�re ligne affich�e.

}
EditUndo;

enum OperUndo
{
	OPERUNDO_SPEC	= 0,	// op�ration sp�ciale
	OPERUNDO_INSERT	= 1,	// insertion de caract�res
	OPERUNDO_DELETE	= 2,	// suppression de caract�res
};

typedef struct
{
	char	name[40];		// nom de l'image (sans diagram\)
	float	offset;			// offset vertical (v texture)
	float	height;			// hauteur de la tranche (dv texture)
	float	width;			// largeur
}
ImageLine;

typedef struct
{
	char	name[40];		// nom du fichier texte (sans help\)
	char	marker[20];		// nom du marqueur
}
HyperLink;

typedef struct
{
	char	name[20];		// nom du marqueur
	int		pos;			// position dans le texte
}
HyperMarker;

typedef struct
{
	char	filename[50];	// nom complet du fichier texte
	int		firstLine;		// rang de la premi�re ligne affich�e
}
HyperHistory;




class CEdit : public CControl
{
public:
	CEdit(CInstanceManager* iMan);
	~CEdit();

	BOOL		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	BOOL		EventProcess(const Event &event);
	void		Draw();

	void		SetText(char *text, BOOL bNew=TRUE);
	void		GetText(char *buffer, int max);
	char*		RetText();
	int			RetTextLength();

	BOOL		ReadText(char *filename, int addSize=0);
	BOOL		WriteText(char *filename);

	void		SetMaxChar(int max);
	int			RetMaxChar();

	void		SetEditCap(BOOL bMode);
	BOOL		RetEditCap();

	void		SetHiliteCap(BOOL bEnable);
	BOOL		RetHiliteCap();

	void		SetInsideScroll(BOOL bInside);
	BOOL		RetInsideScroll();

	void		SetSoluceMode(BOOL bSoluce);
	BOOL		RetSoluceMode();

	void		SetGenericMode(BOOL bGeneric);
	BOOL		RetGenericMode();

	void		SetAutoIndent(BOOL bMode);
	BOOL		RetAutoIndent();

	void		SetCursor(int cursor1, int cursor2);
	void		GetCursor(int &cursor1, int &cursor2);

	void		SetFirstLine(int rank);
	int			RetFirstLine();
	void		ShowSelect();

	void		SetDisplaySpec(BOOL bDisplay);
	BOOL		RetDisplaySpec();

	void		SetMultiFont(BOOL bMulti);
	BOOL		RetMultiFont();

	BOOL		Cut();
	BOOL		Copy();
	BOOL		Paste();
	BOOL		Undo();

	void		HyperFlush();
	void		HyperHome(char *filename);
	BOOL		HyperTest(EventMsg event);
	BOOL		HyperGo(EventMsg event);

	void		SetFontSize(float size);

	BOOL		ClearFormat();
	BOOL		SetFormat(int cursor1, int cursor2, int format);

protected:
	void		SendModifEvent();
	BOOL		IsLinkPos(FPOINT pos);
	void		MouseDoubleClick(FPOINT mouse);
	void		MouseClick(FPOINT mouse);
	void		MouseMove(FPOINT mouse);
	void		MouseRelease(FPOINT mouse);
	int			MouseDetect(FPOINT mouse);
	void		MoveAdjust();

	void		HyperJump(char *name, char *marker);
	BOOL		HyperAdd(char *filename, int firstLine);

	void		DrawImage(FPOINT pos, char *name, float width, float offset, float height, int nbLine);
	void		DrawBack(FPOINT pos, FPOINT dim);
	void		DrawPart(FPOINT pos, FPOINT dim, int icon);

	void		FreeImage();
	void		LoadImage(char *name);
	void		Scroll(int pos, BOOL bAdjustCursor);
	void		Scroll();
	void		MoveChar(int move, BOOL bWord, BOOL bSelect);
	void		MoveLine(int move, BOOL bWord, BOOL bSelect);
	void		MoveHome(BOOL bWord, BOOL bSelect);
	void		MoveEnd(BOOL bWord, BOOL bSelect);
	void		ColumnFix();
	void		Insert(char character);
	void		InsertOne(char character);
	void		Delete(int dir);
	void		DeleteOne(int dir);
	int			IndentCompute();
	int			IndentTabCount();
	void		IndentTabAdjust(int number);
	BOOL		Shift(BOOL bLeft);
	BOOL		MinMaj(BOOL bMaj);
	void		Justif();
	int			RetCursorLine(int cursor);

	void		UndoFlush();
	void		UndoMemorize(OperUndo oper);
	BOOL		UndoRecall();

protected:
	CScroll*	m_scroll;			// ascenseur vertical � droite

	int			m_maxChar;			// lg max du buffer m_text
	char*		m_text;				// texte (sans z�ro terminateur)
	char*		m_format;			// format des caract�res
	int			m_len;				// longueur utilis�e dans m_text
	int			m_cursor1;			// offset curseur
	int			m_cursor2;			// offset curseur

	BOOL		m_bMulti;			// TRUE -> multi lignes
	BOOL		m_bEdit;			// TRUE -> �ditable
	BOOL		m_bHilite;			// TRUE -> hilitable
	BOOL		m_bInsideScroll;	// TRUE -> ascenseur dans le cadre
	BOOL		m_bDisplaySpec;		// TRUE -> affiche les caract�res sp�ciaux
	BOOL		m_bMultiFont;		// TRUE -> plusieurs fontes possible
	BOOL		m_bSoluce;			// TRUE -> montre les liens-solution
	BOOL		m_bGeneric;			// TRUE -> g�n�rique qui d�file
	BOOL		m_bAutoIndent;		// TRUE -> indentation automatique
	float		m_lineHeight;		// hauteur d'une ligne
	float		m_lineAscent;		// hauteur au-dessus de la ligne de base
	float		m_lineDescent;		// hauteur au-dessous de la ligne de base
	int			m_lineVisible;		// nb total de ligne affichables
	int			m_lineFirst;		// premi�re ligne affich�e
	int			m_lineTotal;		// nb lignes utilis�es (ds m_lineOffset)
	int			m_lineOffset[EDITLINEMAX];
	char		m_lineIndent[EDITLINEMAX];
	int			m_imageTotal;
	ImageLine	m_image[EDITIMAGEMAX];
	HyperLink	m_link[EDITLINKMAX];
	int			m_markerTotal;
	HyperMarker	m_marker[EDITLINKMAX];
	int			m_historyTotal;
	int			m_historyCurrent;
	HyperHistory m_history[EDITHISTORYMAX];
	float		m_time;				// temps absolu
	float		m_timeBlink;
	float		m_timeLastClick;
	float		m_timeLastScroll;
	FPOINT		m_mouseFirstPos;
	FPOINT		m_mouseLastPos;
	float		m_column;

	BOOL		m_bCapture;

	BOOL		m_bUndoForce;
	OperUndo	m_undoOper;
	EditUndo	m_undo[EDITUNDOMAX];
};


#endif //_EDIT_H_
