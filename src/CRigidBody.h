#ifndef C_RIGID_BODY_H
#define C_RIGID_BODY_H
#include "Component.h"
#include "Event.h"

#include <rapidxml.hpp>

class Actor;

#define CRIGIDBODY_ID 0

IComponent* buildRigidBody(rapidxml::xml_node<>* node, Actor* actor);

int crigidbody_velocity(lua_State* state);
int crigidbody_angular_velocity(lua_State* state);

const luaL_Reg crigidbody_funcs[] =
{
    {"velocity", crigidbody_velocity},
    {"angular_velocity", crigidbody_angular_velocity},
    {0, 0}
};

class CRigidBody : public IComponent
{
public:
    virtual void init(void);
    virtual void destroy(void);
    virtual void update(float delta_time);
    virtual void setTransform(const btTransform& transform, const btVector3& scale);
    virtual void addForce(btVector3 force, btVector3 vec);
    virtual ComponentID getID(void);
    virtual const luaL_Reg* getFuncs(void) const { return crigidbody_funcs; }

    friend IComponent* buildRigidBody(rapidxml::xml_node<>* node, Actor* actor);
    friend int crigidbody_velocity(lua_State* state);
    friend int crigidbody_angular_velocity(lua_State* state);
protected:
    btRigidBody* m_body;
    btVector3 m_last_scale;
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
