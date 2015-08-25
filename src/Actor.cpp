#include "Actor.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "CRigidBody.h"
#include "CScript.h"
#include "Event.h"
#include "EventSystem.h"
#include "Game.h"
#include "ResourceManager.h"
#include "TweenSystem.h"
#include "Util.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace rapidxml;

ActorConstructionData::ActorConstructionData(void)
{
}

ActorConstructionData::ActorConstructionData(xml_node<>* root_node)
{
    fromXml(root_node);
}

ActorConstructionData::~ActorConstructionData(void)
{
}

bool ActorConstructionData::fromXml(xml_node<>* root_node)
{
    u_root_node = root_node;
    if(xml_attribute<>* type = u_root_node->first_attribute("id", 2, false)) {
        m_type = type->value();
    }
    if(xml_attribute<>* name = u_root_node->first_attribute("name", 4, false)) {
        m_name = name->value();
    }
    attr(u_root_node, "static", &m_static);
    attr(u_root_node, "persistent", &m_persistent);
    if((u_translation_node = u_root_node->first_node("translate", 9, false))) {
        glm::vec3 trans(0, 0, 0);
        attr(u_translation_node, "x", &trans.x);
        attr(u_translation_node, "y", &trans.y);
        attr(u_translation_node, "z", &trans.z);
        m_transform.translate(trans, true);
    }
    if((u_rotation_node = u_root_node->first_node("rotate", 6, false))) {
        glm::vec3 rot(0, 0, 0);
        attr(u_rotation_node, "x", &rot.x);
        attr(u_rotation_node, "y", &rot.y);
        attr(u_rotation_node, "z", &rot.z);
        m_transform.rotate(rot, true);
    }

    return true;
}

StaticActorConstructionData::StaticActorConstructionData(char* text_buffer)
{
    m_text_buffer = text_buffer;
    try {
        m_document.parse<parse_validate_closing_tags>(m_text_buffer);
    } catch(parse_error e) {
        printf("%s: %s\n", e.what(), e.where<char>());
    }
    fromXml(m_document.first_node("actor", 5));
}

void ActorConstructionData::cleanup(void)
{
}

void StaticActorConstructionData::cleanup(void)
{
    delete[] m_text_buffer;
}

Actor::Actor(unsigned long id)
{
    m_id = id;
    m_alive = true;
    m_transform = new Transform();
}

void Actor::initialize(void)
{
    for(auto i : m_components) {
        i->init();
        if(i->getID() == CRIGIDBODY_ID) {
            m_transform->setWorldTransform(((CRigidBody*) i)->getTransform());
        }
    }
}

bool Actor::applyTransform(ActorConstructionData* data, Transform* transform)
{
    if(data->m_type != "") {
        ActorConstructionData* parent_data = g_game->resources()->getActor(data->m_type);
        if(!parent_data) {
            error("Trying to create an Actor with missing superclass data.");
            return false;
        }
        if(!applyTransform(parent_data)) {
            return false;
        }
    }
    (*m_transform) *= data->m_transform;
    if(transform)
        (*m_transform) *= *transform;

    updateTransform();
    return true;
}

bool Actor::applyData(ActorConstructionData* data)
{
    if(data->m_type != "") {
        ActorConstructionData* parent_data = g_game->resources()->getActor(data->m_type);
        if(!parent_data) {
            error("Trying to create an Actor with missing superclass data.");
            return false;
        }
        if(!applyData(parent_data)) {
            return false;
        }
    }

    for(xml_node<>* node = data->u_root_node->first_node(); node != NULL; node = node->next_sibling()) {
        //printf("Found node %s\n", node->name());
        if(node != data->u_translation_node && node != data->u_rotation_node) {
            IComponent* cmp = g_game->components()->buildComponent(node, this);
            if(cmp) {
                addComponent(cmp);
            }
        }
    }

    if(data->m_name != "")
        m_name = data->m_name;

    return true;
}

bool Actor::addComponent(IComponent* component)
{
    if(component->getID() == CSCRIPT_ID)
        m_scripts.insert((CScript*)component);

    if(strlen(component->getName()) > 0) {
        m_named_components[component->getName()] = component;
    }

    if(component->get_has_update())
        m_updating_components.insert(component);

    m_components.insert(component);
    return true;
}

void Actor::update(float delta_time)
{
    for(auto i : m_updating_components)
        i->update(delta_time);

    for(auto i = m_collisions.begin(); i != m_collisions.end();) {
        for(auto j : m_scripts) {
            j->processCollision(i->second, i->first);
            if(i->second == 1)
                j->processCollision(2, i->first);
        }
        if(i->second == 1 || i->second == 2) {
            i->second = 3;
            ++i;
        } else {
            i = m_collisions.erase(i);
        }
    }
}

bool Actor::getAlive(void) const
{
    return m_alive;
}

Transform* Actor::getTransform(void) const
{
    return m_transform;
}

void Actor::updateTransform(void)
{
    btTransform transform;
    m_transform->getWorldTransform(transform);
    glm::vec3 gl_scale = m_transform->getScaling();
    btVector3 scale(gl_scale.x, gl_scale.y, gl_scale.z);
    for(auto i : m_components) {
        if(i->getID() == CRIGIDBODY_ID) {
            ((CRigidBody*) i)->setTransform(transform, scale);
        }
    }
}

unsigned long Actor::getID(void) const
{
    return m_id;
}

void Actor::addCollision(unsigned long other_id)
{
    if(m_static)
        return;
    auto a = m_collisions.find(other_id);
    if(a == m_collisions.end()) {
        m_collisions[other_id] = 1;
    } else {
        a->second = 2;
    }
}

void Actor::_destroy(void)
{
    if(!g_game->isQuitting()) {
        for(auto i : m_components) {
            if(i->getID() == CSCRIPT_ID) {
                ((CScript*)i)->callDestroy();
            }
        }
    }

    ActorDestroyedEvent destroyed_ev(m_id);
    g_game->events()->callEvent(destroyed_ev);
    auto it = m_components.begin();
    while(it != m_components.end()) {
        IComponent* c = *it;
        c->destroy();
        delete c;
        it = m_components.erase(it);
    }
    m_components.clear();
    delete m_transform;

    m_alive = false;
}

void Actor::addForce(btVector3 force, btVector3 vec)
{
    for(auto i : m_components) {
        if(i->getID() == CRIGIDBODY_ID) {
            ((CRigidBody*) i)->addForce(force, vec);
        }
    }
}

void Actor::initTransform(lua_State* state)
{
    int pos = 2;
    glm::mat4 transform(1);
    if(lua_gettop(state) >= 7) {
        if(lua_gettop(state) >= 10) {
            m_transform->scale(glm::vec3(lua_tonumber(state, 8), lua_tonumber(state, 9), lua_tonumber(state, 10)));
        }
        m_transform->rotate(glm::quat(glm::vec3(lua_tonumber(state, 5), lua_tonumber(state, 6), lua_tonumber(state, 7))), true);
    }
    m_transform->translate(glm::vec3(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4)));
    updateTransform();
}

int actor_translate(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to translate, but the Actor is missing its instance!");

    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    glm::vec3 translation(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4));
    bool relative = false;
    if(lua_gettop(state) >= 5)
        relative = lua_toboolean(state, 5);

    actor->getTransform()->translate(translation, relative);
    actor->updateTransform();

    return 0;
}

int actor_rotate(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to rotate, but the Actor is missing its instance!");

    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    glm::vec3 rotation(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4));
    bool relative = false;
    if(lua_gettop(state) >= 5)
        relative = lua_toboolean(state, 5);

    actor->getTransform()->rotate(rotation, relative);
    actor->updateTransform();

    return 0;
}

int actor_scale(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to scale, but the Actor is missing its instance!");

    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);
    glm::vec3 scale(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4));
    bool relative = false;
    if(lua_gettop(state) >= 5)
        relative = lua_toboolean(state, 5);

    actor->getTransform()->scale(scale, relative);
    actor->updateTransform();

    return 0;
}

int actor_destroy(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to destroy, but the Actor is missing its instance!");

    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    actor->destroy();

    lua_pop(state, 1);

    return 0;
}

int actor_apply_force(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to apply a force, but the Actor is missing its instance!");
    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    btVector3 rel(0, 0, 0);
    if(lua_gettop(state) >= 7)
        rel = btVector3(lua_tonumber(state, 5), lua_tonumber(state, 6), lua_tonumber(state, 7));

    actor->addForce(btVector3(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4)), rel);

    return 0;
}

int actor_get_component(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to get a component, but the Actor is missing its instance!");
    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    auto it = actor->m_named_components.find(lua_tostring(state, 2));
    if(it == actor->m_named_components.end()) {
        // TODO: Error here
        warn("Trying to get a component, but the component was not found!");
        return 0;
    }

    lua_newtable(state);
    luaL_setfuncs(state, it->second->getFuncs(), 0);
    IComponent** cmpdat = static_cast<IComponent**>(lua_newuserdata(state, sizeof(IComponent*)));
    *cmpdat = it->second;
    lua_setfield(state, -2, "instance");
    lua_newtable(state);
    luaL_setfuncs(state, it->second->getMetaFuncs(), 0);
    lua_setmetatable(state, -2);

    return 1;
}

int actor_transform(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to get a component, but the Actor is missing its instance!");
    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    // TODO: Get transform here
    Transform* transform = actor->getTransform();
    glm::vec3 position = transform->getPosition();
    glm::vec3 rotation = transform->getRotation();
    glm::vec3 scale = transform->getScaling();
    
    lua_newtable(state);

    lua_newtable(state);
    lua_pushnumber(state, position.x);
    lua_setfield(state, -2, "x");
    lua_pushnumber(state, position.y);
    lua_setfield(state, -2, "y");
    lua_pushnumber(state, position.z);
    lua_setfield(state, -2, "z");
    lua_setfield(state, -2, "position");

    lua_newtable(state);
    lua_pushnumber(state, rotation.x);
    lua_setfield(state, -2, "x");
    lua_pushnumber(state, rotation.y);
    lua_setfield(state, -2, "y");
    lua_pushnumber(state, rotation.z);
    lua_setfield(state, -2, "z");
    lua_setfield(state, -2, "rotation");

    lua_newtable(state);
    lua_pushnumber(state, scale.x);
    lua_setfield(state, -2, "x");
    lua_pushnumber(state, scale.y);
    lua_setfield(state, -2, "y");
    lua_pushnumber(state, scale.z);
    lua_setfield(state, -2, "z");
    lua_setfield(state, -2, "scale");

    return 1;
}

int actor_register_tween(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to register a tween, but the Actor is missing its instance!");
    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    unsigned int actor_id = actor->getID();

    Tween* tween = new Tween();
    if(lua_gettop(state) == 2) {
        lua_getfield(state, 2, "start_value");
        if(lua_isnumber(state, -1))
            tween->start_value = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "end_value");
        if(lua_isnumber(state, -1))
            tween->end_value = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "length");
        if(lua_isnumber(state, -1))
            tween->length = lua_tonumber(state, -1);
        lua_pop(state, 1);
        if(tween->length == 0) {
            warn("Tween length cannot be 0");
            tween->length = 1;
        }
        lua_getfield(state, 2, "offset");
        if(lua_isnumber(state, -1))
            tween->position = lua_tonumber(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "loop");
        if(lua_isboolean(state, -1))
            tween->repeat = lua_toboolean(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "reverse");
        if(lua_isboolean(state, -1))
            tween->reverse = lua_toboolean(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "playing");
        if(lua_isboolean(state, -1))
            tween->playing = lua_toboolean(state, -1);
        lua_pop(state, 1);
        lua_getfield(state, 2, "segments");
        if(lua_istable(state, -1)) {
            lua_pushnil(state);
            while(lua_next(state, -2)) {
                Transition t;
                lua_getfield(state, -1, "position");
                if(lua_isnumber(state, -1)) 
                    t.start = lua_tonumber(state, -1);
                lua_pop(state, 1);
                lua_getfield(state, -1, "curve");
                if(lua_isstring(state, -1)) {
                    const char* str = lua_tostring(state, -1);
                    if(!strcmp(str, "none")) {
                        t.type = CurveType::NONE;
                    } else if(!strcmp(str, "linear"))
                        t.type = CurveType::LINEAR;
                    else if(!strcmp(str, "ease_in"))
                        t.type = CurveType::EASE_IN;
                    else if(!strcmp(str, "ease_out"))
                        t.type = CurveType::EASE_OUT;
                }
                lua_pop(state, 1);
                lua_getfield(state, -1, "value");
                if(lua_isnumber(state, -1))
                    t.start_value = lua_tonumber(state, -1);
                lua_pop(state, 1);
                if(tween->transitions.size() > 0) {
                    tween->transitions[tween->transitions.size() - 1].end = t.start;
                    tween->transitions[tween->transitions.size() - 1].end_value = t.start_value;
                }
                tween->transitions.push_back(t);
                lua_pop(state, 1);
            }
        }
        lua_pop(state, 1);
    }

    if(tween->transitions.size() == 0)
        tween->transitions.push_back(Transition());
    tween->transitions[0].start_value = tween->start_value;
    tween->transitions[tween->transitions.size() - 1].end_value = tween->end_value;

    lua_newtable(state);
    Tween** dat = static_cast<Tween**>(lua_newuserdata(state, sizeof(Tween*)));
    *dat = tween;
    lua_setfield(state, -2, "instance");
    lua_newtable(state);
    luaL_setfuncs(state, tweenMeta, 0);
    lua_setmetatable(state, -2);

    g_game->tweens()->tween_map[actor_id].push_back(tween);
    g_game->tweens()->tweens.insert(tween);
    return 1;
}

int actor_get_tween(lua_State* state)
{
    return 1;
}

int actor_index(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the Actor is missing its instance!");
    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    if(!strcmp(lua_tostring(state, 2), "transform")) {
        lua_newtable(state);
        Transform** t = static_cast<Transform**>(lua_newuserdata(state, sizeof(Transform*)));
        *t = actor->getTransform();
        lua_setfield(state, -2, "instance");
        lua_newtable(state);
        luaL_setfuncs(state, transform_meta, 0);
        lua_setmetatable(state, -2);
    } else
        return 0;
    return 1;
}

int actor_newindex(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to access data, but the Actor is missing its instance!");
    Actor* actor = *static_cast<Actor**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    if(!strcmp(lua_tostring(state, 2), "transform")) {
        lua_settop(state, 3);
        actor->m_transform->setWorldTransform(state);
    }
    return 0;
}
