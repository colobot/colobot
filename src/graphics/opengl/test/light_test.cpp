#include "app/system.h"
#include "common/logger.h"
#include "common/image.h"
#include "common/iman.h"
#include "graphics/opengl/gldevice.h"
#include "math/geometry.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <unistd.h>

#include <iostream>
#include <map>

enum KeySlots
{
    K_Forward,
    K_Back,
    K_Left,
    K_Right,
    K_Up,
    K_Down,
    K_Count
};
bool KEYMAP[K_Count] = { false };

Math::Point MOUSE_POS_BASE;

Math::Vector TRANSLATION(0.0f, 2.0f, 0.0f);
Math::Vector ROTATION, ROTATION_BASE;

float CUBE_ORBIT = 0.0f;

const int FRAME_DELAY = 5000;

SystemTimeStamp *PREV_TIME = NULL, *CURR_TIME = NULL;

void Init(Gfx::CGLDevice *device)
{
    device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, true);
    device->SetShadeModel(Gfx::SHADE_SMOOTH);
}

void Render(Gfx::CGLDevice *device)
{
    device->BeginScene();

    /* Unlit part of scene */

    device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, false);
    device->SetRenderState(Gfx::RENDER_STATE_CULLING, false); // Double-sided drawing

    Math::Matrix persp;
    Math::LoadProjectionMatrix(persp, Math::PI / 4.0f, (800.0f) / (600.0f), 0.1f, 50.0f);
    device->SetTransform(Gfx::TRANSFORM_PROJECTION, persp);


    Math::Matrix viewMat;
    Math::Matrix mat;

    viewMat.LoadIdentity();

    Math::LoadRotationXMatrix(mat, -ROTATION.x);
    viewMat = Math::MultiplyMatrices(viewMat, mat);

    Math::LoadRotationYMatrix(mat, -ROTATION.y);
    viewMat = Math::MultiplyMatrices(viewMat, mat);

    Math::LoadTranslationMatrix(mat, -TRANSLATION);
    viewMat = Math::MultiplyMatrices(viewMat, mat);

    device->SetTransform(Gfx::TRANSFORM_VIEW, viewMat);

    Math::Matrix worldMat;
    worldMat.LoadIdentity();
    device->SetTransform(Gfx::TRANSFORM_WORLD, worldMat);

    Gfx::VertexCol line[2] = { Gfx::VertexCol() };

    for (int x = -40; x <= 40; ++x)
    {
        line[0].color = Gfx::Color(0.7f + x / 120.0f, 0.0f, 0.0f);
        line[0].coord.z = -40;
        line[0].coord.x = x;
        line[1].color = Gfx::Color(0.7f + x / 120.0f, 0.0f, 0.0f);
        line[1].coord.z =  40;
        line[1].coord.x = x;
        device->DrawPrimitive(Gfx::PRIMITIVE_LINES, line, 2);
    }

    for (int z = -40; z <= 40; ++z)
    {
        line[0].color = Gfx::Color(0.0f, 0.7f + z / 120.0f, 0.0f);
        line[0].coord.z = z;
        line[0].coord.x = -40;
        line[1].color = Gfx::Color(0.0f, 0.7f + z / 120.0f, 0.0f);
        line[1].coord.z = z;
        line[1].coord.x =  40;
        device->DrawPrimitive(Gfx::PRIMITIVE_LINES, line, 2);
    }


    Gfx::VertexCol quad[6] = { Gfx::VertexCol() };

    quad[0].coord = Math::Vector(-1.0f, -1.0f, 0.0f);
    quad[1].coord = Math::Vector( 1.0f, -1.0f, 0.0f);
    quad[2].coord = Math::Vector(-1.0f,  1.0f, 0.0f);
    quad[3].coord = Math::Vector( 1.0f,  1.0f, 0.0f);

    for (int i = 0; i < 6; ++i)
        quad[i].color = Gfx::Color(1.0f, 1.0f, 0.0f);

    Math::LoadTranslationMatrix(worldMat, Math::Vector(40.0f, 2.0f, 40.0f));
    device->SetTransform(Gfx::TRANSFORM_WORLD, worldMat);

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, quad, 4);

    for (int i = 0; i < 6; ++i)
        quad[i].color = Gfx::Color(0.0f, 1.0f, 1.0f);

    Math::LoadTranslationMatrix(worldMat, Math::Vector(-40.0f, 2.0f, -40.0f));
    device->SetTransform(Gfx::TRANSFORM_WORLD, worldMat);

    int planes = device->ComputeSphereVisibility(Math::Vector(0.0f, 0.0f, 0.0f), 1.0f);
    printf("Planes:");
    if (planes == 0)
        printf(" (none)");

    if (planes & Gfx::FRUSTUM_PLANE_LEFT)
        printf(" LEFT");

    if (planes & Gfx::FRUSTUM_PLANE_RIGHT)
        printf(" RIGHT");

    if (planes & Gfx::FRUSTUM_PLANE_BOTTOM)
        printf(" BOTTOM");

    if (planes & Gfx::FRUSTUM_PLANE_TOP)
        printf(" TOP");

    if (planes & Gfx::FRUSTUM_PLANE_FRONT)
        printf(" FRONT");

    if (planes & Gfx::FRUSTUM_PLANE_BACK)
        printf(" BACK");

    printf("\n");

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, quad, 4);

    for (int i = 0; i < 6; ++i)
        quad[i].color = Gfx::Color(1.0f, 0.0f, 1.0f);

    Math::LoadTranslationMatrix(worldMat, Math::Vector(10.0f, 4.5f, 5.0f));
    device->SetTransform(Gfx::TRANSFORM_WORLD, worldMat);

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, quad, 4);

    /* Moving lit cube */
    device->SetRenderState(Gfx::RENDER_STATE_LIGHTING, true);
    device->SetRenderState(Gfx::RENDER_STATE_CULLING, true); // Culling (CCW faces)

    device->SetGlobalAmbient(Gfx::Color(0.4f, 0.4f, 0.4f));

    Gfx::Light light1;
    light1.type = Gfx::LIGHT_POINT;
    light1.position = Math::Vector(10.0f, 4.5f, 5.0f);
    light1.ambient = Gfx::Color(0.2f, 0.2f, 0.2f);
    light1.diffuse = Gfx::Color(1.0f, 0.1f, 0.1f);
    light1.specular = Gfx::Color(0.0f, 0.0f, 0.0f);
    device->SetLight(0, light1);
    device->SetLightEnabled(0, true);

    /*Gfx::Light light2;
    device->SetLight(1, light2);
    device->SetLightEnabled(1, true);*/

    Gfx::Material material;
    material.ambient = Gfx::Color(0.3f, 0.3f, 0.3f);
    material.diffuse = Gfx::Color(0.8f, 0.7f, 0.6f);
    material.specular = Gfx::Color(0.0f, 0.0f, 0.0f);
    device->SetMaterial(material);

    const Gfx::Vertex cube[6][4] =
    {
        {
            // Front
            Gfx::Vertex(Math::Vector(-1.0f, -1.0f, -1.0f), Math::Vector( 0.0f,  0.0f, -1.0f)),
            Gfx::Vertex(Math::Vector( 1.0f, -1.0f, -1.0f), Math::Vector( 0.0f,  0.0f, -1.0f)),
            Gfx::Vertex(Math::Vector(-1.0f,  1.0f, -1.0f), Math::Vector( 0.0f,  0.0f, -1.0f)),
            Gfx::Vertex(Math::Vector( 1.0f,  1.0f, -1.0f), Math::Vector( 0.0f,  0.0f, -1.0f))
        },

        {
            // Back
            Gfx::Vertex(Math::Vector( 1.0f, -1.0f,  1.0f), Math::Vector( 0.0f,  0.0f,  1.0f)),
            Gfx::Vertex(Math::Vector(-1.0f, -1.0f,  1.0f), Math::Vector( 0.0f,  0.0f,  1.0f)),
            Gfx::Vertex(Math::Vector( 1.0f,  1.0f,  1.0f), Math::Vector( 0.0f,  0.0f,  1.0f)),
            Gfx::Vertex(Math::Vector(-1.0f,  1.0f,  1.0f), Math::Vector( 0.0f,  0.0f,  1.0f))
        },

        {
            // Top
            Gfx::Vertex(Math::Vector(-1.0f,  1.0f, -1.0f), Math::Vector( 0.0f,  1.0f,  0.0f)),
            Gfx::Vertex(Math::Vector( 1.0f,  1.0f, -1.0f), Math::Vector( 0.0f,  1.0f,  0.0f)),
            Gfx::Vertex(Math::Vector(-1.0f,  1.0f,  1.0f), Math::Vector( 0.0f,  1.0f,  0.0f)),
            Gfx::Vertex(Math::Vector( 1.0f,  1.0f,  1.0f), Math::Vector( 0.0f,  1.0f,  0.0f))
        },

        {
            // Bottom
            Gfx::Vertex(Math::Vector(-1.0f, -1.0f,  1.0f), Math::Vector( 0.0f, -1.0f,  0.0f)),
            Gfx::Vertex(Math::Vector( 1.0f, -1.0f,  1.0f), Math::Vector( 0.0f, -1.0f,  0.0f)),
            Gfx::Vertex(Math::Vector(-1.0f, -1.0f, -1.0f), Math::Vector( 0.0f, -1.0f,  0.0f)),
            Gfx::Vertex(Math::Vector( 1.0f, -1.0f, -1.0f), Math::Vector( 0.0f, -1.0f,  0.0f))
        },

        {
            // Left
            Gfx::Vertex(Math::Vector(-1.0f, -1.0f,  1.0f), Math::Vector(-1.0f,  0.0f,  0.0f)),
            Gfx::Vertex(Math::Vector(-1.0f, -1.0f, -1.0f), Math::Vector(-1.0f,  0.0f,  0.0f)),
            Gfx::Vertex(Math::Vector(-1.0f,  1.0f,  1.0f), Math::Vector(-1.0f,  0.0f,  0.0f)),
            Gfx::Vertex(Math::Vector(-1.0f,  1.0f, -1.0f), Math::Vector(-1.0f,  0.0f,  0.0f))
        },

        {
            // Right
            Gfx::Vertex(Math::Vector( 1.0f, -1.0f, -1.0f), Math::Vector( 1.0f,  0.0f,  0.0f)),
            Gfx::Vertex(Math::Vector( 1.0f, -1.0f,  1.0f), Math::Vector( 1.0f,  0.0f,  0.0f)),
            Gfx::Vertex(Math::Vector( 1.0f,  1.0f, -1.0f), Math::Vector( 1.0f,  0.0f,  0.0f)),
            Gfx::Vertex(Math::Vector( 1.0f,  1.0f,  1.0f), Math::Vector( 1.0f,  0.0f,  0.0f))
        }
    };

    Math::Matrix cubeTrans;
    Math::LoadTranslationMatrix(cubeTrans, Math::Vector(10.0f, 2.0f, 5.0f));
    Math::Matrix cubeRot;
    Math::LoadRotationMatrix(cubeRot, Math::Vector(0.0f, 1.0f, 0.0f), CUBE_ORBIT);
    Math::Matrix cubeRotInv;
    Math::LoadRotationMatrix(cubeRotInv, Math::Vector(0.0f, 1.0f, 0.0f), -CUBE_ORBIT);
    Math::Matrix cubeTransRad;
    Math::LoadTranslationMatrix(cubeTransRad, Math::Vector(0.0f, 0.0f, 6.0f));
    worldMat = Math::MultiplyMatrices(cubeTransRad, cubeRotInv);
    worldMat = Math::MultiplyMatrices(cubeRot, worldMat);
    worldMat = Math::MultiplyMatrices(cubeTrans, worldMat);
    device->SetTransform(Gfx::TRANSFORM_WORLD, worldMat);

    for (int i = 0; i < 6; ++i)
        device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, cube[i], 4);

    device->EndScene();
}

void Update()
{
    const float TRANS_SPEED =  6.0f; // units / sec

    GetCurrentTimeStamp(CURR_TIME);
    float timeDiff = TimeStampDiff(PREV_TIME, CURR_TIME, STU_SEC);
    CopyTimeStamp(PREV_TIME, CURR_TIME);

    CUBE_ORBIT += timeDiff * (Math::PI / 4.0f);

    Math::Vector incTrans;

    if (KEYMAP[K_Forward])
        incTrans.z = +TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Back])
        incTrans.z = -TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Right])
        incTrans.x = +TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Left])
        incTrans.x = -TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Up])
        incTrans.y = +TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Down])
        incTrans.y = -TRANS_SPEED * timeDiff;

    Math::Point rotTrans = Math::RotatePoint(-ROTATION.y, Math::Point(incTrans.x, incTrans.z));
    incTrans.x = rotTrans.x;
    incTrans.z = rotTrans.y;
    TRANSLATION += incTrans;
}

void KeyboardDown(SDLKey key)
{
    switch (key)
    {
        case SDLK_w:
            KEYMAP[K_Forward] = true;
            break;
        case SDLK_s:
            KEYMAP[K_Back] = true;
            break;
        case SDLK_d:
            KEYMAP[K_Right] = true;
            break;
        case SDLK_a:
            KEYMAP[K_Left] = true;
            break;
        case SDLK_z:
            KEYMAP[K_Down] = true;
            break;
        case SDLK_x:
            KEYMAP[K_Up] = true;
            break;
        default:
            break;
    }
}

void KeyboardUp(SDLKey key)
{
    switch (key)
    {
        case SDLK_w:
            KEYMAP[K_Forward] = false;
            break;
        case SDLK_s:
            KEYMAP[K_Back] = false;
            break;
        case SDLK_d:
            KEYMAP[K_Right] = false;
            break;
        case SDLK_a:
            KEYMAP[K_Left] = false;
            break;
        case SDLK_z:
            KEYMAP[K_Down] = false;
            break;
        case SDLK_x:
            KEYMAP[K_Up] = false;
            break;
        default:
            break;
    }
}

void MouseMove(int x, int y)
{
    Math::Point currentPos(static_cast<float>(x), static_cast<float>(y));

    static bool first = true;
    if (first || (x < 10) || (y < 10) || (x > 790) || (y > 590))
    {
        SDL_WarpMouse(400, 300);
        MOUSE_POS_BASE.x = 400;
        MOUSE_POS_BASE.y = 300;
        ROTATION_BASE = ROTATION;
        first = false;
        return;
    }

    ROTATION.y = ROTATION_BASE.y + (static_cast<float> (x - MOUSE_POS_BASE.x) / 800.0f) * Math::PI;
    ROTATION.x = ROTATION_BASE.x + (static_cast<float> (y - MOUSE_POS_BASE.y) / 600.0f) * Math::PI;
}

int main(int argc, char *argv[])
{
    CLogger logger;

    PREV_TIME = CreateTimeStamp();
    CURR_TIME = CreateTimeStamp();

    GetCurrentTimeStamp(PREV_TIME);
    GetCurrentTimeStamp(CURR_TIME);

    CInstanceManager iMan;

    // Without any error checking, for simplicity

    SDL_Init(SDL_INIT_VIDEO);

    IMG_Init(IMG_INIT_PNG);

    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();

    Uint32 videoFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;

    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    if (videoInfo->blit_hw)
        videoFlags |= SDL_HWACCEL;


    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Surface *surface = SDL_SetVideoMode(800, 600, 32, videoFlags);


    SDL_WM_SetCaption("Light Test", "Light Test");

    //SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);

    Gfx::CGLDevice *device = new Gfx::CGLDevice(Gfx::GLDeviceConfig());
    device->Create();

    Init(device);

    bool done = false;
    while (! done)
    {
        Render(device);
        Update();

        SDL_GL_SwapBuffers();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                break;
                done = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_q)
                {
                    done = true;
                    break;
                }
                else
                    KeyboardDown(event.key.keysym.sym);
            }
            else if (event.type == SDL_KEYUP)
                KeyboardUp(event.key.keysym.sym);
            else if (event.type == SDL_MOUSEMOTION)
                MouseMove(event.motion.x, event.motion.y);
        }

        usleep(FRAME_DELAY);
    }

    //SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);

    device->Destroy();
    delete device;

    SDL_FreeSurface(surface);

    IMG_Quit();

    SDL_Quit();

    DestroyTimeStamp(PREV_TIME);
    DestroyTimeStamp(CURR_TIME);

    return 0;
}
