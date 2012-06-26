// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// particule.h

#ifndef _PARTICULE_H_
#define _PARTICULE_H_


#include "d3dengine.h"
#include "sound.h"


class CInstanceManager;
class CRobotMain;
class CTerrain;
class CWater;
class CObject;


#define MAXPARTICULE    500
#define MAXPARTITYPE    5
#define MAXTRACK        100
#define MAXTRACKLEN     10
#define MAXPARTIFOG     100
#define MAXWHEELTRACE   1000

#define SH_WORLD        0       // particle in the world in the interface
#define SH_FRONT        1       // particle in the world on the interface
#define SH_INTERFACE        2       // particle in the interface
#define SH_MAX          3

// type == 0    ->  triangles
// type == 1    ->  effect00 (black background)
// type == 2    ->  effect01 (black background)
// type == 3    ->  effect02 (black background)
// type == 4    ->  text     (white background)


enum ParticuleType
{
    PARTIEXPLOT     = 1,        // technology explosion
    PARTIEXPLOO     = 2,        // organic explosion
    PARTIMOTOR      = 3,        // the engine exhaust gas
    PARTIGLINT      = 4,        // reflection
    PARTIBLITZ      = 5,        // lightning recharging battery
    PARTICRASH      = 6,        // dust after fall
    PARTIGAS        = 7,        // gas from the reactor
    PARTIFIRE       = 9,        // fireball shrinks
    PARTIFIREZ      = 10,       // fireball grows
    PARTIBLUE       = 11,       // blue ball
    PARTISELY       = 12,       // yellow selection
    PARTISELR       = 13,       // red selection
    PARTIGUN1       = 18,       // a bullet (fireball)
    PARTIGUN2       = 19,       // bullet 2 (ant)
    PARTIGUN3       = 20,       // bullet 3 (spider)
    PARTIGUN4       = 21,       // bullet 4 (orgaball)
    PARTIFRAG       = 22,       // triangular fragment
    PARTIQUEUE      = 23,       // inflamed tail
    PARTIORGANIC1       = 24,       // organic ball mother
    PARTIORGANIC2       = 25,       // organic ball daughter
    PARTISMOKE1     = 26,       // black smoke
    PARTISMOKE2     = 27,       // black smoke
    PARTISMOKE3     = 28,       // black smoke
    PARTISMOKE4     = 29,       // black smoke
    PARTIBLOOD      = 30,       // human blood
    PARTIBLOODM     = 31,       // blood laying
    PARTIVAPOR      = 32,       // steam
    PARTIVIRUS1     = 33,       // virus 1
    PARTIVIRUS2     = 34,       // virus 2
    PARTIVIRUS3     = 35,       // virus 3
    PARTIVIRUS4     = 36,       // virus 4
    PARTIVIRUS5     = 37,       // virus 5
    PARTIVIRUS6     = 38,       // virus 6
    PARTIVIRUS7     = 39,       // virus 7
    PARTIVIRUS8     = 40,       // virus 8
    PARTIVIRUS9     = 41,       // virus 9
    PARTIVIRUS10        = 42,       // virus 10
    PARTIRAY1       = 43,       // ray 1 (turn)
    PARTIRAY2       = 44,       // ray 2 (electric arc)
    PARTIRAY3       = 45,       // ray 3
    PARTIRAY4       = 46,       // ray 4
    PARTIFLAME      = 47,       // flame
    PARTIBUBBLE     = 48,       // bubble
    PARTIFLIC       = 49,       // circles in the water
    PARTIEJECT      = 50,       // ejection from the reactor
    PARTISCRAPS     = 51,       // waste from the reactor
    PARTITOTO       = 52,       // reactor of tot
    PARTIERROR      = 53,       // toto says no
    PARTIWARNING        = 54,       // foo says blah
    PARTIINFO       = 54,       // toto says yes
    PARTIQUARTZ     = 55,       // reflection crystal
    PARTISPHERE0        = 56,       // explosion sphere
    PARTISPHERE1        = 57,       // energy sphere
    PARTISPHERE2        = 58,       // analysis sphere
    PARTISPHERE3        = 59,       // shield sphere
    PARTISPHERE4        = 60,       // information sphere (emit)
    PARTISPHERE5        = 61,       // botanical sphere (gravity root)
    PARTISPHERE6        = 62,       // information sphere (receive)
    PARTISPHERE7        = 63,       // sphere
    PARTISPHERE8        = 64,       // sphere
    PARTISPHERE9        = 65,       // sphere
    PARTIGUNDEL     = 66,       // bullet destroyed by shield
    PARTIPART       = 67,       // object part
    PARTITRACK1     = 68,       // drag 1
    PARTITRACK2     = 69,       // drag 2
    PARTITRACK3     = 70,       // drag 3
    PARTITRACK4     = 71,       // drag 4
    PARTITRACK5     = 72,       // drag 5
    PARTITRACK6     = 73,       // drag 6
    PARTITRACK7     = 74,       // drag 7
    PARTITRACK8     = 75,       // drag 8
    PARTITRACK9     = 76,       // drag 9
    PARTITRACK10        = 77,       // drag 10
    PARTITRACK11        = 78,       // drag 11
    PARTITRACK12        = 79,       // drag 12
    PARTITRACK13        = 80,       // drag 13
    PARTITRACK14        = 81,       // drag 14
    PARTITRACK15        = 82,       // drag 15
    PARTITRACK16        = 83,       // drag 16
    PARTITRACK17        = 84,       // drag 17
    PARTITRACK18        = 85,       // drag 18
    PARTITRACK19        = 86,       // drag 19
    PARTITRACK20        = 87,       // drag 20
    PARTIGLINTb     = 88,       // blue reflection
    PARTIGLINTr     = 89,       // red reflection
    PARTILENS1      = 90,       // brilliance 1 (orange)
    PARTILENS2      = 91,       // brilliance 2 (yellow)
    PARTILENS3      = 92,       // brilliance 3 (red)
    PARTILENS4      = 93,       // brilliance 4 (violet)
    PARTICONTROL        = 94,       // reflection on button
    PARTISHOW       = 95,       // shows a place
    PARTICHOC       = 96,       // shock wave
    PARTIGFLAT      = 97,       // shows if the ground is flat
    PARTIRECOVER        = 98,       // blue ball recycler
    PARTIROOT       = 100,      // gravity root smoke
    PARTIPLOUF0     = 101,      // splash
    PARTIPLOUF1     = 102,      // splash
    PARTIPLOUF2     = 103,      // splash
    PARTIPLOUF3     = 104,      // splash
    PARTIPLOUF4     = 105,      // splash
    PARTIDROP       = 106,      // drop
    PARTIFOG0       = 107,      // fog 0
    PARTIFOG1       = 108,      // fog 1
    PARTIFOG2       = 109,      // fog 2
    PARTIFOG3       = 110,      // fog 3
    PARTIFOG4       = 111,      // fog 4
    PARTIFOG5       = 112,      // fog 5
    PARTIFOG6       = 113,      // fog 6
    PARTIFOG7       = 114,      // fog 7
    PARTIFOG8       = 115,      // fog 8
    PARTIFOG9       = 116,      // fog 9
    PARTILIMIT1     = 117,      // shows the limits 1
    PARTILIMIT2     = 118,      // shows the limits 2
    PARTILIMIT3     = 119,      // shows the limits 3
    PARTILIMIT4     = 120,      // shows the limits 4
    PARTIWATER      = 121,      // drop of water
    PARTIEXPLOG1        = 122,      // ball explosion 1
    PARTIEXPLOG2        = 123,      // ball explosion 2
    PARTIBASE       = 124,      // gases of spaceship
    PARTITRACE0     = 140,      // trace
    PARTITRACE1     = 141,      // trace
    PARTITRACE2     = 142,      // trace
    PARTITRACE3     = 143,      // trace
    PARTITRACE4     = 144,      // trace
    PARTITRACE5     = 145,      // trace
    PARTITRACE6     = 146,      // trace
    PARTITRACE7     = 147,      // trace
    PARTITRACE8     = 148,      // trace
    PARTITRACE9     = 149,      // trace
    PARTITRACE10        = 150,      // trace
    PARTITRACE11        = 151,      // trace
    PARTITRACE12        = 152,      // trace
    PARTITRACE13        = 153,      // trace
    PARTITRACE14        = 154,      // trace
    PARTITRACE15        = 155,      // trace
    PARTITRACE16        = 156,      // trace
    PARTITRACE17        = 157,      // trace
    PARTITRACE18        = 158,      // trace
    PARTITRACE19        = 159,      // trace
};

enum ParticulePhase
{
    PARPHSTART      = 0,
    PARPHEND        = 1,
};

typedef struct
{
    char            bUsed;      // TRUE -> particle used
    char            bRay;       // TRUE -> ray with goal
    unsigned short      uniqueStamp;    // unique mark
    short           sheet;      // sheet (0..n)
    ParticuleType       type;       // type PARTI*
    ParticulePhase      phase;      // phase PARPH*
    float           mass;       // mass of the particle (in rebounding)
    float           weight;     // weight of the particle (for noise)
    float           duration;   // length of life
    D3DVECTOR       pos;        // absolute position (relative if object links)
    D3DVECTOR       goal;       // goal position (if bRay)
    D3DVECTOR       speed;      // speed of displacement
    float           windSensitivity;
    short           bounce;     // number of rebounds
    FPOINT          dim;        // dimensions of the rectangle
    float           zoom;       // zoom (0..1)
    float           angle;      // angle of rotation
    float           intensity;  // intensity
    FPOINT          texSup;     // coordinated upper texture
    FPOINT          texInf;     // coordinated lower texture
    float           time;       // age of the particle (0..n)
    float           phaseTime;  // age at the beginning of phase
    float           testTime;   // time since last test
    CObject*        objLink;    // father object (for example reactor)
    CObject*        objFather;  // father object (for example reactor)
    short           objRank;    // rank of the object, or -1
    short           trackRank;  // rank of the drag
}
Particule;

typedef struct
{
    char            bUsed;      // TRUE -> drag used
    char            bDrawParticule;
    float           step;       // duration of not
    float           last;       // increase last not memorized
    float           intensity;  // intensity at starting (0..1)
    float           width;      // tail width
    int         used;       // number of positions in "pos"
    int         head;       // head to write index
    D3DVECTOR       pos[MAXTRACKLEN];
    float           len[MAXTRACKLEN];
}
Track;

typedef struct
{
    ParticuleType       type;       // type PARTI*
    D3DVECTOR       pos[4];     // rectangle positions
    float           startTime;  // beginning of life
}
WheelTrace;



class CParticule
{
public:
    CParticule(CInstanceManager* iMan, CD3DEngine* engine);
    ~CParticule();

    void        SetD3DDevice(LPDIRECT3DDEVICE7 device);

    void        FlushParticule();
    void        FlushParticule(int sheet);
    int         CreateParticule(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim, ParticuleType type, float duration=1.0f, float mass=0.0f, float windSensitivity=1.0f, int sheet=0);
    int         CreateFrag(D3DVECTOR pos, D3DVECTOR speed, D3DTriangle *triangle, ParticuleType type, float duration=1.0f, float mass=0.0f, float windSensitivity=1.0f, int sheet=0);
    int         CreatePart(D3DVECTOR pos, D3DVECTOR speed, ParticuleType type, float duration=1.0f, float mass=0.0f, float weight=0.0f, float windSensitivity=1.0f, int sheet=0);
    int         CreateRay(D3DVECTOR pos, D3DVECTOR goal, ParticuleType type, FPOINT dim, float duration=1.0f, int sheet=0);
    int         CreateTrack(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim, ParticuleType type, float duration=1.0f, float mass=0.0f, float length=10.0f, float width=1.0f);
    void        CreateWheelTrace(const D3DVECTOR &p1, const D3DVECTOR &p2, const D3DVECTOR &p3, const D3DVECTOR &p4, ParticuleType type);
    void        DeleteParticule(ParticuleType type);
    void        DeleteParticule(int channel);
    void        SetObjectLink(int channel, CObject *object);
    void        SetObjectFather(int channel, CObject *object);
    void        SetPosition(int channel, D3DVECTOR pos);
    void        SetDimension(int channel, FPOINT dim);
    void        SetZoom(int channel, float zoom);
    void        SetAngle(int channel, float angle);
    void        SetIntensity(int channel, float intensity);
    void        SetParam(int channel, D3DVECTOR pos, FPOINT dim, float zoom, float angle, float intensity);
    void        SetPhase(int channel, ParticulePhase phase, float duration);
    BOOL        GetPosition(int channel, D3DVECTOR &pos);

    D3DCOLORVALUE RetFogColor(D3DVECTOR pos);

    void        SetFrameUpdate(int sheet, BOOL bUpdate);
    void        FrameParticule(float rTime);
    void        DrawParticule(int sheet);

    BOOL        WriteWheelTrace(char *filename, int width, int height, D3DVECTOR dl, D3DVECTOR ur);

protected:
    void        DeleteRank(int rank);
    BOOL        CheckChannel(int &channel);
    void        DrawParticuleTriangle(int i);
    void        DrawParticuleNorm(int i);
    void        DrawParticuleFlat(int i);
    void        DrawParticuleFog(int i);
    void        DrawParticuleRay(int i);
    void        DrawParticuleSphere(int i);
    void        DrawParticuleCylinder(int i);
    void        DrawParticuleWheel(int i);
    CObject*    SearchObjectGun(D3DVECTOR old, D3DVECTOR pos, ParticuleType type, CObject *father);
    CObject*    SearchObjectRay(D3DVECTOR pos, D3DVECTOR goal, ParticuleType type, CObject *father);
    void        Play(Sound sound, D3DVECTOR pos, float amplitude);
    BOOL        TrackMove(int i, D3DVECTOR pos, float progress);
    void        TrackDraw(int i, ParticuleType type);

protected:
    CInstanceManager*   m_iMan;
    CD3DEngine*         m_engine;
    LPDIRECT3DDEVICE7   m_pD3DDevice;
    CRobotMain*         m_main;
    CTerrain*           m_terrain;
    CWater*             m_water;
    CSound*             m_sound;

    Particule   m_particule[MAXPARTICULE*MAXPARTITYPE];
    D3DTriangle m_triangle[MAXPARTICULE];  // triangle if PartiType == 0
    Track       m_track[MAXTRACK];
    int         m_wheelTraceTotal;
    int         m_wheelTraceIndex;
    WheelTrace  m_wheelTrace[MAXWHEELTRACE];
    int         m_totalInterface[MAXPARTITYPE][SH_MAX];
    BOOL        m_bFrameUpdate[SH_MAX];
    int         m_fogTotal;
    int         m_fog[MAXPARTIFOG];
    int         m_uniqueStamp;
    int         m_exploGunCounter;
    float       m_lastTimeGunDel;
    float       m_absTime;
};


#endif //_PARTICULE_H_
