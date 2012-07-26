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

// light.h

#pragma once


#include "graphics/common/engine.h"
#include "graphics/common/color.h"
#include "math/vector.h"


namespace Gfx {

/** \enum LightType
 *  \brief Type of light */
enum LightType
{
    LIGHT_POINT,
    LIGHT_SPOT,
    LIGHT_DIRECTIONAL
};

/**
 * \struct Light
 * \brief Light
 *
 * This structure was created as analog to DirectX's D3DLIGHT.
 */
struct Light
{
    //! Type of light source
    Gfx::LightType  type;
    //! Color of ambient light
    Gfx::Color      ambient;
    //! Color of diffuse light
    Gfx::Color      diffuse;
    //! Color of specular light
    Gfx::Color      specular;
    //! Position in world space (for point & spot lights)
    Math::Vector    position;
    //! Direction in world space (for directional & spot lights)
    Math::Vector    direction;
    //! Constant attenuation factor
    float           attenuation0;
    //! Linear attenuation factor
    float           attenuation1;
    //! Quadratic attenuation factor
    float           attenuation2;
    //! Angle of spotlight cone (0-90 degrees)
    float           spotAngle;
    //! Intensity of spotlight (0 = uniform; 128 = most intense)
    float           spotIntensity;

    //! Constructor; calls LoadDefault()
    Light()
    {
        LoadDefault();
    }

    //! Loads default values
    void LoadDefault()
    {
        type = LIGHT_POINT;
        ambient = Gfx::Color(0.4f, 0.4f, 0.4f);
        diffuse = Gfx::Color(0.8f, 0.8f, 0.8f);
        specular = Gfx::Color(1.0f, 1.0f, 1.0f);
        position = Math::Vector(0.0f, 0.0f, 0.0f);
        direction = Math::Vector(0.0f, 0.0f, 1.0f);
        attenuation0 = 1.0f;
        attenuation1 = attenuation2 = 0.0f;
        spotAngle = 90.0f;
        spotIntensity = 0.0f;
    }
};

/**
 * \struct LightProgression
 * \brief Describes the progression of light parameters change
 *
 * TODO documentation
 */
struct LightProgression
{
    float   starting;
    float   ending;
    float   current;
    float   progress;
    float   speed;

    LightProgression()
    {
        starting = ending = current = progress = speed = 0.0f;
    }
};

/**
 * \struct DynamicLight
 * \brief Dynamic light in 3D scene
 *
 * TODO documentation
 */
struct DynamicLight
{
    //! true -> light exists
    bool used;
    //! true -> light turned on
    bool enabled;

    //! Type of all objects included
    Gfx::EngineObjectType includeType;
    //! Type of all objects excluded
    Gfx::EngineObjectType excludeType;

    //! Configuration of the light
    Gfx::Light light;

    //! intensity (0 .. 1)
    Gfx::LightProgression intensity;
    Gfx::LightProgression colorRed;
    Gfx::LightProgression colorGreen;
    Gfx::LightProgression colorBlue;
};

/**
  \class CLight
  \brief Manager for dynamic lights in 3D scene
  */
class CLight
{
public:
    CLight(CInstanceManager *iMan, Gfx::CEngine* engine);
    virtual ~CLight();

    void            SetDevice(Gfx::CDevice* device);

    void            FlushLight();
    int             CreateLight();
    bool            DeleteLight(int lightRank);
    bool            SetLight(int lightRank, const Gfx::Light &light);
    bool            GetLight(int lightRank, Gfx::Light &light);
    bool            LightEnable(int lightRank, bool enable);

    bool            SetLightIncludeType(int lightRank, Gfx::EngineObjectType type);
    bool            SetLightExcludeType(int lightRank, Gfx::EngineObjectType type);

    bool            SetLightPos(int lightRank, const Math::Vector &pos);
    Math::Vector    GetLightPos(int lightRank);

    bool            SetLightDir(int lightRank, const Math::Vector &dir);
    Math::Vector    GetLightDir(int lightRank);

    bool            SetLightIntensitySpeed(int lightRank, float speed);
    bool            SetLightIntensity(int lightRank, float value);
    float           GetLightIntensity(int lightRank);
    void            AdaptLightColor(const Gfx::Color &color, float factor);

    bool            SetLightColorSpeed(int lightRank, float speed);
    bool            SetLightColor(int lightRank, const Gfx::Color &color);
    Gfx::Color      GetLightColor(int lightRank);

    void            FrameLight(float rTime);
    void            LightUpdate();
    void            LightUpdate(Gfx::EngineObjectType type);

protected:
    CInstanceManager* m_iMan;
    CEngine*          m_engine;
    CDevice*          m_device;
    float             m_time;
    std::vector<Gfx::DynamicLight> m_dynLights;
};

}; // namespace Gfx
