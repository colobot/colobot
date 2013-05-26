// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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


#include "common/logger.h"
#include "graphics/engine/modelfile.h"
#include "math/func.h"

#include <gtest/gtest.h>

#include <cassert>
#include <sstream>

/* Test model file (new text format) */
const char* const TEXT_MODEL =
"# Colobot text model\n"
"\n"
"### HEAD\n"
"version 1\n"
"total_triangles 2\n"
"\n"
"### TRIANGLES\n"
"p1 c -12.4099 10.0016 -2.54558 n 1 -0 1.87319e-07 t1 0.970703 0.751953 t2 0 0\n"
"p2 c -12.4099 10.0016 2.54558 n 1 -0 1.87319e-07 t1 0.998047 0.751953 t2 0 0\n"
"p3 c -12.4099 4.00165 -2.54558 n 1 -0 1.87319e-07 t1 0.970703 0.998047 t2 0 0\n"
"mat dif 1 1 1 0 amb 0.5 0.5 0.5 0 spc 0 0 0 0\n"
"tex1 lemt.png\n"
"tex2\n"
"var_tex2 N\n"
"lod_level 0\n"
"state 1024\n"
"\n"
"p1 c -19 -1 4 n -1 0 0 t1 0.248047 0.123047 t2 0.905224 0.52067\n"
"p2 c -19 4 4 n -1 0 0 t1 0.248047 0.00195312 t2 0.905224 0.614223\n"
"p3 c -19 4 -4 n -1 0 0 t1 0.00195312 0.00195312 t2 0.0947756 0.614223\n"
"mat dif 1 1 1 0 amb 0.5 0.5 0.5 0 spc 0 0 0 0\n"
"tex1 derrick.png\n"
"tex2\n"
"var_tex2 Y\n"
"lod_level 1\n"
"state 0\n"
"";

// Triangles as defined in model file
Gfx::ModelTriangle TRIANGLE_1;
Gfx::ModelTriangle TRIANGLE_2;

// Sets triangle data
void Init()
{

    TRIANGLE_1.p1 = Gfx::VertexTex2(Math::Vector(-12.4099, 10.0016, -2.54558),
                                        Math::Vector(1, 0, 1.87319e-07),
                                        Math::Point(0.970703, 0.751953),
                                        Math::Point(0, 0));
    TRIANGLE_1.p2 = Gfx::VertexTex2(Math::Vector(-12.4099, 10.0016, 2.54558),
                                        Math::Vector(1, 0, 1.87319e-07),
                                        Math::Point(0.998047, 0.751953),
                                        Math::Point(0, 0));
    TRIANGLE_1.p3 = Gfx::VertexTex2(Math::Vector(-12.4099, 4.00165, -2.54558),
                                        Math::Vector(1, 0, 1.87319e-07),
                                        Math::Point(0.970703, 0.998047),
                                        Math::Point(0, 0));
    TRIANGLE_1.material.diffuse = Gfx::Color(1, 1, 1, 0);
    TRIANGLE_1.material.ambient = Gfx::Color(0.5, 0.5, 0.5, 0);
    TRIANGLE_1.material.specular = Gfx::Color(0, 0, 0, 0);
    TRIANGLE_1.tex1Name = "lemt.png";
    TRIANGLE_1.variableTex2 = false;
    TRIANGLE_1.lodLevel = Gfx::LOD_Constant;
    TRIANGLE_1.state = 1024;

    TRIANGLE_2.p1 = Gfx::VertexTex2(Math::Vector(-19, -1, 4),
                                       Math::Vector(-1, 0, 0),
                                       Math::Point(0.248047, 0.123047),
                                       Math::Point(0.905224, 0.52067));
    TRIANGLE_2.p2 = Gfx::VertexTex2(Math::Vector(-19, 4, 4),
                                       Math::Vector(-1, 0, 0),
                                       Math::Point(0.248047, 0.00195312),
                                       Math::Point(0.905224, 0.614223));
    TRIANGLE_2.p3 = Gfx::VertexTex2(Math::Vector(-19, 4, -4),
                                       Math::Vector(-1, 0, 0),
                                       Math::Point(0.00195312, 0.00195312),
                                       Math::Point(0.0947756, 0.614223));
    TRIANGLE_2.material.diffuse = Gfx::Color(1, 1, 1, 0);
    TRIANGLE_2.material.ambient = Gfx::Color(0.5, 0.5, 0.5, 0);
    TRIANGLE_2.material.specular = Gfx::Color(0, 0, 0, 0);
    TRIANGLE_2.tex1Name = "derrick.png";
    TRIANGLE_2.variableTex2 = true;
    TRIANGLE_2.lodLevel = Gfx::LOD_Low;
    TRIANGLE_2.state = 0;
}


// Compares vertices (within Math::TOLERANCE)
bool CompareVertices(const Gfx::VertexTex2& v1, const Gfx::VertexTex2& v2)
{
    if ( !( Math::IsEqual(v1.coord.x, v2.coord.x) &&
            Math::IsEqual(v1.coord.y, v2.coord.y) &&
            Math::IsEqual(v1.coord.z, v2.coord.z) ) )
        return false;

    if ( !( Math::IsEqual(v1.normal.x, v2.normal.x) &&
            Math::IsEqual(v1.normal.y, v2.normal.y) &&
            Math::IsEqual(v1.normal.z, v2.normal.z) ) )
        return false;

    if ( !( Math::IsEqual(v1.texCoord.x, v2.texCoord.x) &&
            Math::IsEqual(v1.texCoord.y, v2.texCoord.y) ) )
        return false;

    if ( !( Math::IsEqual(v1.texCoord2.x, v2.texCoord2.x) &&
            Math::IsEqual(v1.texCoord2.y, v2.texCoord2.y) ) )
        return false;

    return true;
}

// Compares colors (within Math::TOLERANCE)
bool CompareColors(const Gfx::Color& c1, const Gfx::Color& c2)
{
    return Math::IsEqual(c1.r, c2.r) &&
           Math::IsEqual(c1.g, c2.g) &&
           Math::IsEqual(c1.b, c2.b) &&
           Math::IsEqual(c1.a, c2.a);
}

// Compares model triangles (within Math::TOLERANCE)
bool CompareTriangles(const Gfx::ModelTriangle& t1, const Gfx::ModelTriangle& t2)
{
    if (! CompareVertices(t1.p1, t2.p1))
        return false;

    if (! CompareVertices(t1.p2, t2.p2))
        return false;

    if (! CompareVertices(t1.p3, t2.p3))
        return false;

    if (! CompareColors(t1.material.diffuse, t2.material.diffuse))
        return false;

    if (! CompareColors(t1.material.ambient, t2.material.ambient))
        return false;

    if (! CompareColors(t1.material.specular, t2.material.specular))
        return false;

    if (t1.tex1Name != t2.tex1Name)
        return false;

    if (t1.tex2Name != t2.tex2Name)
        return false;

    if (t1.variableTex2 != t2.variableTex2)
        return false;

    if (t1.lodLevel != t2.lodLevel)
        return false;

    if (t1.state != t2.state)
        return false;

    return true;
}

// Tests reading/writing new text model file
TEST(ModelFileTest, RWTxtModel)
{
    std::stringstream str;
    str.str(TEXT_MODEL);

    Gfx::CModelFile modelFile;

    EXPECT_TRUE(modelFile.ReadTextModel(str));

    EXPECT_EQ(modelFile.GetTriangleCount(), 2);
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[0], TRIANGLE_1));
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[1], TRIANGLE_2));

    str.str("");

    EXPECT_TRUE(modelFile.WriteTextModel(str));
    str.seekg(0);
    EXPECT_TRUE(modelFile.ReadTextModel(str));

    EXPECT_EQ(modelFile.GetTriangleCount(), 2);
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[0], TRIANGLE_1));
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[1], TRIANGLE_2));
}

// Tests reading/writing new binary model
TEST(ModelFileTest, RWBinModel)
{
    std::stringstream str;
    str.str(TEXT_MODEL);

    Gfx::CModelFile modelFile;

    EXPECT_TRUE(modelFile.ReadTextModel(str));

    EXPECT_EQ(modelFile.GetTriangleCount(), 2);
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[0], TRIANGLE_1));
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[1], TRIANGLE_2));

    str.str("");

    EXPECT_TRUE(modelFile.WriteBinaryModel(str));
    str.seekg(0);
    EXPECT_TRUE(modelFile.ReadBinaryModel(str));

    EXPECT_EQ(modelFile.GetTriangleCount(), 2);
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[0], TRIANGLE_1));
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[1], TRIANGLE_2));
}

// Tests reading/writing old model file
TEST(ModelFileTest, RWOldModel)
{
    std::stringstream str;
    str.str(TEXT_MODEL);

    Gfx::CModelFile modelFile;

    EXPECT_TRUE(modelFile.ReadTextModel(str));

    EXPECT_EQ(modelFile.GetTriangleCount(), 2);
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[0], TRIANGLE_1));
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[1], TRIANGLE_2));

    str.str("");

    EXPECT_TRUE(modelFile.WriteModel(str));
    str.seekg(0);
    EXPECT_TRUE(modelFile.ReadModel(str));

    EXPECT_EQ(modelFile.GetTriangleCount(), 2);
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[0], TRIANGLE_1));
    EXPECT_TRUE(CompareTriangles(modelFile.GetTriangles()[1], TRIANGLE_2));
}

int main(int argc, char **argv)
{
    CLogger logger;

    Init();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

