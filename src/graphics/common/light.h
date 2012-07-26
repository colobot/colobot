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

/**
  \enum LightType
  \brief Type of light in 3D scene */
enum LightType
{
    LIGHT_POINT,
    LIGHT_SPOT,
    LIGHT_DIRECTIONAL
};

/**
  \struct Light
  \brief Properties of light in 3D scene

  This structure was created as analog to DirectX's D3DLIGHT. */
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
  \struct LightProgression
  \brief Describes the progression of light parameters change */
struct LightProgression
{
    //! Starting value
    float   starting;
    //! Ending (destination) value
    float   ending;
    //! Current value
    float   current;
    //! Progress from start to end
    float   progress;
    //! Speed of progression
    float   speed;

    LightProgression()
    {
        starting = ending = current = progress = speed = 0.0f;
    }

    //! Initializes the progression
    void Init(float value);

    //! Updates the progression
    void Update(float rTime);

    //! Sets the new end value (starting is set to current)
    void SetTarget(float value);
};

/**
  \struct DynamicLight
  \brief Dynamic light in 3D scene

  It is an extension over standard light properties. Added are dynamic progressions for light
  colors and intensity and types of objects included/excluded in lighting. */
struct DynamicLight
{
    //! Whether the light is used
    bool used;
    //! Whether the light is turned on
    bool enabled;

    //! Configuration of the light
    Gfx::Light light;

    //! Progression of intensity [0, 1]
    Gfx::LightProgression intensity;
    //! Progression of red diffuse color
    Gfx::LightProgression colorRed;
    //! Progression of green diffuse color
    Gfx::LightProgression colorGreen;
    //! Progression of blue diffuse color
    Gfx::LightProgression colorBlue;

    //! Type of objects included in lighting with this light; if Gfx::ENG_OBJTYPE_NULL is used, it is ignored
    Gfx::EngineObjectType includeType;
    //! Type of objects excluded from lighting with this light; if Gfx::ENG_OBJTYPE_NULL is used, it is ignored
    Gfx::EngineObjectType excludeType;

    DynamicLight();
};

/**
  \class CLightManager
  \brief Manager for dynamic lights in 3D scene

  (Old CLight class)

  The class is responsible for managing dynamic lights (struct Gfx::DynamicLight) used in 3D scene.
  The dynamic lights are created, updated and deleted through the class' interface.

  Number of available lights depends on graphics device used. Class allocates vector
  for the total number of lights, but only some are used.
  */
class CLightManager
{
public:
    //! Constructor
    CLightManager(CInstanceManager *iMan, Gfx::CEngine* engine);
    //! Destructor
    virtual ~CLightManager();

    //! Sets the device to be used
    void            SetDevice(Gfx::CDevice* device);

    //! Clears and disables all lights
    void            FlushLights();
    //! Creates a new dynamic light and returns its index (lightRank)
    int             CreateLight();
    //! Deletes and disables the given dynamic light
    bool            DeleteLight(int lightRank);
    //! Sets the light parameters for dynamic light
    bool            SetLight(int lightRank, const Gfx::Light &light);
    //! Returns the light parameters for given dynamic light
    bool            GetLight(int lightRank, Gfx::Light &light);
    //! Enables/disables the given dynamic light
    bool            SetLightEnabled(int lightRank, bool enable);

    //! Sets what objects are included in given dynamic light
    bool            SetLightIncludeType(int lightRank, Gfx::EngineObjectType type);
    //! Sets what objects are excluded from given dynamic light
    bool            SetLightExcludeType(int lightRank, Gfx::EngineObjectType type);

    //! Sets the position of dynamic light
    bool            SetLightPos(int lightRank, const Math::Vector &pos);
    //! Returns the position of dynamic light
    Math::Vector    GetLightPos(int lightRank);

    //! Sets the direction of dynamic light
    bool            SetLightDir(int lightRank, const Math::Vector &dir);
    //! Returns the direction of dynamic light
    Math::Vector    GetLightDir(int lightRank);

    //! Sets the destination intensity for dynamic light's intensity progression
    bool            SetLightIntensity(int lightRank, float value);
    //! Returns the current light intensity
    float           GetLightIntensity(int lightRank);
    //! Sets the rate of change for dynamic light intensity
    bool            SetLightIntensitySpeed(int lightRank, float speed);

    //! Adjusts the color of all dynamic lights
    void            AdaptLightColor(const Gfx::Color &color, float factor);

    //! Sets the destination color for dynamic light's color progression
    bool            SetLightColor(int lightRank, const Gfx::Color &color);
    //! Returns current light color
    Gfx::Color      GetLightColor(int lightRank);
    //! Sets the rate of change for dynamic light colors (RGB)
    bool            SetLightColorSpeed(int lightRank, float speed);

    //! Updates progression of dynamic lights
    void            UpdateProgression(float rTime);
    //! Updates (recalculates) all dynamic lights
    void            UpdateLights();
    //! Enables or disables dynamic lights affecting the given object type
    void            UpdateLightsEnableState(Gfx::EngineObjectType type);

protected:
    CInstanceManager* m_iMan;
    CEngine*          m_engine;
    CDevice*          m_device;

    //! Current time
    float             m_time;
    //! List of dynamic lights
    std::vector<Gfx::DynamicLight> m_dynLights;
};

}; // namespace Gfx
