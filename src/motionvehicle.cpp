// motionvehicle.cpp

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
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "pyro.h"
#include "physics.h"
#include "brain.h"
#include "camera.h"
#include "modfile.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autohome.h"
#include "motion.h"
#include "motionbot.h"
#include "motionvehicle.h"



#define ARM_NEUTRAL_ANGLE1	 110.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE2	-130.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE3	 -50.0f*PI/180.0f



RemovePart tableCar01[] =	// tijuana taxi
{
	{10, 0, -1.0f,  6.0f,  0.0f, -1,-1,-1,-1, 1, 10},  // toît
	{11, 1, -4.2f,  5.4f,  0.0f, -1,-1,-1,-1, 1, 11},  // fenêtre ar
	{12, 0, -3.2f,  5.0f,  2.5f, -1,-1,-1,-1, 1, 12},  // fenêtre latérale g
	{13, 0, -3.2f,  5.0f, -2.5f, -1,-1,-1,-1, 1, 13},  // fenêtre latérale d
	{14, 3, -0.7f,  5.0f,  0.0f, -1,-1,-1,-1, 1, 14},  // montant central
	{15, 3,  1.7f,  4.7f,  0.0f, -1,-1,-1,-1, 0, 15},  // montant av
	{16, 2, -3.9f,  3.0f,  0.0f,  2, 3, 4, 5, 0, 16},  // carro ar
	{17, 4, -2.0f,  3.0f,  2.2f, -1,-1,-1,-1, 0, 17},  // carro g
	{18, 4, -2.0f,  3.0f, -2.2f, -1,-1,-1,-1, 0, 18},  // carro d
	{19, 4,  1.9f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 19},  // carro av
	{20, 0, -0.2f,  0.8f,  3.0f, -1,-1,-1,-1, 0, 20},  // marche pied g
	{21, 0, -0.2f,  0.8f, -3.0f, -1,-1,-1,-1, 0, 20},  // marche pied d
	{22, 1,  4.5f,  1.8f,  0.0f, -1,-1,-1,-1, 0, 22},  // radiateur
	{23, 0,  4.8f,  2.3f,  1.9f,  0,-1,-1,-1, 0, 23},  // phare g
	{24, 0,  4.8f,  2.3f, -1.9f,  1,-1,-1,-1, 0, 23},  // phare d
	{25, 2,  1.2f,  3.0f,  0.0f, -1,-1,-1,-1, 0, 25},  // tableau de bord
	{26, 0, -5.3f,  4.0f,  0.0f, -1,-1,-1,-1, 0, 26},  // malle 
	{27, 1, -4.6f,  2.5f,  0.9f, -1,-1,-1,-1, 0, 27},  // support g
	{28, 1, -4.6f,  2.5f, -0.9f, -1,-1,-1,-1, 0, 27},  // support d
	{29, 0, -3.0f,  3.0f,  3.1f, -1,-1,-1,-1, 0, 29},  // gardeboue ar g
	{30, 0, -3.0f,  3.0f, -3.1f, -1,-1,-1,-1, 0, 30},  // gardeboue ar d
	{-1}
};

RemovePart tableCar02[] =	// ford 32
{
	{10, 0, -0.7f,  5.5f,  0.0f, -1,-1,-1,-1, 0, 10},  // toît
	{11, 0, -2.6f,  4.8f,  0.0f, -1,-1,-1,-1, 0, 11},  // fenêtre ar
	{12, 1, -2.0f,  4.7f, -2.9f, -1,-1,-1,-1, 0, 12},  // fenêtre latérale d
	{13, 1, -2.0f,  4.7f,  2.9f, -1,-1,-1,-1, 0, 13},  // fenêtre latérale g
	{14, 1,  1.2f,  4.4f,  0.0f, -1,-1,-1,-1, 0, 14},  // montant av
	{15, 2, -1.1f,  2.5f, -2.9f, -1,-1,-1,-1, 0, 15},  // porte droite
	{16, 2, -1.1f,  2.5f,  2.9f, -1,-1,-1,-1, 0, 16},  // porte gauche
	{17, 3, -2.9f,  2.8f,  0.0f, -1,-1,-1,-1, 0, 17},  // carro ar
	{18, 3,  1.5f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 18},  // carro av
	{19, 0,  5.7f,  2.8f, -1.4f,  1,-1,-1,-1, 0, 19},  // phare av d
	{20, 0,  5.7f,  2.8f,  1.4f,  0,-1,-1,-1, 0, 19},  // phare av g
	{21, 2,  5.5f,  2.0f,  0.0f, -1,-1,-1,-1, 0, 21},  // radiateur
	{22, 2, -4.5f,  3.0f, -2.5f,  3, 5,-1,-1, 0, 22},  // aile ar d
	{23, 2, -4.5f,  3.0f,  2.5f,  2, 4,-1,-1, 0, 23},  // aile ar g
	{24, 0, -6.2f,  2.3f,  0.0f, -1,-1,-1,-1, 0, 24},  // coffre ar
	{25, 0, -6.5f,  1.9f,  0.0f, -1,-1,-1,-1, 0, 25},  // parchoc ar
	{26, 2,  0.8f,  3.2f,  0.0f, -1,-1,-1,-1, 0, 26},  // tableau de bord
	{-1}
};

RemovePart tableCar03[] =	// pickup
{
	{10, 0, -0.9f,  5.3f,  0.0f, -1,-1,-1,-1, 0, 10},  // toît
	{11, 0, -2.4f,  4.3f,  0.0f, -1,-1,-1,-1, 0, 11},  // fenêtre ar
	{12, 1, -2.0f,  4.4f, -1.9f, -1,-1,-1,-1, 0, 12},  // fenêtre latérale d
	{13, 1, -2.0f,  4.4f,  1.9f, -1,-1,-1,-1, 0, 13},  // fenêtre latérale g
	{14, 1,  0.6f,  4.3f,  0.0f, -1,-1,-1,-1, 0, 14},  // montant av
	{15, 0,  0.8f,  5.2f,  0.0f, -1,-1,-1,-1, 0, 15},  // visière
	{16, 0, -0.5f,  2.5f, -1.9f, -1,-1,-1,-1, 0, 16},  // porte droite
	{17, 0, -0.5f,  2.5f,  1.9f, -1,-1,-1,-1, 0, 17},  // porte gauche
	{18, 2, -2.4f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 18},  // carro ar
	{19, 2, -2.0f,  2.5f, -1.9f, -1,-1,-1,-1, 0, 19},  // carro latérale d
	{20, 2, -2.0f,  2.5f,  1.9f, -1,-1,-1,-1, 0, 20},  // carro latérale g
	{21, 2,  0.8f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 21},  // carro av
	{22, 0,  4.7f,  2.2f, -1.4f,  1,-1,-1,-1, 0, 22},  // phare av d
	{23, 0,  4.7f,  2.2f,  1.4f,  0,-1,-1,-1, 0, 22},  // phare av g
	{24, 1,  4.2f,  2.0f,  0.0f, -1,-1,-1,-1, 0, 24},  // radiateur
	{25, 0, -0.5f,  1.1f, -2.3f, -1,-1,-1,-1, 0, 25},  // marche pied d
	{26, 0, -0.5f,  1.1f,  2.3f, -1,-1,-1,-1, 0, 26},  // marche pied g
	{27, 0,  3.0f,  2.0f, -2.5f, -1,-1,-1,-1, 0, 27},  // garde-boue av d
	{28, 0,  3.0f,  2.0f,  2.5f, -1,-1,-1,-1, 0, 28},  // garde-boue av g
	{29, 0, -4.0f,  2.0f, -2.5f, -1,-1,-1,-1, 0, 29},  // garde-boue ar d
	{30, 0, -4.0f,  2.0f,  2.5f, -1,-1,-1,-1, 0, 30},  // garde-boue ar g
	{31, 0, -4.5f,  3.5f, -2.2f, -1,-1,-1,-1, 0, 31},  // barrière d
	{32, 0, -4.5f,  3.5f,  2.2f, -1,-1,-1,-1, 0, 32},  // barrière g
	{33, 0,  0.2f,  3.0f,  0.0f, -1,-1,-1,-1, 0, 33},  // tableau de bord
	{34, 0, -5.4f,  2.5f, -2.6f,  3, 5,-1,-1, 0, 34},  // phare ar d
	{35, 0, -5.4f,  2.5f,  2.6f,  2, 4,-1,-1, 0, 35},  // phare ar g
	{-1}
};

RemovePart tableCar04[] =	// firecraker
{
	{10, 1,  2.3f,  4.8f,  0.0f, -1,-1,-1,-1, 0, 10},  // fenêtre av
	{11, 0,  1.0f,  1.8f, -1.9f, -1,-1,-1,-1, 0, 11},  // triangle d
	{12, 0,  1.0f,  1.8f,  1.9f, -1,-1,-1,-1, 0, 12},  // triangle g
	{13, 0,  1.6f,  3.3f,  0.0f, -1,-1,-1,-1, 0, 13},  // tableau de bord
	{14, 2,  2.0f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 14},  // carro av
	{15, 2, -1.5f,  3.8f,  0.0f, -1,-1,-1,-1, 0, 15},  // siège
	{16, 0, -3.5f,  4.0f, -2.1f, -1,-1,-1,-1, 0, 16},  // barrière d
	{17, 0, -3.5f,  4.0f,  2.1f, -1,-1,-1,-1, 0, 16},  // barrière g
	{18, 0, -1.0f,  1.5f, -2.7f, -1,-1,-1,-1, 0, 18},  // marche pied d
	{19, 0, -1.0f,  1.5f,  2.7f, -1,-1,-1,-1, 0, 19},  // marche pied g
	{20, 0,  4.5f,  1.8f, -2.7f, -1,-1,-1,-1, 0, 20},  // garde-boue av d
	{21, 0,  4.5f,  1.8f,  2.7f, -1,-1,-1,-1, 0, 21},  // garde-boue av g
	{22, 0, -5.0f,  3.0f, -2.7f,  3, 5,-1,-1, 0, 22},  // garde-boue ar d
	{23, 0, -5.0f,  3.0f,  2.7f,  2, 4,-1,-1, 0, 23},  // garde-boue ar g
	{24, 0,  5.2f,  2.0f, -1.4f,  1,-1,-1,-1, 0, 24},  // phare av d
	{25, 0,  5.2f,  2.0f,  1.4f,  0,-1,-1,-1, 0, 25},  // phare av g
	{26, 0,  6.0f,  1.2f,  0.0f, -1,-1,-1,-1, 0, 26},  // parchoc av
	{27, 0, -7.0f,  2.0f,  0.0f, -1,-1,-1,-1, 0, 27},  // parchoc ar
	{28, 1,  4.5f,  2.0f,  0.0f, -1,-1,-1,-1, 0, 28},  // radiateur
	{29, 0,  3.3f,  3.5f,  0.0f, -1,-1,-1,-1, 0, 29},  // capot sup
	{30, 0,  3.3f,  3.0f, -1.0f, -1,-1,-1,-1, 0, 30},  // capot d
	{31, 0,  3.3f,  3.0f,  1.0f, -1,-1,-1,-1, 0, 31},  // capot g
	{32, 0,  2.3f,  4.0f, -2.3f, -1,-1,-1,-1, 0, 24},  // phare sup d
	{33, 0,  2.3f,  4.0f,  2.3f, -1,-1,-1,-1, 0, 25},  // phare sup g
	{-1}
};

RemovePart tableCar05[] =	// hooligan
{
	{10, 0,  1.0f,  4.3f,  0.0f, -1,-1,-1,-1, 0, 10},  // fenêtre av
	{11, 0, -0.4f,  2.6f, -2.2f, -1,-1,-1,-1, 0, 11},  // porte d
	{12, 0, -0.4f,  2.6f,  2.2f, -1,-1,-1,-1, 0, 12},  // porte g
	{13, 1,  1.3f,  2.7f,  0.0f, -1,-1,-1,-1, 0, 13},  // carro av
	{14, 1, -1.9f,  3.0f,  0.0f, -1,-1,-1,-1, 0, 14},  // carro ar
	{15, 1,  0.6f,  3.0f,  0.0f, -1,-1,-1,-1, 0, 15},  // tableau de bord
	{16, 0,  3.0f,  3.7f,  0.0f, -1,-1,-1,-1, 0, 16},  // capot
	{17, 1,  5.2f,  2.4f,  0.0f, -1,-1,-1,-1, 0, 17},  // radiateur
	{18, 0,  4.8f,  3.4f, -1.6f,  1,-1,-1,-1, 0, 18},  // phare av d
	{19, 0,  4.8f,  3.4f,  1.6f,  0,-1,-1,-1, 0, 18},  // phare av g
	{20, 1, -1.8f,  1.1f, -2.9f, -1,-1,-1,-1, 0, 20},  // marche pied d
	{21, 1, -1.8f,  1.1f,  2.9f, -1,-1,-1,-1, 0, 20},  // marche pied g
	{22, 0,  3.5f,  2.0f, -2.9f, -1,-1,-1,-1, 0, 22},  // garde-boue av d
	{23, 0,  3.5f,  2.0f,  2.9f, -1,-1,-1,-1, 0, 22},  // garde-boue av g
	{24, 0, -4.2f,  2.2f, -2.9f, -1,-1,-1,-1, 0, 24},  // garde-boue ar d
	{25, 0, -4.2f,  2.2f,  2.9f, -1,-1,-1,-1, 0, 24},  // garde-boue ar g
	{26, 0, -5.7f,  3.0f, -2.9f,  3, 5,-1,-1, 0, 26},  // phare ar d
	{27, 0, -5.7f,  3.0f,  2.9f,  2, 4,-1,-1, 0, 27},  // phare ar g
	{28, 1, -4.0f,  2.8f, -2.1f, -1,-1,-1,-1, 0, 28},  // aile ar d
	{29, 1, -4.0f,  2.8f,  2.1f, -1,-1,-1,-1, 0, 29},  // aile ar g
	{30, 0, -6.2f,  1.8f,  0.0f, -1,-1,-1,-1, 0, 30},  // parchoc arrière
	{-1}
};

RemovePart tableCar06[] =	// chevy
{
	{10, 0, -1.0f,  5.0f,  0.0f, -1,-1,-1,-1, 1, 10},  // toît
	{11, 0,  2.2f,  4.0f,  0.0f, -1,-1,-1,-1, 0, 11},  // fenêtre av
	{12, 0, -4.0f,  4.0f,  0.0f, -1,-1,-1,-1, 0, 12},  // fenêtre ar
	{13, 1,  1.7f,  4.0f, -2.6f, -1,-1,-1,-1, 0, 13},  // montant av d
	{14, 1,  1.7f,  4.0f,  2.6f, -1,-1,-1,-1, 0, 14},  // montant av g
	{15, 1, -3.5f,  3.8f, -2.6f, -1,-1,-1,-1, 0, 15},  // montant ar d
	{16, 1, -3.5f,  3.8f,  2.6f, -1,-1,-1,-1, 0, 16},  // montant ar g
	{17, 0, -0.2f,  2.2f, -2.9f, -1,-1,-1,-1, 0, 17},  // porte droite
	{18, 0, -0.2f,  2.2f,  2.9f, -1,-1,-1,-1, 0, 18},  // porte gauche
	{19, 0,  4.5f,  3.0f,  0.0f, -1,-1,-1,-1, 0, 19},  // capot av
	{20, 0, -5.7f,  3.3f,  0.0f, -1,-1,-1,-1, 0, 20},  // capot ar
	{21, 2,  4.0f,  2.2f, -2.5f,  1,-1,-1,-1, 0, 21},  // aile av d
	{22, 2,  4.0f,  2.2f,  2.5f,  0,-1,-1,-1, 0, 22},  // aile av g
	{23, 2, -4.5f,  2.5f, -2.7f,  3, 5,-1,-1, 0, 23},  // aile ar d
	{24, 2, -4.5f,  2.5f,  2.7f,  2, 4,-1,-1, 0, 24},  // aile ar g
	{25, 0,  6.7f,  1.6f,  0.0f, -1,-1,-1,-1, 0, 25},  // parchoc av
	{26, 0, -7.3f,  1.7f,  0.0f, -1,-1,-1,-1, 0, 26},  // parchoc ar
	{-1}
};

RemovePart tableCar07[] =	// reo
{
	{10, 0, -1.8f,  5.3f,  0.0f, -1,-1,-1,-1, 1, 10},  // toît
	{11, 1, -4.7f,  4.5f,  0.0f, -1,-1,-1,-1, 0, 11},  // fenêtre ar
	{12, 1, -4.2f,  4.5f, -1.9f, -1,-1,-1,-1, 1, 12},  // fenêtre latérale d
	{13, 1, -4.2f,  4.5f,  1.9f, -1,-1,-1,-1, 1, 13},  // fenêtre latérale g
	{14, 1, -1.2f,  4.5f, -1.9f, -1,-1,-1,-1, 1, 14},  // montant latéral d
	{15, 1, -1.2f,  4.5f,  1.9f, -1,-1,-1,-1, 1, 15},  // montant latéral g
	{16, 1,  1.2f,  4.5f,  0.0f, -1,-1,-1,-1, 0, 16},  // montant av
	{17, 0,  0.1f,  2.5f, -1.8f, -1,-1,-1,-1, 0, 17},  // porte droite
	{18, 0,  0.1f,  2.5f,  1.8f, -1,-1,-1,-1, 0, 18},  // porte gauche
	{19, 2, -5.0f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 19},  // carro ar
	{20, 2, -3.0f,  2.5f, -1.9f, -1,-1,-1,-1, 0, 20},  // carro latérale d
	{21, 2, -3.0f,  2.5f,  1.9f, -1,-1,-1,-1, 0, 21},  // carro latérale g
	{22, 2,  1.5f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 22},  // carro av
	{23, 0,  5.0f,  2.8f, -1.4f,  1,-1,-1,-1, 0, 23},  // phare av d
	{24, 0,  5.0f,  2.8f,  1.4f,  0,-1,-1,-1, 0, 24},  // phare av g
	{25, 1,  5.0f,  2.3f,  0.0f, -1,-1,-1,-1, 0, 25},  // radiateur
	{26, 0,  0.0f,  1.1f, -2.3f, -1,-1,-1,-1, 0, 26},  // marche pied d
	{27, 0,  0.0f,  1.1f,  2.3f, -1,-1,-1,-1, 0, 27},  // marche pied g
	{28, 0,  4.5f,  2.0f, -2.5f, -1,-1,-1,-1, 0, 28},  // garde-boue av d
	{29, 0,  4.5f,  2.0f,  2.5f, -1,-1,-1,-1, 0, 29},  // garde-boue av g
	{30, 0, -4.0f,  2.0f, -2.5f,  3, 5,-1,-1, 0, 30},  // garde-boue ar d
	{31, 0, -4.0f,  2.0f,  2.5f,  2, 4,-1,-1, 0, 31},  // garde-boue ar g
	{32, 0,  6.6f,  1.1f,  0.0f, -1,-1,-1,-1, 0, 32},  // parchoc av
	{33, 0, -6.3f,  1.1f,  0.0f, -1,-1,-1,-1, 0, 33},  // parchoc ar
	{34, 2,  0.9f,  3.3f,  0.0f, -1,-1,-1,-1, 0, 34},  // tableau de bord
	{-1}
};

RemovePart tableCar08[] =	// torpedo
{
	{10, 0,  1.5f,  4.2f, -0.5f, -1,-1,-1,-1, 0, 10},  // capot d
	{11, 0,  1.5f,  4.2f,  0.5f, -1,-1,-1,-1, 0, 11},  // capot g
	{12, 1,  5.5f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 12},  // radiateur
	{13, 1, -4.5f,  2.5f, -1.4f, -1,-1,-1,-1, 0, 13},  // aile d
	{14, 1, -4.5f,  2.5f,  1.4f, -1,-1,-1,-1, 0, 14},  // aile g
	{15, 0, -7.0f,  2.5f,  0.0f, -1,-1,-1,-1, 0, 15},  // coffre ar
	{16, 1, -2.1f,  3.0f,  0.0f, -1,-1,-1,-1, 0, 16},  // tableau de bord
	{17, 0, -4.2f,  2.5f,  1.8f, -1,-1,-1,-1, 0, 17},  // frein à main
	{-1}
};


// Calcule l'inclinaison d'un axe.

void CalcInclinaison(float hl, float hr, float radius,
					 float &angle, float &offset)
{
	offset = -(hl+hr)/2.0f;
	angle = atanf((hl+offset)/radius);
}

// Calcule l'offset d'une rotation décentrée.

FPOINT AdjustRot(float x, float y, float angle)
{
	FPOINT	p;
		
	p = RotatePoint(angle, FPOINT(-x,-y));
	return FPOINT(p.x+x, p.y+y);
}



// Constructeur de l'objet.

CMotionVehicle::CMotionVehicle(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	int		i;

	CMotion::CMotion(iMan, object);

	m_model = 0;
	m_posTrackLeft  = 0.0f;
	m_posTrackRight = 0.0f;
	m_partTotal = 0;
	m_partUsed = 0;
	m_topRest = 0;
	m_wheelLastPos   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_wheelLastAngle = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_flatTime = 0.0f;
	m_bWater = -1;
	m_bHelico = -1;
	m_lastWheelSpeed[0] = 0.0f;
	m_lastWheelSpeed[1] = 0.0f;

	m_lbAction = FALSE;
	m_lbStart = FALSE;
	m_lbLastParticule = 0.0;

	for ( i=0 ; i<6 ; i++ )
	{
		m_bLight[i] = TRUE;
	}
	for ( i=0 ; i<MAX_PART ; i++ )
	{
		m_part[i].bDeleted = TRUE;
	}
}

// Destructeur de l'objet.

CMotionVehicle::~CMotionVehicle()
{
}


// Supprime un objet.

void CMotionVehicle::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionVehicle::SetAction(int action, float time)
{
	if ( action == MV_FLATY )
	{
		CrashVehicle(D3DVECTOR(1.0f, 0.25f, 1.0f));  // tout plat
		m_flatTime = 10.0f;
		return ERR_OK;
	}

	if ( action == MV_FLATZ )
	{
		CrashVehicle(D3DVECTOR(1.0f, 1.0f, 0.25f));  // tout mince
		m_flatTime = 10.0f;
		return ERR_OK;
	}

	if ( action == MV_DEFLAT )
	{
		CrashVehicle(D3DVECTOR(1.0f, 1.0f, 1.0f));
		m_flatTime = 0.0f;
		return ERR_OK;
	}

	if ( action == MV_LOADBOT )
	{
		m_lbAction = TRUE;
		m_lbStart = TRUE;
		m_lbLastParticule = 0.0f;
	}

	return CMotion::SetAction(action, time);
}

// Ecrase un véhicule sous la porte 2 ou 3 (mince ou plat).

BOOL CMotionVehicle::CrashVehicle(D3DVECTOR zoom)
{
	CObject*	fret;
	CPyro*		pyro;
	D3DMATRIX*	mat;
	D3DVECTOR	iZoom, pos, speed;
	FPOINT		dim;
	float		duration;
	int			i;
	BOOL		bNormal;

	iZoom = m_object->RetZoom(0);
	if ( iZoom.x == zoom.x &&
		 iZoom.y == zoom.y &&
		 iZoom.z == zoom.z )  return FALSE;

	bNormal = ( zoom.x == 1.0f && zoom.y == 1.0f && zoom.z == 1.0f );

	fret = m_object->RetFret();
	if ( fret != 0 )  // robot transporté ?
	{
		mat = m_object->RetWorldMatrix(0);
		pos = Transform(*mat, m_object->RetCharacter()->posFret);
		m_object->SetFret(0);
		fret->SetTruck(0);
		fret->SetPosition(0, pos);

		pyro = new CPyro(m_iMan);
		pyro->Create(PT_EXPLOT, fret);  // explose le robot transporté
	}

	m_object->SetZoom(0, zoom);

#if 0
	if ( zoom.z != 1.0f )  // mince ?
	{
		zoom = 1.0f/zoom;  // garde les roues normales
	}
	else
	{
		zoom = D3DVECTOR(1.0f, 1.0f, 1.0f);
	}
	m_object->SetZoom(3, zoom);
	m_object->SetZoom(4, zoom);
	m_object->SetZoom(5, zoom);
	m_object->SetZoom(6, zoom);
#endif

	if ( bNormal )
	{
		pos = m_engine->RetEyePt();
		m_sound->Play(SOUND_WAYPOINT, pos, 1.0f, 2.0f);
	}
	else
	{
		pos = m_engine->RetEyePt();
		m_sound->Play(SOUND_BOUMm, pos, 1.0f, 0.6f);
	}

	for ( i=0 ; i<50 ; i++ )
	{
		pos = m_object->RetPosition(0);
		pos.x += (Rand()-0.5f)*10.0f;
		pos.z += (Rand()-0.5f)*10.0f;
		pos.y += Rand()*3.0f;
		speed.x = (Rand()-0.5f)*5.0f;
		speed.z = (Rand()-0.5f)*5.0f;
		speed.y = Rand()*2.0f;
		dim.x = (Rand()*12.0f+12.0f)*(bNormal?0.3f:1.0f);
		dim.y = dim.x;
		duration = 0.4f+Rand()*0.4f;
		m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, duration);
	}
	return TRUE;
}


// Crée un véhicule roulant quelconque posé sur le sol.

BOOL CMotionVehicle::Create(D3DVECTOR pos, float angle, ObjectType type, BOOL bPlumb)
{
	CObject*		bot;
	Character*		character;
	CModFile*		pModFile;
	int				rank, i;
	char			name[100];
	StateVehicle*	sv;

	if ( m_engine->RetRestCreate() < 1+5+18+1 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	m_model = m_object->RetModel();
	m_option = m_object->RetOption();
	m_object->SetType(type);
	CreatePhysics();
	character = m_object->RetCharacter();

	sv = (StateVehicle*)m_main->GetCarState();

	// Crée la base principale.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEVEHICULE);  // c'est un objet mobile
	m_object->SetObjectRank(0, rank);

	if ( type == OBJECT_CAR )
	{
		if ( m_model == 1 )
		{
			pModFile->ReadModel("objects\\car0100.mod");
		}
		if ( m_model == 2 )
		{
			pModFile->ReadModel("objects\\car0200.mod");
		}
		if ( m_model == 3 )
		{
			pModFile->ReadModel("objects\\car0300.mod");
		}
		if ( m_model == 4 )
		{
			pModFile->ReadModel("objects\\car0400.mod");
		}
		if ( m_model == 5 )
		{
			pModFile->ReadModel("objects\\car0500.mod");
		}
		if ( m_model == 6 )
		{
			pModFile->ReadModel("objects\\car0600.mod");
		}
		if ( m_model == 7 )
		{
			pModFile->ReadModel("objects\\car0700.mod");
		}
		if ( m_model == 8 )
		{
			pModFile->ReadModel("objects\\car0800.mod");
		}
		if ( m_model == 99 )
		{
			pModFile->ReadModel("objects\\car9900.mod");
		}
	}
	if ( type == OBJECT_MOBILEtg )
	{
		pModFile->ReadModel("objects\\target.mod");
	}
	if ( type == OBJECT_TRAX )
	{
		pModFile->ReadModel("objects\\trax1.mod");
	}
	if ( type == OBJECT_UFO )
	{
		pModFile->ReadModel("objects\\ufo00.mod");
	}
	pModFile->CreateEngineObject(rank);

	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	// Un véhicule doit avoir obligatoirement une sphère de
	// collision avec un centre (0;0;0) (voir GetCrashSphere).
	if ( type == OBJECT_CAR )
	{
		m_object->CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
		m_object->SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 6.0f);
	}
	else if ( type == OBJECT_TRAX )
	{
		m_object->CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
		// bouton rouge :
		m_object->CreateCrashSphere(D3DVECTOR(-11.0f, 3.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.55f);
		m_object->SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 14.0f);
	}	
	else if ( type == OBJECT_UFO )
	{
		m_object->CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
		m_object->SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 6.0f);
	}	
	else
	{
		m_object->CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.5f, SOUND_BOUMm, 0.45f);
		m_object->SetGlobalSphere(D3DVECTOR(0.0f, 4.0f, 0.0f), 6.0f);
	}	

	if ( type == OBJECT_CAR && m_model == 1 )  // tijuana taxi ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0101.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.0f, 0.0f));

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0102.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-4.0f, 1.3f, 0.0f));

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\car0107.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(3.2f, 1.7f, 0.0f));

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\car0108.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.8f, 1.3f, 0.7f));

		// Crée la télécommande.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0109.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(-0.2f, 1.5f, 0.7f));
		m_object->SetAngleZ(9, -5.0f*PI/180.0f);

		CreatePart(pModFile, tableCar01);

		m_object->CreateShadowCircle(4.0f, 1.0f, D3DSHADOWCAR01);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 2 )  // ford 32 ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0201.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.0f, 0.0f));

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0202.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-5.0f, 1.2f, 0.0f));

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\car0207.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(3.2f, 2.0f, 0.0f));

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\car0208.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(-0.2f, 1.1f, 0.8f));

		// Crée la télécommande.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0209.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(-1.1f, 1.1f, 0.8f));
		
		CreatePart(pModFile, tableCar02);

		m_object->CreateShadowCircle(4.8f, 1.0f, D3DSHADOWCAR02);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 3 )  // pickup ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0301.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.0f, 0.0f)*1.1f);

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0302.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-4.0f, 1.1f, 0.0f)*1.1f);

		// Crée l'amortisseur arrière droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(38, rank);
		m_object->SetObjectParent(38, 2);
		pModFile->ReadModel("objects\\car0338.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(38, D3DVECTOR(0.0f, 0.0f, -1.8f)*1.1f);

		// Crée l'amortisseur arrière gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(39, rank);
		m_object->SetObjectParent(39, 2);
		pModFile->ReadModel("objects\\car0338.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(39, D3DVECTOR(0.0f, 0.0f, 1.8f)*1.1f);

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\car0307.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(2.4f, 2.3f, 0.0f)*1.1f);
		m_object->SetAngleZ(7, 10.0f*PI/180.0f);

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\car0308.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(-0.5f, 1.5f, 0.7f)*1.1f);

		// Crée la télécommande.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0309.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(-1.3f, 1.5f, 0.7f)*1.1f);
		
		CreatePart(pModFile, tableCar03, 1.1f);

		m_object->CreateShadowCircle(4.0f*1.1f, 1.0f, D3DSHADOWCAR03);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 4 )  // firecraker ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0401.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.0f, 0.0f));

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0402.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-5.5f, 1.4f, 0.0f));

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\car0407.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(3.0f, 2.2f, 0.0f));

		// Crée le support du volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(34, rank);
		m_object->SetObjectParent(34, 0);
		pModFile->ReadModel("objects\\car0434.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(34, D3DVECTOR(1.8f, 2.1f, 0.7f));
		m_object->SetAngleZ(34, 35.0f*PI/180.0f);

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 34);
		pModFile->ReadModel("objects\\car0408.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.0f, 1.0f, 0.0f));

		// Crée la télécommande.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0409.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(0.0f, 1.8f, 0.7f));
		
		CreatePart(pModFile, tableCar04);

		m_object->CreateShadowCircle(5.0f, 1.0f, D3DSHADOWCAR04);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 5 )  // hooligan ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0501.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.1f, 0.0f));

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0502.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-5.0f, 1.2f, 0.0f));

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\car0507.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(3.2f, 2.0f, 0.0f));

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\car0508.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(-0.2f, 1.2f, 0.8f));

		// Crée la télécommande.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0509.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(-1.1f, 1.2f, 0.8f));
		
		CreatePart(pModFile, tableCar05);

		m_object->CreateShadowCircle(5.0f, 1.0f, D3DSHADOWCAR05);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 6 )  // chevy ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0601.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.0f, 0.0f));

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0602.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-4.0f, 1.1f, 0.0f));

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		if ( m_engine->RetSuperDetail() )
		{
			pModFile->ReadModel("objects\\car0607h.mod");
		}
		else
		{
			pModFile->ReadModel("objects\\car0607.mod");
		}
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(4.2f, 2.3f, 0.0f));

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0609.mod");  // objet inexistant
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(2.1f, 2.9f, 1.0f));
		m_object->SetAngleZ(9, 45.0f*PI/180.0f);

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 9);
		pModFile->ReadModel("objects\\car0608.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.0f, 0.0f, 0.0f));

		CreatePart(pModFile, tableCar06);

		m_object->CreateShadowCircle(4.5f, 1.0f, D3DSHADOWCAR06);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 7 )  // reo ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0701.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.0f, 0.0f)*1.1f);

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0702.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-4.0f, 1.1f, 0.0f)*1.1f);

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\car0707.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(3.2f, 2.2f, 0.0f)*1.1f);

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\car0708.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.3f, 1.5f, 0.8f)*1.1f);

		// Crée la télécommande.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0709.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(-0.6f, 1.5f, 0.8f)*1.1f);

		CreatePart(pModFile, tableCar07, 1.1f);

		m_object->CreateShadowCircle(4.5f*1.1f, 1.0f, D3DSHADOWCAR07);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 8 )  // torpedo ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car0801.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(6.3f, 1.0f, 0.0f)*1.1f);

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car0802.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-4.0f, 1.1f, 0.0f)*1.1f);

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		// Crée le moteur.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\car0807.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(1.5f, 2.2f, 0.0f)*1.1f);

		// Crée le volant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\car0808.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(-2.0f, 4.0f, 0.0f)*1.1f);

		// Crée la télécommande.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\car0809.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(-6.2f, 1.5f, 0.0f)*1.1f);

		CreatePart(pModFile, tableCar08, 1.1f);

		m_object->CreateShadowCircle(5.0f*1.1f, 1.0f, D3DSHADOWCAR08);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_model == 99 )  // fantome ?
	{
		// Crée l'axe avant.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\car9901.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(5.0f, 1.0f, 0.0f)*1.1f);

		// Crée l'axe arrière.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\car9902.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-4.0f, 1.1f, 0.0f)*1.1f);

		// Crée les roues.
		CreateWheel(pModFile, m_nameObjWheel);

		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_CAR && m_option == 1 )  // lance incendie ?
	{
		bot = new CObject(m_iMan);
		if ( !bot->CreateAdditionnal(pos, 0.0f, 1.0f, OBJECT_FIRE, TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			bot->SetPosition(0, character->posFret);
			bot->SetAngle(0, character->angleFret);
		}
	}

	if ( type == OBJECT_CAR && m_option == 2 )  // helicoptère ?
	{
		bot = new CObject(m_iMan);
		if ( !bot->CreateAdditionnal(pos, 0.0f, 1.0f, OBJECT_HELICO, TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			bot->SetPosition(0, character->posFret);
			bot->SetAngle(0, character->angleFret);
		}
	}

	if ( type == OBJECT_CAR && m_option == 3 )  // boussole ?
	{
		bot = new CObject(m_iMan);
		if ( !bot->CreateAdditionnal(pos, 0.0f, 1.0f, OBJECT_COMPASS, TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			bot->SetPosition(0, character->posFret);
			bot->SetAngle(0, character->angleFret);
		}
	}

	if ( type == OBJECT_CAR && m_option == 4 )  // électrocuteur ?
	{
		bot = new CObject(m_iMan);
		if ( !bot->CreateAdditionnal(pos, 0.0f, 1.0f, OBJECT_BLITZER, TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			bot->SetPosition(0, character->posFret);
			bot->SetAngle(0, character->angleFret);
		}
	}

	if ( type == OBJECT_CAR && m_option == 5 )  // crochet ?
	{
		bot = new CObject(m_iMan);
		if ( !bot->CreateAdditionnal(pos, 0.0f, 1.0f, OBJECT_HOOK, TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			bot->SetPosition(0, character->posFret);
			bot->SetAngle(0, character->angleFret);
		}
	}

	if ( type == OBJECT_CAR && m_option >= 11 && m_option <= 20 )
	{
		bot = new CObject(m_iMan);
		if ( !bot->CreateBot(pos, 0.0f, 1.0f, (ObjectType)(OBJECT_BOT1+m_option-11), TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			pos = character->posFret;
			pos.y += bot->RetCharacter()->height;
			bot->SetPosition(0, pos);
			bot->SetAngle(0, character->angleFret);
		}
	}

	if ( type == OBJECT_MOBILEtg )
	{
		// Crée la roue arrière-droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(6, rank);
		m_object->SetObjectParent(6, 0);
		pModFile->ReadModel("objects\\lem2w.mod");
		pModFile->CreateEngineObject(rank);

		// Crée la roue arrière-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\lem2w.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetAngleY(7, PI);

		// Crée la roue avant-droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\lem2w.mod");
		pModFile->CreateEngineObject(rank);

		// Crée la roue avant-gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\lem2w.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetAngleY(9, PI);

		m_object->CreateShadowCircle(4.0f, 1.0f);
		m_object->SetFloorHeight(0.0f);
	}

	if ( type == OBJECT_TRAX )
	{
		// Crée la chenille droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\trax2.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(-1.0f, 3.0f, -4.0f));

		// Crée la chenille gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\trax3.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(-1.0f, 3.0f, 4.0f));

		// Crée la pelle.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(3, rank);
		m_object->SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\trax4.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(3, D3DVECTOR(-1.0f, 4.0f, 0.0f));

		// Crée le levier droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(4, rank);
		m_object->SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\trax5.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(4, D3DVECTOR(-3.5f, 8.0f, -3.0f));

		// Crée le levier gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(5, rank);
		m_object->SetObjectParent(5, 0);
		pModFile->ReadModel("objects\\trax5.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(5, D3DVECTOR(-3.5f, 8.0f, 3.0f));

		m_object->CreateShadowCircle(10.0f, 1.0f);
		m_object->SetFloorHeight(0.0f);

		bot = new CObject(m_iMan);
		if ( !bot->CreateBot(pos, 0.0f, 1.0f, OBJECT_EVIL2, TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			bot->SetPosition(0, D3DVECTOR(-6.0f, 9.0f, 0.0f));
		}
	}

	if ( type == OBJECT_UFO )
	{
		for ( i=1 ; i<=18 ; i++ )
		{
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			m_object->SetObjectRank(i, rank);
			m_object->SetObjectParent(i, 0);
			sprintf(name, "objects\\ufo%.2d.mod", i);
			pModFile->ReadModel(name);
			pModFile->CreateEngineObject(rank);
			m_object->SetPosition(i, D3DVECTOR(0.0f, 0.0f, 0.0f));
		}

		bot = new CObject(m_iMan);
		if ( !bot->CreateBot(pos, 0.0f, 1.0f, OBJECT_EVIL2, TRUE) )
		{
			delete bot;
		}
		else
		{
			bot->SetTruck(m_object);
			m_object->SetFret(bot);
			bot->SetPosition(0, character->posFret);
			bot->SetAngle(0, character->angleFret);
		}

		m_object->CreateShadowCircle(4.0f, 1.0f);
	}

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

	if ( sv == 0 )
	{
		TwistInit();
	}
	else
	{
		TwistInit(sv);
	}

	m_engine->LoadAllTexture();

	delete pModFile;
	return TRUE;
}

// Crée les roues.

void CMotionVehicle::CreateWheel(CModFile* pModFile, char *name)
{
	Character*	character;
	D3DVECTOR	zoom;
	int			rank;

	character = m_object->RetCharacter();

	zoom.x = character->wheelFrontDim;
	zoom.y = character->wheelFrontDim;
	zoom.z = character->wheelFrontWidth;

	// Crée la roue avant-droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(3, rank);
	m_object->SetObjectParent(3, 1);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(3, D3DVECTOR(0.0f, 0.0f, -character->wheelFrontPos.z));
	m_object->SetZoom(3, zoom);

	// Crée la roue avant-gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(4, rank);
	m_object->SetObjectParent(4, 1);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(4, D3DVECTOR(0.0f, 0.0f, character->wheelFrontPos.z));
	m_object->SetZoom(4, zoom);
	m_object->SetAngleY(4, PI);

	zoom.x = character->wheelBackDim;
	zoom.y = character->wheelBackDim;
	zoom.z = character->wheelBackWidth;

	// Crée la roue arrière-droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(5, rank);
	m_object->SetObjectParent(5, 2);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(5, D3DVECTOR(0.0f, 0.0f, -character->wheelBackPos.z));
	m_object->SetZoom(5, zoom);

	// Crée la roue arrière-gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(6, rank);
	m_object->SetObjectParent(6, 2);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(6, D3DVECTOR(0.0f, 0.0f, character->wheelBackPos.z));
	m_object->SetZoom(6, zoom);
	m_object->SetAngleY(6, PI);
}

// Crée toutes les parties mobiles du véhicule.

void CMotionVehicle::CreatePart(CModFile* pModFile, RemovePart* table, float zoom)
{
	StateVehicle*	sv;
	D3DVECTOR		pos;
	int				i, j, rank;
	char			name[50];

	sv = (StateVehicle*)m_main->GetCarState();

	m_partTotal = 0;
	for ( i=0 ; i<MAX_PART ; i++ )
	{
		if ( table[i].part == -1 )  break;
		if ( sv != 0 && sv->state[i].bDeleted )  continue;

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(table[i].part, rank);
		m_object->SetObjectParent(table[i].part, 0);

		if ( m_engine->RetSuperDetail() )
		{
			sprintf(name, "objects\\car%.2d%.2dh.mod", m_model, table[i].file);
			if ( !pModFile->ReadModel(name) )
			{
				sprintf(name, "objects\\car%.2d%.2d.mod", m_model, table[i].file);
				pModFile->ReadModel(name);
			}
		}
		else
		{
			sprintf(name, "objects\\car%.2d%.2d.mod", m_model, table[i].file);
			pModFile->ReadModel(name);
		}
		pModFile->CreateEngineObject(rank);
		pos.x = table[i].x*zoom;
		pos.y = table[i].y*zoom;
		pos.z = table[i].z*zoom;
		m_object->SetPosition(table[i].part, pos);

		if ( m_model == 2 &&  // ford 32 ?
			 m_option != 0 &&
			 table[i].part == 24 )  // coffre + transport ?
		{
			OpenClosePart(1.0f);
		}

		if ( m_model == 6 &&  // chevy ?
			 m_option != 0 &&
			 table[i].part == 10 )  // toît + transport ?
		{
			OpenClosePart(1.0f);
		}

		m_part[i].bDeleted = FALSE;
		m_part[i].part     = table[i].part;
		m_part[i].order    = table[i].order;
		m_part[i].top      = table[i].top;
		for ( j=0 ; j<4 ; j++ )
		{
			m_part[i].light[j] = table[i].light[j];
		}
		m_topRest += m_part[i].top;
		m_partTotal ++;
	}

	m_partUsed = m_partTotal;
}

// Crée la physique de l'objet.

void CMotionVehicle::CreatePhysics()
{
	Character*	character;
	CObject*	helico;
	ObjectType	type;
	float		factor, radius, water, susp, mass, kid;

	character = m_object->RetCharacter();
	type = m_object->RetType();

	if ( type == OBJECT_CAR )
	{
		if ( m_wheelType == WT_BURN )
		{
			factor = 0.5f;
			radius = 0.7f;
		}
		else
		{
			factor = 1.0f;
			radius = 1.0f;
		}

		if ( m_bWater == 1 )
		{
			water = 0.5f;
		}
		else
		{
			water = 1.0f;
		}

		if ( m_main->RetFixScene() )
		{
			susp = 0.5f;
		}
		else
		{
			susp = 1.0f;
		}

		mass = 1.0f;
		helico = m_object->RetFret();
		if ( helico != 0 && helico->RetType() == OBJECT_HELICO )
		{
			mass = 0.2f;
		}

		kid = 1.0f;
		if ( m_main->RetLevel() == 1 )  // niveau CN ?
		{
			kid = 0.7f;
			mass *= 0.5f;
		}
	}

	if ( type == OBJECT_CAR && m_model == 1 )  // tijuana taxi ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.0f, 3.0f);
		character->wheelBackPos   = D3DVECTOR(-4.0f, 1.3f, 2.7f);
		character->wheelFrontDim  = 1.0f*radius;
		character->wheelBackDim   = 1.3f*radius;
		character->wheelFrontWidth= 1.0f;
		character->wheelBackWidth = 1.6f;
		character->crashFront     = 6.0f;
		character->crashBack      =-6.0f;
		character->crashWidth     = 4.2f;
		character->suspDetect     = 2.0f;
		character->suspHeight     = 1.0f*susp;
		character->suspFrequency  = 8.0f;
		character->suspAbsorber   = 1.0f;
		character->rolling        = 0.8f*factor;
		character->nicking        = 0.15f*factor;
		character->maxRolling     = 20.0f*PI/180.0f;
		character->maxNicking     = 15.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 3.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 70.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-2.0f, 1.6f, 0.0f);
		character->angleFret      = D3DVECTOR(0.0f, 0.0f, -5.0f*PI/180.0f);
		character->antenna        = D3DVECTOR(0.35f, 5.1f, 1.15f);
		character->lightFL        = D3DVECTOR(5.2f, 2.25f,  1.9f);
		character->lightFR        = D3DVECTOR(5.2f, 2.25f, -1.9f);
		character->lightSL        = D3DVECTOR(-4.6f, 3.9f,  2.0f);
		character->lightSR        = D3DVECTOR(-4.6f, 3.9f, -2.0f);
		character->lightRL        = D3DVECTOR(-4.6f, 3.3f,  2.0f);
		character->lightRR        = D3DVECTOR(-4.6f, 3.3f, -2.0f);
		character->camera         = D3DVECTOR( 1.9f, 4.1f, 0.0f);
		character->hookDist       = -6.0f;
		character->motorSound     = 1;

		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 15.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0103.mod");
		strcpy(m_nameObjWBurn, "objects\\car0103b.mod");
		strcpy(m_nameTexWheel, "wheel01.tga");
	}

	if ( type == OBJECT_CAR && m_model == 2 )  // ford 32 ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.0f, 2.7f);
		character->wheelBackPos   = D3DVECTOR(-5.0f, 1.2f, 2.5f);
		character->wheelFrontDim  = 1.0f*radius;
		character->wheelBackDim   = 1.2f*radius;
		character->wheelFrontWidth= 1.0f;
		character->wheelBackWidth = 1.4f;
		character->crashFront     = 6.0f;
		character->crashBack      =-6.5f;
		character->crashWidth     = 4.0f;
		character->suspDetect     = 2.0f;
//?		character->suspHeight     = 0.5f*susp;
		character->suspHeight     = 1.0f*susp;
		character->suspFrequency  = 8.0f;
		character->suspAbsorber   = 1.0f;
		character->rolling        = 0.8f*factor;
		character->nicking        = 0.10f*factor;
		character->maxRolling     = 20.0f*PI/180.0f;
		character->maxNicking     = 15.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 3.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 80.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-5.2f, 2.2f, 0.0f);
		character->angleFret      = D3DVECTOR(0.0f, 0.0f, -5.0f*PI/180.0f);
		character->antenna        = D3DVECTOR(-0.75f, 4.55f, 1.34f);
		character->lightFL        = D3DVECTOR(6.2f, 2.8f,  1.4f);
		character->lightFR        = D3DVECTOR(6.2f, 2.8f, -1.4f);
		character->lightSL        = D3DVECTOR(-6.4f, 3.2f,  2.5f);
		character->lightSR        = D3DVECTOR(-6.4f, 3.2f, -2.5f);
		character->lightRL        = D3DVECTOR(-6.4f, 2.6f,  2.5f);
		character->lightRR        = D3DVECTOR(-6.4f, 2.6f, -2.5f);
		character->camera         = D3DVECTOR( 0.9f, 4.9f, 0.0f);
		character->hookDist       = -5.5f;
		character->motorSound     = 5;

		m_physics->SetLinMotionX(MO_ADVSPEED,150.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 15.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0203.mod");
		strcpy(m_nameObjWBurn, "objects\\car0203b.mod");
		strcpy(m_nameTexWheel, "wheel02.tga");
	}

	if ( type == OBJECT_CAR && m_model == 3 )  // pickup ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.0f, 2.5f)*1.1f;
		character->wheelBackPos   = D3DVECTOR(-4.0f, 1.1f, 2.5f)*1.1f;
		character->wheelFrontDim  = 1.0f*1.1f*radius;
		character->wheelBackDim   = 1.1f*1.1f*radius;
		character->wheelFrontWidth= 1.0f*1.1f;
		character->wheelBackWidth = 1.2f*1.1f;
		character->crashFront     = 6.0f*1.1f;
		character->crashBack      =-6.0f*1.1f;
		character->crashWidth     = 3.5f*1.1f;
		character->suspDetect     = 2.0f;
//?		character->suspHeight     = 0.7f*susp;
		character->suspHeight     = 1.0f*susp;
		character->suspFrequency  = 8.0f;
		character->suspAbsorber   = 1.0f;
		character->rolling        = 0.8f*factor;
		character->nicking        = 0.15f*factor;
		character->maxRolling     = 20.0f*PI/180.0f;
		character->maxNicking     = 15.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 3.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 80.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-4.5f, 2.6f, 0.0f)*1.1f;
		character->angleFret      = D3DVECTOR(0.0f, 0.0f,  0.0f);
		character->antenna        = D3DVECTOR(-0.99f, 4.51f, 1.15f)*1.1f;
		character->lightFL        = D3DVECTOR(5.2f, 2.2f,  1.4f)*1.1f;
		character->lightFR        = D3DVECTOR(5.2f, 2.2f, -1.4f)*1.1f;
		character->lightSL        = D3DVECTOR(-5.7f, 2.5f,  2.7f)*1.1f;
		character->lightSR        = D3DVECTOR(-5.7f, 2.5f, -2.7f)*1.1f;
		character->lightRL        = D3DVECTOR(-5.7f, 2.5f,  2.3f)*1.1f;
		character->lightRR        = D3DVECTOR(-5.7f, 2.5f, -2.3f)*1.1f;
		character->camera         = D3DVECTOR( 1.0f, 5.1f, 0.0f);
		character->hookDist       = -4.5f;
		character->motorSound     = 1;

		m_physics->SetLinMotionX(MO_ADVSPEED,130.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 15.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0303.mod");
		strcpy(m_nameObjWBurn, "objects\\car0303b.mod");
		strcpy(m_nameTexWheel, "wheel03.tga");
	}

	if ( type == OBJECT_CAR && m_model == 4 )  // firecraker ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.0f, 2.7f);
		character->wheelBackPos   = D3DVECTOR(-5.5f, 1.4f, 2.7f);
		character->wheelFrontDim  = 1.0f*radius;
		character->wheelBackDim   = 1.4f*radius;
		character->wheelFrontWidth= 1.3f;
		character->wheelBackWidth = 1.7f;
		character->crashFront     = 6.5f;
		character->crashBack      =-7.5f;
		character->crashWidth     = 4.0f;
		character->suspDetect     = 1.5f;
//?		character->suspHeight     = 0.3f*susp;
		character->suspHeight     = 0.7f*susp;
		character->suspFrequency  = 12.0f;
		character->suspAbsorber   = 2.0f;
		character->rolling        = 0.5f*factor;
		character->nicking        = 0.10f*factor;
		character->maxRolling     = 15.0f*PI/180.0f;
		character->maxNicking     = 10.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 2.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 60.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-4.5f, 3.9f, 0.0f);
		character->angleFret      = D3DVECTOR(0.0f, 0.0f, -10.0f*PI/180.0f);
		character->antenna        = D3DVECTOR(0.6f, 4.9f, 1.15f);
		character->lightFL        = D3DVECTOR(5.6f, 2.1f,  1.4f);
		character->lightFR        = D3DVECTOR(5.6f, 2.1f, -1.4f);
		character->lightSL        = D3DVECTOR(-7.2f, 3.0f,  3.0f);
		character->lightSR        = D3DVECTOR(-7.2f, 3.0f, -3.0f);
		character->lightRL        = D3DVECTOR(-7.2f, 3.0f,  2.3f);
		character->lightRR        = D3DVECTOR(-7.2f, 3.0f, -2.3f);
		character->camera         = D3DVECTOR( 2.1f, 4.6f, 0.0f);
		character->hookDist       = -5.0f;
		character->motorSound     = 3;

		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL, 30.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 15.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0403.mod");
		strcpy(m_nameObjWBurn, "objects\\car0403b.mod");
		strcpy(m_nameTexWheel, "wheel04.tga");
	}

	if ( type == OBJECT_CAR && m_model == 5 )  // hooligan ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.1f, 2.7f);
		character->wheelBackPos   = D3DVECTOR(-5.0f, 1.2f, 2.5f);
		character->wheelFrontDim  = 1.1f*radius;
		character->wheelBackDim   = 1.2f*radius;
		character->wheelFrontWidth= 1.3f;
		character->wheelBackWidth = 1.5f;
		character->crashFront     = 6.0f;
		character->crashBack      =-6.5f;
		character->crashWidth     = 4.1f;
		character->suspDetect     = 2.0f;
//?		character->suspHeight     = 0.5f*susp;
		character->suspHeight     = 0.7f*susp;
		character->suspFrequency  = 8.0f;
		character->suspAbsorber   = 1.0f;
		character->rolling        = 0.8f*factor;
		character->nicking        = 0.15f*factor;
		character->maxRolling     = 20.0f*PI/180.0f;
		character->maxNicking     = 15.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 3.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 70.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-4.1f, 2.4f, 0.0f);
		character->angleFret      = D3DVECTOR(0.0f, 0.0f,  0.0f);
		character->antenna        = D3DVECTOR(-0.81f, 4.60f, 1.32f);
		character->lightFL        = D3DVECTOR(5.2f, 3.4f,  1.6f);
		character->lightFR        = D3DVECTOR(5.2f, 3.4f, -1.6f);
		character->lightSL        = D3DVECTOR(-6.0f, 3.0f,  3.2f);
		character->lightSR        = D3DVECTOR(-6.0f, 3.0f, -3.2f);
		character->lightRL        = D3DVECTOR(-6.0f, 3.0f,  2.6f);
		character->lightRR        = D3DVECTOR(-6.0f, 3.0f, -2.6f);
		character->camera         = D3DVECTOR( 0.7f, 5.0f, 0.0f);
		character->hookDist       = -4.0f;
		character->motorSound     = 2;

		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 15.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0503.mod");
		strcpy(m_nameObjWBurn, "objects\\car0503b.mod");
		strcpy(m_nameTexWheel, "wheel05.tga");
	}

	if ( type == OBJECT_CAR && m_model == 6 )  // chevy ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.0f, 2.5f);
		character->wheelBackPos   = D3DVECTOR(-4.0f, 1.1f, 2.5f);
		character->wheelFrontDim  = 1.0f*radius;
		character->wheelBackDim   = 1.1f*radius;
		character->wheelFrontWidth= 1.0f;
		character->wheelBackWidth = 1.2f;
		character->crashFront     = 7.0f;
		character->crashBack      =-7.5f;
		character->crashWidth     = 3.5f;
		character->suspDetect     = 2.0f;
		character->suspHeight     = 0.6f*susp;
		character->suspFrequency  = 8.0f;
		character->suspAbsorber   = 1.0f;
		character->rolling        = 0.8f*factor;
		character->nicking        = 0.15f*factor;
		character->maxRolling     = 20.0f*PI/180.0f;
		character->maxNicking     = 15.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 3.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 80.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-0.5f, 1.5f,  0.0f);
		character->angleFret      = D3DVECTOR( 0.0f, 0.0f,  0.0f);
		character->antenna        = D3DVECTOR(-5.0f, 4.9f,  2.1f);
		character->lightFL        = D3DVECTOR( 7.1f, 2.8f,  2.4f);
		character->lightFR        = D3DVECTOR( 7.1f, 2.8f, -2.4f);
		character->lightSL        = D3DVECTOR(-7.8f, 3.0f,  2.7f);
		character->lightSR        = D3DVECTOR(-7.8f, 3.0f, -2.7f);
		character->lightRL        = D3DVECTOR(-7.9f, 3.3f,  2.7f);
		character->lightRR        = D3DVECTOR(-7.9f, 3.3f, -2.7f);
		character->camera         = D3DVECTOR( 2.1f, 4.9f,  0.0f);
		character->hookDist       = -9.0f;
		character->motorSound     = 6;

		m_physics->SetLinMotionX(MO_ADVSPEED,160.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL, 80.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 20.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0603.mod");
		strcpy(m_nameObjWBurn, "objects\\car0603b.mod");
		strcpy(m_nameTexWheel, "wheel06.tga");
	}

	if ( type == OBJECT_CAR && m_model == 7 )  // reo ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.0f, 2.5f)*1.1f;
		character->wheelBackPos   = D3DVECTOR(-4.0f, 1.1f, 2.5f)*1.1f;
		character->wheelFrontDim  = 1.0f*1.1f*radius;
		character->wheelBackDim   = 1.1f*1.1f*radius;
		character->wheelFrontWidth= 1.0f*1.1f;
		character->wheelBackWidth = 1.2f*1.1f;
		character->crashFront     = 6.6f*1.1f;
		character->crashBack      =-6.4f*1.1f;
		character->crashWidth     = 3.5f*1.1f;
		character->suspDetect     = 1.5f;
		character->suspHeight     = 0.7f*susp;
		character->suspFrequency  = 8.0f;
		character->suspAbsorber   = 1.0f;
		character->rolling        = 0.5f*factor;
		character->nicking        = 0.10f*factor;
		character->maxRolling     = 15.0f*PI/180.0f;
		character->maxNicking     = 10.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 3.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 80.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-2.5f, 1.5f, 0.0f)*1.1f;
		character->angleFret      = D3DVECTOR(0.0f, 0.0f,  0.0f);
		character->antenna        = D3DVECTOR(-0.27f, 4.51f, 1.21f)*1.1f;
		character->lightFL        = D3DVECTOR(5.4f, 2.8f,  1.4f)*1.1f;
		character->lightFR        = D3DVECTOR(5.4f, 2.8f, -1.4f)*1.1f;
		character->lightSL        = D3DVECTOR(-5.5f, 2.3f,  2.5f)*1.1f;
		character->lightSR        = D3DVECTOR(-5.5f, 2.3f, -2.5f)*1.1f;
		character->lightRL        = D3DVECTOR(-5.8f, 1.8f,  2.5f)*1.1f;
		character->lightRR        = D3DVECTOR(-5.8f, 1.8f, -2.5f)*1.1f;
		character->camera         = D3DVECTOR( 1.0f, 5.3f, 0.0f);
		character->hookDist       = -6.5f;
		character->motorSound     = 4;

		m_physics->SetLinMotionX(MO_ADVSPEED,140.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 15.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0703.mod");
		strcpy(m_nameObjWBurn, "objects\\car0703b.mod");
		strcpy(m_nameTexWheel, "wheel07.tga");
	}

	if ( type == OBJECT_CAR && m_model == 8 )  // torpedo ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 6.3f, 1.0f, 2.5f)*1.1f;
		character->wheelBackPos   = D3DVECTOR(-4.0f, 1.1f, 2.5f)*1.1f;
		character->wheelFrontDim  = 1.0f*1.1f*radius;
		character->wheelBackDim   = 1.1f*1.1f*radius;
		character->wheelFrontWidth= 1.0f*1.1f;
		character->wheelBackWidth = 1.2f*1.1f;
		character->crashFront     = 7.0f*1.1f;
		character->crashBack      =-8.0f*1.1f;
		character->crashWidth     = 3.5f*1.1f;
		character->suspDetect     = 1.2f;
		character->suspHeight     = 0.5f*susp;
		character->suspFrequency  = 12.0f;
		character->suspAbsorber   = 2.0f;
		character->rolling        = 0.5f*factor;
		character->nicking        = 0.10f*factor;
		character->maxRolling     = 15.0f*PI/180.0f;
		character->maxNicking     = 10.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 2.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f*mass;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 50.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-4.2f, 1.5f, 0.0f)*1.1f;
		character->angleFret      = D3DVECTOR(0.0f, 0.0f, 0.0f);
		character->antenna        = D3DVECTOR(-6.51f, 5.21f, 0.53f)*1.1f;
		character->lightFL        = D3DVECTOR(0.0f, 2.0f, 0.0f)*1.1f;
		character->lightFR        = D3DVECTOR(0.0f, 2.0f, 0.0f)*1.1f;
		character->lightSL        = D3DVECTOR(0.0f, 2.0f, 0.0f)*1.1f;
		character->lightSR        = D3DVECTOR(0.0f, 2.0f, 0.0f)*1.1f;
		character->lightRL        = D3DVECTOR(0.0f, 2.0f, 0.0f)*1.1f;
		character->lightRR        = D3DVECTOR(0.0f, 2.0f, 0.0f)*1.1f;
		character->camera         = D3DVECTOR(-4.0f, 5.0f, 0.0f)*1.1f;
		character->hookDist       = -5.0f;
		character->motorSound     = 6;

		m_physics->SetLinMotionX(MO_ADVSPEED,170.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_ADVACCEL,100.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_STOACCEL, 30.0f*factor*water*kid);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car0803.mod");
		strcpy(m_nameObjWBurn, "objects\\car0803b.mod");
		strcpy(m_nameTexWheel, "wheel08.tga");
	}

	if ( type == OBJECT_CAR && m_model == 99 )  // fantome ?
	{
		m_physics->SetType(TYPE_RACE);

		character->wheelFrontPos  = D3DVECTOR( 5.0f, 1.0f, 2.5f)*1.1f;
		character->wheelBackPos   = D3DVECTOR(-4.0f, 1.1f, 2.5f)*1.1f;
		character->wheelFrontDim  = 1.0f*1.1f*radius;
		character->wheelBackDim   = 1.1f*1.1f*radius;
		character->wheelFrontWidth= 1.0f*1.1f;
		character->wheelBackWidth = 1.2f*1.1f;
		character->crashFront     = 6.6f*1.1f;
		character->crashBack      =-6.4f*1.1f;
		character->crashWidth     = 3.5f*1.1f;
		character->suspDetect     = 1.5f;
		character->suspHeight     = 0.7f*susp;
		character->suspFrequency  = 8.0f;
		character->suspAbsorber   = 1.0f;
		character->rolling        = 0.5f*factor;
		character->nicking        = 0.10f*factor;
		character->maxRolling     = 15.0f*PI/180.0f;
		character->maxNicking     = 10.0f*PI/180.0f;
		character->overProp       = 5.0f;
		character->overFactor     = 1.0f;
		character->overAngle      = 30.0f*PI/180.0f;
		character->overMul        = 3.0f;
		character->brakeDir       = 0.1f;
		character->mass           = 1500.0f;
		character->turnSlide      = 2.0f;
		character->gripLimit      = 0.3f;
		character->gripSlide      = 80.0f*factor*water;
		character->accelLow       = 1.5f;
		character->accelHigh      = 0.2f;
		character->accelSmooth    = 1.5f;
		character->posFret        = D3DVECTOR(-2.5f, 1.5f, 0.0f)*1.1f;
		character->angleFret      = D3DVECTOR(0.0f, 0.0f,  0.0f);
		character->antenna        = D3DVECTOR(-0.27f, 4.51f, 1.21f)*1.1f;
		character->lightFL        = D3DVECTOR(5.4f, 2.8f,  1.4f)*1.1f;
		character->lightFR        = D3DVECTOR(5.4f, 2.8f, -1.4f)*1.1f;
		character->lightSL        = D3DVECTOR(-5.5f, 2.3f,  2.5f)*1.1f;
		character->lightSR        = D3DVECTOR(-5.5f, 2.3f, -2.5f)*1.1f;
		character->lightRL        = D3DVECTOR(-5.8f, 1.8f,  2.5f)*1.1f;
		character->lightRR        = D3DVECTOR(-5.8f, 1.8f, -2.5f)*1.1f;
		character->camera         = D3DVECTOR( 1.0f, 5.3f, 0.0f);
		character->hookDist       = -8.0f;
		character->motorSound     = 4;

		m_physics->SetLinMotionX(MO_ADVSPEED,140.0f*factor*water);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f*factor*water);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f*factor*water);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f*factor*water);
		m_physics->SetLinMotionX(MO_STOACCEL, 15.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f*factor*water);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 50.0f);

		m_physics->SetLinMotionY(MO_STOACCEL, 10.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

		strcpy(m_nameObjWheel, "objects\\car9903.mod");
		strcpy(m_nameObjWBurn, "objects\\car9903.mod");
		strcpy(m_nameTexWheel, "");
	}

	if ( type == OBJECT_CAR )
	{
		if ( m_wheelType == WT_BURN )
		{
			character->height = -0.3f;
		}
		else
		{
			character->height = 0.0f;
		}
	}

	if ( type == OBJECT_MOBILEtg )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 4.0f, 1.0f, 4.0f);
		character->wheelBackPos  = D3DVECTOR(-3.0f, 1.0f, 4.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->suspDetect    = 2.0f;
		character->mass          = 1000.0f;

		m_physics->SetLinMotionX(MO_ADVSPEED, 20.0f);
		m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
		m_physics->SetLinMotionX(MO_ADVACCEL, 40.0f);
		m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
		m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 20.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  0.8f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  0.8f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL, 10.0f);
		m_physics->SetCirMotionY(MO_RECACCEL, 10.0f);
		m_physics->SetCirMotionY(MO_STOACCEL, 15.0f);
	}

	if ( type == OBJECT_TRAX )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 4.0f, 1.0f, 4.0f);
		character->wheelBackPos  = D3DVECTOR(-3.0f, 1.0f, 4.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->crashFront    = 15.0f;
		character->crashBack     =-10.5f;
		character->crashWidth    = 11.0f;
		character->suspDetect    = 2.0f;
		character->mass          = 3000.0f;
		character->motorSound    = 3;

		m_physics->SetLinMotionX(MO_ADVSPEED, 20.0f);
		m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
		m_physics->SetLinMotionX(MO_ADVACCEL, 40.0f);
		m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
		m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionX(MO_TERFORCE, 10.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  0.2f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  0.2f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  5.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  5.0f);
		m_physics->SetCirMotionY(MO_STOACCEL,  8.0f);
	}

	if ( type == OBJECT_UFO )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 6.0f, 3.0f, 7.0f);
		character->wheelBackPos  = D3DVECTOR(-8.0f, 3.0f, 7.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->suspDetect    = 2.0f;
		character->height        = 2.0f;
		character->mass          = 3000.0f;

#if _SE
		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f);  // comme tijuana taxi
#else
		m_physics->SetLinMotionX(MO_ADVSPEED,150.0f);
#endif
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_STOACCEL, 30.0f);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
	}

	if ( character->crashWidth != 0.0f )
	{
		m_object->CreateCrashLine(FPOINT(character->crashBack,  -character->crashWidth), SOUND_BOUMm, 0.45f);
		m_object->CreateCrashLine(FPOINT(character->crashFront, -character->crashWidth), SOUND_BOUMm, 0.45f);
		m_object->CreateCrashLine(FPOINT(character->crashFront,  character->crashWidth), SOUND_BOUMm, 0.45f);
		m_object->CreateCrashLine(FPOINT(character->crashBack,   character->crashWidth), SOUND_BOUMm, 0.45f);
		m_object->CreateCrashLine(FPOINT(character->crashBack,  -character->crashWidth), SOUND_BOUMm, 0.45f);
	}
}

// Indique si le véhicule est sous l'eau.

BOOL CMotionVehicle::IsUnderWater()
{
	D3DVECTOR	pos;

	pos = m_object->RetPosition(0);
	return ( pos.y+2.0f < m_water->RetLevel() );
}


// Gestion d'un événement.

BOOL CMotionVehicle::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	if ( event.event == EVENT_KEYDOWN )
	{
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionVehicle::EventFrame(const Event &event)
{
	D3DMATRIX*	mat;
	Character*	character;
	CObject*	helico;
	D3DVECTOR	pos, angle, floor;
	FPOINT		rot;
	ObjectType	type;
	float		s, a, d, hl, hr, aLeft, aRight, progress, max, limit[2];
	float		a1, a2;
	BOOL		bWater, bHelico;

	if ( m_engine->RetPause() )  return TRUE;
	if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return TRUE;

	if ( m_lbAction )
	{
		ActionLoadBot(event.rTime);
	}

	bWater = IsUnderWater();
	if ( bWater != m_bWater )
	{
		m_bWater = bWater;
		CreatePhysics();  // adapte la physique
	}

	bHelico = FALSE;
	helico = m_object->RetFret();
	if ( helico != 0 && helico->RetType() == OBJECT_HELICO )
	{
		bHelico = TRUE;
	}
	if ( bHelico != m_bHelico )
	{
		m_bHelico = bHelico;
		CreatePhysics();  // adapte la physique
	}

	type = m_object->RetType();
	character = m_object->RetCharacter();
	mat = m_object->RetWorldMatrix(0);

	if ( m_flatTime > 0.0f )
	{
		m_flatTime -= event.rTime;
		if ( m_flatTime <= 0.0f )
		{
			m_flatTime = 0.0f;
			SetAction(MV_DEFLAT);  // redevient normal
		}
	}

	if ( type == OBJECT_CAR )
	{
		// Tourne les roues.
		s = m_physics->RetWheelSpeed(TRUE);  // roues avants
		UpdateWheelMapping(s, TRUE);
		m_object->SetAngleZ(3, m_object->RetAngleZ(3)+event.rTime*-s);
		m_object->SetAngleZ(4, m_object->RetAngleZ(4)+event.rTime* s);
		s = m_physics->RetWheelSpeed(FALSE);  // roues arrières
		UpdateWheelMapping(s, FALSE);
		m_object->SetAngleZ(5, m_object->RetAngleZ(5)+event.rTime*-s);
		m_object->SetAngleZ(6, m_object->RetAngleZ(6)+event.rTime* s);

		a = m_physics->RetMotorSpeedZ()*8.0f;
		aRight = a*0.07f;
		aLeft  = a*0.07f+PI;
		m_object->SetAngleY(3, m_object->RetAngleY(3)+(aRight-m_object->RetAngleY(3))*event.rTime*8.0f);
		m_object->SetAngleY(4, m_object->RetAngleY(4)+(aLeft-m_object->RetAngleY(4))*event.rTime*8.0f);

		// Tourne le volant.
		if ( m_model == 8 )  // torpedo ?
		{
			m_object->SetAngleX(8, m_object->RetAngleX(8)+(a*0.5f-m_object->RetAngleX(8))*event.rTime*8.0f);
		}
		else
		{
			m_object->SetAngleY(8, m_object->RetAngleY(8)+(a*0.5f-m_object->RetAngleY(8))*event.rTime*8.0f);
		}

		pos = m_object->RetPosition(0);
		angle = m_object->RetAngle(0);

		if ( Length(pos, m_engine->RetEyePt()) < 100.0f &&  // suspension ?
			 !m_object->RetDead() )
		{
			max = character->suspHeight;
			if ( m_camera->RetType() == CAMERA_ONBOARD )  max *= 0.5f;

			pos.x =  character->wheelFrontPos.x;  // roue avant droite
			pos.z = -character->wheelFrontPos.z;
			pos.y = -character->height;
			pos = Transform(*mat, pos);
			hr = m_terrain->RetFloorHeight(pos);
			if ( hr >  max )  hr =  max;
			if ( hr < -max )  hr = -max;

			pos.x =  character->wheelFrontPos.x;  // roue avant gauche
			pos.z =  character->wheelFrontPos.z;
			pos.y = -character->height;
			pos = Transform(*mat, pos);
			hl = m_terrain->RetFloorHeight(pos);
			if ( hl >  max )  hl =  max;
			if ( hl < -max )  hl = -max;

			CalcInclinaison(hl, hr, character->wheelFrontPos.z, a, d);
			m_object->SetAngleX(1, a);  // axe avant
			pos = m_object->RetPosition(1);
			pos.y = character->wheelFrontPos.y+d;
			m_object->SetPosition(1, pos);

			pos.x =  character->wheelBackPos.x;  // roue arrière droite
			pos.z = -character->wheelBackPos.z;
			pos.y = -character->height;
			pos = Transform(*mat, pos);
			hr = m_terrain->RetFloorHeight(pos);
			if ( hr >  max )  hr =  max;
			if ( hr < -max )  hr = -max;

			pos.x =  character->wheelBackPos.x;  // roue arrière gauche
			pos.z =  character->wheelBackPos.z;
			pos.y = -character->height;
			pos = Transform(*mat, pos);
			hl = m_terrain->RetFloorHeight(pos);
			if ( hl >  max )  hl =  max;
			if ( hl < -max )  hl = -max;

			CalcInclinaison(hl, hr, character->wheelBackPos.z, a, d);
			m_object->SetAngleX(2, a);  // axe avant
			pos = m_object->RetPosition(2);
			pos.y = character->wheelBackPos.y+d;
			m_object->SetPosition(2, pos);

			if ( m_model == 3 )
			{
				m_object->SetAngleX(38, -a+0.3f);  // amortisseur droite
				m_object->SetAngleX(39, -a-0.3f);  // amortisseur gauche
//?				m_object->SetZoomY(38, 0.8f-d*0.67f+hr*0.3f);
//?				m_object->SetZoomY(39, 0.8f-d*0.67f+hl*0.3f);
				m_object->SetZoomY(38, 1.0f-d*0.35f+hr*0.5f);
				m_object->SetZoomY(39, 1.0f-d*0.35f+hl*0.5f);
			}
		}
		else
		{
			m_object->SetAngleX(1, 0.0f);  // axe avant
			m_object->SetAngleX(2, 0.0f);  // axe arrière
		}

		if ( m_model == 1 )
		{
			UpdateGlassMapping(15, -5.0f, 5.0f,  1.0f, D3DMAPPING1Z);  // av
			UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, D3DMAPPING1Z);  // ar
		}
		if ( m_model == 2 )
		{
			UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, D3DMAPPING1Z);  // ar
			UpdateGlassMapping(14, -5.0f, 5.0f,  1.0f, D3DMAPPING1Z);  // av
		}
		if ( m_model == 3 )
		{
			UpdateGlassMapping(14, -5.0f, 5.0f,  1.0f, D3DMAPPING1Z);  // av
			UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, D3DMAPPING1Z);  // ar
		}
		if ( m_model == 4 )
		{
			UpdateGlassMapping(10, -5.0f, 5.0f,  1.0f, D3DMAPPING1Z);  // av
		}
		if ( m_model == 5 )
		{
			UpdateGlassMapping(10, -5.0f, 5.0f,  1.0f, D3DMAPPING1Z);  // av
		}
		if ( m_model == 6 )
		{
			UpdateGlassMapping(11, -5.0f, 5.0f,  1.0f, D3DMAPPING1Z, TRUE);  // av
			UpdateGlassMapping(12, -5.0f, 5.0f, -1.0f, D3DMAPPING1Z, TRUE);  // ar
		}
		if ( m_model == 7 )
		{
			UpdateGlassMapping(16, -5.0f, 5.0f,  1.0f, D3DMAPPING1Z, TRUE);  // av
			UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, D3DMAPPING1Z, TRUE);  // ar
//?			UpdateGlassMapping(12, -5.0f, 5.0f,  1.0f, D3DMAPPING1X, TRUE);  // g
//?			UpdateGlassMapping(13, -5.0f, 5.0f, -1.0f, D3DMAPPING1X, TRUE);  // d
		}
		if ( m_model == 8 )
		{
		}
	}

	if ( type == OBJECT_CAR )
	{
		if ( m_actionType == MV_OPEN  ||  // ouvre le toît ?
			 m_actionType == MV_CLOSE )   // ferme le toît ?
		{
			if ( m_model == 1 )  // tijuana taxi ?
			{
				progress = m_progress;
				if ( m_actionType == MV_CLOSE )  progress = 1.0f-m_progress;

				a = progress*PI*1.4f;
				m_object->SetAngleZ(10, a);  // toît
				rot = AdjustRot(-3.1f, 0.4f, a);
				pos = D3DVECTOR(-1.0f+rot.x, 6.0f+rot.y, 0.0f);
				m_object->SetPosition(10, pos);

				a = progress*PI*0.4f;
				m_object->SetAngleX(12, a);  // fenêtre gauche
				rot = AdjustRot(0.0f, -0.8f, -a);
				pos = D3DVECTOR(-3.2f, 5.0f+rot.y, 2.5f+rot.x);
				m_object->SetPosition(12, pos);

				m_object->SetAngleX(13, -a);  // fenêtre droite
				rot = AdjustRot(0.0f, -0.8f, a);
				pos = D3DVECTOR(-3.2f, 5.0f+rot.y, -2.5f+rot.x);
				m_object->SetPosition(13, pos);

				a = progress*PI*0.2f;
				m_object->SetAngleZ(14, -a);  // montant central
				rot = AdjustRot(0.0f, -1.0f, -a);
				pos = D3DVECTOR(-0.7f+rot.x, 5.0f+rot.y, 0.0f);
				m_object->SetPosition(14, pos);

				if ( m_actionType == MV_CLOSE && m_progress == 1.0f )
				{
					m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0), 0.7f, 1.8f);
				}

				if ( m_progress >= 1.0f )
				{
					SetAction(-1);
				}
			}
			else if ( m_model == 2 )  // ford 32 ?
			{
				if ( m_object->RetFret() != 0 )
				{
					SetAction(-1);
				}
				else
				{
					progress = m_progress;
					if ( m_actionType == MV_CLOSE )  progress = 1.0f-m_progress;

					OpenClosePart(progress);

					if ( m_actionType == MV_CLOSE && m_progress == 1.0f )
					{
						m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0), 0.7f, 1.8f);
					}

					if ( m_progress >= 1.0f )
					{
						SetAction(-1);
					}
				}
			}
			else if ( m_model == 6 )  // chevy ?
			{
				if ( m_object->RetFret() != 0 )
				{
					SetAction(-1);
				}
				else
				{
					progress = m_progress;
					if ( m_actionType == MV_CLOSE )  progress = 1.0f-m_progress;

					OpenClosePart(progress);

					if ( m_actionType == MV_CLOSE && m_progress == 1.0f )
					{
						m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0), 0.7f, 1.8f);
					}

					if ( m_progress >= 1.0f )
					{
						SetAction(-1);
					}
				}
			}
			else if ( m_model == 7 )  // reo ?
			{
				progress = m_progress;
				if ( m_actionType == MV_CLOSE )  progress = 1.0f-m_progress;

				a = progress*PI*1.3f;
				m_object->SetAngleZ(10, a);  // toît
				rot = AdjustRot(-2.9f*1.1f, 0.0f*1.1f, a);
				pos = D3DVECTOR(-1.8f*1.1f+rot.x, 5.3f*1.1f+rot.y, 0.0f*1.1f);
				m_object->SetPosition(10, pos);

				a = progress*PI*0.4f;
				m_object->SetAngleX(13, a);  // fenêtre gauche
				rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, -a);
				pos = D3DVECTOR(-4.2f*1.1f, 4.5f*1.1f+rot.y, 1.9f*1.1f+rot.x);
				m_object->SetPosition(13, pos);

				m_object->SetAngleX(12, -a);  // fenêtre droite
				rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, a);
				pos = D3DVECTOR(-4.2f*1.1f, 4.5f*1.1f+rot.y, -1.9f*1.1f+rot.x);
				m_object->SetPosition(12, pos);

				m_object->SetAngleX(15, a);  // montant gauche
				rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, -a);
				pos = D3DVECTOR(-1.2f*1.1f, 4.5f*1.1f+rot.y, 1.9f*1.1f+rot.x);
				m_object->SetPosition(15, pos);

				m_object->SetAngleX(14, -a);  // montant droite
				rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, a);
				pos = D3DVECTOR(-1.2f*1.1f, 4.5f*1.1f+rot.y, -1.9f*1.1f+rot.x);
				m_object->SetPosition(14, pos);
				
				if ( m_actionType == MV_CLOSE && m_progress == 1.0f )
				{
					m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0), 0.7f, 1.8f);
				}

				if ( m_progress >= 1.0f )
				{
					SetAction(-1);
				}
			}
			else
			{
				SetAction(-1);
			}
		}
	}

	if ( type == OBJECT_TRAX )  // chenilles ?
	{
		s = m_physics->RetLinMotionX(MO_MOTSPEED)*0.5f;
		a = m_physics->RetCirMotionY(MO_MOTSPEED)*2.5f;

		m_posTrackLeft  += event.rTime*(s+a);
		m_posTrackRight += event.rTime*(s-a);

		UpdateTrackMapping(m_posTrackLeft, m_posTrackRight);

		pos = m_object->RetPosition(0);
		angle = m_object->RetAngle(0);
		if ( pos.x   != m_wheelLastPos.x   ||
			 pos.y   != m_wheelLastPos.y   ||
			 pos.z   != m_wheelLastPos.z   ||
			 angle.x != m_wheelLastAngle.x ||
			 angle.y != m_wheelLastAngle.y ||
			 angle.z != m_wheelLastAngle.z )
		{
			m_wheelLastPos = pos;
			m_wheelLastAngle = angle;

			limit[0] =  15.0f*PI/180.0f;
			limit[1] = -15.0f*PI/180.0f;

//?			if ( Length(pos, m_engine->RetEyePt()) < 50.0f )  // suspension ?
			if ( FALSE )
			{
				character = m_object->RetCharacter();
				mat = m_object->RetWorldMatrix(0);

				pos.x =  character->wheelFrontPos.x;  // roue avant droite
				pos.z = -character->wheelFrontPos.z;
				pos.y =  0.0f;
				pos = Transform(*mat, pos);
				a1 = atanf(m_terrain->RetFloorHeight(pos)/character->wheelFrontPos.x);

				pos.x = -character->wheelBackPos.x;  // roue arrière droite
				pos.z = -character->wheelBackPos.z;
				pos.y =  0.0f;
				pos = Transform(*mat, pos);
				a2 = atanf(m_terrain->RetFloorHeight(pos)/character->wheelBackPos.x);

				a = (a2-a1)/2.0f;
				if ( a > limit[0] )  a = limit[0];
				if ( a < limit[1] )  a = limit[1];
				m_object->SetAngleZ(1, a);

				pos.x =  character->wheelFrontPos.x;  // roue avant gauche
				pos.z =  character->wheelFrontPos.z;
				pos.y =  0.0f;
				pos = Transform(*mat, pos);
				a1 = atanf(m_terrain->RetFloorHeight(pos)/character->wheelFrontPos.x);

				pos.x = -character->wheelBackPos.x;  // roue arrière gauche
				pos.z =  character->wheelBackPos.z;
				pos.y =  0.0f;
				pos = Transform(*mat, pos);
				a2 = atanf(m_terrain->RetFloorHeight(pos)/character->wheelBackPos.x);

				a = (a2-a1)/2.0f;
				if ( a > limit[0] )  a = limit[0];
				if ( a < limit[1] )  a = limit[1];
				m_object->SetAngleZ(2, a);
			}
			else
			{
				m_object->SetAngleZ(1, 0.0f);
				m_object->SetAngleZ(2, 0.0f);
			}
		}

		// Monte la pelle si tourne.
		a = Abs(m_physics->RetCirMotionY(MO_REASPEED)/m_physics->RetCirMotionY(MO_ADVSPEED));
		a *= 45.0f*PI/180.0f;
		a = Smooth(m_object->RetAngleZ(3), a, event.rTime);
		m_object->SetAngleZ(3, a);

		// Bouge les leviers si tourne.
		a = m_physics->RetCirMotionY(MO_REASPEED)/m_physics->RetCirMotionY(MO_ADVSPEED);
		a *= 45.0f*PI/180.0f;
		a = Smooth(m_object->RetAngleZ(4), a, event.rTime);
		m_object->SetAngleZ(4, a);
		m_object->SetAngleZ(5, -a);
	}

	return TRUE;
}

// Ouvre ou ferme une partie de la voiture lorsqu'un objet est transporté.

void CMotionVehicle::OpenClosePart(float progress)
{
	D3DVECTOR	pos;
	FPOINT		rot;
	float		a;

	if ( m_model == 2 )  // ford 32 ?
	{
		a = progress*(100.0f*PI/180.0f);
		m_object->SetAngleZ(24, a);  // coffre ar
	}

	if ( m_model == 6 )  // chevy ?
	{
		a = progress*PI*0.5f;
		m_object->SetAngleZ(10, a);  // toît
		rot = RotatePoint(progress*PI/2.0f, FPOINT(0.0f, 2.5f));
		pos.x = -1.0f+rot.x;
		pos.y =  2.5f+rot.y;
		pos.z =  0.0f;
		m_object->SetPosition(10, pos);
		m_object->SetZoomX(10, 1.0f-progress*0.8f);
		m_object->SetZoomZ(10, 1.0f-progress*0.3f);
	}
}


// Met à jour le mapping de la texture des roues.

void CMotionVehicle::UpdateWheelMapping(float speed, BOOL bFront)
{
	D3DMATERIAL7	mat;
	float			limit[4], au, bu, s;
	int				w[2], i;

	if ( m_model == 99 )  return;  // voiture fantome ?

	speed = Abs(speed);
	     if ( speed <  5.0f )  s = 0.00f;
	else if ( speed < 10.0f )  s = 0.25f;
	else if ( speed < 20.0f )  s = 0.50f;
	else                       s = 0.75f;

	i = bFront?0:1;
	if ( s == m_lastWheelSpeed[i] )  return;
	m_lastWheelSpeed[i] = s;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	if ( bFront )
	{
		w[0] = m_object->RetObjectRank(3);
		w[1] = m_object->RetObjectRank(4);
	}
	else
	{
		w[0] = m_object->RetObjectRank(5);
		w[1] = m_object->RetObjectRank(6);
	}

//?	limit[0] = 0.0f;
//?	limit[1] = m_engine->RetLimitLOD(0);
//?	limit[2] = limit[1];
//?	limit[3] = m_engine->RetLimitLOD(1);
	limit[0] = 0.0f;
	limit[1] = 1000000.0f;
	limit[2] = 0.0f;
	limit[3] = 1000000.0f;

	// faces des roues :
	au = 0.25f/2.0f;
	bu = s+0.25f/2.0f;
	for ( i=0 ; i<1 ; i++ )
	{
		m_engine->ChangeTextureMapping(w[0],
									   mat, D3DSTATEPART1, m_nameTexWheel, "",
									   limit[i*2+0], limit[i*2+1], D3DMAPPING1X,
									   au, bu, 1.0f, 0.0f);
		m_engine->ChangeTextureMapping(w[1],
									   mat, D3DSTATEPART1, m_nameTexWheel, "",
									   limit[i*2+0], limit[i*2+1], D3DMAPPING1X,
									   au, bu, 1.0f, 0.0f);
	}

	// profil des roues :
	au = s+0.00f+0.5f/256.0f;
	bu = s+0.25f-0.5f/256.0f;
	for ( i=0 ; i<1 ; i++ )
	{
		m_engine->ChangeTextureMapping(w[0],
									   mat, D3DSTATEPART2, m_nameTexWheel, "",
									   limit[i*2+0], limit[i*2+1], D3DMAPPINGMX,
									   au, bu, 0.0f, 0.0f);
		m_engine->ChangeTextureMapping(w[1],
									   mat, D3DSTATEPART2, m_nameTexWheel, "",
									   limit[i*2+0], limit[i*2+1], D3DMAPPINGMX,
									   au, bu, 0.0f, 0.0f);
	}
}

// Met à jour le mapping de la texture des vitres.

void CMotionVehicle::UpdateGlassMapping(int part, float min, float max,
										float dir, D3DMaping mode, BOOL bBlack)
{
	D3DVECTOR		eye, look;
	D3DMATERIAL7	mat;
	float			limit[4], au, bu, angle;
	int				rank, i, mmode;

	rank = m_object->RetObjectRank(part);

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

//?	limit[0] = 0.0f;
//?	limit[1] = m_engine->RetLimitLOD(0);
//?	limit[2] = limit[1];
//?	limit[3] = m_engine->RetLimitLOD(1);
	limit[0] = 0.0f;
	limit[1] = 1000000.0f;
	limit[2] = 0.0f;
	limit[3] = 1000000.0f;

	eye  = m_engine->RetEyePt();
	look = m_engine->RetLookatPt();
	angle = RotateAngle(look.x-eye.x, look.z-eye.z)*dir;
	angle = Mod(angle/PI, 2.0f);
//?	angle = angle/(PI*2.0f)+0.5f;

	au = 0.5f/(max-min);
	bu = -(0.5f+angle);

	if ( bBlack )  mmode = D3DSTATETTw|D3DSTATE2FACE|D3DSTATEWRAP;
	else           mmode = D3DSTATETTb|D3DSTATE2FACE|D3DSTATEWRAP;

	for ( i=0 ; i<1 ; i++ )
	{
		m_engine->ChangeTextureMapping(rank, mat, mmode, m_nameTexWheel, "",
									   limit[i*2+0], limit[i*2+1],
									   mode, au, bu, 1.0f, 0.0f);
	}
}

// Met à jour le mapping de la texture des chenilles.

void CMotionVehicle::UpdateTrackMapping(float left, float right)
{
	D3DMATERIAL7	mat;
	float			limit[4];
	int				rRank, lRank, i;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	rRank = m_object->RetObjectRank(1);
	lRank = m_object->RetObjectRank(2);

//?	limit[0] = 0.0f;
//?	limit[1] = m_engine->RetLimitLOD(0);
//?	limit[2] = limit[1];
//?	limit[3] = m_engine->RetLimitLOD(1);
	limit[0] = 0.0f;
	limit[1] = 1000000.0f;
	limit[2] = 0.0f;
	limit[3] = 1000000.0f;

	for ( i=0 ; i<2 ; i++ )
	{
		m_engine->TrackTextureMapping(rRank, mat, D3DSTATEPART1, "trax.tga", "",
									  limit[i*2+0], limit[i*2+1], D3DMAPPINGX,
									  right, 2.0f, 8.0f, 192.0f, 256.0f);

		m_engine->TrackTextureMapping(lRank, mat, D3DSTATEPART2, "trax.tga", "",
									  limit[i*2+0], limit[i*2+1], D3DMAPPINGX,
									  left, 2.0f, 8.0f, 192.0f, 256.0f);
	}
}


// Initialse la torsion de toutes les pièces pour qu'elles soient
// toutes droites.

void CMotionVehicle::TwistInit()
{
	D3DVECTOR	min, max, factor;
	float		a;
	int			i, objRank;

	for ( i=0 ; i<m_partTotal ; i++ )
	{
		if ( m_part[i].bDeleted )  continue;

		m_part[i].twistState = 0;

		objRank = m_object->RetObjectRank(m_part[i].part);
		if ( objRank == -1 )
		{
			factor = D3DVECTOR(1.0f, 1.0f, 1.0f);
		}
		else
		{
			m_engine->GetBBox(objRank, min, max);
			max = max-min;

			a = RotateAngle(max.y, max.x);
			a = Abs(a-PI/4.0f)/(PI/4.0f);
			factor.x = (1.0f-a*0.8f)*0.7f;

			a = RotateAngle(max.x, max.z);
			a = Abs(a-PI/4.0f)/(PI/4.0f);
			factor.y = (1.0f-a*0.8f)*0.7f;

			a = RotateAngle(max.x, max.y);
			a = Abs(a-PI/4.0f)/(PI/4.0f);
			factor.z = (1.0f-a*0.8f)*0.7f;
		}

		m_part[i].twistAngle.x = (Rand()-0.5f)*factor.x;
		m_part[i].twistAngle.y = (Rand()-0.5f)*factor.y;
		m_part[i].twistAngle.z = (Rand()-0.5f)*factor.z;
	}
}

// Initialise les torsions selon une copie de l'état.

void CMotionVehicle::TwistInit(StateVehicle *sv)
{
	D3DVECTOR	angle;
	int			i;

	for ( i=0 ; i<MAX_PART ; i++ )
	{
		if ( m_part[i].bDeleted )  continue;

		m_part[i].twistState = sv->state[i].twistState;
		m_part[i].twistAngle = sv->state[i].twistAngle;

		angle = m_part[i].twistAngle*(m_part[i].twistState/4.0f);
		m_object->SetAngle(m_part[i].part, angle);
	}
}

// Tord qq pièces suite à un choc.

void CMotionVehicle::TwistPart(D3DVECTOR impact, float force)
{
	D3DVECTOR	angle;
	int			i, j, total;

	total = 0;
	for ( j=0 ; j<1000 ; j++ )
	{
		if ( total >= 5 ) break;  // termine si 5 pces tordues

		i = rand()%MAX_PART;  // choix d'une pièce zau zazar

		if ( m_model == 2 && m_part[i].part == 24 )  continue;  // coffre arrière ?
		if ( m_model == 6 && m_part[i].part == 10 )  continue;  // toît ?

		if ( m_part[i].bDeleted )  continue;
		if ( m_part[i].twistState >= 4 )  continue;

		m_part[i].twistState ++;
		angle = m_part[i].twistAngle*(m_part[i].twistState/4.0f);
		m_object->SetAngle(m_part[i].part, angle);

		total ++;
	}
}

// Retourne le numéro de la prochaine partie à détruire,
// et considère quelle est déjà virtuellement détruite.

int CMotionVehicle::RetRemovePart(int &param)
{
	int			order, i, j;
	int			iCandidate, tCandidate[MAX_PART];

	param = 0;

	iCandidate = 0;
	for ( order=0 ; order<m_partTotal ; order++ )
	{
		for ( i=0 ; i<m_partTotal ; i++ )
		{
			if ( m_part[i].bDeleted )  continue;
			if ( m_part[i].order != order )  continue;
			tCandidate[iCandidate++] = i;
		}
		if ( iCandidate > 0 )  break;
	}

	if ( iCandidate == 0 )  return -1;

	i = (int)(Rand()*iCandidate);
	if ( i == iCandidate )  i--;
	i = tCandidate[i];

	for ( j=0 ; j<4 ; j++ )
	{
		if ( m_part[i].light[j] != -1 )
		{
			m_bLight[m_part[i].light[j]] = FALSE;  // un phare cassé
		}
	}

	m_topRest -= m_part[i].top;
	m_part[i].bDeleted = TRUE;
	m_partUsed --;

	if ( m_model == 2 )  // ford 32 ?
	{
		if ( m_part[i].part == 24 )  // coffre arrière ?
		{
			param = 1;  // force une rotation en x
		}
	}

	if ( m_model == 6 )  // chevy ?
	{
		if ( m_part[i].part == 19 ||  // coffre avant ?
			 m_part[i].part == 20 )   // coffre arrière ?
		{
			param = 1;  // force une rotation en x
		}
	}

	return m_part[i].part;
}


// Indique si un phare existe.

BOOL CMotionVehicle::RetLight(int rank)
{
	return m_bLight[rank];
}

// Indique la présence d'une partie spécifique.

BOOL CMotionVehicle::ExistPart(TypePart part)
{
	if ( part == TP_TOP )  return (m_topRest > 0);
	return FALSE;
}

// Donne le nombre de pièces total du véhicule.

int CMotionVehicle::RetTotalPart()
{
	return m_partTotal;
}

// Donne le nombre de pièces utilisées du véhicule.

int CMotionVehicle::RetUsedPart()
{
	return m_partUsed;
}


// Gestion du type de roues.

void CMotionVehicle::SetWheelType(WheelType type)
{
	CModFile*	pModFile;

	if ( type == m_wheelType )  return;  // inchangé ?

	CMotion::SetWheelType(type);

	if ( type == WT_NORM )
	{
		m_object->DeletePart(3);
		m_object->DeletePart(4);
		m_object->DeletePart(5);
		m_object->DeletePart(6);  // supprime les roues existantes

		pModFile = new CModFile(m_iMan);
		CreateWheel(pModFile, m_nameObjWheel);
		delete pModFile;

		CreatePhysics();  // recrée une physique adaptée
	}

	if ( type == WT_BURN )
	{
		m_object->DeletePart(3);
		m_object->DeletePart(4);
		m_object->DeletePart(5);
		m_object->DeletePart(6);  // supprime les roues existantes

		pModFile = new CModFile(m_iMan);
		CreateWheel(pModFile, m_nameObjWBurn);
		delete pModFile;

		CreatePhysics();  // recrée une physique adaptée
	}
}

WheelType CMotionVehicle::RetWheelType()
{
	return CMotion::RetWheelType();
}


// Gestion de l'état du véhicule.

int CMotionVehicle::RetStateLength()
{
	return sizeof(StateVehicle);
}

void CMotionVehicle::GetStateBuffer(char *buffer)
{
	StateVehicle*	p;
	int				i;

	p = (StateVehicle*)buffer;
	for ( i=0 ; i<MAX_PART ; i++ )
	{
		p->state[i].bDeleted   = m_part[i].bDeleted;
		p->state[i].twistState = m_part[i].twistState;
		p->state[i].twistAngle = m_part[i].twistAngle;
	}
}


// Charge le robot marcheur le plus proche.

void CMotionVehicle::ActionLoadBot(float rTime)
{
	CBrain*		brain;
	CPhysics*	physics;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		s, duration, dist, angle;
	int			i;

	if ( m_lbStart )
	{
		m_lbStart = FALSE;
		m_lbWalker = SearchObject(OBJECT_WALKER, m_object->RetPosition(0), 40.0f);

		if ( m_lbWalker == 0 )
		{
			brain = 0;
			physics = 0;
		}
		else
		{
			brain = m_lbWalker->RetBrain();
			physics = m_lbWalker->RetPhysics();
		}

		s = Abs(m_physics->RetLinMotionX(MO_REASPEED));

		if ( m_lbWalker == 0           ||  // rien trouvé ?
			 s > 1.0f                  ||  // voiture pas arrêté ?
			 brain == 0                ||  // pas de cerveau ?
			 physics == 0              ||  // pas de physique ?
			 brain->RetProgram() == -1 ||  // ne marche pas ?
			 m_object->RetFret() != 0  ||  // porte déjà qq chose ?
			 m_object->RetLock()       )   // occupé ?
		{
			m_lbAction = FALSE;
			SetAction(-1);
			return;
		}

		m_lbProgress = 0.0f;
		m_lbSpeed = m_actionTime;
		m_lbTime = 0.0f;

		brain->StopProgram();  // stoppe le programme en cours
		physics->SetLinMotion(MO_MOTSPEED, D3DVECTOR(0.0f, 0.0f, 0.0f));
		physics->SetLinMotion(MO_REASPEED, D3DVECTOR(0.0f, 0.0f, 0.0f));
		StartObjectAction(m_object, MV_OPEN);
		StartObjectAction(m_lbWalker, MB_GOHOME);
		m_object->SetLock(TRUE);
		m_lbWalker->SetLock(TRUE);

		m_lbCamera = m_camera->RetType();
		if ( m_lbCamera != CAMERA_BACK )  m_camera->SetType(CAMERA_BACK);
		m_lbBackDist = m_camera->RetBackDist();

		m_lbStartPos = m_lbWalker->RetPosition(0);
		m_lbGoalPos = RetVehiclePoint(m_object);
		m_lbGoalPos.y += m_lbWalker->RetCharacter()->height;

		m_lbStartAngle = NormAngle(m_lbWalker->RetAngleY(0));
		m_lbGoalAngle  = NormAngle(m_object->RetAngleY(0))+PI*2.0f;

		i = m_sound->Play(SOUND_FLY, m_lbGoalPos, 0.0f, 1.0f, TRUE);
		m_sound->AddEnvelope(i, 1.0f, 1.5f, 0.5f, SOPER_CONTINUE);
		m_sound->AddEnvelope(i, 1.0f, 1.5f, 2.0f, SOPER_CONTINUE);
		m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.5f, SOPER_STOP);
	}

	m_lbProgress += rTime*m_lbSpeed;
	m_lbTime += rTime;

	if ( m_lbProgress < 1.0f )
	{
		pos = m_lbStartPos+(m_lbGoalPos-m_lbStartPos)*m_lbProgress;
		pos.y += sinf(m_lbProgress*PI)*10.0f;
		m_lbWalker->SetPosition(0, pos);

		angle = m_lbStartAngle+(m_lbGoalAngle-m_lbStartAngle)*m_lbProgress;
		m_lbWalker->SetAngleY(0, angle);

		dist = m_lbBackDist+sinf(m_lbProgress*PI)*30.0f;
		m_camera->SetBackDist(dist);

		if ( m_lbLastParticule+m_engine->ParticuleAdapt(0.05f) <= m_lbTime )
		{
			m_lbLastParticule = m_lbTime;

			pos.y -= m_lbWalker->RetCharacter()->height;
			for ( i=0 ; i<4 ; i++ )
			{
				speed.x = (Rand()-0.5f)*2.0f;
				speed.z = (Rand()-0.5f)*2.0f;
				speed.y = -Rand()*5.0f;
				dim.x = Rand()*1.0f+1.0f;
				dim.y = dim.x;
				duration = Rand()*1.0f+1.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTIGAS, duration);
			}
		}
	}
	else
	{
		m_lbWalker->SetTruck(m_object);
		m_object->SetFret(m_lbWalker);
		pos = m_object->RetCharacter()->posFret;
		pos.y += m_lbWalker->RetCharacter()->height;
		m_lbWalker->SetPosition(0, pos);
		m_lbWalker->SetAngle(0, m_object->RetCharacter()->angleFret);

		m_object->SetLock(FALSE);
		m_lbWalker->SetLock(FALSE);
		StartObjectAction(m_object, MV_CLOSE);
		StartObjectAction(m_lbWalker, MB_WAIT);

		if ( m_lbCamera != CAMERA_BACK )  m_camera->SetType(m_lbCamera);
		m_camera->SetBackDist(m_lbBackDist);

		m_lbAction = FALSE;
	}
}

// Cherche un objet proche.

CObject* CMotionVehicle::SearchObject(ObjectType type, D3DVECTOR center, float radius)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetExplo() )  continue;

		if ( type != pObj->RetType() )  continue;

		pos = pObj->RetPosition(0);
		dist = Length(pos, center);

		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

// Cherche le point où mettre du fret sur un véhicule.

D3DVECTOR CMotionVehicle::RetVehiclePoint(CObject *pObj)
{
	Character*	character;
	D3DMATRIX*	mat;
	D3DVECTOR	pos;

	character = pObj->RetCharacter();
	mat = pObj->RetWorldMatrix(0);
	pos = Transform(*mat, character->posFret);

	return pos;
}

// Démarre une action pour un objet.

void CMotionVehicle::StartObjectAction(CObject *pObj, int action)
{
	CMotion*	motion;
	float		delay;

	motion = pObj->RetMotion();
	if ( motion == 0 )  return;

	delay = 1.0f;
	motion->SetAction(action, delay);
}


