#ifndef ACTOR_H
#define ACTOR_H

#include "ActorSystem.h"
#include "Component.h"
#include "Transform.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <rapidxml.hpp>
#include <set>
#include <unordered_map>


class ActorConstructionData;
class IComponent;
class CScript;

class Actor
{
public:
    Actor(unsigned long id);
    bool applyTransform(ActorConstructionData* data, Transform* transform = NULL);
    bool applyData(ActorConstructionData* data);
    bool addComponent(IComponent* component);
    void initialize(void);
    void update(float delta_time);
    bool getAlive(void) const;
    bool getPersistent(void) const { return m_persistent; }
    void setPersistent(bool persist) { m_persistent = persist; }
    Transform* getTransform(void) const;
    void updateTransform(void);
    unsigned long getID(void) const;
    inline void destroy(void) { m_alive = false; }
    void addCollision(unsigned long other_id);
    void addForce(btVector3 force, btVector3 vec);
    void initTransform(lua_State* state);
    bool isStatic(void) const { return m_static; }
    bool isPersistent(void) const { return m_persistent; }
    std::string getName(void) const { return m_name; }
    void setName(std::string name) { m_name = name; }

    friend class ActorSystem;
    friend int actor_get_component(lua_State* state);
protected:
    std::string m_type;
    std::set<IComponent*> m_components;
    std::set<CScript*> m_scripts;
    std::unordered_map<std::string, IComponent*> m_named_components;
    //std::map<ComponentID, IComponent*> m_components;
    std::unordered_map<unsigned long, char> m_collisions;
    unsigned long m_id;
    bool m_alive;
    bool m_static = false;
    bool m_persistent = false;
    Transform* m_transform;
    std::string m_name = "";

    void _destroy(void);
};

class ActorConstructionData
{
public:
    ActorConstructionData(void);
    ActorConstructionData(rapidxml::xml_node<>* root_node);
    virtual ~ActorConstructionData(void);
    virtual void cleanup(void);
    inline bool isStatic(void) const { return m_static; }
    inline bool isPersistent(void) const { return m_persistent; }
    std::string getName(void) const { return m_name; }
    void setName(std::string name) { m_name = name; }

    friend bool Actor::applyData(ActorConstructionData* data);
    friend bool Actor::applyTransform(ActorConstructionData* data, Transform* transform);
protected:
    virtual bool constructFromXml(rapidxml::xml_node<>* root_node);
    Transform m_transform;
    rapidxml::xml_node<>* u_root_node;
    rapidxml::xml_node<>* u_translation_node;
    rapidxml::xml_node<>* u_rotation_node;
    std::string m_type;
    bool m_static = false;
    bool m_persistent = false;
    std::string m_name = "";
};

class StaticActorConstructionData : public ActorConstructionData
{
public:
    StaticActorConstructionData(char* text_buffer);
    virtual void cleanup(void);

private:
    char* m_text_buffer;
    rapidxml::xml_document<> m_document;
};

int actor_translate(lua_State* state);
int actor_rotate(lua_State* state);
int actor_scale(lua_State* state);
int actor_destroy(lua_State* state);
int actor_apply_force(lua_State* state); // TODO: Move this to CRigidbody
int actor_get_component(lua_State* state);
int actor_transform(lua_State* state);

const luaL_Reg actor_funcs[] =
{
    {"translate", actor_translate},
    {"rotate", actor_rotate},
    {"scale", actor_scale},
    {"apply_force", actor_apply_force},
    {"get_component", actor_get_component},
    {"transform", actor_transform},
    {"destroy", actor_destroy},
    {0, 0}
};

#endif
