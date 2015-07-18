#ifndef C_GRAPHICS_H
#define C_GRAPHICS_H
#include "Component.h"
#include "Event.h"
#include "SceneNode.h"

#include <rapidxml.hpp>

class Actor;
class CameraSceneNode;
#define CGRAPHICS_ID 1
#define CCAMERA_ID 2

IComponent* buildGraphics(rapidxml::xml_node<>* node, Actor* actor);
IComponent* buildCamera(rapidxml::xml_node<>* node, Actor* actor);

int ccamera_lookat(lua_State* state);

const luaL_Reg ccamera_funcs[] =
{
    {"lookat", ccamera_lookat},
    {0, 0}
};

class CGraphics : public IComponent
{
public:
    virtual void init(void);
    virtual void destroy(void);
    virtual void update(float delta_time);
    virtual ComponentID getID(void);
    virtual const luaL_Reg* getFuncs(void) const { return m_node->getFuncs(); }
    ISceneNode* getNode(void) { return m_node; }

    friend IComponent* buildGraphics(rapidxml::xml_node<>* node, Actor* actor);
protected:
    ISceneNode* m_node;
    bool m_updates = false;
};

class CCamera : public IComponent
{
    virtual void init(void);
    virtual void destroy(void);
    virtual void update(float delta_time);
    virtual ComponentID getID(void);
    virtual const luaL_Reg* getFuncs(void) const { return ccamera_funcs; }

    friend IComponent* buildCamera(rapidxml::xml_node<>* node, Actor* actor);
    friend int ccamera_lookat(lua_State* state);
protected:
    CameraSceneNode* m_node;
};

class CGraphicsCreatedEvent : public IEvent
{
    public:
        CGraphicsCreatedEvent(ISceneNode* node, unsigned long id);
        virtual const EventType& getEventType (void) const;
        ISceneNode* getNode(void) const;
        unsigned long getId(void) const;

        static const EventType m_type;
    private:
        ISceneNode* u_node;
        unsigned long m_id;
};

#endif
