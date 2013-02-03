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

// math/test/geometry_test.cpp

/* Unit tests for functions in geometry.h */

#include "math/func.h"
#include "math/geometry.h"

#include "gtest/gtest.h"


const float TEST_TOLERANCE = 1e-5;


// Test for rewritten function RotateAngle()
TEST(GeometryTest, RotateAngleTest)
{
    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(0.0f, 0.0f), 0.0f, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(1.0f, 0.0f), 0.0f, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(1.0f, 1.0f), 0.25f * Math::PI, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(0.0f, 2.0f), 0.5f * Math::PI, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(-0.5f, 0.5f), 0.75f * Math::PI, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(-1.0f, 0.0f), Math::PI, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(-1.0f, -1.0f), 1.25f * Math::PI, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(0.0f, -2.0f), 1.5f * Math::PI, TEST_TOLERANCE));

    EXPECT_TRUE(Math::IsEqual(Math::RotateAngle(1.0f, -1.0f), 1.75f * Math::PI, TEST_TOLERANCE));
}

// Tests for other altered, complex or uncertain functions

/*

  TODO: write meaningful tests with proper test values

int TestAngle()
{
    const Math::Vector u(-0.0786076246943884, 0.2231249091714256, -1.1601361718477805);
    const Math::Vector v(-1.231228742001907, -1.720549809950561, -0.690468438834111);

    float mathResult = Math::Angle(u, v);
    float oldMathResult = Angle(VEC_TO_D3DVEC(u), VEC_TO_D3DVEC(v));

    if (! Math::IsEqual(mathResult, oldMathResult, TEST_TOLERANCE) )
        return __LINE__;

    return 0;
}

int TestRotateView()
{
    const Math::Vector center(0.617909142705555, 0.896939729454538, -0.615041943652284);
    const float angleH = 44.5;
    const float angleV = 12.3;
    const float dist = 34.76;

    Math::Vector mathResult = Math::RotateView(center, angleH, angleV, dist);
    Math::Vector oldMathResult = D3DVEC_TO_VEC(RotateView(VEC_TO_D3DVEC(center), angleH, angleV, dist));

    if (! Math::VectorsEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLookatPoint()
{
    const Math::Vector eye(-2.451183170579471,  0.241270270546559, -0.490677411454893);
    const float angleH = 48.4;
    const float angleV = 32.4;
    const float length = 74.44;

    Math::Vector mathResult = Math::LookatPoint(eye, angleH, angleV, length);
    Math::Vector oldMathResult = D3DVEC_TO_VEC(LookatPoint(VEC_TO_D3DVEC(eye), angleH, angleV, length));

    if (! Math::VectorsEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestProjection()
{
    const Math::Vector a(0.852064846846319, -0.794279497087496, -0.655779805476688);
    const Math::Vector b(-0.245838834102304, -0.841115596038861, 0.470457161487799);
    const Math::Vector p(2.289326061164255, -0.505511362271196,  0.660204551169491);

    Math::Vector mathResult = Math::Projection(a, b, p);
    Math::Vector oldMathResult = D3DVEC_TO_VEC(Projection(VEC_TO_D3DVEC(a), VEC_TO_D3DVEC(b), VEC_TO_D3DVEC(p)));

    if (! Math::VectorsEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadViewMatrix()
{
    const Math::Vector from(2.5646013154868874, -0.6058794133917031, -0.0441195127419744);
    const Math::Vector at(0.728044925765569, -0.206343977871841, 2.543158236935463);
    const Math::Vector worldUp(-1.893738133660711, -1.009584441407070, 0.521745988225582);

    Math::Matrix mathResult;
    Math::LoadViewMatrix(mathResult, from, at, worldUp);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DVECTOR fromD3D = VEC_TO_D3DVEC(from);
        D3DVECTOR atD3D = VEC_TO_D3DVEC(at);
        D3DVECTOR worldUpD3D = VEC_TO_D3DVEC(worldUp);
        D3DUtil_SetViewMatrix(mat, fromD3D, atD3D, worldUpD3D);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadProjectionMatrix()
{
    const float fov = 76.3f;
    const float aspect = 0.891f;
    const float nearPlane = 12.3f;
    const float farPlane = 1238.9f;

    Math::Matrix mathResult;
    Math::LoadProjectionMatrix(mathResult, fov, aspect, nearPlane, farPlane);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DUtil_SetProjectionMatrix(mat, fov, aspect, nearPlane, farPlane);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadTranslationMatrix()
{
    const Math::Vector translation(-0.3631590720995237, 1.6976327614875211, 0.0148815191502145);

    Math::Matrix mathResult;
    Math::LoadTranslationMatrix(mathResult, translation);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DUtil_SetTranslateMatrix(mat, translation.x, translation.y, translation.z);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadScaleMatrix()
{
    const Math::Vector scale(0.612236460285503, -0.635566935025364, -0.254321375332065);

    Math::Matrix mathResult;
    Math::LoadScaleMatrix(mathResult, scale);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DUtil_SetScaleMatrix(mat, scale.x, scale.y, scale.z);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadRotationXMatrix()
{
    const float angle = 0.513790685774275;

    Math::Matrix mathResult;
    Math::LoadRotationXMatrix(mathResult, angle);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DUtil_SetRotateXMatrix(mat, angle);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadRotationYMatrix()
{
    const float angle = -0.569166650127303;

    Math::Matrix mathResult;
    Math::LoadRotationYMatrix(mathResult, angle);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DUtil_SetRotateYMatrix(mat, angle);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadRotationZMatrix()
{
    const float angle = 0.380448034347452;

    Math::Matrix mathResult;
    Math::LoadRotationZMatrix(mathResult, angle);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DUtil_SetRotateZMatrix(mat, angle);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadRotationMatrix()
{
    const float angle = -0.987747190637790;
    const Math::Vector dir(-0.113024727688331, -0.781265998072571, 1.838972397076884);

    Math::Matrix mathResult;
    Math::LoadRotationMatrix(mathResult, dir, angle);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        D3DVECTOR dirD3D = VEC_TO_D3DVEC(dir);
        D3DUtil_SetRotationMatrix(mat, dirD3D, angle);
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadRotationXZYMatrix()
{
    const Math::Vector angles(-0.841366567984597, -0.100543315396357, 1.610647811559988);

    Math::Matrix mathResult;
    Math::LoadRotationXZYMatrix(mathResult, angles);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        MatRotateXZY(mat, VEC_TO_D3DVEC(angles));
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestLoadRotationZXYMatrix()
{
    const Math::Vector angles(0.275558495480206, -0.224328265970090, 0.943077216574253);

    Math::Matrix mathResult;
    Math::LoadRotationZXYMatrix(mathResult, angles);

    Math::Matrix oldMathResult;
    {
        D3DMATRIX mat;
        MatRotateZXY(mat, VEC_TO_D3DVEC(angles));
        oldMathResult = D3DMAT_TO_MAT(mat);
    }

    if (! Math::MatricesEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

int TestTransform()
{
    Math::Matrix transformMatrix(
        (float[4][4])
        {
            { -0.9282074720977896,  0.6794734970319730, -1.3234304946882685,  0.0925294727863890 },
            { -0.0395527963683484,  0.2897634352353881,  1.9144398570315440, -1.4062267508968478 },
            {  0.9133323625282361, -0.6741836434774530, -0.2188812951424338, -1.0089184339952666 },
            {  0.0f, 0.0f, 0.0f, 1.0f }
        }
    );
    Math::Vector vector(-0.314596433318370, -0.622681232583150, -0.371307535743574);

    Math::Vector mathResult = Math::Transform(transformMatrix, vector);
    Math::Vector oldMathResult = Transform(transformMatrix, vector);

    if (! Math::VectorsEqual(mathResult, oldMathResult, TEST_TOLERANCE))
        return __LINE__;

    return 0;
}

*/

