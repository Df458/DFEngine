#ifndef EVENT_H
#define EVENT_H

typedef unsigned int EventType;

class IEvent {
public:
    virtual ~IEvent(void) = 0;
    virtual const EventType& getEventType (void) const = 0;
};

inline IEvent::~IEvent() {}

class ActorDestroyedEvent : public IEvent {
    public:
        ActorDestroyedEvent(unsigned long id);
        virtual const EventType& getEventType (void) const;
        unsigned long getId(void) const;

        static const EventType m_type;
    private:
        unsigned long m_id;
};

#endif
