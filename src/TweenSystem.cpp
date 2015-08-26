#include "TweenSystem.h"
#include "Actor.h"
#include "Game.h"
#include "EventSystem.h"
#include "Event.h"
#include "Util.h"
#include <functional>

using namespace std::placeholders;

bool TweenSystem::initialize(void)
{
    auto destr_callback = std::bind(&TweenSystem::actorRemovedCallback, static_cast<TweenSystem*>(this), _1);
    g_game->events()->addSubscription(Callback(this, destr_callback), ActorDestroyedEvent::m_type);
    return true;
}

void TweenSystem::update(float dt)
{
    for(auto i : tween_map) {
        for(auto j : i.second) {
            if(j->playing) {
                j->position += dt * (j->reverse ? -1 : 1);
            }
            while(!j->reverse && j->position > j->length) {
                if(j->repeat)
                    j->position -= j->length;
                else
                    j->position = j->length;
                j->current_transition = 0;
            }
            while(j->reverse && j->position < 0) {
                if(j->repeat)
                    j->position += j->length;
                else
                    j->position = 0;
                j->current_transition = 0;
            }
            while(!j->reverse && j->position < j->length && j->position / j->length > j->transitions[j->current_transition].end) {
                j->current_transition++;
            }
            while(j->reverse && j->position > 0 && j->position / j->length < j->transitions[j->current_transition].start) {
                j->current_transition++;
            }
            if(j->transitions[j->current_transition].start_value - j->transitions[j->current_transition].end_value != 0) {
                Transition t = j->transitions[j->current_transition];
                float start = t.start_value;
                float end = t.end_value;
                float position = (j->position / j->length) - t.start;
                float length = t.end - t.start;
                switch(j->transitions[j->current_transition].type) {
                    case CurveType::NONE:
                        j->current_value = start;
                        break;
                    case CurveType::LINEAR:
                        j->current_value = (position / length) * (end - start) + start;
                    break;
                    case CurveType::EASE_IN:
                        j->current_value = (1 - cos(position / length * 90 * DEGTORAD)) * (end - start) + start;
                    break;
                    case CurveType::EASE_OUT:
                        j->current_value = (sin(position / length * 90 * DEGTORAD)) * (end - start) + start;
                    break;
                    default:
                        j->current_value = start;
                        break;
                }
            } else
                j->current_value = j->transitions[j->current_transition].start_value;
        }
    }
}

void TweenSystem::cleanup(void)
{
}

void TweenSystem::actorRemovedCallback(const IEvent& event)
{
    if(event.getEventType() != ActorDestroyedEvent::m_type) {
        warn("TweenSystem expected ActorDestroyedEvent, but received something else.");
        return;
    }
    const ActorDestroyedEvent* e = dynamic_cast<const ActorDestroyedEvent*>(&event);
    for(auto i : tween_map[e->getId()])
        tweens.erase(i);
    tween_map.erase(e->getId());
}

bool TweenSystem::contains(Tween* tween)
{
    return tweens.find(tween) != tweens.end();
}

int tweenIndex(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the Tween is missing its instance!");
    Tween* tween = *static_cast<Tween**>(lua_touserdata(state, -1));
    lua_pop(state, 1);
    if(!g_game->tweens()->contains(tween))
        return luaL_error(state, "Trying to access data, but the Tween is unregistered!");

    if(!strcmp(lua_tostring(state, 2), "value")) {
        lua_pushnumber(state, tween->current_value);
    } else if(!strcmp(lua_tostring(state, 2), "position")) {
        lua_pushnumber(state, tween->position);
    } else if(!strcmp(lua_tostring(state, 2), "start_value")) {
        lua_pushnumber(state, tween->start_value);
    } else if(!strcmp(lua_tostring(state, 2), "end_value")) {
        lua_pushnumber(state, tween->end_value);
    } else if(!strcmp(lua_tostring(state, 2), "length")) {
        lua_pushnumber(state, tween->length);
    } else if(!strcmp(lua_tostring(state, 2), "repeat")) {
        lua_pushboolean(state, tween->repeat);
    } else if(!strcmp(lua_tostring(state, 2), "reverse")) {
        lua_pushboolean(state, tween->reverse);
    } else if(!strcmp(lua_tostring(state, 2), "playing")) {
        lua_pushboolean(state, tween->playing);
    } else
        return 0;

    return 1;
}

int tweenNewIndex(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the Tween is missing its instance!");
    Tween* tween = *static_cast<Tween**>(lua_touserdata(state, -1));
    lua_pop(state, 1);
    if(!g_game->tweens()->contains(tween))
        return luaL_error(state, "Trying to access data, but the Tween is unregistered!");

    if(!strcmp(lua_tostring(state, 2), "start_value")) {
        tween->start_value = lua_tonumber(state, 3);
        tween->transitions[0].start_value = tween->start_value;
    } else if(!strcmp(lua_tostring(state, 2), "end_value")) {
        tween->end_value = lua_tonumber(state, 3);
        tween->transitions[tween->transitions.size() - 1].end_value = tween->end_value;
    } else if(!strcmp(lua_tostring(state, 2), "length")) {
        tween->length = lua_tonumber(state, 3);
    } else if(!strcmp(lua_tostring(state, 2), "repeat")) {
        tween->repeat = lua_toboolean(state, 3);
    } else if(!strcmp(lua_tostring(state, 2), "playing")) {
        tween->playing = lua_toboolean(state, 3);
    } else if(!strcmp(lua_tostring(state, 2), "reverse")) {
        tween->reverse = lua_toboolean(state, 3);
    } else if(!strcmp(lua_tostring(state, 2), "position")) {
        tween->position = lua_tonumber(state, 3);
    } else
        return 0;

    return 0;
}
