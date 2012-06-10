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

// edit.h

#pragma once


#include "common/struct.h"
#include "ui/control.h"


class CD3DEngine;
class CScroll;



#define EDITSTUDIOMAX	20000		// maximum number of characters in CBOT edit
#define EDITLINEMAX	1000		// maximum total number of lines
#define EDITIMAGEMAX	50		// maximum total number of lines with images
#define EDITLINKMAX	100		// maximum number of links
#define EDITHISTORYMAX	50		// max number of levels preserves

#define EDITUNDOMAX	20		// max number of successive undo

typedef struct
{
	char*		text;			// original text
	int		len;			// length of the text
	int		cursor1;		// offset cursor
	int		cursor2;		// offset cursor
	int		lineFirst;		// the first line displayed.

}
EditUndo;

enum OperUndo
{
	OPERUNDO_SPEC	= 0,	// special operation
	OPERUNDO_INSERT	= 1,	// inserting characters
	OPERUNDO_DELETE	= 2,	// deleting characters
};

typedef struct
{
	char	name[40];		// name of the image (without diagram \)
	float	offset;			// vertical offset (v texture)
	float	height;			// height of the part (dv texture)
	float	width;			// width
}
ImageLine;

typedef struct
{
	char	name[40];		// text file name (without help \)
	char	marker[20];		// name of the marker
}
HyperLink;

typedef struct
{
	char	name[20];		// name of the marker
	int	pos;			// position in the text
}
HyperMarker;

typedef struct
{
	char	filename[50];		// full file name text
	int	firstLine;		// rank of the first displayed line
}
HyperHistory;




class CEdit : public CControl
{
public:
	CEdit(CInstanceManager* iMan);
	virtual ~CEdit();

	bool		Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);

	void		SetPos(FPOINT pos);
	void		SetDim(FPOINT dim);

	bool		EventProcess(const Event &event);
	void		Draw();

	void		SetText(char *text, bool bNew=true);
	void		GetText(char *buffer, int max);
	char*		RetText();
	int			RetTextLength();

	bool		ReadText(char *filename, int addSize=0);
	bool		WriteText(char *filename);

	void		SetMaxChar(int max);
	int			RetMaxChar();

	void		SetEditCap(bool bMode);
	bool		RetEditCap();

	void		SetHiliteCap(bool bEnable);
	bool		RetHiliteCap();

	void		SetInsideScroll(bool bInside);
	bool		RetInsideScroll();

	void		SetSoluceMode(bool bSoluce);
	bool		RetSoluceMode();

	void		SetGenericMode(bool bGeneric);
	bool		RetGenericMode();

	void		SetAutoIndent(bool bMode);
	bool		RetAutoIndent();

	void		SetCursor(int cursor1, int cursor2);
	void		GetCursor(int &cursor1, int &cursor2);

	void		SetFirstLine(int rank);
	int			RetFirstLine();
	void		ShowSelect();

	void		SetDisplaySpec(bool bDisplay);
	bool		RetDisplaySpec();

	void		SetMultiFont(bool bMulti);
	bool		RetMultiFont();

	bool		Cut();
	bool		Copy();
	bool		Paste();
	bool		Undo();

	void		HyperFlush();
	void		HyperHome(char *filename);
	bool		HyperTest(EventMsg event);
	bool		HyperGo(EventMsg event);

	void		SetFontSize(float size);

	bool		ClearFormat();
	bool		SetFormat(int cursor1, int cursor2, int format);

protected:
	void		SendModifEvent();
	bool		IsLinkPos(FPOINT pos);
	void		MouseDoubleClick(FPOINT mouse);
	void		MouseClick(FPOINT mouse);
	void		MouseMove(FPOINT mouse);
	void		MouseRelease(FPOINT mouse);
	int			MouseDetect(FPOINT mouse);
	void		MoveAdjust();

	void		HyperJump(char *name, char *marker);
	bool		HyperAdd(char *filename, int firstLine);

	void		DrawImage(FPOINT pos, char *name, float width, float offset, float height, int nbLine);
	void		DrawBack(FPOINT pos, FPOINT dim);
	void		DrawPart(FPOINT pos, FPOINT dim, int icon);

	void		FreeImage();
	void		LoadImage(char *name);
	void		Scroll(int pos, bool bAdjustCursor);
	void		Scroll();
	void		MoveChar(int move, bool bWord, bool bSelect);
	void		MoveLine(int move, bool bWord, bool bSelect);
	void		MoveHome(bool bWord, bool bSelect);
	void		MoveEnd(bool bWord, bool bSelect);
	void		ColumnFix();
	void		Insert(char character);
	void		InsertOne(char character);
	void		Delete(int dir);
	void		DeleteOne(int dir);
	int			IndentCompute();
	int			IndentTabCount();
	void		IndentTabAdjust(int number);
	bool		Shift(bool bLeft);
	bool		MinMaj(bool bMaj);
	void		Justif();
	int			RetCursorLine(int cursor);

	void		UndoFlush();
	void		UndoMemorize(OperUndo oper);
	bool		UndoRecall();

protected:
	CScroll*	m_scroll;			// vertical scrollbar on the right

	int		m_maxChar;			// max length of the buffer m_text
	char*		m_text;				// text (without zero terminator)
	char*		m_format;			// format characters
	int		m_len;				// length used in m_text
	int		m_cursor1;			// offset cursor
	int		m_cursor2;			// offset cursor

	bool		m_bMulti;			// true -> multi-line
	bool		m_bEdit;			// true -> editable
	bool		m_bHilite;			// true -> hilitable
	bool		m_bInsideScroll;		// true -> lift as part
	bool		m_bDisplaySpec;			// true -> displays the special characters
	bool		m_bMultiFont;			// true -> more fonts possible
	bool		m_bSoluce;			// true -> shows the links-solution
	bool		m_bGeneric;			// true -> generic that defile
	bool		m_bAutoIndent;			// true -> automatic indentation
	float		m_lineHeight;			// height of a row
	float		m_lineAscent;			// height above the baseline
	float		m_lineDescent;			// height below the baseline
	int		m_lineVisible;			// total number of viewable lines
	int		m_lineFirst;			// the first line displayed
	int		m_lineTotal;			// number lines used (in m_lineOffset)
	int		m_lineOffset[EDITLINEMAX];
	char		m_lineIndent[EDITLINEMAX];
	int		m_imageTotal;
	ImageLine	m_image[EDITIMAGEMAX];
	HyperLink	m_link[EDITLINKMAX];
	int		m_markerTotal;
	HyperMarker	m_marker[EDITLINKMAX];
	int		m_historyTotal;
	int		m_historyCurrent;
	HyperHistory 	m_history[EDITHISTORYMAX];
	float		m_time;				// absolute time
	float		m_timeBlink;
	float		m_timeLastClick;
	float		m_timeLastScroll;
	FPOINT		m_mouseFirstPos;
	FPOINT		m_mouseLastPos;
	float		m_column;

	bool		m_bCapture;

	bool		m_bUndoForce;
	OperUndo	m_undoOper;
	EditUndo	m_undo[EDITUNDOMAX];
};


