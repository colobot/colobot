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
 * \brief Main graphics engine - Gfx::CEngine class
 */

#pragma once

#include "app/system.h"
#include "common/event.h"
#include "graphics/core/color.h"
#include "graphics/core/material.h"
#include "graphics/core/texture.h"
#include "graphics/core/vertex.h"
#include "math/intpoint.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"


#include <string>
#include <vector>
#include <map>
#include <set>


class CApplication;
class CInstanceManager;
class CObject;
class CSoundInterface;


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
  \enum EngineTriangleType
  \brief Type of triangles drawn for engine objects */
enum EngineTriangleType
{
    //! Triangles
    ENG_TRIANGLE_TYPE_6T        = 1,
    //! Surfaces
    ENG_TRIANGLE_TYPE_6S        = 2
};

/**
  \struct EngineTriangle
  \brief A triangle drawn by the graphics engine */
struct EngineTriangle
{
    //! Triangle vertices
    Gfx::VertexTex2     triangle[3];
    //! Material
    Gfx::Material       material;
    //! Render state
    int                 state;
    //! 1st texture
    Gfx::Texture        tex1;
    //! 2nd texture
    Gfx::Texture        tex2;

    EngineTriangle()
    {
        state = 0;
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
    ENG_OBJTYPE_VEHICULE    = 3,
    //! Part of a moving object
    ENG_OBJTYPE_DESCENDANT  = 4,
    //! Fixed object type quartz
    ENG_OBJTYPE_QUARTZ      = 5,
    //! Fixed object type metal
    ENG_OBJTYPE_METAL       = 6
};

/**
  \struct EngineObject
  \brief Object drawn by the graphics engine */
struct EngineObject
{
    //! If true, the object is drawn
    bool                   visible;
    //! If true, object is behind the 2D interface
    bool                   drawWorld;
    //! If true, the shape is before the 2D interface
    bool                   drawFront;
    //! Number of triangles
    int                    totalTriangles;
    //! Type of object
    Gfx::EngineObjectType  type;
    //! Transformation matrix
    Math::Matrix           transform;
    //! Distance view - origin (TODO: ?)
    float                  distance;
    //! Bounding box min (origin 0,0,0 always included)
    Math::Vector           bboxMin;
    //! bounding box max (origin 0,0,0 always included)
    Math::Vector           bboxMax;
    //! Radius of the sphere at the origin
    float                  radius;
    //! Rank of the associated shadow
    int                    shadowRank;
    //! Transparency of the object [0, 1]
    float                  transparency;

    EngineObject()
    {
        visible = false;
        drawWorld = false;
        drawFront = false;
        totalTriangles = 0;
        distance = 0.0f;
        radius = 0.0f;
        shadowRank = 0;
        transparency = 0.0f;
    }
};

struct EngineObjLevel1;
struct EngineObjLevel2;
struct EngineObjLevel3;
struct EngineObjLevel4;
struct EngineObjLevel5;

/**
  \struct EngineObjLevel5
  \brief Tier 5 of object tree */
struct EngineObjLevel5
{
    Gfx::Material                material;
    int                          state;
    Gfx::EngineTriangleType      type;
    std::vector<Gfx::VertexTex2> vertices;

    EngineObjLevel5()
    {
        state = 0;
    }
};

/**
  \struct EngineObjLevel4
  \brief Tier 4 of object tree */
struct EngineObjLevel4
{
    int                                reserved;
    std::vector<Gfx::EngineObjLevel5>  up;
    Gfx::EngineObjLevel3*              down;

    EngineObjLevel4()
    {
        reserved = 0;
    }
};

/**
  \struct EngineObjLevel3
  \brief Tier 3 of object tree */
struct EngineObjLevel3
{
    float                              min;
    float                              max;
    std::vector<Gfx::EngineObjLevel4>  up;
    Gfx::EngineObjLevel2*              down;

    EngineObjLevel3()
    {
        min = max = 0.0f;
    }
};

/**
  \struct EngineObjLevel2
  \brief Tier 2 of object tree */
struct EngineObjLevel2
{
    int                                objRank;
    std::vector<Gfx::EngineObjLevel3>  up;
    Gfx::EngineObjLevel1*              down;

    EngineObjLevel2()
    {
        objRank = 0;
    }
};

/**
  \struct EngineObjLevel1
  \brief Tier 1 of object tree */
struct EngineObjLevel1
{
    Gfx::Texture                       tex1;
    Gfx::Texture                       tex2;
    std::vector<Gfx::EngineObjLevel2>  up;

    EngineObjLevel1() {}
};

/**
  \struct EngineShadowType
  \brief Type of shadow drawn by the graphics engine */
enum EngineShadowType
{
    //! Normal shadow
    ENG_SHADOW_NORM = 0,
    //! TODO: ?
    ENG_SHADOW_WORM = 1
};

/**
  \struct EngineShadow
  \brief Shadow drawn by the graphics engine */
struct EngineShadow
{
    //! If true, shadow is invisible (object being carried for example)
    bool                hide;
    //! Rank of the associated object
    int                 objRank;
    //! Type of shadow
    Gfx::EngineShadowType type;
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

    EngineShadow()
    {
        hide = false;
        objRank = 0;
        angle = radius = intensity = height = 0.0f;
    }
};

/**
  \struct EngineGroundSpot
  \brief A spot (large shadow) drawn on the ground by the graphics engine */
struct EngineGroundSpot
{
    //! Color of the shadow
    Gfx::Color      color;
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

    EngineGroundSpot()
    {
        min = max = smooth = radius = drawRadius = 0.0f;
    }
};

/**
  \enum EngineGroundMarkPhase
  \brief Phase of life of an EngineGroundMark */
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
  \struct EngineGroundMark
  \brief A mark on ground drawn by the graphics engine */
struct EngineGroundMark
{
    //! If true, draw mark
    bool                        draw;
    //! Phase of life
    Gfx::EngineGroundMarkPhase  phase;
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

    EngineGroundMark()
    {
        draw = false;
        delay[0] = delay[1] = delay[2] = 0.0f;
        fix = radius = intensity = drawRadius = drawIntensity = 0.0f;
        dx = dy = 0;
        table = NULL;
    }
};

/**
  \enum EngineTextureMapping
  \brief Type of texture mapping
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
  \enum EngineRenderState
  \brief Render state of graphics engine

  States are used for settings certain modes, for instance texturing and blending.
  The enum is a bitmask and some of the states can be OR'd together. */
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
    ENG_RSTATE_OPAQUE_TEXTURE     = (1<<19),
    //! Only opaque color, no texture, blending, etc.
    ENG_RSTATE_OPAQUE_COLOR     = (1<<20)
};


/**
  \enum EngineMouseType
  \brief Type of mouse cursor displayed in-game */
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
  \struct EngineMouse
  \brief Information about mouse cursor */
struct EngineMouse
{
    //! Index of texture element for 1st image
    int icon1;
    //! Index of texture element for 2nd image
    int icon2;
    //! Shadow texture part
    int iconShadow;
    //! Mode to render 1st image in
    Gfx::EngineRenderState mode1;
    //! Mode to render 2nd image in
    Gfx::EngineRenderState mode2;
    //! Hot point
    Math::Point hotPoint;

    EngineMouse(int icon1 = -1, int icon2 = -1, int iconShadow = -1,
                Gfx::EngineRenderState mode1 = Gfx::ENG_RSTATE_NORMAL,
                Gfx::EngineRenderState mode2 = Gfx::ENG_RSTATE_NORMAL,
                Math::Point hotPoint = Math::Point())
    {
        this->icon1      = icon1;
        this->icon2      = icon2;
        this->iconShadow = iconShadow;
        this->mode1      = mode1;
        this->mode2      = mode2;
        this->hotPoint   = hotPoint;
    }
};


/**
  \class CEngine
  \brief The graphics engine

  This is the main class for graphics engine. It is responsible for drawing the 3D scene,
  setting various render states, and facilitating the drawing of 2D interface.

  It uses a lower-level CDevice object which is implementation-independent core engine.

  \section Objecs Engine objects

  The 3D scene is composed of objects which are basically collections of triangles forming
  a surface or simply independent triangles in space. Objects are stored in the engine
  as a tree structure which is composed of 5 tiers (EngineObjLevel1, EngineObjLevel2 and so on).
  Each tier stores some data about object triangle, like textures or materials used.
  Additional information on objects stored are in EngineObject structure.
  Each object is uniquely identified by its rank.

  ...
 */
class CEngine
{
public:
    CEngine(CInstanceManager* iMan, CApplication* app);
    ~CEngine();

    //! Sets the device to be used
    void            SetDevice(Gfx::CDevice* device);
    //! Returns the current device
    Gfx::CDevice*   GetDevice();

    //! Sets the terrain object
    void            SetTerrain(Gfx::CTerrain* terrain);

    //! Returns the text rendering engine
    CText*          GetText();


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
    void            FrameMove(float rTime);
    //! Evolved throughout the game
    void            StepSimulation(float rTime);


    //! Writes a screenshot containing the current frame
    bool            WriteScreenShot(const std::string& fileName, int width, int height);


    //! Reads settings from INI
    bool            ReadSettings();
    //! Writes settings to INI
    bool            WriteSettings();

    //@{
    //! Management of game pause mode
    void            SetPause(bool pause);
    bool            GetPause();
    //@}

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
    //! Returns the last size of viewport window
    Math::IntPoint   GetLastWindowSize();

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

    //! Returns the name of directory with textures
    std::string     GetTextureDir();

    //! Increments the triangle counter for the current frame
    void            AddStatisticTriangle(int nb);
    //! Returns the number of triangles in current frame
    int             GetStatisticTriangle();


    /* *************** Object management *************** */

    int             CreateObject();
    void            FlushObject();
    bool            DeleteObject(int objRank);
    bool            SetDrawWorld(int objRank, bool draw);
    bool            SetDrawFront(int objRank, bool draw);

    bool            AddTriangle(int objRank, Gfx::VertexTex2* vertex, int nb, const Gfx::Material& mat,
                                int state, std::string texName1, std::string texName2,
                                float min, float max, bool globalUpdate);
    bool            AddSurface(int objRank, Gfx::VertexTex2* vertex, int nb, const Gfx::Material& mat,
                               int state, std::string texName1, std::string texName2,
                               float min, float max, bool globalUpdate);
    bool            AddQuick(int objRank, const Gfx::EngineObjLevel5& buffer,
                             std::string texName1, std::string texName2,
                             float min, float max, bool globalUpdate);
    Gfx::EngineObjLevel5* SearchTriangle(int objRank, const Gfx::Material& mat,
                                         int state, std::string texName1, std::string texName2,
                                         float min, float max);

    void            ChangeLOD();
    bool            ChangeSecondTexture(int objRank, const std::string& texName2);
    int             GetTotalTriangles(int objRank);
    int             GetTriangles(int objRank, float min, float max, Gfx::EngineTriangle* buffer, int size, float percent);
    bool            GetBBox(int objRank, Math::Vector& min, Math::Vector& max);
    bool            ChangeTextureMapping(int objRank, const Gfx::Material& mat, int state,
                                         const std::string& texName1, const std::string& texName2,
                                         float min, float max, Gfx::EngineTextureMapping mode,
                                         float au, float bu, float av, float bv);
    bool            TrackTextureMapping(int objRank, const Gfx::Material& mat, int state,
                                        const std::string& texName1, const std::string& texName2,
                                        float min, float max, Gfx::EngineTextureMapping mode,
                                        float pos, float factor, float tl, float ts, float tt);
    bool            SetObjectTransform(int objRank, const Math::Matrix& transform);
    bool            GetObjectTransform(int objRank, Math::Matrix& transform);
    bool            SetObjectType(int objRank, Gfx::EngineObjectType type);
    Gfx::EngineObjectType GetObjectType(int objRank);
    bool            SetObjectTransparency(int objRank, float value);

    bool            ShadowCreate(int objRank);
    void            ShadowDelete(int objRank);
    bool            SetObjectShadowHide(int objRank, bool hide);
    bool            SetObjectShadowType(int objRank, Gfx::EngineShadowType type);
    bool            SetObjectShadowPos(int objRank, const Math::Vector& pos);
    bool            SetObjectShadowNormal(int objRank, const Math::Vector& n);
    bool            SetObjectShadowAngle(int objRank, float angle);
    bool            SetObjectShadowRadius(int objRank, float radius);
    bool            SetObjectShadowIntensity(int objRank, float intensity);
    bool            SetObjectShadowHeight(int objRank, float h);
    float           GetObjectShadowRadius(int objRank);

    //! Lists the ranks of objects and subobjects selected
    void            SetHighlightRank(int* rankList);
    //! Returns the highlighted rectangle
    bool            GetHighlight(Math::Point& p1, Math::Point& p2);

    void            GroundSpotFlush();
    int             GroundSpotCreate();
    void            GroundSpotDelete(int rank);
    bool            SetObjectGroundSpotPos(int rank, const Math::Vector& pos);
    bool            SetObjectGroundSpotRadius(int rank, float radius);
    bool            SetObjectGroundSpotColor(int rank, const Gfx::Color& color);
    bool            SetObjectGroundSpotMinMax(int rank, float min, float max);
    bool            SetObjectGroundSpotSmooth(int rank, float smooth);

    int             GroundMarkCreate(Math::Vector pos, float radius,
                                     float delay1, float delay2, float delay3,
                                     int dx, int dy, char* table);
    bool            GroundMarkDelete(int rank);

    //! Updates the state after creating objects
    void            Update();


    /* *************** Mode setting *************** */

    //! Sets the current rendering state
    void            SetState(int state, const Gfx::Color& color = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));

    //! Sets the current material
    void            SetMaterial(const Gfx::Material& mat);

    //! Specifies the location and direction of view
    void            SetViewParams(const Math::Vector& eyePt, const Math::Vector& lookatPt,
                                  const Math::Vector& upVec, float eyeDistance);

    //! Creates texture with the specified params
    Gfx::Texture    CreateTexture(const std::string& texName,
                                  const Gfx::TextureCreateParams& params);
    //! Creates texture
    Gfx::Texture    CreateTexture(const std::string& texName);

    //! Destroys texture, unloading it and removing from cache
    void            DestroyTexture(const std::string& texName);

    //! Loads texture, creating it if not already present
    bool            LoadTexture(const std::string& name);
    //! Loads all necessary textures
    bool            LoadAllTextures();

    //! Sets texture for given stage; if not present in cache, the texture is loaded
    bool            SetTexture(const std::string& name, int stage = 0);

    //@{
    //! Border management (distance limits) depends of the resolution (LOD = level-of-detail)
    void            SetLimitLOD(int rank, float limit);
    float           GetLimitLOD(int rank, bool last=false);
    //@}

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

    //! Indicates whether it is possible to give a color SetState
    bool            GetStateColor();

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
    void            SetAmbientColor(const Gfx::Color& color, int rank = 0);
    Gfx::Color      GetAmbientColor(int rank = 0);
    //@}

    //@{
    //! Color management under water
    void            SetWaterAddColor(const Gfx::Color& color);
    Gfx::Color      GetWaterAddColor();
    //@}

    //@{
    //! Management of the fog color
    void            SetFogColor(const Gfx::Color& color, int rank = 0);
    Gfx::Color      GetFogColor(int rank = 0);
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
    void            SetBackground(const std::string& name, Gfx::Color up = Gfx::Color(), Gfx::Color down = Gfx::Color(),
                                  Gfx::Color cloudUp = Gfx::Color(), Gfx::Color cloudDown = Gfx::Color(),
                                  bool full = false, bool quarter = false);
    void            GetBackground(std::string& name, Gfx::Color& up, Gfx::Color& down,
                                  Gfx::Color& cloudUp, Gfx::Color& cloudDown,
                                  bool& full, bool& quarter);
    //@}

    //! Specifies the name of foreground texture
    void            SetForegroundName(const std::string& name);
    //! Specifies whether to draw the foreground
    void            SetOverFront(bool front);
    //! Sets the foreground overlay color
    void            SetOverColor(const Gfx::Color& color = Gfx::Color(), int mode = ENG_RSTATE_TCOLOR_BLACK);

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
    //! Management of game speed
    void            SetSpeed(float speed);
    float           GetSpeed();

    //@{
    //! Management of precision of robot tracks
    void            SetTracePrecision(float factor);
    float           GetTracePrecision();
    //@}

    //@{
    //! Management of mouse cursor visibility
    void                 SetMouseVisible(bool show);
    bool                 GetMouseVisible();
    //@}

    //@{
    //! Management of mouse cursor position
    void                 SetMousePos(Math::Point pos);
    Math::Point          GetMousePos();
    //@}

    //@{
    //! Management of mouse cursor type
    void                 SetMouseType(Gfx::EngineMouseType type);
    Gfx::EngineMouseType GetMouseType();
    //@}

    //! Returns the view matrix
    const Math::Matrix& GetMatView();
    //! Returns the camera center point
    Math::Vector    GetEyePt();
    //! Returns the camera target point
    Math::Vector    GetLookatPt();
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
    //! Draws the user interface over the scene
    void        DrawInterface();

    //! Updates the textures used for drawing ground spot
    void        UpdateGroundSpotTextures();

    //! Draws shadows
    void        DrawShadow();
    //! Draws the gradient background
    void        DrawBackground();
    //! Draws the gradient background
    void        DrawBackgroundGradient(const Gfx::Color& up, const Gfx::Color& down);
    //! Draws a portion of the image background
    void        DrawBackgroundImageQuarter(Math::Point p1, Math::Point p2, const std::string& name);
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

    //! Tests whether the given object is visible
    bool        IsVisible(int objRank);

    //! Detects whether an object is affected by the mouse
    bool        DetectBBox(int objRank, Math::Point mouse);

    //! Compute and return the 2D box on screen of any object
    bool        GetBBox2D(int objRank, Math::Point& min, Math::Point& max);

    //! Detects the target object that is selected with the mouse
    /** Returns the rank of the object or -1. */
    int         DetectObject(Math::Point mouse);

    //! Detects whether the mouse is in a triangle.
    bool        DetectTriangle(Math::Point mouse, Gfx::VertexTex2* triangle, int objRank, float& dist);

    //! Transforms a 3D point (x, y, z) in 2D space (x, y, -) of the window
    /** The coordinated p2D.z gives the distance. */
    bool        TransformPoint(Math::Vector& p2D, int objRank, Math::Vector p3D);

    //! Calculates the distances between the viewpoint and the origin of different objects
    void        ComputeDistance();

    //! Updates all the geometric parameters of objects
    void        UpdateGeometry();

protected:
    CInstanceManager*   m_iMan;
    CApplication*       m_app;
    CSoundInterface*    m_sound;
    Gfx::CDevice*       m_device;
    Gfx::CText*         m_text;
    Gfx::CLightManager* m_lightMan;
    Gfx::CParticle*     m_particle;
    Gfx::CWater*        m_water;
    Gfx::CCloud*        m_cloud;
    Gfx::CLightning*    m_lightning;
    Gfx::CPlanet*       m_planet;
    Gfx::CTerrain*      m_terrain;

    //! Last encountered error
    std::string     m_error;

    //! Whether to show stats (FPS, etc)
    bool            m_showStats;

    //! Speed of animation
    float           m_speed;
    //! Pause mode
    bool            m_pause;
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
    //! Previous size of viewport window
    Math::IntPoint   m_lastSize;

    //! Root of tree object structure (level 1 list)
    std::vector<Gfx::EngineObjLevel1>  m_objectTree;
    //! Object parameters
    std::vector<Gfx::EngineObject>     m_objects;
    //! Shadow list
    std::vector<Gfx::EngineShadow>     m_shadows;
    //! Ground spot list
    std::vector<Gfx::EngineGroundSpot> m_groundSpots;
    //! Ground mark
    Gfx::EngineGroundMark              m_groundMark;

    //! Location of camera
    Math::Vector    m_eyePt;
    //! Camera target
    Math::Vector    m_lookatPt;
    float           m_eyeDirH;
    float           m_eyeDirV;
    int             m_rankView;
    Gfx::Color      m_ambientColor[2];
    Gfx::Color      m_backColor[2];
    Gfx::Color      m_fogColor[2];
    float           m_deepView[2];
    float           m_fogStart[2];
    Gfx::Color      m_waterAddColor;
    int             m_statisticTriangle;
    bool            m_updateGeometry;
    int             m_alphaMode;
    bool            m_stateColor;
    bool            m_forceStateColor;
    bool            m_groundSpotVisible;
    bool            m_shadowVisible;
    bool            m_dirty;
    bool            m_fog;
    bool            m_firstGroundSpot;
    int             m_secondTexNum;
    std::string     m_backgroundName;
    Gfx::Color      m_backgroundColorUp;
    Gfx::Color      m_backgroundColorDown;
    Gfx::Color      m_backgroundCloudUp;
    Gfx::Color      m_backgroundCloudDown;
    bool            m_backgroundFull;
    bool            m_backgroundQuarter;
    bool            m_overFront;
    Gfx::Color      m_overColor;
    int             m_overMode;
    std::string     m_foregroundName;
    bool            m_drawWorld;
    bool            m_drawFront;
    float           m_limitLOD[2];
    float           m_particleDensity;
    float           m_clippingDistance;
    float           m_lastClippingDistance;
    float           m_objectDetail;
    float           m_lastObjectDetail;
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

    //! Texture directory name
    std::string     m_texPath;
    //! Default texture create params
    Gfx::TextureCreateParams m_defaultTexParams;

    //! Map of loaded textures (by name)
    std::map<std::string, Gfx::Texture> m_texNameMap;
    //! Reverse map of loaded textures (by texture)
    std::map<Gfx::Texture, std::string> m_revTexNameMap;
    //! Blacklist map of textures
    /** Textures on this list were not successful in first loading,
     *  so are disabled for subsequent load calls. */
    std::set<std::string> m_texBlacklist;

    //! Mouse cursor definitions
    Gfx::EngineMouse     m_mice[Gfx::ENG_MOUSE_COUNT];
    //! Texture with mouse cursors
    Gfx::Texture         m_miceTexture;
    //! Size of mouse cursor
    Math::Point          m_mouseSize;
    //! Type of mouse cursor
    Gfx::EngineMouseType m_mouseType;
    //! Position of mouse in interface coords
    Math::Point          m_mousePos;
    //! Is mouse visible?
    bool                 m_mouseVisible;

    //! Last engine render state (-1 at the beginning of frame)
    int             m_lastState;
    //! Last color set with render state
    Gfx::Color      m_lastColor;
    //! Last texture names for 2 used texture stages
    std::string     m_lastTexture[2];
    //! Last material
    Gfx::Material   m_lastMaterial;
};

}; // namespace Gfx
