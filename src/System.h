#ifndef SYSTEM_H
#define SYSTEM_H

class ISystem
{
public:
    virtual ~ISystem(void) = 0;
    virtual bool initialize(void) = 0;
    virtual void cleanup(void) = 0;
};

inline ISystem::~ISystem(void) {}

#endif
