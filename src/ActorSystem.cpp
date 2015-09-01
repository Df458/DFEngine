#include "Actor.h"
#include "ActorSystem.h"
#include "Game.h"
#include "ResourceManager.h"
#include "Transform.h"
#include "Util.h"
#include <climits>

ActorSystem::ActorSystem(void)
{
}

bool ActorSystem::initialize(void)
{
    return true;
}

void ActorSystem::update(float delta_time)
{
    if(m_new_actors.size() != 0) {
        for(auto i : m_new_actors) {
            i->initialize();
            m_actors[i->getID()] = i;
        }
        m_new_actors.clear();
    }

    for(auto i = m_actors.begin(); i != m_actors.end();) {
        Actor* actor = i->second;
        if(!actor->isStatic() && !m_soft_clearing && actor->getAlive())
            actor->update(delta_time);
        if(!actor->getAlive()) {
            i = m_actors.erase(i);
            actor->_destroy();
            delete actor;
            continue;
        }
        ++i;
    }
    m_soft_clearing = false;
}

void ActorSystem::cleanup(void)
{
    clear();
}

void ActorSystem::clear(void)
{
    for(auto i : m_actors) {
        Actor* actor = i.second;
        actor->_destroy();
        delete actor;
    }
    m_actors.clear();
}

void ActorSystem::softClear(void)
{
    for(auto i : m_actors) {
        Actor* actor = i.second;
        if(!actor->isPersistent())
            actor->destroy();
    }
    m_soft_clearing = true;
}
 
Actor* ActorSystem::getActor(unsigned long id) const
{
    if(m_actors.find(id) == m_actors.end()) {
        //warn("Requesting an Actor that doesn't exist. (Actor id " + std::to_string(id) + ")");
        return NULL;
    }

    return m_actors.at(id);
}

Actor* ActorSystem::getActor(const char* name) const
{
    for(auto i = m_actors.begin(); i != m_actors.end(); ++i) {
        if(i->second->getName() == name)
            return i->second;
    }

    //warn("Requesting an Actor that doesn't exist.");
    //fprintf(stderr, "Actor name: %s\n", name);
    return NULL;
}

std::vector<Actor*> ActorSystem::getActors(const char* name) const
{
    std::vector<Actor*> actors;
    for(auto i = m_actors.begin(); i != m_actors.end(); ++i) {
        if(i->second->getName() == name)
            actors.push_back(i->second);
    }

    return actors;
}

Actor* ActorSystem::getLastActor() const
{
    if(m_actors.find(last_id - 1) == m_actors.end()) {
        warn("Last Actor doesn't exist.");
        return NULL;
    }

    return m_actors.at(last_id - 1);
}

Actor* ActorSystem::createActor(ActorConstructionData* data, Transform* transform)
{
    if(!data) {
        error("Trying to construct a null Actor.");
        return 0;
    } else if(last_id == ULONG_MAX) {
        error("Either the application memory is corrupted, or you've been playing for waaaaay too long.");
        return 0;
    }
    Actor* actor = new Actor(last_id);
    actor->m_id = last_id;
    actor->m_static = data->isStatic();
    actor->m_persistent = data->isPersistent();
    if(!actor->applyTransform(data, transform)) {
        // TODO: Fix that
    }

    if(!actor->applyData(data)) {
        warn("Could not fully construct Actor from existing data.");
    }
    m_new_actors.insert(actor);
    ++last_id;
    u_last_actor = actor;
    return actor;
}

Actor* ActorSystem::createActor(std::string name, Transform* transform)
{
    if(last_id == ULONG_MAX) {
        error("Either the application memory is corrupted, or you've been playing for waaaaay too long.");
        return 0;
    }
    ActorConstructionData* data = g_game->resources()->getActor(name);
    return createActor(data, transform);
}

Actor* ActorSystem::createActor(lua_State* state)
{
    if(last_id == ULONG_MAX) {
        error("Either the application memory is corrupted, or you've been playing for waaaaay too long.");
        return 0;
    }
    int arg_count = lua_gettop(state);
    Transform* transform = NULL;
    if(arg_count >= 2) {
        lua_settop(state, 2);
        transform = new Transform(state);
    }
    Actor* actor = createActor(lua_tostring(state, 1), transform);
    if(!actor)
        return 0;

    if(arg_count >= 4)
        actor->initTransform(state);

    return actor;
}

bool ActorSystem::exists(unsigned long id) const
{
    return m_actors.find(id) != m_actors.end();
}
