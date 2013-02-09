// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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


/**
 * \file ui/edit.h
 * \brief CEdit class
 */

#pragma once


#include "ui/control.h"
#include "ui/scroll.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/text.h"

#include "common/event.h"
#include "common/misc.h"
#include "common/iman.h"
#include "common/restext.h"

#include <set>



namespace Ui {



//! maximum number of characters in CBOT edit
const int EDITSTUDIOMAX     = 20000;
//! maximum total number of lines
const int EDITLINEMAX       = 1000;
//! maximum total number of lines with images
const int EDITIMAGEMAX      = 50;
//! maximum number of links
const int EDITLINKMAX       = 100;
//! max number of levels preserves
const int EDITHISTORYMAX    = 50;

//! max number of successive undo
const int EDITUNDOMAX = 20;

struct EditUndo
{
    //! original text
    char*       text;
    //! length of the text
    int     len;
    //! offset cursor
    int     cursor1;
    //! offset cursor
    int     cursor2;
    //! the first line displayed.
    int     lineFirst;

};

enum OperUndo
{
    //! special operation
    OPERUNDO_SPEC   = 0,
    //! inserting characters
    OPERUNDO_INSERT = 1,
    //! deleting characters
    OPERUNDO_DELETE = 2,
};

struct ImageLine
{
    //! name of the image (without diagram \)
    char    name[40];
    //! vertical offset (v texture)
    float   offset;
    //! height of the part (dv texture)
    float   height;
    //! width
    float   width;
};

struct HyperLink
{
    //! text file name (without help \)
    char    name[40];
    //! name of the marker
    char    marker[20];
};

struct HyperMarker
{
    //! name of the marker
    char    name[20];
    //! position in the text
    int pos;
};

struct HyperHistory
{
    //! full file name text
    char    filename[50];
    //! rank of the first displayed line
    int firstLine;
};




class CEdit : public CControl
{
public:
    CEdit ();

    virtual ~CEdit();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventType);

    void        SetPos(Math::Point pos);
    void        SetDim(Math::Point dim);

    bool        EventProcess(const Event &event);
    void        Draw();

    void        SetText(const char *text, bool bNew=true);
    void        GetText(char *buffer, int max);
    char*       GetText();
    int         GetTextLength();

    bool        ReadText(const char *filename, int addSize=0);
    bool        WriteText(const char *filename);

    void        SetMaxChar(int max);
    int         GetMaxChar();

    void        SetEditCap(bool bMode);
    bool        GetEditCap();

    void        SetHiliteCap(bool bEnable);
    bool        GetHiliteCap();

    void        SetInsideScroll(bool bInside);
    bool        GetInsideScroll();

    void        SetSoluceMode(bool bSoluce);
    bool        GetSoluceMode();

    void        SetGenericMode(bool bGeneric);
    bool        GetGenericMode();

    void        SetAutoIndent(bool bMode);
    bool        GetAutoIndent();

    void        SetCursor(int cursor1, int cursor2);
    void        GetCursor(int &cursor1, int &cursor2);

    void        SetFirstLine(int rank);
    int         GetFirstLine();
    void        ShowSelect();

    void        SetDisplaySpec(bool bDisplay);
    bool        GetDisplaySpec();

    void        SetMultiFont(bool bMulti);
    bool        GetMultiFont();

    bool        Cut();
    bool        Copy();
    bool        Paste();
    bool        Undo();

    void        HyperFlush();
    void        HyperHome(const char *filename);
    bool        HyperTest(EventType event);
    bool        HyperGo(EventType event);

    void        SetFontSize(float size);

    bool        ClearFormat();
    bool        SetFormat(int cursor1, int cursor2, int format);

protected:
    void        SendModifEvent();
    bool        IsLinkPos(Math::Point pos);
    void        MouseDoubleClick(Math::Point mouse);
    void        MouseClick(Math::Point mouse);
    void        MouseMove(Math::Point mouse);
    void        MouseRelease(Math::Point mouse);
    int         MouseDetect(Math::Point mouse);
    void        MoveAdjust();

    void        HyperJump(const char *name, const char *marker);
    bool        HyperAdd(const char *filename, int firstLine);

    void        DrawImage(Math::Point pos, const char *name, float width, float offset, float height, int nbLine);
    void        DrawBack(Math::Point pos, Math::Point dim);
    void        DrawPart(Math::Point pos, Math::Point dim, int icon);

    void        FreeImage();
    void        LoadImage(const char *name);
    void        Scroll(int pos, bool bAdjustCursor);
    void        Scroll();
    void        MoveChar(int move, bool bWord, bool bSelect);
    void        MoveLine(int move, bool bWord, bool bSelect);
    void        MoveHome(bool bWord, bool bSelect);
    void        MoveEnd(bool bWord, bool bSelect);
    void        ColumnFix();
    void        Insert(char character);
    void        InsertOne(char character);
    void        Delete(int dir);
    void        DeleteOne(int dir);
    int         IndentCompute();
    int         IndentTabCount();
    void        IndentTabAdjust(int number);
    bool        Shift(bool bLeft);
    bool        MinMaj(bool bMaj);
    void        Justif();
    int         GetCursorLine(int cursor);

    void        UndoFlush();
    void        UndoMemorize(OperUndo oper);
    bool        UndoRecall();
    
    void        UpdateScroll();

protected:
    CScroll*    m_scroll;           // vertical scrollbar on the right

    int     m_maxChar;          // max length of the buffer m_text
    char*       m_text;             // text (without zero terminator)
    std::vector<Gfx::FontMetaChar> m_format;           // format characters
    int     m_len;              // length used in m_text
    int     m_cursor1;          // offset cursor
    int     m_cursor2;          // offset cursor

    bool        m_bMulti;           // true -> multi-line
    bool        m_bEdit;            // true -> editable
    bool        m_bHilite;          // true -> hilitable
    bool        m_bInsideScroll;        // true -> lift as part
    bool        m_bDisplaySpec;         // true -> displays the special characters
    bool        m_bMultiFont;           // true -> more fonts possible
    bool        m_bSoluce;          // true -> shows the links-solution
    bool        m_bGeneric;         // true -> generic that defile
    bool        m_bAutoIndent;          // true -> automatic indentation
    float       m_lineHeight;           // height of a row
    float       m_lineAscent;           // height above the baseline
    float       m_lineDescent;          // height below the baseline
    int     m_lineVisible;          // total number of viewable lines
    int     m_lineFirst;            // the first line displayed
    int     m_lineTotal;            // number lines used (in m_lineOffset)
    int     m_lineOffset[EDITLINEMAX];
    char        m_lineIndent[EDITLINEMAX];
    int     m_imageTotal;
    ImageLine   m_image[EDITIMAGEMAX];
    HyperLink   m_link[EDITLINKMAX];
    int     m_markerTotal;
    HyperMarker m_marker[EDITLINKMAX];
    int     m_historyTotal;
    int     m_historyCurrent;
    HyperHistory    m_history[EDITHISTORYMAX];
    float       m_time;             // absolute time
    float       m_timeBlink;
    float       m_timeLastClick;
    float       m_timeLastScroll;
    Math::Point     m_mouseFirstPos;
    Math::Point     m_mouseLastPos;
    float       m_column;

    bool        m_bCapture;

    bool        m_bUndoForce;
    OperUndo    m_undoOper;
    EditUndo    m_undo[EDITUNDOMAX];
};


}
