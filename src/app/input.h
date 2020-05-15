/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file app/input.h
 * \brief CInput class
 */

#pragma once

#include "common/key.h"
#include "common/singleton.h"

#include "math/intpoint.h"
#include "math/point.h"
#include "math/vector.h"

#include <map>

struct Event;

/**
 * \struct InputBinding
 * \brief Binding for input slot
 */
struct InputBinding
{
    //! Primary and secondary bindings
    //! Can be regular key, virtual key or virtual joystick button
    unsigned int primary, secondary;

    InputBinding(unsigned int p = KEY_INVALID, unsigned int s = KEY_INVALID)
    : primary(p), secondary(s) {}
};

/**
 * \struct JoyAxisBinding
 * \brief Binding for joystick axis
 */
struct JoyAxisBinding
{
    //! Axis index or AXIS_INVALID
    int axis = 0;
    //! True to invert axis value
    bool invert = false;
};

//! Invalid value for axis binding (no axis assigned)
const int AXIS_INVALID = -1;

/**
 * \class CInput
 * \brief Management of mouse, keyboard and joystick
 */
class CInput : public CSingleton<CInput>
{
public:
    //! Constructor
    CInput();

    //! Process an incoming event, also sets .kmodState, .mousePos, .mouseButtonsState and .key.slot
    void EventProcess(Event &event);

    //! Called by CApplication on SDL MOUSE_MOTION event
    void MouseMove(Math::IntPoint pos);


    //! Returns whether the key is pressed
    bool        GetKeyState(InputSlot key) const;

    //! Returns whether the mouse button is pressed
    bool        GetMouseButtonState(int index) const;

    //! Resets tracked key states and modifiers
    void        ResetKeyStates();

    //! Returns the position of mouse cursor (in interface coords)
    Math::Point GetMousePos() const;


    //! Sets the default input bindings (keys and joystick axes)
    void        SetDefaultInputBindings();

    //! Management of input bindings
    //@{
    void        SetInputBinding(InputSlot slot, InputBinding binding);
    const InputBinding& GetInputBinding(InputSlot slot);
    //@}

    //! Management of joystick axis bindings
    //@{
    void        SetJoyAxisBinding(JoyAxisSlot slot, JoyAxisBinding binding);
    const JoyAxisBinding& GetJoyAxisBinding(JoyAxisSlot slot);
    //@}

    //! Management of joystick deadzone
    //@{
    void        SetJoystickDeadzone(float zone);
    float       GetJoystickDeadzone();
    //@}

    //! Get binding slot for given key
    InputSlot   FindBinding(unsigned int key);

    //! Saving/loading key bindings to colobot.ini
    //@{
    void        SaveKeyBindings();
    void        LoadKeyBindings();
    //@}

    //! Seeks a InputSlot by id. Returns INPUT_SLOT_MAX if not found
    InputSlot   SearchKeyById(std::string id);

    //! Returns string describing keys to be pressed
    //@{
    std::string GetKeysString(InputBinding binding);
    std::string GetKeysString(InputSlot slot);
    //@}

private:
    //! Current state of keys
    bool            m_keyPresses[INPUT_SLOT_MAX];


    //! Current position of mouse cursor
    Math::Point     m_mousePos;
    //! Current state of mouse buttons (bitmask of MouseButton enum values)
    unsigned int    m_mouseButtonsState;


    //! Motion vector set by keyboard or joystick buttons
    Math::Vector    m_keyMotion;
    //! Motion vector set by joystick axes
    Math::Vector    m_joyMotion;
    //! Camera motion vector set by joystick axes
    Math::Vector    m_joyMotionCam;

    //! Camera controls on the numpad
    Math::Vector    m_cameraKeyMotion;

    //! Bindings for user inputs
    InputBinding    m_inputBindings[INPUT_SLOT_MAX];
    JoyAxisBinding  m_joyAxisBindings[JOY_AXIS_SLOT_MAX];
    float           m_joystickDeadzone;

    std::map<InputSlot, std::string> m_keyTable;
};
