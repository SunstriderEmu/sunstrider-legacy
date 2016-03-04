/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _UTIL_H
#define _UTIL_H

#include "Common.h"
#include "Containers.h"
#include "Random.h"

#include <string>
#include <vector>

typedef std::vector<std::string> Tokens;

Tokens StrSplit(const std::string &src, const std::string &sep);

void stripLineInvisibleChars(std::string &src);

int32 MoneyStringToMoney(const std::string& moneyString);

struct tm* localtime_r(const time_t* time, struct tm *result);

std::string secsToTimeString(uint32 timeInSecs, bool shortText = false, bool hoursOnly = false);
uint32 TimeStringToSecs(const std::string& timestring);
std::string TimeToTimestampStr(time_t t);

inline uint32 secsToTimeBitFields(time_t secs)
{
    tm* lt = localtime(&secs);
    return (lt->tm_year - 100) << 24 | lt->tm_mon  << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min;
}

inline void ApplyPercentModFloatVar(float& var, float val, bool apply)
{
    if (val == -100.0f)     // prevent set var to zero
        val = -99.99f;
    var *= (apply ? (100.0f + val) / 100.0f : 100.0f / (100.0f + val));
}

// Percentage calculation
template <class T, class U>
inline T CalculatePct(T base, U pct)
{
    return T(base * static_cast<float>(pct) / 100.0f);
}

template <class T, class U>
inline T AddPct(T &base, U pct)
{
    return base += CalculatePct(base, pct);
}

template <class T, class U>
inline T ApplyPct(T &base, U pct)
{
    return base = CalculatePct(base, pct);
}

inline void ApplyModUInt32Var(uint32& var, int32 val, bool apply)
{
    int32 cur = var;
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    var = cur;
}

inline void ApplyModFloatVar(float& var, float  val, bool apply)
{
    var += (apply ? val : -val);
    if(var < 0)
        var = 0;
}

bool Utf8toWStr(const std::string& utf8str, std::wstring& wstr);
// in wsize==max size of buffer, out wsize==real string size
bool Utf8toWStr(char const* utf8str, size_t csize, wchar_t* wstr, size_t& wsize);
inline bool Utf8toWStr(const std::string& utf8str, wchar_t* wstr, size_t& wsize)
{
    return Utf8toWStr(utf8str.c_str(), utf8str.size(), wstr, wsize);
}

bool WStrToUtf8(std::wstring wstr, std::string& utf8str);
// size==real string size
bool WStrToUtf8(wchar_t* wstr, size_t size, std::string& utf8str);

size_t utf8length(std::string& utf8str);                    // set string to "" if invalid utf8 sequence
void utf8truncate(std::string& utf8str,size_t len);

inline bool isBasicLatinCharacter(wchar_t wchar)
{
    if(wchar >= L'a' && wchar <= L'z')                      // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
        return true;
    if(wchar >= L'A' && wchar <= L'Z')                      // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
        return true;
    return false;
}

inline bool isExtendedLatinCharacter(wchar_t wchar)
{
    if(isBasicLatinCharacter(wchar))
        return true;
    if(wchar >= 0x00C0 && wchar <= 0x00D6)                  // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
        return true;
    if(wchar >= 0x00D8 && wchar <= 0x00DF)                  // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
        return true;
    if(wchar == 0x00DF)                                     // LATIN SMALL LETTER SHARP S
        return true;
    if(wchar >= 0x00E0 && wchar <= 0x00F6)                  // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
        return true;
    if(wchar >= 0x00F8 && wchar <= 0x00FE)                  // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
        return true;
    if(wchar >= 0x0100 && wchar <= 0x012F)                  // LATIN CAPITAL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK
        return true;
    if(wchar == 0x1E9E)                                     // LATIN CAPITAL LETTER SHARP S
        return true;
    return false;
}

inline bool isCyrillicCharacter(wchar_t wchar)
{
    if(wchar >= 0x0410 && wchar <= 0x044F)                  // CYRILLIC CAPITAL LETTER A - CYRILLIC SMALL LETTER YA
        return true;
    if(wchar == 0x0401 || wchar == 0x0451)                  // CYRILLIC CAPITAL LETTER IO, CYRILLIC SMALL LETTER IO
        return true;
    return false;
}

inline bool isEastAsianCharacter(wchar_t wchar)
{
    if(wchar >= 0x1100 && wchar <= 0x11F9)                  // Hangul Jamo
        return true;
    if(wchar >= 0x3041 && wchar <= 0x30FF)                  // Hiragana + Katakana
        return true;
    if(wchar >= 0x3131 && wchar <= 0x318E)                  // Hangul Compatibility Jamo
        return true;
    if(wchar >= 0x31F0 && wchar <= 0x31FF)                  // Katakana Phonetic Ext.
        return true;
    if(wchar >= 0x3400 && wchar <= 0x4DB5)                  // CJK Ideographs Ext. A
        return true;
    if(wchar >= 0x4E00 && wchar <= 0x9FC3)                  // Unified CJK Ideographs
        return true;
    if(wchar >= 0xAC00 && wchar <= 0xD7A3)                  // Hangul Syllables
        return true;
    if(wchar >= 0xFF01 && wchar <= 0xFFEE)                  // Halfwidth forms
        return true;
    return false;
}

inline bool isNumeric(wchar_t wchar)
{
    return (wchar >= L'0' && wchar <=L'9');
}

inline bool isNumeric(char c)
{
    return (c >= '0' && c <='9');
}

inline bool isNumericOrSpace(wchar_t wchar)
{
    return isNumeric(wchar) || wchar == L' ';
}

inline bool isBasicLatinString(std::wstring wstr, bool numericOrSpace)
{
    for(size_t i = 0; i < wstr.size(); ++i)
        if(!isBasicLatinCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
            return false;
    return true;
}

inline bool isExtendedLatinString(std::wstring wstr, bool numericOrSpace)
{
    for(size_t i = 0; i < wstr.size(); ++i)
        if(!isExtendedLatinCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
            return false;
    return true;
}

inline bool isCyrillicString(std::wstring wstr, bool numericOrSpace)
{
    for(size_t i = 0; i < wstr.size(); ++i)
        if(!isCyrillicCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
            return false;
    return true;
}

inline bool isEastAsianString(std::wstring wstr, bool numericOrSpace)
{
    for(size_t i = 0; i < wstr.size(); ++i)
        if(!isEastAsianCharacter(wstr[i]) && (!numericOrSpace || !isNumericOrSpace(wstr[i])))
            return false;
    return true;
}

inline wchar_t wcharToUpper(wchar_t wchar)
{
    if(wchar >= L'a' && wchar <= L'z')                      // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
        return wchar_t(uint16(wchar)-0x0020);
    if(wchar == 0x00DF)                                     // LATIN SMALL LETTER SHARP S
        return wchar_t(0x1E9E);
    if(wchar >= 0x00E0 && wchar <= 0x00F6)                  // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
        return wchar_t(uint16(wchar)-0x0020);
    if(wchar >= 0x00F8 && wchar <= 0x00FE)                  // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
        return wchar_t(uint16(wchar)-0x0020);
    if(wchar >= 0x0101 && wchar <= 0x012F)                  // LATIN SMALL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK (only %2=1)
    {
        if(wchar % 2 == 1)
            return wchar_t(uint16(wchar)-0x0001);
    }
    if(wchar >= 0x0430 && wchar <= 0x044F)                  // CYRILLIC SMALL LETTER A - CYRILLIC SMALL LETTER YA
        return wchar_t(uint16(wchar)-0x0020);
    if(wchar == 0x0451)                                     // CYRILLIC SMALL LETTER IO
        return wchar_t(0x0401);

    return wchar;
}

inline wchar_t wcharToUpperOnlyLatin(wchar_t wchar)
{
    return isBasicLatinCharacter(wchar) ? wcharToUpper(wchar) : wchar;
}

inline wchar_t wcharToLower(wchar_t wchar)
{
    if(wchar >= L'A' && wchar <= L'Z')                      // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
        return wchar_t(uint16(wchar)+0x0020);
    if(wchar >= 0x00C0 && wchar <= 0x00D6)                  // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
        return wchar_t(uint16(wchar)+0x0020);
    if(wchar >= 0x00D8 && wchar <= 0x00DE)                  // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
        return wchar_t(uint16(wchar)+0x0020);
    if(wchar >= 0x0100 && wchar <= 0x012E)                  // LATIN CAPITAL LETTER A WITH MACRON - LATIN CAPITAL LETTER I WITH OGONEK (only %2=0)
    {
        if(wchar % 2 == 0)
            return wchar_t(uint16(wchar)+0x0001);
    }
    if(wchar == 0x1E9E)                                     // LATIN CAPITAL LETTER SHARP S
        return wchar_t(0x00DF);
    if(wchar == 0x0401)                                     // CYRILLIC CAPITAL LETTER IO
        return wchar_t(0x0451);
    if(wchar >= 0x0410 && wchar <= 0x042F)                  // CYRILLIC CAPITAL LETTER A - CYRILLIC CAPITAL LETTER YA
        return wchar_t(uint16(wchar)+0x0020);

    return wchar;
}

inline void wstrToUpper(std::wstring& str)
{
    std::transform( str.begin(), str.end(), str.begin(), wcharToUpper );
}

inline void wstrToLower(std::wstring& str)
{
    std::transform( str.begin(), str.end(), str.begin(), wcharToLower );
}

std::wstring GetMainPartOfName(std::wstring wname, uint32 declension);

bool utf8ToConsole(const std::string& utf8str, std::string& conStr);
bool consoleToUtf8(const std::string& conStr,std::string& utf8str);
bool Utf8FitTo(const std::string& str, std::wstring search);
void utf8printf(FILE* out, const char *str, ...);
void vutf8printf(FILE* out, const char *str, va_list* ap);

std::string ByteArrayToHexStr(uint8 const* bytes, uint32 length, bool reverse = false );

/* Select a random element from a container. Note: make sure you explicitly empty check the container */
template <class C> typename C::value_type const& SelectRandomContainerElement(C const& container)
{
    typename C::const_iterator it = container.begin();
    std::advance(it, urand(0, container.size() - 1));
    return *it;
}

#if PLATFORM == PLATFORM_WINDOWS
#define UTF8PRINTF(OUT,FRM,RESERR)                      \
{                                                       \
    char temp_buf[6000];                                \
    va_list ap;                                         \
    va_start(ap, FRM);                                  \
    size_t temp_len = vsnprintf(temp_buf,6000,FRM,ap);  \
    va_end(ap);                                         \
                                                        \
    wchar_t wtemp_buf[6000];                            \
    size_t wtemp_len = 6000-1;                          \
    if(!Utf8toWStr(temp_buf,temp_len,wtemp_buf,wtemp_len)) \
        return RESERR;                                  \
    CharToOemBuffW(&wtemp_buf[0],&temp_buf[0],wtemp_len+1);\
    fprintf(OUT,temp_buf);                              \
}
#else
#define UTF8PRINTF(OUT,FRM,RESERR)                      \
{                                                       \
    va_list ap;                                         \
    va_start(ap, FRM);                                  \
    vfprintf(OUT, FRM, ap );                            \
    va_end(ap);                                         \
}
#endif

enum ComparisionType
{
    COMP_TYPE_EQ = 0,
    COMP_TYPE_HIGH,
    COMP_TYPE_LOW,
    COMP_TYPE_HIGH_EQ,
    COMP_TYPE_LOW_EQ,
    COMP_TYPE_MAX
};

template <class T>
bool CompareValues(ComparisionType type, T val1, T val2)
{
    switch (type)
    {
        case COMP_TYPE_EQ:
            return val1 == val2;
        case COMP_TYPE_HIGH:
            return val1 > val2;
        case COMP_TYPE_LOW:
            return val1 < val2;
        case COMP_TYPE_HIGH_EQ:
            return val1 >= val2;
        case COMP_TYPE_LOW_EQ:
            return val1 <= val2;
        default:
            // incorrect parameter
            ASSERT(false);
            return false;
    }
}

bool IsIPAddress(char const* ipaddress);
uint32 CreatePIDFile(const std::string& filename);

class Tokenizer
{
public:
    typedef std::vector<char const*> StorageType;

    typedef StorageType::size_type size_type;

    typedef StorageType::const_iterator const_iterator;
    typedef StorageType::reference reference;
    typedef StorageType::const_reference const_reference;

public:
    Tokenizer(const std::string &src, char const sep, uint32 vectorReserve = 0);
    ~Tokenizer() { delete[] m_str; }

    const_iterator begin() const { return m_storage.begin(); }
    const_iterator end() const { return m_storage.end(); }

    size_type size() const { return m_storage.size(); }

    reference operator [] (size_type i) { return m_storage[i]; }
    const_reference operator [] (size_type i) const { return m_storage[i]; }

private:
    char* m_str;
    StorageType m_storage;
};

static float GetDistance(float x, float y, float z, float x2, float y2, float z2) 
{
    float dx = x2 - x;
    float dy = y2 - y;
    float dz = z2 - z;
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz));
    return ( dist > 0 ? dist : 0);     
};

static float GetDistance(float x, float y, float x2, float y2) 
{
    float dx = x2 - x;
    float dy = y2 - y;
    float dist = sqrt((dx*dx) + (dy*dy));
    return ( dist > 0 ? dist : 0);     
};

//return true of timer reached 0
static bool DecreaseTimer(uint32& timer, uint32 const diff)
{
    if (timer > 0)
    {
        if (diff >= timer)
            timer = 0;
        else
        {
            timer -= diff;
            return false;
        }
    }
    return true;
};


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
    * @param eventId The id of the new event.
    * @param time The time in milliseconds until the event occurs.
    * @param group The group which the event is associated to. Has to be between 1 and 8. 0 means it has no group.
    * @param phase The phase in which the event can occur. Has to be between 1 and 8. 0 means it can occur in all phases.
    */
    void ScheduleEvent(uint32 eventId, uint32 time, uint32 group = 0, uint32 phase = 0)
    {
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

