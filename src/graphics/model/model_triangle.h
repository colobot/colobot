#pragma once

#include "graphics/core/color.h"
#include "graphics/core/vertex.h"

namespace Gfx {

/**
 * \enum ModelSpecialMark
 * \brief Special marking for some models
 *
 * TODO: refactor/remove in the future
 */
enum class ModelSpecialMark
{
    None,
    Part1,
    Part2,
    Part3
};

/**
 * \enum ModelTransparentMode
 * \brief Describes how to deal with texture transparency
 *
 * TODO: get rid of it in the future (use only alpha channel)
 */
enum class ModelTransparentMode
{
    None,            //!< no transparency
    AlphaChannel,    //!< use alpha channel
    MapBlackToAlpha, //!< map black color to alpha
    MapWhiteToAlpha  //!< map white color to alpha
};

/**
 * \struct ModelTriangle
 * \brief A single triangle in mesh as saved in model file
 */
struct ModelTriangle
{
    //! 1st vertex
    VertexTex2  p1;
    //! 2nd vertex
    VertexTex2  p2;
    //! 3rd vertex
    VertexTex2  p3;
    //! Diffuse color
    Color diffuse;
    //! Ambient color
    Color ambient;
    //! Specular color
    Color specular;
    //! Name of 1st texture
    std::string tex1Name;
    //! Name of 2nd texture
    std::string tex2Name;
    //! If true, 2nd texture will be taken from current engine setting
    bool variableTex2 = false;
    //! Whether to render as double-sided surface
    bool doubleSided = false;
    //! How to deal with texture transparency
    ModelTransparentMode transparentMode = ModelTransparentMode::None;
    //! Special marking
    ModelSpecialMark specialMark = ModelSpecialMark::None;
};

} // namespace Gfx
