#ifndef TWEEN_SYSTEM_H
#define TWEEN_SYSTEM_H
#include "Actor.h"
#include "System.h"
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

#include <vector>
#include <map>
#include <set>

class IEvent;

enum class CurveType
{
    LINEAR,
    EASE_IN,
    EASE_OUT,
};

struct Transition
{
    float start = 0;
    CurveType type = CurveType::LINEAR;
};

struct Tween
{
    std::vector<Transition> transitions;
    float position = 0;
    float start_value = 0;
    float end_value = 1;
    float current_value = 0;
    float length = 1;
    bool repeat = false;
    bool playing = true;
    bool reverse = false;
};

class TweenSystem : public ISystem
{
public:
    virtual bool initialize(void);
    virtual void update(float dt);
    virtual void cleanup(void);
    virtual void actorRemovedCallback(const IEvent& event);
    virtual bool contains(Tween* tween);
    friend int actor_register_tween(lua_State* state);
    friend int actor_get_tween_value(lua_State* state);
    friend int actor_get_tween(lua_State* state);
protected:
    std::map<unsigned long, std::vector<Tween*>> tween_map;
    std::set<Tween*> tweens;
};


int tweenIndex(lua_State* state);
int tweenNewIndex(lua_State* state);

#endif

static const luaL_Reg tweenMeta[] = {
    {"__index", tweenIndex},
    {"__newindex", tweenNewIndex},
    {0, 0}
};
