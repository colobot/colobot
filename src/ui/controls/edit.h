/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */


/**
 * \file ui/controls/edit.h
 * \brief CEdit class
 */

#pragma once

#include "ui/controls/control.h"

#include <array>
#include <memory>

namespace Ui
{

class CScroll;

//! max number of levels preserves
const int EDITHISTORYMAX    = 50;
//! max number of successive undo
const short EDITUNDOMAX = 20;

struct EditUndo
{
    //! original text
    std::string text;
    //! length of the text
    std::size_t     len = 0;
    //! offset cursor
    std::size_t     cursor1 = 0;
    //! offset cursor
    std::size_t     cursor2 = 0;
    //! the first line displayed.
    std::size_t     lineFirst = 0;

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
    //! name of the image (without icons/)
    std::string    name;
    //! vertical offset (v texture)
    float   offset = 0.0f;
    //! height of the part (dv texture)
    float   height = 0.0f;
    //! width
    float   width = 0.0f;
};

struct HyperLink
{
    //! text file name (without help/)
    std::string    name;
    //! name of the marker
    std::string    marker;
};

struct HyperMarker
{
    //! name of the marker
    std::string    name;
    //! position in the text
    std::size_t pos = 0;
};

struct HyperHistory
{
    //! full file name text
    std::string    filename;
    //! rank of the first displayed line
    int firstLine = 0;
};




class CEdit : public CControl
{
public:
    CEdit ();

    virtual ~CEdit();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventType) override;

    void        SetPos(Math::Point pos) override;
    void        SetDim(Math::Point dim) override;

    bool        EventProcess(const Event &event) override;
    void        Draw() override;

    void               SetText(const std::string& text, const bool bNew=true);
    std::string        GetText(std::size_t max)const;
    const std::string& GetText()const;
    std::size_t        GetTextLength()const;

    bool        ReadText(const std::string& filename);
    bool        WriteText(const std::string& filename);

    void        SetMaxChar(const std::size_t max);
    std::size_t GetMaxChar()const;

    void        SetEditCap(const bool bMode);
    bool        GetEditCap()const;

    void        SetHighlightCap(const bool bEnable);
    bool        GetHighlightCap()const;

    void        SetInsideScroll(const bool bInside);
    bool        GetInsideScroll()const;

    void        SetSoluceMode(const bool bSoluce);
    bool        GetSoluceMode()const;

    void        SetGenericMode(const bool bGeneric);
    bool        GetGenericMode()const;

    void        SetAutoIndent(const bool bMode);
    bool        GetAutoIndent()const;

    void        SetCursor(std::size_t cursor1, std::size_t cursor2);
    void        GetCursor(std::size_t &cursor1, std::size_t &cursor2)const;

    void        SetFirstLine(const int rank);
    int         GetFirstLine()const;
    void        ShowSelect();

    void        SetDisplaySpec(const bool bDisplay);
    bool        GetDisplaySpec()const;

    void        SetMultiFont(const bool bMulti);
    bool        GetMultiFont()const;

    bool        Cut();
    bool        Copy(const bool memorize_cursor = false);
    bool        Paste();
    bool        Undo();

    void        HyperFlush();
    void        HyperHome(const std::string& filename);
    bool        HyperTest(const EventType event);
    bool        HyperGo(const EventType event);

    void        SetFontSize(const float size) override;

    bool        ClearFormat();
    bool        SetFormat(const std::size_t cursor1, const std::size_t cursor2, const int format);

protected:
    void        SendModifEvent();
    bool        IsLinkPos(const Math::Point pos);
    void        MouseDoubleClick(const Math::Point mouse);
    void        MouseClick(const Math::Point mouse);
    void        MouseMove(Math::Point mouse);
    void        MouseRelease(const Math::Point mouse);
    std::size_t      MouseDetect(const Math::Point mouse);
    void        MoveAdjust();

    void        HyperJump(const std::string& name, const std::string& marker);
    bool        HyperAdd(const std::string& filename, const int firstLine);

    void        DrawImage(Math::Point pos, std::string name, float width, float offset, float height, int nbLine);
    void        DrawBack(Math::Point pos, Math::Point dim);

    void        DrawHorizontalGradient(Math::Point pos, Math::Point dim, Gfx::Color color1, Gfx::Color color2);
    void        DrawColor(Math::Point pos, Math::Point dim, Gfx::Color color);

    void        FreeImage();
    void        Scroll(const std::size_t pos, const bool bAdjustCursor);
    void        Scroll();
    void        MoveChar(int move, const bool bWord, const bool bSelect);
    void        MoveLine(int move, const bool bWord, const bool bSelect);
    void        MoveHome(const bool bWord, const bool bSelect);
    void        MoveEnd(const bool bEoF, const bool bSelect);
    void        ColumnFix();
    void        Insert(const char character);
    void        InsertOne(const char character);

    // Inserts a string (ended by a null char)
    void InsertTxt(const char* str);
    inline void InsertTxt(const std::string& str)
    {
        InsertTxt(str.c_str());
    }
    void        Delete(const int dir);
    void        DeleteOne(const int dir);
    void        DeleteWord(const int dir);
    int         IndentCompute()const;
    int         IndentTabCount()const;
    void        IndentTabAdjust(const int number);
    bool        Shift(const bool bLeft);
    bool        MinMaj(const bool bMaj);
    void        Justif();
    int         GetCursorLine(const std::size_t cursor)const;

    void        UndoFlush();
    void        UndoMemorize(OperUndo oper);
    bool        UndoRecall();

    void        UpdateScroll();

    void        SetFocus(CControl* control) override;
    void        UpdateFocus();      // Start/stop text input mode, this toggles the on-screen keyboard

    void        GetIndentedText(std::ostream& stream, const std::size_t start, const std::size_t end);

protected:
    std::unique_ptr<CScroll> m_scroll;           // vertical scrollbar on the right

    std::size_t m_maxChar;
    std::string m_text;             // text (without zero terminator)
    std::vector<Gfx::FontMetaChar> m_format;           // format characters

    std::size_t m_len;              // length used in m_text
    std::size_t m_cursor1;          // offset cursor
    std::size_t m_cursor2;          // offset cursor

    bool                     m_bMulti;           // true -> multi-line
    bool                     m_bEdit;            // true -> editable
    bool                     m_bHilite;          // true -> hilitable
    bool                     m_bInsideScroll;        // true -> lift as part
    bool                     m_bDisplaySpec;         // true -> displays the special characters
    bool                     m_bMultiFont;           // true -> more fonts possible
    bool                     m_bSoluce;          // true -> shows the links-solution
    bool                     m_bGeneric;         // true -> generic that defile
    bool                     m_bAutoIndent;          // true -> automatic indentation
    float                    m_lineHeight;           // height of a row
    float                    m_lineAscent;           // height above the baseline
    float                    m_lineDescent;          // height below the baseline
    int                      m_lineVisible;          // total number of viewable lines
    int                      m_lineFirst;            // the first line displayed
    int                      m_lineTotal;            // number lines used (in m_lineOffset)
    std::vector<std::size_t> m_lineOffset;
    std::vector<short>       m_lineIndent;
    std::vector<ImageLine>   m_image;
    std::vector<HyperLink>   m_link;
    std::vector<HyperMarker> m_marker;
    int                      m_historyTotal;
    int                      m_historyCurrent;
    std::array<HyperHistory, EDITHISTORYMAX> m_history;
    float                    m_time;             // absolute time
    float                    m_timeBlink;
    float                    m_timeLastClick;
    float                    m_timeLastScroll;
    Math::Point              m_mouseFirstPos;
    Math::Point              m_mouseLastPos;
    float                    m_column;

    bool        m_bCapture;

    bool        m_bUndoForce;
    OperUndo    m_undoOper;
    std::array<EditUndo, EDITUNDOMAX> m_undo;
};


}
