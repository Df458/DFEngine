#include "Actor.h"
#include "CGraphics.h"
#include "EventSystem.h"
#include "Font.h"
#include "Game.h"
#include "Model.h"
#include "SceneNode.h"
#include "Shader.h"
#include "Util.h"

#include <string>

using namespace rapidxml;

IComponent* buildGraphics(rapidxml::xml_node<>* node, Actor* actor)
{
    bool updates = false;

    ISceneNode* scene_node = NULL;
    for(xml_node<>* gfx_node = node->first_node(); gfx_node; gfx_node = gfx_node->next_sibling()) {
        std::string id_str = gfx_node->name();
        if(id_str == "model") {
            if(scene_node) {
                warn("Trying to create a Graphics Component with multiple nodes");
                continue;
            }
            scene_node = new ModelSceneNode();
        } else if(id_str == "sprite") {
            if(scene_node) {
                warn("Trying to create a Graphics Component with multiple nodes");
                continue;
            }
            scene_node = new BillboardSceneNode();
        } else if(id_str == "emitter") {
            if(scene_node) {
                warn("Trying to create a Graphics Component with multiple nodes");
                continue;
            }
            scene_node = new ParticleSceneNode();
            updates = true;
        } else if(id_str == "text") {
            if(scene_node) {
                warn("Trying to create a Graphics Component with multiple nodes");
                continue;
            }
            scene_node = new TextSceneNode();
        } else if(id_str == "light") {
            if(scene_node) {
                warn("Trying to create a Graphics Component with multiple nodes");
                continue;
            }
            scene_node = new LightSceneNode(gfx_node);
        }
    }

    if(!scene_node) {
        scene_node = new SceneNode();
    }

    scene_node->setTransform(actor->getTransform());
    scene_node->fromXml(node->first_node());

    CGraphics* component = new CGraphics();
    component->m_node = scene_node;
    component->m_updates = updates;
    CGraphicsCreatedEvent created_ev(scene_node, actor->getID());
    g_game->events()->callEvent(created_ev);

    return component;
}

IComponent* buildCamera(rapidxml::xml_node<>* node, Actor* actor)
{
    CameraSceneNode* scene_node = new CameraSceneNode();

    scene_node->setTransform(actor->getTransform());
    scene_node->fromXml(node);

    CCamera* component = new CCamera();
    component->m_node = scene_node;
    CGraphicsCreatedEvent created_ev(scene_node, actor->getID());
    g_game->events()->callEvent(created_ev);

    return component;
}

void CGraphics::init(void)
{
}

void CGraphics::destroy(void)
{
    delete m_node;
}

void CGraphics::update(float delta_time)
{
    if(m_updates) {
        ((UpdatingSceneNode*)m_node)->update(delta_time);
    }
}

ComponentID CGraphics::getID(void)
{
    return CGRAPHICS_ID;
}

void CCamera::init(void)
{
}

void CCamera::destroy(void)
{
    delete m_node;
}

void CCamera::update(float delta_time)
{
}

ComponentID CCamera::getID(void)
{
    return CCAMERA_ID;
}

CGraphicsCreatedEvent::CGraphicsCreatedEvent(ISceneNode* node, unsigned long id)
{
    u_node = node;
    m_id = id;
}

const EventType CGraphicsCreatedEvent::m_type(3);

const EventType& CGraphicsCreatedEvent::getEventType(void) const
{
    return m_type;
}

ISceneNode* CGraphicsCreatedEvent::getNode(void) const
{
    return u_node;
}

unsigned long CGraphicsCreatedEvent::getId(void) const
{
    return m_id;
}

int ccamera_lookat(lua_State* state)
{
    lua_getfield(state, 1, "instance");
    if(!lua_isuserdata(state, -1))
        return luaL_error(state, "Trying to set lookat target, but the Camera Component is missing its instance!");
    CCamera* cam = *static_cast<CCamera**>(lua_touserdata(state, -1));
    lua_pop(state, 1);

    if(lua_gettop(state) == 4)
        cam->m_node->lookAt(glm::vec3(lua_tonumber(state, 2), lua_tonumber(state, 3), lua_tonumber(state, 4)));
    else
        return 0; // TODO: Error here
    return 0;
}
