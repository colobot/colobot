/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include <filesystem>
#include <memory>

namespace Ui
{

class CScroll;

//! max number of levels preserves
const int EDITHISTORYMAX    = 50;
//! max number of successive undo
const int EDITUNDOMAX = 20;

struct EditUndo
{
    //! original text
    std::string text;
    //! length of the text
    int     len = 0;
    //! offset cursor
    int     cursor1 = 0;
    //! offset cursor
    int     cursor2 = 0;
    //! the first line displayed.
    int     lineFirst = 0;

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
    std::filesystem::path name;
    //! name of the marker
    std::string    marker;
};

struct HyperMarker
{
    //! name of the marker
    std::string    name;
    //! position in the text
    int pos = 0;
};

struct HyperHistory
{
    //! full file name text
    std::filesystem::path filename;
    //! rank of the first displayed line
    int firstLine = 0;
};




class CEdit : public CControl
{
public:
    CEdit ();

    virtual ~CEdit();

    bool        Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventType) override;

    void        SetPos(const glm::vec2& pos) override;
    void        SetDim(const glm::vec2& dim) override;

    bool        EventProcess(const Event &event) override;
    void        Draw() override;

    void               SetText(const std::string& text, bool bNew=true);
    std::string        GetText(int max);
    const std::string& GetText();
    int                GetTextLength();

    bool        ReadText(const std::filesystem::path& filename);
    bool        WriteText(const std::filesystem::path& filename);

    void        SetMaxChar(int max);
    int         GetMaxChar();

    void        SetEditCap(bool bMode);
    bool        GetEditCap();

    void        SetHighlightCap(bool bEnable);
    bool        GetHighlightCap();

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
    bool        Copy(bool memorize_cursor = false);
    bool        Paste();
    bool        Undo();

    void        HyperFlush();
    void        HyperHome(std::string filename);
    bool        HyperTest(EventType event);
    bool        HyperGo(EventType event);

    void        SetFontSize(float size) override;

    bool        ClearFormat();
    bool        SetFormat(int cursor1, int cursor2, int format);

protected:
    void        SendModifEvent();
    bool        IsLinkPos(const glm::vec2& pos);
    void        MouseDoubleClick(const glm::vec2& mouse);
    void        MouseClick(const glm::vec2& mouse);
    void        MouseMove(const glm::vec2& mouse);
    void        MouseRelease(const glm::vec2& mouse);
    int         MouseDetect(const glm::vec2& mouse);
    void        MoveAdjust();

    void        HyperJump(const std::filesystem::path& name, std::string marker);
    bool        HyperAdd(const std::filesystem::path& filename, int firstLine);

    void        DrawImage(const glm::vec2& pos, std::string name, float width, float offset, float height, int nbLine);
    void        DrawBack(const glm::vec2& pos, const glm::vec2& dim);

    void        DrawHorizontalGradient(const glm::vec2& pos, const glm::vec2& dim, Gfx::Color color1, Gfx::Color color2);
    void        DrawColor(const glm::vec2& pos, const glm::vec2& dim, Gfx::Color color);

    void        FreeImage();
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
    void        DeleteWord(int dir);
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

    void        SetFocus(CControl* control) override;
    void        UpdateFocus();      // Start/stop text input mode, this toggles the on-screen keyboard

    void        GetIndentedText(std::ostream& stream, int start, int end);

protected:
    std::unique_ptr<CScroll> m_scroll;           // vertical scrollbar on the right

    int m_maxChar;
    std::string m_text;             // text (without zero terminator)
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
    std::vector<int> m_lineOffset;
    std::vector<char> m_lineIndent;
    std::vector<ImageLine> m_image;
    std::vector<HyperLink> m_link;
    std::vector<HyperMarker> m_marker;
    int     m_historyTotal;
    int     m_historyCurrent;
    std::array<HyperHistory, EDITHISTORYMAX> m_history;
    float       m_time;             // absolute time
    float       m_timeBlink;
    float       m_timeLastClick;
    float       m_timeLastScroll;
    glm::vec2   m_mouseFirstPos;
    glm::vec2   m_mouseLastPos;
    float       m_column;

    bool        m_bCapture;

    bool        m_bUndoForce;
    OperUndo    m_undoOper;
    std::array<EditUndo, EDITUNDOMAX> m_undo;
};


}
