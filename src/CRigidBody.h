#ifndef C_RIGID_BODY_H
#define C_RIGID_BODY_H
#include "Component.h"
#include "Event.h"

#include <rapidxml.hpp>

class Actor;

#define CRIGIDBODY_ID 0

IComponent* buildRigidBody(rapidxml::xml_node<>* node, Actor* actor);

int crigidbody_Index(lua_State* state);
int crigidbody_NewIndex(lua_State* state);

const luaL_Reg crigidbody_funcs[] =
{
    {0, 0}
};

const luaL_Reg crigidbody_meta[] =
{
    {"__index", crigidbody_Index},
    {"__newindex", crigidbody_NewIndex},
    {0, 0}
};

class CRigidBody : public IComponent
{
public:
    virtual void init(void);
    virtual void destroy(void);
    virtual void update(float delta_time);
    virtual void setTransform(const btTransform& transform, const btVector3& scale);
    virtual const btTransform& getTransform(void) const { return m_body->getWorldTransform(); }
    virtual void addForce(btVector3 force, btVector3 vec);
    virtual ComponentID getID(void);
    virtual const luaL_Reg* getFuncs(void) const { return crigidbody_funcs; }
    virtual const luaL_Reg* getMetaFuncs(void) const { return crigidbody_meta; }
    virtual bool get_has_update(void) const { return false; }

    friend IComponent* buildRigidBody(rapidxml::xml_node<>* node, Actor* actor);
    friend int crigidbody_Index(lua_State* state);
    friend int crigidbody_NewIndex(lua_State* state);
protected:
    btRigidBody* m_body;
    btVector3 m_last_scale;
    int m_mask = -1;
    int m_group = -1;
};

class CRigidBodyCreatedEvent : public IEvent
{
    public:
        CRigidBodyCreatedEvent(btRigidBody* body, unsigned long id);
        virtual const EventType& getEventType (void) const;
        btRigidBody* getBody(void) const;
        unsigned long getId(void) const;

        static const EventType m_type;
    private:
        btRigidBody* u_body;
        unsigned long m_id;
};

#endif
