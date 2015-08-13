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
int ccamera_Index(lua_State* state);
int ccamera_NewIndex(lua_State* state);
int cgraphics_Index(lua_State* state);
int cgraphics_NewIndex(lua_State* state);

const luaL_Reg ccamera_funcs[] =
{
    {"lookat", ccamera_lookat},
    {0, 0}
};

const luaL_Reg ccamera_meta[] =
{
    {"__index", ccamera_Index},
    {"__newindex", ccamera_NewIndex},
    {0, 0}
};

const luaL_Reg cgraphics_meta[] =
{
    {"__index", cgraphics_Index},
    {"__newindex", cgraphics_NewIndex},
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
    virtual const luaL_Reg* getMetaFuncs(void) const { return cgraphics_meta; }
    virtual const luaL_Reg* getAttrFuncs(void) const { return m_node->getAttrFuncs(); }
    ISceneNode* getNode(void) { return m_node; }

    friend IComponent* buildGraphics(rapidxml::xml_node<>* node, Actor* actor);
protected:
    ISceneNode* m_node;
    bool m_updates = false;
};

class CCamera : public IComponent
{
public:
    virtual void init(void);
    virtual void destroy(void);
    virtual void update(float delta_time);
    virtual ComponentID getID(void);
    virtual const luaL_Reg* getFuncs(void) const { return ccamera_funcs; }
    virtual const luaL_Reg* getMetaFuncs(void) const { return ccamera_meta; }
    virtual const luaL_Reg* getAttrFuncs(void) const { return m_node->getAttrFuncs(); }

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
