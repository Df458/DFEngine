#include "Component.h"
#include "ComponentFactory.h"
#include "Util.h"

#include <algorithm>

using namespace rapidxml;

IComponent* ComponentFactory::buildComponent(xml_node<>* node, Actor* actor) const {
    if(!node) {
        error("Trying to create a component from null data.");
        return NULL;
    }

    std::string id = node->name();
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    IComponent* component;
    buildfunc builder;
    try {
        builder = u_builders.at(id);
    } catch(const std::out_of_range& e) {
        error(("Trying to create a component with no registered builder. (got " + id + ")").c_str());
        return NULL;
    }
    component = builder(node, actor);
    if(xml_attribute<>* att = node->first_attribute("name", 4, false)) {
        component->setName(att->value());
    }

    return component;
}

bool ComponentFactory::registerComponentBuilder(buildfunc builder, std::string component_type) {
    if(u_builders.find(component_type) != u_builders.end()) {
        warn("Trying to register a component builder for an existing component type.");
        return false;
    }

    u_builders[component_type] = builder;
    return true;
}
