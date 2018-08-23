#include "Chat.h"
#include "Language.h"

class zone_commandscript : public CommandScript
{
public:
    zone_commandscript() : CommandScript("zone_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> zoneCommandTable =
        {
            { "buff",           SEC_GAMEMASTER3,  false, &HandleZoneBuffCommand,                 "" },
            { "morph",          SEC_GAMEMASTER3,  false, &HandleZoneMorphCommand,                "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "zone",           SEC_GAMEMASTER3,  false, nullptr,                                "", zoneCommandTable },
        };
        return commandTable;
    }

    static bool HandleZoneBuffCommand(ChatHandler* handler, char const* args)
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
            if (p && p->IsInWorld() && p->GetZoneId() == handler->GetSession()->GetPlayer()->GetZoneId())
                p->CastSpell(p, atoi(bufid), true);
        }

        return true;
    }

    static bool HandleZoneMorphCommand(ChatHandler* handler, char const* args)
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
            if (p && p->IsInWorld() && p->GetZoneId() == handler->GetSession()->GetPlayer()->GetZoneId() &&
                ((faction_id == 1 && p->GetTeam() == ALLIANCE) || (faction_id == 2 && p->GetTeam() == HORDE) || faction_id == 0))
                p->SetDisplayId(display_id);
        }

        return true;
    }
};

void AddSC_zone_commandscript()
{
    new zone_commandscript();
}
