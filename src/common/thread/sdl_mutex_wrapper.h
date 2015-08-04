#pragma once

#include <SDL_thread.h>

/**
 * \class CSDLMutexWrapper
 * \brief Wrapper for safe creation/deletion of SDL_mutex
 */
class CSDLMutexWrapper
{
public:
    CSDLMutexWrapper()
        : m_mutex(SDL_CreateMutex())
    {}

    ~CSDLMutexWrapper()
    {
        SDL_DestroyMutex(m_mutex);
    }

    CSDLMutexWrapper(const CSDLMutexWrapper&) = delete;
    CSDLMutexWrapper& operator=(const CSDLMutexWrapper&) = delete;

    SDL_mutex* operator*()
    {
        return m_mutex;
    }

private:
    SDL_mutex* m_mutex;
};
