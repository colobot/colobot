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

// text.h

#ifndef _TEXT_H_
#define _TEXT_H_


#include "d3dengine.h"


class CInstanceManager;



#define SMALLFONT   10.0f
#define BIGFONT     15.0f

#define NORMSTRETCH 0.8f



enum FontType
{
    FONT_COLOBOT    = 0,
    FONT_COURIER    = 1,
    FONT_BUTTON     = 2,
};

enum FontTitle
{
    TITLE_BIG       = 0x04,
    TITLE_NORM      = 0x08,
    TITLE_LITTLE    = 0x0c,
};

enum FontColor
{
    COLOR_LINK      = 0x10,
    COLOR_TOKEN     = 0x20,
    COLOR_TYPE      = 0x30,
    COLOR_CONST     = 0x40,
    COLOR_REM       = 0x50,
    COLOR_KEY       = 0x60,
    COLOR_TABLE     = 0x70,
};

#define FONT_MASK   0x03
#define TITLE_MASK  0x0c
#define COLOR_MASK  0x70
#define IMAGE_MASK  0x80



class CText
{
public:
    CText(CInstanceManager *iMan, CD3DEngine* engine);
    ~CText();

    void        SetD3DDevice(LPDIRECT3DDEVICE7 device);

    void        DrawText(char *string, char *format, int len, FPOINT pos, float width, int justif, float size, float stretch, int eol);
    void        DrawText(char *string, char *format, FPOINT pos, float width, int justif, float size, float stretch, int eol);
    void        DrawText(char *string, int len, FPOINT pos, float width, int justif, float size, float stretch, FontType font, int eol);
    void        DrawText(char *string, FPOINT pos, float width, int justif, float size, float stretch, FontType font, int eol);
    void        DimText(char *string, char *format, int len, FPOINT pos, int justif, float size, float stretch, FPOINT &start, FPOINT &end);
    void        DimText(char *string, char *format, FPOINT pos, int justif, float size, float stretch, FPOINT &start, FPOINT &end);
    void        DimText(char *string, int len, FPOINT pos, int justif, float size, float stretch, FontType font, FPOINT &start, FPOINT &end);
    void        DimText(char *string, FPOINT pos, int justif, float size, float stretch, FontType font, FPOINT &start, FPOINT &end);

    float       RetAscent(float size, FontType font);
    float       RetDescent(float size, FontType font);
    float       RetHeight(float size, FontType font);

    float       RetStringWidth(char *string, char *format, int len, float size, float stretch);
    float       RetStringWidth(char *string, int len, float size, float stretch, FontType font);
    float       RetCharWidth(int character, float offset, float size, float stretch, FontType font);

    int         Justif(char *string, char *format, int len, float width, float size, float stretch);
    int         Justif(char *string, int len, float width, float size, float stretch, FontType font);
    int         Detect(char *string, char *format, int len, float offset, float size, float stretch);
    int         Detect(char *string, int len, float offset, float size, float stretch, FontType font);

protected:
    void        DrawString(char *string, char *format, int len, FPOINT pos, float width, float size, float stretch, int eol);
    void        DrawString(char *string, int len, FPOINT pos, float width, float size, float stretch, FontType font, int eol);
    void        DrawColor(FPOINT pos, float size, float width, int color);
    void        DrawChar(int character, FPOINT pos, float size, float stretch, FontType font);

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*       m_engine;
    LPDIRECT3DDEVICE7 m_pD3DDevice;

};


#endif //_TEXT_H_
