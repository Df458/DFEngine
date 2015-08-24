#include "XmlSerializable.h"
#include <cstring>

using namespace rapidxml;

bool IXmlSerializable::attr(rapidxml::xml_node<>* node, const char* name, float* f)
{
    if(xml_attribute<>* a = node->first_attribute(name)) {
        *f = atof(a->value());
        return true;
    }
    return false;
}

bool IXmlSerializable::attr(rapidxml::xml_node<>* node, const char* name, int* i)
{
    if(xml_attribute<>* a = node->first_attribute(name)) {
        *i = atoi(a->value());
        return true;
    }
    return false;
}

bool IXmlSerializable::attr(rapidxml::xml_node<>* node, const char* name, bool* b)
{
    if(xml_attribute<>* a = node->first_attribute(name)) {
        *b = strcmp(a->value(), "false");
        return true;
    }
    return false;
}
