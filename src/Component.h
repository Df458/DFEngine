#ifndef COMPONENT_H
#define COMPONENT_H

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <btBulletDynamicsCommon.h>
#include <string>

typedef unsigned int ComponentID;

class Actor;

class IComponent
{
public:
    virtual ~IComponent(void) = 0;
    virtual void init(void) = 0;
    virtual void destroy(void) = 0;
    virtual void update(float delta_time) = 0;
    virtual ComponentID getID(void) = 0;
    inline const char* getName(void) const { return m_name; }
    inline void setName(const char* name) { m_name = name; }
    virtual const luaL_Reg* getFuncs(void) const = 0;
    virtual const luaL_Reg* getMetaFuncs(void) const = 0;
    virtual bool get_has_update(void) const = 0;
protected:
    Actor* u_owner;
private:
    void setOwner(Actor* owner);
    const char* m_name = "";
};
inline IComponent::~IComponent() {}

#endif
