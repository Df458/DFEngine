#include "Actor.h"
#include "CRigidBody.h"
#include "Event.h"
#include "EventSystem.h"
#include "Game.h"
#include "PhysicsSystem.h"
#include "Util.h"

#include <bullet/BulletCollision/CollisionShapes/btShapeHull.h>
#include <functional>

using namespace std::placeholders;

static void physicsTickCallback(btDynamicsWorld* world, btScalar dt)
{
    int manifold_count = world->getDispatcher()->getNumManifolds();
    for(int i = 0; i < manifold_count; ++i) {
        btPersistentManifold* manifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject* o1 = manifold->getBody0();
        const btCollisionObject* o2 = manifold->getBody1();
        Actor* a1 = (Actor*)o1->getUserPointer();
        Actor* a2 = (Actor*)o2->getUserPointer();

        a1->addCollision(a2->getID());
        a2->addCollision(a1->getID());
    }
}

PhysicsSystem::PhysicsSystem(void)
{
}

PhysicsSystem::~PhysicsSystem(void)
{
}

bool PhysicsSystem::initialize()
{
    m_broadphase = new btDbvtBroadphase();
    m_config = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_config);
    m_solver = new btSequentialImpulseConstraintSolver();
    m_physics_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_config);
    m_physics_world->setGravity(btVector3(0, -9.8, 0));
    m_physics_world->setInternalTickCallback(physicsTickCallback);

    auto destroyed_callback = std::bind(&PhysicsSystem::actorRemovedCallback, this, _1);
    g_game->events()->addSubscription(Callback(this, destroyed_callback), ActorDestroyedEvent::m_type);
    auto created_callback = std::bind(&PhysicsSystem::CRigidBodyCreatedCallback, this, _1);
    g_game->events()->addSubscription(Callback(this, created_callback), CRigidBodyCreatedEvent::m_type);
    return true;
}

void PhysicsSystem::step(float delta_time)
{
    m_physics_world->stepSimulation(delta_time);
    m_physics_world->debugDrawWorld();
}

void PhysicsSystem::cleanup(void)
{
    delete m_physics_world;
    delete m_solver;
    delete m_dispatcher;
    delete m_config;
    delete m_broadphase;
}

void PhysicsSystem::actorRemovedCallback(const IEvent& event)
{
    if(event.getEventType() != ActorDestroyedEvent::m_type) {
        warn("PhysicsSystem expected ActorDestroyedEvent, but received something else.");
        return;
    }
    const ActorDestroyedEvent* e = static_cast<const ActorDestroyedEvent*>(&event);

    if(u_rigid_bodies.find(e->getId()) != u_rigid_bodies.end()) {
        m_physics_world->removeRigidBody(u_rigid_bodies.at(e->getId()));
        u_rigid_bodies.erase(e->getId());
    }
}

void PhysicsSystem::CRigidBodyCreatedCallback(const IEvent& event)
{
    if(event.getEventType() != CRigidBodyCreatedEvent::m_type) {
        warn("PhysicsSystem expected CRigidbodyCreatedEvent, but received something else.");
        return;
    }
    const CRigidBodyCreatedEvent* e = dynamic_cast<const CRigidBodyCreatedEvent*>(&event);
    m_physics_world->addRigidBody(e->getBody());
    u_rigid_bodies.emplace(e->getId(), e->getBody());
}
