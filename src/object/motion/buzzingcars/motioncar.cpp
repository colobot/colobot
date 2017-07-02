/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either  version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A Gfx::PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */


#include "object/motion/buzzingcars/motioncar.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/all.h"

#include "object/old_object.h"
#include "object/object_manager.h"

#include "object/motion/buzzingcars/motionbot.h"

#include "physics/physics.h"

#include "sound/sound.h"

//const float ARM_NEUTRAL_ANGLE1 = 110.0f*Math::Math::PI/180.0f;
//const float ARM_NEUTRAL_ANGLE2 = -130.0f*Math::Math::PI/180.0f;
//const float ARM_NEUTRAL_ANGLE3 = -50.0f*Math::Math::PI/180.0f




RemovePart tableCar01[] =    // tijuana taxi
{
    {10, 0, -1.0f,  6.0f,  0.0f, {-1,-1,-1,-1}, 1, 10},  // toît
    {11, 1, -4.2f,  5.4f,  0.0f, {-1,-1,-1,-1}, 1, 11},  // fenêtre ar
    {12, 0, -3.2f,  5.0f,  2.5f, {-1,-1,-1,-1}, 1, 12},  // fenêtre latérale g
    {13, 0, -3.2f,  5.0f, -2.5f, {-1,-1,-1,-1}, 1, 13},  // fenêtre latérale d
    {14, 3, -0.7f,  5.0f,  0.0f, {-1,-1,-1,-1}, 1, 14},  // montant central
    {15, 3,  1.7f,  4.7f,  0.0f, {-1,-1,-1,-1}, 0, 15},  // montant av
    {16, 2, -3.9f,  3.0f,  0.0f, { 2, 3, 4, 5}, 0, 16},  // carro ar
    {17, 4, -2.0f,  3.0f,  2.2f, {-1,-1,-1,-1}, 0, 17},  // carro g
    {18, 4, -2.0f,  3.0f, -2.2f, {-1,-1,-1,-1}, 0, 18},  // carro d
    {19, 4,  1.9f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 19},  // carro av
    {20, 0, -0.2f,  0.8f,  3.0f, {-1,-1,-1,-1}, 0, 20},  // marche pied g
    {21, 0, -0.2f,  0.8f, -3.0f, {-1,-1,-1,-1}, 0, 20},  // marche pied d
    {22, 1,  4.5f,  1.8f,  0.0f, {-1,-1,-1,-1}, 0, 22},  // radiateur
    {23, 0,  4.8f,  2.3f,  1.9f, { 0,-1,-1,-1}, 0, 23},  // phare g
    {24, 0,  4.8f,  2.3f, -1.9f, { 1,-1,-1,-1}, 0, 23},  // phare d
    {25, 2,  1.2f,  3.0f,  0.0f, {-1,-1,-1,-1}, 0, 25},  // tableau de bord
    {26, 0, -5.3f,  4.0f,  0.0f, {-1,-1,-1,-1}, 0, 26},  // malle
    {27, 1, -4.6f,  2.5f,  0.9f, {-1,-1,-1,-1}, 0, 27},  // support g
    {28, 1, -4.6f,  2.5f, -0.9f, {-1,-1,-1,-1}, 0, 27},  // support d
    {29, 0, -3.0f,  3.0f,  3.1f, {-1,-1,-1,-1}, 0, 29},  // gardeboue ar g
    {30, 0, -3.0f,  3.0f, -3.1f, {-1,-1,-1,-1}, 0, 30},  // gardeboue ar d
    {-1}
};

RemovePart tableCar02[] =    // ford 32
{
    {10, 0, -0.7f,  5.5f,  0.0f, {-1,-1,-1,-1}, 0, 10},  // toît
    {11, 0, -2.6f,  4.8f,  0.0f, {-1,-1,-1,-1}, 0, 11},  // fenêtre ar
    {12, 1, -2.0f,  4.7f, -2.9f, {-1,-1,-1,-1}, 0, 12},  // fenêtre latérale d
    {13, 1, -2.0f,  4.7f,  2.9f, {-1,-1,-1,-1}, 0, 13},  // fenêtre latérale g
    {14, 1,  1.2f,  4.4f,  0.0f, {-1,-1,-1,-1}, 0, 14},  // montant av
    {15, 2, -1.1f,  2.5f, -2.9f, {-1,-1,-1,-1}, 0, 15},  // porte droite
    {16, 2, -1.1f,  2.5f,  2.9f, {-1,-1,-1,-1}, 0, 16},  // porte gauche
    {17, 3, -2.9f,  2.8f,  0.0f, {-1,-1,-1,-1}, 0, 17},  // carro ar
    {18, 3,  1.5f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 18},  // carro av
    {19, 0,  5.7f,  2.8f, -1.4f, { 1,-1,-1,-1}, 0, 19},  // phare av d
    {20, 0,  5.7f,  2.8f,  1.4f, { 0,-1,-1,-1}, 0, 19},  // phare av g
    {21, 2,  5.5f,  2.0f,  0.0f, {-1,-1,-1,-1}, 0, 21},  // radiateur
    {22, 2, -4.5f,  3.0f, -2.5f, { 3, 5,-1,-1}, 0, 22},  // aile ar d
    {23, 2, -4.5f,  3.0f,  2.5f, { 2, 4,-1,-1}, 0, 23},  // aile ar g
    {24, 0, -6.2f,  2.3f,  0.0f, {-1,-1,-1,-1}, 0, 24},  // coffre ar
    {25, 0, -6.5f,  1.9f,  0.0f, {-1,-1,-1,-1}, 0, 25},  // parchoc ar
    {26, 2,  0.8f,  3.2f,  0.0f, {-1,-1,-1,-1}, 0, 26},  // tableau de bord
    {-1}
};

RemovePart tableCar03[] =    // pickup
{
    {10, 0, -0.9f,  5.3f,  0.0f, {-1,-1,-1,-1}, 0, 10},  // toît
    {11, 0, -2.4f,  4.3f,  0.0f, {-1,-1,-1,-1}, 0, 11},  // fenêtre ar
    {12, 1, -2.0f,  4.4f, -1.9f, {-1,-1,-1,-1}, 0, 12},  // fenêtre latérale d
    {13, 1, -2.0f,  4.4f,  1.9f, {-1,-1,-1,-1}, 0, 13},  // fenêtre latérale g
    {14, 1,  0.6f,  4.3f,  0.0f, {-1,-1,-1,-1}, 0, 14},  // montant av
    {15, 0,  0.8f,  5.2f,  0.0f, {-1,-1,-1,-1}, 0, 15},  // visière
    {16, 0, -0.5f,  2.5f, -1.9f, {-1,-1,-1,-1}, 0, 16},  // porte droite
    {17, 0, -0.5f,  2.5f,  1.9f, {-1,-1,-1,-1}, 0, 17},  // porte gauche
    {18, 2, -2.4f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 18},  // carro ar
    {19, 2, -2.0f,  2.5f, -1.9f, {-1,-1,-1,-1}, 0, 19},  // carro latérale d
    {20, 2, -2.0f,  2.5f,  1.9f, {-1,-1,-1,-1}, 0, 20},  // carro latérale g
    {21, 2,  0.8f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 21},  // carro av
    {22, 0,  4.7f,  2.2f, -1.4f, { 1,-1,-1,-1}, 0, 22},  // phare av d
    {23, 0,  4.7f,  2.2f,  1.4f, { 0,-1,-1,-1}, 0, 22},  // phare av g
    {24, 1,  4.2f,  2.0f,  0.0f, {-1,-1,-1,-1}, 0, 24},  // radiateur
    {25, 0, -0.5f,  1.1f, -2.3f, {-1,-1,-1,-1}, 0, 25},  // marche pied d
    {26, 0, -0.5f,  1.1f,  2.3f, {-1,-1,-1,-1}, 0, 26},  // marche pied g
    {27, 0,  3.0f,  2.0f, -2.5f, {-1,-1,-1,-1}, 0, 27},  // garde-boue av d
    {28, 0,  3.0f,  2.0f,  2.5f, {-1,-1,-1,-1}, 0, 28},  // garde-boue av g
    {29, 0, -4.0f,  2.0f, -2.5f, {-1,-1,-1,-1}, 0, 29},  // garde-boue ar d
    {30, 0, -4.0f,  2.0f,  2.5f, {-1,-1,-1,-1}, 0, 30},  // garde-boue ar g
    {31, 0, -4.5f,  3.5f, -2.2f, {-1,-1,-1,-1}, 0, 31},  // barrière d
    {32, 0, -4.5f,  3.5f,  2.2f, {-1,-1,-1,-1}, 0, 32},  // barrière g
    {33, 0,  0.2f,  3.0f,  0.0f, {-1,-1,-1,-1}, 0, 33},  // tableau de bord
    {34, 0, -5.4f,  2.5f, -2.6f, { 3, 5,-1,-1}, 0, 34},  // phare ar d
    {35, 0, -5.4f,  2.5f,  2.6f, { 2, 4,-1,-1}, 0, 35},  // phare ar g
    {-1}
};

RemovePart tableCar04[] =    // firecraker
{
    {10, 1,  2.3f,  4.8f,  0.0f, {-1,-1,-1,-1}, 0, 10},  // fenêtre av
    {11, 0,  1.0f,  1.8f, -1.9f, {-1,-1,-1,-1}, 0, 11},  // triangle d
    {12, 0,  1.0f,  1.8f,  1.9f, {-1,-1,-1,-1}, 0, 12},  // triangle g
    {13, 0,  1.6f,  3.3f,  0.0f, {-1,-1,-1,-1}, 0, 13},  // tableau de bord
    {14, 2,  2.0f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 14},  // carro av
    {15, 2, -1.5f,  3.8f,  0.0f, {-1,-1,-1,-1}, 0, 15},  // siège
    {16, 0, -3.5f,  4.0f, -2.1f, {-1,-1,-1,-1}, 0, 16},  // barrière d
    {17, 0, -3.5f,  4.0f,  2.1f, {-1,-1,-1,-1}, 0, 16},  // barrière g
    {18, 0, -1.0f,  1.5f, -2.7f, {-1,-1,-1,-1}, 0, 18},  // marche pied d
    {19, 0, -1.0f,  1.5f,  2.7f, {-1,-1,-1,-1}, 0, 19},  // marche pied g
    {20, 0,  4.5f,  1.8f, -2.7f, {-1,-1,-1,-1}, 0, 20},  // garde-boue av d
    {21, 0,  4.5f,  1.8f,  2.7f, {-1,-1,-1,-1}, 0, 21},  // garde-boue av g
    {22, 0, -5.0f,  3.0f, -2.7f, { 3, 5,-1,-1}, 0, 22},  // garde-boue ar d
    {23, 0, -5.0f,  3.0f,  2.7f, { 2, 4,-1,-1}, 0, 23},  // garde-boue ar g
    {24, 0,  5.2f,  2.0f, -1.4f, { 1,-1,-1,-1}, 0, 24},  // phare av d
    {25, 0,  5.2f,  2.0f,  1.4f, { 0,-1,-1,-1}, 0, 25},  // phare av g
    {26, 0,  6.0f,  1.2f,  0.0f, {-1,-1,-1,-1}, 0, 26},  // parchoc av
    {27, 0, -7.0f,  2.0f,  0.0f, {-1,-1,-1,-1}, 0, 27},  // parchoc ar
    {28, 1,  4.5f,  2.0f,  0.0f, {-1,-1,-1,-1}, 0, 28},  // radiateur
    {29, 0,  3.3f,  3.5f,  0.0f, {-1,-1,-1,-1}, 0, 29},  // capot sup
    {30, 0,  3.3f,  3.0f, -1.0f, {-1,-1,-1,-1}, 0, 30},  // capot d
    {31, 0,  3.3f,  3.0f,  1.0f, {-1,-1,-1,-1}, 0, 31},  // capot g
    {32, 0,  2.3f,  4.0f, -2.3f, {-1,-1,-1,-1}, 0, 24},  // phare sup d
    {33, 0,  2.3f,  4.0f,  2.3f, {-1,-1,-1,-1}, 0, 25},  // phare sup g
    {-1}
};

RemovePart tableCar05[] =    // hooligan
{
    {10, 0,  1.0f,  4.3f,  0.0f, {-1,-1,-1,-1}, 0, 10},  // fenêtre av
    {11, 0, -0.4f,  2.6f, -2.2f, {-1,-1,-1,-1}, 0, 11},  // porte d
    {12, 0, -0.4f,  2.6f,  2.2f, {-1,-1,-1,-1}, 0, 12},  // porte g
    {13, 1,  1.3f,  2.7f,  0.0f, {-1,-1,-1,-1}, 0, 13},  // carro av
    {14, 1, -1.9f,  3.0f,  0.0f, {-1,-1,-1,-1}, 0, 14},  // carro ar
    {15, 1,  0.6f,  3.0f,  0.0f, {-1,-1,-1,-1}, 0, 15},  // tableau de bord
    {16, 0,  3.0f,  3.7f,  0.0f, {-1,-1,-1,-1}, 0, 16},  // capot
    {17, 1,  5.2f,  2.4f,  0.0f, {-1,-1,-1,-1}, 0, 17},  // radiateur
    {18, 0,  4.8f,  3.4f, -1.6f, { 1,-1,-1,-1}, 0, 18},  // phare av d
    {19, 0,  4.8f,  3.4f,  1.6f, { 0,-1,-1,-1}, 0, 18},  // phare av g
    {20, 1, -1.8f,  1.1f, -2.9f, {-1,-1,-1,-1}, 0, 20},  // marche pied d
    {21, 1, -1.8f,  1.1f,  2.9f, {-1,-1,-1,-1}, 0, 20},  // marche pied g
    {22, 0,  3.5f,  2.0f, -2.9f, {-1,-1,-1,-1}, 0, 22},  // garde-boue av d
    {23, 0,  3.5f,  2.0f,  2.9f, {-1,-1,-1,-1}, 0, 22},  // garde-boue av g
    {24, 0, -4.2f,  2.2f, -2.9f, {-1,-1,-1,-1}, 0, 24},  // garde-boue ar d
    {25, 0, -4.2f,  2.2f,  2.9f, {-1,-1,-1,-1}, 0, 24},  // garde-boue ar g
    {26, 0, -5.7f,  3.0f, -2.9f, { 3, 5,-1,-1}, 0, 26},  // phare ar d
    {27, 0, -5.7f,  3.0f,  2.9f, { 2, 4,-1,-1}, 0, 27},  // phare ar g
    {28, 1, -4.0f,  2.8f, -2.1f, {-1,-1,-1,-1}, 0, 28},  // aile ar d
    {29, 1, -4.0f,  2.8f,  2.1f, {-1,-1,-1,-1}, 0, 29},  // aile ar g
    {30, 0, -6.2f,  1.8f,  0.0f, {-1,-1,-1,-1}, 0, 30},  // parchoc arrière
    {-1}
};

RemovePart tableCar06[] =    // chevy
{
    {10, 0, -1.0f,  5.0f,  0.0f, {-1,-1,-1,-1}, 1, 10},  // toît
    {11, 0,  2.2f,  4.0f,  0.0f, {-1,-1,-1,-1}, 0, 11},  // fenêtre av
    {12, 0, -4.0f,  4.0f,  0.0f, {-1,-1,-1,-1}, 0, 12},  // fenêtre ar
    {13, 1,  1.7f,  4.0f, -2.6f, {-1,-1,-1,-1}, 0, 13},  // montant av d
    {14, 1,  1.7f,  4.0f,  2.6f, {-1,-1,-1,-1}, 0, 14},  // montant av g
    {15, 1, -3.5f,  3.8f, -2.6f, {-1,-1,-1,-1}, 0, 15},  // montant ar d
    {16, 1, -3.5f,  3.8f,  2.6f, {-1,-1,-1,-1}, 0, 16},  // montant ar g
    {17, 0, -0.2f,  2.2f, -2.9f, {-1,-1,-1,-1}, 0, 17},  // porte droite
    {18, 0, -0.2f,  2.2f,  2.9f, {-1,-1,-1,-1}, 0, 18},  // porte gauche
    {19, 0,  4.5f,  3.0f,  0.0f, {-1,-1,-1,-1}, 0, 19},  // capot av
    {20, 0, -5.7f,  3.3f,  0.0f, {-1,-1,-1,-1}, 0, 20},  // capot ar
    {21, 2,  4.0f,  2.2f, -2.5f, { 1,-1,-1,-1}, 0, 21},  // aile av d
    {22, 2,  4.0f,  2.2f,  2.5f, { 0,-1,-1,-1}, 0, 22},  // aile av g
    {23, 2, -4.5f,  2.5f, -2.7f, { 3, 5,-1,-1}, 0, 23},  // aile ar d
    {24, 2, -4.5f,  2.5f,  2.7f, { 2, 4,-1,-1}, 0, 24},  // aile ar g
    {25, 0,  6.7f,  1.6f,  0.0f, {-1,-1,-1,-1}, 0, 25},  // parchoc av
    {26, 0, -7.3f,  1.7f,  0.0f, {-1,-1,-1,-1}, 0, 26},  // parchoc ar
    {-1}
};

RemovePart tableCar07[] =    // reo
{
    {10, 0, -1.8f,  5.3f,  0.0f, {-1,-1,-1,-1}, 1, 10},  // toît
    {11, 1, -4.7f,  4.5f,  0.0f, {-1,-1,-1,-1}, 0, 11},  // fenêtre ar
    {12, 1, -4.2f,  4.5f, -1.9f, {-1,-1,-1,-1}, 1, 12},  // fenêtre latérale d
    {13, 1, -4.2f,  4.5f,  1.9f, {-1,-1,-1,-1}, 1, 13},  // fenêtre latérale g
    {14, 1, -1.2f,  4.5f, -1.9f, {-1,-1,-1,-1}, 1, 14},  // montant latéral d
    {15, 1, -1.2f,  4.5f,  1.9f, {-1,-1,-1,-1}, 1, 15},  // montant latéral g
    {16, 1,  1.2f,  4.5f,  0.0f, {-1,-1,-1,-1}, 0, 16},  // montant av
    {17, 0,  0.1f,  2.5f, -1.8f, {-1,-1,-1,-1}, 0, 17},  // porte droite
    {18, 0,  0.1f,  2.5f,  1.8f, {-1,-1,-1,-1}, 0, 18},  // porte gauche
    {19, 2, -5.0f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 19},  // carro ar
    {20, 2, -3.0f,  2.5f, -1.9f, {-1,-1,-1,-1}, 0, 20},  // carro latérale d
    {21, 2, -3.0f,  2.5f,  1.9f, {-1,-1,-1,-1}, 0, 21},  // carro latérale g
    {22, 2,  1.5f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 22},  // carro av
    {23, 0,  5.0f,  2.8f, -1.4f, { 1,-1,-1,-1}, 0, 23},  // phare av d
    {24, 0,  5.0f,  2.8f,  1.4f, { 0,-1,-1,-1}, 0, 24},  // phare av g
    {25, 1,  5.0f,  2.3f,  0.0f, {-1,-1,-1,-1}, 0, 25},  // radiateur
    {26, 0,  0.0f,  1.1f, -2.3f, {-1,-1,-1,-1}, 0, 26},  // marche pied d
    {27, 0,  0.0f,  1.1f,  2.3f, {-1,-1,-1,-1}, 0, 27},  // marche pied g
    {28, 0,  4.5f,  2.0f, -2.5f, {-1,-1,-1,-1}, 0, 28},  // garde-boue av d
    {29, 0,  4.5f,  2.0f,  2.5f, {-1,-1,-1,-1}, 0, 29},  // garde-boue av g
    {30, 0, -4.0f,  2.0f, -2.5f, { 3, 5,-1,-1}, 0, 30},  // garde-boue ar d
    {31, 0, -4.0f,  2.0f,  2.5f, { 2, 4,-1,-1}, 0, 31},  // garde-boue ar g
    {32, 0,  6.6f,  1.1f,  0.0f, {-1,-1,-1,-1}, 0, 32},  // parchoc av
    {33, 0, -6.3f,  1.1f,  0.0f, {-1,-1,-1,-1}, 0, 33},  // parchoc ar
    {34, 2,  0.9f,  3.3f,  0.0f, {-1,-1,-1,-1}, 0, 34},  // tableau de bord
    {-1}
};

RemovePart tableCar08[] =    // torpedo
{
    {10, 0,  1.5f,  4.2f, -0.5f, {-1,-1,-1,-1}, 0, 10},  // capot d
    {11, 0,  1.5f,  4.2f,  0.5f, {-1,-1,-1,-1}, 0, 11},  // capot g
    {12, 1,  5.5f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 12},  // radiateur
    {13, 1, -4.5f,  2.5f, -1.4f, {-1,-1,-1,-1}, 0, 13},  // aile d
    {14, 1, -4.5f,  2.5f,  1.4f, {-1,-1,-1,-1}, 0, 14},  // aile g
    {15, 0, -7.0f,  2.5f,  0.0f, {-1,-1,-1,-1}, 0, 15},  // coffre ar
    {16, 1, -2.1f,  3.0f,  0.0f, {-1,-1,-1,-1}, 0, 16},  // tableau de bord
    {17, 0, -4.2f,  2.5f,  1.8f, {-1,-1,-1,-1}, 0, 17},  // frein à main
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

Math::Point AdjustRot(float x, float y, float angle)
{
    Math::Point    p;
        
    p = Math::RotatePoint(angle, Math::Point(-x,-y));
    return Math::Point(p.x+x, p.y+y);
}



// Constructeur de l'objet.

CMotionCar::CMotionCar(COldObject* object)
    : CMotion(object)
{
    m_model = 0;
    m_posTrackLeft  = 0.0f;
    m_posTrackRight = 0.0f;
    m_partTotal = 0;
    m_partUsed = 0;
    m_topRest = 0;
    m_wheelLastPos   = Math::Vector(0.0f, 0.0f, 0.0f);
    m_wheelLastAngle = Math::Vector(0.0f, 0.0f, 0.0f);
    m_flatTime = 0.0f;
    m_bWater = -1;
    m_bHelico = -1;
    m_lastWheelSpeed[0] = 0.0f;
    m_lastWheelSpeed[1] = 0.0f;

    m_lbAction = false;
    m_lbStart = false;
    m_lbLastParticle = 0.0;

    for ( int i=0 ; i<6 ; i++ )
    {
        m_bLight[i] = true;
    }
    for ( int i=0 ; i<MAX_PART ; i++ )
    {
        m_part[i].bDeleted = true;
    }
}

// Destructeur de l'objet.

CMotionCar::~CMotionCar()
{
}


// Supprime un objet.

void CMotionCar::DeleteObject(bool bAll)
{
}


// Démarre une action spéciale.

Error CMotionCar::SetAction(int action, float time)
{
    if ( action == MV_FLATY )
    {
        CrashVehicle(Math::Vector(1.0f, 0.25f, 1.0f));  // tout plat
        m_flatTime = 10.0f;
        return ERR_OK;
    }

    if ( action == MV_FLATZ )
    {
        CrashVehicle(Math::Vector(1.0f, 1.0f, 0.25f));  // tout mince
        m_flatTime = 10.0f;
        return ERR_OK;
    }

    if ( action == MV_DEFLAT )
    {
        CrashVehicle(Math::Vector(1.0f, 1.0f, 1.0f));
        m_flatTime = 0.0f;
        return ERR_OK;
    }

    if ( action == MV_LOADBOT )
    {
        m_lbAction = true;
        m_lbStart = true;
        m_lbLastParticle = 0.0f;
    }

    return CMotion::SetAction(action, time);
}

// Ecrase un véhicule sous la porte 2 ou 3 (mince ou plat).

bool CMotionCar::CrashVehicle(Math::Vector zoom)
{
    CObject*    fret;
    Math::Matrix*    mat;
    Math::Vector    iZoom, pos, speed;
    Math::Point        dim;
    float        duration;
    int            i;
    bool        bNormal;

    iZoom = m_object->GetScale();
    if ( iZoom.x == zoom.x &&
         iZoom.y == zoom.y &&
         iZoom.z == zoom.z )  return false;

    bNormal = ( zoom.x == 1.0f && zoom.y == 1.0f && zoom.z == 1.0f );

    fret = m_object->GetCargo();
    if ( fret != 0 )  // robot transporté ?
    {
        assert(fret->Implements(ObjectInterfaceType::Transportable));
        mat = m_object->GetWorldMatrix(0);
        pos = Transform(*mat, m_object->GetCharacter()->posFret);
        m_object->SetCargo(0);
        dynamic_cast<CTransportableObject*>(fret)->SetTransporter(0);
        fret->SetPosition(pos);

        m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOT, fret);  // explose le robot transporté
    }

    m_object->SetScale(zoom);

#if 0
    if ( zoom.z != 1.0f )  // mince ?
    {
        zoom = 1.0f/zoom;  // garde les roues normales
    }
    else
    {
        zoom = Math::Vector(1.0f, 1.0f, 1.0f);
    }
    m_object->SetPartScale(3, zoom);
    m_object->SetPartScale(4, zoom);
    m_object->SetPartScale(5, zoom);
    m_object->SetPartScale(6, zoom);
#endif

    if ( bNormal )
    {
        pos = m_engine->GetEyePt();
        m_sound->Play(SOUND_WAYPOINT, pos, 1.0f, 2.0f);
    }
    else
    {
        pos = m_engine->GetEyePt();
        m_sound->Play(SOUND_BOUMm, pos, 1.0f, 0.6f);
    }

    for ( i=0 ; i<50 ; i++ )
    {
        pos = m_object->GetPosition();
        pos.x += (Math::Rand()-0.5f)*10.0f;
        pos.z += (Math::Rand()-0.5f)*10.0f;
        pos.y += Math::Rand()*3.0f;
        speed.x = (Math::Rand()-0.5f)*5.0f;
        speed.z = (Math::Rand()-0.5f)*5.0f;
        speed.y = Math::Rand()*2.0f;
        dim.x = (Math::Rand()*12.0f+12.0f)*(bNormal?0.3f:1.0f);
        dim.y = dim.x;
        duration = 0.4f+Math::Rand()*0.4f;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, duration);
    }
    return true;
}


// Crée un véhicule roulant quelconque posé sur le sol.

void CMotionCar::Create(Math::Vector pos, float angle, ObjectType type, float power, Gfx::COldModelManager* modelManager)
{
    Character*        character;
    int                rank, i;
    char            name[100];

//    if ( m_engine->GetRestCreate() < 1+5+18+1 )  return false;

    m_model = m_object->GetModel();
    m_option = m_object->GetOption();
    m_object->SetType(type);
    CreatePhysics();
    character = m_object->GetCharacter();

    // Crée la base principale.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // c'est un objet mobile
    m_object->SetObjectRank(0, rank);

    if ( type == OBJECT_CAR )
    {
        if ( m_model == 1 )
        {
            modelManager->AddModelReference("buzzingcars/car0100.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 2 )
        {
            modelManager->AddModelReference("buzzingcars/car0200.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 3 )
        {
            modelManager->AddModelReference("buzzingcars/car0300.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 4 )
        {
            modelManager->AddModelReference("buzzingcars/car0400.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 5 )
        {
            modelManager->AddModelReference("buzzingcars/car0500.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 6 )
        {
            modelManager->AddModelReference("buzzingcars/car0600.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 7 )
        {
            modelManager->AddModelReference("buzzingcars/car0700.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 8 )
        {
            modelManager->AddModelReference("buzzingcars/car0800.mod", false, rank, m_object->GetTeam());
        }
        if ( m_model == 99 )
        {
            modelManager->AddModelReference("buzzingcars/car9900.mod", false, rank, m_object->GetTeam());
        }
    }
    if ( type == OBJECT_MOBILEtg )
    {
        modelManager->AddModelReference("buzzingcars/target.mod", false, rank, m_object->GetTeam());
    }
    if ( type == OBJECT_TRAX )
    {
        modelManager->AddModelReference("buzzingcars/trax1.mod", false, rank, m_object->GetTeam());
    }
    if ( type == OBJECT_UFO )
    {
        modelManager->AddModelReference("buzzingcars/ufo00.mod", false, rank, m_object->GetTeam());
    }

    m_object->SetPosition(pos);
    m_object->SetRotationY(angle);

    // Un véhicule doit avoir obligatoirement une sphère de
    // collision avec un centre (0;0;0) (voir GetCrashSphere).
    if ( type == OBJECT_CAR )
    {
        m_object->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        m_object->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 4.0f, 0.0f), 6.0f));
    }
    else if ( type == OBJECT_TRAX )
    {
        m_object->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f));
        // bouton rouge :
        m_object->AddCrashSphere(CrashSphere(Math::Vector(-11.0f, 3.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.55f));
        m_object->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 4.0f, 0.0f), 14.0f));
    }    
    else if ( type == OBJECT_UFO )
    {
        m_object->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
        m_object->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 4.0f, 0.0f), 6.0f));
    }    
    else
    {
        m_object->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.5f, SOUND_BOUMm, 0.45f));
        m_object->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 4.0f, 0.0f), 6.0f));
    }    

    if ( type == OBJECT_CAR && m_model == 1 )  // tijuana taxi ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0101.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.0f, 0.0f));

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0102.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-4.0f, 1.3f, 0.0f));

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/car0107.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, Math::Vector(3.2f, 1.7f, 0.0f));

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("buzzingcars/car0108.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(0.8f, 1.3f, 0.7f));

        // Crée la télécommande.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0109.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, Math::Vector(-0.2f, 1.5f, 0.7f));
        m_object->SetPartRotationZ(9, -5.0f*Math::PI/180.0f);

        CreatePart(modelManager, tableCar01, 0);

//        m_object->CreateShadowCircle(4.0f, 1.0f, D3DSHADOWCAR01);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 2 )  // ford 32 ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0201.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.0f, 0.0f));

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0202.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-5.0f, 1.2f, 0.0f));

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/car0207.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, Math::Vector(3.2f, 2.0f, 0.0f));

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("buzzingcars/car0208.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(-0.2f, 1.1f, 0.8f));

        // Crée la télécommande.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0209.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, Math::Vector(-1.1f, 1.1f, 0.8f));

        CreatePart(modelManager, tableCar02, 0);

//        m_object->CreateShadowCircle(4.8f, 1.0f, D3DSHADOWCAR02);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 3 )  // pickup ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0301.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.0f, 0.0f)*1.1f);

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0302.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-4.0f, 1.1f, 0.0f)*1.1f);

        // Crée l'amortisseur arrière droite.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(38, rank);
        m_object->SetObjectParent(38, 2);
        modelManager->AddModelReference("buzzingcars/car0338.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(38, Math::Vector(0.0f, 0.0f, -1.8f)*1.1f);

        // Crée l'amortisseur arrière gauche.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(39, rank);
        m_object->SetObjectParent(39, 2);
        modelManager->AddModelReference("buzzingcars/car0338.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(39, Math::Vector(0.0f, 0.0f, 1.8f)*1.1f);

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/car0307.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, Math::Vector(2.4f, 2.3f, 0.0f)*1.1f);
        m_object->SetPartRotationZ(7, 10.0f*Math::PI/180.0f);

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("buzzingcars/car0308.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(-0.5f, 1.5f, 0.7f)*1.1f);

        // Crée la télécommande.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0309.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, Math::Vector(-1.3f, 1.5f, 0.7f)*1.1f);

        CreatePart(modelManager, tableCar03, 1.1f);

//        m_object->CreateShadowCircle(4.0f*1.1f, 1.0f, D3DSHADOWCAR03);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 4 )  // firecraker ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0401.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.0f, 0.0f));

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0402.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-5.5f, 1.4f, 0.0f));

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/car0407.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, Math::Vector(3.0f, 2.2f, 0.0f));

        // Crée le support du volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(34, rank);
        m_object->SetObjectParent(34, 0);
        modelManager->AddModelReference("buzzingcars/car0434.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(34, Math::Vector(1.8f, 2.1f, 0.7f));
        m_object->SetPartRotationZ(34, 35.0f*Math::PI/180.0f);

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 34);
        modelManager->AddModelReference("buzzingcars/car0408.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(0.0f, 1.0f, 0.0f));

        // Crée la télécommande.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0409.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, Math::Vector(0.0f, 1.8f, 0.7f));

        CreatePart(modelManager, tableCar04, 0);

//        m_object->CreateShadowCircle(5.0f, 1.0f, D3DSHADOWCAR04);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 5 )  // hooligan ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0501.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.1f, 0.0f));

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0502.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-5.0f, 1.2f, 0.0f));

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/car0507.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, Math::Vector(3.2f, 2.0f, 0.0f));

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("buzzingcars/car0508.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(-0.2f, 1.2f, 0.8f));

        // Crée la télécommande.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0509.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, Math::Vector(-1.1f, 1.2f, 0.8f));

        CreatePart(modelManager, tableCar05, 0);

//        m_object->CreateShadowCircle(5.0f, 1.0f, D3DSHADOWCAR05);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 6 )  // chevy ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0601.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.0f, 0.0f));

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0602.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-4.0f, 1.1f, 0.0f));

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        if ( true /*m_engine->GetSuperDetail()*/ )
        {
            modelManager->AddModelReference("buzzingcars/car0607h.mod", false, rank, m_object->GetTeam());
        }
        else
        {
            modelManager->AddModelReference("buzzingcars/car0607.mod", false, rank, m_object->GetTeam());
        }
        m_object->SetPartPosition(7, Math::Vector(4.2f, 2.3f, 0.0f));

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0609.mod", false, rank, m_object->GetTeam());  // objet inexistant
        m_object->SetPartPosition(9, Math::Vector(2.1f, 2.9f, 1.0f));
        m_object->SetPartRotationZ(9, 45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 9);
        modelManager->AddModelReference("buzzingcars/car0608.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(0.0f, 0.0f, 0.0f));

        CreatePart(modelManager, tableCar06, 0);

//        m_object->CreateShadowCircle(4.5f, 1.0f, D3DSHADOWCAR06);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 7 )  // reo ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0701.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.0f, 0.0f)*1.1f);

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0702.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-4.0f, 1.1f, 0.0f)*1.1f);

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/car0707.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, Math::Vector(3.2f, 2.2f, 0.0f)*1.1f);

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("buzzingcars/car0708.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(0.3f, 1.5f, 0.8f)*1.1f);

        // Crée la télécommande.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0709.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, Math::Vector(-0.6f, 1.5f, 0.8f)*1.1f);

        CreatePart(modelManager, tableCar07, 1.1f);

//        m_object->CreateShadowCircle(4.5f*1.1f, 1.0f, D3DSHADOWCAR07);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 8 )  // torpedo ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car0801.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(6.3f, 1.0f, 0.0f)*1.1f);

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car0802.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-4.0f, 1.1f, 0.0f)*1.1f);

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        // Crée le moteur.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/car0807.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, Math::Vector(1.5f, 2.2f, 0.0f)*1.1f);

        // Crée le volant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("buzzingcars/car0808.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, Math::Vector(-2.0f, 4.0f, 0.0f)*1.1f);

        // Crée la télécommande.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/car0809.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, Math::Vector(-6.2f, 1.5f, 0.0f)*1.1f);

        CreatePart(modelManager, tableCar08, 1.1f);

//        m_object->CreateShadowCircle(5.0f*1.1f, 1.0f, D3DSHADOWCAR08);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_CAR && m_model == 99 )  // fantome ?
    {
        // Crée l'axe avant.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/car9901.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(5.0f, 1.0f, 0.0f)*1.1f);

        // Crée l'axe arrière.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/car9902.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-4.0f, 1.1f, 0.0f)*1.1f);

        // Crée les roues.
        CreateWheel(modelManager, m_nameObjWheel);

        m_object->SetFloorHeight(0.0f);
    }

    CObjectManager* objMan = CObjectManager::GetInstancePointer();

    if ( type == OBJECT_CAR && m_option == 1 )  // lance incendie ?
    {
        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, OBJECT_FIRE));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        bot->SetPosition(character->posFret);
        bot->SetRotation(character->angleFret);
    }

    if ( type == OBJECT_CAR && m_option == 2 )  // helicoptère ?
    {
        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, OBJECT_HELICO));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        bot->SetPosition(character->posFret);
        bot->SetRotation(character->angleFret);
    }

    if ( type == OBJECT_CAR && m_option == 3 )  // boussole ?
    {
        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, OBJECT_COMPASS));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        bot->SetPosition(character->posFret);
        bot->SetRotation(character->angleFret);
    }

    if ( type == OBJECT_CAR && m_option == 4 )  // électrocuteur ?
    {
        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, OBJECT_BLITZER));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        bot->SetPosition(character->posFret);
        bot->SetRotation(character->angleFret);
    }

    if ( type == OBJECT_CAR && m_option == 5 )  // crochet ?
    {
        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, OBJECT_HOOK));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        bot->SetPosition(character->posFret);
        bot->SetRotation(character->angleFret);
    }

    if ( type == OBJECT_CAR && m_option >= 11 && m_option <= 20 )
    {
        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, static_cast<ObjectType>(OBJECT_BOT1+m_option-11)));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        pos = character->posFret;
        pos.y += bot->GetCharacter()->height;
        bot->SetPosition(pos);
        bot->SetRotation(character->angleFret);
    }

    if ( type == OBJECT_MOBILEtg )
    {
        // Crée la roue arrière-droite.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelReference("buzzingcars/lem2w.mod", false, rank, m_object->GetTeam());
    
        // Crée la roue arrière-gauche.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("buzzingcars/lem2w.mod", false, rank, m_object->GetTeam());
        m_object->SetPartRotationY(7, Math::PI);

        // Crée la roue avant-droite.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("buzzingcars/lem2w.mod", false, rank, m_object->GetTeam());
    
        // Crée la roue avant-gauche.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("buzzingcars/lem2w.mod", false, rank, m_object->GetTeam());
        m_object->SetPartRotationY(9, Math::PI);

        m_object->CreateShadowCircle(4.0f, 1.0f);
        m_object->SetFloorHeight(0.0f);
    }

    if ( type == OBJECT_TRAX )
    {
        // Crée la chenille droite.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/trax2.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, Math::Vector(-1.0f, 3.0f, -4.0f));

        // Crée la chenille gauche.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/trax3.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, Math::Vector(-1.0f, 3.0f, 4.0f));

        // Crée la pelle.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(3, rank);
        m_object->SetObjectParent(3, 0);
        modelManager->AddModelReference("buzzingcars/trax4.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(3, Math::Vector(-1.0f, 4.0f, 0.0f));

        // Crée le levier droite.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(4, rank);
        m_object->SetObjectParent(4, 0);
        modelManager->AddModelReference("buzzingcars/trax5.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(4, Math::Vector(-3.5f, 8.0f, -3.0f));

        // Crée le levier gauche.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(5, rank);
        m_object->SetObjectParent(5, 0);
        modelManager->AddModelReference("buzzingcars/trax5.mod", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(5, Math::Vector(-3.5f, 8.0f, 3.0f));

        m_object->CreateShadowCircle(10.0f, 1.0f);
        m_object->SetFloorHeight(0.0f);

        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, OBJECT_EVIL2));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        bot->SetPosition(Math::Vector(-6.0f, 9.0f, 0.0f));
    }

    if ( type == OBJECT_UFO )
    {
        for ( i=1 ; i<=18 ; i++ )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(i, rank);
            m_object->SetObjectParent(i, 0);
            sprintf(name, "buzzingcars/ufo%.2d.mod", i);
            modelManager->AddModelReference(name, false, rank, m_object->GetTeam());
            m_object->SetPartPosition(i, Math::Vector(0.0f, 0.0f, 0.0f));
        }

        COldObject* bot = dynamic_cast<COldObject*>(objMan->CreateObject(pos, 0.0f, OBJECT_EVIL2));
        bot->SetTransporter(m_object);
        m_object->SetCargo(bot);
        bot->SetPosition(character->posFret);
        bot->SetRotation(character->angleFret);

        m_object->CreateShadowCircle(4.0f, 1.0f);
    }

    pos = m_object->GetPosition();
    m_object->SetPosition(pos);  // pour afficher les ombres tout de suite

//TODO (krzys_h):    StateVehicle* sv = (StateVehicle*)m_main->GetCarState();
    if ( true /*sv == 0*/ )
    {
        TwistInit();
    }
    else
    {
//        TwistInit(sv);
    }

    m_engine->LoadAllTextures();
}

// Crée les roues.

void CMotionCar::CreateWheel(Gfx::COldModelManager* modelManager, char* name)
{
    Character*    character;
    Math::Vector    zoom;
    int            rank;

    character = m_object->GetCharacter();

    zoom.x = character->wheelFrontDim;
    zoom.y = character->wheelFrontDim;
    zoom.z = character->wheelFrontWidth;

    // Crée la roue avant-droite.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 1);
    modelManager->AddModelReference(name, false, rank, m_object->GetTeam());
    m_object->SetPartPosition(3, Math::Vector(0.0f, 0.0f, -character->wheelFrontPos.z));
    m_object->SetPartScale(3, zoom);

    // Crée la roue avant-gauche.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 1);
    modelManager->AddModelReference(name, false, rank, m_object->GetTeam());
    m_object->SetPartPosition(4, Math::Vector(0.0f, 0.0f, character->wheelFrontPos.z));
    m_object->SetPartScale(4, zoom);
    m_object->SetPartRotationY(4, Math::PI);

    zoom.x = character->wheelBackDim;
    zoom.y = character->wheelBackDim;
    zoom.z = character->wheelBackWidth;

    // Crée la roue arrière-droite.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 2);
    modelManager->AddModelReference(name, false, rank, m_object->GetTeam());
    m_object->SetPartPosition(5, Math::Vector(0.0f, 0.0f, -character->wheelBackPos.z));
    m_object->SetPartScale(5, zoom);

    // Crée la roue arrière-gauche.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 2);
    modelManager->AddModelReference(name, false, rank, m_object->GetTeam());
    m_object->SetPartPosition(6, Math::Vector(0.0f, 0.0f, character->wheelBackPos.z));
    m_object->SetPartScale(6, zoom);
    m_object->SetPartRotationY(6, Math::PI);
}

// Crée toutes les parties mobiles du véhicule.

void CMotionCar::CreatePart(Gfx::COldModelManager* modelManager, RemovePart* table, float zoom)
{
    Math::Vector        pos;
    int                i, j, rank;
    char            name[50];

//TODO (krzys_h):    StateVehicle* sv = (StateVehicle*)m_main->GetCarState();
    if(zoom == 0) zoom = 1; // TODO: check

    m_partTotal = 0;
    for ( i=0 ; i<MAX_PART ; i++ )
    {
        if ( table[i].part == -1 )  break;
//TODO (krzys_h):        if ( sv != nullptr && sv->state[i].bDeleted )  continue;

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(table[i].part, rank);
        m_object->SetObjectParent(table[i].part, 0);

        if ( true /*m_engine->GetSuperDetail()*/ )
        {
            sprintf(name, "buzzingcars/car%.2d%.2dh.mod", m_model, table[i].file);
            if ( !modelManager->AddModelReference(name, false, rank, m_object->GetTeam()) )
            {
                sprintf(name, "buzzingcars/car%.2d%.2d.mod", m_model, table[i].file);
                modelManager->AddModelReference(name, false, rank, m_object->GetTeam());
            }
        }
        else
        {
            sprintf(name, "buzzingcars/car%.2d%.2d.mod", m_model, table[i].file);
            modelManager->AddModelReference(name, false, rank, m_object->GetTeam());
        }
        pos.x = table[i].x*zoom;
        pos.y = table[i].y*zoom;
        pos.z = table[i].z*zoom;
        m_object->SetPartPosition(table[i].part, pos);

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

        m_part[i].bDeleted = false;
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

void CMotionCar::CreatePhysics()
{
    Character*    character;
    CObject*    helico;
    ObjectType    type;
    float        factor, radius, water, susp, mass, kid;

    character = m_object->GetCharacter();
    type = m_object->GetType();

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

        if ( m_main->GetFixScene() )
        {
            susp = 0.5f;
        }
        else
        {
            susp = 1.0f;
        }

        mass = 1.0f;
        helico = m_object->GetCargo();
        if ( helico != 0 && helico->GetType() == OBJECT_HELICO )
        {
            mass = 0.2f;
        }

        kid = 1.0f;
        if ( m_main->GetSelectedDifficulty() == 1 )  // niveau CN ?
        {
            kid = 0.7f;
            mass *= 0.5f;
        }
    }

    if ( type == OBJECT_CAR && m_model == 1 )  // tijuana taxi ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.0f, 3.0f);
        character->wheelBackPos   = Math::Vector(-4.0f, 1.3f, 2.7f);
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
        character->maxRolling     = 20.0f*Math::PI/180.0f;
        character->maxNicking     = 15.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 3.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 70.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-2.0f, 1.6f, 0.0f);
        character->angleFret      = Math::Vector(0.0f, 0.0f, -5.0f*Math::PI/180.0f);
        character->antenna        = Math::Vector(0.35f, 5.1f, 1.15f);
        character->lightFL        = Math::Vector(5.2f, 2.25f,  1.9f);
        character->lightFR        = Math::Vector(5.2f, 2.25f, -1.9f);
        character->lightSL        = Math::Vector(-4.6f, 3.9f,  2.0f);
        character->lightSR        = Math::Vector(-4.6f, 3.9f, -2.0f);
        character->lightRL        = Math::Vector(-4.6f, 3.3f,  2.0f);
        character->lightRR        = Math::Vector(-4.6f, 3.3f, -2.0f);
        character->camera         = Math::Vector( 1.9f, 4.1f, 0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0103.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0103b.mod");
        strcpy(m_nameTexWheel, "wheel01.tga");
    }

    if ( type == OBJECT_CAR && m_model == 2 )  // ford 32 ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.0f, 2.7f);
        character->wheelBackPos   = Math::Vector(-5.0f, 1.2f, 2.5f);
        character->wheelFrontDim  = 1.0f*radius;
        character->wheelBackDim   = 1.2f*radius;
        character->wheelFrontWidth= 1.0f;
        character->wheelBackWidth = 1.4f;
        character->crashFront     = 6.0f;
        character->crashBack      =-6.5f;
        character->crashWidth     = 4.0f;
        character->suspDetect     = 2.0f;
//?        character->suspHeight     = 0.5f*susp;
        character->suspHeight     = 1.0f*susp;
        character->suspFrequency  = 8.0f;
        character->suspAbsorber   = 1.0f;
        character->rolling        = 0.8f*factor;
        character->nicking        = 0.10f*factor;
        character->maxRolling     = 20.0f*Math::PI/180.0f;
        character->maxNicking     = 15.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 3.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 80.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-5.2f, 2.2f, 0.0f);
        character->angleFret      = Math::Vector(0.0f, 0.0f, -5.0f*Math::PI/180.0f);
        character->antenna        = Math::Vector(-0.75f, 4.55f, 1.34f);
        character->lightFL        = Math::Vector(6.2f, 2.8f,  1.4f);
        character->lightFR        = Math::Vector(6.2f, 2.8f, -1.4f);
        character->lightSL        = Math::Vector(-6.4f, 3.2f,  2.5f);
        character->lightSR        = Math::Vector(-6.4f, 3.2f, -2.5f);
        character->lightRL        = Math::Vector(-6.4f, 2.6f,  2.5f);
        character->lightRR        = Math::Vector(-6.4f, 2.6f, -2.5f);
        character->camera         = Math::Vector( 0.9f, 4.9f, 0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0203.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0203b.mod");
        strcpy(m_nameTexWheel, "wheel02.tga");
    }

    if ( type == OBJECT_CAR && m_model == 3 )  // pickup ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.0f, 2.5f)*1.1f;
        character->wheelBackPos   = Math::Vector(-4.0f, 1.1f, 2.5f)*1.1f;
        character->wheelFrontDim  = 1.0f*1.1f*radius;
        character->wheelBackDim   = 1.1f*1.1f*radius;
        character->wheelFrontWidth= 1.0f*1.1f;
        character->wheelBackWidth = 1.2f*1.1f;
        character->crashFront     = 6.0f*1.1f;
        character->crashBack      =-6.0f*1.1f;
        character->crashWidth     = 3.5f*1.1f;
        character->suspDetect     = 2.0f;
//?        character->suspHeight     = 0.7f*susp;
        character->suspHeight     = 1.0f*susp;
        character->suspFrequency  = 8.0f;
        character->suspAbsorber   = 1.0f;
        character->rolling        = 0.8f*factor;
        character->nicking        = 0.15f*factor;
        character->maxRolling     = 20.0f*Math::PI/180.0f;
        character->maxNicking     = 15.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 3.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 80.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-4.5f, 2.6f, 0.0f)*1.1f;
        character->angleFret      = Math::Vector(0.0f, 0.0f,  0.0f);
        character->antenna        = Math::Vector(-0.99f, 4.51f, 1.15f)*1.1f;
        character->lightFL        = Math::Vector(5.2f, 2.2f,  1.4f)*1.1f;
        character->lightFR        = Math::Vector(5.2f, 2.2f, -1.4f)*1.1f;
        character->lightSL        = Math::Vector(-5.7f, 2.5f,  2.7f)*1.1f;
        character->lightSR        = Math::Vector(-5.7f, 2.5f, -2.7f)*1.1f;
        character->lightRL        = Math::Vector(-5.7f, 2.5f,  2.3f)*1.1f;
        character->lightRR        = Math::Vector(-5.7f, 2.5f, -2.3f)*1.1f;
        character->camera         = Math::Vector( 1.0f, 5.1f, 0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0303.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0303b.mod");
        strcpy(m_nameTexWheel, "wheel03.tga");
    }

    if ( type == OBJECT_CAR && m_model == 4 )  // firecraker ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.0f, 2.7f);
        character->wheelBackPos   = Math::Vector(-5.5f, 1.4f, 2.7f);
        character->wheelFrontDim  = 1.0f*radius;
        character->wheelBackDim   = 1.4f*radius;
        character->wheelFrontWidth= 1.3f;
        character->wheelBackWidth = 1.7f;
        character->crashFront     = 6.5f;
        character->crashBack      =-7.5f;
        character->crashWidth     = 4.0f;
        character->suspDetect     = 1.5f;
//?        character->suspHeight     = 0.3f*susp;
        character->suspHeight     = 0.7f*susp;
        character->suspFrequency  = 12.0f;
        character->suspAbsorber   = 2.0f;
        character->rolling        = 0.5f*factor;
        character->nicking        = 0.10f*factor;
        character->maxRolling     = 15.0f*Math::PI/180.0f;
        character->maxNicking     = 10.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 2.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 60.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-4.5f, 3.9f, 0.0f);
        character->angleFret      = Math::Vector(0.0f, 0.0f, -10.0f*Math::PI/180.0f);
        character->antenna        = Math::Vector(0.6f, 4.9f, 1.15f);
        character->lightFL        = Math::Vector(5.6f, 2.1f,  1.4f);
        character->lightFR        = Math::Vector(5.6f, 2.1f, -1.4f);
        character->lightSL        = Math::Vector(-7.2f, 3.0f,  3.0f);
        character->lightSR        = Math::Vector(-7.2f, 3.0f, -3.0f);
        character->lightRL        = Math::Vector(-7.2f, 3.0f,  2.3f);
        character->lightRR        = Math::Vector(-7.2f, 3.0f, -2.3f);
        character->camera         = Math::Vector( 2.1f, 4.6f, 0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0403.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0403b.mod");
        strcpy(m_nameTexWheel, "wheel04.tga");
    }

    if ( type == OBJECT_CAR && m_model == 5 )  // hooligan ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.1f, 2.7f);
        character->wheelBackPos   = Math::Vector(-5.0f, 1.2f, 2.5f);
        character->wheelFrontDim  = 1.1f*radius;
        character->wheelBackDim   = 1.2f*radius;
        character->wheelFrontWidth= 1.3f;
        character->wheelBackWidth = 1.5f;
        character->crashFront     = 6.0f;
        character->crashBack      =-6.5f;
        character->crashWidth     = 4.1f;
        character->suspDetect     = 2.0f;
//?        character->suspHeight     = 0.5f*susp;
        character->suspHeight     = 0.7f*susp;
        character->suspFrequency  = 8.0f;
        character->suspAbsorber   = 1.0f;
        character->rolling        = 0.8f*factor;
        character->nicking        = 0.15f*factor;
        character->maxRolling     = 20.0f*Math::PI/180.0f;
        character->maxNicking     = 15.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 3.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 70.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-4.1f, 2.4f, 0.0f);
        character->angleFret      = Math::Vector(0.0f, 0.0f,  0.0f);
        character->antenna        = Math::Vector(-0.81f, 4.60f, 1.32f);
        character->lightFL        = Math::Vector(5.2f, 3.4f,  1.6f);
        character->lightFR        = Math::Vector(5.2f, 3.4f, -1.6f);
        character->lightSL        = Math::Vector(-6.0f, 3.0f,  3.2f);
        character->lightSR        = Math::Vector(-6.0f, 3.0f, -3.2f);
        character->lightRL        = Math::Vector(-6.0f, 3.0f,  2.6f);
        character->lightRR        = Math::Vector(-6.0f, 3.0f, -2.6f);
        character->camera         = Math::Vector( 0.7f, 5.0f, 0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0503.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0503b.mod");
        strcpy(m_nameTexWheel, "wheel05.tga");
    }

    if ( type == OBJECT_CAR && m_model == 6 )  // chevy ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.0f, 2.5f);
        character->wheelBackPos   = Math::Vector(-4.0f, 1.1f, 2.5f);
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
        character->maxRolling     = 20.0f*Math::PI/180.0f;
        character->maxNicking     = 15.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 3.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 80.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-0.5f, 1.5f,  0.0f);
        character->angleFret      = Math::Vector( 0.0f, 0.0f,  0.0f);
        character->antenna        = Math::Vector(-5.0f, 4.9f,  2.1f);
        character->lightFL        = Math::Vector( 7.1f, 2.8f,  2.4f);
        character->lightFR        = Math::Vector( 7.1f, 2.8f, -2.4f);
        character->lightSL        = Math::Vector(-7.8f, 3.0f,  2.7f);
        character->lightSR        = Math::Vector(-7.8f, 3.0f, -2.7f);
        character->lightRL        = Math::Vector(-7.9f, 3.3f,  2.7f);
        character->lightRR        = Math::Vector(-7.9f, 3.3f, -2.7f);
        character->camera         = Math::Vector( 2.1f, 4.9f,  0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0603.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0603b.mod");
        strcpy(m_nameTexWheel, "wheel06.tga");
    }

    if ( type == OBJECT_CAR && m_model == 7 )  // reo ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.0f, 2.5f)*1.1f;
        character->wheelBackPos   = Math::Vector(-4.0f, 1.1f, 2.5f)*1.1f;
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
        character->maxRolling     = 15.0f*Math::PI/180.0f;
        character->maxNicking     = 10.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 3.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 80.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-2.5f, 1.5f, 0.0f)*1.1f;
        character->angleFret      = Math::Vector(0.0f, 0.0f,  0.0f);
        character->antenna        = Math::Vector(-0.27f, 4.51f, 1.21f)*1.1f;
        character->lightFL        = Math::Vector(5.4f, 2.8f,  1.4f)*1.1f;
        character->lightFR        = Math::Vector(5.4f, 2.8f, -1.4f)*1.1f;
        character->lightSL        = Math::Vector(-5.5f, 2.3f,  2.5f)*1.1f;
        character->lightSR        = Math::Vector(-5.5f, 2.3f, -2.5f)*1.1f;
        character->lightRL        = Math::Vector(-5.8f, 1.8f,  2.5f)*1.1f;
        character->lightRR        = Math::Vector(-5.8f, 1.8f, -2.5f)*1.1f;
        character->camera         = Math::Vector( 1.0f, 5.3f, 0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0703.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0703b.mod");
        strcpy(m_nameTexWheel, "wheel07.tga");
    }

    if ( type == OBJECT_CAR && m_model == 8 )  // torpedo ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 6.3f, 1.0f, 2.5f)*1.1f;
        character->wheelBackPos   = Math::Vector(-4.0f, 1.1f, 2.5f)*1.1f;
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
        character->maxRolling     = 15.0f*Math::PI/180.0f;
        character->maxNicking     = 10.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 2.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f*mass;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 50.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-4.2f, 1.5f, 0.0f)*1.1f;
        character->angleFret      = Math::Vector(0.0f, 0.0f, 0.0f);
        character->antenna        = Math::Vector(-6.51f, 5.21f, 0.53f)*1.1f;
        character->lightFL        = Math::Vector(0.0f, 2.0f, 0.0f)*1.1f;
        character->lightFR        = Math::Vector(0.0f, 2.0f, 0.0f)*1.1f;
        character->lightSL        = Math::Vector(0.0f, 2.0f, 0.0f)*1.1f;
        character->lightSR        = Math::Vector(0.0f, 2.0f, 0.0f)*1.1f;
        character->lightRL        = Math::Vector(0.0f, 2.0f, 0.0f)*1.1f;
        character->lightRR        = Math::Vector(0.0f, 2.0f, 0.0f)*1.1f;
        character->camera         = Math::Vector(-4.0f, 5.0f, 0.0f)*1.1f;
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car0803.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car0803b.mod");
        strcpy(m_nameTexWheel, "wheel08.tga");
    }

    if ( type == OBJECT_CAR && m_model == 99 )  // fantome ?
    {
        m_physics->SetType(TYPE_RACE);

        character->wheelFrontPos  = Math::Vector( 5.0f, 1.0f, 2.5f)*1.1f;
        character->wheelBackPos   = Math::Vector(-4.0f, 1.1f, 2.5f)*1.1f;
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
        character->maxRolling     = 15.0f*Math::PI/180.0f;
        character->maxNicking     = 10.0f*Math::PI/180.0f;
        character->overProp       = 5.0f;
        character->overFactor     = 1.0f;
        character->overAngle      = 30.0f*Math::PI/180.0f;
        character->overMul        = 3.0f;
        character->brakeDir       = 0.1f;
        character->mass           = 1500.0f;
        character->turnSlide      = 2.0f;
        character->gripLimit      = 0.3f;
        character->gripSlide      = 80.0f*factor*water;
        character->accelLow       = 1.5f;
        character->accelHigh      = 0.2f;
        character->accelSmooth    = 1.5f;
        character->posFret        = Math::Vector(-2.5f, 1.5f, 0.0f)*1.1f;
        character->angleFret      = Math::Vector(0.0f, 0.0f,  0.0f);
        character->antenna        = Math::Vector(-0.27f, 4.51f, 1.21f)*1.1f;
        character->lightFL        = Math::Vector(5.4f, 2.8f,  1.4f)*1.1f;
        character->lightFR        = Math::Vector(5.4f, 2.8f, -1.4f)*1.1f;
        character->lightSL        = Math::Vector(-5.5f, 2.3f,  2.5f)*1.1f;
        character->lightSR        = Math::Vector(-5.5f, 2.3f, -2.5f)*1.1f;
        character->lightRL        = Math::Vector(-5.8f, 1.8f,  2.5f)*1.1f;
        character->lightRR        = Math::Vector(-5.8f, 1.8f, -2.5f)*1.1f;
        character->camera         = Math::Vector( 1.0f, 5.3f, 0.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);

        strcpy(m_nameObjWheel, "buzzingcars/car9903.mod");
        strcpy(m_nameObjWBurn, "buzzingcars/car9903.mod");
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

        character->wheelFrontPos = Math::Vector( 4.0f, 1.0f, 4.0f);
        character->wheelBackPos  = Math::Vector(-3.0f, 1.0f, 4.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_RECACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_STOACCEL, 15.0f);
    }

    if ( type == OBJECT_TRAX )
    {
        m_physics->SetType(TYPE_TANK);

        character->wheelFrontPos = Math::Vector( 4.0f, 1.0f, 4.0f);
        character->wheelBackPos  = Math::Vector(-3.0f, 1.0f, 4.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.2f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.2f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  5.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  5.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  8.0f);
    }

    if ( type == OBJECT_UFO )
    {
        m_physics->SetType(TYPE_TANK);

        character->wheelFrontPos = Math::Vector( 6.0f, 3.0f, 7.0f);
        character->wheelBackPos  = Math::Vector(-8.0f, 3.0f, 7.0f);
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

        m_physics->SetCirMotionY(MO_ADVSPEED,  2.0f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  2.0f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
        m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
    }

    if ( character->crashWidth != 0.0f )
    {
        m_object->AddCrashLine(CrashLine(Math::Point(character->crashBack,  -character->crashWidth), SOUND_BOUMm, 0.45f));
        m_object->AddCrashLine(CrashLine(Math::Point(character->crashFront, -character->crashWidth), SOUND_BOUMm, 0.45f));
        m_object->AddCrashLine(CrashLine(Math::Point(character->crashFront,  character->crashWidth), SOUND_BOUMm, 0.45f));
        m_object->AddCrashLine(CrashLine(Math::Point(character->crashBack,   character->crashWidth), SOUND_BOUMm, 0.45f));
        m_object->AddCrashLine(CrashLine(Math::Point(character->crashBack,  -character->crashWidth), SOUND_BOUMm, 0.45f));
    }
}

// Indique si le véhicule est sous l'eau.

bool CMotionCar::IsUnderWater()
{
    Math::Vector    pos;

    pos = m_object->GetPosition();
    return ( pos.y+2.0f < m_water->GetLevel() );
}


// Management of an event.

bool CMotionCar::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    return true;
}

// Management of an event.

bool CMotionCar::EventFrame(const Event &event)
{
    Math::Matrix*    mat;
    Character*    character;
    CObject*    helico;
    Math::Vector    pos, angle, floor;
    Math::Point        rot;
    ObjectType    type;
    float        s, a, d, hl, hr, aLeft, aRight, progress, max, limit[2];
    float        a1, a2;
    bool        bWater, bHelico;

    if ( m_engine->GetPause() )  return true;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition()) )  return true;

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

    bHelico = false;
    helico = m_object->GetCargo();
    if ( helico != 0 && helico->GetType() == OBJECT_HELICO )
    {
        bHelico = true;
    }
    if ( bHelico != m_bHelico )
    {
        m_bHelico = bHelico;
        CreatePhysics();  // adapte la physique
    }

    type = m_object->GetType();
    character = m_object->GetCharacter();
    mat = m_object->GetWorldMatrix(0);

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
        s = m_physics->GetWheelSpeed(true);  // roues avants
        UpdateWheelMapping(s, true);
        m_object->SetPartRotationZ(3, m_object->GetPartRotationZ(3)+event.rTime*-s);
        m_object->SetPartRotationZ(4, m_object->GetPartRotationZ(4)+event.rTime* s);
        s = m_physics->GetWheelSpeed(false);  // roues arrières
        UpdateWheelMapping(s, false);
        m_object->SetPartRotationZ(5, m_object->GetPartRotationZ(5)+event.rTime*-s);
        m_object->SetPartRotationZ(6, m_object->GetPartRotationZ(6)+event.rTime* s);

        a = m_physics->GetMotorSpeedZ()*8.0f;
        aRight = a*0.07f;
        aLeft  = a*0.07f+Math::PI;
        m_object->SetPartRotationY(3, m_object->GetPartRotationY(3)+(aRight-m_object->GetPartRotationY(3))*event.rTime*8.0f);
        m_object->SetPartRotationY(4, m_object->GetPartRotationY(4)+(aLeft-m_object->GetPartRotationY(4))*event.rTime*8.0f);

        // Tourne le volant.
        if ( m_model == 8 )  // torpedo ?
        {
            m_object->SetPartRotationX(8, m_object->GetPartRotationX(8)+(a*0.5f-m_object->GetPartRotationX(8))*event.rTime*8.0f);
        }
        else
        {
            m_object->SetPartRotationY(8, m_object->GetPartRotationY(8)+(a*0.5f-m_object->GetPartRotationY(8))*event.rTime*8.0f);
        }

        pos = m_object->GetPosition();
        angle = m_object->GetRotation();

        if ( Math::Distance(pos, m_engine->GetEyePt()) < 100.0f &&  // suspension ?
             !m_object->IsDying() )
        {
            max = character->suspHeight;
            if ( m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )  max *= 0.5f;

            pos.x =  character->wheelFrontPos.x;  // roue avant droite
            pos.z = -character->wheelFrontPos.z;
            pos.y = -character->height;
            pos = Transform(*mat, pos);
            hr = m_terrain->GetHeightToFloor(pos);
            if ( hr >  max )  hr =  max;
            if ( hr < -max )  hr = -max;

            pos.x =  character->wheelFrontPos.x;  // roue avant gauche
            pos.z =  character->wheelFrontPos.z;
            pos.y = -character->height;
            pos = Transform(*mat, pos);
            hl = m_terrain->GetHeightToFloor(pos);
            if ( hl >  max )  hl =  max;
            if ( hl < -max )  hl = -max;

            CalcInclinaison(hl, hr, character->wheelFrontPos.z, a, d);
            m_object->SetPartRotationX(1, a);  // axe avant
            pos = m_object->GetPartPosition(1);
            pos.y = character->wheelFrontPos.y+d;
            m_object->SetPartPosition(1, pos);

            pos.x =  character->wheelBackPos.x;  // roue arrière droite
            pos.z = -character->wheelBackPos.z;
            pos.y = -character->height;
            pos = Transform(*mat, pos);
            hr = m_terrain->GetHeightToFloor(pos);
            if ( hr >  max )  hr =  max;
            if ( hr < -max )  hr = -max;

            pos.x =  character->wheelBackPos.x;  // roue arrière gauche
            pos.z =  character->wheelBackPos.z;
            pos.y = -character->height;
            pos = Transform(*mat, pos);
            hl = m_terrain->GetHeightToFloor(pos);
            if ( hl >  max )  hl =  max;
            if ( hl < -max )  hl = -max;

            CalcInclinaison(hl, hr, character->wheelBackPos.z, a, d);
            m_object->SetPartRotationX(2, a);  // axe avant
            pos = m_object->GetPartPosition(2);
            pos.y = character->wheelBackPos.y+d;
            m_object->SetPartPosition(2, pos);

            if ( m_model == 3 )
            {
                m_object->SetPartRotationX(38, -a+0.3f);  // amortisseur droite
                m_object->SetPartRotationX(39, -a-0.3f);  // amortisseur gauche
//?                m_object->SetPartScaleY(38, 0.8f-d*0.67f+hr*0.3f);
//?                m_object->SetPartScaleY(39, 0.8f-d*0.67f+hl*0.3f);
                m_object->SetPartScaleY(38, 1.0f-d*0.35f+hr*0.5f);
                m_object->SetPartScaleY(39, 1.0f-d*0.35f+hl*0.5f);
            }
        }
        else
        {
            m_object->SetPartRotationX(1, 0.0f);  // axe avant
            m_object->SetPartRotationX(2, 0.0f);  // axe arrière
        }

        if ( m_model == 1 )
        {
            UpdateGlassMapping(15, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // av
            UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // ar
        }
        if ( m_model == 2 )
        {
            UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // ar
            UpdateGlassMapping(14, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // av
        }
        if ( m_model == 3 )
        {
            UpdateGlassMapping(14, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // av
            UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // ar
        }
        if ( m_model == 4 )
        {
            UpdateGlassMapping(10, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // av
        }
        if ( m_model == 5 )
        {
            UpdateGlassMapping(10, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1Z);  // av
        }
        if ( m_model == 6 )
        {
            UpdateGlassMapping(11, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1Z, true);  // av
            UpdateGlassMapping(12, -5.0f, 5.0f, -1.0f, Gfx::ENG_TEX_MAPPING_1Z, true);  // ar
        }
        if ( m_model == 7 )
        {
            UpdateGlassMapping(16, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1Z, true);  // av
            UpdateGlassMapping(11, -5.0f, 5.0f, -1.0f, Gfx::ENG_TEX_MAPPING_1Z, true);  // ar
//?            UpdateGlassMapping(12, -5.0f, 5.0f,  1.0f, Gfx::ENG_TEX_MAPPING_1X, true);  // g
//?            UpdateGlassMapping(13, -5.0f, 5.0f, -1.0f, Gfx::ENG_TEX_MAPPING_1X, true);  // d
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

                a = progress*Math::PI*1.4f;
                m_object->SetPartRotationZ(10, a);  // toît
                rot = AdjustRot(-3.1f, 0.4f, a);
                pos = Math::Vector(-1.0f+rot.x, 6.0f+rot.y, 0.0f);
                m_object->SetPartPosition(10, pos);

                a = progress*Math::PI*0.4f;
                m_object->SetPartRotationX(12, a);  // fenêtre gauche
                rot = AdjustRot(0.0f, -0.8f, -a);
                pos = Math::Vector(-3.2f, 5.0f+rot.y, 2.5f+rot.x);
                m_object->SetPartPosition(12, pos);

                m_object->SetPartRotationX(13, -a);  // fenêtre droite
                rot = AdjustRot(0.0f, -0.8f, a);
                pos = Math::Vector(-3.2f, 5.0f+rot.y, -2.5f+rot.x);
                m_object->SetPartPosition(13, pos);

                a = progress*Math::PI*0.2f;
                m_object->SetPartRotationZ(14, -a);  // montant central
                rot = AdjustRot(0.0f, -1.0f, -a);
                pos = Math::Vector(-0.7f+rot.x, 5.0f+rot.y, 0.0f);
                m_object->SetPartPosition(14, pos);

                if ( m_actionType == MV_CLOSE && m_progress == 1.0f )
                {
                    m_sound->Play(SOUND_CLOSE, m_object->GetPosition(), 0.7f, 1.8f);
                }

                if ( m_progress >= 1.0f )
                {
                    SetAction(-1);
                }
            }
            else if ( m_model == 2 )  // ford 32 ?
            {
                if ( m_object->GetCargo() != 0 )
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
                        m_sound->Play(SOUND_CLOSE, m_object->GetPosition(), 0.7f, 1.8f);
                    }

                    if ( m_progress >= 1.0f )
                    {
                        SetAction(-1);
                    }
                }
            }
            else if ( m_model == 6 )  // chevy ?
            {
                if ( m_object->GetCargo() != 0 )
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
                        m_sound->Play(SOUND_CLOSE, m_object->GetPosition(), 0.7f, 1.8f);
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

                a = progress*Math::PI*1.3f;
                m_object->SetPartRotationZ(10, a);  // toît
                rot = AdjustRot(-2.9f*1.1f, 0.0f*1.1f, a);
                pos = Math::Vector(-1.8f*1.1f+rot.x, 5.3f*1.1f+rot.y, 0.0f*1.1f);
                m_object->SetPartPosition(10, pos);

                a = progress*Math::PI*0.4f;
                m_object->SetPartRotationX(13, a);  // fenêtre gauche
                rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, -a);
                pos = Math::Vector(-4.2f*1.1f, 4.5f*1.1f+rot.y, 1.9f*1.1f+rot.x);
                m_object->SetPartPosition(13, pos);

                m_object->SetPartRotationX(12, -a);  // fenêtre droite
                rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, a);
                pos = Math::Vector(-4.2f*1.1f, 4.5f*1.1f+rot.y, -1.9f*1.1f+rot.x);
                m_object->SetPartPosition(12, pos);

                m_object->SetPartRotationX(15, a);  // montant gauche
                rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, -a);
                pos = Math::Vector(-1.2f*1.1f, 4.5f*1.1f+rot.y, 1.9f*1.1f+rot.x);
                m_object->SetPartPosition(15, pos);

                m_object->SetPartRotationX(14, -a);  // montant droite
                rot = AdjustRot(0.0f*1.1f, -0.6f*1.1f, a);
                pos = Math::Vector(-1.2f*1.1f, 4.5f*1.1f+rot.y, -1.9f*1.1f+rot.x);
                m_object->SetPartPosition(14, pos);
                
                if ( m_actionType == MV_CLOSE && m_progress == 1.0f )
                {
                    m_sound->Play(SOUND_CLOSE, m_object->GetPosition(), 0.7f, 1.8f);
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
        s = m_physics->GetLinMotionX(MO_MOTSPEED)*0.5f;
        a = m_physics->GetCirMotionY(MO_MOTSPEED)*2.5f;

        m_posTrackLeft  += event.rTime*(s+a);
        m_posTrackRight += event.rTime*(s-a);

        UpdateTrackMapping(m_posTrackLeft, m_posTrackRight);

        pos = m_object->GetPosition();
        angle = m_object->GetRotation();
        if ( pos.x   != m_wheelLastPos.x   ||
             pos.y   != m_wheelLastPos.y   ||
             pos.z   != m_wheelLastPos.z   ||
             angle.x != m_wheelLastAngle.x ||
             angle.y != m_wheelLastAngle.y ||
             angle.z != m_wheelLastAngle.z )
        {
            m_wheelLastPos = pos;
            m_wheelLastAngle = angle;

            limit[0] =  15.0f*Math::PI/180.0f;
            limit[1] = -15.0f*Math::PI/180.0f;

//?            if ( Math::Distance(pos, m_engine->GetEyePt()) < 50.0f )  // suspension ?
            if ( false )
            {
                character = m_object->GetCharacter();
                mat = m_object->GetWorldMatrix(0);

                pos.x =  character->wheelFrontPos.x;  // roue avant droite
                pos.z = -character->wheelFrontPos.z;
                pos.y =  0.0f;
                pos = Transform(*mat, pos);
                a1 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelFrontPos.x);

                pos.x = -character->wheelBackPos.x;  // roue arrière droite
                pos.z = -character->wheelBackPos.z;
                pos.y =  0.0f;
                pos = Transform(*mat, pos);
                a2 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelBackPos.x);

                a = (a2-a1)/2.0f;
                if ( a > limit[0] )  a = limit[0];
                if ( a < limit[1] )  a = limit[1];
                m_object->SetPartRotationZ(1, a);

                pos.x =  character->wheelFrontPos.x;  // roue avant gauche
                pos.z =  character->wheelFrontPos.z;
                pos.y =  0.0f;
                pos = Transform(*mat, pos);
                a1 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelFrontPos.x);

                pos.x = -character->wheelBackPos.x;  // roue arrière gauche
                pos.z =  character->wheelBackPos.z;
                pos.y =  0.0f;
                pos = Transform(*mat, pos);
                a2 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelBackPos.x);

                a = (a2-a1)/2.0f;
                if ( a > limit[0] )  a = limit[0];
                if ( a < limit[1] )  a = limit[1];
                m_object->SetPartRotationZ(2, a);
            }
            else
            {
                m_object->SetPartRotationZ(1, 0.0f);
                m_object->SetPartRotationZ(2, 0.0f);
            }
        }

        // Monte la pelle si tourne.
        a = fabs(m_physics->GetCirMotionY(MO_REASPEED)/m_physics->GetCirMotionY(MO_ADVSPEED));
        a *= 45.0f*Math::PI/180.0f;
        a = Math::Smooth(m_object->GetPartRotationZ(3), a, event.rTime);
        m_object->SetPartRotationZ(3, a);

        // Bouge les leviers si tourne.
        a = m_physics->GetCirMotionY(MO_REASPEED)/m_physics->GetCirMotionY(MO_ADVSPEED);
        a *= 45.0f*Math::PI/180.0f;
        a = Math::Smooth(m_object->GetPartRotationZ(4), a, event.rTime);
        m_object->SetPartRotationZ(4, a);
        m_object->SetPartRotationZ(5, -a);
    }

    return true;
}

// Ouvre ou ferme une partie de la voiture lorsqu'un objet est transporté.

void CMotionCar::OpenClosePart(float progress)
{
    Math::Vector    pos;
    Math::Point        rot;
    float        a;

    if ( m_model == 2 )  // ford 32 ?
    {
        a = progress*(100.0f*Math::PI/180.0f);
        m_object->SetPartRotationZ(24, a);  // coffre ar
    }

    if ( m_model == 6 )  // chevy ?
    {
        a = progress*Math::PI*0.5f;
        m_object->SetPartRotationZ(10, a);  // toît
        rot = Math::RotatePoint(progress*Math::PI/2.0f, Math::Point(0.0f, 2.5f));
        pos.x = -1.0f+rot.x;
        pos.y =  2.5f+rot.y;
        pos.z =  0.0f;
        m_object->SetPartPosition(10, pos);
        m_object->SetPartScaleX(10, 1.0f-progress*0.8f);
        m_object->SetPartScaleZ(10, 1.0f-progress*0.3f);
    }
}


// Met à jour le mapping de la texture des roues.

void CMotionCar::UpdateWheelMapping(float speed, bool bFront)
{
    Gfx::Material    mat;
    float            au, bu, s;
    int                w[2], i;

    if ( m_model == 99 )  return;  // voiture fantome ?

    speed = fabs(speed);
         if ( speed <  5.0f )  s = 0.00f;
    else if ( speed < 10.0f )  s = 0.25f;
    else if ( speed < 20.0f )  s = 0.50f;
    else                       s = 0.75f;

    i = bFront?0:1;
    if ( s == m_lastWheelSpeed[i] )  return;
    m_lastWheelSpeed[i] = s;

    mat.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);  // blanc
    mat.ambient = Gfx::Color(0.5f, 0.5f, 0.5f);

    if ( bFront )
    {
        w[0] = m_object->GetObjectRank(3);
        w[1] = m_object->GetObjectRank(4);
    }
    else
    {
        w[0] = m_object->GetObjectRank(5);
        w[1] = m_object->GetObjectRank(6);
    }

    if (w[0] == -1 || w[1] == -1) return; // TODO: check ~krzys_h

    // faces des roues :
    au = 0.25f/2.0f;
    bu = s+0.25f/2.0f;
    for ( i=0 ; i<1 ; i++ )
    {
        m_engine->ChangeTextureMapping(w[0],
                                       mat, Gfx::ENG_RSTATE_PART1, m_nameTexWheel, "",
                                       Gfx::ENG_TEX_MAPPING_1X,
                                       au, bu, 1.0f, 0.0f);
        m_engine->ChangeTextureMapping(w[1],
                                       mat, Gfx::ENG_RSTATE_PART1, m_nameTexWheel, "",
                                       Gfx::ENG_TEX_MAPPING_1X,
                                       au, bu, 1.0f, 0.0f);
    }

    // profil des roues :
    au = s+0.00f+0.5f/256.0f;
    bu = s+0.25f-0.5f/256.0f;
    for ( i=0 ; i<1 ; i++ )
    {
        m_engine->ChangeTextureMapping(w[0],
                                       mat, Gfx::ENG_RSTATE_PART2, m_nameTexWheel, "",
                                       Gfx::ENG_TEX_MAPPING_X,
                                       au, bu, 0.0f, 0.0f);
        m_engine->ChangeTextureMapping(w[1],
                                       mat, Gfx::ENG_RSTATE_PART2, m_nameTexWheel, "",
                                       Gfx::ENG_TEX_MAPPING_X,
                                       au, bu, 0.0f, 0.0f);
    }
}

// Met à jour le mapping de la texture des vitres.

void CMotionCar::UpdateGlassMapping(int part, float min, float max,
                                    float dir, Gfx::EngineTextureMapping mode, bool bBlack)
{
    Math::Vector        eye, look;
    Gfx::Material    mat;
    float            au, bu, angle;
    int                rank, i, mmode;

    rank = m_object->GetObjectRank(part);

    mat.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);  // blanc
    mat.ambient = Gfx::Color(0.5f, 0.5f, 0.5f);

    eye  = m_engine->GetEyePt();
    look = m_engine->GetLookatPt();
    angle = Math::RotateAngle(look.x-eye.x, look.z-eye.z)*dir;
    angle = Math::Mod(angle/Math::PI, 2.0f);
//?    angle = angle/(Math::PI*2.0f)+0.5f;

    au = 0.5f/(max-min);
    bu = -(0.5f+angle);

    if ( bBlack )  mmode = Gfx::ENG_RSTATE_TTEXTURE_WHITE|Gfx::ENG_RSTATE_2FACE|Gfx::ENG_RSTATE_WRAP;
    else           mmode = Gfx::ENG_RSTATE_TTEXTURE_BLACK|Gfx::ENG_RSTATE_2FACE|Gfx::ENG_RSTATE_WRAP;

    for ( i=0 ; i<1 ; i++ )
    {
        m_engine->ChangeTextureMapping(rank, mat, mmode, m_nameTexWheel, "",
                                       mode, au, bu, 1.0f, 0.0f);
    }
}

// Met à jour le mapping de la texture des chenilles.

void CMotionCar::UpdateTrackMapping(float left, float right)
{
    Gfx::Material    mat;
    int                rRank, lRank, i;

    mat.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f);  // blanc
    mat.ambient = Gfx::Color(0.5f, 0.5f, 0.5f);

    rRank = m_object->GetObjectRank(1);
    lRank = m_object->GetObjectRank(2);

    for ( i=0 ; i<2 ; i++ )
    {
        m_engine->TrackTextureMapping(rRank, mat, Gfx::ENG_RSTATE_PART1, "trax.tga", "",
                                      Gfx::ENG_TEX_MAPPING_X,
                                      right, 2.0f, 8.0f, 192.0f, 256.0f);

        m_engine->TrackTextureMapping(lRank, mat, Gfx::ENG_RSTATE_PART2, "trax.tga", "",
                                      Gfx::ENG_TEX_MAPPING_X,
                                      left, 2.0f, 8.0f, 192.0f, 256.0f);
    }
}


// Initialse la torsion de toutes les pièces pour qu'elles soient
// toutes droites.

void CMotionCar::TwistInit()
{
    Math::Vector    min, max, factor;
    float        a;
    int            i, objRank;

    for ( i=0 ; i<m_partTotal ; i++ )
    {
        if ( m_part[i].bDeleted )  continue;

        m_part[i].twistState = 0;

        objRank = m_object->GetObjectRank(m_part[i].part);
        if ( objRank == -1 )
        {
            factor = Math::Vector(1.0f, 1.0f, 1.0f);
        }
        else
        {
            m_engine->GetObjectBBox(objRank, min, max);
            max = max-min;

            a = Math::RotateAngle(max.y, max.x);
            a = fabs(a-Math::PI/4.0f)/(Math::PI/4.0f);
            factor.x = (1.0f-a*0.8f)*0.7f;

            a = Math::RotateAngle(max.x, max.z);
            a = fabs(a-Math::PI/4.0f)/(Math::PI/4.0f);
            factor.y = (1.0f-a*0.8f)*0.7f;

            a = Math::RotateAngle(max.x, max.y);
            a = fabs(a-Math::PI/4.0f)/(Math::PI/4.0f);
            factor.z = (1.0f-a*0.8f)*0.7f;
        }

        m_part[i].twistAngle.x = (Math::Rand()-0.5f)*factor.x;
        m_part[i].twistAngle.y = (Math::Rand()-0.5f)*factor.y;
        m_part[i].twistAngle.z = (Math::Rand()-0.5f)*factor.z;
    }
}

// Initialise les torsions selon une copie de l'état.

void CMotionCar::TwistInit(StateVehicle *sv)
{
    Math::Vector    angle;
    int            i;

    for ( i=0 ; i<MAX_PART ; i++ )
    {
        if ( m_part[i].bDeleted )  continue;

        m_part[i].twistState = sv->state[i].twistState;
        m_part[i].twistAngle = sv->state[i].twistAngle;

        angle = m_part[i].twistAngle*(m_part[i].twistState/4.0f);
        m_object->SetPartRotation(m_part[i].part, angle);
    }
}

// Tord qq pièces suite à un choc.

void CMotionCar::TwistPart(Math::Vector impact, float force)
{
    Math::Vector    angle;
    int            i, j, total;

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
        m_object->SetPartRotation(m_part[i].part, angle);

        total ++;
    }
}

// Getourne le numéro de la prochaine partie à détruire,
// et considère quelle est déjà virtuellement détruite.

int CMotionCar::GetRemovePart(int &param)
{
    int            order, i, j;
    int            iCandidate, tCandidate[MAX_PART];

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

    i = static_cast<int>(Math::Rand()*iCandidate);
    if ( i == iCandidate )  i--;
    i = tCandidate[i];

    for ( j=0 ; j<4 ; j++ )
    {
        if ( m_part[i].light[j] != -1 )
        {
            m_bLight[m_part[i].light[j]] = false;  // un phare cassé
        }
    }

    m_topRest -= m_part[i].top;
    m_part[i].bDeleted = true;
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

bool CMotionCar::GetLight(int rank)
{
    return m_bLight[rank];
}

// Indique la présence d'une partie spécifique.

bool CMotionCar::ExistPart(TypePart part)
{
    if ( part == TP_TOP )  return (m_topRest > 0);
    return false;
}

// Donne le nombre de pièces total du véhicule.

int CMotionCar::GetTotalPart()
{
    return m_partTotal;
}

// Donne le nombre de pièces utilisées du véhicule.

int CMotionCar::GetUsedPart()
{
    return m_partUsed;
}


// Gestion du type de roues.

void CMotionCar::SetWheelType(WheelType type)
{
    Gfx::COldModelManager* modelManager = m_engine->GetModelManager();

    if ( type == m_wheelType )  return;  // inchangé ?

    CMotion::SetWheelType(type);

    if ( type == WT_NORM )
    {
        m_object->DeletePart(3);
        m_object->DeletePart(4);
        m_object->DeletePart(5);
        m_object->DeletePart(6);  // supprime les roues existantes

        CreateWheel(modelManager, m_nameObjWheel);

        CreatePhysics();  // recrée une physique adaptée
    }

    if ( type == WT_BURN )
    {
        m_object->DeletePart(3);
        m_object->DeletePart(4);
        m_object->DeletePart(5);
        m_object->DeletePart(6);  // supprime les roues existantes

        CreateWheel(modelManager, m_nameObjWBurn);

        CreatePhysics();  // recrée une physique adaptée
    }
}

WheelType CMotionCar::GetWheelType()
{
    return CMotion::GetWheelType();
}


// Gestion de l'état du véhicule.

int CMotionCar::GetStateLength()
{
    return sizeof(StateVehicle);
}

void CMotionCar::GetStateBuffer(char *buffer)
{
    StateVehicle*    p;
    int                i;

    p = reinterpret_cast<StateVehicle*>(buffer);
    for ( i=0 ; i<MAX_PART ; i++ )
    {
        p->state[i].bDeleted   = m_part[i].bDeleted;
        p->state[i].twistState = m_part[i].twistState;
        p->state[i].twistAngle = m_part[i].twistAngle;
    }
}


// Charge le robot marcheur le plus proche.

void CMotionCar::ActionLoadBot(float rTime)
{
    CPhysics*    physics;
    Math::Vector    pos, speed;
    Math::Point        dim;
    float        s, duration, dist, angle;
    int            i;

    if ( m_lbStart )
    {
        m_lbStart = false;
        m_lbWalker = dynamic_cast<COldObject*>(CObjectManager::GetInstancePointer()->FindNearest(m_object, OBJECT_WALKER, 40.0f));

        if ( m_lbWalker == 0 )
        {
            physics = 0;
        }
        else
        {
            physics = m_lbWalker->GetPhysics();
        }

        s = fabs(m_physics->GetLinMotionX(MO_REASPEED));

        if ( m_lbWalker == 0           ||  // rien trouvé ?
             s > 1.0f                  ||  // voiture pas arrêté ?
             physics == 0              ||  // pas de physique
             !m_lbWalker->IsProgram()  ||  // ne marche pas ?
             m_object->GetCargo() != 0 ||  // porte déjà qq chose ?
             m_object->GetLock()       )   // occupé ?
        {
            m_lbAction = false;
            SetAction(-1);
            return;
        }

        m_lbProgress = 0.0f;
        m_lbSpeed = m_actionTime;
        m_lbTime = 0.0f;

        m_lbWalker->StopProgram();  // stoppe le programme en cours
        physics->SetLinMotion(MO_MOTSPEED, Math::Vector(0.0f, 0.0f, 0.0f));
        physics->SetLinMotion(MO_REASPEED, Math::Vector(0.0f, 0.0f, 0.0f));
        StartObjectAction(m_object, MV_OPEN);
        StartObjectAction(m_lbWalker, MB_GOHOME);
        m_object->SetLock(true);
        m_lbWalker->SetLock(true);

        m_lbCamera = m_camera->GetType();
        if ( m_lbCamera != Gfx::CAM_TYPE_BACK )  m_camera->SetType(Gfx::CAM_TYPE_BACK);
        m_lbBackDist = m_camera->GetBackDist();

        m_lbStartPos = m_lbWalker->GetPosition();
        m_lbGoalPos = GetVehiclePoint(m_object);
        m_lbGoalPos.y += m_lbWalker->GetCharacter()->height;

        m_lbStartAngle = Math::NormAngle(m_lbWalker->GetRotationY());
        m_lbGoalAngle  = Math::NormAngle(m_object->GetRotationY())+Math::PI*2.0f;

        i = m_sound->Play(SOUND_FLY, m_lbGoalPos, 0.0f, 1.0f, true);
        m_sound->AddEnvelope(i, 1.0f, 1.5f, 0.5f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 1.0f, 1.5f, 2.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 0.0f, 1.0f, 0.5f, SOPER_STOP);
    }

    m_lbProgress += rTime*m_lbSpeed;
    m_lbTime += rTime;

    if ( m_lbProgress < 1.0f )
    {
        pos = m_lbStartPos+(m_lbGoalPos-m_lbStartPos)*m_lbProgress;
        pos.y += sinf(m_lbProgress*Math::PI)*10.0f;
        m_lbWalker->SetPosition(pos);

        angle = m_lbStartAngle+(m_lbGoalAngle-m_lbStartAngle)*m_lbProgress;
        m_lbWalker->SetRotationY(angle);

        dist = m_lbBackDist+sinf(m_lbProgress*Math::PI)*30.0f;
        m_camera->SetBackDist(dist);

        if ( m_lbLastParticle+m_engine->ParticleAdapt(0.05f) <= m_lbTime )
        {
            m_lbLastParticle = m_lbTime;

            pos.y -= m_lbWalker->GetCharacter()->height;
            for ( i=0 ; i<4 ; i++ )
            {
                speed.x = (Math::Rand()-0.5f)*2.0f;
                speed.z = (Math::Rand()-0.5f)*2.0f;
                speed.y = -Math::Rand()*5.0f;
                dim.x = Math::Rand()*1.0f+1.0f;
                dim.y = dim.x;
                duration = Math::Rand()*1.0f+1.0f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, duration);
            }
        }
    }
    else
    {
        m_lbWalker->SetTransporter(m_object);
        m_object->SetCargo(m_lbWalker);
        pos = m_object->GetCharacter()->posFret;
        pos.y += m_lbWalker->GetCharacter()->height;
        m_lbWalker->SetPosition(pos);
        m_lbWalker->SetRotation(m_object->GetCharacter()->angleFret);

        m_object->SetLock(false);
        m_lbWalker->SetLock(false);
        StartObjectAction(m_object, MV_CLOSE);
        StartObjectAction(m_lbWalker, MB_WAIT);

        if ( m_lbCamera != Gfx::CAM_TYPE_BACK )  m_camera->SetType(m_lbCamera);
        m_camera->SetBackDist(m_lbBackDist);

        m_lbAction = false;
    }
}

// Cherche le point où mettre du fret sur un véhicule.

Math::Vector CMotionCar::GetVehiclePoint(CObject *pObj)
{
    Character*    character;
    Math::Matrix*    mat;
    Math::Vector    pos;

    character = pObj->GetCharacter();
    mat = pObj->GetWorldMatrix(0);
    pos = Transform(*mat, character->posFret);

    return pos;
}

// Démarre une action pour un objet.

void CMotionCar::StartObjectAction(COldObject* pObj, int action)
{
    CMotion*    motion;
    float        delay;

    motion = pObj->GetMotion();
    if ( motion == 0 )  return;

    delay = 1.0f;
    motion->SetAction(action, delay);
}


