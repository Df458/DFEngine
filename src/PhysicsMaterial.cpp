#include "PhysicsMaterial.h"

using namespace rapidxml;

PhysicsMaterial::PhysicsMaterial(rapidxml::xml_node<>* node)
{
    if(xml_attribute<>* attr = node->first_attribute("mass", 4, false))
        mass = atof(attr->value());
    if(xml_attribute<>* attr = node->first_attribute("restitution", 11, false))
        restitution = atof(attr->value());
    if(xml_node<>* n = node->first_node("friction", 8, false)) {
        if(xml_attribute<>* attr = n->first_attribute("sliding", 7, false))
            sliding_friction = atof(attr->value());
        if(xml_attribute<>* attr = n->first_attribute("rolling", 7, false))
            rolling_friction = atof(attr->value());
    }
}
