#include "Actor.h"
#include "SceneNode.h"
#include "Level.h"
#include "Scene.h"

using namespace rapidxml;

Level::Level(char* text_buffer)
{
    m_text_buffer = text_buffer;
    m_document.parse<0>(m_text_buffer);
    xml_node<>* root_node = m_document.first_node();
    
    fromXml(root_node);
}

bool Level::fromXml(xml_node<>* node)
{
    attr(node, "world_scale", &m_world_scale);
    attr(node, "gravity.x", &m_gravity.x);
    attr(node, "gravity.y", &m_gravity.y);
    attr(node, "gravity.z", &m_gravity.z);
    for(xml_node<>* i = node->first_node("actor", 5, false); i; i = i->next_sibling("actor", 5, false)) {
        m_actors.push_back(new ActorConstructionData(i));
    }

    return true;
}

void Level::cleanup(void)
{
    delete[] m_text_buffer;
    for(auto a : m_actors)
        delete a;
}

std::vector<ActorConstructionData*> Level::getActorList(void)
{
    return m_actors;
}
