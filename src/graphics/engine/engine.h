/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/engine/engine.h
 * \brief Main graphics engine - CEngine class
 */

#pragma once

#include "common/singleton.h"

#include "graphics/core/color.h"
#include "graphics/core/material.h"
#include "graphics/core/texture.h"
#include "graphics/core/vertex.h"

#include "math/intpoint.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/sphere.h"
#include "math/vector.h"


#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <unordered_map>


class CApplication;
class CSoundInterface;
class CImage;
class CSystemUtils;
struct SystemTimeStamp;
struct Event;


// Graphics module namespace
namespace Gfx
{


class CDevice;
class COldModelManager;
class CLightManager;
class CText;
class CParticle;
class CWater;
class CCloud;
class CLightning;
class CPlanet;
class CTerrain;
class CPyroManager;
class CModelMesh;
struct ModelShadowSpot;
struct ModelTriangle;


/**
 * \enum EngineRenderState
 * \brief Render state of graphics engine
 *
 * States are used for settings certain modes, for instance texturing and blending.
 * The enum is a bitmask and some of the states can be OR'd together.
 */
enum EngineRenderState
{
    //! Normal opaque materials
    ENG_RSTATE_NORMAL           = 0,
    //! The transparent texture (black = no)
    ENG_RSTATE_TTEXTURE_BLACK   = (1<<0),
    //! The transparent texture (white = no)
    ENG_RSTATE_TTEXTURE_WHITE   = (1<<1),
    //! The transparent diffuse color
    ENG_RSTATE_TDIFFUSE         = (1<<2),
    //! Texture wrap
    ENG_RSTATE_WRAP             = (1<<3),
    //! Texture borders with solid color
    ENG_RSTATE_CLAMP            = (1<<4),
    //! Light texture (ambient max)
    ENG_RSTATE_LIGHT            = (1<<5),
    //! Double black texturing
    ENG_RSTATE_DUAL_BLACK       = (1<<6),
    //! Double white texturing
    ENG_RSTATE_DUAL_WHITE       = (1<<7),
    //! Part 1 (no change in. MOD!)
    ENG_RSTATE_PART1            = (1<<8),
    //! Part 2
    ENG_RSTATE_PART2            = (1<<9),
    //! Part 3
    ENG_RSTATE_PART3            = (1<<10),
    //! Part 4
    ENG_RSTATE_PART4            = (1<<11),
    //! Double-sided face
    ENG_RSTATE_2FACE            = (1<<12),
    //! Image using alpha channel
    ENG_RSTATE_ALPHA            = (1<<13),
    //! Always use 2nd floor texturing
    ENG_RSTATE_SECOND           = (1<<14),
    //! Causes the fog
    ENG_RSTATE_FOG              = (1<<15),
    //! The transparent color (black = no)
    ENG_RSTATE_TCOLOR_BLACK     = (1<<16),
    //! The transparent color (white = no)
    ENG_RSTATE_TCOLOR_WHITE     = (1<<17),
    //! Mode for rendering text
    ENG_RSTATE_TEXT             = (1<<18),
    //! Only opaque texture, no blending, etc.
    ENG_RSTATE_OPAQUE_TEXTURE   = (1<<19),
    //! Only opaque color, no texture, blending, etc.
    ENG_RSTATE_OPAQUE_COLOR     = (1<<20),
    //! Texture using alpha channel
    ENG_RSTATE_TTEXTURE_ALPHA   = (1<<21),
    //! Color with transparency
    ENG_RSTATE_TCOLOR_ALPHA     = (1<<22)
};


/**
 * \enum EngineTriangleType
 * \brief Type of triangles drawn for engine objects
 */
enum EngineTriangleType
{
    //! Triangles
    ENG_TRIANGLE_TYPE_TRIANGLES = 1,
    //! Surfaces
    ENG_TRIANGLE_TYPE_SURFACE   = 2
};

/**
 * \struct EngineTriangle
 * \brief A triangle drawn by the graphics engine
 */
struct EngineTriangle
{
    //! Triangle vertices
    VertexTex2     triangle[3];
    //! Material
    Material       material;
    //! Render state
    int            state = ENG_RSTATE_NORMAL;
    //! 1st texture
    std::string    tex1Name;
    //! 2nd texture
    std::string    tex2Name;
};

/**
  \enum EngineObjectType
  \brief Class of graphics engine object */
enum EngineObjectType
{
    //! Object doesn't exist
    ENG_OBJTYPE_NULL        = 0,
    //! Terrain
    ENG_OBJTYPE_TERRAIN     = 1,
    //! Fixed object
    ENG_OBJTYPE_FIX         = 2,
    //! Moving object
    ENG_OBJTYPE_VEHICLE    = 3,
    //! Part of a moving object
    ENG_OBJTYPE_DESCENDANT  = 4,
    //! Fixed object type quartz
    ENG_OBJTYPE_QUARTZ      = 5,
    //! Fixed object type metal
    ENG_OBJTYPE_METAL       = 6
};


/**
 * \struct EngineBaseObjDataTier
 * \brief Tier 3 of object tree (data)
 */
struct EngineBaseObjDataTier
{
    EngineTriangleType      type;
    Material                material;
    int                     state;
    std::vector<VertexTex2> vertices;
    unsigned int            staticBufferId;
    bool                    updateStaticBuffer;

    inline EngineBaseObjDataTier(EngineTriangleType type = ENG_TRIANGLE_TYPE_TRIANGLES,
                                 const Material& material = Material(),
                                 int state = ENG_RSTATE_NORMAL)
     : type(type)
     , material(material)
     , state(state)
     , staticBufferId(0)
     , updateStaticBuffer(false)
    {}
};

/**
 * \struct EngineBaseObjTexTier
 * \brief Tier 2 of base object tree (textures)
 */
struct EngineBaseObjTexTier
{
    std::string                        tex1Name;
    Texture                            tex1;
    std::string                        tex2Name;
    Texture                            tex2;
    std::vector<EngineBaseObjDataTier>  next;

    inline EngineBaseObjTexTier(const std::string& tex1Name = "",
                                const std::string& tex2Name = "")
     : tex1Name(tex1Name)
     , tex2Name(tex2Name)
    {}
};

/**
 * \struct BaseEngineObject
 * \brief Base (template) object - geometry for engine objects
 *
 * This is also the tier 1 of base object tree.
 */
struct EngineBaseObject
{
    //! If true, base object is valid in objects vector
    bool used = false;
    //! Number of triangles
    int                    totalTriangles = 0;
    //! Bounding box min (origin 0,0,0 always included)
    Math::Vector           bboxMin;
    //! bounding box max (origin 0,0,0 always included)
    Math::Vector           bboxMax;
    //! A bounding sphere that contains all the vertices in this EngineBaseObject
    Math::Sphere           boundingSphere;
    //! Next tier (Tex)
    std::vector<EngineBaseObjTexTier> next;

    inline void LoadDefault()
    {
        *this = EngineBaseObject();
    }
};

/**
 * \struct EngineObject
 * \brief Object drawn by the graphics engine
 */
struct EngineObject
{
    //! If true, object is valid in objects vector
    bool                   used = false;
    //! Rank of associated base engine object
    int                    baseObjRank = -1;
    //! If true, the object is drawn
    bool                   visible = false;
    //! If true, object is behind the 2D interface
    bool                   drawWorld = false;
    //! If true, the shape is before the 2D interface
    bool                   drawFront = false;
    //! Type of object
    EngineObjectType       type = ENG_OBJTYPE_NULL;
    //! Transformation matrix
    Math::Matrix           transform;
    //! Distance to object from eye point
    float                  distance = 0.0f;
    //! Rank of the associated shadow
    int                    shadowRank = -1;
    //! Transparency of the object [0, 1]
    float                  transparency = 0.0f;

    //! Loads default values
    inline void LoadDefault()
    {
        *this = EngineObject();
    }
};

/**
 * \struct EngineShadowType
 * \brief Type of shadow drawn by the graphics engine
 */
enum EngineShadowType
{
    //! Normal shadow
    ENG_SHADOW_NORM = 0,
    //! TODO: ?
    ENG_SHADOW_WORM = 1
};

/**
 * \struct EngineShadow
 * \brief Shadow drawn by the graphics engine
 */
struct EngineShadow
{
    //! If true, shadow is valid
    bool                used = false;
    //! If true, shadow is invisible (object being carried for example)
    bool                hide = false;
    //! Rank of the associated object
    int                 objRank = -1;
    //! Type of shadow
    EngineShadowType type = ENG_SHADOW_NORM;
    //! Position of the shadow
    Math::Vector        pos;
    //! Normal to the terrain
    Math::Vector        normal;
    //! Angle of the shadow
    float               angle = 0.0f;
    //! Radius of the shadow
    float               radius = 0.0f;
    //! Intensity of the shadow
    float               intensity = 0.0f;
    //! Height from the ground
    float               height = 0.0f;

    void LoadDefault()
    {
        *this = EngineShadow();
    }
};

/**
 * \struct EngineGroundSpot
 * \brief A spot (large shadow) drawn on the ground by the graphics engine
 */
struct EngineGroundSpot
{
    //! If true, ground spot is valid
    bool            used = false;
    //! Color of the shadow
    Color      color;
    //! Min altitude
    float           min = 0.0f;
    //! Max altitude
    float           max = 0.0f;
    //! Transition area
    float           smooth = 0.0f;
    //! Position for the shadow
    Math::Vector    pos;
    //! Radius of the shadow
    float           radius = 0.0f;
    //! Position of the shadow drawn
    Math::Vector    drawPos;
    //! Radius of the shadow drawn
    float           drawRadius = 0.0f;

    void LoadDefault()
    {
        *this = EngineGroundSpot();
    }
};

/**
 * \enum EngineGroundMarkPhase
 * \brief Phase of life of an EngineGroundMark
 */
enum EngineGroundMarkPhase
{
    //! Null phase
    ENG_GR_MARK_PHASE_NULL = 0,
    //! Increase
    ENG_GR_MARK_PHASE_INC = 1,
    //! Fixed
    ENG_GR_MARK_PHASE_FIX = 2,
    //! Decrease
    ENG_GR_MARK_PHASE_DEC = 3
};

/**
 * \struct EngineGroundMark
 * \brief A mark on ground drawn by the graphics engine
 */
struct EngineGroundMark
{
    //! If true, draw mark
    bool                        draw = false;
    //! Phase of life
    EngineGroundMarkPhase  phase = ENG_GR_MARK_PHASE_NULL;
    //! Times for 3 life phases
    float                       delay[3] = {};
    //! Fixed time
    float                       fix = 0.0f;
    //! Position for marks
    Math::Vector                pos;
    //! Radius of marks
    float                       radius = 0.0f;
    //! Color intensity
    float                       intensity = 0.0f;
    //! Draw position for marks
    Math::Vector                drawPos;
    //! Radius for  marks
    float                       drawRadius = 0.0f;
    //! Draw intensity for marks
    float                       drawIntensity = 0.0f;
    //! X dimension of table
    int                         dx = 0;
    //! Y dimension of table
    int                         dy = 0;
    //! Pointer to the table
    char*                       table = nullptr;

    void LoadDefault()
    {
        *this = EngineGroundMark();
    }
};

/**
 * \enum EngineTextureMapping
 * \brief Type of texture mapping
 */
enum EngineTextureMapping
{
    ENG_TEX_MAPPING_X       = 1,
    ENG_TEX_MAPPING_Y       = 2,
    ENG_TEX_MAPPING_Z       = 3,
    ENG_TEX_MAPPING_1X      = 4,
    ENG_TEX_MAPPING_1Y      = 5,
    ENG_TEX_MAPPING_1Z      = 6
};


/**
 * \enum EngineMouseType
 * \brief Type of mouse cursor displayed in-game
 */
enum EngineMouseType
{
    //! Normal cursor (arrow)
    ENG_MOUSE_NORM      = 0,
    //! Busy
    ENG_MOUSE_WAIT      = 1,
    //! Edit (I-beam)
    ENG_MOUSE_EDIT      = 2,
    //! Hand
    ENG_MOUSE_HAND      = 3,
    //! Small cross
    ENG_MOUSE_CROSS     = 4,
    //! TODO: ?
    ENG_MOUSE_SHOW      = 5,
    //! Crossed out sign
    ENG_MOUSE_NO        = 6,
    //! Resize
    ENG_MOUSE_MOVE      = 7,
    //! Resize horizontally
    ENG_MOUSE_MOVEH     = 8,
    //! Resize vertically
    ENG_MOUSE_MOVEV     = 9,
    //! Resize diagonally bottom-left to top-right
    ENG_MOUSE_MOVED     = 10,
    //! Resize diagonally top-left to bottom-right
    ENG_MOUSE_MOVEI     = 11,
    //! Scroll to the left
    ENG_MOUSE_SCROLLL   = 12,
    //! Scroll to the right
    ENG_MOUSE_SCROLLR   = 13,
    //! Scroll up
    ENG_MOUSE_SCROLLU   = 14,
    //! Scroll down
    ENG_MOUSE_SCROLLD   = 15,
    //! Larger crosshair
    ENG_MOUSE_TARGET    = 16,

    //! Number of items in enum
    ENG_MOUSE_COUNT
};


/**
 * \class CEngine
 * \brief The graphics engine
 *
 * This is the main class for graphics engine. It is responsible for drawing the 3D scene,
 * setting various render states, and facilitating the drawing of 2D interface.
 *
 * It uses a lower-level CDevice object which is implementation-independent core engine.
 *
 * \section Scene 3D Scene
 *
 * The 3D scene is drawn with view coordinates set from camera position in 3D space and
 * a perspective projection matrix. The world matrix depends on the object drawn.
 * The coordinate system is left-handed: X axis is to the right, Y axis to the top and Z axis
 * is into the screen (Z = 0 is the sceen surface).
 *
 * In general, the 3D scene is composed of the following things:
 *  - sky background (gradient or texture image)
 *  - planets orbiting in the sky (drawn by CPlanet)
 *  - terrain - ground of the game level (part of engine objects)
 *  - main scene objects - robots, buildings, etc. (engine objects)
 *  - water/lava (drawn by CWater)
 *  - cloud layer (drawn by CCloud)
 *  - fire, lightning and particle effects (CPyro, CLightning and CParticle)
 *  - foreground image overlaid onto the scene at the end - for example, aiming crosshairs
 *  - 2D interface controls available in-game
 *  - mouse cursor
 *  - animated highlight box of the selected object(s)
 *
 * \section Interface 2D Interface
 *
 * The 2D interface is drawn in fixed XY coordinates, independent from window size.
 * Lower-left corner of the screen is (0,0) and upper-right corner is (1,1).
 * Matrices for world, view and projection are therefore fixed and never change.
 *
 * The class tracks the change of window coordinates and conversion functions
 * between the window and interface coordinates are provided.
 *
 * Interface drawing is delegated to CInterface class and particular controls
 * are instances of CControl class. The source code for these classes is in
 * src/ui directory.
 *
 * \section Objects Engine Objects
 *
 * The 3D scene is composed of objects which are basically collections of triangles forming
 * a surface or simply independent triangles in space.
 *
 * Objects are uniquely identified by object rank obtained at object creation. Creating an
 * object equals to allocating space for EngineObject structure which holds object parameters.
 *
 * Object's geometric data is stored as a separate object -- base engine object. Each object
 * must reference a valid base engine object or an empty base engine object (with rank = -1).
 * This many-to-one association allows to share same geometric data (e.g. from same model)
 * across objects.
 *
 * Base engine object data is stored in a 4-tier tree which splits the data describing triangles.
 *
 * The 4 tiers contain the following information:
 *  - level 1 (EngineBaseObject) - geometric statistics
 *  - level 2 (EngineBaseObjTexTier) - two textures (names and structs) applied to triangles,
 *  - level 3 (EngineBaseObjDataTier) - type of object*, material, render state and the actual vertex data
 *
 *  *NOTE: type of object in this context means only the internal type in 3D engine. It is not related
 *  to CObject types.
 *
 * Last tier containing vertex data contains also an ID of static buffer holding the data.
 * The static buffer is created and updated with new data as needed.
 *
 * Such tiered structure complicates loops over all object data, but saves a lot of memory and
 * optimizes the rendering process.
 *
 * \section Shadows Shadows
 *
 * Each engine object can be associated with a shadow (EngineShadow). Like objects, shadows are
 * identified by their rank obtained upon creation.
 *
 * Shadows are drawn as circular spots on the ground, except for shadows for worms, which have
 * special mode for them.
 *
 * \section RenderStates Render States
 *
 * Almost every primitive drawn on screen is drawn in state set through EngineRenderState enum.
 * In some functions, custom modes are still set, using CDevice's SetRenderState. However, it
 * will be subject to removal in the future. Generally, setting render states should be minimized
 * to avoid unnecessary overhead.
 *
 * Some states are clearly the result of legacy drawing and texturing methods. For example, TTEXTURE
 * states should really be removed and the textures changed to ones with alpha channel. In the future,
 * the whole modesetting code will probably be refactored to something more maintainable.
 *
 * \section Textures Textures
 *
 * Textures are loaded from a texture subdir in data directory. In the old code, textures were identified
 * by file name and loaded using some D3D util code. With new code and OpenGL backend, this approach is not
 * efficient - name comparison, etc. takes a lot of time. In the future, textures should be loaded once
 * at object creation time, and then referenced to as Texture structs, or even as unsigned int ID's
 * which is what OpenGL actually wants. The old method is kept for now, with mapping between texture names
 * and texture structs but it will also be subject to refactoring in the future.
 */
class CEngine : public CSingleton<CEngine>
{
public:
    CEngine(CApplication* app, CSystemUtils* systemUtils);
    ~CEngine();

    //! Sets the device to be used
    void            SetDevice(CDevice* device);
    //! Returns the current device
    CDevice*        GetDevice();

    //! Returns the text rendering engine
    CText*          GetText();
    COldModelManager* GetModelManager();
    CPyroManager*   GetPyroManager();
    //! Returns the light manager
    CLightManager*  GetLightManager();
    //! Returns the particle manager
    CParticle*      GetParticle();
    //! Returns the terrain manager
    CTerrain*       GetTerrain();
    //! Returns the water manager
    CWater*         GetWater();
    //! Returns the lighting manager
    CLightning*     GetLightning();
    //! Returns the planet manager
    CPlanet*        GetPlanet();
    //! Returns the fog manager
    CCloud*         GetCloud();

    //! Sets the terrain object
    void            SetTerrain(CTerrain* terrain);


    //! Performs the initialization; must be called after device was set
    bool            Create();
    //! Frees all resources before exit
    void            Destroy();


    //! Called once per frame, the call is the entry point for rendering
    void            Render();


    //! Processes incoming event
    bool            ProcessEvent(const Event& event);

    //! Called once per frame, the call is the entry point for animating the scene
    void            FrameUpdate();


    //! Writes a screenshot containing the current frame
    void            WriteScreenShot(const std::string& fileName);


    //@{
    //! Management of animation pause mode
    void            SetPause(bool pause);
    bool            GetPause();
    //@}

    //@{
    //! Management of displaying statistic information
    void            SetShowStats(bool show);
    bool            GetShowStats();
    //@}

    //! Enables/disables rendering
    void            SetRenderEnable(bool enable);

    void            SetRenderInterface(bool enable);
    bool            GetRenderInterface();

    //! Management of "screenshot mode" (disables interface particle rendering)
    //@{
    void            SetScreenshotMode(bool screenshotMode);
    bool            GetScreenshotMode();
    //@}

    //! Returns current size of viewport window
    Math::IntPoint   GetWindowSize();

    //@{
    //! Conversion functions between window and interface coordinates
    /** Window coordinates are from top-left (0,0) to bottom-right (w,h) - size of window
        Interface cords are from bottom-left (0,0) to top-right (1,1) - and do not depend on window size */
    Math::Point     WindowToInterfaceCoords(Math::IntPoint pos);
    Math::IntPoint  InterfaceToWindowCoords(Math::Point pos);
    //@}

    //@{
    //! Conversion functions between window and interface sizes
    /** Unlike coordinate conversions, this is only scale conversion, not translation and scale. */
    Math::Point      WindowToInterfaceSize(Math::IntPoint size);
    Math::IntPoint   InterfaceToWindowSize(Math::Point size);
    //@}

    //! Increments the triangle counter for the current frame
    void            AddStatisticTriangle(int count);
    //! Returns the number of triangles in current frame
    int             GetStatisticTriangle();

    //! Sets the coordinates to display in stats window
    void            SetStatisticPos(Math::Vector pos);

    //! Sets text to display as mission timer
    void            SetTimerDisplay(const std::string& text);


    /* *************** New mesh interface *************** */
    //! Add new instance of static mesh
    /**
     * Static meshes never change their geometry or texture mapping,
     * so specific instances can share mesh data.
     *
     * @param mesh mesh data
     * @param key key unique per object class
     * @return mesh instance handle
     */
    int AddStaticMesh(const std::string& key, const Gfx::CModelMesh* mesh, const Math::Matrix& worldMatrix);

    //! Removes given static mesh
    void DeleteStaticMesh(int meshHandle);

    //! Adds a shadow spot to mesh
    void AddStaticMeshShadowSpot(int meshHandle, const Gfx::ModelShadowSpot& shadowSpot);

    //! Returns static mesh world matrix
    const Math::Matrix& GetStaticMeshWorldMatrix(int meshHandle);

    //! Sets transparency for static mesh
    void SetStaticMeshTransparency(int meshHandle, float value);


    /* *************** Object management *************** */

    // Base objects

    //! Creates a base object and returns its rank
    int             CreateBaseObject();
    //! Deletes a base object
    void            DeleteBaseObject(int baseObjRank);
    //! Deletes all base objects
    void            DeleteAllBaseObjects();

    //! Copies geometry between two base objects
    void            CopyBaseObject(int sourceBaseObjRank, int destBaseObjRank);

    //! Adds triangles to given object with the specified params
    void AddBaseObjTriangles(int baseObjRank, const std::vector<Gfx::ModelTriangle>& triangles);

    //! Adds a tier 4 engine object directly
    void            AddBaseObjQuick(int baseObjRank, const EngineBaseObjDataTier& buffer,
                                    std::string tex1Name, std::string tex2Name,
                                    bool globalUpdate);

    // Objects

    //! Print debug info about an object
    void            DebugObject(int objRank);

    //! Creates a new object and returns its rank
    int             CreateObject();
    //! Deletes all objects, shadows and ground spots
    void            DeleteAllObjects();
    //! Deletes the given object
    void            DeleteObject(int objRank);

    //@{
    //! Management of the base object rank for engine object
    void            SetObjectBaseRank(int objRank, int baseObjRank);
    int             GetObjectBaseRank(int objRank);
    //@}

    //@{
    //! Management of engine object type
    void            SetObjectType(int objRank, EngineObjectType type);
    EngineObjectType GetObjectType(int objRank);
    //@}

    //@{
    //! Management of object transform
    void            SetObjectTransform(int objRank, const Math::Matrix& transform);
    void            GetObjectTransform(int objRank, Math::Matrix& transform);
    //@}

    //! Sets drawWorld for given object
    void            SetObjectDrawWorld(int objRank, bool draw);
    //! Sets drawFront for given object
    void            SetObjectDrawFront(int objRank, bool draw);

    //! Sets the transparency level for given object
    void            SetObjectTransparency(int objRank, float value);

    //! Returns the bounding box for an object
    void            GetObjectBBox(int objRank, Math::Vector& min, Math::Vector& max);

    //! Returns the total number of triangles of given object
    int             GetObjectTotalTriangles(int objRank);

    //! Returns the first found tier 4 engine object for the given params or nullptr if not found
    EngineBaseObjDataTier* FindTriangles(int objRank, const Material& material,
                                         int state, std::string tex1Name, std::string tex2Name);

    //! Returns a partial list of triangles for given object
    int             GetPartialTriangles(int objRank, float percent, int maxCount,
                                        std::vector<EngineTriangle>& triangles);

    //! Changes the 2nd texure for given object
    void            ChangeSecondTexture(int objRank, const std::string& tex2Name);

    //! Changes (recalculates) texture mapping for given object
    void            ChangeTextureMapping(int objRank, const Material& mat, int state,
                                         const std::string& tex1Name, const std::string& tex2Name,
                                         EngineTextureMapping mode,
                                         float au, float bu, float av, float bv);

    //! Changes texture mapping for robot tracks
    void            TrackTextureMapping(int objRank, const Material& mat, int state,
                                        const std::string& tex1Name, const std::string& tex2Name,
                                        EngineTextureMapping mode,
                                        float pos, float factor, float tl, float ts, float tt);

    //! Detects the target object that is selected with the mouse
    /** Returns the rank of the object or -1. */
    int             DetectObject(Math::Point mouse, Math::Vector& targetPos, bool terrain = false);

    //! Creates a shadow for the given object
    void            CreateShadowSpot(int objRank);
    //! Deletes the shadow for given object
    void            DeleteShadowSpot(int objRank);

    //@{
    //! Management of different shadow params
    void            SetObjectShadowSpotHide(int objRank, bool hide);
    void            SetObjectShadowSpotType(int objRank, EngineShadowType type);
    void            SetObjectShadowSpotPos(int objRank, const Math::Vector& pos);
    void            SetObjectShadowSpotAngle(int objRank, float angle);
    void            SetObjectShadowSpotRadius(int objRank, float radius);
    void            SetObjectShadowSpotIntensity(int objRank, float intensity);
    void            SetObjectShadowSpotHeight(int objRank, float height);
    void            UpdateObjectShadowSpotNormal(int objRank);
    //@}

    //! Lists the ranks of objects and subobjects selected
    void            SetHighlightRank(int* rankList);
    //! Returns the highlighted rectangle
    bool            GetHighlight(Math::Point& p1, Math::Point& p2);

    //! Deletes all ground spots
    void            DeleteAllGroundSpots();
    //! Creates a new ground spot and returns its rank
    int             CreateGroundSpot();
    //! Deletes the given ground spot
    void            DeleteGroundSpot(int rank);

    //@{
    //! Management of different ground spot params
    void            SetObjectGroundSpotPos(int rank, const Math::Vector& pos);
    void            SetObjectGroundSpotRadius(int rank, float radius);
    void            SetObjectGroundSpotColor(int rank, const Color& color);
    void            SetObjectGroundSpotMinMax(int rank, float min, float max);
    void            SetObjectGroundSpotSmooth(int rank, float smooth);
    //@}

    //! Creates the ground mark with the given params
    void            CreateGroundMark(Math::Vector pos, float radius,
                                     float delay1, float delay2, float delay3,
                                     int dx, int dy, char* table);
    //! Deletes the ground mark
    void            DeleteGroundMark(int rank);

    //! Updates the state after creating objects
    void            Update();


    /* *************** Mode setting *************** */

    //! Sets the current rendering state
    void            SetState(int state, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f));

    //! Sets the current material
    void            SetMaterial(const Material& mat);

    //! Specifies the location and direction of view
    void SetViewParams(const Math::Vector &eyePt, const Math::Vector &lookatPt, const Math::Vector &upVec);

    //! Updates the textures used for drawing ground spot
    void        UpdateGroundSpotTextures();

    //! Loads texture, creating it if not already present
    Texture         LoadTexture(const std::string& name);
    //! Loads texture from existing image
    Texture         LoadTexture(const std::string& name, CImage* image);
    //! Loads texture, creating it with given params if not already present
    Texture         LoadTexture(const std::string& name, const TextureCreateParams& params);
    //! Loads all necessary textures
    bool            LoadAllTextures();

    //! Changes colors in a texture
    //@{
    bool            ChangeTextureColor(const std::string& texName,
                                       const std::string& srcName,
                                       Color colorRef1, Color colorNew1,
                                       Color colorRef2, Color colorNew2,
                                       float tolerance1, float tolerance2,
                                       Math::Point ts, Math::Point ti,
                                       Math::Point *exclude = nullptr,
                                       float shift = 0.0f, bool hsv = false);
    bool            ChangeTextureColor(const std::string& texName,
                                       Color colorRef1, Color colorNew1,
                                       Color colorRef2, Color colorNew2,
                                       float tolerance1, float tolerance2,
                                       Math::Point ts, Math::Point ti,
                                       Math::Point *exclude = nullptr,
                                       float shift = 0.0f, bool hsv = false);
    //@}

    //! Sets texture for given stage; if not present in cache, the texture is loaded
    /** If loading fails, returns false. */
    bool            SetTexture(const std::string& name, int stage = 0);
    //! Sets texture for given stage
    void            SetTexture(const Texture& tex, int stage = 0);

    //! Deletes the given texture, unloading it and removing from cache
    void            DeleteTexture(const std::string& texName);
    //! Deletes the given texture, unloading it and removing from cache
    void            DeleteTexture(const Texture& tex);

    //! Creates or updates the given texture with given image data
    void            CreateOrUpdateTexture(const std::string& texName, CImage* img);

    //! Empties the texture cache
    void            FlushTextureCache();

    //! Defines of the distance field of vision
    void            SetTerrainVision(float vision);

    //@{
    //! Management of camera vertical field-of-view angle.
    /** This is specified in radians.
    Horizontal FoV is calculated based on vertical FoV and aspect ratio.
    0.75 = normal
    1.50 = wide-angle */
    void            SetFocus(float focus);
    //! Deprecated alias for GetVFovAngle
    float           GetFocus();
    float           GetVFovAngle();
    float           GetHFovAngle();
    //@}

    //@{
    //! Management of the global mode of contamination
    // NOTE: This is an user configuration setting
    void            SetDirty(bool mode);
    bool            GetDirty();
    //@}

    //@{
    //! Management of the global mode of horizontal fog patches
    // NOTE: This is an user configuration setting
    void            SetFog(bool mode);
    bool            GetFog();
    //@}

    //@{
    //! Management of the global mode of secondary texturing
    void            SetSecondTexture(const std::string& texNum);
    const std::string& GetSecondTexture();
    //@}

    //@{
    //! Management of view mode
    void            SetRankView(int rank);
    int             GetRankView();
    //@}

    //! Whether to draw the world
    void            SetDrawWorld(bool draw);

    //! Whether to draw the world on the interface
    void            SetDrawFront(bool draw);

    //@{
    //! Ambient color management
    void            SetAmbientColor(const Color& color, int rank = 0);
    Color           GetAmbientColor(int rank = 0);
    //@}

    //@{
    //! Color management under water
    void            SetWaterAddColor(const Color& color);
    Color           GetWaterAddColor();
    //@}

    //@{
    //! Management of the fog color
    void            SetFogColor(const Color& color, int rank = 0);
    Color           GetFogColor(int rank = 0);
    //@}

    //@{
    //! Management of the depth of field.
    /** Beyond this distance, nothing is visible.
        Shortly (according SetFogStart), one enters the fog. */
    void            SetDeepView(float length, int rank = 0, bool ref=false);
    float           GetDeepView(int rank = 0);
    //@}


    //@{
    //! Management the start of fog.
    /** With 0.0, the fog from the point of view (fog max).
        With 1.0, the fog from the depth of field (no fog). */
    void            SetFogStart(float start, int rank = 0);
    float           GetFogStart(int rank = 0);
    //@}

    //@{
    //! Management of the background image to use
    void            SetBackground(const std::string& name, Color up = Color(), Color down = Color(),
                                  Color cloudUp = Color(), Color cloudDown = Color(),
                                  bool full = false, bool scale = false);
    void            GetBackground(std::string& name, Color& up, Color& down,
                                  Color& cloudUp, Color& cloudDown,
                                  bool& full, bool& scale);
    //@}

    //! Specifies the name of foreground texture
    void            SetForegroundName(const std::string& name);
    //! Specifies whether to draw the foreground
    void            SetOverFront(bool front);
    //! Sets the foreground overlay color
    void            SetOverColor(const Color& color = Color(), int mode = ENG_RSTATE_TCOLOR_BLACK);

    //@{
    //! Management of the particle density
    // NOTE: This is an user configuration setting
    void            SetParticleDensity(float value);
    float           GetParticleDensity();
    //@}

    //! Adapts particle factor according to particle density
    float           ParticleAdapt(float factor);

    //@{
    //! Management of the distance of clipping.
    // NOTE: This is an user configuration setting
    void            SetClippingDistance(float value);
    float           GetClippingDistance();
    //@}

    //@{
    //! Management the texture filter mode
    // NOTE: This is an user configuration setting
    void            SetTextureFilterMode(TexFilter value);
    TexFilter       GetTextureFilterMode();
    //@}

    //@{
    //! Management the mipmap level for textures
    // NOTE: This is an user configuration setting
    void            SetTextureMipmapLevel(int value);
    int             GetTextureMipmapLevel();
    //@}

    //@{
    //! Management the anisotropy level for textures
    // NOTE: This is an user configuration setting
    void            SetTextureAnisotropyLevel(int value);
    int             GetTextureAnisotropyLevel();
    //@}

    //@{
    //! Management of shadow mapping
    // NOTE: These are user configuration settings
    bool            IsShadowMappingSupported();
    void            SetShadowMapping(bool value);
    bool            GetShadowMapping();
    void            SetShadowMappingOffscreen(bool value);
    bool            GetShadowMappingOffscreen();
    void            SetShadowMappingOffscreenResolution(int resolution);
    int             GetShadowMappingOffscreenResolution();
    bool            IsShadowMappingQualitySupported();
    void            SetShadowMappingQuality(bool value);
    bool            GetShadowMappingQuality();
    void            SetTerrainShadows(bool value);
    bool            GetTerrainShadows();
    //@}

    //@{
    //! Management of vertical synchronization
    // NOTE: This is an user configuration setting
    void            SetVSync(int value);
    int             GetVSync();
    //@}

    //@{
    //! Management of shadow color
    // NOTE: This is a setting configurable only in INI file
    void            SetShadowColor(float value);
    float           GetShadowColor();
    //@}

    //@{
    //! Management of shadow range
    // NOTE: This is a setting configurable only in INI file
    void            SetShadowRange(float value);
    float           GetShadowRange();
    //@}

    //@{
    //! Management of shadow range
    // NOTE: This is an user configuration setting
    void            SetMultiSample(int value);
    int             GetMultiSample();
    //@}

    //@{
    //! Management the mode of background
    void            SetBackForce(bool present);
    bool            GetBackForce();
    //@}

    //@{
    //! Managing the mode of dynamic lights.
    // NOTE: This is an user configuration setting
    void            SetLightMode(bool present);
    bool            GetLightMode();
    //@}

    //@{
    //! Management of the indentation mode while editing (CEdit)
    // NOTE: This is an user configuration setting
    // TODO: Move to CSettings
    void            SetEditIndentMode(bool autoIndent);
    bool            GetEditIndentMode();
    //@}

    //@{
    //! Management of tab indent when editing (CEdit)
    // NOTE: This is an user configuration setting
    // TODO: Move to CSettings
    void            SetEditIndentValue(int value);
    int             GetEditIndentValue();
    //@}

    //@{
    //! Management of precision of robot tracks
    void            SetTracePrecision(float factor);
    float           GetTracePrecision();
    //@}

    //@{
    //! Management of mouse cursor type
    void            SetMouseType(EngineMouseType type);
    EngineMouseType GetMouseType();
    //@}

    //@{
    //! Management of pause blur
    void            SetPauseBlurEnabled(bool enable);
    bool            GetPauseBlurEnabled();
    //@}

    //! Returns the view matrix
    const Math::Matrix& GetMatView();
    //! Returns the projection matrix
    const Math::Matrix& GetMatProj();
    //! Returns the camera center point
    TEST_VIRTUAL Math::Vector GetEyePt();
    //! Returns the camera target point
    TEST_VIRTUAL Math::Vector GetLookatPt();
    //! Returns the horizontal direction angle of view
    float           GetEyeDirH();
    //! Returns the vertical direction angle of view
    float           GetEyeDirV();
    //! Indicates whether a point is visible
    bool            IsVisiblePoint(const Math::Vector& pos);

    //! Resets the projection matrix after changes
    void            UpdateMatProj();

    //! Updates the scene after a change of parameters
    void            ApplyChange();

    void            RenderDebugSphere(const Math::Sphere&, const Math::Matrix& transform = Math::Matrix{}, const Color& = Color{0.0f, 0.0f, 1.0f, 1.0f});
    void            RenderDebugBox(const Math::Vector& mins, const Math::Vector& maxs, const Math::Matrix& transform = Math::Matrix{}, const Color& = Color{0.0f, 0.0f, 1.0f, 1.0f});

    void            SetDebugLights(bool debugLights);
    bool            GetDebugLights();
    void            DebugDumpLights();

    void            SetDebugResources(bool debugResources);
    bool            GetDebugResources();

    void            SetDebugGoto(bool debugGoto);
    bool            GetDebugGoto();
    void            AddDebugGotoLine(std::vector<Gfx::VertexCol> line);
    void            SetDebugGotoBitmap(std::unique_ptr<CImage> debugImage);

    void            SetWindowCoordinates();
    void            SetInterfaceCoordinates();

    void            EnablePauseBlur();
    void            DisablePauseBlur();

protected:
    //! Resets some states and flushes textures after device was changed (e.g. resoulution changed)
    /** Instead of calling this directly, send EVENT_RESOLUTION_CHANGED event **/
    void            ResetAfterVideoConfigChanged();

    //! Prepares the interface for 3D scene
    void        Draw3DScene();
    //! Capture the 3D scene for pause blur
    void        Capture3DScene();
    //! Draw the 3D scene capured for pause blur
    void        DrawCaptured3DScene();
    //! Renders shadow map
    void        RenderShadowMap();
    //! Enables or disables shadow mapping
    void        UseShadowMapping(bool enable);
    //! Enables or disables MSAA
    void        UseMSAA(bool enable);
    //! Draw 3D object
    void        DrawObject(const EngineBaseObjDataTier& p4);
    //! Draws the user interface over the scene
    void        DrawInterface();

    //! Draws old-style shadow spots
    void        DrawShadowSpots();
    //! Draws the gradient background
    void        DrawBackground();
    //! Draws the gradient background
    void        DrawBackgroundGradient(const Color& up, const Color& down);
    //! Draws the image background
    void        DrawBackgroundImage();
    //! Draws all the planets
    void        DrawPlanet();
    //! Draws the image foreground
    void        DrawForegroundImage();
    //! Draws the foreground color
    void        DrawOverColor();
    //! Draws the rectangle of the object highlighted
    void        DrawHighlight();
    //! Draws the mouse cursor
    void        DrawMouse();
    //! Draw part of mouse cursor sprite
    void        DrawMouseSprite(Math::IntPoint pos, Math::IntPoint size, int icon);
    //! Draw statistic texts
    void        DrawStats();
    //! Draw mission timer
    void        DrawTimer();
    void        RenderPendingDebugDraws();

    //! Creates a new tier 2 object (texture)
    EngineBaseObjTexTier&  AddLevel2(EngineBaseObject& p1, const std::string& tex1Name, const std::string& tex2Name);
    //! Creates a new tier 3 object (data)
    EngineBaseObjDataTier& AddLevel3(EngineBaseObjTexTier &p3, EngineTriangleType type,
                                     const Material& material, int state);

    //! Create texture and add it to cache
    Texture CreateTexture(const std::string &texName, const TextureCreateParams &params, CImage* image = nullptr);

    //! Tests whether the given object is visible
    bool        IsVisible(int objRank);

    //! Detects whether an object is affected by the mouse
    bool        DetectBBox(int objRank, Math::Point mouse);

    //! Compute and return the 2D box on screen of any object
    bool        GetBBox2D(int objRank, Math::Point& min, Math::Point& max);

    //! Detects whether the mouse is in a triangle.
    bool        DetectTriangle(Math::Point mouse, VertexTex2* triangle, int objRank, float& dist, Math::Vector& pos);

    //! Transforms a 3D point (x, y, z) in 2D space (x, y, -) of the window
    /** The coordinated p2D.z gives the distance. */
    bool        TransformPoint(Math::Vector& p2D, int objRank, Math::Vector p3D);

    //! Calculates the distances between the viewpoint and the origin of different objects
    void        ComputeDistance();

    //! Updates geometric parameters of objects (bounding box and radius)
    void        UpdateGeometry();

    //! Updates a given static buffer
    void        UpdateStaticBuffer(EngineBaseObjDataTier& p4);

    //! Updates static buffers of changed objects
    void        UpdateStaticBuffers();

    void            AddBaseObjTriangles(int baseObjRank, const std::vector<VertexTex2>& vertices,
                                        const Material& material, int state,
                                        std::string tex1Name, std::string tex2Name);

    int GetEngineState(const ModelTriangle& triangle);

    struct WriteScreenShotData
    {
        std::unique_ptr<CImage> img;
        std::string fileName;
    };
    static void WriteScreenShotThread(std::unique_ptr<WriteScreenShotData> data);

    //! Reloads all textures
    /** This additionally sends EVENT_RELOAD_TEXTURES to reload all textures not maintained by CEngine **/
    void ReloadAllTextures();

protected:
    CApplication*     m_app;
    CSystemUtils*     m_systemUtils;
    CSoundInterface*  m_sound;
    CDevice*          m_device;
    CTerrain*         m_terrain;
    std::unique_ptr<COldModelManager> m_modelManager;
    std::unique_ptr<CText>            m_text;
    std::unique_ptr<CLightManager>    m_lightMan;
    std::unique_ptr<CParticle>        m_particle;
    std::unique_ptr<CWater>           m_water;
    std::unique_ptr<CCloud>           m_cloud;
    std::unique_ptr<CLightning>       m_lightning;
    std::unique_ptr<CPlanet>          m_planet;
    std::unique_ptr<CPyroManager> m_pyroManager;

    //! Last encountered error
    std::string     m_error;

    SystemTimeStamp* m_lastFrameTime;
    SystemTimeStamp* m_currentFrameTime;
    int             m_fpsCounter;
    float           m_fps;

    //! Whether to show stats (FPS, etc)
    bool            m_showStats;
    //! Rendering enabled?
    bool            m_render;
    //! Render / hide the UI?
    bool            m_renderInterface;

    //! Screenshot mode?
    bool            m_screenshotMode;

    //! Projection matrix for 3D scene
    Math::Matrix    m_matProj;
    //! View matrix for 3D scene
    Math::Matrix    m_matView;
    //! Camera vertical field-of-view angle for 3D scene. A.k.a. m_vfov
    float           m_focus;
    //! Horizontal field-of-view angle, calculated from vertical FOV and aspect ratio
    float           m_hfov;

    //! Projection matrix for rendering shadow maps
    Math::Matrix    m_shadowProjMat;
    //! View matrix for rendering shadow maps
    Math::Matrix    m_shadowViewMat;
    //! Texture matrix for rendering shadow maps
    Math::Matrix    m_shadowTextureMat;
    //! Texture bias for sampling shadow maps
    Math::Matrix    m_shadowBias;

    //! Vertical synchronization controll
    int m_vsync;

    //! World matrix for 2D interface
    Math::Matrix    m_matWorldInterface;
    //! Projection matrix for 2D interface
    Math::Matrix    m_matProjInterface;
    //! View matrix for 2D interface
    Math::Matrix    m_matViewInterface;

    //! Current size of viewport window
    Math::IntPoint   m_size;

    //! Base objects (also level 1 tier list)
    std::vector<EngineBaseObject> m_baseObjects;
    //! Object parameters
    std::vector<EngineObject>     m_objects;
    //! Shadow list
    std::vector<EngineShadow>     m_shadowSpots;
    //! Ground spot list
    std::vector<EngineGroundSpot> m_groundSpots;
    //! Ground mark
    EngineGroundMark              m_groundMark;

    //! Location of camera
    Math::Vector    m_eyePt;
    //! Camera target
    Math::Vector    m_lookatPt;
    float           m_eyeDirH;
    float           m_eyeDirV;
    int             m_rankView;
    Color           m_ambientColor[2];
    Color           m_backColor[2];
    Color           m_fogColor[2];
    float           m_deepView[2];
    float           m_fogStart[2];
    Color           m_waterAddColor;
    int             m_statisticTriangle;
    Math::Vector    m_statisticPos;
    bool            m_updateGeometry;
    bool            m_updateStaticBuffers;
    bool            m_firstGroundSpot;
    std::string     m_secondTex;
    bool            m_backgroundFull;
    bool            m_backgroundScale;
    std::string     m_backgroundName;
    Texture         m_backgroundTex;
    Color           m_backgroundColorUp;
    Color           m_backgroundColorDown;
    Color           m_backgroundCloudUp;
    Color           m_backgroundCloudDown;
    bool            m_overFront;
    Color           m_overColor;
    int             m_overMode;
    std::string     m_foregroundName;
    Texture         m_foregroundTex;
    bool            m_drawWorld;
    bool            m_drawFront;
    float           m_terrainVision;
    bool            m_backForce;
    float           m_tracePrecision;
    bool            m_pauseBlurEnabled;

    bool            m_dirty;
    bool            m_fog;
    float           m_particleDensity;
    float           m_clippingDistance;
    bool            m_lightMode;
    bool            m_editIndentMode;
    int             m_editIndentValue;

    Texture         m_shadowMap;

    struct PendingDebugDraw
    {
        std::vector<VertexCol> vertices;
        std::vector<int> firsts;
        std::vector<int> counts;
    }
    m_pendingDebugDraws;

    //! Ranks of highlighted objects
    int             m_highlightRank[100];
    //! Highlight visible?
    bool            m_highlight;
    //! Time counter for highlight animation
    float           m_highlightTime;
    //@{
    //! Highlight rectangle points
    Math::Point     m_highlightP1;
    Math::Point     m_highlightP2;
    //@}

    //! Default texture create params
    TextureCreateParams m_defaultTexParams;
    //! Create params for terrain textures
    TextureCreateParams m_terrainTexParams;
    //! Requested texture mipmap level
    int m_textureMipmapLevel;
    //! Requested texture anisotropy level
    int m_textureAnisotropy;
    //! true if shadow mapping enabled
    bool m_shadowMapping;
    //! true enables offscreen shadow rendering
    bool m_offscreenShadowRendering;
    //! Offscreen shadow rendering resolution
    int m_offscreenShadowRenderingResolution;
    //! true enables higher quality shadows
    bool m_qualityShadows;
    //! true enables casting shadows by terrain
    bool m_terrainShadows;
    //! Shadow color
    float m_shadowColor;
    //! Shadow range
    float m_shadowRange;
    //! Number of samples for multisample rendering
    int m_multisample;

    //! Map of loaded textures (by name)
    std::map<std::string, Texture> m_texNameMap;
    //! Reverse map of loaded textures (by texture)
    std::map<Texture, std::string> m_revTexNameMap;
    //! Blacklist map of textures
    /** Textures on this list were not successful in first loading,
     *  so are disabled for subsequent load calls. */
    std::set<std::string> m_texBlacklist;

    //! Texture with mouse cursors
    Texture         m_miceTexture;
    //! Type of mouse cursor
    EngineMouseType m_mouseType;

    //! Last engine render state (-1 at the beginning of frame)
    int             m_lastState;
    //! Last color set with render state
    Color           m_lastColor;
    //! Last texture names for 2 used texture stages
    std::string     m_lastTexture[2];
    //! Last material
    Material        m_lastMaterial;

    //! True when drawing 2D UI
    bool            m_interfaceMode;

    bool            m_debugLights;
    bool            m_debugDumpLights;
    bool            m_debugCrashSpheres = false;
    bool            m_debugResources = false;
    bool            m_debugGoto = false;

    std::string     m_timerText;

    std::unordered_map<std::string, int> m_staticMeshBaseObjects;

    std::vector<std::vector<VertexCol>> m_displayGoto;
    std::unique_ptr<CImage> m_displayGotoImage;

    //! Pause the animation updates
    bool            m_pause = false;

    //! true means that current 3D scene was captured and is not to be rendered again
    bool            m_worldCaptured = false;
    //! true means that currently rendered world is to be captured
    bool            m_captureWorld = false;
    //! Texture with captured 3D world
    Texture         m_capturedWorldTexture;
};


} // namespace Gfx
