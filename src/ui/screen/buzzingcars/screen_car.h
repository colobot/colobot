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

#pragma once

#include "ui/screen/screen.h"

#include "graphics/core/color.h"

namespace Ui
{

enum StateCar
{
    SC_SHOWCASE,	// voiture en vitrine
    SC_OWNER,		// voiture acquise
    SC_FORSALE,		// voiture à vendre
};

typedef struct
{
    int				total;			// nb de voitures achetées
    int				buyable;		// nb de voitures achetables
    int				bonus;			// nb de voitures dispo en bonus
    int				selectCar;		// voiture choisie (0..n)
    int				usedCars[50];	// voitures achetées (1..n)
    int				subModel[50];	// peintures utilisées (1..n)
    Gfx::Color	colorBody[50];	// couleurs carrosseries
    StateCar		stateCars[50];	// états des voitures (pour _SE)
    int				pesetas;		// argent disponible
    int				level;			// niveau de difficulté (1..3)
}
GamerPerso;

typedef struct
{
    float		minSpeed;			// vitesse minimale
    float		maxSpeed;			// vitesse maximale
}
CarSpec;

class CScreenCar : public CScreen
{
public:
    CScreenCar();
    void CreateInterface() override;
    bool EventProcess(const Event &event) override;

    void	LaunchSimul();
    void	BuyAllPerso();

    int		GetModel();
    int		GetSubModel();
    Gfx::Color GetGamerColorCar();

protected:
    void	UpdatePerso();
    void	ElevationPerso();
    void	CameraPerso();
    bool	IsBuyablePerso();
    void	BuyablePerso();
    void	NextPerso(int dir);
    void	SelectPerso();
    void	BuyPerso();
    int		GetPersoModel(int rank);
    int		IndexPerso(int model);
    bool	UsedPerso(int model);
    int		PricePerso(int model);
    const std::string NamePerso(int model);
    void	SpecPerso(CarSpec &spec, int model);
    void	FixPerso(int rank);
    void	FlushPerso();
    void	DefPerso(int rank);

protected:
    void SetCamera(float x, float y, float cameraDistance);

protected:
    bool			m_bPesetas = false;

    GamerPerso		m_perso;			// perso: description
//    GamerPerso		m_persoCopy;		// perso: copie si annulation
    float			m_persoElevation;	// perso: élévation caméra
    float			m_persoAngle;		// perso: angle de présentation
    float			m_persoTime;		// perso: temps absolu
    bool			m_persoRun;			// perso: moteur tourne ?
};

} // namespace Ui
