#include "app/system.h"

#include "common/config.h"
#include "common/logger.h"
#include "common/image.h"

#include "graphics/engine/modelfile.h"
#include "graphics/opengl/gldevice.h"

#include "math/geometry.h"

#include <SDL.h>
#include <SDL_image.h>
#include <unistd.h>

#include <iostream>
#include <map>

enum KeySlots
{
    K_RotXUp,
    K_RotXDown,
    K_RotYLeft,
    K_RotYRight,
    K_Forward,
    K_Back,
    K_Left,
    K_Right,
    K_Up,
    K_Down,
    K_Count
};
bool KEYMAP[K_Count] = { false };

Math::Vector TRANSLATION(0.0f, 0.0f, 30.0f);
Math::Vector ROTATION;

const int FRAME_DELAY = 5000;

std::map<std::string, Gfx::Texture> TEXS;

SystemTimeStamp *PREV_TIME = NULL, *CURR_TIME = NULL;

Gfx::Texture GetTexture(const std::string &name)
{
    std::map<std::string, Gfx::Texture>::iterator it = TEXS.find(name);
    if (it == TEXS.end())
        return Gfx::Texture();

    return (*it).second;
}

void LoadTexture(Gfx::CGLDevice *device, const std::string &name)
{
    if (name.empty())
        return;

    Gfx::Texture tex = GetTexture(name);

    if (tex.Valid())
        return;

    CImage img;
    if (! img.Load(std::string("tex/") + name))
    {
        std::string err = img.GetError();
        GetLogger()->Error("Texture not loaded, error: %s!\n", err.c_str());
    }
    else
    {
        Gfx::TextureCreateParams texCreateParams;
        texCreateParams.mipmap = true;
        texCreateParams.minFilter = Gfx::TEX_MIN_FILTER_LINEAR_MIPMAP_LINEAR;
        texCreateParams.magFilter = Gfx::TEX_MAG_FILTER_LINEAR;

        tex = device->CreateTexture(&img, texCreateParams);
    }

    TEXS[name] = tex;
}

void Init(Gfx::CGLDevice *device, Gfx::CModelFile *model)
{
    const std::vector<Gfx::ModelTriangle> &triangles = model->GetTriangles();

    for (int i = 0; i < static_cast<int>( triangles.size() ); ++i)
    {
        LoadTexture(device, triangles[i].tex1Name);
        LoadTexture(device, triangles[i].tex2Name);
    }

    device->SetRenderState(Gfx::RENDER_STATE_LIGHTING,   true);
    device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, true);
    device->SetShadeModel(Gfx::SHADE_SMOOTH);

    Gfx::Light light;
    light.type = Gfx::LIGHT_DIRECTIONAL;
    light.ambient = Gfx::Color(0.4f, 0.4f, 0.4f, 0.0f);
    light.diffuse = Gfx::Color(0.8f, 0.8f, 0.8f, 0.0f);
    light.specular = Gfx::Color(0.2f, 0.2f, 0.2f, 0.0f);
    light.position = Math::Vector(0.0f, 0.0f, -1.0f);
    light.direction = Math::Vector(0.0f, 0.0f, 1.0f);

    device->SetGlobalAmbient(Gfx::Color(0.5f, 0.5f, 0.5f, 0.0f));
    device->SetLight(0, light);
    device->SetLightEnabled(0, true);
}

void Render(Gfx::CGLDevice *device, Gfx::CModelFile *modelFile)
{
    device->BeginScene();

    Math::Matrix persp;
    Math::LoadProjectionMatrix(persp, Math::PI / 4.0f, (800.0f) / (600.0f), 0.1f, 100.0f);
    device->SetTransform(Gfx::TRANSFORM_PROJECTION, persp);

    Math::Matrix id;
    id.LoadIdentity();
    device->SetTransform(Gfx::TRANSFORM_WORLD, id);

    Math::Matrix viewMat;
    Math::LoadTranslationMatrix(viewMat, TRANSLATION);
    Math::Matrix rot;
    Math::LoadRotationXZYMatrix(rot, ROTATION);
    viewMat = Math::MultiplyMatrices(viewMat, rot);
    device->SetTransform(Gfx::TRANSFORM_VIEW, viewMat);

    const std::vector<Gfx::ModelTriangle> &triangles = modelFile->GetTriangles();

    Gfx::VertexTex2 tri[3];

    for (int i = 0; i < static_cast<int>( triangles.size() ); ++i)
    {
        device->SetTexture(0, GetTexture(triangles[i].tex1Name));
        device->SetTexture(1, GetTexture(triangles[i].tex2Name));
        device->SetTextureEnabled(0, true);
        device->SetTextureEnabled(1, true);

        device->SetMaterial(triangles[i].material);

        tri[0] = triangles[i].p1;
        tri[1] = triangles[i].p2;
        tri[2] = triangles[i].p3;

        device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, tri, 3);
    }

    device->EndScene();
}

void Update()
{
    const float ROT_SPEED = 80.0f * Math::DEG_TO_RAD; // rad / sec
    const float TRANS_SPEED =  3.0f; // units / sec

    GetSystemUtils()->GetCurrentTimeStamp(CURR_TIME);
    float timeDiff = GetSystemUtils()->TimeStampDiff(PREV_TIME, CURR_TIME, STU_SEC);
    GetSystemUtils()->CopyTimeStamp(PREV_TIME, CURR_TIME);

    if (KEYMAP[K_RotYLeft])
        ROTATION.y -= ROT_SPEED * timeDiff;
    if (KEYMAP[K_RotYRight])
        ROTATION.y += ROT_SPEED * timeDiff;
    if (KEYMAP[K_RotXDown])
        ROTATION.x -= ROT_SPEED * timeDiff;
    if (KEYMAP[K_RotXUp])
        ROTATION.x += ROT_SPEED * timeDiff;

    if (KEYMAP[K_Forward])
        TRANSLATION.z -= TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Back])
        TRANSLATION.z += TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Left])
        TRANSLATION.x += TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Right])
        TRANSLATION.x -= TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Up])
        TRANSLATION.y += TRANS_SPEED * timeDiff;
    if (KEYMAP[K_Down])
        TRANSLATION.y -= TRANS_SPEED * timeDiff;
}

void KeyboardDown(SDLKey key)
{
    switch (key)
    {
        case SDLK_LEFT:
            KEYMAP[K_RotYLeft] = true;
            break;
        case SDLK_RIGHT:
            KEYMAP[K_RotYRight] = true;
            break;
        case SDLK_UP:
            KEYMAP[K_RotXUp] = true;
            break;
        case SDLK_DOWN:
            KEYMAP[K_RotXDown] = true;
            break;
        case SDLK_w:
            KEYMAP[K_Forward] = true;
            break;
        case SDLK_s:
            KEYMAP[K_Back] = true;
            break;
        case SDLK_a:
            KEYMAP[K_Left] = true;
            break;
        case SDLK_d:
            KEYMAP[K_Right] = true;
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
        case SDLK_LEFT:
            KEYMAP[K_RotYLeft] = false;
            break;
        case SDLK_RIGHT:
            KEYMAP[K_RotYRight] = false;
            break;
        case SDLK_UP:
            KEYMAP[K_RotXUp] = false;
            break;
        case SDLK_DOWN:
            KEYMAP[K_RotXDown] = false;
            break;
        case SDLK_w:
            KEYMAP[K_Forward] = false;
            break;
        case SDLK_s:
            KEYMAP[K_Back] = false;
            break;
        case SDLK_a:
            KEYMAP[K_Left] = false;
            break;
        case SDLK_d:
            KEYMAP[K_Right] = false;
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

extern "C"
{

int SDL_MAIN_FUNC(int argc, char *argv[])
{
    CLogger logger;

    PREV_TIME = GetSystemUtils()->CreateTimeStamp();
    CURR_TIME = GetSystemUtils()->CreateTimeStamp();

    GetSystemUtils()->GetCurrentTimeStamp(PREV_TIME);
    GetSystemUtils()->GetCurrentTimeStamp(CURR_TIME);

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << "{old|new_txt|new_bin} model_file" << std::endl;
        return 1;
    }

    Gfx::CModelFile *modelFile = new Gfx::CModelFile();
    if (std::string(argv[1]) == "old")
    {
        if (! modelFile->ReadModel(argv[2]))
        {
            std::cerr << "Error reading model file" << std::endl;
            return 1;
        }
    }
    else if (std::string(argv[1]) == "new_txt")
    {
        if (! modelFile->ReadTextModel(argv[2]))
        {
            std::cerr << "Error reading model file" << std::endl;
            return 1;
        }
    }
    else if (std::string(argv[1]) == "new_bin")
    {
        if (! modelFile->ReadBinaryModel(argv[2]))
        {
            std::cerr << "Error reading model file" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << "{old|new_txt|new_bin} model_file" << std::endl;
        return 1;
    }

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


    SDL_WM_SetCaption("Model Test", "Model Test");

    Gfx::CGLDevice *device = new Gfx::CGLDevice(Gfx::GLDeviceConfig());
    device->Create();

    Init(device, modelFile);

    bool done = false;
    while (! done)
    {
        Render(device, modelFile);
        Update();

        SDL_GL_SwapBuffers();

        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            done = true;
        else if (event.type == SDL_KEYDOWN)
            KeyboardDown(event.key.keysym.sym);
        else if (event.type == SDL_KEYUP)
            KeyboardUp(event.key.keysym.sym);

        usleep(FRAME_DELAY);
    }

    delete modelFile;

    device->Destroy();
    delete device;

    SDL_FreeSurface(surface);

    IMG_Quit();

    SDL_Quit();

    GetSystemUtils()->DestroyTimeStamp(PREV_TIME);
    GetSystemUtils()->DestroyTimeStamp(CURR_TIME);

    return 0;
}

} // extern "C"
