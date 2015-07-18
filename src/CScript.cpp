#include "Actor.h"
#include "AudioSystem.h"
#include "CScript.h"
#include "Game.h"
#include "InputSystem.h"
#include "ResourceManager.h"
#include "Util.h"

using namespace rapidxml;

const char* collision_strs[] =
{
    "collision_enter",
    "collision_tick",
    "collision_leave",
};

IComponent* buildScript(xml_node<>* node, Actor* actor)
{
    CScript* component = new CScript();
    component->m_state = luaL_newstate();
    luaL_openlibs(component->m_state);
    if(xml_attribute<>* attr = node->first_attribute("init")) {
        luaL_loadstring(component->m_state, g_game->resources()->getScript(attr->value()));
        lua_setglobal(component->m_state, "init");
    }

    if(xml_attribute<>* attr = node->first_attribute("update")) {
        luaL_loadstring(component->m_state, g_game->resources()->getScript(attr->value()));
        lua_setglobal(component->m_state, "update");
    }

    if(xml_attribute<>* attr = node->first_attribute("collision_enter")) {
        luaL_loadstring(component->m_state, g_game->resources()->getScript(attr->value()));
        lua_setglobal(component->m_state, "collision_enter");
    }

    if(xml_attribute<>* attr = node->first_attribute("collision_tick")) {
        luaL_loadstring(component->m_state, g_game->resources()->getScript(attr->value()));
        lua_setglobal(component->m_state, "collision_tick");
    }

    if(xml_attribute<>* attr = node->first_attribute("collision_leave")) {
        luaL_loadstring(component->m_state, g_game->resources()->getScript(attr->value()));
        lua_setglobal(component->m_state, "collision_leave");
    }

    for(xml_node<>* in = node->first_node("int", 3, false); in; in = in->next_sibling("int", 3, 0))
        if(xml_attribute<>* na = in->first_attribute("name", 4, false))
            if(xml_attribute<>* va = in->first_attribute("value", 5, false)) {
                lua_pushinteger(component->m_state, atoi(va->value()));
                lua_setglobal(component->m_state, na->value());
            }
    for(xml_node<>* in = node->first_node("number", 6, false); in; in = in->next_sibling("number", 6, 0))
        if(xml_attribute<>* na = in->first_attribute("name", 4, false))
            if(xml_attribute<>* va = in->first_attribute("value", 5, false)) {
                lua_pushnumber(component->m_state, atof(va->value()));
                lua_setglobal(component->m_state, na->value());
            }
    for(xml_node<>* in = node->first_node("string", 6, false); in; in = in->next_sibling("string", 6, 0))
        if(xml_attribute<>* na = in->first_attribute("name", 4, false))
            if(xml_attribute<>* va = in->first_attribute("value", 5, false)) {
                lua_pushstring(component->m_state, va->value());
                lua_setglobal(component->m_state, na->value());
            }
    for(xml_node<>* in = node->first_node("bool", 4, false); in; in = in->next_sibling("bool", 4, 0))
        if(xml_attribute<>* na = in->first_attribute("name", 4, false))
            if(xml_attribute<>* va = in->first_attribute("value", 5, false)) {
                lua_pushboolean(component->m_state, strcmp(va->value(), "false"));
                lua_setglobal(component->m_state, na->value());
            }

    lua_newtable(component->m_state);
    luaL_setfuncs(component->m_state, actor_funcs, 0);
    Actor** actordat = static_cast<Actor**>(lua_newuserdata(component->m_state, sizeof(Actor*)));
    *actordat = actor;
    lua_setfield(component->m_state, 1, "instance");
    lua_setglobal(component->m_state, "this");

    lua_newtable(component->m_state);
    luaL_setfuncs(component->m_state, actor_funcs, 0);
    component->m_other_actor = static_cast<Actor**>(lua_newuserdata(component->m_state, sizeof(Actor*)));
    lua_setfield(component->m_state, 1, "instance");
    lua_setglobal(component->m_state, "other");

    lua_newtable(component->m_state);
    luaL_setfuncs(component->m_state, game_funcs, 0);
    lua_setglobal(component->m_state, "game");

    lua_newtable(component->m_state);
    luaL_setfuncs(component->m_state, input_funcs, 0);
    lua_setglobal(component->m_state, "input");

    lua_newtable(component->m_state);
    luaL_setfuncs(component->m_state, audio_funcs, 0);
    lua_setglobal(component->m_state, "audio");

    lua_pushinteger(component->m_state, 3);
    lua_setglobal(component->m_state, "KEY_UP");
    lua_pushinteger(component->m_state, 2);
    lua_setglobal(component->m_state, "KEY_DOWN");
    lua_pushinteger(component->m_state, 0);
    lua_setglobal(component->m_state, "KEY_RELEASED");
    lua_pushinteger(component->m_state, 1);
    lua_setglobal(component->m_state, "KEY_PRESSED");

    return component;
}

void CScript::init(void)
{
    lua_getglobal(m_state, "init");
    if(!lua_isfunction(m_state, -1)) {
        lua_pop(m_state, 1);
    } else if(lua_pcall(m_state, 0, 0, 0)) {
        warn(lua_tostring(m_state, -1));
    }
}

void CScript::destroy(void)
{
    lua_close(m_state);
}

void CScript::update(float delta_time)
{
    lua_getglobal(m_state, "update");
    if(!lua_isfunction(m_state, -1))
        lua_pop(m_state, 1);
    else {
        lua_pushnumber(m_state, delta_time);
        if(lua_pcall(m_state, 1, 0, 0)) {
            warn(lua_tostring(m_state, -1));
        }
    }
}

ComponentID CScript::getID(void)
{
    return CSCRIPT_ID;
}

void CScript::processCollision(char collision_type, unsigned long other_id)
{
    *m_other_actor = g_game->actors()->getActor(other_id);
    lua_getglobal(m_state, collision_strs[collision_type - 1]);
    if(!lua_isfunction(m_state, -1))
        lua_pop(m_state, 1);
    else {
        lua_pushinteger(m_state, other_id);
        if(lua_pcall(m_state, 1, 0, 0)) {
            warn(lua_tostring(m_state, -1));
        }
    }
}
