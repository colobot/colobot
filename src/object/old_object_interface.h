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
 * \file object/old_object_interface.h
 * \brief Legacy CObject interface
 */

#pragma once

#include "math/vector.h"

#include "object/object_type.h"

namespace Math
{
struct Matrix;
} // namespace Math

class CAuto;


struct Character
{
    float        height = 0.0f;         // normal height on top of ground

    // Used by legacy Colobot code, TODO: refactor to new format? ~krzys_h
    float        wheelFront = 0.0f;     // position X of the front wheels
    float        wheelBack = 0.0f;      // position X of the back wheels
    float        wheelLeft = 0.0f;      // position Z of the left wheels
    float        wheelRight = 0.0f;     // position Z of the right wheels

    // Used by BuzzingCars code
    Math::Vector    wheelFrontPos;    // position roue avant gauche
    Math::Vector    wheelBackPos;    // position roue arričre gauche
    float        wheelFrontDim;    // rayon roues avants
    float        wheelBackDim;    // rayon roues arričres
    float        wheelFrontWidth;// largeur des pneus avants
    float        wheelBackWidth;    // largeur des pneus arričres
    float        crashFront;        // distance jusqu'ŕ l'avant pour collisions
    float        crashBack;        // distance jusqu'ŕ l'arričre pour collisions
    float        crashWidth;        // distance latérale pour collisions
    float        suspDetect;        // détection suspension
    float        suspHeight;        // hauteur suspension
    float        suspFrequency;    // fréquence suspension
    float        suspAbsorber;    // amortisseur suspension
    float        rolling;        // tandance au roulis dans les virages
    float        nicking;        // tandance au nick avant/arričre
    float        maxRolling;        // angle max de roulis
    float        maxNicking;        // angle max de nick
    float        overProp;        // tandance ŕ survirer si propulsion
    float        overFactor;        // tandance au survirage dans les courbes
    float        overAngle;        // angle max de survirage
    float        overMul;        // accentiation de l'effet de survirage
    float        brakeDir;        // efficacité direction pendant freinage
    Math::Vector    posFret;        // position pour le fret
    Math::Vector    angleFret;        // angle pour le fret
    Math::Vector    antenna;        // pointe de l'antenne
    Math::Vector    lightFL;        // phase avant gauche
    Math::Vector    lightFR;        // phase avant droite
    Math::Vector    lightSL;        // phase stop gauche
    Math::Vector    lightSR;        // phase stop droite
    Math::Vector    lightRL;        // phase recule gauche
    Math::Vector    lightRR;        // phase recule droite
    Math::Vector    camera;            // position caméra en mode CAMERA_ONBOARD
    float        hookDist;        // distance pour porter avec le crochet
    float        mass;            // poid du véhicule
    float        turnSlide;        // glissement si virage rapide
    float        gripLimit;        // limite d'adérance latérale
    float        gripSlide;        // tenue de route latérale
    float        accelLow;        // facteur d'accélération si vitesse nulle
    float        accelHigh;        // facteur d'accélération si vitesse max
    float        accelSmooth;    // progresion de l'accélération
    int            motorSound;        // son pour le moteur
};

class COldObjectInterface
{
public:
    virtual ~COldObjectInterface() {}

    virtual void        Simplify();
    virtual void        DeletePart(int part);
    virtual void        SetType(ObjectType type);

    virtual int         GetObjectRank(int part);

    virtual int         GetOption();

    virtual void        SetDrawFront(bool bDraw);


    virtual void        FloorAdjust();

    virtual void        SetLinVibration(Math::Vector dir);
    virtual void        SetCirVibration(Math::Vector dir);
    virtual void        SetCirChoc(Math::Vector dir);

    virtual Math::Vector    GetTilt();

    virtual void        SetMasterParticle(int part, int parti);

    virtual Math::Matrix*   GetWorldMatrix(int part);

    virtual Character*  GetCharacter();

    virtual void        FlatParent();

    // Not sure. Maybe a separate interface, or maybe CControllableObject (buildings can have viruses too)
    virtual void        SetVirusMode(bool bEnable);
    virtual bool        GetVirusMode();

    // This will be eventually removed after refactoring to subclasses
    virtual CAuto*      GetAuto();

    virtual void SetBurnShield(float level);
    virtual float GetBurnShield();
};
