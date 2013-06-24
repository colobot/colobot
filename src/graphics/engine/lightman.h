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

/**
 * \file graphics/engine/lightman.h
 * \brief Dynamic light manager - CLightManager class
 */

#pragma once


#include "graphics/core/color.h"
#include "graphics/core/light.h"
#include "graphics/engine/engine.h"

#include "math/vector.h"


// Graphics module namespace
namespace Gfx {

/**
 * \struct LightProgression
 * \brief Describes the progression of light parameters change
 */
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
 * \enum LightPriority
 * \brief Priority in light assignment
 */
enum LightPriority
{
    LIGHT_PRI_HIGHEST = 0, //!< always highest weight (always picked)
    LIGHT_PRI_HIGH = 1,    //!< high weight
    LIGHT_PRI_LOW  = 2     //!< low weight
};

/**
 * \struct DynamicLight
 * \brief Dynamic light in 3D scene
 *
 * It is an extension over standard light properties. Added are dynamic progressions for light
 * colors and intensity and types of objects included/excluded in lighting.
 */
struct DynamicLight
{
    //! Rank (index)
    int rank;

    //! Whether the light is used
    bool used;
    //! Whether the light is turned on
    bool enabled;

    //! Priority in assignment
    LightPriority priority;

    //! Configuration of the light
    Light light;

    //! Progression of intensity [0, 1]
    LightProgression intensity;
    //! Progression of red diffuse color
    LightProgression colorRed;
    //! Progression of green diffuse color
    LightProgression colorGreen;
    //! Progression of blue diffuse color
    LightProgression colorBlue;

    //! Type of objects included in lighting with this light; if ENG_OBJTYPE_NULL is used, it is ignored
    EngineObjectType includeType;
    //! Type of objects excluded from lighting with this light; if ENG_OBJTYPE_NULL is used, it is ignored
    EngineObjectType excludeType;

    DynamicLight();
};

/**
 * \class CLightManager
 * \brief Manager for dynamic lights in 3D scene
 *
 * The class is responsible for managing dynamic lights (struct DynamicLight) used in 3D scene.
 * The dynamic lights are created, updated and deleted through the class' interface.
 *
 * Since there is a limit on total number of lights available in OpenGL (usually up to 8), the dynamic lights
 * must be emulated by displaying only some of them. All functions normally operate only on DynamicLight structs,
 * updating the models with new values, while only one function, UpdateDeviceLights(), performs the actual
 * synchronization to the device. It allocates device's light slots as necessary, with two priority levels
 * for lights.
 */
class CLightManager
{
public:
    //! Constructor
    CLightManager(CEngine* engine);
    //! Destructor
    virtual ~CLightManager();

    //! Sets the device to be used
    void            SetDevice(CDevice* device);

    //! Prints debug info
    void DebugDumpLights();

    //! Clears and disables all lights
    void            FlushLights();
    //! Creates a new dynamic light and returns its index (lightRank)
    int             CreateLight(LightPriority priority = LIGHT_PRI_LOW);
    //! Deletes and disables the given dynamic light
    bool            DeleteLight(int lightRank);
    //! Sets the light parameters for dynamic light
    bool            SetLight(int lightRank, const Light &light);
    //! Returns the light parameters for given dynamic light
    bool            GetLight(int lightRank, Light &light);
    //! Enables/disables the given dynamic light
    bool            SetLightEnabled(int lightRank, bool enable);
    //! Changes the light priority
    bool            SetLightPriority(int lightRank, LightPriority priority);

    //! Sets what objects are included in given dynamic light
    bool            SetLightIncludeType(int lightRank, EngineObjectType type);
    //! Sets what objects are excluded from given dynamic light
    bool            SetLightExcludeType(int lightRank, EngineObjectType type);

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
    void            AdaptLightColor(const Color &color, float factor);

    //! Sets the destination color for dynamic light's color progression
    bool            SetLightColor(int lightRank, const Color &color);
    //! Returns current light color
    Color           GetLightColor(int lightRank);
    //! Sets the rate of change for dynamic light colors (RGB)
    bool            SetLightColorSpeed(int lightRank, float speed);

    //! Updates progression of dynamic lights
    void            UpdateProgression(float rTime);
    //! Updates (recalculates) all dynamic lights
    void            UpdateLights();
    //! Enables or disables dynamic lights affecting the given object type
    void            UpdateDeviceLights(EngineObjectType type);

protected:
    class LightsComparator
    {
        public:
            LightsComparator(Math::Vector eyePos, EngineObjectType objectType);

            bool operator()(const DynamicLight& left, const DynamicLight& right);

        private:
            float GetLightWeight(const DynamicLight& dynLight);

            Math::Vector m_eyePos;
            EngineObjectType m_objectType;
    };

protected:
    CEngine*          m_engine;
    CDevice*          m_device;

    //! Current time
    float             m_time;
    //! List of dynamic lights
    std::vector<DynamicLight> m_dynLights;
    //! Map of current light allocation: graphics light -> dynamic light
    std::vector<int>  m_lightMap;
};

}; // namespace Gfx

