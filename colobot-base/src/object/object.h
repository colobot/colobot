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

/**
 * \file object/object.h
 * \brief CObject - base class for all game objects
 */

#pragma once

#include "object/crash_sphere.h"
#include "object/object_create_params.h"
#include "object/object_interface_type.h"
#include "object/old_object_interface.h"

#include <vector>
#include <optional>

namespace Gfx
{
struct ModelCrashSphere;
} // namespace Gfx

class CLevelParserLine;

namespace CBot
{
class CBotVar;
}

/**
 * \class CObject
 * \brief Base class for all 3D in-game objects
 *
 * CObject serves as a base class for all in-game objects, including:
 *  - buildings,
 *  - robots,
 *  - astronaut,
 *  - plants,
 *  - aliens.
 *
 * As every object has its specific behavior, there are or will be
 * separate subclasses for each of the specific objects. For the time being,
 * old object interface is still present, but its functions will be moved to
 * appropriate subclasses with time. The new CObject interface implemented
 * here will feature only functions common to all objects.
 */
class CObject : public COldObjectInterface
{
protected:
    //! Constructor only accessible to subclasses
    CObject(int id, ObjectType type);

public:
    CObject(const CObject&) = delete;
    CObject& operator=(const CObject&) = delete;

    virtual ~CObject();

    //! Returns object type
    inline ObjectType  GetType() const
    {
        return m_type;
    }
    //! Returns object's unique id
    inline int GetID() const
    {
        return m_id;
    }

    //! Writes object properties to line in level file
    virtual void Write(CLevelParserLine* line) = 0;
    //! Reads object properties from line in level file
    virtual void Read(CLevelParserLine* line) = 0;
    //! Reads params required for object creation
    static ObjectCreateParams ReadCreateParams(CLevelParserLine* line);

    //! Updates all interface controls
    virtual void UpdateInterface() {};

    //! Check if object implements the given type of interface
    inline bool Implements(ObjectInterfaceType type) const
    {
        return m_implementedInterfaces[static_cast<int>(type)];
    }

    //! Returns object's position
    virtual glm::vec3 GetPosition() const;
    //! Sets object's position
    virtual void SetPosition(const glm::vec3& pos);

    //! Returns object's rotation (Euler angles)
    /** Angles are given in radians */
    virtual glm::vec3 GetRotation() const;
    //! Sets object's rotation (Euler angles)
    /** Angles are given in radians */
    virtual void SetRotation(const glm::vec3& rotation);

    //!@{
    //! Shortcuts for rotation components
    void SetRotationX(float angle);
    void SetRotationY(float angle);
    void SetRotationZ(float angle);
    float GetRotationX();
    float GetRotationY();
    float GetRotationZ();
    //!@}

    //! Returns object's scale
    virtual glm::vec3 GetScale() const;
    //! Sets objects's scale
    virtual void SetScale(const glm::vec3& scale);
    //! Sets objects's scale (uniform value)
    void SetScale(float scale);

    //! Overrides objects's scale that is saved when saving the game
    void SetScaleOverride(std::optional<glm::vec3> scale);
    //! Return objects's scale that should be written to a save file
    glm::vec3 GetScaleForSave() const;

    //!@{
    //! Shortcuts for scale components
    void SetScaleX(float angle);
    void SetScaleY(float angle);
    void SetScaleZ(float angle);
    float GetScaleX();
    float GetScaleY();
    float GetScaleZ();
    //!@}

    //! Sets crash spheres for object
    void SetCrashSpheres(const std::vector<Gfx::ModelCrashSphere>& crashSpheres);
    //! Adds a new crash sphere
    /** Crash sphere position is given in object coordinates */
    void AddCrashSphere(const CrashSphere& crashSphere);
    //! Returns total number of crash spheres
    int GetCrashSphereCount();
    //! Returns the first crash sphere (assumes it exists)
    /** Crash sphere position is returned in world coordinates */
    CrashSphere GetFirstCrashSphere();
    //! Returns all crash spheres
    /** Crash sphere position is returned in world coordinates */
    std::vector<CrashSphere> GetAllCrashSpheres();
    //! Removes all crash spheres
    void DeleteAllCrashSpheres();
    //! Returns true if this object can collide with the other one
    bool CanCollideWith(CObject* other);

    //! Returns sphere used to test for camera collisions
    Math::Sphere GetCameraCollisionSphere();
    //! Sets sphere used to test for camera collisions
    // TODO: remove from here once no longer necessary
    void SetCameraCollisionSphere(const Math::Sphere& sphere);

    //! Sets object's ghost mode
    virtual void SetGhostMode(bool enabled) = 0;

    //! Sets flag controlling animation effect on level reset
    void SetAnimateOnReset(bool animateOnReset);
    //! Returns flag controlling animation effect on level reset
    bool GetAnimateOnReset();

    //! Turns object collisions on/off
    void SetCollisions(bool collisions);
    //! Returns true if collisions are enabled
    bool GetCollisions();

    //! Sets object team (shouldn't be called after creation because the model won't update!)
    void SetTeam(int team);
    //! Returns object team
    int GetTeam();

    //! Enable object activation only after you come close
    void SetProxyActivate(bool activate);
    //! Returns close activation mode
    bool GetProxyActivate();

    //! Sets distance for close activation
    void SetProxyDistance(float distance);
    //! Returns distance for close activation
    float GetProxyDistance();

    //! Returns CBot "object" variable associated with this object
    CBot::CBotVar* GetBotVar();

    //! Returns tooltip text for an object
    std::string GetTooltipText();

    //! Set "lock" mode of an object (for example, a robot while it's being factored, or a building while it's built)
    void SetLock(bool lock);
    //! Return "lock" mode of an object
    bool GetLock() const;
    //! Overrides the "lock" mode of an object that is saved when saving the game
    void SetLockOverride(std::optional<bool> lock);
    //! Return "lock" mode of an object that should be written to a save file
    bool GetLockForSave() const;

    //! Is this object active (not dead)?
    virtual bool GetActive() { return true; }
    //! Is this object detectable (not dead and not underground)?
    virtual bool GetDetectable() { return true; }

    //! Returns true if this object can collide with bullets even though it's not damageable itself
    //! This is useful to make Barriers protect from bullets
    //! \todo It will work like this for now but later I'd like to refactor this to something more manageable ~krzys_h
    virtual bool IsBulletWall() { return false; }

    //! Set whether the object is saved when creating a save file
    void SetPersistent(bool);
    bool GetPersistent() const;

protected:
    //! Transform crash sphere by object's world matrix
    virtual void TransformCrashSphere(Math::Sphere& crashSphere) = 0;
    //! Transform crash sphere by object's world matrix
    virtual void TransformCameraCollisionSphere(Math::Sphere& collisionSphere) = 0;

protected:
    const int m_id; //!< unique identifier
    ObjectType m_type; //!< object type
    ObjectInterfaceTypes m_implementedInterfaces; //!< interfaces that the object implements
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    std::optional<glm::vec3> m_scaleOverride;
    std::vector<CrashSphere> m_crashSpheres; //!< crash spheres
    Math::Sphere m_cameraCollisionSphere;
    bool m_animateOnReset;
    bool m_collisions;
    int m_team;
    bool m_proxyActivate;
    float m_proxyDistance;
    CBot::CBotVar* m_botVar;
    bool m_lock;
    std::optional<bool> m_lockOverride;
    bool m_persistent;
};
