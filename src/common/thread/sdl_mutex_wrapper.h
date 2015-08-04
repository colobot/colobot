#pragma once

#include <SDL_thread.h>

class SDLMutexWrapper
{
public:
    SDLMutexWrapper()
        : m_mutex(SDL_CreateMutex())
    {}

    ~SDLMutexWrapper()
    {
        SDL_DestroyMutex(m_mutex);
    }

    SDLMutexWrapper(const SDLMutexWrapper&) = delete;
    SDLMutexWrapper& operator=(const SDLMutexWrapper&) = delete;

    SDL_mutex* operator*()
    {
        return m_mutex;
    }

private:
    SDL_mutex* m_mutex;
};
