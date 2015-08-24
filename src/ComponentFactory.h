#ifndef COMPONENT_FACTORY_H
#define COMPONENT_FACTORY_H
#include <map>
#include <rapidxml.hpp>
#include <string>

class Actor;
class IComponent;

typedef IComponent* (*buildfunc) (rapidxml::xml_node<>*, Actor*);

class IComponentFactory {
public:
    virtual ~IComponentFactory(void) = 0;
    virtual IComponent* buildComponent(rapidxml::xml_node<>* node, Actor* actor) const = 0;
    virtual bool registerComponentBuilder(buildfunc builder, std::string component_type) = 0;
};

inline IComponentFactory::~IComponentFactory(void) {}

class ComponentFactory : public IComponentFactory {
public:
    virtual IComponent* buildComponent(rapidxml::xml_node<>* node, Actor* actor) const;
    virtual bool registerComponentBuilder(buildfunc builder, std::string component_type);
protected:
    std::map<std::string, buildfunc> u_builders;
};

#endif
