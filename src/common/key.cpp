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

#include "common/key.h"

unsigned int GetVirtualKey(unsigned int key)
{
    if(key == KEY(LCTRL) || key == KEY(RCTRL))
        return VIRTUAL_KMOD(CTRL);
    if(key == KEY(LSHIFT) || key == KEY(RSHIFT))
        return VIRTUAL_KMOD(SHIFT);
    if(key == KEY(LALT) || key == KEY(RALT))
        return VIRTUAL_KMOD(ALT);
    if(key == KEY(LGUI) || key == KEY(RGUI))
        return VIRTUAL_KMOD(GUI);

    if(key == KEY(KP_ENTER))
        return KEY(RETURN);

    return key;
}
