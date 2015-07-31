#ifndef XML_SERIALIZABLE_H
#define XML_SERIALIZABLE_H
#include <rapidxml.hpp>

class IXmlSerializable
{
public:
    virtual ~IXmlSerializable() {}
    virtual bool fromXml(rapidxml::xml_node<>* node) = 0;
    //virtual void toXml(void) = 0;
protected:
private:
};

#endif
