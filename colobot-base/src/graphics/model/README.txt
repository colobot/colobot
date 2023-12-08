/**
 * \namespace Gfx::ModelInput
 * \brief Functions related to model loading
 */

/**
 * \namespace Gfx::ModelOutput
 * \brief Functions related to model saving
 */

/**
 * \page models Models
 *
 * Model formats and associated issues are described briefly for graphics designers and developers.
 *
 * \section format Model format
 *
 * \todo Update for the new model format
 *
 * Colobot models are basically a collection of triangles with some associated data.
 * <span style="text-decoration: line-through;">In the code, the class Gfx::CModel (src/graphics/model/model.h)
 * is responsible for reading/writing model files.</span> Each triangle of model contains the information
 * as stored in Gfx::ModelTriangle struct defined in model_triangle.h header, that is:
 * * 3 triangle points (coordinates, normals, UV texture coordinates for 2 textures)
 * * material (ambient, diffuse, specular colors)
 * * file names for 1st and 2nd texture (or, for 2nd texture - variable flag)
 * * rendering state
 * * <span style="text-decoration: line-through;">min and max values of LOD (= level of details)</span>
 *
 * \subsection textures Textures
 *
 * 1st texture is always static - the assigned image name.
 *
 * 2nd texture can be set explicitly in 2nd texture name, with variable flag set to false.
 * It is then static as 1st texture.
 *
 * But if variable flag is set, the texture will be applied dynamically by the graphics engine.
 * It will be one of dirtyXX.png textures, depending on current setup.
 *
 * \subsection renderstates Rendering states
 *
 * Rendering state is one of Gfx::CEngine's rendering states, that is a mask of enum Gfx::EngineRenderState values
 * from src/graphics/engine/engine.h.
 *
 * For most purposes, the default render (Gfx::ENG_RSTATE_NORMAL = 0) state will be sufficient.
 * This state enables regular one-texture rendering.
 *
 * To use 2nd texture, set one of Gfx::ENG_RSTATE_DUAL_BLACK or Gfx::ENG_RSTATE_DUAL_WHITE states.
 * Other states, enabling specific behavior may be used in rare cases.
 *
 * \subsection lod Min and max LOD
 *
 * <span style="text-decoration: line-through;">LOD is used to display different model triangles based
 * on distance to viewer. The given triangle will only be displayed if the distance is within bounds [min, max].</span>
 *
 * <span style="text-decoration: line-through;">For now, use standard definitions of min and max which
 * fall into 3 categories:</span>
 * * <span style="text-decoration: line-through;">min = 0, max = 100 - max detail</span>
 * * <span style="text-decoration: line-through;">min = 100, max = 200 - medium detail</span>
 * * <span style="text-decoration: line-through;">min = 200, max = 1 000 000 - low detail</span>
 *
 * \section fileformats File formats
 *
 * There are currently 3 file formats recognized by Gfx::ModelInput and Gfx::ModelOutput:
 * * old binary format (in 3 versions, though mostly only the 3rd one is used) - this is the format
 *   of original model files; it is deprecated now and will be removed in the future
 * * new text format - preferred for now, as it is easy to handle and convert to other formats as necessary
 * * new binary format - contains the same information as new text format
 *
 * \section blenderimport Import/export in Blender
 *
 * The plugin to import and export models in Blender is contained in \p tools/blender-scipts.py.
 * To use it, install it as per instructions [on Blender wiki](http://wiki.blender.org/index.php/Doc:2.6/Manual/Extensions/Python/Add-Ons).
 * It will register new menu entries under File -> Import and File -> Export. Import is always active, but to export,
 * you have to select a mesh object first.
 *
 * Textures are loaded from the same directory as the model file.
 *
 * Additional data like state, variable texture flag and min and max LOD can be given as user attributes.
 *
 * If you have any problems, please contact \b piotrdz on ICC forum or IRC channels.
 */
