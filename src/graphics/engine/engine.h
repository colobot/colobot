// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX&  EPSITEC SA, www.epsitec.ch
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
 * \file graphics/engine/engine.h
 * \brief Main graphics engine - CEngine class
 */

#pragma once


#include "app/system.h"
#include "app/pausemanager.h"

#include "common/event.h"
#include "common/singleton.h"

#include "graphics/core/color.h"
#include "graphics/core/material.h"
#include "graphics/core/texture.h"
#include "graphics/core/vertex.h"

#include "graphics/engine/modelfile.h"

#include "math/intpoint.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"


#include <string>
#include <vector>
#include <map>
#include <set>


class CApplication;
class CObject;
class CSoundInterface;
class CImage;


// Graphics module namespace
namespace Gfx {


class CDevice;
class CLightManager;
class CText;
class CParticle;
class CWater;
class CCloud;
class CLightning;
class CPlanet;
class CTerrain;


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
    ENG_RSTATE_OPAQUE_COLOR     = (1<<20)
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
    int            state;
    //! 1st texture
    std::string    tex1Name;
    //! 2nd texture
    std::string    tex2Name;

    inline EngineTriangle()
    {
        state = ENG_RSTATE_NORMAL;
    }
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
 * \brief Tier 4 of object tree (data)
 */
struct EngineBaseObjDataTier
{
    EngineTriangleType      type;
    Material                material;
    int                     state;
    std::vector<VertexTex2> vertices;
    unsigned int            staticBufferId;
    bool                    updateStaticBuffer;

    inline EngineBaseObjDataTier(EngineTriangleType _type = ENG_TRIANGLE_TYPE_TRIANGLES,
                                 const Material& _material = Material(),
                                 int _state = ENG_RSTATE_NORMAL)
     : type(_type)
     , material(_material)
     , state(_state)
     , staticBufferId(0)
     , updateStaticBuffer(false)
    {}
};

/**
 * \struct EngineBaseObjLODTier
 * \brief Tier 3 of base object tree (LOD)
 */
struct EngineBaseObjLODTier
{
    LODLevel                            lodLevel;
    std::vector<EngineBaseObjDataTier>  next;

    inline EngineBaseObjLODTier(LODLevel _lodLevel = LOD_Constant)
     : lodLevel(_lodLevel)
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
    std::vector<EngineBaseObjLODTier>  next;

    inline EngineBaseObjTexTier(const std::string& _tex1Name = "", const std::string& _tex2Name = "")
     : tex1Name(_tex1Name)
     , tex2Name(_tex2Name)
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
    bool used;
    //! Number of triangles
    int                    totalTriangles;
    //! Bounding box min (origin 0,0,0 always included)
    Math::Vector           bboxMin;
    //! bounding box max (origin 0,0,0 always included)
    Math::Vector           bboxMax;
    //! Radius of the sphere at the origin
    float                  radius;
    //! Next tier (LOD)
    std::vector<EngineBaseObjTexTier> next;

    inline EngineBaseObject()
    {
        LoadDefault();
    }

    inline void LoadDefault()
    {
        used = false;
        totalTriangles = 0;
        bboxMax.LoadZero();
        bboxMin.LoadZero();
        radius = 0.0f;
    }
};

/**
 * \struct EngineObject
 * \brief Object drawn by the graphics engine
 */
struct EngineObject
{
    //! If true, object is valid in objects vector
    bool                   used;
    //! Rank of associated base engine object
    int                    baseObjRank;
    //! If true, the object is drawn
    bool                   visible;
    //! If true, object is behind the 2D interface
    bool                   drawWorld;
    //! If true, the shape is before the 2D interface
    bool                   drawFront;
    //! Type of object
    EngineObjectType       type;
    //! Transformation matrix
    Math::Matrix           transform;
    //! Distance to object from eye point
    float                  distance;
    //! Rank of the associated shadow
    int                    shadowRank;
    //! Transparency of the object [0, 1]
    float                  transparency;

    //! Calls LoadDefault()
    inline EngineObject()
    {
        LoadDefault();
    }

    //! Loads default values
    inline void LoadDefault()
    {
        used = false;
        baseObjRank = -1;
        visible = false;
        drawWorld = false;
        drawFront = false;
        type = ENG_OBJTYPE_NULL;
        transform.LoadIdentity();
        distance = 0.0f;
        shadowRank = -1;
        transparency = 0.0f;
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
    bool                used;
    //! If true, shadow is invisible (object being carried for example)
    bool                hide;
    //! Rank of the associated object
    int                 objRank;
    //! Type of shadow
    EngineShadowType type;
    //! Position of the shadow
    Math::Vector        pos;
    //! Normal to the terrain
    Math::Vector        normal;
    //! Angle of the shadow
    float               angle;
    //! Radius of the shadow
    float               radius;
    //! Intensity of the shadow
    float               intensity;
    //! Height from the ground
    float               height;

    inline EngineShadow()
    {
        LoadDefault();
    }

    inline void LoadDefault()
    {
        used = false;
        hide = false;
        objRank = 0;
        type = ENG_SHADOW_NORM;
        pos.LoadZero();
        normal.LoadZero();
        angle = radius = intensity = height = 0.0f;
    }
};

/**
 * \struct EngineGroundSpot
 * \brief A spot (large shadow) drawn on the ground by the graphics engine
 */
struct EngineGroundSpot
{
    //! If true, ground spot is valid
    bool            used;
    //! Color of the shadow
    Color      color;
    //! Min altitude
    float           min;
    //! Max altitude
    float           max;
    //! Transition area
    float           smooth;
    //! Position for the shadow
    Math::Vector    pos;
    //! Radius of the shadow
    float           radius;
    //! Position of the shadow drawn
    Math::Vector    drawPos;
    //! Radius of the shadow drawn
    float           drawRadius;

    inline EngineGroundSpot()
    {
        LoadDefault();
    }

    inline void LoadDefault()
    {
        used = false;
        color = Color();
        pos.LoadZero();
        drawPos.LoadZero();
        min = max = smooth = radius = drawRadius = 0.0f;
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
    bool                        draw;
    //! Phase of life
    EngineGroundMarkPhase  phase;
    //! Times for 3 life phases
    float                       delay[3];
    //! Fixed time
    float                       fix;
    //! Position for marks
    Math::Vector                pos;
    //! Radius of marks
    float                       radius;
    //! Color intensity
    float                       intensity;
    //! Draw position for marks
    Math::Vector                drawPos;
    //! Radius for  marks
    float                       drawRadius;
    //! Draw intensity for marks
    float                       drawIntensity;
    //! X dimension of table
    int                         dx;
    //! Y dimension of table
    int                         dy;
    //! Pointer to the table
    char*                       table;

    inline EngineGroundMark()
    {
        LoadDefault();
    }

    inline void LoadDefault()
    {
        draw = false;
        phase = ENG_GR_MARK_PHASE_NULL;
        pos = Math::Vector();
        drawPos = Math::Vector();
        delay[0] = delay[1] = delay[2] = 0.0f;
        fix = radius = intensity = drawRadius = drawIntensity = 0.0f;
        dx = dy = 0;
        table = nullptr;
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
 * \struct EngineMouse
 * \brief Information about mouse cursor
 */
struct EngineMouse
{
    //! Index of texture element for 1st image
    int icon1;
    //! Index of texture element for 2nd image
    int icon2;
    //! Shadow texture part
    int iconShadow;
    //! Mode to render 1st image in
    EngineRenderState mode1;
    //! Mode to render 2nd image in
    EngineRenderState mode2;
    //! Hot point
    Math::Point hotPoint;

    inline EngineMouse(int _icon1 = -1, int _icon2 = -1, int _iconShadow = -1,
                       EngineRenderState _mode1 = ENG_RSTATE_NORMAL,
                       EngineRenderState _mode2 = ENG_RSTATE_NORMAL,
                       Math::Point _hotPoint = Math::Point())
     : icon1(_icon1)
     , icon2(_icon2)
     , iconShadow(_iconShadow)
     , mode1(_mode1)
     , mode2(_mode2)
     , hotPoint(_hotPoint)
    {}
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
 *  - level 3 (EngineBaseObjLODTier) - minumum and maximum LOD (=level of detail)
 *  - level 4 (EngineBaseObjDataTier) - type of object*, material, render state and the actual vertex data
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
    CEngine(CApplication* app);
    ~CEngine();

    //! Sets the device to be used
    void            SetDevice(CDevice* device);
    //! Returns the current device
    CDevice*        GetDevice();

    //! Returns the text rendering engine
    CText*          GetText();
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

    //! Resets some states and flushes textures after device was changed (e.g. resoulution changed)
    void            ResetAfterDeviceChanged();


    //! Called once per frame, the call is the entry point for rendering
    void            Render();


    //! Processes incoming event
    bool            ProcessEvent(const Event& event);

    //! Called once per frame, the call is the entry point for animating the scene
    void            FrameUpdate();


    //! Writes a screenshot containing the current frame
    bool            WriteScreenShot(const std::string& fileName, int width, int height);


    //! Get pause mode
    TEST_VIRTUAL bool GetPause();

    //@{
    //! Management of lock for the duration of movie sequence
    void            SetMovieLock(bool lock);
    bool            GetMovieLock();
    //@}

    //@{
    //! Management of displaying statistic information
    void            SetShowStats(bool show);
    bool            GetShowStats();
    //@}

    //! Enables/disables rendering
    void            SetRenderEnable(bool enable);

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
    void            AddStatisticTriangle(int nb);
    //! Returns the number of triangles in current frame
    int             GetStatisticTriangle();


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
    void            AddBaseObjTriangles(int baseObjRank, const std::vector<VertexTex2>& vertices,
                                        EngineTriangleType triangleType,
                                        const Material& material, int state,
                                        std::string tex1Name, std::string tex2Name,
                                        LODLevel lodLevel, bool globalUpdate);

    //! Adds a tier 4 engine object directly
    void            AddBaseObjQuick(int baseObjRank, const EngineBaseObjDataTier& buffer,
                                    std::string tex1Name, std::string tex2Name,
                                    LODLevel lodLevel, bool globalUpdate);

    // Objects

    //! Print debug info about an object
    void            DebugObject(int rank);

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
                                         int state, std::string tex1Name, std::string tex2Name,
                                         int lodLevelMask);

    //! Returns a partial list of triangles for given object
    int             GetPartialTriangles(int objRank, int lodLevelMask, float percent, int maxCount,
                                        std::vector<EngineTriangle>& triangles);

    //! Changes the 2nd texure for given object
    void            ChangeSecondTexture(int objRank, const std::string& tex2Name);

    //! Changes (recalculates) texture mapping for given object
    void            ChangeTextureMapping(int objRank, const Material& mat, int state,
                                         const std::string& tex1Name, const std::string& tex2Name,
                                         int lodLevelMask, EngineTextureMapping mode,
                                         float au, float bu, float av, float bv);

    //! Changes texture mapping for robot tracks
    void            TrackTextureMapping(int objRank, const Material& mat, int state,
                                        const std::string& tex1Name, const std::string& tex2Name,
                                        int lodLevelMask, EngineTextureMapping mode,
                                        float pos, float factor, float tl, float ts, float tt);

    //! Detects the target object that is selected with the mouse
    /** Returns the rank of the object or -1. */
    int             DetectObject(Math::Point mouse);

    //! Creates a shadow for the given object
    void            CreateShadow(int objRank);
    //! Deletes the shadow for given object
    void            DeleteShadow(int objRank);

    //@{
    //! Management of different shadow params
    void            SetObjectShadowHide(int objRank, bool hide);
    void            SetObjectShadowType(int objRank, EngineShadowType type);
    void            SetObjectShadowPos(int objRank, const Math::Vector& pos);
    void            SetObjectShadowNormal(int objRank, const Math::Vector& normal);
    void            SetObjectShadowAngle(int objRank, float angle);
    void            SetObjectShadowRadius(int objRank, float radius);
    void            SetObjectShadowIntensity(int objRank, float intensity);
    void            SetObjectShadowHeight(int objRank, float height);
    float           GetObjectShadowRadius(int objRank);
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
    void            SetViewParams(const Math::Vector& eyePt, const Math::Vector& lookatPt,
                                  const Math::Vector& upVec, float eyeDistance);

    //! Loads texture, creating it if not already present
    Texture         LoadTexture(const std::string& name);
    //! Loads texture from existing image
    Texture         LoadTexture(const std::string& name, CImage* image);
    //! Loads texture, creating it with given params if not already present
    Texture         LoadTexture(const std::string& name, const TextureCreateParams& params);
    //! Loads all necessary textures
    bool            LoadAllTextures();

    //! Changes colors in a texture
    bool            ChangeTextureColor(const std::string& texName,
                                       Color colorRef1, Color colorNew1,
                                       Color colorRef2, Color colorNew2,
                                       float tolerance1, float tolerance2,
                                       Math::Point ts, Math::Point ti,
                                       Math::Point *exclude = nullptr,
                                       float shift = 0.0f, bool hsv = false);

    //! Sets texture for given stage; if not present in cache, the texture is loaded
    /** If loading fails, returns false. */
    bool            SetTexture(const std::string& name, int stage = 0);
    //! Sets texture for given stage
    void            SetTexture(const Texture& tex, int stage = 0);

    //! Deletes the given texture, unloading it and removing from cache
    void            DeleteTexture(const std::string& name);
    //! Deletes the given texture, unloading it and removing from cache
    void            DeleteTexture(const Texture& tex);

    //! Empties the texture cache
    void            FlushTextureCache();

    //! Defines of the distance field of vision
    void            SetTerrainVision(float vision);

    //@{
    //! Management of camera angle
    /**
    0.75 = normal
    1.50 = wide-angle */
    void            SetFocus(float focus);
    float           GetFocus();
    //@}

    //@{
    //! Management of the global mode of marking
    void            SetGroundSpot(bool mode);
    bool            GetGroundSpot();
    //@}

    //@{
    //! Management of the global mode of shading
    void            SetShadow(bool mode);
    bool            GetShadow();
    //@}

    //@{
    //! Management of the global mode of contamination
    void            SetDirty(bool mode);
    bool            GetDirty();
    //@}

    //@{
    //! Management of the global mode of horizontal fog patches
    void            SetFog(bool mode);
    bool            GetFog();
    //@}

    //@{
    //! Management of the global mode of secondary texturing
    void            SetSecondTexture(int texNum);
    int             GetSecondTexture();
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
                                  bool full = false);
    void            GetBackground(std::string& name, Color& up, Color& down,
                                  Color& cloudUp, Color& cloudDown,
                                  bool& full);
    //@}

    //! Specifies the name of foreground texture
    void            SetForegroundName(const std::string& name);
    //! Specifies whether to draw the foreground
    void            SetOverFront(bool front);
    //! Sets the foreground overlay color
    void            SetOverColor(const Color& color = Color(), int mode = ENG_RSTATE_TCOLOR_BLACK);

    //@{
    //! Management of the particle density
    void            SetParticleDensity(float value);
    float           GetParticleDensity();
    //@}

    //! Adapts particle factor according to particle density
    float           ParticleAdapt(float factor);

    //@{
    //! Management of the distance of clipping.
    void            SetClippingDistance(float value);
    float           GetClippingDistance();
    //@}

    //@{
    //! Management of objects detals.
    void            SetObjectDetail(float value);
    float           GetObjectDetail();
    //@}

    //@{
    //! The amount of management objects gadgets
    void            SetGadgetQuantity(float value);
    float           GetGadgetQuantity();
    //@}

    //@{
    //! Management the quality of textures
    void            SetTextureQuality(int value);
    int             GetTextureQuality();
    //@}

    //@{
    //! Management mode of toto
    void            SetTotoMode(bool present);
    bool            GetTotoMode();
    //@}

    //@{
    //! Management the mode of foreground
    void            SetLensMode(bool present);
    bool            GetLensMode();
    //@}

    //@{
    //! Management the mode of water
    void            SetWaterMode(bool present);
    bool            GetWaterMode();
    //@}

    void            SetLightingMode(bool present);
    bool            GetLightingMode();

    //@{
    //! Management the mode of sky
    void            SetSkyMode(bool present);
    bool            GetSkyMode();
    //@}

    //@{
    //! Management the mode of background
    void            SetBackForce(bool present);
    bool            GetBackForce();
    //@}

    //@{
    //! Management the mode of planets
    void            SetPlanetMode(bool present);
    bool            GetPlanetMode();
    //@}

    //@{
    //! Managing the mode of dynamic lights.
    void            SetLightMode(bool present);
    bool            GetLightMode();
    //@}

    //@{
    // TODO: move to more appropriate class ?
    //! Management of the indentation mode while editing (CEdit)
    void            SetEditIndentMode(bool autoIndent);
    bool            GetEditIndentMode();
    //@}

    //@{
    // TODO: move to more appropriate class ?
    //! Management of tab indent when editing (CEdit)
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

    //! Returns the view matrix
    const Math::Matrix& GetMatView();
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

protected:
    //! Prepares the interface for 3D scene
    void        Draw3DScene();
    //! Draw 3D object
    void        DrawObject(const EngineBaseObjDataTier& p4);
    //! Draws the user interface over the scene
    void        DrawInterface();

    //! Updates the textures used for drawing ground spot
    void        UpdateGroundSpotTextures();

    //! Draws shadows
    void        DrawShadow();
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
    void        DrawMouseSprite(Math::Point pos, Math::Point dim, int icon);
    //! Draw statistic texts
    void        DrawStats();

    //! Creates a new tier 2 object (texture)
    EngineBaseObjTexTier&  AddLevel2(EngineBaseObject& p1, const std::string& tex1Name, const std::string& tex2Name);
    //! Creates a new tier 3 object (LOD)
    EngineBaseObjLODTier&  AddLevel3(EngineBaseObjTexTier &p2, LODLevel lodLevel);
    //! Creates a new tier 4 object (data)
    EngineBaseObjDataTier& AddLevel4(EngineBaseObjLODTier &p3, EngineTriangleType type,
                                     const Material& mat, int state);

    //! Create texture and add it to cache
    Texture CreateTexture(const std::string &texName, const TextureCreateParams &params, CImage* image = nullptr);

    //! Tests whether the given object is visible
    bool        IsVisible(int objRank);

    //! Checks whether the given distance is within LOD min & max limit
    bool        IsWithinLODLimit(float distance, LODLevel lodLevel);

    //! Detects whether an object is affected by the mouse
    bool        DetectBBox(int objRank, Math::Point mouse);

    //! Compute and return the 2D box on screen of any object
    bool        GetBBox2D(int objRank, Math::Point& min, Math::Point& max);

    //! Detects whether the mouse is in a triangle.
    bool        DetectTriangle(Math::Point mouse, VertexTex2* triangle, int objRank, float& dist);

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

protected:
    CApplication*     m_app;
    CSoundInterface*  m_sound;
    CDevice*          m_device;
    CText*            m_text;
    CLightManager*    m_lightMan;
    CParticle*        m_particle;
    CWater*           m_water;
    CCloud*           m_cloud;
    CLightning*       m_lightning;
    CPlanet*          m_planet;
    CTerrain*         m_terrain;
    CPauseManager*    m_pause;

    //! Last encountered error
    std::string     m_error;

    SystemTimeStamp* m_lastFrameTime;
    SystemTimeStamp* m_currentFrameTime;
    int             m_fpsCounter;
    float           m_fps;

    //! Whether to show stats (FPS, etc)
    bool            m_showStats;
    std::string     m_fpsText;
    //! Rendering enabled?
    bool            m_render;
    //! Lock for duration of movie?
    bool            m_movieLock;

    //! Projection matrix for 3D scene
    Math::Matrix    m_matProj;
    //! View matrix for 3D scene
    Math::Matrix    m_matView;
    //! Camera angle for 3D scene
    float           m_focus;

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
    std::vector<EngineShadow>     m_shadows;
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
    bool            m_updateGeometry;
    bool            m_updateStaticBuffers;
    int             m_alphaMode;
    bool            m_groundSpotVisible;
    bool            m_shadowVisible;
    bool            m_dirty;
    bool            m_fog;
    bool            m_firstGroundSpot;
    int             m_secondTexNum;
    bool            m_backgroundFull;
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
    float           m_particleDensity;
    float           m_clippingDistance;
    float           m_lastClippingDistance;
    float           m_objectDetail;
    float           m_terrainVision;
    float           m_gadgetQuantity;
    int             m_textureQuality;
    bool            m_totoMode;
    bool            m_lensMode;
    bool            m_waterMode;
    bool            m_skyMode;
    bool            m_backForce;
    bool            m_planetMode;
    bool            m_lightMode;
    bool            m_editIndentMode;
    int             m_editIndentValue;
    float           m_tracePrecision;

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

    //! Map of loaded textures (by name)
    std::map<std::string, Texture> m_texNameMap;
    //! Reverse map of loaded textures (by texture)
    std::map<Texture, std::string> m_revTexNameMap;
    //! Blacklist map of textures
    /** Textures on this list were not successful in first loading,
     *  so are disabled for subsequent load calls. */
    std::set<std::string> m_texBlacklist;

    //! Mouse cursor definitions
    EngineMouse     m_mice[ENG_MOUSE_COUNT];
    //! Texture with mouse cursors
    Texture         m_miceTexture;
    //! Size of mouse cursor
    Math::Point     m_mouseSize;
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
};


} // namespace Gfx

