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
* \file graphics/core/framebuffer.h
* \brief Abstract representation of framebuffer and offscreen buffers
*/

#include "graphics/core/framebuffer.h"

namespace Gfx
{

CDefaultFramebuffer::CDefaultFramebuffer(const FramebufferParams& params)
    : m_width(params.width), m_height(params.height), m_depth(params.depth)
{
}

bool CDefaultFramebuffer::Create()
{
    return true;
}

void CDefaultFramebuffer::Destroy()
{
}

bool CDefaultFramebuffer::IsDefault()
{
    return true;
}

//! Returns width of buffers in this framebuffer
int CDefaultFramebuffer::GetWidth()
{
    return m_width;
}

//! Returns height of buffers in this framebuffer
int CDefaultFramebuffer::GetHeight()
{
    return m_height;
}

//! Returns depth size in bits
int CDefaultFramebuffer::GetDepth()
{
    return m_depth;
}

//! Returns number of samples or 1 if multisampling is not supported
int CDefaultFramebuffer::GetSamples()
{
    return 1;
}

//! Returns texture that contains color buffer or 0 if not available
int CDefaultFramebuffer::GetColorTexture()
{
    return 0;
}

//! Returns texture that contains depth buffer or 0 if not available
int CDefaultFramebuffer::GetDepthTexture()
{
    return 0;
}

//! Binds this framebuffer to context
void CDefaultFramebuffer::Bind()
{
}

//! Unbinds this framebuffer from context
void CDefaultFramebuffer::Unbind()
{
}

void CDefaultFramebuffer::CopyToScreen(int fromX, int fromY, int fromWidth, int fromHeight, int toX, int toY, int toWidth, int toHeight)
{
}

} // end of Gfx
