#ifndef LEVEL_H
#define LEVEL_H

#include <glm/vec3.hpp>
#include <rapidxml.hpp>
#include <vector>

class ActorConstructionData;
class Scene;

class Level
{
public:
    Level(char* text_buffer);
    void cleanup(void);
    std::vector<ActorConstructionData*> getActorList(void);
    inline glm::vec3 getGravity(void) const { return m_gravity; }
    //const Scene* getSceneData(void) const;
protected:
    char* m_text_buffer;
    rapidxml::xml_document<> m_document;
    std::vector<ActorConstructionData*> m_actors;
    glm::vec3 m_gravity;
    //Scene* m_scene;
};

#endif
