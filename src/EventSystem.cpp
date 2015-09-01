#include "Event.h"
#include "EventSystem.h"
#include "Util.h"

EventSystem::EventSystem(void)
{
}

EventSystem::~EventSystem(void)
{
}

bool EventSystem::initialize(void)
{
    return true;
}

void EventSystem::update(float dt)
{
    while(!m_event_queue.empty()) {
        callEvent(*m_event_queue.front());
        m_event_queue.pop_front();
    }
}

void EventSystem::cleanup(void)
{
}

bool EventSystem::addSubscription(const Callback& subscription_callback, const EventType& type)
{
    std::list<Callback> list = m_subscriptions[type];
    for(auto i = list.begin(); i != list.end(); ++i) {
        if(subscription_callback.first == (*i).first) {
            warn("Trying to add existing subscription again.");
            return false;
        }
    }
    m_subscriptions[type].push_back(subscription_callback);
    return true;
}

bool EventSystem::rmSubscription(void* subscriber, const EventType& type)
{
    std::list<Callback> list = m_subscriptions[type];
    for(auto i = list.begin(); i != list.end(); ++i) {
        if(subscriber == (*i).first) {
            list.erase(i);
            return true;
        }
    }
    warn("Trying to remove a subscription that doesn't exits.");

    return false;
}

bool EventSystem::callEvent(const IEvent& event) const
{
    std::list<Callback> list;
    try {
        list = m_subscriptions.at(event.getEventType());
    } catch(std::out_of_range e) {
        return false;
    }
    bool called = false;
    for(auto i : list) {
        called = true;
        i.second(event);
    }
    return called;
}

bool EventSystem::queueEvent(IEvent* event)
{
    m_event_queue.push_back(event);
    return true;
}

bool EventSystem::clearEvent(const EventType& type, bool next_only)
{
//:TODO: 27.12.14 13:02:50, df458
// implement this
    error("Function clearEvent is unimplemented.");
    return false;
}
