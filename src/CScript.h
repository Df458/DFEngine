#ifndef COMPONENT_SCRIPT_H
#define COMPONENT_SCRIPT_H
#include "Component.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <rapidxml.hpp>

class Actor;
#define CSCRIPT_ID 3

IComponent* buildScript(rapidxml::xml_node<>* node, Actor* actor);
int cscriptIndex(lua_State* state);
int cscriptNewIndex(lua_State* state);

const luaL_Reg cscript_funcs[] =
{
    {0, 0}
};

const luaL_Reg cscript_meta[] =
{
    {"__index", cscriptIndex},
    {"__newindex", cscriptNewIndex},
    {0, 0}
};

class CScript : public IComponent
{
public:
    virtual void init(void);
    virtual void destroy(void);
    virtual void update(float delta_time);
    virtual ComponentID getID(void);
    virtual void processCollision(char collision_type, unsigned long other_id);
    virtual const luaL_Reg* getFuncs(void) const { return cscript_funcs; }
    virtual const luaL_Reg* getMetaFuncs(void) const { return cscript_meta; }

    friend IComponent* buildScript(rapidxml::xml_node<>* node, Actor* actor);
    friend int cscriptIndex(lua_State* state);
    friend int cscriptNewIndex(lua_State* state);
protected:
    lua_State* m_state;
    Actor** m_other_actor;
};

#endif
