#include "Chat.h"
#include "Language.h"

bool ChatHandler::HandleZoneBuffCommand(const char* args)
{
    ARGS_CHECK

    char *bufid = strtok((char *)args, " ");
    if (!bufid)
        return false;

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
    Player *p;

    for (const auto & player : players) {
        p = player.second;
        if (p && p->IsInWorld() && p->GetZoneId() == m_session->GetPlayer()->GetZoneId())
            p->CastSpell(p, atoi(bufid), true);
    }

    return true;
}

bool ChatHandler::HandleZoneMorphCommand(const char* args)
{
    ARGS_CHECK

    char *displid = strtok((char *)args, " ");
    if (!displid)
        return false;
    char *factid = strtok(nullptr, " ");

    uint16 display_id = (uint16)atoi((char *)args);
    uint8 faction_id = factid ? (uint8)atoi(factid) : 0;

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
    Player *p;

    for (const auto & player : players) {
        p = player.second;
        if (p && p->IsInWorld() && p->GetZoneId() == m_session->GetPlayer()->GetZoneId() &&
            ((faction_id == 1 && p->GetTeam() == ALLIANCE) || (faction_id == 2 && p->GetTeam() == HORDE) || faction_id == 0))
            p->SetDisplayId(display_id);
    }

    return true;
}