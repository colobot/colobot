// text.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "text.h"



#define WIDTH_COLOBOT	34.0f
#define HEIGHT_COLOBOT	34.0f
#define WIDTH_HILITE	30.0f
#define HEIGHT_HILITE	28.0f



static short table_text_colobot[] =
{
//	x1,	y1,	x2,	y2
	240,140,254,167,	// 0
	196,168,204,182,	// .
	246,168,254,182,	// carré
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	205,168,214,182,	// \t (... verticaux)
	214,168,224,182,	// \n (flèche _/)
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,	// \r
	240,140,254,167,
	240,140,254,167,
	236,168,244,182,	// >
	226,168,234,182,	// <
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,

	0,	0,	6,	27,		// 32
	8,	0,	13,	27,		// !
	15,	0,	24,	27,		// "
	25,	0,	46,	27,		// #
	47,	0,	59,	27,		// $
	60,	0,	78,	27,		// %
	79,	0,	95,	27,		// &
	96,	0,	101,27,		// '
	104,0,	112,27,		// (
	114,0,	121,27,		// )
	122,0,	134,27,		// *
	135,0,	146,27,		// +
	148,0,	155,27,		// ,
	155,0,	164,27,		// -
	165,0,	171,27,		// .
	171,0,	182,27,		// /
	195,0,	209,27,		// 0
	210,0,	224,27,		// 1
	225,0,	239,27,		// 2
	240,0,	254,27,		// 3
	0,	28,	14,	55,		// 4
	15,	28,	29,	55,		// 5
	30,	28,	44,	55,		// 6
	45,	28,	59,	55,		// 7
	60,	28,	74,	55,		// 8
	75,	28,	89,	55,		// 9
	92,	28,	99,	55,		// :
	99,	28,	106,55,		// ;
	106,28,	114,55,		// <
	115,28,	125,55,		// =
	127,28,	136,55,		// >
	138,28,	150,55,		// ?

	151,28,	171,55,		// 64
	172,28,	187,55,		// A
	188,28,	200,55,		// B
	201,28,	215,55,		// C
	216,28,	230,55,		// D
	231,28,	245,55,		// E
	0,	56,	13,	83,		// F
	14,	56,	30,	83,		// G
	31,	56,	47,	83,		// H
	48,	56,	61,	83,		// I
	62,	56,	77,	83,		// J
	78,	56,	91,	83,		// K
	92,	56,	104,83,		// L
	105,56,	125,83,		// M
	126,56,	144,83,		// N
	145,56,	163,83,		// O
	164,56,	176,83,		// P
	177,56,	197,83,		// Q
	198,56,	212,83,		// R
	213,56,	228,83,		// S
	229,56,	245,83,		// T
	0,	84,	16,	111,	// U
	17,	84,	32,	111,	// V
	33,	84,	56,	111,	// W
	58,	84,	75,	111,	// X
	76,	84,	91,	111,	// Y
	92,	84,	108,111,	// Z
	109,84,	119,111,	// [
	120,84,	131,111,	// \ 
	131,84,	141,111,	// ]
	141,84,	152,111,	// ^
	153,84,	169,111,	// _

	240,140,254,167,	// 96
	170,84,	182,111,	// a
	183,84,	196,111,	// b
	197,84,	208,111,	// c
	209,84,	222,111,	// d
	223,84,	235,111,	// e
	236,84,	247,111,	// f
	0,	112,11,	139,	// g
	13,	112,24,	139,	// h
	27,	112,32,	139,	// i
	32,	112,42,	139,	// j
	44,	112,56,	139,	// k
	58,	112,63,	139,	// l
	65,	112,80,	139,	// m
	81,	112,92,	139,	// n
	93,	112,104,139,	// o
	106,112,117,139,	// p
	118,112,129,139,	// q
	131,112,141,139,	// r
	143,112,153,139,	// s
	154,112,165,139,	// t
	166,112,177,139,	// u
	178,112,190,139,	// v
	191,112,207,139,	// w
	208,112,222,139,	// x
	223,112,236,139,	// y
	237,112,249,139,	// z
	0,	140,10,	167,	// {
	11,	140,17,	167,	// |
	17,	140,28,	167,	// }
	28,	140,41,	167,	// ~
	219,140,225,167,	// 

	219,34,	225,50,		// 128 80
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	165,0,	171,27,		// 133 85 ...
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,

	219,34,	225,50,		// 144
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,

	240,140,254,167,	// 160
	205,168,208,195,	// 161 A1 ! inversé
	240,140,254,167,
	240,140,254,167,	// 163 A3 £
	240,140,254,167,
	240,140,254,167,
	0,	0,	4,	16,		// 166 A6 ¦ (cadratin)
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,

	240,140,254,167,	// 176
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
	192,168,204,195,	// 191 BF ? inversé

	172,28,	187,55,		// 192 C0 à maj
	172,28,	187,55,		// 193 C1 á maj
	172,28,	187,55,		// 194 C2 â maj
	172,28,	187,55,		// 195 C3 ã maj
	60, 168,75, 195,	// 196 C4 ä maj
	240,140,254,167,
	240,140,254,167,
	201,28,	215,55,		// 199 C7 ç maj
	231,28,	245,55,		// 200 C8 è maj
	231,28,	245,55,		// 201 C9 é maj
	231,28,	245,55,		// 202 CA ê maj
	76, 168,90, 195,	// 203 CB ë maj
	48,	56,	61,	83,		// 236 EC ì maj
	48,	56,	61,	83,		// 205 CD í maj
	48,	56,	61,	83,		// 206 CE î maj
	91,	168,104,195,	// 207 CF ï maj

	240,140,254,167,	// 208
	126,56,	144,83,		// 209 D1 ñ maj
	240,140,254,167,
	145,56,	163,83,		// 211 D3 ó maj
	145,56,	163,83,		// 212 D4 ô maj
	145,56,	163,83,		// 245 D5 õ maj
	105,168,123,195,	// 214 D6 ö maj
	240,140,254,167,
	240,140,254,167,
	0,	84,	16,	111,	// 217 D9 ù maj
	0,	84,	16,	111,	// 218 DA ú maj
	0,	84,	16,	111,	// 219 DB û maj
	124,168,140,195,	// 220 DC ü maj
	240,140,254,167,
	240,140,254,167,
	141,168,151,195,	// 223 DF ss allemand

	55,	140,67,	167,	// 224 E0 à min
	42,	140,54,	167,	// 225 E1 á min
	68,	140,80,	167,	// 226 E2 â min
	165,168,178,195,	// 227 E3 ã min
	81,	140,93,	167,	// 228 E4 ä min
	240,140,254,167,
	240,140,254,167,
	48,	168,59,	195,	// 231 E7 ç min
	107,140,119,167,	// 232 E8 è min
	94,	140,106,167,	// 233 E9 é min
	120,140,132,167,	// 234 EA ê min
	134,140,146,167,	// 235 EB ë min
	229,140,235,167,	// 236 EC ì min
	222,140,228,167,	// 237 ED í min
	147,140,158,167,	// 238 EE î min
	160,140,169,167,	// 239 EF ï min

	240,140,254,167,	// 240
	152,168,164,195,	// 241 F1 ñ min
	240,140,254,167,
	170,140,181,167,	// 243 F3 ó min
	196,140,207,167,	// 244 F4 ô min
	179,168,191,195,	// 245 F5 õ min
	209,140,220,167,	// 246 F6 ö min
	240,140,254,167,
	240,140,254,167,
	12,	168,23,	195,	// 249 F9 ù min
	0,	168,11,	195,	// 250 FA ú min
	24,	168,35,	195,	// 251 FB û min
	36,	168,47,	195,	// 252 FC ü min
	240,140,254,167,
	240,140,254,167,
	240,140,254,167,
};



// Retourne le pointeur à la table selon la fonte.

short* RetTable(FontType font)
{
	return table_text_colobot;
}



// Constructeur de l'objet.

CText::CText(CInstanceManager* iMan, CD3DEngine* engine)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_TEXT, this);

	m_pD3DDevice = 0;
	m_engine = engine;
}

// Destructeur de l'objet.

CText::~CText()
{
	m_iMan->DeleteInstance(CLASS_TEXT, this);
}


void CText::SetD3DDevice(LPDIRECT3DDEVICE7 device)
{
	m_pD3DDevice = device;
}


// Affiche un texte multi-fonte.
// La position verticale est en bas de la boîte du caractère.

void CText::DrawText(char *string, char *format, int len, FPOINT pos,
					 float width, int justif, float size, float stretch,
					 int eol)
{
	float	sw;

	if ( justif == 0 )  // centré ?
	{
		sw = RetStringWidth(string, format, len, size, stretch);
		if ( sw > width )  sw = width;
		pos.x -= sw/2.0f;
	}
	if ( justif < 0 )  // drapeau à gauche ?
	{
		sw = RetStringWidth(string, format, len, size, stretch);
		if ( sw > width )  sw = width;
		pos.x -= sw;
	}
	DrawString(string, format, len, pos, width, size, stretch, eol);
}

// Affiche un texte multi-fonte.
// La position verticale est en bas de la boîte du caractère.

void CText::DrawText(char *string, char *format, FPOINT pos, float width,
					 int justif, float size, float stretch,
					 int eol)
{
	DrawText(string, format, strlen(string), pos, width, justif, size, stretch, eol);
}

// Affiche un texte.
// La position verticale est en bas de la boîte du caractère.

void CText::DrawText(char *string, int len, FPOINT pos, float width,
					 int justif, float size, float stretch, FontType font,
					 int eol)
{
	float	sw;

	if ( justif == 0 )  // centré ?
	{
		sw = RetStringWidth(string, len, size, stretch, font);
		if ( sw > width )  sw = width;
		pos.x -= sw/2.0f;
	}
	if ( justif < 0 )  // drapeau à gauche ?
	{
		sw = RetStringWidth(string, len, size, stretch, font);
		if ( sw > width )  sw = width;
		pos.x -= sw;
	}
	DrawString(string, len, pos, width, size, stretch, font, eol);
}

// Affiche un texte.
// La position verticale est en bas de la boîte du caractère.

void CText::DrawText(char *string, FPOINT pos, float width,
					 int justif, float size, float stretch, FontType font,
					 int eol)
{
	DrawText(string, strlen(string), pos, width, justif, size, stretch, font, eol);
}


// Retourne les dimensions d'un texte multi-fonte.

void CText::DimText(char *string, char *format, int len, FPOINT pos,
					int justif, float size, float stretch,
					FPOINT &start, FPOINT &end)
{
	float	sw;

	start = end = pos;

	sw = RetStringWidth(string, format, len, size, stretch);
	end.x += sw;
	if ( justif == 0 )  // centré ?
	{
		start.x -= sw/2.0f;
		end.x   -= sw/2.0f;
	}
	if ( justif < 0 )  // drapeau à gauche ?
	{
		start.x -= sw;
		end.x   -= sw;
	}

	start.y -= RetDescent(size, FONT_COLOBOT);
	end.y   += RetAscent(size, FONT_COLOBOT);
}
	
// Retourne les dimensions d'un texte multi-fonte.

void CText::DimText(char *string, char *format, FPOINT pos, int justif,
					float size, float stretch,
					FPOINT &start, FPOINT &end)
{
	DimText(string, format, strlen(string), pos, justif, size, stretch, start, end);
}

// Retourne les dimensions d'un texte.

void CText::DimText(char *string, int len, FPOINT pos, int justif,
					float size, float stretch, FontType font,
					FPOINT &start, FPOINT &end)
{
	float	sw;

	start = end = pos;

	sw = RetStringWidth(string, len, size, stretch, font);
	end.x += sw;
	if ( justif == 0 )  // centré ?
	{
		start.x -= sw/2.0f;
		end.x   -= sw/2.0f;
	}
	if ( justif < 0 )  // drapeau à gauche ?
	{
		start.x -= sw;
		end.x   -= sw;
	}

	start.y -= RetDescent(size, font);
	end.y   += RetAscent(size, font);
}
	
// Retourne les dimensions d'un texte.

void CText::DimText(char *string, FPOINT pos, int justif,
					float size, float stretch, FontType font,
					FPOINT &start, FPOINT &end)
{
	DimText(string, strlen(string), pos, justif, size, stretch, font, start, end);
}


// Retourne la hauteur au-dessus de la ligne de base.

float CText::RetAscent(float size, FontType font)
{
	if ( font == FONT_HILITE )  return (21.0f/256.0f)*(size/HEIGHT_HILITE);
	else                        return (21.0f/256.0f)*(size/HEIGHT_COLOBOT);
}

// Retourne la hauteur au-dessous de la ligne de base.

float CText::RetDescent(float size, FontType font)
{
	if ( font == FONT_HILITE )  return (7.0f/256.0f)*(size/HEIGHT_HILITE);
	else                        return (7.0f/256.0f)*(size/HEIGHT_COLOBOT);
}

// Retourne la hauteur totale du caractère.

float CText::RetHeight(float size, FontType font)
{
	if ( font == FONT_HILITE )  return (28.0f/256.0f)*(size/HEIGHT_HILITE);
	else                        return (28.0f/256.0f)*(size/HEIGHT_COLOBOT);
}

	
// Retourne la largeur d'une chaîne de caractères multi-fonte.

float CText::RetStringWidth(char *string, char *format, int len,
							float size, float stretch)
{
	FontType	font;
	float		st, ss, tab, w, width = 0.0f;
	short		*table, *pt;
	int			i, c;

	for ( i=0 ; i<len ; i++ )
	{
		font = (FontType)(format[i]&FONT_MASK);
		table = RetTable(font);
		c = (unsigned char)string[i];

		if ( font == FONT_HILITE )  ss = size/WIDTH_HILITE;
		else                        ss = size/WIDTH_COLOBOT;

		if ( c == '\t' )
		{
			pt = table+' '*4;
			tab = (float)(pt[2]-pt[0])/256.0f*ss*stretch*m_engine->RetEditIndentValue();
			w = tab-Mod(width, tab);
			if ( w < tab*0.1f )  w += tab;
			width += w;
			continue;
		}

		if ( c > 255 )  continue;

		pt = table+c*4;
		st = stretch;
		width += (float)(pt[2]-pt[0])/256.0f*ss*st;
	}

	return width;
}

// Retourne la largeur d'une chaîne de caractères.

float CText::RetStringWidth(char *string, int len,
							float size, float stretch, FontType font)
{
	float	st, ss, tab, w, width = 0.0f;
	short	*table, *pt;
	int		i, c;

	if ( font == FONT_HILITE )  ss = size/WIDTH_HILITE;
	else                        ss = size/WIDTH_COLOBOT;

	table = RetTable(font);
	for ( i=0 ; i<len ; i++ )
	{
		c = (unsigned char)string[i];

		if ( c == '\t' )
		{
			pt = table+' '*4;
			tab = (float)(pt[2]-pt[0])/256.0f*ss*stretch*m_engine->RetEditIndentValue();
			w = tab-Mod(width, tab);
			if ( w < tab*0.1f )  w += tab;
			width += w;
			continue;
		}

		if ( c > 255 )  continue;

		pt = table+c*4;
		st = stretch;
		width += (float)(pt[2]-pt[0])/256.0f*ss*st;
	}

	return width;
}

// Retourne la largeur d'un caractère.
// 'offset' est la position actuelle dans la ligne.

float CText::RetCharWidth(int character, float offset,
						  float size, float stretch, FontType font)
{
	float		st, ss, tab, w;
	short*		pt;

	if ( font == FONT_HILITE )  ss = size/WIDTH_HILITE;
	else                        ss = size/WIDTH_COLOBOT;

	if ( character == '\t' )
	{
		pt = RetTable(font)+' '*4;
		tab = (float)(pt[2]-pt[0])/256.0f*ss*stretch*m_engine->RetEditIndentValue();
		w = tab-Mod(offset, tab);
		if ( w < tab*0.1f )  w += tab;
		return w;
	}

	if ( character > 255 )  return 0.0f;

	pt = RetTable(font)+character*4;
	st = stretch;
	return (float)(pt[2]-pt[0])/256.0f*ss*st;
}


// Justifie une ligne de texte multi-fonte. Retourne l'offset de la coupure.

int CText::Justif(char *string, char *format, int len, float width,
				  float size, float stretch)
{
	FontType	font;
	float		pos;
	int			i, character, cut;

	pos = 0.0f;
	cut = 0;
	for ( i=0 ; i<len ; i++ )
	{
		font = (FontType)(format[i]&FONT_MASK);
		character = (unsigned char)string[i];

		if ( character == 0 )
		{
			return i;
		}
		if ( character == '\n' )
		{
			return i+1;
		}
		if ( character == ' ' )
		{
			cut = i+1;
		}

		pos += RetCharWidth(character, pos, size, stretch, font);
		if ( pos > width )
		{
			if ( cut == 0 )  return i;
			else             return cut;
		}
	}
	return i;
}

// Justifie une ligne de texte. Retourne l'offset de la coupure.

int CText::Justif(char *string, int len, float width,
				  float size, float stretch, FontType font)
{
	float	pos;
	int		i, character, cut;

	pos = 0.0f;
	cut = 0;
	for ( i=0 ; i<len ; i++ )
	{
		character = (unsigned char)string[i];

		if ( character == 0 )
		{
			return i;
		}
		if ( character == '\n' )
		{
			return i+1;
		}
		if ( character == ' ' )
		{
			cut = i+1;
		}

		pos += RetCharWidth(character, pos, size, stretch, font);
		if ( pos > width )
		{
			if ( cut == 0 )  return i;
			else             return cut;
		}
	}
	return i;
}

// Retourne la position convenant le mieux à une offset donnée (multi-fonte).

int CText::Detect(char *string, char *format, int len, float offset,
				  float size, float stretch)
{
	FontType	font;
	float		pos, width;
	int			i, character, cut;

	pos = 0.0f;
	cut = 0;
	for ( i=0 ; i<len ; i++ )
	{
		font = (FontType)(format[i]&FONT_MASK);
		character = (unsigned char)string[i];

		if ( character == 0 )
		{
			return i;
		}
		if ( character == '\n' )
		{
			return i;
		}

		width = RetCharWidth(character, pos, size, stretch, font);
		if ( offset <= pos+width/2.0f )
		{
			return i;
		}
		pos += width;
	}
	return i;
}

// Retourne la position convenant le mieux à une offset donnée.

int CText::Detect(char *string, int len, float offset,
				  float size, float stretch, FontType font)
{
	float	pos, width;
	int		i, character, cut;

	pos = 0.0f;
	cut = 0;
	for ( i=0 ; i<len ; i++ )
	{
		character = (unsigned char)string[i];

		if ( character == 0    ||
			 character == '\n' )
		{
			return i;
		}

		width = RetCharWidth(character, pos, size, stretch, font);
		if ( offset <= pos+width/2.0f )
		{
			return i;
		}
		pos += width;
	}
	return i;
}


// Affiche un texte multi-fonte.

void CText::DrawString(char *string, char *format, int len, FPOINT pos,
					   float width, float size, float stretch, int eol)
{
	FontType	font;
	float		start, offset, cw;
	int			i, c;

	m_engine->SetTexture("text.tga");
	m_engine->SetState(D3DSTATETTw);

	font = FONT_COLOBOT;

	start = pos.x;
	offset = 0.0f;
	for ( i=0 ; i<len ; i++ )
	{
		font = (FontType)(format[i]&FONT_MASK);
		c = (unsigned char)string[i];
		cw = RetCharWidth(c, offset, size, stretch, font);

		if ( offset+cw > width )  // dépasse la largeur maximale ?
		{
//?			cw = RetCharWidth(16, offset, size, stretch, font);
//?			pos.x = start+width-cw;
//?			DrawChar(16, pos, size, stretch, font);  // >
			break;
		}

		if ( (format[i]&COLOR_MASK) != 0 )
		{
			DrawColor(pos, size, cw, format[i]&COLOR_MASK);
		}
		DrawChar(c, pos, size, stretch, font);
		offset += cw;
		pos.x += cw;
	}

	if ( eol != 0 )
	{
		DrawChar(eol, pos, size, stretch, font);
	}
}

// Affiche un texte.

void CText::DrawString(char *string, int len, FPOINT pos, float width,
					   float size, float stretch, FontType font,
					   int eol)
{
	float		start, offset, cw;
	int			i, c;

	m_engine->SetTexture("text.tga");
	m_engine->SetState(D3DSTATETTw);

	start = pos.x;
	offset = 0.0f;
	for ( i=0 ; i<len ; i++ )
	{
		c = (unsigned char)string[i];
		cw = RetCharWidth(c, offset, size, stretch, font);

		if ( offset+cw > width )  // dépasse la largeur maximale ?
		{
//?			cw = RetCharWidth(16, offset, size, stretch, font);
//?			pos.x = start+width-cw;
//?			DrawChar(16, pos, size, stretch, font);  // >
			break;
		}

		DrawChar(c, pos, size, stretch, font);
		offset += cw;
		pos.x += cw;
	}

	if ( eol != 0 )
	{
		DrawChar(eol, pos, size, stretch, font);
	}
}

// Affiche le lien d'un caractère.

void CText::DrawColor(FPOINT pos, float size, float width, int color)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	FPOINT		p1, p2;
	POINT		dim;
	D3DVECTOR	n;
	float		h, u1, u2, v1, v2, dp;
	int			icon;

	icon = -1;
	if ( color == COLOR_LINK  )  icon =  9;  // bleu
	if ( color == COLOR_TOKEN )  icon =  4;  // orange
	if ( color == COLOR_TYPE  )  icon =  5;  // vert
	if ( color == COLOR_CONST )  icon =  8;  // rouge
	if ( color == COLOR_REM   )  icon =  6;  // magenta
	if ( color == COLOR_KEY   )  icon = 10;  // gris
	if ( icon == -1 )  return;

	if ( color == COLOR_LINK )
	{
		m_engine->SetState(D3DSTATENORMAL);
	}

	dim = m_engine->RetDim();
	if ( dim.y <= 768.0f )  // 1024x768 ou moins ?
	{
		h = 1.01f/dim.y;  // 1 pixel
	}
	else	// plus que 1024x768 ?
	{
		h = 2.0f/dim.y;  // 2 pixels
	}

	p1.x = pos.x;
	p2.x = pos.x + width;

	if ( color == COLOR_LINK )
	{
		p1.y = pos.y;
		p2.y = pos.y + h;  // juste souligné
	}
	else
	{
#if 1
		p1.y = pos.y;
		p2.y = pos.y + (16.0f/256.0f)*(size/WIDTH_COLOBOT);
//?		p2.y = pos.y + h*4.0f;  // juste souligné épais
#else
		p1.y = pos.y;
		p2.y = pos.y + (16.0f/256.0f)*(size/WIDTH_COLOBOT)/4.0f;
#endif
	}

	u1 = (64.0f/256.0f)+(16.0f/256.0f)*(icon%16);
	v1 = (190.0f/256.0f);
	u2 = (16.0f/256.0f)+u1;
	v2 = (16.0f/256.0f)+v1;

	dp = 0.5f/256.0f;
	u1 += dp;
	v1 += dp;
	u2 -= dp;
	v2 -= dp;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);

	if ( color == COLOR_LINK )
	{
		m_engine->SetState(D3DSTATETTw);
	}
}

// Affiche un caractère.

void CText::DrawChar(int character, FPOINT pos, float size,
					 float stretch, FontType font)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	FPOINT		p1, p2;
	D3DVECTOR	n;
	float		width, height, u1, u2, v1, v2, dp, shift;
	short*		pt;

	dp = 0.5f/256.0f;
	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	if ( character > 255 )  return;

	pt = RetTable(font)+character*4;
	width  = (float)(pt[2]-pt[0])/256.0f*stretch*0.9f;
	height = (float)(pt[3]-pt[1])/256.0f;

	p1.x = pos.x;
	p1.y = pos.y;
	if ( font == FONT_HILITE )
	{
		shift = 1.5f/256.0f*stretch*(size/WIDTH_HILITE);
		width -= shift;
		p2.x = pos.x + width*(size/WIDTH_HILITE);
		p2.y = pos.y + height*(size/HEIGHT_HILITE);
	}
	else
	{
		p2.x = pos.x + width*(size/WIDTH_COLOBOT);
		p2.y = pos.y + height*(size/HEIGHT_COLOBOT);
	}

	u1 = (float)pt[0]/256.0f;
	v1 = (float)pt[1]/256.0f;
	u2 = (float)pt[2]/256.0f;
	v2 = (float)pt[3]/256.0f;

	u1 += dp;
	v1 += dp;
	u2 += dp;
	v2 += dp;

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);

	if ( font == FONT_HILITE )
	{
		p1.x += shift;
		p2.x += shift;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}
}

