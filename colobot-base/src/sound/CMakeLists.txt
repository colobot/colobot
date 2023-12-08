target_sources(Colobot-Base PRIVATE
    sound.cpp
    sound.h
    sound_type.cpp
    sound_type.h
)

if(OPENAL_SOUND)
    target_sources(Colobot-Base PRIVATE
        oalsound/alsound.cpp
        oalsound/alsound.h
        oalsound/buffer.cpp
        oalsound/buffer.h
        oalsound/channel.cpp
        oalsound/channel.h
        oalsound/check.cpp
        oalsound/check.h
    )
endif()
