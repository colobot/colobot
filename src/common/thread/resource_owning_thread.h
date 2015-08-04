#pragma once

#include "common/thread/sdl_cond_wrapper.h"
#include "common/thread/sdl_mutex_wrapper.h"

#include <SDL_thread.h>

#include <memory>

/**
 * \class ResourceOwningThread
 * \brief Wrapper around SDL thread allowing passing of resources in safe manner
 *
 * This class is a workaround for passing ownership of resources in a safe
 * manner to newly created threads. It takes a pointer to a function to call
 * in new thread and a unique_ptr to resource which is to be passed to the new thread.
 *
 * This is how it works:
 *  - in main thread: create a new thread passing to it a special temporary context,
 *  - in main thread: wait for synchronization signal that the ownership was passed,
 *  - in new thread: acquire the resource from the context
 *  - in new thread: signal back to main thread that the resource was acquired,
 *  - in main thread: clean up temporary context and exit
 *  - in new thread: run the specified function with the acquired resource.
 *
 * It's a bit complicated, but that's the safe (thread-safe and exception-safe)
 * way of doing this.
 */
template<typename Resource>
class ResourceOwningThread
{
public:
    using ResourceUPtr = std::unique_ptr<Resource>;
    using ThreadFunctionPtr = void(*)(ResourceUPtr);

    ResourceOwningThread(ThreadFunctionPtr threadFunction, ResourceUPtr resource)
        : m_threadFunction(threadFunction),
          m_resource(std::move(resource))
    {}

    void Start()
    {
        SDLMutexWrapper mutex;
        SDLCondWrapper cond;
        bool condition = false;

        ThreadData data;
        data.resource = std::move(m_resource);
        data.threadFunction = m_threadFunction;
        data.mutex = &mutex;
        data.cond = &cond;
        data.condition = &condition;

        SDL_LockMutex(*mutex);

        SDL_CreateThread(Run, reinterpret_cast<void*>(&data));

        while (!condition)
        {
            SDL_CondWait(*cond, *mutex);
        }

        SDL_UnlockMutex(*mutex);
    }

private:
    static int Run(void* data)
    {
        ThreadFunctionPtr threadFunction = nullptr;
        ResourceUPtr resource;

        ThreadData* threadData = reinterpret_cast<ThreadData*>(data);
        SDL_LockMutex(**threadData->mutex);

        threadFunction = threadData->threadFunction;
        resource = std::move(threadData->resource);

        *threadData->condition = true;
        SDL_CondSignal(**threadData->cond);
        SDL_UnlockMutex(**threadData->mutex);

        threadFunction(std::move(resource));
        return 0;
    }

private:
    struct ThreadData
    {
        ResourceUPtr resource;
        SDLMutexWrapper* mutex = nullptr;
        SDLCondWrapper* cond = nullptr;
        bool* condition = nullptr;
        ThreadFunctionPtr threadFunction = nullptr;
    };

    ThreadFunctionPtr m_threadFunction;
    ResourceUPtr m_resource;
};
