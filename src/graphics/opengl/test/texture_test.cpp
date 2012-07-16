#include "common/logger.h"
#include "common/image.h"
#include "graphics/opengl/gldevice.h"
#include "math/geometry.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <unistd.h>


void Init(Gfx::CGLDevice *device)
{
    device->SetRenderState(Gfx::RENDER_STATE_DEPTH_TEST, false);
    device->SetShadeModel(Gfx::SHADE_SMOOTH);

    CImage img1;
    if (! img1.Load("tex1.png"))
    {
        std::string err = img1.GetError();
        GetLogger()->Error("texture 1 not loaded, error: %d!\n", err.c_str());
    }
    CImage img2;
    if (! img2.Load("tex2.png"))
    {
        std::string err = img2.GetError();
        GetLogger()->Error("texture 2 not loaded, error: %d!\n", err.c_str());
    }

    Gfx::TextureCreateParams tex1CreateParams;
    tex1CreateParams.mipmap = true;
    tex1CreateParams.format = Gfx::TEX_IMG_RGBA;
    tex1CreateParams.minFilter = Gfx::TEX_MIN_FILTER_LINEAR_MIPMAP_LINEAR;
    tex1CreateParams.magFilter = Gfx::TEX_MAG_FILTER_LINEAR;
    tex1CreateParams.wrapT = Gfx::TEX_WRAP_CLAMP;

    Gfx::TextureCreateParams tex2CreateParams;
    tex2CreateParams.mipmap = true;
    tex2CreateParams.format = Gfx::TEX_IMG_RGBA;
    tex2CreateParams.minFilter = Gfx::TEX_MIN_FILTER_NEAREST_MIPMAP_NEAREST;
    tex2CreateParams.magFilter = Gfx::TEX_MAG_FILTER_NEAREST;
    tex2CreateParams.wrapS = Gfx::TEX_WRAP_CLAMP;

    Gfx::Texture* tex1 = device->CreateTexture(&img1, tex1CreateParams);
    Gfx::Texture* tex2 = device->CreateTexture(&img2, tex2CreateParams);

    device->SetTexture(0, tex1);
    device->SetTexture(1, tex2);

    Gfx::TextureParams tex1Params;
    tex1Params.alphaOperation = Gfx::TEX_MIX_OPER_MODULATE;
    tex1Params.colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
    device->SetTextureParams(0, tex1Params);

    Gfx::TextureParams tex2Params;
    tex2Params.alphaOperation = Gfx::TEX_MIX_OPER_MODULATE;
    tex2Params.colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
    device->SetTextureParams(1, tex2Params);

    device->SetRenderState(Gfx::RENDER_STATE_TEXTURING, true);
}

void Render(Gfx::CGLDevice *device)
{
    device->BeginScene();

    Math::Matrix ortho;
    Math::LoadOrthoProjectionMatrix(ortho, -10, 10, -10, 10);
    device->SetTransform(Gfx::TRANSFORM_PROJECTION, ortho);

    Math::Matrix id;
    id.LoadIdentity();

    device->SetTransform(Gfx::TRANSFORM_WORLD, id);
    device->SetTransform(Gfx::TRANSFORM_VIEW, id);

    static Gfx::VertexTex2 quad[] =
    {
        Gfx::VertexTex2(Math::Vector(-2.0f, -2.0f, 0.0f), Math::Vector(), Math::Point(0.0f, 1.0f), Math::Point(0.0f, 1.0f)),
        Gfx::VertexTex2(Math::Vector( 2.0f, -2.0f, 0.0f), Math::Vector(), Math::Point(1.0f, 1.0f), Math::Point(1.0f, 1.0f)),
        Gfx::VertexTex2(Math::Vector( 2.0f,  2.0f, 0.0f), Math::Vector(), Math::Point(1.0f, 0.0f), Math::Point(1.0f, 0.0f)),

        Gfx::VertexTex2(Math::Vector( 2.0f,  2.0f, 0.0f), Math::Vector(), Math::Point(1.0f, 0.0f), Math::Point(1.0f, 0.0f)),
        Gfx::VertexTex2(Math::Vector(-2.0f,  2.0f, 0.0f), Math::Vector(), Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f)),
        Gfx::VertexTex2(Math::Vector(-2.0f, -2.0f, 0.0f), Math::Vector(), Math::Point(0.0f, 1.0f), Math::Point(0.0f, 1.0f)),
    };

    Math::Matrix t;
    Math::LoadTranslationMatrix(t, Math::Vector(-4.0f, 4.0f, 0.0f));
    device->SetTransform(Gfx::TRANSFORM_VIEW, t);

    device->SetTextureEnabled(0, true);
    device->SetTextureEnabled(1, false);

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, quad, 6);

    Math::LoadTranslationMatrix(t, Math::Vector( 4.0f, 4.0f, 0.0f));
    device->SetTransform(Gfx::TRANSFORM_VIEW, t);

    device->SetTextureEnabled(0, false);
    device->SetTextureEnabled(1, true);

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, quad, 6);

    device->SetTextureEnabled(0, true);
    device->SetTextureEnabled(1, true);

    Math::LoadTranslationMatrix(t, Math::Vector( 0.0f, -4.0f, 0.0f));
    device->SetTransform(Gfx::TRANSFORM_VIEW, t);

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, quad, 6);

    device->EndScene();
}

int main()
{
    CLogger();

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


    SDL_WM_SetCaption("Texture Test", "Texture Test");

    Gfx::CGLDevice *device = new Gfx::CGLDevice();
    device->Create();

    Init(device);

    bool done = false;
    while (! done)
    {
        Render(device);

        SDL_GL_SwapBuffers();

        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            done = true;

        usleep(10000);
    }

    device->Destroy();
    delete device;

    SDL_FreeSurface(surface);

    IMG_Quit();

    SDL_Quit();

    return 0;
}
