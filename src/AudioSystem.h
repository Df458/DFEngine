#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "System.h"

#include <AL/al.h>
#include <AL/alc.h>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

class Level;

class IAudio
{
public:
    virtual ~IAudio(void) = 0;
    virtual void render(void) const = 0;
};

inline IAudio::~IAudio() {}

class AudioSystem : public IAudio, public ISystem
{
public:
    AudioSystem(void);
    virtual ~AudioSystem(void) {}
    virtual bool initialize(void);
    virtual void update(float dt);
    virtual void render(void) const;
    virtual void cleanup(void);

private:
    ALCdevice* m_device;
    ALCcontext* m_context;
};

int audio_play(lua_State* state);

const luaL_Reg audio_funcs[] =
{
    {"play", audio_play},
    {0, 0}
};

#endif
