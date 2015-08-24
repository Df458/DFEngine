#include "Event.h"

const EventType ActorDestroyedEvent::m_type(1);

const EventType& ActorDestroyedEvent::getEventType(void) const {
    return m_type;
}

ActorDestroyedEvent::ActorDestroyedEvent(unsigned long id) {
    m_id = id;
}

unsigned long ActorDestroyedEvent::getId(void) const {
    return m_id;
}
