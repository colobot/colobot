/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "graphics/engine/particle.h"

#include "app/app.h"

#include "common/logger.h"
#include "common/stringutils.h"
#include "common/codepoint.h"

#include "graphics/core/device.h"
#include "graphics/core/renderers.h"
#include "graphics/core/triangle.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/text.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/interface/damageable_object.h"

#include "object/subclass/shielder.h"

#include "sound/sound.h"

#include <cstring>
#include <filesystem>


// Graphics module namespace
namespace Gfx
{


const float FOG_HSUP    = 10.0f;
const float FOG_HINF    = 100.0f;


//! Check if an object is a destroyable enemy
static bool IsAlien(ObjectType type)
{
    return ( type == OBJECT_ANT      ||
             type == OBJECT_SPIDER   ||
             type == OBJECT_BEE      ||
             type == OBJECT_WORM     ||
             type == OBJECT_MOTHER   ||
             type == OBJECT_NEST     ||
             type == OBJECT_BULLET   ||
             type == OBJECT_EGG      ||
             type == OBJECT_TEEN28   ||
             type == OBJECT_TEEN31   );
}

CParticle::CParticle(CEngine* engine)
    : m_engine(engine), m_triangle(MAXPARTICULE)
{
    std::fill_n(m_frameUpdate, SH_MAX, true);
}

CParticle::~CParticle()
{
}

void CParticle::SetDevice(CDevice* device)
{
    m_device = device;
    m_renderer = device->GetParticleRenderer();
}

void CParticle::FlushParticle()
{
    for (int i = 0; i < MAXPARTICULE*MAXPARTITYPE; i++)
        m_particle[i].used = false;

    for (int i = 0; i < MAXPARTITYPE; i++)
    {
        for (int j = 0; j < SH_MAX; j++)
        {
            m_totalInterface[i][j] = 0;
        }
    }

    for (int i = 0; i < MAXTRACK; i++)
        m_track[i].used = false;

    m_wheelTraceTotal = 0;
    m_wheelTraceIndex = 0;

    for (int i = 0; i < SH_MAX; i++)
        m_frameUpdate[i] = true;

    m_fogTotal = 0;
    m_exploGunCounter = 0;
}

void CParticle::FlushParticle(int sheet)
{
    for (int i = 0; i < MAXPARTICULE*MAXPARTITYPE; i++)
    {
        if (!m_particle[i].used) continue;
        if (m_particle[i].sheet != sheet) continue;

        m_particle[i].used = false;
    }

    for (int i = 0; i < MAXPARTITYPE; i++)
        m_totalInterface[i][sheet] = 0;

    for (int i = 0; i < MAXTRACK; i++)
        m_track[i].used = false;

    if (sheet == SH_WORLD)
    {
        m_wheelTraceTotal = 0;
        m_wheelTraceIndex = 0;
    }
}


//! Returns file name of the effect effectNN.png, with NN = number
static std::filesystem::path NameParticle(int num)
{
    switch(num)
    {
        case 1: return "effect00.png";
        case 2: return "effect01.png";
        case 3: return "effect02.png";
        case 4: return "effect03.png";
        default: return "";
    }
}

//! Returns random letter for use as virus particle
static char RandomLetter()
{
    static std::vector<char> chars;
    if (chars.empty())
    {
        // Add each special character once
        chars.push_back('{');
        chars.push_back('}');
        chars.push_back('(');
        chars.push_back(')');
        chars.push_back('<');
        chars.push_back('>');
        chars.push_back('+');
        chars.push_back('-');
        chars.push_back('=');
        chars.push_back('!');

        // Add each letter once
        for(char c = 'A'; c <= 'Z'; c++) chars.push_back(c);

        // Add each number 4 times
        for(char c = '0'; c <= '9'; c++) for(int i = 0; i < 4; i++) chars.push_back(c);
    }

    return chars[rand()%chars.size()];
}

/** Returns the channel of the particle created or -1 on error. */
int CParticle::CreateParticle(glm::vec3 pos, glm::vec3 speed, const glm::vec2& dim,
                              ParticleType type,
                              float duration, float mass,
                              float windSensitivity, int sheet)
{
    if (m_main == nullptr)
        m_main = CRobotMain::GetInstancePointer();

    int t = -1;
    if ( type == PARTIEXPLOT   ||
         type == PARTIEXPLOO   ||
         type == PARTIMOTOR    ||
         type == PARTIBLITZ    ||
         type == PARTICRASH    ||
         type == PARTIVAPOR    ||
         type == PARTIGAS      ||
         type == PARTIBASE     ||
         type == PARTIFIRE     ||
         type == PARTIFIREZ    ||
         type == PARTIBLUE     ||
         type == PARTIROOT     ||
         type == PARTIRECOVER  ||
         type == PARTIEJECT    ||
         type == PARTISCRAPS   ||
         type == PARTIGUN2     ||
         type == PARTIGUN3     ||
         type == PARTIGUN4     ||
         type == PARTIQUEUE    ||
         type == PARTIORGANIC1 ||
         type == PARTIORGANIC2 ||
         type == PARTIFLAME    ||
         type == PARTIBUBBLE   ||
         type == PARTIERROR    ||
         type == PARTIWARNING  ||
         type == PARTIINFO     ||
         type == PARTISPHERE1  ||
         type == PARTISPHERE2  ||
         type == PARTISPHERE4  ||
         type == PARTISPHERE5  ||
         type == PARTISPHERE6  ||
         type == PARTIPLOUF0   ||
         type == PARTITRACK1   ||
         type == PARTITRACK2   ||
         type == PARTITRACK3   ||
         type == PARTITRACK4   ||
         type == PARTITRACK5   ||
         type == PARTITRACK6   ||
         type == PARTITRACK7   ||
         type == PARTITRACK8   ||
         type == PARTITRACK9   ||
         type == PARTITRACK10  ||
         type == PARTITRACK11  ||
         type == PARTITRACK12  ||
         type == PARTILENS1    ||
         type == PARTILENS2    ||
         type == PARTILENS3    ||
         type == PARTILENS4    ||
         type == PARTIGFLAT    ||
         type == PARTIDROP     ||
         type == PARTIWATER    ||
         type == PARTILIMIT1   ||
         type == PARTILIMIT2   ||
         type == PARTILIMIT3   ||
         type == PARTIEXPLOG1  ||
         type == PARTIEXPLOG2  )
    {
        t = 1;  // effect00
    }
    if ( type == PARTIGLINT   ||
         type == PARTIGLINTb  ||
         type == PARTIGLINTr  ||
         type == PARTITOTO    ||
         type == PARTISELY    ||
         type == PARTISELR    ||
         type == PARTIQUARTZ  ||
         type == PARTIGUNDEL  ||
         type == PARTICONTROL ||
         type == PARTISHOW    ||
         type == PARTICHOC    ||
         type == PARTIFOG4    ||
         type == PARTIFOG5    ||
         type == PARTIFOG6    ||
         type == PARTIFOG7    )
    {
        t = 2;  // effect01
    }
    if ( type == PARTIGUN1    ||
         type == PARTIFLIC    ||
         type == PARTISPHERE0 ||
         type == PARTISPHERE3 ||
         type == PARTIFOG0    ||
         type == PARTIFOG1    ||
         type == PARTIFOG2    ||
         type == PARTIFOG3    )
    {
        t = 3;  // effect02
    }
    if ( type == PARTISMOKE1  ||
         type == PARTISMOKE2  ||
         type == PARTISMOKE3  ||
         type == PARTIBLOOD   ||
         type == PARTIBLOODM  )
    {
        t = 4;  // effect03 (ENG_RSTATE_TTEXTURE_WHITE)
    }
    if ( type == PARTIVIRUS )
    {
        t = 5; // text render
    }
    if (t >= MAXPARTITYPE) return -1;
    if (t == -1) return -1;

    for (int j = 0; j < MAXPARTICULE; j++)
    {
        int i = MAXPARTICULE*t+j;

        if (! m_particle[i].used)
        {
            m_particle[i] = Particle();
            m_particle[i].used      = true;
            m_particle[i].ray       = false;
            m_particle[i].uniqueStamp = m_uniqueStamp++;
            m_particle[i].sheet     = sheet;
            m_particle[i].mass      = mass;
            m_particle[i].duration  = duration;
            m_particle[i].pos       = pos;
            m_particle[i].goal      = pos;
            m_particle[i].speed     = speed;
            m_particle[i].windSensitivity = windSensitivity;
            m_particle[i].dim       = dim;
            m_particle[i].zoom      = 1.0f;
            m_particle[i].angle     = 0.0f;
            m_particle[i].intensity = 1.0f;
            m_particle[i].type      = type;
            m_particle[i].phase     = PARPHSTART;
            m_particle[i].texSup.x  = 0.0f;
            m_particle[i].texSup.y  = 0.0f;
            m_particle[i].texInf.x  = 0.0f;
            m_particle[i].texInf.y  = 0.0f;
            m_particle[i].time      = 0.0f;
            m_particle[i].phaseTime = 0.0f;
            m_particle[i].testTime  = 0.0f;
            m_particle[i].objLink   = nullptr;
            m_particle[i].objFather = nullptr;
            m_particle[i].trackRank = -1;

            m_totalInterface[t][sheet] ++;

            if ( type == PARTIEXPLOT ||
                 type == PARTIEXPLOO )
            {
                m_particle[i].angle = Math::Rand()*Math::PI*2.0f;
            }

            if ( type == PARTIGUN1 ||
                 type == PARTIGUN4 )
            {
                m_particle[i].testTime = 1.0f;  // impact immediately
            }

            if ( type == PARTIVIRUS )
            {
                m_particle[i].text = RandomLetter();
            }

            if ( type >= PARTIFOG0 &&
                 type <= PARTIFOG7 )
            {
                if (m_fogTotal < MAXPARTIFOG)
                m_fog[m_fogTotal++] = i;
            }

            return i | ((m_particle[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}

/** Returns the channel of the particle created or -1 on error */
int CParticle::CreateFrag(glm::vec3 pos, glm::vec3 speed,
                          EngineTriangle *triangle,
                          ParticleType type,
                          float duration, float mass,
                          float windSensitivity, int sheet)
{
    int t = 0;
    for (int j = 0; j < MAXPARTICULE; j++)
    {
        int i = MAXPARTICULE*t+j;

        if (!m_particle[i].used)
        {
            m_particle[i] = Particle();
            m_particle[i].used      = true;
            m_particle[i].ray       = false;
            m_particle[i].uniqueStamp = m_uniqueStamp++;
            m_particle[i].sheet     = sheet;
            m_particle[i].mass      = mass;
            m_particle[i].duration  = duration;
            m_particle[i].pos       = pos;
            m_particle[i].goal      = pos;
            m_particle[i].speed     = speed;
            m_particle[i].windSensitivity = windSensitivity;
            m_particle[i].zoom      = 1.0f;
            m_particle[i].angle     = 0.0f;
            m_particle[i].intensity = 1.0f;
            m_particle[i].type      = type;
            m_particle[i].phase     = PARPHSTART;
            m_particle[i].texSup.x  = 0.0f;
            m_particle[i].texSup.y  = 0.0f;
            m_particle[i].texInf.x  = 0.0f;
            m_particle[i].texInf.y  = 0.0f;
            m_particle[i].time      = 0.0f;
            m_particle[i].phaseTime = 0.0f;
            m_particle[i].testTime  = 0.0f;
            m_particle[i].objLink   = nullptr;
            m_particle[i].objFather = nullptr;
            m_particle[i].trackRank = -1;
            m_triangle[i] = *triangle;

            m_totalInterface[t][sheet] ++;

            glm::vec3    p1;
            p1.x = m_triangle[i].triangle[0].position.x;
            p1.y = m_triangle[i].triangle[0].position.y;
            p1.z = m_triangle[i].triangle[0].position.z;

            glm::vec3 p2;
            p2.x = m_triangle[i].triangle[1].position.x;
            p2.y = m_triangle[i].triangle[1].position.y;
            p2.z = m_triangle[i].triangle[1].position.z;

            glm::vec3 p3;
            p3.x = m_triangle[i].triangle[2].position.x;
            p3.y = m_triangle[i].triangle[2].position.y;
            p3.z = m_triangle[i].triangle[2].position.z;

            float l1 = glm::distance(p1, p2);
            float l2 = glm::distance(p2, p3);
            float l3 = glm::distance(p3, p1);
            float dx = fabs(Math::Min(l1, l2, l3))*0.5f;
            float dy = fabs(Math::Max(l1, l2, l3))*0.5f;
            p1 = glm::vec3(-dx,  dy, 0.0f);
            p2 = glm::vec3( dx,  dy, 0.0f);
            p3 = glm::vec3(-dx, -dy, 0.0f);

            m_triangle[i].triangle[0].position.x = p1.x;
            m_triangle[i].triangle[0].position.y = p1.y;
            m_triangle[i].triangle[0].position.z = p1.z;

            m_triangle[i].triangle[1].position.x = p2.x;
            m_triangle[i].triangle[1].position.y = p2.y;
            m_triangle[i].triangle[1].position.z = p2.z;

            m_triangle[i].triangle[2].position.x = p3.x;
            m_triangle[i].triangle[2].position.y = p3.y;
            m_triangle[i].triangle[2].position.z = p3.z;

            glm::vec3 n(0.0f, 0.0f, -1.0f);

            m_triangle[i].triangle[0].normal.x = n.x;
            m_triangle[i].triangle[0].normal.y = n.y;
            m_triangle[i].triangle[0].normal.z = n.z;

            m_triangle[i].triangle[1].normal.x = n.x;
            m_triangle[i].triangle[1].normal.y = n.y;
            m_triangle[i].triangle[1].normal.z = n.z;

            m_triangle[i].triangle[2].normal.x = n.x;
            m_triangle[i].triangle[2].normal.y = n.y;
            m_triangle[i].triangle[2].normal.z = n.z;

            if (type == PARTIFRAG)
                m_particle[i].angle = Math::Rand()*Math::PI*2.0f;

            return i | ((m_particle[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}


/** Returns the channel of the particle created or -1 on error */
int CParticle::CreatePart(glm::vec3 pos, glm::vec3 speed,
                          ParticleType type,
                          float duration, float mass, float weight,
                          float windSensitivity, int sheet)
{
    int t = 0;
    for (int j = 0; j < MAXPARTICULE; j++)
    {
        int i = MAXPARTICULE*t+j;

        if (!m_particle[i].used)
        {
            m_particle[i] = Particle();
            m_particle[i].used      = true;
            m_particle[i].ray       = false;
            m_particle[i].uniqueStamp = m_uniqueStamp++;
            m_particle[i].sheet     = sheet;
            m_particle[i].mass      = mass;
            m_particle[i].weight    = weight;
            m_particle[i].duration  = duration;
            m_particle[i].pos       = pos;
            m_particle[i].goal      = pos;
            m_particle[i].speed     = speed;
            m_particle[i].windSensitivity = windSensitivity;
            m_particle[i].zoom      = 1.0f;
            m_particle[i].angle     = 0.0f;
            m_particle[i].intensity = 1.0f;
            m_particle[i].type      = type;
            m_particle[i].phase     = PARPHSTART;
            m_particle[i].texSup.x  = 0.0f;
            m_particle[i].texSup.y  = 0.0f;
            m_particle[i].texInf.x  = 0.0f;
            m_particle[i].texInf.y  = 0.0f;
            m_particle[i].time      = 0.0f;
            m_particle[i].phaseTime = 0.0f;
            m_particle[i].testTime  = 0.0f;
            m_particle[i].trackRank = -1;

            m_totalInterface[t][sheet] ++;

            return i | ((m_particle[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}

/** Returns the channel of the particle created or -1 on error */
int CParticle::CreateRay(glm::vec3 pos, glm::vec3 goal,
                          ParticleType type, const glm::vec2& dim,
                          float duration, int sheet)
{
    int t = -1;
    if ( type == PARTIRAY1 ||
         type == PARTIRAY2 ||
         type == PARTIRAY3 )
    {
        t = 3;  // effect02
    }
    if (t >= MAXPARTITYPE) return -1;
    if (t == -1) return -1;

    for (int j = 0; j < MAXPARTICULE; j++)
    {
        int i = MAXPARTICULE*t+j;

        if (!m_particle[i].used)
        {
            m_particle[i] = Particle();
            m_particle[i].used      = true;
            m_particle[i].ray       = true;
            m_particle[i].uniqueStamp = m_uniqueStamp++;
            m_particle[i].sheet     = sheet;
            m_particle[i].mass      = 0.0f;
            m_particle[i].duration  = duration;
            m_particle[i].pos       = pos;
            m_particle[i].goal      = goal;
            m_particle[i].speed     = glm::vec3(0.0f, 0.0f, 0.0f);
            m_particle[i].windSensitivity = 0.0f;
            m_particle[i].dim       = dim;
            m_particle[i].zoom      = 1.0f;
            m_particle[i].angle     = 0.0f;
            m_particle[i].intensity = 1.0f;
            m_particle[i].type      = type;
            m_particle[i].phase     = PARPHSTART;
            m_particle[i].texSup.x  = 0.0f;
            m_particle[i].texSup.y  = 0.0f;
            m_particle[i].texInf.x  = 0.0f;
            m_particle[i].texInf.y  = 0.0f;
            m_particle[i].time      = 0.0f;
            m_particle[i].phaseTime = 0.0f;
            m_particle[i].testTime  = 0.0f;
            m_particle[i].objLink   = nullptr;
            m_particle[i].objFather = nullptr;
            m_particle[i].trackRank = -1;

            m_totalInterface[t][sheet] ++;

            return i | ((m_particle[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}

/** "length" is the length of the tail of drag (in seconds)! */
int CParticle::CreateTrack(glm::vec3 pos, glm::vec3 speed, const glm::vec2& dim,
                           ParticleType type, float duration, float mass,
                           float length, float width)
{
    // Creates the normal particle.
    int channel = CreateParticle(pos, speed, dim, type, duration, mass, 0.0f, 0);
    if (channel == -1) return -1;

    // Seeks a streak free.
    for (int i = 0; i < MAXTRACK; i++)
    {
        if (!m_track[i].used)  // free?
        {
            int rank = channel;
            if (!CheckChannel(rank)) return -1;
            m_particle[rank].trackRank = i;

            m_track[i].used = true;
            m_track[i].step = (length/duration) / MAXTRACKLEN;
            m_track[i].last = 0.0f;
            m_track[i].intensity = 1.0f;
            m_track[i].width = width;
            m_track[i].posUsed = 1;
            m_track[i].head = 0;
            m_track[i].pos[0] = pos;
            break;
        }
    }

    return channel;
}

void CParticle::CreateWheelTrace(const glm::vec3 &p1, const glm::vec3 &p2,
                                 const glm::vec3 &p3, const glm::vec3 &p4,
                                 TraceColor color)
{
    int max = MAXWHEELTRACE;
    int i = m_wheelTraceIndex++;
    if (m_wheelTraceIndex > max)  m_wheelTraceIndex = 0;

    m_wheelTrace[i].color = color;
    m_wheelTrace[i].pos[0] = p1;  // ul
    m_wheelTrace[i].pos[1] = p2;  // dl
    m_wheelTrace[i].pos[2] = p3;  // ur
    m_wheelTrace[i].pos[3] = p4;  // dr

    if (m_terrain == nullptr)
        m_terrain = m_main->GetTerrain();

    m_terrain->AdjustToFloor(m_wheelTrace[i].pos[0]);
    m_wheelTrace[i].pos[0].y += 0.2f;  // just above the ground

    m_terrain->AdjustToFloor(m_wheelTrace[i].pos[1]);
    m_wheelTrace[i].pos[1].y += 0.2f;  // just above the ground

    m_terrain->AdjustToFloor(m_wheelTrace[i].pos[2]);
    m_wheelTrace[i].pos[2].y += 0.2f;  // just above the ground

    m_terrain->AdjustToFloor(m_wheelTrace[i].pos[3]);
    m_wheelTrace[i].pos[3].y += 0.2f;  // just above the ground

    if (m_wheelTraceTotal < max)
        m_wheelTraceTotal++;
    else
        m_wheelTraceTotal = max;
}



/** Adapts the channel so it can be used as an offset in m_particle */
bool CParticle::CheckChannel(int &channel)
{
    int uniqueStamp = (channel>>16)&0xffff;
    channel &= 0xffff;

    if (channel < 0)  return false;
    if (channel >= MAXPARTICULE*MAXPARTITYPE) return false;

    if (!m_particle[channel].used)
    {
        GetLogger()->Trace("Particle %%:%% doesn't exist anymore (used=false)", channel, uniqueStamp);
        return false;
    }

    if (m_particle[channel].uniqueStamp != uniqueStamp)
    {
        GetLogger()->Trace("Particle %%:%% doesn't exist anymore (uniqueStamp changed)", channel, uniqueStamp);
        return false;
    }

    return true;
}

void CParticle::DeleteRank(int rank)
{
    if (m_totalInterface[rank/MAXPARTICULE][m_particle[rank].sheet] > 0)
        m_totalInterface[rank/MAXPARTICULE][m_particle[rank].sheet]--;

    int i = m_particle[rank].trackRank;
    if (i != -1)  // drag associated?
        m_track[i].used = false;  // frees the drag

    m_particle[rank].used = false;
}

void CParticle::DeleteParticle(ParticleType type)
{
    for (int i = 0; i < MAXPARTICULE*MAXPARTITYPE; i++)
    {
        if (!m_particle[i].used) continue;
        if (m_particle[i].type != type) continue;

        DeleteRank(i);
    }
}

void CParticle::DeleteParticle(int channel)
{
    if (!CheckChannel(channel)) return;

    if (m_totalInterface[channel/MAXPARTICULE][m_particle[channel].sheet] > 0 )
        m_totalInterface[channel/MAXPARTICULE][m_particle[channel].sheet]--;

    int i = m_particle[channel].trackRank;
    if (i != -1)  // drag associated?
        m_track[i].used = false;  // frees the drag

    m_particle[channel].used = false;
}

void CParticle::SetObjectLink(int channel, CObject *object)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].objLink = object;
}

void CParticle::SetObjectFather(int channel, CObject *object)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].objFather = object;
}

void CParticle::SetPosition(int channel, glm::vec3 pos)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].pos = pos;
}

void CParticle::SetDimension(int channel, const glm::vec2& dim)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].dim = dim;
}

void CParticle::SetZoom(int channel, float zoom)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].zoom = zoom;
}

void CParticle::SetAngle(int channel, float angle)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].angle = angle;
}

void CParticle::SetIntensity(int channel, float intensity)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].intensity = intensity;
}

void CParticle::SetParam(int channel, glm::vec3 pos, const glm::vec2& dim, float zoom,
                          float angle, float intensity)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].pos       = pos;
    m_particle[channel].dim       = dim;
    m_particle[channel].zoom      = zoom;
    m_particle[channel].angle     = angle;
    m_particle[channel].intensity = intensity;
}

void CParticle::SetPhase(int channel, ParticlePhase phase, float duration)
{
    if (!CheckChannel(channel))  return;
    m_particle[channel].phase = phase;
    m_particle[channel].duration = duration;
    m_particle[channel].phaseTime = m_particle[channel].time;
}

bool CParticle::GetPosition(int channel, glm::vec3 &pos)
{
    if (!CheckChannel(channel))  return false;
    pos = m_particle[channel].pos;
    return true;
}

void CParticle::SetFrameUpdate(int sheet, bool update)
{
    m_frameUpdate[sheet] = update;
}

void CParticle::FrameParticle(float rTime)
{
    if (m_main == nullptr)
        m_main = CRobotMain::GetInstancePointer();

    bool pause = (m_engine->GetPause() && !m_main->GetInfoLock());

    if (m_terrain == nullptr)
        m_terrain = m_main->GetTerrain();

    if (m_water == nullptr)
        m_water = m_engine->GetWater();

    if (!pause)
    {
        m_lastTimeGunDel += rTime;
        m_absTime += rTime;
    }

    glm::vec3 wind = m_terrain->GetWind();
    glm::vec3 eye = m_engine->GetEyePt();

    glm::vec3 pos = { 0, 0, 0 };

    for (int i = 0; i < MAXPARTICULE*MAXPARTITYPE; i++)
    {
        glm::vec2 ts(0), ti(0);
        if (!m_particle[i].used) continue;
        if (!m_frameUpdate[m_particle[i].sheet]) continue;

        if (m_particle[i].type != PARTISHOW)
        {
            if (pause && m_particle[i].sheet != SH_INTERFACE) continue;
        }

        if (m_particle[i].type != PARTIQUARTZ)
            m_particle[i].pos += m_particle[i].speed*rTime;

        if (m_particle[i].sheet == SH_WORLD)
        {
            float h = rTime*m_particle[i].windSensitivity*Math::Rand()*2.0f;
            m_particle[i].pos += wind*h;
        }

        float progress = (m_particle[i].time-m_particle[i].phaseTime)/m_particle[i].duration;

        // Manages the particles with mass that bounce.
        if ( m_particle[i].mass != 0.0f        &&
             m_particle[i].type != PARTIQUARTZ )
        {
            m_particle[i].speed.y -= m_particle[i].mass*rTime;

            float h;
            if (m_particle[i].sheet == SH_INTERFACE)
                h = 0.0f;
            else
                h = m_terrain->GetFloorLevel(m_particle[i].pos, true);

            h += m_particle[i].dim.y*0.75f;
            if (m_particle[i].pos.y < h)  // impact with the ground?
            {
                if ( m_particle[i].type == PARTIPART &&
                     m_particle[i].weight > 3.0f &&  // heavy enough?
                     m_particle[i].bounce < 3 )
                {
                    float amplitude = m_particle[i].weight*0.1f;
                    amplitude *= 1.0f-0.3f*m_particle[i].bounce;
                    if (amplitude > 1.0f)  amplitude = 1.0f;
                    if (amplitude > 0.0f)
                    {
                        Play(SOUND_BOUM, m_particle[i].pos, amplitude);
                    }
                }

                if (m_particle[i].bounce < 3)
                {
                    m_particle[i].pos.y = h;
                    m_particle[i].speed.y *= -0.4f;
                    m_particle[i].speed.x *=  0.4f;
                    m_particle[i].speed.z *=  0.4f;
                    m_particle[i].bounce ++;  // more impact
                }
                else    // disappears after 3 bounces?
                {
                    if ( m_particle[i].pos.y < h-10.0f ||
                         m_particle[i].time >= 20.0f   )
                    {
                        DeleteRank(i);
                        continue;
                    }
                }
            }
        }

        // Manages drag associated.
        int r = m_particle[i].trackRank;
        if (r != -1)  // drag exists?
        {
            if (TrackMove(r, m_particle[i].pos, progress))
            {
                DeleteRank(i);
                continue;
            }

            m_track[r].drawParticle = (progress < 1.0f);
        }

        if (m_particle[i].type == PARTITRACK1)  // technical explosion?
        {
            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.375f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTITRACK2)  // spray blue?
        {
            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.500f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTITRACK3)  // spider?
        {
            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.500f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTITRACK4)  // insect explosion?
        {
            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.625f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTITRACK5)  // derrick?
        {
            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.750f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTITRACK6)  // reset in/out?
        {
            ts.x = 0.0f;
            ts.y = 0.0f;
            ti.x = 0.0f;
            ti.y = 0.0f;
        }

        if ( m_particle[i].type == PARTITRACK7  ||  // win-1 ?
             m_particle[i].type == PARTITRACK8  ||  // win-2 ?
             m_particle[i].type == PARTITRACK9  ||  // win-3 ?
             m_particle[i].type == PARTITRACK10 )   // win-4 ?
        {
            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.25f*(m_particle[i].type-PARTITRACK7);
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTITRACK11)  // phazer shot?
        {
            CObject* object = SearchObjectGun(m_particle[i].goal, m_particle[i].pos, m_particle[i].type, m_particle[i].objFather);
            m_particle[i].goal = m_particle[i].pos;
            if (object != nullptr && object->Implements(ObjectInterfaceType::Damageable))
            {
                dynamic_cast<CDamageableObject&>(*object).DamageObject(DamageType::Phazer, 0.002f, m_particle[i].objFather);
            }

            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.375f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTITRACK12)  // drag reactor?
        {
            m_particle[i].zoom = 1.0f;

            ts.x = 0.375f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIMOTOR)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.000f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIBLITZ)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress;
            m_particle[i].angle = Math::Rand()*Math::PI*2.0f;

            ts.x = 0.125f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTICRASH)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.25f)
                m_particle[i].zoom = progress/0.25f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.25f)/0.75f;

            ts.x = 0.000f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIVAPOR)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].intensity = 1.0f-progress;
            m_particle[i].zoom = 1.0f+progress*3.0f;

            ts.x = 0.000f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIGAS)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress;

            ts.x = 0.375f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIBASE)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f+progress*7.0f;
            m_particle[i].intensity = powf(1.0f-progress, 3.0f);

            ts.x = 0.375f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particle[i].type == PARTIFIRE  ||
             m_particle[i].type == PARTIFIREZ )
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (m_particle[i].type == PARTIFIRE)
                m_particle[i].zoom = 1.0f-progress;
            else
                m_particle[i].zoom = progress;

            ts.x = 0.500f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIGUN1)  // fireball shot?
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (m_particle[i].testTime >= 0.05f)
            {
                m_particle[i].testTime = 0.0f;

                if (m_terrain->GetHeightToFloor(m_particle[i].pos, true) < -2.0f)
                {
                    m_exploGunCounter++;

                    if (m_exploGunCounter % 2 == 0)
                    {
                        pos = m_particle[i].goal;
                        m_terrain->AdjustToFloor(pos, true);
                        glm::vec3 speed;
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = 0.0f;
                        glm::vec2 dim;
                        dim.x = Math::Rand()*6.0f+6.0f;
                        dim.y = dim.x;
                        float duration = Math::Rand()*1.0f+1.0f;
                        float mass = 0.0f;
                        CreateParticle(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);

                        pos.y += 1.0f;
                        int total = static_cast<int>(2.0f*m_engine->GetParticleDensity());
                        for (int j = 0; j < total; j++)
                        {
                            speed.x = (Math::Rand()-0.5f)*20.0f;
                            speed.z = (Math::Rand()-0.5f)*20.0f;
                            speed.y = Math::Rand()*20.0f;
                            dim.x = 1.0f;
                            dim.y = dim.x;
                            duration = Math::Rand()*1.0f+1.0f;
                            mass = Math::Rand()*10.0f+15.0f;
                            CreateParticle(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);
                        }
                    }

                    if (m_exploGunCounter % 4 == 0)
                        Play(SOUND_EXPLOg1, pos, 0.5f);

                    DeleteRank(i);
                    continue;
                }

                CObject* object = SearchObjectGun(m_particle[i].goal, m_particle[i].pos, m_particle[i].type, m_particle[i].objFather);
                m_particle[i].goal = m_particle[i].pos;
                if (object != nullptr)
                {
                    if (object->Implements(ObjectInterfaceType::Damageable))
                    {
                        dynamic_cast<CDamageableObject&>(*object).DamageObject(DamageType::Fire, 0.001f, m_particle[i].objFather);
                    }

                    m_exploGunCounter++;

                    if (m_exploGunCounter % 2 == 0)
                    {
                        pos = m_particle[i].pos;
                        glm::vec3 speed;
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = 0.0f;
                        glm::vec2 dim;
                        dim.x = Math::Rand()*6.0f+6.0f;
                        dim.y = dim.x;
                        float duration = Math::Rand()*1.0f+1.0f;
                        float mass = 0.0f;
                        CreateParticle(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);

                        pos.y += 1.0f;
                        int total = static_cast<int>(2.0f*m_engine->GetParticleDensity());
                        for (int j = 0; j < total; j++)
                        {
                            speed.x = (Math::Rand()-0.5f)*20.0f;
                            speed.z = (Math::Rand()-0.5f)*20.0f;
                            speed.y = Math::Rand()*20.0f;
                            dim.x = 1.0f;
                            dim.y = dim.x;
                            duration = Math::Rand()*1.0f+1.0f;
                            mass = Math::Rand()*10.0f+15.0f;
                            CreateParticle(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);
                        }
                    }

                    if (m_exploGunCounter % 4 == 0)
                        Play(SOUND_EXPLOg1, pos, 0.5f);

                    DeleteRank(i);
                    continue;
                }
            }

            m_particle[i].angle -= rTime*Math::PI*8.0f;
            m_particle[i].zoom = 1.0f-progress;

            ts.x = 0.00f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIGUN2)  // ant shot?
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (m_particle[i].testTime >= 0.1f)
            {
                m_particle[i].testTime = 0.0f;
                CObject* object = SearchObjectGun(m_particle[i].goal, m_particle[i].pos, m_particle[i].type, m_particle[i].objFather);
                m_particle[i].goal = m_particle[i].pos;
                if (object != nullptr)
                {
                    if (object->GetType() == OBJECT_MOBILErs && dynamic_cast<CShielder&>(*object).GetActiveShieldRadius() > 0.0f)  // protected by shield?
                    {
                        CreateParticle(m_particle[i].pos, glm::vec3(0.0f, 0.0f, 0.0f), { 6.0f, 6.0f }, PARTIGUNDEL, 2.0f);
                        if (m_lastTimeGunDel > 0.2f)
                        {
                            m_lastTimeGunDel = 0.0f;
                            Play(SOUND_GUNDEL, m_particle[i].pos, 1.0f);
                        }
                        DeleteRank(i);
                        continue;
                    }
                    else
                    {
                        if (object->GetType() != OBJECT_HUMAN)
                            Play(SOUND_TOUCH, m_particle[i].pos, 1.0f);

                        if (object->Implements(ObjectInterfaceType::Damageable))
                        {
                            dynamic_cast<CDamageableObject&>(*object).DamageObject(DamageType::AlienAnt, 0.1f, m_particle[i].objFather);  // starts explosion
                        }
                    }
                }
            }

            m_particle[i].angle = Math::Rand()*Math::PI*2.0f;
            m_particle[i].zoom = 1.0f-progress;

            ts.x = 0.125f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIGUN3)  // spider suicides?
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (m_particle[i].testTime >= 0.1f)
            {
                m_particle[i].testTime = 0.0f;
                CObject* object = SearchObjectGun(m_particle[i].goal, m_particle[i].pos, m_particle[i].type, m_particle[i].objFather);
                m_particle[i].goal = m_particle[i].pos;
                if (object != nullptr)
                {
                    if (object->GetType() == OBJECT_MOBILErs && dynamic_cast<CShielder&>(*object).GetActiveShieldRadius() > 0.0f)
                    {
                        CreateParticle(m_particle[i].pos, glm::vec3(0.0f, 0.0f, 0.0f), { 6.0f, 6.0f }, PARTIGUNDEL, 2.0f);
                        if (m_lastTimeGunDel > 0.2f)
                        {
                            m_lastTimeGunDel = 0.0f;
                            Play(SOUND_GUNDEL, m_particle[i].pos, 1.0f);
                        }
                        DeleteRank(i);
                        continue;
                    }
                    else
                    {
                        if (object->Implements(ObjectInterfaceType::Damageable))
                        {
                            dynamic_cast<CDamageableObject&>(*object).DamageObject(DamageType::Fire, std::numeric_limits<float>::infinity(), m_particle[i].objFather);  // starts explosion
                        }
                    }
                }
            }

            ts.x = 0.500f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIGUN4)  // orgaball shot?
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (m_particle[i].testTime >= 0.05f)
            {
                m_particle[i].testTime = 0.0f;

                if (m_terrain->GetHeightToFloor(m_particle[i].pos, true) < -2.0f)
                {
                    m_exploGunCounter ++;

                    if (m_exploGunCounter % 2 == 0)
                    {
                        pos = m_particle[i].goal;
                        m_terrain->AdjustToFloor(pos, true);
                        glm::vec3 speed = { 0, 0, 0 };
                        glm::vec2 dim;
                        dim.x = Math::Rand()*4.0f+2.0f;
                        dim.y = dim.x;
                        float duration = Math::Rand()*0.7f+0.7f;
                        float mass = 0.0f;
                        CreateParticle(pos, speed, dim, PARTIEXPLOG2, duration, mass, 1.0f);
                    }

                    if (m_exploGunCounter % 4 == 0)
                    {
                        Play(SOUND_EXPLOg2, pos, 0.5f);
                    }

                    DeleteRank(i);
                    continue;
                }

                CObject* object = SearchObjectGun(m_particle[i].goal, m_particle[i].pos, m_particle[i].type, m_particle[i].objFather);
                m_particle[i].goal = m_particle[i].pos;
                if (object != nullptr)
                {
                    if (object->Implements(ObjectInterfaceType::Damageable))
                    {
                        dynamic_cast<CDamageableObject&>(*object).DamageObject(DamageType::OrgaShooter, 0.001f, m_particle[i].objFather);
                    }

                    m_exploGunCounter ++;

                    if (m_exploGunCounter % 2 == 0)
                    {
                        pos = m_particle[i].pos;
                        glm::vec3 speed = { 0, 0, 0 };
                        glm::vec2 dim;
                        dim.x = Math::Rand()*4.0f+2.0f;
                        dim.y = dim.x;
                        float duration = Math::Rand()*0.7f+0.7f;
                        float mass = 0.0f;
                        CreateParticle(pos, speed, dim, PARTIEXPLOG2, duration, mass, 1.0f);
                    }

                    if (m_exploGunCounter % 4 == 0)
                        Play(SOUND_EXPLOg2, pos, 0.5f);

                    DeleteRank(i);
                    continue;
                }
            }

            m_particle[i].angle = Math::Rand()*Math::PI*2.0f;
            m_particle[i].zoom = 1.0f-progress;

            ts.x = 0.125f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIFLIC)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 0.1f+progress;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.00f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTISHOW)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.5f) m_particle[i].intensity = progress/0.5f;
            else                 m_particle[i].intensity = 2.0f-progress/0.5f;
            m_particle[i].zoom = 1.0f-progress*0.8f;
            m_particle[i].angle -= rTime*Math::PI*0.5f;

            ts.x = 0.50f;
            ts.y = 0.00f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTICHOC)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 0.1f+progress;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.50f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIGFLAT)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 0.1f+progress;
            m_particle[i].intensity = 1.0f-progress;
            m_particle[i].angle -= rTime*Math::PI*2.0f;

            ts.x = 0.00f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTILIMIT1)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f;
            m_particle[i].intensity = 1.0f;

            ts.x = 0.000f;
            ts.y = 0.125f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }
        if (m_particle[i].type == PARTILIMIT2)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f;
            m_particle[i].intensity = 1.0f;

            ts.x = 0.375f;
            ts.y = 0.125f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }
        if (m_particle[i].type == PARTILIMIT3)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f;
            m_particle[i].intensity = 1.0f;

            ts.x = 0.500f;
            ts.y = 0.125f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIFOG0)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.3f+sinf(progress)*0.15f;
            m_particle[i].angle += rTime*0.05f;

            ts.x = 0.25f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }
        if (m_particle[i].type == PARTIFOG1)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.3f+sinf(progress)*0.15f;
            m_particle[i].angle -= rTime*0.07f;

            ts.x = 0.25f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIFOG2)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.6f+sinf(progress)*0.15f;
            m_particle[i].angle += rTime*0.05f;

            ts.x = 0.75f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }
        if (m_particle[i].type == PARTIFOG3)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.6f+sinf(progress)*0.15f;
            m_particle[i].angle -= rTime*0.07f;

            ts.x = 0.75f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIFOG4)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.5f+sinf(progress)*0.2f;
            m_particle[i].angle += rTime*0.05f;

            ts.x = 0.00f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }
        if (m_particle[i].type == PARTIFOG5)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.5f+sinf(progress)*0.2f;
            m_particle[i].angle -= rTime*0.07f;

            ts.x = 0.00f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIFOG6)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.5f+sinf(progress)*0.2f;
            m_particle[i].angle += rTime*0.05f;

            ts.x = 0.50f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }
        if (m_particle[i].type == PARTIFOG7)
        {
            m_particle[i].zoom = progress;
            m_particle[i].intensity = 0.5f+sinf(progress)*0.2f;
            m_particle[i].angle -= rTime*0.07f;

            ts.x = 0.50f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        // Decreases the intensity if the camera
        // is almost at the same height (fog was eye level).
        if ( m_particle[i].type >= PARTIFOG0 &&
             m_particle[i].type <= PARTIFOG7 )
        {
            float h = 10.0f;

            if ( m_particle[i].pos.y >= eye.y   &&
                 m_particle[i].pos.y <  eye.y+h )
            {
                m_particle[i].intensity *= (m_particle[i].pos.y-eye.y)/h;
            }
            if ( m_particle[i].pos.y >  eye.y-h &&
                 m_particle[i].pos.y <  eye.y   )
            {
                m_particle[i].intensity *= (eye.y-m_particle[i].pos.y)/h;
            }
        }

        if ( m_particle[i].type == PARTIEXPLOT ||
             m_particle[i].type == PARTIEXPLOO )
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress/2.0f;
            m_particle[i].intensity = 1.0f-progress;

            if (m_particle[i].type == PARTIEXPLOT)  ts.x = 0.750f;
            else                                    ts.x = 0.875f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIEXPLOG1)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.375f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }
        if (m_particle[i].type == PARTIEXPLOG2)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.625f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIFLAME)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress/2.0f;
            if (progress < 0.5f)
            {
                m_particle[i].intensity = progress/0.5f;
            }
            else
            {
                m_particle[i].intensity = 2.0f-progress/0.5f;
            }

            ts.x = 0.750f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIBUBBLE)
        {
            if ( progress >= 1.0f ||
                 m_particle[i].pos.y >= m_water->GetLevel() )
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress/2.0f;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.250f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particle[i].type == PARTISMOKE1 ||
             m_particle[i].type == PARTISMOKE2 ||
             m_particle[i].type == PARTISMOKE3 )
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.25f)
            {
                m_particle[i].zoom = progress/0.25f;
            }
            else
            {
                m_particle[i].intensity = 1.0f-(progress-0.25f)/0.75f;
            }

            ts.x = 0.500f+0.125f*(m_particle[i].type-PARTISMOKE1);
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIBLOOD)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.750f+(rand()%2)*0.125f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIBLOODM)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.875f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particle[i].type == PARTIVIRUS )
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.25f)
                m_particle[i].zoom = progress/0.25f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.25f)/0.75f;

            m_particle[i].angle += rTime*Math::PI*1.0f;
        }

        if (m_particle[i].type == PARTIBLUE)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress;

            ts.x = 0.625f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIROOT)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.25f)
            {
                m_particle[i].zoom = progress/0.25f;
            }
            else
            {
                m_particle[i].intensity = 1.0f-(progress-0.25f)/0.75f;
            }

            ts.x = 0.000f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIRECOVER)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.25f)
            {
                m_particle[i].zoom = progress/0.25f;
            }
            else
            {
                m_particle[i].intensity = 1.0f-(progress-0.25f)/0.75f;
            }

            ts.x = 0.875f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIEJECT)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f+powf(progress, 2.0f)*5.0f;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.625f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTISCRAPS)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress;

            ts.x = 0.625f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIFRAG)
        {
            m_particle[i].angle += rTime*Math::PI*0.5f;

            ts.x = 0.0f;
            ts.y = 0.0f;
            ti.x = 0.0f;
            ti.y = 0.0f;
        }

        if (m_particle[i].type == PARTIPART)
        {
            ts.x = 0.0f;
            ts.y = 0.0f;
            ti.x = 0.0f;
            ti.y = 0.0f;
        }

        if (m_particle[i].type == PARTIQUEUE)
        {
            if (m_particle[i].testTime >= 0.05f)
            {
                m_particle[i].testTime = 0.0f;

                pos = m_particle[i].pos;
                glm::vec3 speed = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec2 dim;
                dim.x = 1.0f*(Math::Rand()*0.8f+0.6f);
                dim.y = dim.x;
                CreateParticle(pos, speed, dim, PARTIGAS, 0.5f);
            }

            ts.x = 0.375f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIORGANIC1)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);

                pos = m_particle[i].pos;
                glm::vec2 dim;
                dim.x    = m_particle[i].dim.x/4.0f;
                dim.y    = dim.x;
                float duration = m_particle[i].duration;
                float mass     = m_particle[i].mass;
                int total = static_cast<int>((10.0f*m_engine->GetParticleDensity()));
                for (int j = 0; j < total; j++)
                {
                    glm::vec3 speed;
                    speed.x = (Math::Rand()-0.5f)*20.0f;
                    speed.y = (Math::Rand()-0.5f)*20.0f;
                    speed.z = (Math::Rand()-0.5f)*20.0f;
                    CreateParticle(pos, speed, dim, PARTIORGANIC2, duration, mass);
                }
                total = static_cast<int>((5.0f*m_engine->GetParticleDensity()));
                for (int j = 0; j < total; j++)
                {
                    glm::vec3 speed;
                    speed.x = (Math::Rand()-0.5f)*20.0f;
                    speed.y = (Math::Rand()-0.5f)*20.0f;
                    speed.z = (Math::Rand()-0.5f)*20.0f;
                    duration *= Math::Rand()+0.8f;
                    CreateTrack(pos, speed, dim, PARTITRACK4, duration, mass, duration*0.2f, dim.x*2.0f);
                }
                continue;
            }

            m_particle[i].zoom = (m_particle[i].time-m_particle[i].duration);

            ts.x = 0.125f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIORGANIC2)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration);

            ts.x = 0.125f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIGLINT)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress > 0.5f)
                m_particle[i].zoom = 1.0f-(progress-0.5f)*2.0f;

            m_particle[i].angle = m_particle[i].time*Math::PI;

            ts.x = 0.75f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIGLINTb)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress > 0.5f)
                m_particle[i].zoom = 1.0f-(progress-0.5f)*2.0f;

            m_particle[i].angle = m_particle[i].time*Math::PI;

            ts.x = 0.75f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIGLINTr)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress > 0.5f)
                m_particle[i].zoom = 1.0f-(progress-0.5f)*2.0f;

            m_particle[i].angle = m_particle[i].time*Math::PI;

            ts.x = 0.75f;
            ts.y = 0.00f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if ( m_particle[i].type >= PARTILENS1 &&
             m_particle[i].type <= PARTILENS4 )
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.5f)
                m_particle[i].zoom = progress*2.0f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.5f)*2.0f;

            ts.x = 0.25f*(m_particle[i].type-PARTILENS1);
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTICONTROL)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.3f)
            {
                m_particle[i].zoom = progress/0.3f;
            }
            else
            {
                m_particle[i].zoom = 1.0f;
                m_particle[i].intensity = 1.0f-(progress-0.3f)/0.7f;
            }

            ts.x = 0.00f;
            ts.y = 0.00f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIGUNDEL)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress > 0.5f)
                m_particle[i].zoom = 1.0f-(m_particle[i].time-m_particle[i].duration/2.0f);

            m_particle[i].angle = m_particle[i].time*Math::PI;

            ts.x = 0.75f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIQUARTZ)
        {
            if (progress >= 1.0f)
            {
                m_particle[i].time = 0.0f;
                m_particle[i].duration = 0.5f+Math::Rand()*2.0f;
                m_particle[i].pos.x = m_particle[i].speed.x + (Math::Rand()-0.5f)*m_particle[i].mass;
                m_particle[i].pos.y = m_particle[i].speed.y + (Math::Rand()-0.5f)*m_particle[i].mass;
                m_particle[i].pos.z = m_particle[i].speed.z + (Math::Rand()-0.5f)*m_particle[i].mass;
                m_particle[i].dim.x = 0.5f+Math::Rand()*1.5f;
                m_particle[i].dim.y = m_particle[i].dim.x;
                progress = 0.0f;
            }

            if (progress < 0.2f)
            {
                m_particle[i].zoom = progress/0.2f;
                m_particle[i].intensity = 1.0f;
            }
            else
            {
                m_particle[i].zoom = 1.0f;
                m_particle[i].intensity = 1.0f-(progress-0.2f)/0.8f;
            }

            ts.x = 0.25f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTITOTO)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress;
            if (progress < 0.15f)
                m_particle[i].intensity = progress/0.15f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.15f)/0.85f;

            m_particle[i].intensity *= 0.5f;

            ts.x = 0.25f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIERROR)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = progress*1.0f;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.500f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIWARNING)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = progress*1.0f;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.875f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIINFO)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = progress*1.0f;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.750f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTISELY)
        {
            ts.x = 0.75f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }
        if (m_particle[i].type == PARTISELR)
        {
            ts.x = 0.75f;
            ts.y = 0.00f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTISPHERE0)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = progress*m_particle[i].dim.x;

            if (progress < 0.65f)
                m_particle[i].intensity = progress/0.65f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.65f)/0.35f;

            m_particle[i].intensity *= 0.5f;

            ts.x = 0.50f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTISPHERE1)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.30f)
                m_particle[i].intensity = progress/0.30f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.30f)/0.70f;

            m_particle[i].zoom = progress*m_particle[i].dim.x;
            m_particle[i].angle = m_particle[i].time*Math::PI*2.0f;

            ts.x = 0.000f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTISPHERE2)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (progress < 0.20f)
                m_particle[i].intensity = 1.0f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.20f)/0.80f;

            m_particle[i].zoom = progress*m_particle[i].dim.x;
            m_particle[i].angle = m_particle[i].time*Math::PI*2.0f;

            ts.x = 0.125f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTISPHERE3)
        {
            if (m_particle[i].phase == PARPHEND &&
                progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (m_particle[i].phase == PARPHSTART)
            {
                m_particle[i].intensity = progress;
                if (m_particle[i].intensity > 1.0f)
                    m_particle[i].intensity = 1.0f;
            }

            if (m_particle[i].phase == PARPHEND)
                m_particle[i].intensity = 1.0f-progress;

            m_particle[i].zoom = m_particle[i].dim.x;
            m_particle[i].angle = m_particle[i].time*Math::PI*0.2f;

            ts.x = 0.25f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTISPHERE4)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = progress*m_particle[i].dim.x;

            if (progress < 0.65 )
                m_particle[i].intensity = progress/0.65f;
            else
                m_particle[i].intensity = 1.0f-(progress-0.65f)/0.35f;

            m_particle[i].intensity *= 0.5f;

            ts.x = 0.125f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTISPHERE5)
        {
            m_particle[i].intensity = 0.7f+sinf(progress)*0.3f;
            m_particle[i].zoom = m_particle[i].dim.x*(1.0f+sinf(progress*0.7f)*0.01f);
            m_particle[i].angle = m_particle[i].time*Math::PI*0.2f;

            ts.x = 0.25f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTISPHERE6)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = (1.0f-progress)*m_particle[i].dim.x;
            m_particle[i].intensity = progress*0.5f;

            ts.x = 0.125f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIPLOUF0)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = progress;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.50f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIDROP)
        {
            if (progress >= 1.0f ||
                m_particle[i].pos.y < m_water->GetLevel())
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].zoom = 1.0f-progress;
            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.750f;
            ts.y = 0.500f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIWATER)
        {
            if (progress >= 1.0f ||
                m_particle[i].pos.y < m_water->GetLevel())
            {
                DeleteRank(i);
                continue;
            }

            m_particle[i].intensity = 1.0f-progress;

            ts.x = 0.125f;
            ts.y = 0.125f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if (m_particle[i].type == PARTIRAY1)  // tower ray ?
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            if (m_particle[i].testTime >= 0.2f)
            {
                m_particle[i].testTime = 0.0f;
                CObject* object = SearchObjectRay(m_particle[i].pos, m_particle[i].goal,
                                         m_particle[i].type, m_particle[i].objFather);
                if (object != nullptr)
                {
                    assert(object->Implements(ObjectInterfaceType::Damageable));
                    dynamic_cast<CDamageableObject&>(*object).DamageObject(DamageType::Tower, std::numeric_limits<float>::infinity(), m_particle[i].objFather);
                }
            }

            ts.x = 0.00f;
            ts.y = 0.00f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if (m_particle[i].type == PARTIRAY2 ||
            m_particle[i].type == PARTIRAY3)
        {
            if (progress >= 1.0f)
            {
                DeleteRank(i);
                continue;
            }

            ts.x = 0.00f;
            ts.y = 0.25f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        float dp = (1.0f/256.0f)/2.0f;
        m_particle[i].texSup.x = ts.x+dp;
        m_particle[i].texSup.y = ts.y+dp;
        m_particle[i].texInf.x = ti.x-dp;
        m_particle[i].texInf.y = ti.y-dp;
        m_particle[i].time     += rTime;
        m_particle[i].testTime += rTime;
    }
}

bool CParticle::TrackMove(int i, glm::vec3 pos, float progress)
{
    if (i < 0 || i >= MAXTRACK)  return true;
    if (! m_track[i].used) return true;

    if (progress < 1.0f)  // particle exists?
    {
        int h = m_track[i].head;

        glm::vec3 last = { 0, 0, 0 };

        if ( m_track[i].posUsed == 1 ||
             m_track[i].last+m_track[i].step <= progress )
        {
            m_track[i].last = progress;
            last = m_track[i].pos[h];
            h ++;
            if (h == MAXTRACKLEN)  h = 0;
            if (m_track[i].posUsed < MAXTRACKLEN)  m_track[i].posUsed++;
        }
        else
        {
            int hh = h-1;
            if (hh < 0)  hh = MAXTRACKLEN-1;
            last = m_track[i].pos[hh];
        }
        m_track[i].pos[h] = pos;
        m_track[i].len[h] = glm::distance(pos, last);

        m_track[i].head = h;

        m_track[i].intensity = 1.0f-progress;
    }
    else    // slow death of the track ?
    {
        m_track[i].intensity = 0.0f;
    }

    return (m_track[i].intensity <= 0.0f);
}

void CParticle::TrackDraw(int i, ParticleType type)
{
    // Calculates the total length memorized.
    float lTotal = 0.0f;
    int h = m_track[i].head;
    for (int counter = 0; counter < m_track[i].posUsed-1; counter++)
    {
        lTotal += m_track[i].len[h];
        h--;
        if (h < 0) h = MAXTRACKLEN-1;
    }

    glm::mat4 mat = glm::mat4(1.0f);
    m_renderer->SetModelMatrix(mat);

    glm::vec2 texInf{ 0, 0 }, texSup{ 0, 0 };

    if (type == PARTITRACK1)  // technical explosion?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 21.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 22.0f/256.0f;  // orange
    }
    if (type == PARTITRACK2)  // blue spray?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 13.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 14.0f/256.0f;  // blue
    }
    if (type == PARTITRACK3)  // spider?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y =  5.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y =  6.0f/256.0f;  // brown
    }
    if (type == PARTITRACK4)  // insect explosion?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y =  9.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 10.0f/256.0f;  // dark green
    }
    if (type == PARTITRACK5)  // derrick?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 29.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 30.0f/256.0f;  // dark brown
    }
    if (type == PARTITRACK6)  // reset in/out?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 17.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 18.0f/256.0f;  // cyan
    }
    if (type == PARTITRACK7)  // win-1?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 41.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 42.0f/256.0f;  // orange
    }
    if (type == PARTITRACK8)  // win-2?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 45.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 46.0f/256.0f;  // yellow
    }
    if (type == PARTITRACK9)  // win-3?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 49.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 50.0f/256.0f;  // red
    }
    if (type == PARTITRACK10)  // win-4?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 53.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 54.0f/256.0f;  // violet
    }
    if (type == PARTITRACK11)  // phazer shot?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 21.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 22.0f/256.0f;  // orange
    }
    if (type == PARTITRACK12)  // drag reactor?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 21.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 22.0f/256.0f;  // orange
    }

    h  = m_track[i].head;
    glm::vec3 p1 = m_track[i].pos[h];
    float f1 = m_track[i].intensity;

    glm::vec3 eye = m_engine->GetEyePt();
    float a = Math::RotateAngle(eye.x-p1.x, eye.z-p1.z);

    VertexParticle vertex[4];
    glm::vec3 corner[4];

    for (int counter = 0; counter < m_track[i].posUsed-1; counter++)
    {
        float f2 = f1-(m_track[i].len[h]/lTotal);
        if (f2 < 0.0f) f2 = 0.0f;

        h --;
        if (h < 0) h = MAXTRACKLEN-1;

        glm::vec3 p2 = m_track[i].pos[h];

        glm::vec2 rot;

        glm::vec3 p = p1;
        p.x += f1*m_track[i].width;
        rot = Math::RotatePoint({ p1.x, p1.z }, a + Math::PI / 2.0f, { p.x, p.z });
        corner[0].x = rot.x;
        corner[0].y = p1.y;
        corner[0].z = rot.y;
        rot = Math::RotatePoint({ p1.x, p1.z }, a - Math::PI / 2.0f, { p.x, p.z });
        corner[1].x = rot.x;
        corner[1].y = p1.y;
        corner[1].z = rot.y;

        p = p2;
        p.x += f2*m_track[i].width;
        rot = Math::RotatePoint({ p2.x, p2.z }, a + Math::PI / 2.0f, { p.x, p.z });
        corner[2].x = rot.x;
        corner[2].y = p2.y;
        corner[2].z = rot.y;
        rot = Math::RotatePoint({ p2.x, p2.z }, a - Math::PI / 2.0f, { p.x, p.z });
        corner[3].x = rot.x;
        corner[3].y = p2.y;
        corner[3].z = rot.y;

        glm::u8vec4 white(255);

        if (p2.y < p1.y)
        {
            vertex[0] = { corner[1], white, { texSup.x, texSup.y } };
            vertex[1] = { corner[0], white, { texInf.x, texSup.y } };
            vertex[2] = { corner[3], white, { texSup.x, texInf.y } };
            vertex[3] = { corner[2], white, { texInf.x, texInf.y } };
        }
        else
        {
            vertex[0] = { corner[0], white, { texSup.x, texSup.y } };
            vertex[1] = { corner[1], white, { texInf.x, texSup.y } };
            vertex[2] = { corner[2], white, { texSup.x, texInf.y } };
            vertex[3] = { corner[3], white, { texInf.x, texInf.y } };
        }

        m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
        m_engine->AddStatisticTriangle(2);

        if (f2 < 0.0f) break;
        f1 = f2;
        p1 = p2;
    }
}

void CParticle::DrawParticleTriangle(int i)
{
    if (m_particle[i].zoom == 0.0f)  return;

    glm::vec3 eye = m_engine->GetEyePt();
    glm::vec3 pos = m_particle[i].pos;

    CObject* object = m_particle[i].objLink;
    if (object != nullptr)
        pos += object->GetPosition();

    glm::vec3 angle;
    angle.x = -Math::RotateAngle(Math::DistanceProjected(pos, eye), pos.y-eye.y);
    angle.y = Math::RotateAngle(pos.z-eye.z, pos.x-eye.x);
    angle.z = m_particle[i].angle;

    glm::mat4 mat;
    Math::LoadRotationXZYMatrix(mat, angle);
    mat[3][0] = pos.x;
    mat[3][1] = pos.y;
    mat[3][2] = pos.z;
    m_renderer->SetModelMatrix(mat);

    VertexParticle vertices[3];

    for (size_t j = 0; j < 3; j++)
    {
        vertices[j].position = m_triangle[i].triangle[j].position;
        vertices[j].color = m_triangle[i].triangle[j].color;
        vertices[j].uv = m_triangle[i].triangle[j].uv;
    }

    m_renderer->DrawParticle(PrimitiveType::TRIANGLES, 3, vertices);
    m_engine->AddStatisticTriangle(1);
}

void CParticle::DrawParticleNorm(int i)
{
    float zoom = m_particle[i].zoom;

    if (zoom == 0.0f) return;
    if (m_particle[i].intensity == 0.0f) return;


    glm::vec3 corner[4];
    VertexParticle vertex[4];

    if (m_particle[i].sheet == SH_INTERFACE)
    {
        glm::vec3 pos = m_particle[i].pos;

        glm::vec3 n(0.0f, 0.0f, -1.0f);

        glm::vec2 dim;
        dim.x = m_particle[i].dim.x * zoom;
        dim.y = m_particle[i].dim.y * zoom;

        corner[0].x = pos.x+dim.x;
        corner[0].y = pos.y+dim.y;
        corner[0].z = 0.0f;

        corner[1].x = pos.x-dim.x;
        corner[1].y = pos.y+dim.y;
        corner[1].z = 0.0f;

        corner[2].x = pos.x+dim.x;
        corner[2].y = pos.y-dim.y;
        corner[2].z = 0.0f;

        corner[3].x = pos.x-dim.x;
        corner[3].y = pos.y-dim.y;
        corner[3].z = 0.0f;

        glm::u8vec4 white(255);

        vertex[0] = { corner[1], white, { m_particle[i].texSup.x, m_particle[i].texSup.y } };
        vertex[1] = { corner[0], white, { m_particle[i].texInf.x, m_particle[i].texSup.y } };
        vertex[2] = { corner[3], white, { m_particle[i].texSup.x, m_particle[i].texInf.y } };
        vertex[3] = { corner[2], white, { m_particle[i].texInf.x, m_particle[i].texInf.y } };

        m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
        m_engine->AddStatisticTriangle(2);
    }
    else
    {
        glm::vec3 eye = m_engine->GetEyePt();
        glm::vec3 pos = m_particle[i].pos;

        CObject* object = m_particle[i].objLink;
        if (object != nullptr)
            pos += object->GetPosition();

        glm::vec3 angle;
        angle.x = -Math::RotateAngle(Math::DistanceProjected(pos, eye), pos.y-eye.y);
        angle.y = Math::RotateAngle(pos.z-eye.z, pos.x-eye.x);
        angle.z = m_particle[i].angle;

        glm::mat4 mat;
        Math::LoadRotationXZYMatrix(mat, angle);
        mat[3][0] = pos.x;
        mat[3][1] = pos.y;
        mat[3][2] = pos.z;
        m_renderer->SetModelMatrix(mat);

        glm::vec3 n(0.0f, 0.0f, -1.0f);

        glm::vec2 dim;
        dim.x = m_particle[i].dim.x * zoom;
        dim.y = m_particle[i].dim.y * zoom;

        corner[0].x =  dim.x;
        corner[0].y =  dim.y;
        corner[0].z =  0.0f;

        corner[1].x = -dim.x;
        corner[1].y =  dim.y;
        corner[1].z =  0.0f;

        corner[2].x =  dim.x;
        corner[2].y = -dim.y;
        corner[2].z =  0.0f;

        corner[3].x = -dim.x;
        corner[3].y = -dim.y;
        corner[3].z =  0.0f;

        glm::u8vec4 color = ColorToIntColor(m_particle[i].color);

        vertex[0] = { corner[1], color, { m_particle[i].texSup.x, m_particle[i].texSup.y } };
        vertex[1] = { corner[0], color, { m_particle[i].texInf.x, m_particle[i].texSup.y } };
        vertex[2] = { corner[3], color, { m_particle[i].texSup.x, m_particle[i].texInf.y } };
        vertex[3] = { corner[2], color, { m_particle[i].texInf.x, m_particle[i].texInf.y } };

        m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
        m_engine->AddStatisticTriangle(2);
    }
}

void CParticle::DrawParticleFlat(int i)
{
    if (m_particle[i].zoom == 0.0f) return;
    if (m_particle[i].intensity == 0.0f) return;

    glm::vec3 pos = m_particle[i].pos;

    CObject* object = m_particle[i].objLink;
    if (object != nullptr)
        pos += object->GetPosition();

    glm::vec3 angle;
    angle.x = Math::PI/2.0f;
    angle.y = 0.0f;
    angle.z = m_particle[i].angle;

    if (m_engine->GetRankView() == 1)  // underwater?
        pos.y -= 1.0f;

    glm::vec3 eye = m_engine->GetEyePt();
    if (pos.y > eye.y)  // seen from below?
        angle.x = -Math::PI/2.0f;

    glm::mat4 mat;
    Math::LoadRotationXZYMatrix(mat, angle);
    mat[3][0] = pos.x;
    mat[3][1] = pos.y;
    mat[3][2] = pos.z;
    m_renderer->SetModelMatrix(mat);

    glm::vec3 n(0.0f, 0.0f, -1.0f);

    glm::vec2 dim;
    dim.x = m_particle[i].dim.x * m_particle[i].zoom;
    dim.y = m_particle[i].dim.y * m_particle[i].zoom;

    glm::vec3 corner[4];
    corner[0].x =  dim.x;
    corner[0].y =  dim.y;
    corner[0].z =  0.0f;

    corner[1].x = -dim.x;
    corner[1].y =  dim.y;
    corner[1].z =  0.0f;

    corner[2].x =  dim.x;
    corner[2].y = -dim.y;
    corner[2].z =  0.0f;

    corner[3].x = -dim.x;
    corner[3].y = -dim.y;
    corner[3].z =  0.0f;

    glm::u8vec4 white(255);

    VertexParticle vertex[4];
    vertex[0] = { corner[1], white, { m_particle[i].texSup.x, m_particle[i].texSup.y } };
    vertex[1] = { corner[0], white, { m_particle[i].texInf.x, m_particle[i].texSup.y } };
    vertex[2] = { corner[3], white, { m_particle[i].texSup.x, m_particle[i].texInf.y } };
    vertex[3] = { corner[2], white, { m_particle[i].texInf.x, m_particle[i].texInf.y } };

    m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
    m_engine->AddStatisticTriangle(2);
}

void CParticle::DrawParticleFog(int i)
{
    if (!m_engine->GetFog()) return;
    if (m_particle[i].intensity == 0.0f) return;

    glm::vec3 pos = m_particle[i].pos;
    glm::vec2 dim = m_particle[i].dim;

    if ( m_particle[i].type == PARTIFOG0 ||
         m_particle[i].type == PARTIFOG2 ||
         m_particle[i].type == PARTIFOG4 ||
         m_particle[i].type == PARTIFOG6 )
    {
        dim.x *= 1.0f+sinf(m_particle[i].zoom*2.0f)/6.0f;
        dim.y *= 1.0f+cosf(m_particle[i].zoom*2.7f)/6.0f;
    }
    if ( m_particle[i].type == PARTIFOG1 ||
         m_particle[i].type == PARTIFOG3 ||
         m_particle[i].type == PARTIFOG5 ||
         m_particle[i].type == PARTIFOG7 )
    {
        dim.x *= 1.0f+sinf(m_particle[i].zoom*3.0f)/6.0f;
        dim.y *= 1.0f+cosf(m_particle[i].zoom*3.7f)/6.0f;
    }


    CObject* object = m_particle[i].objLink;
    if (object != nullptr)
        pos += object->GetPosition();

    glm::vec3 angle;
    angle.x = Math::PI/2.0f;
    angle.y = 0.0f;
    angle.z = m_particle[i].angle;

    if (m_engine->GetRankView() == 1)  // underwater?
        pos.y -= 1.0f;

    glm::vec3 eye = m_engine->GetEyePt();
    if (pos.y > eye.y)  // seen from below?
        angle.x = -Math::PI/2.0f;

    glm::mat4 mat;
    Math::LoadRotationXZYMatrix(mat, angle);
    mat[3][0] = pos.x;
    mat[3][1] = pos.y;
    mat[3][2] = pos.z;
    m_renderer->SetModelMatrix(mat);

    glm::vec3 corner[4];

    corner[0].x =  dim.x;
    corner[0].y =  dim.y;
    corner[0].z =  0.0f;

    corner[1].x = -dim.x;
    corner[1].y =  dim.y;
    corner[1].z =  0.0f;

    corner[2].x =  dim.x;
    corner[2].y = -dim.y;
    corner[2].z =  0.0f;

    corner[3].x = -dim.x;
    corner[3].y = -dim.y;
    corner[3].z =  0.0f;

    glm::u8vec4 white(255);

    VertexParticle vertex[4];

    vertex[0] = { corner[1], white, { m_particle[i].texSup.x, m_particle[i].texSup.y } };
    vertex[1] = { corner[0], white, { m_particle[i].texInf.x, m_particle[i].texSup.y } };
    vertex[2] = { corner[3], white, { m_particle[i].texSup.x, m_particle[i].texInf.y } };
    vertex[3] = { corner[2], white, { m_particle[i].texInf.x, m_particle[i].texInf.y } };

    m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
    m_engine->AddStatisticTriangle(2);
}

void CParticle::DrawParticleRay(int i)
{
    if (m_particle[i].zoom == 0.0f)  return;
    if (m_particle[i].intensity == 0.0f)  return;

    glm::vec3 eye = m_engine->GetEyePt();
    glm::vec3 pos = m_particle[i].pos;
    glm::vec3 goal = m_particle[i].goal;

    CObject* object = m_particle[i].objLink;
    if (object != nullptr)
        pos += object->GetPosition();

    float a = Math::RotateAngle({ pos.x,pos.z }, { goal.x, goal.z }, { eye.x, eye.z });
    bool left = (a < Math::PI);

    glm::vec3 proj = Math::Projection(pos, goal, eye);
    glm::vec3 angle;
    angle.x = -Math::RotateAngle(Math::DistanceProjected(proj, eye), proj.y-eye.y);
    angle.y = Math::RotateAngle(pos.z-goal.z, pos.x-goal.x)+Math::PI/2.0f;
    angle.z = -Math::RotateAngle(Math::DistanceProjected(pos, goal), pos.y-goal.y);
    if (left)  angle.x = -angle.x;

    glm::mat4 mat;
    Math::LoadRotationZXYMatrix(mat, angle);
    mat[3][0] = pos.x;
    mat[3][1] = pos.y;
    mat[3][2] = pos.z;
    m_renderer->SetModelMatrix(mat);

    glm::vec3 n(0.0f, 0.0f, left ? 1.0f : -1.0f);

    glm::vec2 dim;
    dim.x = m_particle[i].dim.x * m_particle[i].zoom;
    dim.y = m_particle[i].dim.y * m_particle[i].zoom;

    if (left) dim.y = -dim.y;

    float len = glm::distance(pos, goal);
    float adv = 0.0f;

    int step = static_cast<int>((len/(dim.x*2.0f))+1);

    float vario1, vario2;

    if (step == 1)
    {
        vario1 = 1.0f;
        vario2 = 1.0f;
    }
    else
    {
        vario1 = 0.0f;
        vario2 = 2.0f;
    }

    int first, last;

    if (m_particle[i].type == PARTIRAY2)
    {
        first = 0;
        last  = step;
        vario1 = 0.0f;
        vario2 = 0.0f;
    }
    else if (m_particle[i].type == PARTIRAY3)
    {
        if (m_particle[i].time < m_particle[i].duration*0.40f)
        {
            float prop = m_particle[i].time / (m_particle[i].duration*0.40f);
            first = 0;
            last  = static_cast<int>(prop*step);
        }
        else if (m_particle[i].time < m_particle[i].duration*0.60f)
        {
            first = 0;
            last  = step;
        }
        else
        {
            float prop = (m_particle[i].time-m_particle[i].duration*0.60f) / (m_particle[i].duration*0.40f);
            first = static_cast<int>(prop*step);
            last  = step;
        }
    }
    else
    {
        if (m_particle[i].time < m_particle[i].duration*0.50f)
        {
            float prop = m_particle[i].time / (m_particle[i].duration*0.50f);
            first = 0;
            last  = static_cast<int>(prop*step);
        }
        else if (m_particle[i].time < m_particle[i].duration*0.75f)
        {
            first = 0;
            last  = step;
        }
        else
        {
            float prop = (m_particle[i].time-m_particle[i].duration*0.75f) / (m_particle[i].duration*0.25f);
            first = static_cast<int>(prop*step);
            last  = step;
        }
    }

    glm::vec3 corner[4];

    corner[0].x = adv;
    corner[2].x = adv;
    corner[0].y =  dim.y;
    corner[2].y = -dim.y;
    corner[0].z = (Math::Rand()-0.5f)*vario1;
    corner[1].z = (Math::Rand()-0.5f)*vario1;
    corner[2].z = (Math::Rand()-0.5f)*vario1;
    corner[3].z = (Math::Rand()-0.5f)*vario1;

    VertexParticle vertex[4];

    glm::u8vec4 white(255);

    for (int rank = 0; rank < step; rank++)
    {
        corner[1].x = corner[0].x;
        corner[3].x = corner[2].x;
        corner[0].x = adv+dim.x*2.0f+(Math::Rand()-0.5f)*vario2;
        corner[2].x = adv+dim.x*2.0f+(Math::Rand()-0.5f)*vario2;

        corner[1].y = corner[0].y;
        corner[3].y = corner[2].y;
        corner[0].y =  dim.y+(Math::Rand()-0.5f)*vario2;
        corner[2].y = -dim.y+(Math::Rand()-0.5f)*vario2;

        if (rank >= first && rank <= last)
        {
            glm::vec2 texInf = m_particle[i].texInf;
            glm::vec2 texSup = m_particle[i].texSup;

            int r = rand() % 16;
            texInf.x += 0.25f*(r/4);
            texSup.x += 0.25f*(r/4);
            if (r % 2 < 1 && adv > 0.0f && m_particle[i].type != PARTIRAY1)
                Math::Swap(texInf.x, texSup.x);

            if (r % 4 < 2)
                Math::Swap(texInf.y, texSup.y);

            vertex[0] = { corner[1], white, { texSup.x, texSup.y } };
            vertex[1] = { corner[0], white, { texInf.x, texSup.y } };
            vertex[2] = { corner[3], white, { texSup.x, texInf.y } };
            vertex[3] = { corner[2], white, { texInf.x, texInf.y } };

            m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
            m_engine->AddStatisticTriangle(2);
        }
        adv += dim.x*2.0f;
    }
}

void CParticle::DrawParticleSphere(int i)
{
    float zoom = m_particle[i].zoom;

    if (zoom == 0.0f) return;

    m_renderer->SetTransparency(TransparencyMode::BLACK);
    m_renderer->SetColor(IntensityToColor(m_particle[i].intensity));

    glm::mat4 mat = glm::mat4(1.0f);
    mat[0][0] = zoom;
    mat[1][1] = zoom;
    mat[2][2] = zoom;
    mat[3][0] = m_particle[i].pos.x;
    mat[3][1] = m_particle[i].pos.y;
    mat[3][2] = m_particle[i].pos.z;

    if (m_particle[i].angle != 0.0f)
    {
        glm::vec3 angle;
        angle.x = m_particle[i].angle*0.4f;
        angle.y = m_particle[i].angle*1.0f;
        angle.z = m_particle[i].angle*0.7f;
        glm::mat4 rot;
        Math::LoadRotationZXYMatrix(rot, angle);
        mat = mat * rot;
    }

    m_renderer->SetModelMatrix(mat);

    glm::vec2 ts, ti;
    ts.x = m_particle[i].texSup.x;
    ts.y = m_particle[i].texSup.y;
    ti.x = m_particle[i].texInf.x;
    ti.y = m_particle[i].texInf.y;

    int numRings, numSegments;

    // Choose a tesselation level.
    if ( m_particle[i].type == PARTISPHERE3 ||
         m_particle[i].type == PARTISPHERE5 )
    {
        numRings    = 16;
        numSegments = 16;
    }
    else
    {
        numRings    = 8;
        numSegments = 10;
    }

    // Establish constants used in sphere generation.
    float deltaRingAngle = Math::PI/numRings;
    float deltaSegAngle  = 2.0f*Math::PI/numSegments;

    std::vector<VertexParticle> vertex(2*16*(16+1));

    glm::u8vec4 white(255);

    // Generate the group of rings for the sphere.
    int j = 0;
    for (int ring = 0; ring < numRings; ring++)
    {
        float r0   = sinf((ring+0)*deltaRingAngle);
        float r1   = sinf((ring+1)*deltaRingAngle);

        glm::vec3 v0, v1;

        v0.y = cosf((ring+0)*deltaRingAngle);
        v1.y = cosf((ring+1)*deltaRingAngle);

        float tv0 = (ring+0)/static_cast<float>(numRings);
        float tv1 = (ring+1)/static_cast<float>(numRings);
        tv0 = ts.y+(ti.y-ts.y)*tv0;
        tv1 = ts.y+(ti.y-ts.y)*tv1;

        // Generate the group of segments for the current ring.
        for (int seg = 0; seg < numSegments+1; seg++)
        {
            v0.x = r0*sinf(seg*deltaSegAngle);
            v0.z = r0*cosf(seg*deltaSegAngle);
            v1.x = r1*sinf(seg*deltaSegAngle);
            v1.z = r1*cosf(seg*deltaSegAngle);

            // Add two vertices to the strip which makes up the sphere.
            float tu0 = (static_cast<float>(seg))/numSegments;
            tu0 = ts.x+(ti.x-ts.x)*tu0;
            float tu1 = tu0;

            vertex[j++] = { v0, white, { tu0, tv0 } };
            vertex[j++] = { v1, white, { tu1, tv1 } };
        }
    }

    m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, j, vertex.data());
    m_engine->AddStatisticTriangle(j);

    m_renderer->SetColor(IntensityToColor(m_particle[i].intensity));
    m_renderer->SetTransparency(TransparencyMode::BLACK);
}

//! Returns the height depending on the progress
static float ProgressCylinder(float progress)
{
    if (progress < 0.5f)
        return 1.0f - (powf(1.0f-progress*2.0f, 2.0f));
    else
        return 1.0f - (powf(progress*2.0f-1.0f, 2.0f));
}

void CParticle::DrawParticleCylinder(int i)
{
    float progress = m_particle[i].zoom;
    float zoom = m_particle[i].dim.x;
    float diam = m_particle[i].dim.y;
    if (progress >= 1.0f || zoom == 0.0f)  return;

    m_renderer->SetColor(IntensityToColor(m_particle[i].intensity));
    m_renderer->SetTransparency(TransparencyMode::BLACK);

    glm::mat4 mat = glm::mat4(1.0f);
    mat[0][0] = zoom;
    mat[1][1] = zoom;
    mat[2][2] = zoom;
    mat[3][0] = m_particle[i].pos.x;
    mat[3][1] = m_particle[i].pos.y;
    mat[3][2] = m_particle[i].pos.z;

    m_renderer->SetModelMatrix(mat);

    glm::vec2 ts, ti;
    ts.x = m_particle[i].texSup.x;
    ts.y = m_particle[i].texSup.y;
    ti.x = m_particle[i].texInf.x;
    ti.y = m_particle[i].texInf.y;

    int numRings = 5;
    int numSegments = 10;
    float deltaSegAngle  = 2.0f*Math::PI/numSegments;

    float h[6] = { 0.0f };
    float d[6] = { 0.0f };

    if (m_particle[i].type == PARTIPLOUF0)
    {
        float p1 = progress;  // front
        float p2 = powf(progress, 5.0f);  // back

        for (int ring = 0; ring <= numRings; ring++)
        {
            float pp = p2+(p1-p2)*(static_cast<float>(ring)/numRings);
            d[ring] = diam/zoom+pp*2.0f;
            h[ring] = ProgressCylinder(pp);
        }
    }

    std::vector<VertexParticle> vertex(2*5*(10+1));

    glm::u8vec4 white(255);

    int j = 0;
    for (int ring = 0; ring < numRings; ring++)
    {
        glm::vec3 v0, v1;

        float r0   = 1.0f*d[ring+0];  // radius at the base
        float r1   = 1.0f*d[ring+1];  // radius at the top

        v0.y = 1.0f*h[ring+0];  // bottom
        v1.y = 1.0f*h[ring+1];  // top

        float tv0 = 1.0f-(ring+0)*(1.0f/numRings);
        float tv1 = 1.0f-(ring+1)*(1.0f/numRings);
        tv0 = ts.y+(ti.y-ts.y)*tv0;
        tv1 = ts.y+(ti.y-ts.y)*tv1;

        for (int seg = 0; seg < numSegments+1; seg++)
        {
            v0.x = r0*sinf(seg*deltaSegAngle);
            v0.z = r0*cosf(seg*deltaSegAngle);
            v1.x = r1*sinf(seg*deltaSegAngle);
            v1.z = r1*cosf(seg*deltaSegAngle);

            float tu0 = (seg % 2) ? 0.0f : 1.0f;
            tu0 = ts.x+(ti.x-ts.x)*tu0;
            float tu1 = tu0;

            vertex[j++] = { v0, white, { tu0, tv0 } };
            vertex[j++] = { v1, white, { tu1, tv1 } };
        }
    }

    m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, j, vertex.data());
    m_engine->AddStatisticTriangle(j);

    m_renderer->SetColor(IntensityToColor(m_particle[i].intensity));
    m_renderer->SetTransparency(TransparencyMode::BLACK);
}

void CParticle::DrawParticleText(int i)
{
    StrUtils::CodePoint ch = std::string_view(&m_particle[i].text, 1);

    CharTexture tex = m_engine->GetText()->GetCharTexture(ch, FONT_STUDIO, FONT_SIZE_BIG*2.0f);
    if (tex.id == 0) return;

    m_renderer->SetTexture({ tex.id });
    m_renderer->SetColor(IntensityToColor(m_particle[i].intensity));
    m_renderer->SetTransparency(TransparencyMode::ALPHA);

    glm::ivec2 fontTextureSize = m_engine->GetText()->GetFontTextureSize();
    m_particle[i].texSup.x = static_cast<float>(tex.charPos.x) / fontTextureSize.x;
    m_particle[i].texSup.y = static_cast<float>(tex.charPos.y) / fontTextureSize.y;
    m_particle[i].texInf.x = static_cast<float>(tex.charPos.x + tex.charSize.x) / fontTextureSize.x;
    m_particle[i].texInf.y = static_cast<float>(tex.charPos.y + tex.charSize.y) / fontTextureSize.y;
    m_particle[i].color = Color(0.0f, 0.0f, 0.0f, 1.0f);

    DrawParticleNorm(i);
}

void CParticle::DrawParticleWheel(int i)
{
    float dist = Math::DistanceProjected(m_engine->GetEyePt(), m_wheelTrace[i].pos[0]);
    if (dist > 300.0f)  return;

    glm::u8vec4 white(255);

    if (m_wheelTrace[i].color == TraceColor::BlackArrow || m_wheelTrace[i].color == TraceColor::RedArrow)
    {
        auto texture = m_engine->LoadTexture("textures/effect03.png");
        m_renderer->SetTexture(texture);
        m_renderer->SetTransparency(TransparencyMode::ALPHA);
        m_renderer->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

        glm::vec3 pos[4];
        pos[0] = m_wheelTrace[i].pos[0];
        pos[1] = m_wheelTrace[i].pos[1];
        pos[2] = m_wheelTrace[i].pos[2];
        pos[3] = m_wheelTrace[i].pos[3];

        glm::vec3 n(0.0f, 1.0f, 0.0f);

        glm::vec2 ts(160.0f/256.0f, 224.0f/256.0f);
        glm::vec2 ti(ts.x+16.0f/256.0f, ts.y+16.0f/256.0f);

        float dp = (1.0f/256.0f)/2.0f;
        ts.x = ts.x+dp;
        ts.y = ts.y+dp;
        ti.x = ti.x-dp;
        ti.y = ti.y-dp;

        auto color = ColorToIntColor(TraceColorColor(m_wheelTrace[i].color));

        VertexParticle vertex[4];
        vertex[0] = { pos[0], color, { ts.x, ts.y } };
        vertex[1] = { pos[1], color, { ti.x, ts.y } };
        vertex[2] = { pos[2], color, { ts.x, ti.y } };
        vertex[3] = { pos[3], color, { ti.x, ti.y } };

        m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
        m_engine->AddStatisticTriangle(2);
    }
    else
    {
        glm::vec3 pos[4];
        pos[0] = m_wheelTrace[i].pos[0];
        pos[1] = m_wheelTrace[i].pos[1];
        pos[2] = m_wheelTrace[i].pos[2];
        pos[3] = m_wheelTrace[i].pos[3];

        auto color = ColorToIntColor(TraceColorColor(m_wheelTrace[i].color));

        VertexParticle vertex[4];
        vertex[0] = { pos[0], color };
        vertex[1] = { pos[1], color };
        vertex[2] = { pos[2], color };
        vertex[3] = { pos[3], color };

        m_renderer->DrawParticle(PrimitiveType::TRIANGLE_STRIP, 4, vertex);
        m_engine->AddStatisticTriangle(2);
    }
}

void CParticle::DrawParticle(int sheet)
{
    // Draw the basic particles of triangles.
    if (m_totalInterface[0][sheet] > 0)
    {
        for (int i = 0; i < MAXPARTICULE; i++)
        {
            if (!m_particle[i].used)  continue;
            if (m_particle[i].sheet != sheet)  continue;
            if (m_particle[i].type == PARTIPART)  continue;

            auto texture = m_engine->LoadTexture(!m_triangle[i].material.albedoTexture.empty()
                ? "textures" / m_triangle[i].material.albedoTexture
                : "");

            m_renderer->SetTexture(texture);
            //m_engine->SetState(m_triangle[i].state);
            DrawParticleTriangle(i);
        }
    }

    // Draw tire marks.
    if (m_wheelTraceTotal > 0 && sheet == SH_WORLD)
    {
        glm::mat4 matrix = glm::mat4(1.0f);
        m_renderer->SetModelMatrix(matrix);
        m_renderer->SetTransparency(TransparencyMode::NONE);
        m_renderer->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

        for (int i = 0; i < m_wheelTraceTotal; i++)
            DrawParticleWheel(i);
    }

    for (int t = MAXPARTITYPE-1; t >= 1; t--)  // black behind!
    {
        if (m_totalInterface[t][sheet] == 0)  continue;

        bool loadTexture = false;

        TransparencyMode mode = TransparencyMode::ALPHA;

        if (t == 4)
            mode = TransparencyMode::WHITE;
        else
            mode = TransparencyMode::BLACK;

        m_renderer->SetTransparency(mode);
        m_renderer->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

        for (int j = 0; j < MAXPARTICULE; j++)
        {
            int i = MAXPARTICULE*t+j;
            if (!m_particle[i].used)  continue;
            if (m_particle[i].sheet != sheet)  continue;

            if (!loadTexture && t != 5)
            {
                auto texture = m_engine->LoadTexture("textures" / NameParticle(t));
                m_renderer->SetTexture(texture);
                loadTexture = true;
            }

            int r = m_particle[i].trackRank;
            if (r != -1)
            {
                m_renderer->SetTransparency(mode);
                m_renderer->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
                //m_engine->SetState(state);
                TrackDraw(r, m_particle[i].type);  // draws the drag
                if (!m_track[r].drawParticle)  continue;
            }

            m_renderer->SetTransparency(mode);
            m_renderer->SetColor(IntensityToColor(m_particle[i].intensity));

            if (m_particle[i].ray)  // ray?
            {
                DrawParticleRay(i);
            }
            else if ( m_particle[i].type == PARTIFLIC  ||  // circle in the water?
                      m_particle[i].type == PARTISHOW  ||
                      m_particle[i].type == PARTICHOC  ||
                      m_particle[i].type == PARTIGFLAT )
            {
                DrawParticleFlat(i);
            }
            else if ( m_particle[i].type >= PARTIFOG0 &&
                      m_particle[i].type <= PARTIFOG7 )
            {
                DrawParticleFog(i);
            }
            else if ( m_particle[i].type >= PARTISPHERE0 &&
                      m_particle[i].type <= PARTISPHERE6 )  // sphere?
            {
                DrawParticleSphere(i);
            }
            else if ( m_particle[i].type == PARTIPLOUF0 )  // cylinder?
            {
                DrawParticleCylinder(i);
            }
            else if ( m_particle[i].type == PARTIVIRUS )
            {
                DrawParticleText(i);
            }
            else    // normal?
            {
                DrawParticleNorm(i);
            }
        }
    }
}

CObject* CParticle::SearchObjectGun(glm::vec3 old, glm::vec3 pos,
                                    ParticleType type, CObject *father)
{
    if (m_main->GetMovieLock()) return nullptr;  // current movie?

    float min = 5.0f;
    if (type == PARTIGUN2) min = 2.0f;  // shooting insect?
    if (type == PARTIGUN3) min = 3.0f;  // suiciding spider?

    glm::vec3 box1 = old;
    glm::vec3 box2 = pos;
    if (box1.x > box2.x)  Math::Swap(box1.x, box2.x);  // box1 < box2
    if (box1.y > box2.y)  Math::Swap(box1.y, box2.y);
    if (box1.z > box2.z)  Math::Swap(box1.z, box2.z);
    box1.x -= min;
    box1.y -= min;
    box1.z -= min;
    box2.x += min;
    box2.y += min;
    box2.z += min;

    CObject* best = nullptr;
    float best_dist = std::numeric_limits<float>::infinity();
    bool shield = false;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (!obj->GetDetectable()) continue;  // inactive?
        if (obj == father) continue;

        ObjectType oType = obj->GetType();

        if (oType == OBJECT_TOTO)  continue;

        if (type == PARTIGUN1)  // fireball shooting?
        {
            if (oType == OBJECT_MOTHER)  continue;
        }
        else if (type == PARTIGUN2)  // shooting insect?
        {
            if (IsAlien(obj->GetType()))  continue;
        }
        else if (type == PARTIGUN3)  // suiciding spider?
        {
            if (IsAlien(obj->GetType()))  continue;
        }
        else if (type == PARTIGUN4)  // orgaball shooting?
        {
            if (oType == OBJECT_MOTHER)  continue;
        }
        else if (type == PARTITRACK11)  // phazer shooting?
        {
        }
        else
        {
            continue;
        }
        if (!obj->Implements(ObjectInterfaceType::Damageable) && !obj->IsBulletWall())  continue;
        if (obj->Implements(ObjectInterfaceType::Jostleable))  continue;

        glm::vec3 oPos = obj->GetPosition();

        if (obj->GetType() == OBJECT_MOBILErs)
        {
            CShielder* shielder = dynamic_cast<CShielder*>(obj);
            if ( type == PARTIGUN2 ||  // shooting insect?
                 type == PARTIGUN3 )   // suiciding spider?
            {
                // Test if the ball is entered into the sphere of a shield.
                float shieldRadius = shielder->GetActiveShieldRadius();
                if (shieldRadius > 0.0f)
                {
                    float dist = glm::distance(oPos, pos);
                    if (dist <= shieldRadius)
                    {
                        best = obj;
                        shield = true;
                    }
                }
            }
        }
        if (shield)  continue;

        // Test the center of the object, which is necessary for objects
        // that have no sphere in the center (station).
        float dist = glm::distance(oPos, pos)-4.0f;
        float obj_dist = glm::distance(old, oPos);
        if (dist < min && obj_dist < best_dist)
        {
            best = obj;
            best_dist = obj_dist;
        }

        for (const auto& crashSphere : obj->GetAllCrashSpheres())
        {
            oPos = crashSphere.sphere.pos;
            float oRadius = crashSphere.sphere.radius;

            if ( oPos.x+oRadius < box1.x || oPos.x-oRadius > box2.x ||  // outside the box?
                 oPos.y+oRadius < box1.y || oPos.y-oRadius > box2.y ||
                 oPos.z+oRadius < box1.z || oPos.z-oRadius > box2.z )  continue;

            glm::vec3 p = Math::Projection(old, pos, oPos);
            float ddist = glm::distance(p, oPos)-oRadius;
            float obj_dist = glm::distance(old, oPos);
            if (ddist < min && obj_dist < best_dist)
            {
                best = obj;
                best_dist = obj_dist;
            }
        }
    }

    return best;
}

CObject* CParticle::SearchObjectRay(glm::vec3 pos, glm::vec3 goal,
                                    ParticleType type, CObject *father)
{
    if (m_main->GetMovieLock()) return nullptr;  // current movie?

    float min = 10.0f;

    glm::vec3 box1 = pos;
    glm::vec3 box2 = goal;
    if (box1.x > box2.x)  Math::Swap(box1.x, box2.x);  // box1 < box2
    if (box1.y > box2.y)  Math::Swap(box1.y, box2.y);
    if (box1.z > box2.z)  Math::Swap(box1.z, box2.z);
    box1.x -= min;
    box1.y -= min;
    box1.z -= min;
    box2.x += min;
    box2.y += min;
    box2.z += min;

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (!obj->GetDetectable()) continue;  // inactive?
        if (obj == father) continue;

        ObjectType oType = obj->GetType();

        if (oType == OBJECT_TOTO)  continue;

        if ( type  == PARTIRAY1       &&
             oType != OBJECT_MOBILEtg &&
             oType != OBJECT_TEEN28   &&
             oType != OBJECT_TEEN31   &&
             oType != OBJECT_ANT      &&
             oType != OBJECT_SPIDER   &&
             oType != OBJECT_BEE      &&
             oType != OBJECT_WORM     &&
             oType != OBJECT_MOTHER   &&
             oType != OBJECT_NEST     )  continue;

        glm::vec3 oPos = obj->GetPosition();

        if ( oPos.x < box1.x || oPos.x > box2.x ||  // outside the box?
             oPos.y < box1.y || oPos.y > box2.y ||
             oPos.z < box1.z || oPos.z > box2.z )  continue;

        glm::vec3 p = Math::Projection(pos, goal, oPos);
        float dist = glm::distance(p, oPos);
        if (dist < min)  return obj;
    }

    return nullptr;
}

void CParticle::Play(SoundType sound, glm::vec3 pos, float amplitude)
{
    if (m_sound == nullptr)
        m_sound = CApplication::GetInstancePointer()->GetSound();

    m_sound->Play(sound, pos, amplitude);
}

Color CParticle::GetFogColor(glm::vec3 pos)
{
    Color result;
    result.r = 0.0f;
    result.g = 0.0f;
    result.b = 0.0f;
    result.a = 0.0f;

    for (int fog = 0; fog < m_fogTotal; fog++)
    {
        int i = m_fog[fog];  // i = rank of the particle

        if (pos.y >= m_particle[i].pos.y+FOG_HSUP)  continue;
        if (pos.y <= m_particle[i].pos.y-FOG_HINF)  continue;

        float dist = Math::DistanceProjected(pos, m_particle[i].pos);
        if (dist >= m_particle[i].dim.x*1.5f)  continue;

        // Calculates the horizontal distance.
        float factor = 1.0f-powf(dist/(m_particle[i].dim.x*1.5f), 4.0f);

        // Calculates the vertical distance.
        if (pos.y > m_particle[i].pos.y)
            factor *= 1.0f-(pos.y-m_particle[i].pos.y)/FOG_HSUP;
        else
            factor *= 1.0f-(m_particle[i].pos.y-pos.y)/FOG_HINF;

        factor *= 0.3f;

        Color color;

        if ( m_particle[i].type == PARTIFOG0 ||
             m_particle[i].type == PARTIFOG1 )  // blue?
        {
            color.r = 0.0f;
            color.g = 0.5f;
            color.b = 1.0f;
        }
        else if ( m_particle[i].type == PARTIFOG2 ||
                  m_particle[i].type == PARTIFOG3 )  // red?
        {
            color.r = 2.0f;
            color.g = 1.0f;
            color.b = 0.0f;
        }
        else if ( m_particle[i].type == PARTIFOG4 ||
                  m_particle[i].type == PARTIFOG5 )  // white?
        {
            color.r = 1.0f;
            color.g = 1.0f;
            color.b = 1.0f;
        }
        else if ( m_particle[i].type == PARTIFOG6 ||
                  m_particle[i].type == PARTIFOG7 )  // yellow?
        {
            color.r = 0.8f;
            color.g = 1.0f;
            color.b = 0.4f;
        }
        else
        {
            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
        }

        result.r += color.r*factor;
        result.g += color.g*factor;
        result.b += color.b*factor;
    }

    if (result.r > 0.6f)  result.r = 0.6f;
    if (result.g > 0.6f)  result.g = 0.6f;
    if (result.b > 0.6f)  result.b = 0.6f;

    return result;
}

void CParticle::CutObjectLink(CObject* obj)
{
    for (int i = 0; i < MAXPARTICULE*MAXPARTITYPE; i++)
    {
        if (!m_particle[i].used) continue;

        if (m_particle[i].objLink == obj)
        {
            // If the object this particle's coordinates are linked to doesn't exist anymore, remove the particle
            DeleteRank(i);
        }

        if (m_particle[i].objFather == obj)
        {
            // If the object that spawned this partcle doesn't exist anymore, remove the link
            m_particle[i].objFather = nullptr;
        }
    }
}

} // namespace Gfx
