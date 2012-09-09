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

/**
 * \file common/event.h
 * \brief Event types, structs and event queue
 */

#pragma once


#include "common/key.h"
#include "common/event_ids.h"
#include "math/point.h"

class CInstanceManager;


/** \enum PressState
    \brief State of key/mouse button */
enum PressState
{
    STATE_PRESSED,
    STATE_RELEASED
};


/** \struct KeyEventData
    \brief Additional data for keyboard event */
struct KeyEventData
{
    //! STATE_PRESSED or STATE_RELEASED */
    PressState state;
    //! Key symbol: KEY(...) macro value (from common/key.h)
    unsigned int key;
    //! Keyboard modifiers: a bitmask made of KEY_MOD(...) macro values (from common/key.h)
    unsigned int mod;
    //! Unicode character
    unsigned int unicode;

    KeyEventData()
        : state(STATE_PRESSED), key(0), mod(0), unicode(0) {}
};

/** \struct MouseMotionEventData
    \brief Additional data for mouse move event */
struct MouseMoveEventData
{
    //! Current button state
    unsigned char state;
    //! Position of mouse in normalized coordinates (0..1)
    Math::Point pos;

    MouseMoveEventData()
        : state(STATE_PRESSED) {}
};

/** \struct MouseButtonEventData
    \brief Additional data mouse button event */
struct MouseButtonEventData
{
    //! The mouse button index
    unsigned char button;
    //! STATE_PRESSED or STATE_RELEASED
    PressState state;
    //! Position of mouse in normalized coordinates (0..1)
    Math::Point pos;

    MouseButtonEventData()
        : button(0), state(STATE_PRESSED) {}
};

/** \struct JoyAxisEventData
    \brief Additional data for joystick axis event */
struct JoyAxisEventData
{
    //! The joystick axis index
    unsigned char axis;
    //! The axis value (range: -32768 to 32767)
    int value;

    JoyAxisEventData()
        : axis(axis), value(value) {}
};

/** \struct JoyButtonEventData
    \brief Additional data for joystick button event */
struct JoyButtonEventData
{
    //! The joystick button index
    unsigned char button;
    //! STATE_PRESSED or STATE_RELEASED
    PressState state;

    JoyButtonEventData()
        : button(0), state(STATE_PRESSED) {}
};

/** \enum ActiveEventFlags
    \brief Type of focus gained/lost */
enum ActiveEventFlags
{
    //! Application window focus
    ACTIVE_APP   = 0x01,
    //! Input focus
    ACTIVE_INPUT = 0x02,
    //! Mouse focus
    ACTIVE_MOUSE = 0x04

};

/** \struct ActiveEventData
    \brief Additional data for active event */
struct ActiveEventData
{
    //! Flags (bitmask of enum values ActiveEventFlags)
    unsigned char flags;
    //! True if the focus was gained; false otherwise
    bool gain;

    ActiveEventData()
        : flags(0), gain(false) {}
};


/**
  \struct Event
  \brief Event sent by system, interface or game

  Event is described by its type (EventType) and the union
  \a data contains additional data about the event.
  Different members of the union are filled with different event types.
  With some events, nothing is filled (it's zeroed out).
  The union contains roughly the same information as SDL_Event struct
  but packaged to independent structs and fields.
 **/
struct Event
{
    //! Type of event (EVENT_*)
    EventType type;

    //! If true, the event was produced by system (SDL); else, it has come from user interface
    bool systemEvent;

    union
    {
        //! Additional data for EVENT_KEY_DOWN and EVENT_KEY_UP
        KeyEventData key;
        //! Additional data for EVENT_MOUSE_BUTTON_DOWN and EVENT_MOUSE_BUTTON_UP
        MouseButtonEventData mouseButton;
        //! Additional data for EVENT_MOUSE_MOVE
        MouseMoveEventData mouseMove;
        //! Additional data for EVENT_JOY
        JoyAxisEventData joyAxis;
        //! Additional data for EVENT_JOY_AXIS
        JoyButtonEventData joyButton;
        //! Additional data for EVENT_ACTIVE
        ActiveEventData active;
    };

    // TODO: refactor/rewrite
    long         param;      // parameter
    Math::Point  pos;        // mouse position (0 .. 1)
    float        axeX;       // control the X axis (-1 .. 1)
    float        axeY;       // control of the Y axis (-1 .. 1)
    float        axeZ;       // control the Z axis (-1 .. 1)
    short        keyState;   // state of the keyboard (KS_ *)
    float        rTime;      // relative time

    Event(EventType aType = EVENT_NULL)
    {
        type = aType;
        systemEvent = false;

        param = 0;
        axeX = axeY = axeZ = 0.0f;
        keyState = 0;
        rTime = 0.0f;
    }
};


//! Returns an unique event type (above the standard IDs)
EventType GetUniqueEventType();


/**
  \class CEventQueue
  \brief Global event queue

  Provides an interface to a global FIFO queue with events (both system- and user-generated).
  The queue has a fixed maximum size but it should not be a problem.
 */
class CEventQueue
{
public:
    //! Constant maximum size of queue
    static const int MAX_EVENT_QUEUE = 100;

public:
    //! Object's constructor
    CEventQueue(CInstanceManager* iMan);
    //! Object's destructor
    ~CEventQueue();

    //! Empties the FIFO of events
    void    Flush();
    //! Adds an event to the queue
    bool    AddEvent(const Event &event);
    bool    GetEvent(Event &event);

protected:
    CInstanceManager* m_iMan;
    Event        m_fifo[MAX_EVENT_QUEUE];
    int          m_head;
    int          m_tail;
    int          m_total;
};
