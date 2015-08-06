#include "sound/oalsound/check.h"

namespace
{
ALenum g_errorCode = AL_NO_ERROR;
} // anonymous namespace

bool CheckOpenALError()
{
    g_errorCode = alGetError();
    return g_errorCode != AL_NO_ERROR;
}

ALenum GetOpenALErrorCode()
{
    ALenum ret = g_errorCode;
    g_errorCode = AL_NO_ERROR;
    return ret;
}
