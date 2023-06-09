/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "app/input.h"

#include "core/stringutils.h"

#include "common/config_file.h"
#include "common/logger.h"
#include "common/restext.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"


#include <sstream>
#include <SDL_system.h>

CInput::CInput()
    : m_keyPresses()
{
    m_keyTable = std::map<InputSlot, std::string>
    {
        { INPUT_SLOT_LEFT,     "left"    },
        { INPUT_SLOT_RIGHT,    "right"   },
        { INPUT_SLOT_UP,       "up"      },
        { INPUT_SLOT_DOWN,     "down"    },
        { INPUT_SLOT_GUP,      "gup"     },
        { INPUT_SLOT_GDOWN,    "gdown"   },
        { INPUT_SLOT_CAMERA,   "camera"  },
        { INPUT_SLOT_DESEL,    "desel"   },
        { INPUT_SLOT_ACTION,   "action"  },
        { INPUT_SLOT_CAM_LEFT, "cleft"   },
        { INPUT_SLOT_CAM_RIGHT,"cright"  },
        { INPUT_SLOT_CAM_UP,   "cup"     },
        { INPUT_SLOT_CAM_DOWN, "cdown"   },
        { INPUT_SLOT_CAM_NEAR, "near"    },
        { INPUT_SLOT_CAM_AWAY, "away"    },
        { INPUT_SLOT_CAM_ALT,  "camalt"  },
        { INPUT_SLOT_NEXT,     "next"    },
        { INPUT_SLOT_HUMAN,    "human"   },
        { INPUT_SLOT_QUIT,     "quit"    },
        { INPUT_SLOT_HELP,     "help"    },
        { INPUT_SLOT_PROG,     "prog"    },
        { INPUT_SLOT_VISIT,    "visit"   },
        { INPUT_SLOT_SPEED_DEC,  "speed_dec" },
        { INPUT_SLOT_SPEED_RESET,"speed_reset" },
        { INPUT_SLOT_SPEED_INC,  "speed_inc" },
        { INPUT_SLOT_QUICKSAVE,  "quicksave" },
        { INPUT_SLOT_QUICKLOAD,  "quickload" },
        { INPUT_SLOT_PAUSE,    "pause"   },
        { INPUT_SLOT_CMDLINE,  "cmdline" },
    };

    m_mousePos = Math::Point();
    m_mouseButtonsState = 0;
    std::fill_n(m_keyPresses, static_cast<std::size_t>(INPUT_SLOT_MAX), false);

    m_joystickDeadzone = 0.2f;
    SetDefaultInputBindings();
}

void CInput::EventProcess(Event& event)
{
    // Use the occasion to update mouse button state
    if (event.type == EVENT_MOUSE_BUTTON_DOWN)
    {
        auto data = event.GetData<MouseButtonEventData>();
        m_mouseButtonsState |= data->button;
    }
    if (event.type == EVENT_MOUSE_BUTTON_UP)
    {
        auto data = event.GetData<MouseButtonEventData>();
        m_mouseButtonsState &= ~data->button;
    }


    if (event.type == EVENT_KEY_DOWN ||
        event.type == EVENT_KEY_UP)
    {
        auto data = event.GetData<KeyEventData>();
        data->slot = FindBinding(data->key);
    }

    event.kmodState = SDL_GetModState();
    event.mousePos = m_mousePos;
    event.mouseButtonsState = m_mouseButtonsState;


    if (event.type == EVENT_KEY_DOWN ||
        event.type == EVENT_KEY_UP)
    {
        auto data = event.GetData<KeyEventData>();
        m_keyPresses[data->slot] = (event.type == EVENT_KEY_DOWN);
    }



    /* Motion vector management */

    if (event.type == EVENT_KEY_DOWN && !(event.kmodState & KEY_MOD(ALT) ) )
    {
        auto data = event.GetData<KeyEventData>();

        if (data->slot == INPUT_SLOT_UP   ) m_keyMotion.y =  1.0f;
        if (data->slot == INPUT_SLOT_DOWN ) m_keyMotion.y = -1.0f;
        if (data->slot == INPUT_SLOT_LEFT ) m_keyMotion.x = -1.0f;
        if (data->slot == INPUT_SLOT_RIGHT) m_keyMotion.x =  1.0f;
        if (data->slot == INPUT_SLOT_GUP  ) m_keyMotion.z =  1.0f;
        if (data->slot == INPUT_SLOT_GDOWN) m_keyMotion.z = -1.0f;

        if (data->slot == INPUT_SLOT_CAM_LEFT ) m_cameraKeyMotion.x = -1.0f;
        if (data->slot == INPUT_SLOT_CAM_RIGHT) m_cameraKeyMotion.x =  1.0f;
        if (data->slot == INPUT_SLOT_CAM_UP   ) m_cameraKeyMotion.y =  1.0f;
        if (data->slot == INPUT_SLOT_CAM_DOWN ) m_cameraKeyMotion.y = -1.0f;
        if (data->slot == INPUT_SLOT_CAM_NEAR ) m_cameraKeyMotion.z = -1.0f;
        if (data->slot == INPUT_SLOT_CAM_AWAY ) m_cameraKeyMotion.z =  1.0f;
    }
    else if (event.type == EVENT_KEY_UP)
    {
        auto data = event.GetData<KeyEventData>();

        if (data->slot == INPUT_SLOT_UP   ) m_keyMotion.y = 0.0f;
        if (data->slot == INPUT_SLOT_DOWN ) m_keyMotion.y = 0.0f;
        if (data->slot == INPUT_SLOT_LEFT ) m_keyMotion.x = 0.0f;
        if (data->slot == INPUT_SLOT_RIGHT) m_keyMotion.x = 0.0f;
        if (data->slot == INPUT_SLOT_GUP  ) m_keyMotion.z = 0.0f;
        if (data->slot == INPUT_SLOT_GDOWN) m_keyMotion.z = 0.0f;

        if (data->slot == INPUT_SLOT_CAM_LEFT ) m_cameraKeyMotion.x = 0.0f;
        if (data->slot == INPUT_SLOT_CAM_RIGHT) m_cameraKeyMotion.x = 0.0f;
        if (data->slot == INPUT_SLOT_CAM_UP   ) m_cameraKeyMotion.y = 0.0f;
        if (data->slot == INPUT_SLOT_CAM_DOWN ) m_cameraKeyMotion.y = 0.0f;
        if (data->slot == INPUT_SLOT_CAM_NEAR ) m_cameraKeyMotion.z = 0.0f;
        if (data->slot == INPUT_SLOT_CAM_AWAY ) m_cameraKeyMotion.z = 0.0f;
    }
    else if (event.type == EVENT_JOY_AXIS)
    {
        auto data = event.GetData<JoyAxisEventData>();

        if (data->axis == GetJoyAxisBinding(JOY_AXIS_SLOT_X).axis)
        {
            m_joyMotion.x = Math::Neutral(data->value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_X).invert)
                m_joyMotion.x *= -1.0f;
        }

        if (data->axis == GetJoyAxisBinding(JOY_AXIS_SLOT_Y).axis)
        {
            m_joyMotion.y = -Math::Neutral(data->value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_Y).invert)
                m_joyMotion.y *= -1.0f;
        }

        if (data->axis == GetJoyAxisBinding(JOY_AXIS_SLOT_Z).axis)
        {
            m_joyMotion.z = -Math::Neutral(data->value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_Z).invert)
                m_joyMotion.z *= -1.0f;
        }

        if (data->axis == GetJoyAxisBinding(JOY_AXIS_SLOT_CAM_X).axis)
        {
            m_joyMotionCam.x = -Math::Neutral(data->value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_CAM_X).invert)
                m_joyMotionCam.x *= -1.0f;
        }

        if (data->axis == GetJoyAxisBinding(JOY_AXIS_SLOT_CAM_Y).axis)
        {
            m_joyMotionCam.y = -Math::Neutral(data->value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_CAM_Y).invert)
                m_joyMotionCam.y *= -1.0f;
        }

        if (data->axis == GetJoyAxisBinding(JOY_AXIS_SLOT_CAM_Z).axis)
        {
            m_joyMotionCam.z = -Math::Neutral(data->value / 32768.0f, m_joystickDeadzone);
            if (GetJoyAxisBinding(JOY_AXIS_SLOT_CAM_Z).invert)
                m_joyMotionCam.z *= -1.0f;
        }
    }

    event.motionInput = Math::Clamp(m_joyMotion + m_keyMotion, Math::Vector(-1.0f, -1.0f, -1.0f), Math::Vector(1.0f, 1.0f, 1.0f));
    event.cameraInput = Math::Clamp(m_joyMotionCam + m_cameraKeyMotion, Math::Vector(-1.0f, -1.0f, -1.0f), Math::Vector(1.0f, 1.0f, 1.0f));
}

void CInput::MouseMove(Math::IntPoint pos)
{
    m_mousePos = Gfx::CEngine::GetInstancePointer()->WindowToInterfaceCoords(pos);
}

bool CInput::GetKeyState(InputSlot key) const
{
    return m_keyPresses[key];
}

bool CInput::GetMouseButtonState(int index) const
{
    return (m_mouseButtonsState & (1<<index)) != 0;
}

void CInput::ResetKeyStates()
{
    GetLogger()->Trace("Reset key states\n");
    m_keyMotion = Math::Vector(0.0f, 0.0f, 0.0f);
    m_joyMotion = Math::Vector(0.0f, 0.0f, 0.0f);
    m_cameraKeyMotion = Math::Vector(0.0f, 0.0f, 0.0f);
    m_joyMotionCam = Math::Vector(0.0f, 0.0f, 0.0f);
    for(int i=0; i<INPUT_SLOT_MAX; i++)
        m_keyPresses[i] = false;
}

Math::Point CInput::GetMousePos() const
{
    return m_mousePos;
}

void CInput::SetDefaultInputBindings()
{
    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        m_inputBindings[i].primary = m_inputBindings[i].secondary = KEY_INVALID;
    }

    for (int i = 0; i < JOY_AXIS_SLOT_MAX; i++)
    {
        m_joyAxisBindings[i].axis = AXIS_INVALID;
        m_joyAxisBindings[i].invert = false;
    }

    m_inputBindings[INPUT_SLOT_LEFT   ].primary   = KEY(LEFT);
    m_inputBindings[INPUT_SLOT_RIGHT  ].primary   = KEY(RIGHT);
    m_inputBindings[INPUT_SLOT_UP     ].primary   = KEY(UP);
    m_inputBindings[INPUT_SLOT_DOWN   ].primary   = KEY(DOWN);
    m_inputBindings[INPUT_SLOT_LEFT   ].secondary = KEY(a);
    m_inputBindings[INPUT_SLOT_RIGHT  ].secondary = KEY(d);
    m_inputBindings[INPUT_SLOT_UP     ].secondary = KEY(w);
    m_inputBindings[INPUT_SLOT_DOWN   ].secondary = KEY(s);
    m_inputBindings[INPUT_SLOT_GUP    ].primary   = VIRTUAL_KMOD(SHIFT);
    m_inputBindings[INPUT_SLOT_GDOWN  ].primary   = VIRTUAL_KMOD(CTRL);
    m_inputBindings[INPUT_SLOT_CAMERA ].primary   = KEY(SPACE);
    m_inputBindings[INPUT_SLOT_DESEL  ].primary   = KEY(KP_0);
    m_inputBindings[INPUT_SLOT_ACTION ].primary   = KEY(RETURN);
    m_inputBindings[INPUT_SLOT_ACTION ].secondary = KEY(e);
    m_inputBindings[INPUT_SLOT_CAM_LEFT ].primary = KEY(KP_4);
    m_inputBindings[INPUT_SLOT_CAM_RIGHT].primary = KEY(KP_6);
    m_inputBindings[INPUT_SLOT_CAM_UP   ].primary = KEY(KP_8);
    m_inputBindings[INPUT_SLOT_CAM_DOWN ].primary = KEY(KP_2);
    m_inputBindings[INPUT_SLOT_CAM_NEAR ].primary = KEY(KP_PLUS);
    m_inputBindings[INPUT_SLOT_CAM_AWAY ].primary = KEY(KP_MINUS);
    m_inputBindings[INPUT_SLOT_CAM_ALT  ].primary = VIRTUAL_KMOD(ALT);
    m_inputBindings[INPUT_SLOT_NEXT   ].primary   = KEY(TAB);
    m_inputBindings[INPUT_SLOT_HUMAN  ].primary   = KEY(HOME);
    m_inputBindings[INPUT_SLOT_QUIT   ].primary   = KEY(ESCAPE);
    m_inputBindings[INPUT_SLOT_HELP   ].primary   = KEY(F1);
    m_inputBindings[INPUT_SLOT_PROG   ].primary   = KEY(F2);
    m_inputBindings[INPUT_SLOT_VISIT  ].primary   = KEY(KP_PERIOD);
    m_inputBindings[INPUT_SLOT_QUICKSAVE].primary   = KEY(F5);
    m_inputBindings[INPUT_SLOT_SPEED_DEC].primary   = KEY(F6);
    m_inputBindings[INPUT_SLOT_SPEED_RESET].primary   = KEY(F7);
    m_inputBindings[INPUT_SLOT_SPEED_INC].primary   = KEY(F8);
    m_inputBindings[INPUT_SLOT_QUICKLOAD].primary   = KEY(F9);
    m_inputBindings[INPUT_SLOT_PAUSE].primary       = KEY(PAUSE);
    m_inputBindings[INPUT_SLOT_PAUSE].secondary     = KEY(p);
    m_inputBindings[INPUT_SLOT_CMDLINE].primary     = KEY(BACKQUOTE);

    m_joyAxisBindings[JOY_AXIS_SLOT_X].axis = 0;
    m_joyAxisBindings[JOY_AXIS_SLOT_Y].axis = 1;
    m_joyAxisBindings[JOY_AXIS_SLOT_Z].axis = 2;
    m_joyAxisBindings[JOY_AXIS_SLOT_CAM_X].axis = -1;
    m_joyAxisBindings[JOY_AXIS_SLOT_CAM_Y].axis = -1;
    m_joyAxisBindings[JOY_AXIS_SLOT_CAM_Z].axis = -1;
}

void CInput::SetInputBinding(InputSlot slot, InputBinding binding)
{
    unsigned int index = static_cast<unsigned int>(slot);
    m_inputBindings[index] = binding;
}

const InputBinding& CInput::GetInputBinding(InputSlot slot)
{
    unsigned int index = static_cast<unsigned int>(slot);
    return m_inputBindings[index];
}

void CInput::SetJoyAxisBinding(JoyAxisSlot slot, JoyAxisBinding binding)
{
    unsigned int index = static_cast<unsigned int>(slot);
    m_joyAxisBindings[index] = binding;
}

const JoyAxisBinding& CInput::GetJoyAxisBinding(JoyAxisSlot slot)
{
    unsigned int index = static_cast<unsigned int>(slot);
    return m_joyAxisBindings[index];
}

void CInput::SetJoystickDeadzone(float zone)
{
    m_joystickDeadzone = zone;
}

float CInput::GetJoystickDeadzone()
{
    return m_joystickDeadzone;
}

InputSlot CInput::FindBinding(unsigned int key)
{
    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        InputSlot slot = static_cast<InputSlot>(i);
        InputBinding b = GetInputBinding(slot);
        if(b.primary == key || b.secondary == key)
            return slot;
    }
    return INPUT_SLOT_MAX;
}

void CInput::SaveKeyBindings()
{
    std::stringstream key;
    GetConfigFile().SetStringProperty("Keybindings", "_Version", "SDL2");
    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        InputBinding b = GetInputBinding(static_cast<InputSlot>(i));

        key.clear();
        key.str("");
        key << b.primary << " " << b.secondary;

        GetConfigFile().SetStringProperty("Keybindings", m_keyTable[static_cast<InputSlot>(i)], key.str());
    }

    for (int i = 0; i < JOY_AXIS_SLOT_MAX; i++)
    {
        JoyAxisBinding b = GetJoyAxisBinding(static_cast<JoyAxisSlot>(i));

        GetConfigFile().SetIntProperty("Setup", "JoystickAxisBinding"+StrUtils::ToString(i), b.axis);
        GetConfigFile().SetIntProperty("Setup", "JoystickAxisInvert"+StrUtils::ToString(i), b.invert);
    }
    GetConfigFile().SetFloatProperty("Setup", "JoystickDeadzone", GetJoystickDeadzone());
}

void CInput::LoadKeyBindings()
{
    std::stringstream skey;
    std::string keys;
    if (GetConfigFile().GetStringProperty("Keybindings", "_Version", keys) && keys == "SDL2") // Keybindings from SDL1.2 are incompatible with SDL2 !!
    {
        for (int i = 0; i < INPUT_SLOT_MAX; i++)
        {
            InputBinding b;

            if (!GetConfigFile().GetStringProperty("Keybindings", m_keyTable[static_cast<InputSlot>(i)], keys))
                continue;
            skey.clear();
            skey.str(keys);

            skey >> b.primary;
            skey >> b.secondary;

            SetInputBinding(static_cast<InputSlot>(i), b);
        }
    }

    for (int i = 0; i < JOY_AXIS_SLOT_MAX; i++)
    {
        JoyAxisBinding b;

        if (!GetConfigFile().GetIntProperty("Setup", "JoystickAxisBinding"+StrUtils::ToString(i), b.axis))
            continue;

        int x = 0;
        GetConfigFile().GetIntProperty("Setup", "JoystickAxisInvert"+StrUtils::ToString(i), x); // If doesn't exist, use default (0)
        b.invert = (x != 0);

        SetJoyAxisBinding(static_cast<JoyAxisSlot>(i), b);
    }
    float deadzone;
    if (GetConfigFile().GetFloatProperty("Setup", "JoystickDeadzone", deadzone))
        SetJoystickDeadzone(deadzone);
}

InputSlot CInput::SearchKeyById(std::string id)
{
    for(auto& key : m_keyTable)
    {
        if ( id == key.second )
        {
            return key.first;
        }
    }
    return INPUT_SLOT_MAX;
}

std::string CInput::GetKeysString(InputBinding binding)
{
    std::ostringstream ss;
    if ( binding.primary != KEY_INVALID )
    {
        std::string iNameStr;
        if ( GetResource(RES_KEY, binding.primary, iNameStr) )
        {
            ss << iNameStr;

            if ( binding.secondary != KEY_INVALID )
            {
                if ( GetResource(RES_KEY, binding.secondary, iNameStr) )
                {
                    std::string textStr;
                    GetResource(RES_TEXT, RT_KEY_OR, textStr);

                    ss << textStr << iNameStr;
                }
            }
        }
    }
    else
    {
        return "?";
    }
    return ss.str();
}


std::string CInput::GetKeysString(InputSlot slot)
{
    InputBinding b = GetInputBinding(slot);
    return GetKeysString(b);
}
