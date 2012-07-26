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

// engine.h

#pragma once


#include "common/event.h"
#include "graphics/common/color.h"
#include "graphics/common/material.h"
#include "graphics/common/texture.h"
#include "graphics/common/vertex.h"
#include "math/intpoint.h"
#include "math/matrix.h"
#include "math/point.h"
#include "math/vector.h"


#include <string>
#include <vector>
#include <map>


class CApplication;
class CInstanceManager;
class CObject;
class CSound;


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
    //! Render state (TODO: ?)
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

    EngineObjLevel5();
};

/**
  \struct EngineObjLevel4
  \brief Tier 4 of object tree */
struct EngineObjLevel4
{
    int                                reserved;
    std::vector<Gfx::EngineObjLevel5>  up;
    Gfx::EngineObjLevel3*              down;

    EngineObjLevel4();
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

    EngineObjLevel3();
};

/**
  \struct EngineObjLevel2
  \brief Tier 2 of object tree */
struct EngineObjLevel2
{
    int                                objRank;
    std::vector<Gfx::EngineObjLevel3>  up;
    Gfx::EngineObjLevel1*              down;

    EngineObjLevel2();
};

/**
  \struct EngineObjLevel1
  \brief Tier 1 of object tree */
struct EngineObjLevel1
{
    Gfx::Texture                       tex1;
    Gfx::Texture                       tex2;
    std::vector<Gfx::EngineObjLevel2>  up;

    EngineObjLevel1();
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
    //! Increase
    ENG_GR_MARK_PHASE_INC = 1,
    //! Fixed
    ENG_GR_MARK_PHASE_FIX = 2,
    //! Decrease
    ENG_GR_MARK_PHASE_DEC = 2
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
    ENG_RSTATE_TCOLOR_WHITE     = (1<<17)
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
    CEngine(CInstanceManager *iMan, CApplication *app);
    ~CEngine();

    bool            GetWasInit();
    std::string     GetError();

    bool            Create();
    void            Destroy();

    void            SetDevice(Gfx::CDevice *device);
    Gfx::CDevice*   GetDevice();

    bool            AfterDeviceSetInit();

    void            SetTerrain(Gfx::CTerrain* terrain);

    bool            ProcessEvent(const Event &event);

    bool            Render();


    bool            WriteProfile();

    void            SetPause(bool pause);
    bool            GetPause();

    void            SetMovieLock(bool lock);
    bool            GetMovieLock();

    void            SetShowStat(bool show);
    bool            GetShowStat();

    void            SetRenderEnable(bool enable);

    int             OneTimeSceneInit();
    int             InitDeviceObjects();
    int             DeleteDeviceObjects();
    int             RestoreSurfaces();
    int             FrameMove(float rTime);
    void            StepSimulation(float rTime);
    int             FinalCleanup();
    void            AddStatisticTriangle(int nb);
    int             GetStatisticTriangle();
    void            SetHiliteRank(int *rankList);
    bool            GetHilite(Math::Point &p1, Math::Point &p2);
    bool            GetSpriteCoord(int &x, int &y);
    void            SetInfoText(int line, char* text);
    char*           GetInfoText(int line);
    void            FirstExecuteAdapt(bool first);

    bool            GetFullScreen();

    Math::Matrix*   GetMatView();
    Math::Matrix*   GetMatLeftView();
    Math::Matrix*   GetMatRightView();

    void            TimeInit();
    void            TimeEnterGel();
    void            TimeExitGel();
    float           TimeGet();

    int             GetRestCreate();
    int             CreateObject();
    void            FlushObject();
    bool            DeleteObject(int objRank);
    bool            SetDrawWorld(int objRank, bool draw);
    bool            SetDrawFront(int objRank, bool draw);

    bool            AddTriangle(int objRank, Gfx::VertexTex2* vertex, int nb, const Gfx::Material &mat,
                                int state, std::string texName1, std::string texName2,
                                float min, float max, bool globalUpdate);
    bool            AddSurface(int objRank, Gfx::VertexTex2* vertex, int nb, const Gfx::Material &mat,
                               int state, std::string texName1, std::string texName2,
                               float min, float max, bool globalUpdate);
    bool            AddQuick(int objRank, Gfx::EngineObjLevel5* buffer,
                             std::string texName1, std::string texName2,
                             float min, float max, bool globalUpdate);
    Gfx::EngineObjLevel5* SearchTriangle(int objRank, const Gfx::Material &mat,
                                         int state, std::string texName1, std::string texName2,
                                         float min, float max);

    void            ChangeLOD();
    bool            ChangeSecondTexture(int objRank, char* texName2);
    int             GetTotalTriangles(int objRank);
    int             GetTriangles(int objRank, float min, float max, Gfx::EngineTriangle* buffer, int size, float percent);
    bool            GetBBox(int objRank, Math::Vector &min, Math::Vector &max);
    bool            ChangeTextureMapping(int objRank, const Gfx::Material &mat, int state,
                                         const std::string &texName1, const std::string &texName2,
                                         float min, float max, Gfx::EngineTextureMapping mode,
                                         float au, float bu, float av, float bv);
    bool            TrackTextureMapping(int objRank, const Gfx::Material &mat, int state,
                                        const std::string &texName1, const std::string &texName2,
                                        float min, float max, Gfx::EngineTextureMapping mode,
                                        float pos, float factor, float tl, float ts, float tt);
    bool            SetObjectTransform(int objRank, const Math::Matrix &transform);
    bool            GetObjectTransform(int objRank, Math::Matrix &transform);
    bool            SetObjectType(int objRank, Gfx::EngineObjectType type);
    Gfx::EngineObjectType GetObjectType(int objRank);
    bool            SetObjectTransparency(int objRank, float value);

    bool            ShadowCreate(int objRank);
    void            ShadowDelete(int objRank);
    bool            SetObjectShadowHide(int objRank, bool hide);
    bool            SetObjectShadowType(int objRank, Gfx::EngineShadowType type);
    bool            SetObjectShadowPos(int objRank, const Math::Vector &pos);
    bool            SetObjectShadowNormal(int objRank, const Math::Vector &n);
    bool            SetObjectShadowAngle(int objRank, float angle);
    bool            SetObjectShadowRadius(int objRank, float radius);
    bool            SetObjectShadowIntensity(int objRank, float intensity);
    bool            SetObjectShadowHeight(int objRank, float h);
    float           GetObjectShadowRadius(int objRank);

    void            GroundSpotFlush();
    int             GroundSpotCreate();
    void            GroundSpotDelete(int rank);
    bool            SetObjectGroundSpotPos(int rank, const Math::Vector &pos);
    bool            SetObjectGroundSpotRadius(int rank, float radius);
    bool            SetObjectGroundSpotColor(int rank, const Gfx::Color &color);
    bool            SetObjectGroundSpotMinMax(int rank, float min, float max);
    bool            SetObjectGroundSpotSmooth(int rank, float smooth);

    int             GroundMarkCreate(Math::Vector pos, float radius,
                                     float delay1, float delay2, float delay3,
                                     int dx, int dy, char* table);
    bool            GroundMarkDelete(int rank);

    void            Update();

    void            SetViewParams(const Math::Vector &eyePt, const Math::Vector &lookatPt,
                                  const Math::Vector &upVec, float eyeDistance);

    Gfx::Texture    CreateTexture(const std::string &texName,
                                  const Gfx::TextureCreateParams &params);
    Gfx::Texture    CreateTexture(const std::string &texName);
    void            DestroyTexture(const std::string &texName);

    bool            LoadTexture(const std::string &name, int stage = 0);
    bool            LoadAllTextures();

    void            SetLimitLOD(int rank, float limit);
    float           GetLimitLOD(int rank, bool last=false);

    void            SetTerrainVision(float vision);

    void            SetGroundSpot(bool mode);
    bool            GetGroundSpot();
    void            SetShadow(bool mode);
    bool            GetShadow();
    void            SetDirty(bool mode);
    bool            GetDirty();
    void            SetFog(bool mode);
    bool            GetFog();
    bool            GetStateColor();

    void            SetSecondTexture(int texNum);
    int             GetSecondTexture();

    void            SetRankView(int rank);
    int             GetRankView();

    void            SetDrawWorld(bool draw);
    void            SetDrawFront(bool draw);

    void            SetAmbientColor(const Gfx::Color &color, int rank = 0);
    Gfx::Color      GetAmbientColor(int rank = 0);

    void            SetWaterAddColor(const Gfx::Color &color);
    Gfx::Color      GetWaterAddColor();

    void            SetFogColor(const Gfx::Color &color, int rank = 0);
    Gfx::Color      GetFogColor(int rank = 0);

    void            SetDeepView(float length, int rank = 0, bool ref=false);
    float           GetDeepView(int rank = 0);

    void            SetFogStart(float start, int rank = 0);
    float           GetFogStart(int rank = 0);

    void            SetBackground(const std::string &name, Gfx::Color up = Gfx::Color(), Gfx::Color down = Gfx::Color(),
                                  Gfx::Color cloudUp = Gfx::Color(), Gfx::Color cloudDown = Gfx::Color(),
                                  bool full = false, bool quarter = false);
    void            GetBackground(const std::string &name, Gfx::Color &up, Gfx::Color &down,
                                  Gfx::Color &cloudUp, Gfx::Color &cloudDown,
                                  bool &full, bool &quarter);
    void            SetFrontsizeName(char *name);
    void            SetOverFront(bool front);
    void            SetOverColor(const Gfx::Color &color = Gfx::Color(), int mode = ENG_RSTATE_TCOLOR_BLACK);

    void            SetParticleDensity(float value);
    float           GetParticleDensity();
    float           ParticleAdapt(float factor);

    void            SetClippingDistance(float value);
    float           GetClippingDistance();

    void            SetObjectDetail(float value);
    float           GetObjectDetail();

    void            SetGadgetQuantity(float value);
    float           GetGadgetQuantity();

    void            SetTextureQuality(int value);
    int             GetTextureQuality();

    void            SetTotoMode(bool present);
    bool            GetTotoMode();

    void            SetLensMode(bool present);
    bool            GetLensMode();

    void            SetWaterMode(bool present);
    bool            GetWaterMode();

    void            SetLightingMode(bool present);
    bool            GetLightingMode();

    void            SetSkyMode(bool present);
    bool            GetSkyMode();

    void            SetBackForce(bool present);
    bool            GetBackForce();

    void            SetPlanetMode(bool present);
    bool            GetPlanetMode();

    void            SetLightMode(bool present);
    bool            GetLightMode();

    void            SetEditIndentMode(bool autoIndent);
    bool            GetEditIndentMode();

    void            SetEditIndentValue(int value);
    int             GetEditIndentValue();

    void            SetSpeed(float speed);
    float           GetSpeed();

    void            SetTracePrecision(float factor);
    float           GetTracePrecision();

    void            SetFocus(float focus);
    float           GetFocus();
    Math::Vector    GetEyePt();
    Math::Vector    GetLookatPt();
    float           GetEyeDirH();
    float           GetEyeDirV();
    Math::Point     GetDim();
    void            UpdateMatProj();

    void            ApplyChange();

    void            FlushPressKey();
    void            ResetKey();
    void            SetKey(int keyRank, int option, int key);
    int             GetKey(int keyRank, int option);

    void            SetJoystick(bool enable);
    bool            GetJoystick();

    void            SetDebugMode(bool mode);
    bool            GetDebugMode();
    bool            GetSetupMode();

    bool            IsVisiblePoint(const Math::Vector &pos);

    int             DetectObject(Math::Point mouse);
    void            SetState(int state, Gfx::Color color = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
    void            SetTexture(const std::string &name, int stage = 0);
    void            SetMaterial(const Gfx::Material &mat);

    void                 SetMouseVisible(bool show);
    bool                 GetMouseVisible();
    void                 SetMousePos(Math::Point pos);
    Math::Point          GetMousePos();
    void                 SetMouseType(Gfx::EngineMouseType type);
    Gfx::EngineMouseType GetMouseType();

    CText*          GetText();

    bool            ChangeColor(char *name, Gfx::Color colorRef1, Gfx::Color colorNew1,
                                Gfx::Color colorRef2, Gfx::Color colorNew2,
                                float tolerance1, float tolerance2,
                                Math::Point ts, Math::Point ti,
                                Math::Point *pExclu=0, float shift=0.0f, bool hSV=false);
    bool            OpenImage(char *name);
    bool            CopyImage();
    bool            LoadImage();
    bool            ScrollImage(int dx, int dy);
    bool            SetDot(int x, int y, Gfx::Color color);
    bool            CloseImage();
    bool            WriteScreenShot(char *filename, int width, int height);
    //bool      GetRenderDC(HDC &hDC);
    //bool      ReleaseRenderDC(HDC &hDC);
    //PBITMAPINFO   CreateBitmapInfoStruct(HBITMAP hBmp);
    //bool      CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);

protected:

    void        SetUp3DView();
    bool        Draw3DScene();

    void        SetUpInterfaceView();
    bool        DrawInterface();

    void        DrawGroundSpot();
    void        DrawShadow();
    void        DrawBackground();
    void        DrawBackgroundGradient(Gfx::Color up, Gfx::Color down);
    void        DrawBackgroundImageQuarter(Math::Point p1, Math::Point p2, char *name);
    void        DrawBackgroundImage();
    void        DrawPlanet();
    void        DrawFrontsize();
    void        DrawOverColor();
    void        DrawHilite();
    void        DrawMouse();
    void        DrawMouseSprite(Math::Point pos, Math::Point dim, int icon);

    /*
    Gfx::ObjLevel2* AddLevel1(Gfx::ObjLevel1 *&p1, char* texName1, char* texName2);
    Gfx::ObjLevel3* AddLevel2(Gfx::ObjLevel2 *&p2, int objRank);
    Gfx::ObjLevel4* AddLevel3(Gfx::ObjLevel3 *&p3, float min, float max);
    Gfx::ObjLevel5* AddLevel4(Gfx::ObjLevel4 *&p4, int reserve);
    Gfx::ObjLevel6* AddLevel5(Gfx::ObjLevel5 *&p5, Gfx::TriangleType type, const Gfx::Material &mat, int state, int nb);*/

    bool        IsVisible(int objRank);
    bool        DetectBBox(int objRank, Math::Point mouse);
    bool        GetBBox2D(int objRank, Math::Point &min, Math::Point &max);
    bool        DetectTriangle(Math::Point mouse, Gfx::VertexTex2 *triangle, int objRank, float &dist);
    bool        TransformPoint(Math::Vector &p2D, int objRank, Math::Vector p3D);
    void        ComputeDistance();
    void        UpdateGeometry();

protected:
    CInstanceManager*   m_iMan;
    CApplication*       m_app;
    CSound*             m_sound;
    Gfx::CDevice*       m_device;
    Gfx::CText*         m_text;
    Gfx::CLightManager* m_lightMan;
    Gfx::CParticle*     m_particle;
    Gfx::CWater*        m_water;
    Gfx::CCloud*        m_cloud;
    Gfx::CLightning*    m_lightning;
    Gfx::CPlanet*       m_planet;
    Gfx::CTerrain*      m_terrain;

    bool            m_wasInit;
    std::string     m_error;

    int             m_blackSrcBlend[2];
    int             m_blackDestBlend[2];
    int             m_whiteSrcBlend[2];
    int             m_whiteDestBlend[2];
    int             m_diffuseSrcBlend[2];
    int             m_diffuseDestBlend[2];
    int             m_alphaSrcBlend[2];
    int             m_alphaDestBlend[2];

    Math::Matrix    m_matProj;
    Math::Matrix    m_matLeftView;
    Math::Matrix    m_matRightView;
    Math::Matrix    m_matView;
    float           m_focus;

    Math::Matrix    m_matWorldInterface;
    Math::Matrix    m_matProjInterface;
    Math::Matrix    m_matViewInterface;

    long            m_baseTime;
    long            m_stopTime;
    float           m_absTime;
    float           m_lastTime;
    float           m_speed;
    bool            m_pause;
    bool            m_render;
    bool            m_movieLock;

    Math::IntPoint  m_dim;
    Math::IntPoint  m_lastDim;

    std::vector<Gfx::EngineObjLevel1>  m_objectTree;
    std::vector<Gfx::EngineObject>     m_objects;
    std::vector<Gfx::EngineShadow>     m_shadow;
    std::vector<Gfx::EngineGroundSpot> m_groundSpot;
    Gfx::EngineGroundMark              m_groundMark;

    Math::Vector    m_eyePt;
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
    //char            m_infoText[10][200];
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
    std::string     m_frontsizeName;
    bool            m_drawWorld;
    bool            m_drawFront;
    float           m_limitLOD[2];
    float           m_particuleDensity;
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

    int             m_hiliteRank[100];
    bool            m_hilite;
    Math::Point     m_hiliteP1;
    Math::Point     m_hiliteP2;

    int             m_lastState;
    Gfx::Color      m_lastColor;
    char            m_lastTexture[2][50];
    Gfx::Material   m_lastMaterial;

    std::string     m_texPath;
    Gfx::TextureCreateParams m_defaultTexParams;

    std::map<std::string, Gfx::Texture> m_texNameMap;
    std::map<Gfx::Texture, std::string> m_revTexNameMap;

    Gfx::EngineMouse     m_mice[Gfx::ENG_MOUSE_COUNT];
    Gfx::Texture         m_miceTexture;
    Math::Point          m_mouseSize;
    Gfx::EngineMouseType m_mouseType;
    Math::Point          m_mousePos;
    bool                 m_mouseVisible;

    //LPDIRECTDRAWSURFACE7 m_imageSurface;
    //DDSURFACEDESC2        m_imageDDSD;
    //WORD*             m_imageCopy;
    //int                   m_imageDX;
    //int                   m_imageDY;
};

}; // namespace Gfx
