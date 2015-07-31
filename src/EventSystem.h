#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "Event.h"
#include "System.h"
#include <functional>
#include <list>
#include <map>

typedef std::pair<void*, std::function<void(const IEvent&)>> Callback;

class IEventManager
{
public:
    virtual ~IEventManager(void) = 0;
    virtual bool addSubscription(const Callback& subscription_callback, const EventType& type) = 0;
    virtual bool rmSubscription (void* subscriber, const EventType& type) = 0;
    virtual bool callEvent (const IEvent& event) const = 0;
    virtual bool queueEvent(IEvent* event) = 0;
    virtual bool clearEvent(const EventType& type, bool next_only = true) = 0;
};

inline IEventManager::~IEventManager() {}

class EventSystem : public IEventManager, public ISystem
{
public:
    EventSystem(void);
    virtual ~EventSystem(void);
    bool initialize(void);
    void update(void);
    void cleanup(void);
    virtual bool addSubscription(const Callback& subscription_callback, const EventType& type);
    virtual bool rmSubscription (void* subscriber, const EventType& type);
    virtual bool callEvent (const IEvent& event) const;
    virtual bool queueEvent(IEvent* event);
    virtual bool clearEvent(const EventType& type, bool next_only = true);
private:
    std::list<IEvent*> m_event_queue;
    std::map<EventType, std::list<Callback>> m_subscriptions;
};

#endif
