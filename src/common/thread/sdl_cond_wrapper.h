#pragma once

#include <SDL_thread.h>

/**
 * \class CSDLCondWrapper
 * \brief Wrapper for safe creation/deletion of SDL_cond
 */
class CSDLCondWrapper
{
public:
    CSDLCondWrapper()
        : m_cond(SDL_CreateCond())
    {}

    ~CSDLCondWrapper()
    {
        SDL_DestroyCond(m_cond);
    }

    CSDLCondWrapper(const CSDLCondWrapper&) = delete;
    CSDLCondWrapper& operator=(const CSDLCondWrapper&) = delete;

    SDL_cond* operator*()
    {
        return m_cond;
    }

private:
    SDL_cond* m_cond;
};
