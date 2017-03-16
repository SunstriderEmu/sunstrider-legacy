#include "Chat.h"
#include "Language.h"
#include "GameEventMgr.h"

bool ChatHandler::HandleEventInfoCommand(const char* args)
{
    ARGS_CHECK

    // id or [name] Shift-click form |color|Hgameevent:id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameevent");
    if(!cId)
        return false;

    uint32 event_id = atoi(cId);

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();

    if(event_id >=events.size())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventData const& eventData = events[event_id];
    if(!eventData.isValid())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr->GetActiveEventList();
    bool active = activeEvents.find(event_id) != activeEvents.end();
    char const* activeStr = active ? GetTrinityString(LANG_ACTIVE) : "";

    std::string startTimeStr = TimeToTimestampStr(eventData.start);
    std::string endTimeStr = TimeToTimestampStr(eventData.end);

    uint32 delay = sGameEventMgr->NextCheck(event_id);
    time_t nextTime = time(nullptr)+delay;
    std::string nextStr = nextTime >= eventData.start && nextTime < eventData.end ? TimeToTimestampStr(time(nullptr)+delay) : "-";

    std::string occurenceStr = secsToTimeString(eventData.occurence * MINUTE);
    std::string lengthStr = secsToTimeString(eventData.length * MINUTE);

    PSendSysMessage(LANG_EVENT_INFO,event_id,eventData.description.c_str(),activeStr,
        startTimeStr.c_str(),endTimeStr.c_str(),occurenceStr.c_str(),lengthStr.c_str(),
        nextStr.c_str());
    return true;
}

bool ChatHandler::HandleEventStartCommand(const char* args)
{
    ARGS_CHECK

    // id or [name] Shift-click form |color|Hgameevent:id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameevent");
    if(!cId)
        return false;

    int32 event_id = atoi(cId);

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();

    if(event_id < 1 || event_id >=events.size())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventData const& eventData = events[event_id];
    if(!eventData.isValid())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    if(sGameEventMgr->IsActiveEvent(event_id))
    {
        PSendSysMessage(LANG_EVENT_ALREADY_ACTIVE,event_id);
        SetSentErrorMessage(true);
        return false;
    }

    sGameEventMgr->StartEvent(event_id,true);
    return true;
}

bool ChatHandler::HandleEventStopCommand(const char* args)
{
    ARGS_CHECK

    // id or [name] Shift-click form |color|Hgameevent:id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameevent");
    if(!cId)
        return false;

    int32 event_id = atoi(cId);

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();

    if(event_id < 1 || event_id >=events.size())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventData const& eventData = events[event_id];
    if(!eventData.isValid())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr->GetActiveEventList();

    if(activeEvents.find(event_id) == activeEvents.end())
    {
        PSendSysMessage(LANG_EVENT_NOT_ACTIVE,event_id);
        SetSentErrorMessage(true);
        return false;
    }

    sGameEventMgr->StopEvent(event_id,true);
    return true;
}

/* event create #id $name */
bool ChatHandler::HandleEventCreateCommand(const char* args)
{
    //cause crash, fix me
    /*
    ARGS_CHECK

    if(strcmp(args,"") == 0)
        return false;

    int16 createdEventId = 0;
    bool success = sGameEventMgr->CreateGameEvent(args,createdEventId);
    if(success)
        PSendSysMessage("L'event \"%s\" (id: %i) a été créé.",args,createdEventId);
    else
        PSendSysMessage("Erreur : L'event \"%s\" (id: %i) n'a pas pu être créé.",args,createdEventId);
 */
    return true;
}

bool ChatHandler::HandleEventActiveListCommand(const char* args)
{
    uint32 counter = 0;

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();
    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr->GetActiveEventList();

    char const* active = GetTrinityString(LANG_ACTIVE);

    for(unsigned short event_id : activeEvents)
    {
        GameEventData const& eventData = events[event_id];

        if(m_session)
            PSendSysMessage(LANG_EVENT_ENTRY_LIST_CHAT,event_id,event_id,eventData.description.c_str(),active );
        else
            PSendSysMessage(LANG_EVENT_ENTRY_LIST_CONSOLE,event_id,eventData.description.c_str(),active );

        ++counter;
    }

    if (counter==0)
        SendSysMessage(LANG_NOEVENTFOUND);

    return true;
}
