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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// restext.cpp

#include "common/restext.h"

#include "common/global.h"
#include "common/event.h"
#include "common/logger.h"
#include "CBot/resource.h"
#include "object/object.h"
#include "object/robotmain.h"

#include <libintl.h>
#include <SDL/SDL_keyboard.h>


static char         g_gamerName[100];

void SetGlobalGamerName(char *name)
{
    strcpy(g_gamerName, name);
}



struct KeyDesc
{
    InputSlot   key;
    char        name[20];
};

static KeyDesc keyTable[22] =
{
    { INPUT_SLOT_LEFT,     "left;"    },
    { INPUT_SLOT_RIGHT,    "right;"   },
    { INPUT_SLOT_UP,       "up;"      },
    { INPUT_SLOT_DOWN,     "down;"    },
    { INPUT_SLOT_GUP,      "gup;"     },
    { INPUT_SLOT_GDOWN,    "gdown;"   },
    { INPUT_SLOT_CAMERA,   "camera;"  },
    { INPUT_SLOT_DESEL,    "desel;"   },
    { INPUT_SLOT_ACTION,   "action;"  },
    { INPUT_SLOT_NEAR,     "near;"    },
    { INPUT_SLOT_AWAY,     "away;"    },
    { INPUT_SLOT_NEXT,     "next;"    },
    { INPUT_SLOT_HUMAN,    "human;"   },
    { INPUT_SLOT_QUIT,     "quit;"    },
    { INPUT_SLOT_HELP,     "help;"    },
    { INPUT_SLOT_PROG,     "prog;"    },
    { INPUT_SLOT_CBOT,     "cbot;"    },
    { INPUT_SLOT_VISIT,    "visit;"   },
    { INPUT_SLOT_SPEED10,  "speed10;" },
    { INPUT_SLOT_SPEED15,  "speed15;" },
    { INPUT_SLOT_SPEED20,  "speed20;" }
};

// Seeks a key.

bool SearchKey(const char *cmd, InputSlot &key)
{
    int     i;

    for ( i=0 ; i<22 ; i++ )
    {
        if ( strstr(cmd, keyTable[i].name) == cmd )
        {
            key = keyTable[i].key;
            return true;
        }
    }
    return false;
}

// Replaces the commands "\key name;" in a text.

static void PutKeyName(char* dst, const char* src)
{
    InputSlot key;
    char    name[50];
    int     s, d, n;
    unsigned int res;

    s = d = 0;
    while ( src[s] != 0 )
    {
        if ( src[s+0] == '\\' &&
             src[s+1] == 'k'  &&
             src[s+2] == 'e'  &&
             src[s+3] == 'y'  &&
             src[s+4] == ' '  )
        {
            if ( SearchKey(src+s+5, key) )
            {
                res = CRobotMain::GetInstancePointer()->GetInputBinding(key).key;
                if (res != KEY_INVALID)
                {
                    if ( GetResource(RES_KEY, res, name) )
                    {
                        n = 0;
                        while ( name[n] != 0 )
                        {
                            dst[d++] = name[n++];
                        }
                        while ( src[s++] != ';' );
                        continue;
                    }
                }
            }
        }

        dst[d++] = src[s++];
    }
    dst[d++] = 0;
}

// Returns the translated text of a resource that needs key substitution

static const char* GetResourceBase(ResType type, int num)
{
    const char *str = NULL;

    assert(num >= 0);

    switch (type)
    {
    case RES_TEXT:
    assert(num < strings_text_len);
    str = strings_text[num];
    break;
    case RES_EVENT:
    // assert(num < strings_event_len);
    if (num >= strings_event_len)
    {
        GetLogger()->Warn("GetResource invalid event num: %d\n", num);
        return "";
    }
    str = strings_event[num];
    break;
    case RES_OBJECT:
    assert(num < strings_object_len);
    if (num == OBJECT_HUMAN)
        return g_gamerName;
    str = strings_object[num];
    break;
    case RES_ERR:
    assert(num < strings_err_len);
    str = strings_err[num];
    break;
    case RES_CBOT:
    assert(num < strings_cbot_len);
    str = strings_cbot[num];
    break;
    case RES_KEY:
    assert(num < SDLK_LAST);
    // TODO: virtual keys
    str = SDL_GetKeyName(static_cast<SDLKey>(num));
    break;
    default:
    assert(false);
    }
    return gettext(str);
}

// Returns the text of a resource.

bool GetResource(ResType type, int num, char* text)
{
    const char *tmpl = GetResourceBase(type, num);

    if (!tmpl)
    {
        text[0] = 0;
        return false;
    }

    PutKeyName(text, tmpl);
    return true;
}
