#pragma once

#include <SDL_thread.h>

class SDLCondWrapper
{
public:
    SDLCondWrapper()
        : m_cond(SDL_CreateCond())
    {}

    ~SDLCondWrapper()
    {
        SDL_DestroyCond(m_cond);
    }

    SDLCondWrapper(const SDLCondWrapper&) = delete;
    SDLCondWrapper& operator=(const SDLCondWrapper&) = delete;

    SDL_cond* operator*()
    {
        return m_cond;
    }

private:
    SDL_cond* m_cond;
};
