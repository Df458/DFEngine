#include "Actor.h"
#include "CRigidBody.h"
#include "EventSystem.h"
#include "Game.h"
#include "PhysicsMaterial.h"
#include "PhysicsSystem.h"
#include "ResourceManager.h"
#include "Util.h"

#include <algorithm>
#include <string>

using namespace rapidxml;

IComponent* buildRigidBody(xml_node<>* node, Actor* actor)
{
    xml_node<>* shape_node = node->first_node("shape", 5, false);
    xml_node<>* mat_node = node->first_node("material", 8, false);
    PhysicsMaterial mat;
    xml_attribute<>* shape_id = NULL;
    btCollisionShape* shape = NULL;
    btRigidBody* rigid_body = NULL;
    if(!shape_node || !(shape_id = shape_node->first_attribute("type", 4, false))) {
        error("Trying to create a rigid body with no shape.");
        return NULL;
    }

    std::string id_str = shape_id->value();
    std::transform(id_str.begin(), id_str.end(), id_str.begin(), ::tolower);
    if(id_str == "box") {
        btVector3 dims(0, 0, 0);
        if(xml_attribute<>* xa = shape_node->first_attribute("x", 1, false))
            dims.setX(atof(xa->value()));
        if(xml_attribute<>* ya = shape_node->first_attribute("y", 1, false))
            dims.setY(atof(ya->value()));
        if(xml_attribute<>* za = shape_node->first_attribute("z", 1, false))
            dims.setZ(atof(za->value()));
        shape = new btBoxShape(dims);
    } else if(id_str == "sphere") {
        float radius = 0;
        if(xml_attribute<>* ra = shape_node->first_attribute("radius", 6, false))
            radius = atof(ra->value());
        shape = new btSphereShape(radius);
    } else if(id_str == "capsule") {
        float radius = 0;
        float height = 1;
        if(xml_attribute<>* ra = shape_node->first_attribute("radius", 6, false))
            radius = atof(ra->value());
        if(xml_attribute<>* ha = shape_node->first_attribute("height", 6, false))
            radius = atof(ha->value());
        shape = new btCapsuleShape(radius, height);
    } else if(id_str == "cone") {
        float radius = 0;
        float height = 1;
        if(xml_attribute<>* ra = shape_node->first_attribute("radius", 6, false))
            radius = atof(ra->value());
        if(xml_attribute<>* ha = shape_node->first_attribute("height", 6, false))
            radius = atof(ha->value());
        shape = new btConeShape(radius, height);
    } else if(id_str == "mesh") {
        warn("Mesh collision shape generation is unimplemented!");
    }

    if(!shape) {
        error("Couldn't create rigidbody shape.");
        return NULL;
    }

    if(mat_node)
        if(xml_attribute<>* id = mat_node->first_attribute("id", 2, false))
            mat = g_game->resources()->getPhysicsMaterial(id->value());

    float mass = 0;
    if(xml_attribute<>* ma = node->first_attribute("mass", 4, false))
        mass = atof(ma->value()) * mat.mass;
    btVector3 inertia(0, 0, 0);
    shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo cinfo = btRigidBody::btRigidBodyConstructionInfo(mass, actor->getTransform(), shape, inertia);
    cinfo.m_restitution = mat.restitution;
    cinfo.m_friction = mat.sliding_friction;
    cinfo.m_rollingFriction = mat.rolling_friction;
    rigid_body = new btRigidBody(cinfo);

    CRigidBody* component = new CRigidBody();
    component->m_body = rigid_body;
    rigid_body->setUserPointer(actor);
    CRigidBodyCreatedEvent created_ev(rigid_body, actor->getID());
    g_game->events()->callEvent(created_ev);

    return component;
}

const EventType CRigidBodyCreatedEvent::m_type(2);

const EventType& CRigidBodyCreatedEvent::getEventType(void) const
{
    return m_type;
}

CRigidBodyCreatedEvent::CRigidBodyCreatedEvent(btRigidBody* body, unsigned long id)
{
    m_id = id;
    u_body = body;
}

unsigned long CRigidBodyCreatedEvent::getId(void) const
{
    return m_id;
}

btRigidBody* CRigidBodyCreatedEvent::getBody(void) const
{
    return u_body;
}

void CRigidBody::init(void)
{
}

void CRigidBody::destroy(void)
{
    delete m_body->getCollisionShape();
    delete m_body;
}

void CRigidBody::update(float delta_time)
{
}

void CRigidBody::setTransform(const btTransform& transform, const btVector3& scale)
{
    m_body->setWorldTransform(transform);
    if(m_last_scale != scale) {
        m_last_scale = scale;
        m_body->getCollisionShape()->setLocalScaling(scale);
        g_game->physics()->updateAABB(m_body);
    }
}

void CRigidBody::addForce(btVector3 force, btVector3 vec)
{
    m_body->applyForce(force, vec);
    m_body->setActivationState(ACTIVE_TAG);
}

ComponentID CRigidBody::getID(void) 
{
    return CRIGIDBODY_ID;
}

int crigidbody_velocity(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access velocity, but the Rigid Body Component is missing its instance!");
    CRigidBody* body = *static_cast<CRigidBody**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    btVector3 vec = body->m_body->getLinearVelocity();
    if(lua_gettop(state) >= 4) {
        bool relative = false;
        if(lua_gettop(state) >= 5)
            relative = lua_toboolean(state, 5);
        btVector3 vel(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4));
        if(relative)
            vel += vec; 
        body->m_body->setLinearVelocity(vel);
        body->m_body->setActivationState(ACTIVE_TAG);
        return 0;
    }

    lua_newtable(state);
    lua_pushnumber(state, vec.x());
    lua_setfield(state, -2, "x");
    lua_pushnumber(state, vec.y());
    lua_setfield(state, -2, "y");
    lua_pushnumber(state, vec.z());
    lua_setfield(state, -2, "z");
    return 1;
}

int crigidbody_angular_velocity(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access angular velocity, but the Rigid Body Component is missing its instance!");
    CRigidBody* body = *static_cast<CRigidBody**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    btVector3 vec = body->m_body->getAngularVelocity();
    if(lua_gettop(state) >= 4) {
        bool relative = false;
        if(lua_gettop(state) >= 5)
            relative = lua_toboolean(state, 5);
        btVector3 vel(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4));
        if(relative)
            vel += vec; 
        body->m_body->setAngularVelocity(vel);
        body->m_body->setActivationState(ACTIVE_TAG);
        return 0;
    }

    lua_newtable(state);
    lua_pushnumber(state, vec.x());
    lua_setfield(state, -2, "x");
    lua_pushnumber(state, vec.y());
    lua_setfield(state, -2, "y");
    lua_pushnumber(state, vec.z());
    lua_setfield(state, -2, "z");
    return 1;
}
