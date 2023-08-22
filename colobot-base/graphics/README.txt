/**
 * \dir src/graphics
 * \brief Graphics engine
 */

/**
 * \namespace Gfx
 * \brief Namespace for (new) graphics code
 *
 * This namespace was created to avoid clashing with old code, but now it still serves,
 * defining a border between pure graphics engine and other parts of application.
 */

/**
 * \page graphics Graphics engine
 *
 * The graphics engine consists of 3 parts:
 * * core  - low-level device code (currently with only OpenGL implementation)
 * * main engine - managing and displaying 3D environment (terrain, models, water, sky, effects, camera, etc.)
 * * 2D interface - classes drawing the 2D interface (menus, buttons, editor, HUD elements)
 *
 * \section coords Drawing coordinates
 *
 * \subsection coords2d 2D interface
 *
 * 2D interface is drawn by setting orthogonal projection yielding the following 2D coordinate system:
 *
 * \image html 2d_coord.png
 *
 * Depth test is disabled for 2D interface, so Z coordinates are irrelevant.
 *
 * The coordinate system is constant and is independent of resolution or screen proportions.
 *
 * UI elements are laid out by computing these standard coordinates, using 640x480 resoultion as reference.
 * That is, their coordinates are computed like so: x = 32.0f/640.0f, y = 400.0f/480.0f.
 *
 * \subsection coords3d 3D environment
 *
 * 3D environment is drawn using the following coordinate system:
 *
 * \image html 3d_canonical_coords.png
 *
 * The base coordinate system is like depicted above with viewport on Z=0 plane.
 * The coordinates are then transformed by world, view and projection matrices to yield screen coordinates.
 *
 * The base coordinates are also model coordinates. All models must be modelled in this setup.
 * Scale should be kept proportional to existing models.
 *
 * The world matrix defines the transformation from model coordinate system to the point and orientation
 * in 3D scene. This matrix is defined one per every <i>graphics engine</i> object.
 * (Note the emphasis - the objects as defined in game engine do not necessarily correspond to
 * one graphics engine object.)
 *
 * The view and projection matrices define the viewing point and volume, and are mostly managed by Gfx::CCamera.
 * View is defined by Math::LoadViewMatrix() function, that is using 3 vectors: eye position (eyePt),
 * target point (lookatPt) and up vector (upVec). Projection is always perspective,
 * with changing view angle (focus).
 */