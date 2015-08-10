#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "PhysicsRenderer.h"
#include "System.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <glm/vec3.hpp>
#include <map>

class Actor;
class IEvent;

class IPhysics
{
public:
    virtual ~IPhysics(void) = 0;
    virtual void step(float delta_time) = 0;
};

inline IPhysics::~IPhysics() {}

class PhysicsSystem : public IPhysics, public ISystem
{
public:
    PhysicsSystem(void);
    virtual ~PhysicsSystem(void);
    bool initialize(void);
    void step(float delta_time);
    void cleanup(void);
    void setPhysicsDebug(PhysicsRenderer* pr) { u_physics_debug = pr; m_physics_world->setDebugDrawer(u_physics_debug); }
    inline void setGravity(glm::vec3 gravity) { m_physics_world->setGravity(btVector3(gravity.x, gravity.y, gravity.z)); }
    inline void updateAABB(btRigidBody* body) { m_physics_world->updateSingleAabb(body); }
    inline float getWorldScale(void) const { return m_world_scale; }
    inline void setWorldScale(float world_scale) { m_world_scale = world_scale; }

private:
    virtual void CRigidBodyCreatedCallback(const IEvent& event);
    virtual void actorRemovedCallback(const IEvent& event);
    btBroadphaseInterface* m_broadphase;
    btDefaultCollisionConfiguration* m_config;
    btCollisionDispatcher* m_dispatcher;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_physics_world;
    std::map<unsigned long, btRigidBody*> u_rigid_bodies;
    PhysicsRenderer* u_physics_debug;
    float m_world_scale = 1;
};

#endif
