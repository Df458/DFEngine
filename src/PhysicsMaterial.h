#ifndef PHYSICS_MATERIAL_H
#define PHYSICS_MATERIAL_H
#include <rapidxml.hpp>

struct PhysicsMaterial
{
    PhysicsMaterial() {}
    PhysicsMaterial(rapidxml::xml_node<>* node);

    float mass = 1;
    float sliding_friction = 0.5;
    float rolling_friction = 0;
    float restitution = 0;
};

#endif
