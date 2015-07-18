#include "AudioSystem.h"
#include "Game.h"
#include "ResourceManager.h"
#include "Sound.h"
#include "Util.h"

#include <cstdlib>

AudioSystem::AudioSystem()
{
}

bool AudioSystem::initialize()
{
    m_device = alcOpenDevice(NULL);
    m_context = alcCreateContext(m_device, NULL);
    alcMakeContextCurrent(m_context);
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListener3f(AL_ORIENTATION, 0, 0, -1);
    return true;
}

void AudioSystem::render(void) const
{
    warn("AudioSystem::render is unimplemented!");
}

void AudioSystem::cleanup(void)
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}

int audio_play(lua_State* state)
{
    g_game->resources()->getAudio(lua_tostring(state, 1))->play();
    return 0;
}
