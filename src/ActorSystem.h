#ifndef ACTOR_SYSTEM_H
#define ACTOR_SYSTEM_H
#include "System.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <map>
#include <set>

class Actor;
class ActorConstructionData;
class Transform;

class ActorSystem : public ISystem
{
public:
    ActorSystem(void);
    bool initialize(void);
    void step(float delta_time);
    void cleanup(void);
    void clear(void);
    void softClear(void);

    Actor* getActor(unsigned long id) const;
    Actor* getActor(const char* name) const;
    Actor* getLastActor() const;
    Actor* createActor(ActorConstructionData* actor_data, Transform* transform = NULL);
    Actor* createActor(std::string name, Transform* transform = NULL);
    Actor* createActor(lua_State* state);
    bool exists(unsigned long id) const;

    void initTransform(lua_State* state);
private:
    std::map<unsigned long, Actor*> m_actors;
    std::set<Actor*> m_new_actors;
    unsigned long last_id = 0;
    Actor* u_last_actor = 0;
    bool m_soft_clearing = false;
};

#endif
