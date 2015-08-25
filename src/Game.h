#ifndef GAME_H
#define GAME_H

#include "Event.h"
#include "System.h"
#include <functional>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

typedef std::pair<void*, std::function<void(const IEvent&)>> Callback;
class ActorSystem;
class AudioSystem;
class IComponentFactory;
class EventSystem;
class GraphicsSystem;
class InputSystem;
class PhysicsSystem;
class ResourceManager;
class TweenSystem;

class Game
{
public:
    Game(void);
    virtual bool initialize(void);
    virtual void mainLoop(void);
    virtual void cleanup(void);
    virtual bool buildLevel(std::string level, bool keep_actors = false);
    void quit(void);
    bool isQuitting(void) { return m_quit; }

    inline ResourceManager* resources(void) const { return m_resources; }
    inline IComponentFactory* components(void) const { return m_components; }
    inline InputSystem* input(void) const { return m_input; }
    inline EventSystem* events(void) const { return m_events; }
    inline PhysicsSystem* physics(void) const { return m_physics; }
    inline ActorSystem* actors(void) const { return m_actors; }
    inline GraphicsSystem* graphics(void) const { return m_graphics; }
    inline TweenSystem* tweens(void) const { return m_tweens; }
protected:
    EventSystem* m_events;
    ActorSystem* m_actors;
    AudioSystem* m_audio;
    GraphicsSystem* m_graphics;
    InputSystem* m_input;
    PhysicsSystem* m_physics;
    ResourceManager* m_resources;
    TweenSystem* m_tweens;
    IComponentFactory* m_components;
    float m_delta_time;
private:
    bool m_quit = false;
};

class DFBaseGame : public Game
{
public:
    DFBaseGame(void);
    virtual bool initialize(void) final;
    virtual void cleanup(void) final;
};

// TODO: Break up game script callbacks so that they're tied to system userdata
int game_create_actor(lua_State* state);
int game_exit(lua_State* state);
int game_debug_render(lua_State* state);
int game_get_actor(lua_State* state);
int game_get_actors(lua_State* state);
int game_load_level(lua_State* state);

const luaL_Reg game_funcs[] =
{
    {"create_actor", game_create_actor},
    {"get_actor", game_get_actor},
    {"get_actors", game_get_actors},
    {"debug_render", game_debug_render},
    {"load_level", game_load_level},
    {"exit", game_exit},
    {0, 0}
};

extern Game* g_game;

#endif
