#ifndef _EVENTMAP_H
#define _EVENTMAP_H

#include "Errors.h"
#include "Define.h"

class EventMap
{
    typedef std::multimap<uint32, uint32> EventStore;

public:
    EventMap() : _time(0), _phase(0) { }

    /**
    * @name Reset
    * @brief Removes all scheduled events and resets time and phase.
    */
    void Reset()
    {
        _eventMap.clear();
        _time = 0;
        _phase = 0;
    }

    /**
    * @name Update
    * @brief Updates the timer of the event map.
    * @param time Value to be added to time.
    */
    void Update(uint32 time)
    {
        _time += time;
    }

    /**
    * @name GetTimer
    * @return Current timer value.
    */
    uint32 GetTimer() const
    {
        return _time;
    }

    void SetTimer(uint32 time)
    {
        _time = time;
    }

    /**
    * @name GetPhaseMask
    * @return Active phases as mask.
    */
    uint8 GetPhaseMask() const
    {
        return _phase;
    }

    /**
    * @name Empty
    * @return True, if there are no events scheduled.
    */
    bool Empty() const
    {
        return _eventMap.empty();
    }

    /**
    * @name SetPhase
    * @brief Sets the phase of the map (absolute).
    * @param phase Phase which should be set. Values: 1 - 8. 0 resets phase.
    */
    void SetPhase(uint8 phase)
    {
        if (!phase)
            _phase = 0;
        else if (phase <= 8)
            _phase = (1 << (phase - 1));
    }

    /**
    * @name AddPhase
    * @brief Activates the given phase (bitwise).
    * @param phase Phase which should be activated. Values: 1 - 8
    */
    void AddPhase(uint8 phase)
    {
        if (phase && phase <= 8)
            _phase |= (1 << (phase - 1));
    }

    /**
    * @name RemovePhase
    * @brief Deactivates the given phase (bitwise).
    * @param phase Phase which should be deactivated. Values: 1 - 8.
    */
    void RemovePhase(uint8 phase)
    {
        if (phase && phase <= 8)
            _phase &= ~(1 << (phase - 1));
    }

    /**
    * @name ScheduleEvent
    * @brief Creates new event entry in map.
    * @param eventId The id of the new event. Can't be 0.
    * @param time The time in milliseconds until the event occurs.
    * @param group The group which the event is associated to. Has to be between 1 and 8. 0 means it has no group.
    * @param phase The phase in which the event can occur. Has to be between 1 and 8. 0 means it can occur in all phases.
    */
    void ScheduleEvent(uint32 eventId, uint32 time, uint32 group = 0, uint32 phase = 0)
    {
		DEBUG_ASSERT(eventId != 0); //Never use eventId 0, as this is a value returned as "no event" by ExecuteEvent.

        if (group && group <= 8)
            eventId |= (1 << (group + 15));

        if (phase && phase <= 8)
            eventId |= (1 << (phase + 23));

        _eventMap.insert(EventStore::value_type(_time + time, eventId));
    }

    /**
    * @name RescheduleEvent
    * @brief Cancels the given event and reschedules it.
    * @param eventId The id of the event.
    * @param time The time in milliseconds until the event occurs.
    * @param group The group which the event is associated to. Has to be between 1 and 8. 0 means it has no group.
    * @param phase The phase in which the event can occur. Has to be between 1 and 8. 0 means it can occur in all phases.
    */
    void RescheduleEvent(uint32 eventId, uint32 time, uint32 groupId = 0, uint32 phase = 0)
    {
        CancelEvent(eventId);
        ScheduleEvent(eventId, time, groupId, phase);
    }

    bool IsEventScheduled(uint32 eventId, uint32 groupId = 0, uint32 phase = 0)
    {
        if (Empty())
            return false;

        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end(); itr++)
            if (eventId == (itr->second & 0x0000FFFF))
                return true;

        return false;
    }

    /**
    * @name RepeatEvent

    */
    void RepeatEvent(uint32 time)
    {
        if (Empty())
            return;

        uint32 eventId = _eventMap.begin()->second;
        _eventMap.erase(_eventMap.begin());
        ScheduleEvent(eventId, time);
    }

    /**
    * @name PopEvent
    * @brief Remove the first event in the map.
    */
    void PopEvent()
    {
        if (!Empty())
            _eventMap.erase(_eventMap.begin());
    }

    /**
    * @name ExecuteEvent
    * @brief Returns the next event to execute and removes it from map.
    * @return Id of the event to execute.
    */
    uint32 ExecuteEvent()
    {
        while (!Empty())
        {
            EventStore::iterator itr = _eventMap.begin();

            if (itr->first > _time)
                return 0;
            else if (_phase && (itr->second & 0xFF000000) && !((itr->second >> 24) & _phase))
                _eventMap.erase(itr);
            else
            {
                uint32 eventId = (itr->second & 0x0000FFFF);
                _eventMap.erase(itr);
                return eventId;
            }
        }

        return 0;
    }

    /**
    * @name GetEvent
    * @brief Returns the next event to execute.
    * @return Id of the event to execute.
    */
    uint32 GetEvent()
    {
        while (!Empty())
        {
            EventStore::iterator itr = _eventMap.begin();

            if (itr->first > _time)
                return 0;
            else if (_phase && (itr->second & 0xFF000000) && !(itr->second & (_phase << 24)))
                _eventMap.erase(itr);
            else
                return (itr->second & 0x0000FFFF);
        }

        return 0;
    }

    /**
    * @name DelayEvents
    * @brief Delays all events in the map. If delay is greater than or equal internal timer, delay will be 0.
    * @param delay Amount of delay.
    */
    void DelayEvents(uint32 delay)
    {
        _time = delay < _time ? _time - delay : 0;
    }

    void DelayEventsToMax(uint32 delay, uint32 group)
    {
        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (itr->first < _time + delay && (group == 0 || ((1 << (group + 15)) & itr->second)))
            {
                ScheduleEvent(itr->second, delay);
                _eventMap.erase(itr);
                itr = _eventMap.begin();
            }
            else
                ++itr;
        }
    }

    /** 
    Check events with given id delay and set it to delay if it's smaller
    */
    void SetMinimalDelay(uint32 eventId, uint32 delay)
    {
        if (Empty())
            return;

        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (eventId == (itr->second & 0x0000FFFF))
            {
                if (itr->first < delay)
                {
                    _eventMap.insert(EventStore::value_type(delay, itr->second));
                    itr = _eventMap.erase(itr);
                    continue;
                }

            }
            itr++;
        }
    }

    /**
    * @name DelayEvents
    * @brief Delay all events of the same group.
    * @param delay Amount of delay.
    * @param group Group of the events.
    */
    void DelayEvents(uint32 delay, uint32 group)
    {
        if (group > 8 || Empty())
            return;

        EventStore delayed;

        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (!group || (itr->second & (1 << (group + 15))))
            {
                delayed.insert(EventStore::value_type(itr->first + delay, itr->second));
                _eventMap.erase(itr++);
            }
            else
                ++itr;
        }

        _eventMap.insert(delayed.begin(), delayed.end());
    }

    /**
    * @name CancelEvent
    * @brief Cancels all events of the specified id.
    * @param eventId Event id to cancel.
    */
    void CancelEvent(uint32 eventId)
    {
        if (Empty())
            return;

        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (eventId == (itr->second & 0x0000FFFF))
                _eventMap.erase(itr++);
            else
                ++itr;
        }
    }

    /**
    * @name CancelEventGroup
    * @brief Cancel events belonging to specified group.
    * @param group Group to cancel.
    */
    void CancelEventGroup(uint32 group)
    {
        if (!group || group > 8 || Empty())
            return;

        uint32 groupMask = (1 << (group + 15));
        for (EventStore::iterator itr = _eventMap.begin(); itr != _eventMap.end();)
        {
            if (itr->second & groupMask)
            {
                _eventMap.erase(itr);
                itr = _eventMap.begin();
            }
            else
                ++itr;
        }
    }

    /**
    * @name GetNextEventTime
    * @brief Returns closest occurence of specified event.
    * @param eventId Wanted event id.
    * @return Time of found event.
    */
    uint32 GetNextEventTime(uint32 eventId) const
    {
        if (Empty())
            return 0;

        for (EventStore::const_iterator itr = _eventMap.begin(); itr != _eventMap.end(); ++itr)
            if (eventId == (itr->second & 0x0000FFFF))
                return itr->first;

        return 0;
    }

    /**
    * @name GetNextEventTime
    * @return Time of next event.
    */
    uint32 GetNextEventTime() const
    {
        return Empty() ? 0 : _eventMap.begin()->first;
    }

    /**
    * @name IsInPhase
    * @brief Returns wether event map is in specified phase or not.
    * @param phase Wanted phase.
    * @return True, if phase of event map contains specified phase.
    */
    bool IsInPhase(uint8 phase)
    {
        return phase <= 8 && (!phase || _phase & (1 << (phase - 1)));
    }

private:
    uint32 _time;
    uint32 _phase;

    EventStore _eventMap;
};

#endif