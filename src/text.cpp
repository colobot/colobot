// text.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "text.h"



static short table_text_colobot[] =
{
//	x1,	y1,	x2,	y2
	219,34,	225,50,		// 0
	 1,188, 9,203,		// .
	51,188,59,203,		// carré
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	11,188,19,203,		// \t
	21,188,29,203,		// \n
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,		// \r
	219,34,	225,50,
	219,34,	225,50,
	41,188,49,203,		// >
	31,188,39,203,		// <
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

#if _NEWLOOK
	0,	0,	4,	16,		// 32
	7,	0,	9,	16,		// !
	9,	0,	13,	16,		// "
	13,	0,	24,	16,		// #
	24,	0,	31,	16,		// $
	31,	0,	43,	16,		// %
	43,	0,	54,	16,		// &
	54,	0,	56,	16,		// '
	56,	0,	61,	16,		// (
	61,	0,	67,	16,		// )
	67,	0,	73,	16,		// *
	73,	0,	83,	16,		// +
	83,	0,	87,	16,		// ,
	87,	0,	92,	16,		// -
	92,	0,	94,	16,		// .
	94,	0,	101,16,		// /
	101,0,	109,16,		// 0
	109,0,	114,16,		// 1
	114,0,	122,16,		// 2
	122,0,	129,16,		// 3
	129,0,	138,16,		// 4
	138,0,	146,16,		// 5
	146,0,	154,16,		// 6
	154,0,	161,16,		// 7
	161,0,	169,16,		// 8
	169,0,	177,16,		// 9
	177,0,	179,16,		// :
	179,0,	183,16,		// ;
	183,0,	193,16,		// <
	193,0,	203,16,		// =
	203,0,	213,16,		// >
	213,0,	219,16,		// ?

	0,	17,	14,	33,		// @ 64
	14,	17,	26,	33,		// A
	26,	17,	33,	33,		// B
	33,	17,	42,	33,		// C
	42,	17,	51,	33,		// D
	51,	17,	58,	33,		// E
	58,	17,	63,	33,		// F
	63,	17,	73,	33,		// G
	73,	17,	82,	33,		// H
	82,	17,	84,	33,		// I
	84,	17,	90,	33,		// J
	90,	17,	98,	33,		// K
	98,	17,	103,33,		// L
	103,17,	115,33,		// M
	115,17,	124,33,		// N
	124,17,	136,33,		// O
	136,17,	142,33,		// P
	142,17,	154,33,		// Q
	154,17,	160,33,		// R
	160,17,	167,33,		// S
	167,17,	175,33,		// T
	175,17,	183,33,		// U
	183,17,	194,33,		// V
	194,17,	208,33,		// W
	208,17,	218,33,		// X
	218,17,	227,33,		// Y
	227,17,	236,33,		// Z
	236,17,	241,33,		// [
	241,17,	248,33,		// \ 
	248,17,	252,33,		// ]
	219,0,	229,16,		// ^
	0,	34,	9,	50,		// _

	54,	0,	56,	16,		// ` 96
	9,	34,	16,	50,		// a
	16,	34,	25,	50,		// b
	25,	34,	33,	50,		// c
	33,	34,	42,	50,		// d
	42,	34,	50,	50,		// e
	50,	34,	55,	50,		// f
	55,	34,	62,	50,		// g
	62,	34,	69,	50,		// h
	69,	34,	71,	50,		// i
	71,	34,	75,	50,		// j
	75,	34,	81,	50,		// k
	81,	34,	83,	50,		// l
	83,	34,	93,	50,		// m
	93,	34,	100,50,		// n
	100,34,	109,50,		// o
	109,34,	118,50,		// p
	118,34,	127,50,		// q
	127,34,	132,50,		// r
	132,34,	138,50,		// s
	138,34,	143,50,		// t
	143,34,	150,50,		// u
	150,34,	158,50,		// v
	158,34,	171,50,		// w
	171,34,	179,50,		// x
	179,34,	187,50,		// y
	187,34,	195,50,		// z
	195,34,	201,50,		// {
	201,34,	203,50,		// |
	203,34,	209,50,		// }
	209,34,	219,50,		// ~
	219,34,	228,50,		// 

	219,34,	225,50,		// 128
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

	219,34,	225,50,		// 160
	219,34,	225,50,		// 161 A1 ! inversé
	219,34,	225,50,
	219,34,	225,50,		// 163 A3 £
	219,34,	225,50,
	219,34,	225,50,
	0,	0,	4,	16,		// 166 A6 ¦ (cadratin)
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,

	219,34,	225,50,		// 176
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
	219,34,	225,50,		// 191 BF ? inversé

	12,	51,	24,	67,		// 192 C0 à maj
	0,	51,	12,	67,		// 193 C1 á maj
	24,	51,	36,	67,		// 194 C2 â maj
	48,	51,	60,	67,		// 195 C3 ã maj
	36,	51,	48,	67,		// 196 C4 ä maj
	219,34,	225,50,
	219,34,	225,50,
	60,	51,	69,	67,		// 199 C7 ç maj
	77,	51,	84,	67,		// 200 C8 è maj
	70,	51,	77,	67,		// 201 C9 é maj
	85,	51,	92,	67,		// 202 CA ê maj
	93,	51,	100,67,		// 203 CB ë maj
	219,34,	225,50,
	100,51,	104,67,		// 205 CD í maj
	108,51,	113,67,		// 206 CE î maj
	113,51,	117,67,		// 207 CF ï maj

	219,34,	225,50,		// 208
	117,51,	126,67,		// 209 D1 ñ maj
	219,34,	225,50,
	126,51,	138,67,		// 211 D3 ó maj
	150,51,	162,67,		// 212 D4 ô maj
	219,34,	225,50,
	162,51,	174,67,		// 214 D6 ö maj
	219,34,	225,50,
	219,34,	225,50,
	194,51,	202,67,		// 217 D9 ù maj
	186,51,	194,67,		// 218 DA ú maj
	202,51,	210,67,		// 219 DB û maj
	210,51,	218,67,		// 220 DC ü maj
	219,34,	225,50,
	219,34,	225,50,
	218,51,	227,67,		// 223 DF ss allemand

	7,	68,	14,	84,		// 224 E0 à min
	0,	68,	7,	84,		// 225 E1 á min
	14,	68,	21,	84,		// 226 E2 â min
	28,	68,	35,	84,		// 227 E3 ã min
	21,	68,	28,	84,		// 228 E4 ä min
	219,34,	225,50,
	219,34,	225,50,
	35,	68,	43,	84,		// 231 E7 ç min
	51,	68,	59,	84,		// 232 E8 è min
	43,	68,	51,	84,		// 233 E9 é min
	59,	68,	67,	84,		// 234 EA ê min
	67,	68,	75,	84,		// 235 EB ë min
	219,34,	225,50,
	75,	68,	79,	84,		// 237 ED í min
	83,	68,	88,	84,		// 238 EE î min
	88,	68,	92,	84,		// 239 EF ï min

	219,34,	225,50,		// 240
	92,	68,	99,	84,		// 241 F1 ñ min
	219,34,	225,50,
	99,	68,	108,84,		// 243 F3 ó min
	117,68,	126,84,		// 244 F4 ô min
	219,34,	225,50,
	126,68,	135,84,		// 246 F6 ö min
	219,34,	225,50,
	219,34,	225,50,
	151,68,	158,84,		// 249 F9 ù min
	144,68,	151,84,		// 250 FA ú min
	158,68,	165,84,		// 251 FB û min
	165,68,	172,84,		// 252 FC ü min
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
#else
	0,	0,	4,	16,		// 32
	4,	0,	7,	16,		// !
	7,	0,	13,	16,
	14,	0,	21,	16,
	22,	0,	28,	16,
	29,	0,	38,	16,
	39,	0,	48,	16,
	48,	0,	51,	16,
	51,	0,	55,	16,
	55,	0,	59,	16,
	59,	0,	65,	16,
	66,	0,	72,	16,
	73,	0,	76,	16,
	76,	0,	82,	16,
	82,	0,	85,	16,
	85,	0,	90,	16,
	90,	0,	97,	16,
	98,	0,	103,16,
	104,0,	111,16,
	111,0,	118,16,
	118,0,	125,16,
	125,0,	132,16,
	132,0,	139,16,
	139,0,	146,16,
	146,0,	153,16,
	153,0,	160,16,
	160,0,	165,16,		// :
	164,0,	169,16,		// ;
	169,0,	177,16,		// <
	177,0,	185,16,		// =
	185,0,	193,16,		// >
	193,0,	201,16,		// ?

	201,0,	215,16,		// 64
	0,	17,	10,	33,		// A
	10,	17,	18,	33,
	19,	17,	28,	33,
	28,	17,	36,	33,
	37,	17,	44,	33,
	45,	17,	52,	33,
	53,	17,	62,	33,
	63,	17,	71,	33,
	72,	17,	75,	33,
	75,	17,	82,	33,
	83,	17,	91,	33,
	92,	17,	99,	33,
	100,17,	110,33,
	111,17,	119,33,
//?	120,17,	129,33,		// O
	216,0,	227,16,		// O
	130,17,	138,33,
	139,17,	148,33,
	149,17,	158,33,
	158,17,	166,33,
	166,17,	175,33,
	175,17,	183,33,
	183,17,	193,33,
	193,17,	207,33,
	207,17,	215,33,
	215,17,	224,33,
	224,17,	232,33,		// Z
	232,17,	236,33,
	236,17,	241,33,
	241,17,	245,33,
	245,17,	252,33,		// ^
	0,	34,	8,	50,		// _

	45,	17,	52,	33,		// 96
	8,	34,	15,	50,		// a
	16,	34,	23,	50,
	24,	34,	31,	50,
	31,	34,	38,	50,
	39,	34,	46,	50,
	46,	34,	52,	50,
	52,	34,	59,	50,
	60,	34,	67,	50,
	68,	34,	71,	50,
	71,	34,	76,	50,
	77,	34,	84,	50,
	84,	34,	87,	50,
	88,	34,	99,	50,
	100,34,	107,50,
//?	108,34,	115,50,		// o
	238,0,	246,16,		// o
	116,34,	123,50,
	124,34,	131,50,
	132,34,	137,50,
	137,34,	144,50,
	144,34,	149,50,
	149,34,	156,50,
	156,34,	164,50,
	164,34,	176,50,
	176,34,	183,50,
	183,34,	191,50,
	191,34,	197,50,		// z
	197,34,	203,50,
	203,34,	205,50,
	205,34,	211,50,
	211,34,	219,50,
	219,34,	225,50,

#if _POLISH
	219,34,	225,50,		// 128
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
	0,	51,	8,	67,		// 140 S´
	219,34,	225,50,
	219,34,	225,50,
	9,	51,	17,	67,		// 143 Z´

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
	0,	68,	7,	84,		// 156 s´
	219,34,	225,50,
	219,34,	225,50,
	8,	68,	14,	84,		// 159 z´

	219,34,	225,50,		// 160
	219,34,	225,50,
	219,34,	225,50,
	18,	51,	27,	67,		// 163 L/
	219,34,	225,50,
	28,	51,	39,	67,		// 165 A,
	0,	0,	4,	16,		// 166 A6 ¦ (cadratin)
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	40,	51,	48,	67,		// 175 Zo

	219,34,	225,50,		// 176
	219,34,	225,50,
	219,34,	225,50,
	16,	68,	21,	84,		// 179 l/
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	23,	68,	31,	84,		// 185 a,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	32,	68,	38,	84,		// 191 zo

	219,34,	225,50,		// 192
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	49,	51,	58,	67,		// 198 C´
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	59,	51,	66,	67,		// 202 E,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,

	219,34,	225,50,		// 208
	67,	51,	75,	67,		// 209 N´
	219,34,	225,50,
//?	76,	51,	85,	67,		// 211 O´
	86,	51,	97,	67,		// 211 O´
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

	219,34,	225,50,		// 224
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	39,	68,	46,	84,		// 230 c´
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	47,	68,	54,	84,		// 234 e,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,

	219,34,	225,50,		// 240
	55,	68,	62,	84,		// 241 n´
	219,34,	225,50,
//?	63,	68,	70,	84,		// 243 o´
	71,	68,	79,	84,		// 243 o´
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
#else
	219,34,	225,50,		// 128
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

	219,34,	225,50,		// 160
	219,34,	225,50,		// 161 A1 ! inversé
	219,34,	225,50,
	219,34,	225,50,		// 163 A3 £
	219,34,	225,50,
	219,34,	225,50,
	0,	0,	4,	16,		// 166 A6 ¦ (cadratin)
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,

	219,34,	225,50,		// 176
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
	219,34,	225,50,		// 191 BF ? inversé

	10,	51,	20,	67,		// 192 C0 à maj
	0,	51,	10,	67,		// 193 C1 á maj
	20,	51,	30,	67,		// 194 C2 â maj
	40,	51,	50,	67,		// 195 C3 ã maj
	30,	51,	40,	67,		// 196 C4 ä maj
	219,34,	225,50,
	219,34,	225,50,
	50,	51,	59,	67,		// 199 C7 ç maj
	67,	51,	74,	67,		// 200 C8 è maj
	59,	51,	66,	67,		// 201 C9 é maj
	75,	51,	82,	67,		// 202 CA ê maj
	83,	51,	90,	67,		// 203 CB ë maj
	219,34,	225,50,
	91,	51,	95,	67,		// 205 CD í maj
	100,51,	103,67,		// 206 CE î maj
	104,51,	109,67,		// 207 CF ï maj

	219,34,	225,50,		// 208
	109,51,	117,67,		// 209 D1 ñ maj
	219,34,	225,50,
	118,51,	127,67,		// 211 D3 ó maj
	138,51,	147,67,		// 212 D4 ô maj
	219,34,	225,50,
	148,51,	157,67,		// 214 D6 ö maj
	219,34,	225,50,
	219,34,	225,50,
	177,51,	185,67,		// 217 D9 ù maj
	168,51,	176,67,		// 218 DA ú maj
	186,51,	194,67,		// 219 DB û maj
	195,51,	203,67,		// 220 DC ü maj
	219,34,	225,50,
	219,34,	225,50,
	211,51,	220,67,		// 223 DF ss allemand

	8,	68,	15,	84,		// 224 E0 à min
	0,	68,	7,	84,		// 225 E1 á min
	16,	68,	23,	84,		// 226 E2 â min
	32,	68,	39,	84,		// 227 E3 ã min
	24,	68,	31,	84,		// 228 E4 ä min
	219,34,	225,50,
	219,34,	225,50,
	40,	68,	47,	84,		// 231 E7 ç min
	55,	68,	62,	84,		// 232 E8 è min
	47,	68,	54,	84,		// 233 E9 é min
	63,	68,	70,	84,		// 234 EA ê min
	71,	68,	78,	84,		// 235 EB ë min
	219,34,	225,50,
	79,	68,	83,	84,		// 237 ED í min
	88,	68,	92,	84,		// 238 EE î min
	92,	68,	97,	84,		// 239 EF ï min

	219,34,	225,50,		// 240
	97,	68,	104,84,		// 241 F1 ñ min
	219,34,	225,50,
	105,68,	112,84,		// 243 F3 ó min
	121,68,	128,84,		// 244 F4 ô min
	219,34,	225,50,
	129,68,	136,84,		// 246 F6 ö min
	219,34,	225,50,
	219,34,	225,50,
	153,68,	160,84,		// 249 F9 ù min
	145,68,	152,84,		// 250 FA ú min
	161,68,	168,84,		// 251 FB û min
	169,68,	176,84,		// 252 FC ü min
	219,34,	225,50,
	219,34,	225,50,
	219,34,	225,50,
#endif
#endif
};


static short table_text_courier[] =
{
//	 x1, y1, x2, y2
	231,137,239,153, 	// 0
	 1,188, 9,204,		// .
	51,188,59,204,		// carré
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	11,188,19,204,		// \t
	21,188,29,204,		// \n
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,	// \r
	231,137,239,153,
	231,137,239,153,
	41,188,49,204,		// >
	31,188,39,204,		// <
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,

	  1, 86,  9,102,	// 32
	 11, 86, 19,102,
	 21, 86, 29,102,
	 31, 86, 39,102,
	 41, 86, 49,102,
	 51, 86, 59,102,
	 61, 86, 69,102,
	 71, 86, 79,102,
	 81, 86, 89,102,
	 91, 86, 99,102,
	101, 86,109,102,
	111, 86,119,102,
	121, 86,129,102,
	131, 86,139,102,
	141, 86,149,102,
	151, 86,159,102,
	161, 86,169,102,
	171, 86,179,102,
	181, 86,189,102,
	191, 86,199,102,
	201, 86,209,102,
	211, 86,219,102,
	221, 86,229,102,
	231, 86,239,102,
	  1,103,  9,119,	// 56
	 11,103, 19,119,
	 21,103, 29,119,
	 31,103, 39,119,
	 41,103, 49,119,
	 51,103, 59,119,
	 61,103, 69,119,
	 71,103, 79,119,

	 81,103, 89,119,	// @
	 91,103, 99,119,
	101,103,109,119,
	111,103,119,119,
	121,103,129,119,
	131,103,139,119,
	141,103,149,119,
	151,103,159,119,
	161,103,169,119,
	171,103,179,119,
	181,103,189,119,
	191,103,199,119,
	201,103,209,119,
	211,103,219,119,
	221,103,229,119,
	231,103,239,119,
	  1,120,  9,136,	// P
	 11,120, 19,136,
	 21,120, 29,136,
	 31,120, 39,136,
	 41,120, 49,136,
	 51,120, 59,136,
	 61,120, 69,136,
	 71,120, 79,136,
	 81,120, 89,136,
	 91,120, 99,136,
	101,120,109,136,
	111,120,119,136,	// [
	121,120,129,136,
	131,120,139,136,
	141,120,149,136,
	151,120,159,136,	// _

	161,120,169,136,
	171,120,179,136,	// a
	181,120,189,136,
	191,120,199,136,
	201,120,209,136,
	211,120,219,136,
	221,120,229,136,
	231,120,239,136,
	  1,137,  9,153,
	 11,137, 19,153,
	 21,137, 29,153,
	 31,137, 39,153,
	 41,137, 49,153,
	 51,137, 59,153,
	 61,137, 69,153,
	 71,137, 79,153,	// o
	 81,137, 89,153,
	 91,137, 99,153,
	101,137,109,153,
	111,137,119,153,
	121,137,129,153,
	131,137,139,153,
	141,137,149,153,
	151,137,159,153,
	161,137,169,153,
	171,137,179,153,
	181,137,189,153,
	191,137,199,153,
	201,137,209,153,
	211,137,219,153,
	221,137,229,153,	// ~
	231,137,239,153,

#if _POLISH
	231,137,239,153,	// 128
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	  1,154,  9,170,	// 140 S´
	231,137,239,153,
	231,137,239,153,
	 11,154, 19,170,	// 143 Z´

	231,137,239,153,	// 144
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	  1,171,  9,187,	// 156 s´
	231,137,239,153,
	231,137,239,153,
	 11,171, 19,187,	// 159 z´

	231,137,239,153,	// 160
	231,137,239,153,
	231,137,239,153,
	 21,154, 29,170,	// 163 L/
	231,137,239,153,
	 31,154, 39,170,	// 165 A,
	  1, 86,  9,102,	// 166 A6 ¦ (cadratin)
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	 41,154, 49,170,	// 175 Zo

	231,137,239,153,	// 176
	231,137,239,153,
	231,137,239,153,
	 21,171, 29,187,	// 179 l/
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	 31,171, 39,187,	// 185 a,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	 41,171, 49,187,	// 191 zo

	231,137,239,153,	// 192
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	 51,154, 59,170,	// 198 C´
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	 61,154, 69,170,	// 202 E,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,

	231,137,239,153,	// 208
	 71,154, 79,170,	// 209 N´
	231,137,239,153,
	 81,171, 89,187,	// 211 O´
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,

	231,137,239,153,	// 224
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	 51,171, 59,187,	// 230 c´
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	 61,171, 69,187,	// 234 e,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,

	231,137,239,153,	// 240
	 71,171, 79,187,	// 241 n´
	231,137,239,153,
	 81,171, 89,187,	// 243 ó min
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
#else
	231,137,239,153,	// 128
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,

	231,137,239,153,	// 144
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,

	231,137,239,153,	// 160
	231,137,239,153,	// 161 A1 ! inversé
	231,137,239,153,
	231,137,239,153,	// 163 A3 £
	231,137,239,153,
	231,137,239,153,
	  1, 86,  9,102,	// 166 A6 ¦ (cadratin)
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,

	231,137,239,153,	// 176
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,	// 191 BF ? inversé

	 11,154, 19,170,	// à maj
	  1,154,  9,170,	// á maj
	 21,154, 29,170,	// â maj
	 41,154, 49,170,	// ã maj
	 31,154, 39,170,	// ä maj
	231,137,239,153,
	231,137,239,153,
	 51,154, 59,170,	// ç maj
	 71,154, 79,170,	// è maj
	 61,154, 69,170,	// é maj
	 81,154, 89,170,	// ê maj
	 91,154, 99,170,	// ë maj
	231,137,239,153,
	101,154,109,170,	// í maj
	121,154,129,170,	// î maj
	131,154,139,170,	// ï maj
	231,137,239,153,
	141,154,149,170,	// ñ maj
	231,137,239,153,
	151,154,159,170,	// ó maj
	171,154,179,170,	// ô maj
	231,137,239,153,
	181,154,189,170,	// ö maj
	231,137,239,153,
	231,137,239,153,
	211,154,219,170,	// ù maj
	201,154,209,170,	// ú maj
	221,154,229,170,	// û maj
	231,154,239,170,	// ü maj
	231,137,239,153,
	231,137,239,153,
	241,154,249,170,	// 223 DF ss allemand

	 11,171, 19,187,	// à min
	  1,171,  9,187,	// á min
	 21,171, 29,187,	// â min
	 41,171, 49,187,	// ã min
	 31,171, 39,187,	// ä min
	231,137,239,153,
	231,137,239,153,
	 51,171, 59,187,	// ç min
	 71,171, 79,187,	// è min
	 61,171, 69,187,	// é min
	 81,171, 89,187,	// ê min
	 91,171, 99,187,	// ë min
	231,137,239,153,
	111,171,119,187,	// ì min
	121,171,129,187,	// î min
	131,171,139,187,	// ï min
	231,137,239,153,
	141,171,149,187,	// ñ min
	231,137,239,153,
	151,171,159,187,	// ó min
	171,171,179,187,	// ô min
	231,137,239,153,
	181,171,189,187,	// ö min
	231,137,239,153,
	231,137,239,153,
	211,171,219,187,	// ù min
	201,171,209,187,	// ú min
	221,171,229,187,	// û min
	231,171,239,187,	// ü min
	231,137,239,153,
	231,137,239,153,
	231,137,239,153,
#endif
};


// Retourne le pointeur à la table selon la fonte.

short* RetTable(FontType font)
{
	if ( font == FONT_COLOBOT )  return table_text_colobot;
	else                         return table_text_courier;
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
	return (13.0f/256.0f)*(size/20.0f);
}

// Retourne la hauteur au-dessous de la ligne de base.

float CText::RetDescent(float size, FontType font)
{
	return (3.0f/256.0f)*(size/20.0f);
}

// Retourne la hauteur totale du caractère.

float CText::RetHeight(float size, FontType font)
{
	return (16.0f/256.0f)*(size/20.0f);
}

	
// Retourne la largeur d'une chaîne de caractères multi-fonte.

float CText::RetStringWidth(char *string, char *format, int len,
							float size, float stretch)
{
	FontType	font;
	float		st, tab, w, width = 0.0f;
	short		*table, *pt;
	int			i, c;

	for ( i=0 ; i<len ; i++ )
	{
		font = (FontType)(format[i]&FONT_MASK);
		if ( font == FONT_BUTTON )
		{
			width += (12.0f/256.0f)*(size/20.0f);
		}
		else
		{
			table = RetTable(font);
			c = (unsigned char)string[i];

			if ( c == '\t' )
			{
				pt = table+' '*4;
				tab = (float)(pt[2]-pt[0])/256.0f*(size/20.0f)*stretch*m_engine->RetEditIndentValue();
				w = tab-Mod(width, tab);
				if ( w < tab*0.1f )  w += tab;
				width += w;
				continue;
			}

			if ( c > 255 )  continue;

			pt = table+c*4;
			st = stretch;
			if ( font == FONT_COLOBOT && (c == 'O' || c == 'o') )  st = 0.8f;
			width += (float)(pt[2]-pt[0])/256.0f*(size/20.0f)*st;
		}
	}

	return width;
}

// Retourne la largeur d'une chaîne de caractères.

float CText::RetStringWidth(char *string, int len,
							float size, float stretch, FontType font)
{
	float	st, tab, w, width = 0.0f;
	short	*table, *pt;
	int		i, c;

	table = RetTable(font);
	for ( i=0 ; i<len ; i++ )
	{
		c = (unsigned char)string[i];

		if ( c == '\t' )
		{
			pt = table+' '*4;
			tab = (float)(pt[2]-pt[0])/256.0f*(size/20.0f)*stretch*m_engine->RetEditIndentValue();
			w = tab-Mod(width, tab);
			if ( w < tab*0.1f )  w += tab;
			width += w;
			continue;
		}

		if ( c > 255 )  continue;

		pt = table+c*4;
		st = stretch;
		if ( font == FONT_COLOBOT && (c == 'O' || c == 'o') )  st = 0.8f;
		width += (float)(pt[2]-pt[0])/256.0f*(size/20.0f)*st;
	}

	return width;
}

// Retourne la largeur d'un caractère.
// 'offset' est la position actuelle dans la ligne.

float CText::RetCharWidth(int character, float offset,
						  float size, float stretch, FontType font)
{
	float		st, tab, w;
	short*		pt;

	if ( font == FONT_BUTTON )  return (12.0f/256.0f)*(size/20.0f);

	if ( character == '\t' )
	{
		pt = RetTable(font)+' '*4;
		tab = (float)(pt[2]-pt[0])/256.0f*(size/20.0f)*stretch*m_engine->RetEditIndentValue();
		w = tab-Mod(offset, tab);
		if ( w < tab*0.1f )  w += tab;
		return w;
	}

	if ( character > 255 )  return 0.0f;

	pt = RetTable(font)+character*4;
	st = stretch;
#if !_NEWLOOK
	if ( font == FONT_COLOBOT && (character == 'O' || character == 'o') )  st = 0.8f;
#endif
	return (float)(pt[2]-pt[0])/256.0f*(size/20.0f)*st;
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
		if ( font != FONT_BUTTON )
		{
			if ( character == '\n' )
			{
				return i+1;
			}
			if ( character == ' ' )
			{
				cut = i+1;
			}
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
		if ( font != FONT_BUTTON )
		{
			if ( character == '\n' )
			{
				return i;
			}
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

#if _POLISH
	m_engine->SetTexture("textp.tga");
#else
	m_engine->SetTexture("text.tga");
#endif
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
			cw = RetCharWidth(16, offset, size, stretch, font);
			pos.x = start+width-cw;
			DrawChar(16, pos, size, stretch, font);  // >
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

#if _POLISH
	m_engine->SetTexture("textp.tga");
#else
	m_engine->SetTexture("text.tga");
#endif
	m_engine->SetState(D3DSTATETTw);

	start = pos.x;
	offset = 0.0f;
	for ( i=0 ; i<len ; i++ )
	{
		c = (unsigned char)string[i];
		cw = RetCharWidth(c, offset, size, stretch, font);

		if ( offset+cw > width )  // dépasse la largeur maximale ?
		{
			cw = RetCharWidth(16, offset, size, stretch, font);
			pos.x = start+width-cw;
			DrawChar(16, pos, size, stretch, font);  // >
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
		p2.y = pos.y + (16.0f/256.0f)*(size/20.0f);
//?		p2.y = pos.y + h*4.0f;  // juste souligné épais
#else
		p1.y = pos.y;
		p2.y = pos.y + (16.0f/256.0f)*(size/20.0f)/4.0f;
#endif
	}

	u1 = (16.0f/256.0f)*(icon%16);
	v1 = (240.0f/256.0f);
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
	float		width, height, u1, u2, v1, v2, dp;
	short*		pt;

	dp = 0.5f/256.0f;
	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	if ( font == FONT_BUTTON )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);

		p1.x = pos.x;
		p1.y = pos.y;
		p2.x = pos.x + (12.0f/256.0f)*(size/20.0f);
		p2.y = pos.y + (16.0f/256.0f)*(size/20.0f);

		if ( character <= 64 || character >= 128+56 )
		{
			u1 =  66.0f/256.0f;
			v1 =   2.0f/256.0f;
			u2 =  94.0f/256.0f;
			v2 =  30.0f/256.0f;
		}
		else
		{
			u1 = 224.0f/256.0f;
			v1 =  32.0f/256.0f;
			u2 = 256.0f/256.0f;
			v2 =  64.0f/256.0f;
		}

		u1 += dp;
		v1 += dp;
		u2 -= dp;
		v2 -= dp;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);

//?		p1.x += (12.0f/256.0f)*(size/20.0f)*0.1f;
//?		p1.y += (16.0f/256.0f)*(size/20.0f)*0.1f;
//?		p2.x -= (12.0f/256.0f)*(size/20.0f)*0.1f;
//?		p2.y -= (16.0f/256.0f)*(size/20.0f)*0.1f;

		if ( character >= 64 && character < 64+64 )
		{
			character -= 64;
			m_engine->SetTexture("button2.tga");
		}
		if ( character >= 128 && character < 128+64 )
		{
			character -= 128;
			m_engine->SetTexture("button3.tga");
		}

		m_engine->SetState(D3DSTATETTw);

		u1 = (32.0f/256.0f)*(character%8);
		v1 = (32.0f/256.0f)*(character/8);  // uv texture
		u2 = (32.0f/256.0f)+u1;
		v2 = (32.0f/256.0f)+v1;

		u1 += dp;
		v1 += dp;
		u2 -= dp;
		v2 -= dp;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);


#if _POLISH
		m_engine->SetTexture("textp.tga");
#else
		m_engine->SetTexture("text.tga");
#endif
		return;
	}

	if ( character > 255 )  return;

//?	if ( character == '\t' )  character = ' ';  // si tab, n'affiche pas ->

#if !_NEWLOOK
	if ( font == FONT_COLOBOT && (character == 'O' || character == 'o') )
	{
		stretch = 0.8f;
	}
#endif
	if ( font == FONT_COURIER )
	{
		stretch *= 1.2f;
	}

	pt = RetTable(font)+character*4;
	width  = (float)(pt[2]-pt[0])/256.0f*stretch*0.9f;
//?	width  = (float)(pt[2]-pt[0])/256.0f*stretch;
	height = (float)(pt[3]-pt[1])/256.0f;

#if _NEWLOOK
		pos.y += height*(size/20.0f)/17.0f;
#endif
	p1.x = pos.x;
	p1.y = pos.y;
	p2.x = pos.x + width*(size/20.0f);
	p2.y = pos.y + height*(size/20.0f);

	u1 = (float)pt[0]/256.0f;
	v1 = (float)pt[1]/256.0f;
	u2 = (float)pt[2]/256.0f;
	v2 = (float)pt[3]/256.0f;

	if ( font == FONT_COLOBOT )
	{
		u1 += dp;
		u2 += dp;
#if _NEWLOOK
		v2 += dp;
#endif
	}
	if ( font == FONT_COURIER )
	{
		u1 -= dp;
		u2 += dp*2.0f;
	}


	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}

