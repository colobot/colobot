#pragma once

#include "math/vector.h"
#include "math/point.h"

namespace Gfx {

/**
 * \struct Vertex Vertex of a primitive
 *
 * This structure was created as analog to DirectX's D3DVERTEX.
 *
 * It contains:
 *  - vertex coordinates (x,y,z) as Math::Vector,
 *  - normal coordinates (nx,ny,nz) as Math::Vector
 *  - texture coordinates (u,v) as Math::Point.
 */
struct Vertex
{
	Math::Vector coord;
	Math::Vector normal;
	Math::Point texCoord;

	Vertex(Math::Vector aCoord = Math::Vector(),
		   Math::Vector aNormal = Math::Vector(),
		   Math::Point aTexCoord = Math::Point())
		: coord(aCoord), normal(aNormal), texCoord(aTexCoord) {}
};

/**
 * \struct VertexTex2 Vertex with secondary texture coordinates
 *
 * In addition to fields from Gfx::Vector, it contains
 * secondary texture coordinates (u2, v2) as Math::Point
 */
struct VertexTex2 : public Gfx::Vertex
{
	Math::Point texCoord2;

	VertexTex2(Math::Vector aCoord = Math::Vector(),
			   Math::Vector aNormal = Math::Vector(),
			   Math::Point aTexCoord = Math::Point(),
			   Math::Point aTexCoord2 = Math::Point())
		: Vertex(aCoord, aNormal, aTexCoord), texCoord2(aTexCoord2) {}
};

};
