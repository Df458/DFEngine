#include "Actor.h"
#include "SceneNode.h"
#include "Level.h"
#include "Scene.h"

using namespace rapidxml;

Level::Level(char* text_buffer)
{
    m_text_buffer = text_buffer;
    //printf("Level data:\n--%s--\n", m_text_buffer);
    m_document.parse<0>(m_text_buffer);
    xml_node<>* root_node = m_document.first_node();
    if(xml_attribute<>* sa = root_node->first_attribute("world_scale", 11, false))
        m_world_scale = atof(sa->value());
    if(xml_attribute<>* gxa = root_node->first_attribute("gravity.x", 9, false))
        m_gravity.x = atof(gxa->value());
    if(xml_attribute<>* gya = root_node->first_attribute("gravity.y", 9, false))
        m_gravity.y = atof(gya->value());
    if(xml_attribute<>* gza = root_node->first_attribute("gravity.z", 9, false))
        m_gravity.z = atof(gza->value());
    for(xml_node<>* i = root_node->first_node("actor", 5, false); i; i = i->next_sibling("actor", 5, false)) {
        m_actors.push_back(new ActorConstructionData(i));
    }

    //m_scene = new Scene(root_node);
}

void Level::cleanup(void)
{
    delete[] m_text_buffer;
    for(auto a : m_actors)
        delete a;
    //delete m_scene;
}

std::vector<ActorConstructionData*> Level::getActorList(void)
{
    return m_actors;
}

//const Scene* Level::getSceneData(void) const
//{
    //return m_scene;
//}
