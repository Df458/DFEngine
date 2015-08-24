#ifndef LEVEL_H
#define LEVEL_H

#include "XmlSerializable.h"
#include <glm/vec3.hpp>
#include <rapidxml.hpp>
#include <vector>

class ActorConstructionData;
class Scene;

class Level : public IXmlSerializable
{
public:
    Level(char* text_buffer);
    virtual bool fromXml(rapidxml::xml_node<>* node);
    void cleanup(void);
    std::vector<ActorConstructionData*> getActorList(void);
    inline glm::vec3 getGravity(void) const { return m_gravity; }
    inline float getWorldScale(void) const { return m_world_scale; }
protected:
    char* m_text_buffer;
    rapidxml::xml_document<> m_document;
    std::vector<ActorConstructionData*> m_actors;
    glm::vec3 m_gravity;
    float m_world_scale = 1;
};

#endif
