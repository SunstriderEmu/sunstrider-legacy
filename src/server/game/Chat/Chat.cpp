
#include "Common.h"
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Chat.h"
#include "MapManager.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ChannelMgr.h"
#include "AccountMgr.h"
#include "LogsDatabaseAccessor.h"
#include "CharacterCache.h"
#include "ScriptMgr.h"
#include <boost/algorithm/string/replace.hpp>

// Lazy loading of the command table cache from commands and the
// ScriptMgr should be thread safe since the player commands,
// cli commands and ScriptMgr updates are all dispatched one after
// one inside the world update loop.
static Optional<std::vector<ChatCommand>> commandTableCache;

std::vector<ChatCommand> const& ChatHandler::getCommandTable()
{
    if (!commandTableCache)
    {
        // We need to initialize this at top since SetDataForCommandInTable
        // calls getCommandTable() recursively.
        commandTableCache = sScriptMgr->GetChatCommands();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_COMMANDS);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                std::string name = fields[0].GetString();

                SetDataForCommandInTable(*commandTableCache, name.c_str(), fields[1].GetUInt8(), fields[2].GetString(), name);
            } while (result->NextRow());
        }
    }

    return *commandTableCache;
}

bool ChatHandler::SetDataForCommandInTable(std::vector<ChatCommand>& table, char const* text, uint32 securityLevel, std::string const& help, std::string const& fullcommand)
{
    std::string cmd = "";

    //get first word in 'cmd'
    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    //make text point on next word
    while (*text == ' ') ++text;

    for (auto & i : table)
    {
        if (!i.Name)
            return false;

        // for data fill use full explicit command names
        if (i.Name != cmd)
            continue;

        // select subcommand from child commands list (including "")
        if (!i.ChildCommands.empty())
        {
            if (SetDataForCommandInTable(i.ChildCommands, text, securityLevel, help, fullcommand))
                return true;
            else if (*text)
                return false;

            // fail with "" subcommands, then use normal level up command instead
        }
        // expected subcommand by full name DB content
        else if (*text)
        {
            TC_LOG_ERROR("sql.sql", "Table `command` have unexpected subcommand '%s' in command '%s', skip.", text, fullcommand.c_str());
            return false;
        }

        if (i.SecurityLevel != securityLevel)
            TC_LOG_DEBUG("misc", "Table `command` overwrite for command '%s' default permission (%u) by %u", fullcommand.c_str(), i.SecurityLevel, securityLevel);

        i.SecurityLevel = securityLevel;
        i.Help = help;
        return true;
    }

    // in case "" command let process by caller
    if (!cmd.empty())
    {
        if (&table == &getCommandTable())
            TC_LOG_ERROR("sql.sql", "Table `command` have not existing command '%s', skip.", cmd.c_str());
        else
            TC_LOG_ERROR("sql.sql", "Table `command` have not existing subcommand '%s' in command '%s', skip.", cmd.c_str(), fullcommand.c_str());
    }
    return true;
}

ChatHandler::ChatHandler(WorldSession* session)
    : m_session(session), sentErrorMessage(false)
{}

ChatHandler::ChatHandler(Player* player)
    : m_session(player->GetSession()), sentErrorMessage(false)
{}

std::string ChatHandler::GetNameLink(Player* chr) const
{
    return playerLink(chr->GetName());
}

void ChatHandler::SendMessageWithoutAuthor(char const* channel, const char* msg)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, LANG_UNIVERSAL, nullptr, nullptr, msg, 0, channel);

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for(auto const & itr : m)
    {
        if (itr.second && itr.second->GetSession()->GetPlayer() && itr.second->GetSession()->GetPlayer()->IsInWorld())
        {
            if(ChannelMgr* cMgr = channelMgr(itr.second->GetSession()->GetPlayer()->GetTeam()))
            {
                if(Channel *chn = cMgr->GetChannel(channel, itr.second->GetSession()->GetPlayer()))
                {
                    itr.second->SendDirectMessage(&data);
                }
            }
        }
    }
}

void ChatHandler::invalidateCommandTable()
{
    commandTableCache.reset();
}

bool ChatHandler::HasLowerSecurity(Player* target, ObjectGuid guid, bool strong)
{
    WorldSession* target_session = nullptr;
    uint32 target_account = 0;

    if (target)
        target_session = target->GetSession();
    else if (guid)
        target_account = sCharacterCache->GetCharacterAccountIdByGuid(guid);

    if (!target_session && !target_account)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return true;
    }

    return HasLowerSecurityAccount(target_session, target_account, strong);
}

bool ChatHandler::HasLowerSecurityAccount(WorldSession* target, uint32 target_account, bool strong)
{
    uint32 target_sec;

    // allow everything from console and RA console
    if (!m_session)
        return false;

    // ignore only for non-players for non strong checks (when allow apply command at least to same sec level)
    /* TC
    if (m_session->HasPermission(rbac::RBAC_PERM_CHECK_FOR_LOWER_SECURITY) && !strong && !sWorld->getBoolConfig(CONFIG_GM_LOWER_SECURITY))
        return false;
    */

    if (target)
        target_sec = target->GetSecurity();
    else if (target_account)
        target_sec = AccountMgr::GetSecurity(target_account, realm.Id.Realm);
    else
        return true;                                        // caller must report error for (target == nullptr && target_account == 0)

    AccountTypes target_ac_sec = AccountTypes(target_sec);
    if (m_session->GetSecurity() < target_ac_sec || (strong && m_session->GetSecurity() <= target_ac_sec))
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return true;
    }

    return false;
}

const char* ChatHandler::GetTrinityString(int32 entry) const
{
    if (m_session)
        return m_session->GetTrinityString(entry);
    else
        return sObjectMgr->GetTrinityString(entry, sWorld->GetDefaultDbcLocale());
}

std::string ChatHandler::GetTrinityStringVA(int32 entry, ...) const
{
    const char *format = GetTrinityString(entry);
    va_list ap;
    char str [1024];
    va_start(ap, entry);
    vsnprintf(str, 1024, format, ap );
    va_end(ap);
    return str;
}

bool ChatHandler::isAvailable(ChatCommand const& cmd) const
{
    return m_session->GetSecurity() >= cmd.SecurityLevel;
}

bool ChatHandler::hasStringAbbr(const char* name, const char* part) const
{
    if (!name)
        return false;

    // non "" command
    if( *name )
    {
        // "" part from non-"" command
        if( !*part )
            return false;

        for(;;)
        {
            if( !*part )
                return true;
            else if( !*name )
                return false;
            else if( tolower( *name ) != tolower( *part ) )
                return false;
            ++name; ++part;
        }
    }
    // allow with any for ""

    return true;
}

void ChatHandler::SendSysMessage(const char *str, bool escapeCharacters)
{
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf;
    char* pos;

    if (escapeCharacters && strchr(str, '|'))
    {
        size_t startPos = 0;
        std::ostringstream o;
        while (const char* charPos = strchr(str + startPos, '|'))
        {
            o.write(str + startPos, charPos - str - startPos);
            o << "||";
            startPos = charPos - str + 1;
        }
        o.write(str + startPos, strlen(str) - startPos);
        buf = strdup(o.str().c_str());
    }
    else
    {
        buf = strdup(str);
    }

    pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        m_session->SendPacket(&data);
    }

    free(buf);
}

void ChatHandler::SendGlobalSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(str);
    char* pos = buf;

    while(char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        sWorld->SendGlobalMessage(&data);
    }

    free(buf);
}

void ChatHandler::SendGlobalGMSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(str);
    char* pos = buf;

    while(char* line = LineFromMessage(pos))
    {
        BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        sWorld->SendGlobalGMMessage(&data);
    }
    free(buf);
}

void ChatHandler::SendSysMessage(int32 entry)
{
    SendSysMessage(GetTrinityString(entry));
}

bool ChatHandler::ExecuteCommandInTable(std::vector<ChatCommand> const& table, const char* text, const std::string& fullcmd)
{
    char const* oldtext = text;
    std::string cmd = "";

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    for (uint32 i = 0; i < table.size(); ++i)
    {
        if( !hasStringAbbr(table[i].Name, cmd.c_str()) )
            continue;

        bool match = false;
        if (strlen(table[i].Name) > cmd.length())
        {
            for (const auto & j : table)
            {
                if (!hasStringAbbr(j.Name, cmd.c_str()))
                    continue;

                if (strcmp(j.Name, cmd.c_str()) == 0)
                {
                    match = true;
                    break;
                }
            }
        }
        if (match)
            continue;

        // select subcommand from child commands list
        if(!table[i].ChildCommands.empty())
        {
            if(!ExecuteCommandInTable(table[i].ChildCommands, text, fullcmd))
            {
                if(text[0] != '\0')
                    SendSysMessage(LANG_NO_SUBCMD);
                else
                    SendSysMessage(LANG_CMD_SYNTAX);

                ShowHelpForCommand(table[i].ChildCommands,text);
            }

            return true;
        }

        // must be available and have handler
        if(!table[i].HasHandler() || !isAvailable(table[i]))
            continue;

        SetSentErrorMessage(false);
        // table[i].Name == "" is special case: send original command to handler
        if (table[i](this, table[i].Name[0] != '\0' ? text : oldtext))
        {
            //log command
            Unit const* target = m_session ? (m_session->GetPlayer() ? m_session->GetPlayer()->GetSelectedUnit() : nullptr) : nullptr;
            LogsDatabaseAccessor::GMCommand(m_session, target, fullcmd);

            if (!m_session) // ignore console
                return true;

            Player* player = m_session->GetPlayer();
            if (!AccountMgr::IsPlayerAccount(m_session->GetSecurity()))
            {
                ObjectGuid guid = player->GetTarget();
                uint32 areaId = player->GetAreaId();
                std::string areaName = "Unknown";
                std::string zoneName = "Unknown";
                if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId))
                {
                    int locale = GetSessionDbcLocale();
                    areaName = area->area_name[locale];
                    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(area->zone))
                        zoneName = zone->area_name[locale];
                }

                sLog->outCommand(m_session->GetAccountId(), "Command: %s [Player: %s (%s) (Account: %u) X: %f Y: %f Z: %f Map: %u (%s) Area: %u (%s) Zone: %s Selected: %s (%s)]",
                    fullcmd.c_str(), player->GetName().c_str(), player->GetGUID().ToString().c_str(),
                    m_session->GetAccountId(), player->GetPositionX(), player->GetPositionY(),
                    player->GetPositionZ(), player->GetMapId(),
                    player->FindMap() ? player->FindMap()->GetMapName() : "Unknown",
                    areaId, areaName.c_str(), zoneName.c_str(),
                    (player->GetSelectedUnit()) ? player->GetSelectedUnit()->GetName().c_str() : "",
                    guid.ToString().c_str());
            }
        }
        // some commands have custom error messages. Don't send the default one in these cases.
        else if(!sentErrorMessage)
        {
            if(!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());
            else
                SendSysMessage(LANG_CMD_SYNTAX);

            SetSentErrorMessage(true);
        }

        return true;
    }

    return false;
}

bool ChatHandler::_ParseCommands(char const* text)
{
    if (ExecuteCommandInTable(getCommandTable(), text, text))
        return true;

    // Pretend commands don't exist for regular players
    if (m_session && m_session->GetSecurity() == SEC_PLAYER) // !m_session->HasPermission(rbac::RBAC_PERM_COMMANDS_NOTIFY_COMMAND_NOT_FOUND_ERROR))
        return false;

    // Send error message for GMs
    SendSysMessage(LANG_NO_CMD);
    SetSentErrorMessage(true);
    return true;
}

bool ChatHandler::ParseCommands(const char* text)
{
    ASSERT(text);
    ASSERT(*text);

    /// chat case (.command or !command format)
    if(text[0] != '!' && text[0] != '.')
        return 0;

    /// ignore single . or ! in line
    if (!text[1])
        return false;
    // original `text` can't be used. It content destroyed in command code processing.

    /// ignore messages starting with many . or !
    if (text[1] == '!' || text[1] == '.')
        return 0;

    return _ParseCommands(text + 1); /// skip first . or !
}

bool ChatHandler::ShowHelpForSubCommands(std::vector<ChatCommand> const& table, char const* cmd, char const* subcmd)
{
    std::string list;
    for (const auto & i : table)
    {
        // must be available (ignore handler existence for show command with possibe avalable subcomands
        if(!isAvailable(i))
            continue;

        // for empty subcmd show all available
        if( *subcmd && !hasStringAbbr(i.Name, subcmd))
            continue;

        if(m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += i.Name;

        if(!i.ChildCommands.empty())
            list += " ...";
    }

    if(list.empty())
        return false;

    if (&table == &getCommandTable())
    {
        SendSysMessage(LANG_AVAILABLE_CMD);
        PSendSysMessage("%s",list.c_str());
    }
    else
        PSendSysMessage(LANG_SUBCMDS_LIST,cmd,list.c_str());

    return true;
}

bool ChatHandler::ShowHelpForCommand(std::vector<ChatCommand> const& table, const char* cmd)
{
    if(*cmd)
    {
        for (const auto & i : table)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if(!isAvailable(i))
                continue;

            if( !hasStringAbbr(i.Name, cmd) )
                continue;

            // have subcommand
            char const* subcmd = (*cmd) ? strtok(nullptr, " ") : "";

            if(!i.ChildCommands.empty() && subcmd && *subcmd)
            {
                if(ShowHelpForCommand(i.ChildCommands, subcmd))
                    return true;
            }

            if(!i.Help.empty())
                SendSysMessage(i.Help.c_str());

            if(!i.ChildCommands.empty())
                if(ShowHelpForSubCommands(i.ChildCommands,i.Name,subcmd ? subcmd : ""))
                    return true;

            return !i.Help.empty();
        }
    }
    else
    {
        for (const auto & i : table)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if(!isAvailable(i))
                continue;

            if(strlen(i.Name))
                continue;

            if(!i.Help.empty())
                SendSysMessage(i.Help.c_str());

            if(!i.ChildCommands.empty())
                if(ShowHelpForSubCommands(i.ChildCommands,"",""))
                    return true;

            return !i.Help.empty();
        }
    }

    return ShowHelpForSubCommands(table,"",cmd);
}

size_t ChatHandler::BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, ObjectGuid senderGUID, ObjectGuid receiverGUID, std::string const& message, uint8 chatTag,
                                  std::string const& senderName /*= ""*/, std::string const& receiverName /*= ""*/,
                                  uint32 achievementId /*= 0*/, bool gmMessage /*= false*/, std::string const& channelName /*= ""*/)
{

#ifndef LICH_KING
    gmMessage = false;  // SMSG_GM_MESSAGECHAT seems to have a different structure on BC, disable it for now
#endif
    size_t receiverGUIDPos = 0;
    data.Initialize(!gmMessage ? SMSG_MESSAGECHAT : SMSG_GM_MESSAGECHAT);
    data << uint8(chatType);
    data << int32(language);
    data << uint64(senderGUID);
    data << uint32(0); // some flags
    switch (chatType)
    {
        case CHAT_MSG_MONSTER_SAY:
        case CHAT_MSG_MONSTER_PARTY:
        case CHAT_MSG_MONSTER_YELL:
        case CHAT_MSG_MONSTER_WHISPER:
        case CHAT_MSG_MONSTER_EMOTE:
        case CHAT_MSG_RAID_BOSS_EMOTE:
        case CHAT_MSG_RAID_BOSS_WHISPER:
#ifdef LICH_KING
        case CHAT_MSG_BATTLENET:
#endif
            data << uint32(senderName.length() + 1);
            data << senderName;
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            if (receiverGUID && !receiverGUID.IsPlayer() && !receiverGUID.IsPet())
            {
                data << uint32(receiverName.length() + 1);
                data << receiverName;
            }
            break;
#ifdef LICH_KING
        case CHAT_MSG_WHISPER_FOREIGN:
            data << uint32(senderName.length() + 1);
            data << senderName;
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            break;
#endif
        case CHAT_MSG_BG_SYSTEM_NEUTRAL:
        case CHAT_MSG_BG_SYSTEM_ALLIANCE:
        case CHAT_MSG_BG_SYSTEM_HORDE:
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            if (receiverGUID && !receiverGUID.IsPlayer())
            {
                data << uint32(receiverName.length() + 1);
                data << receiverName;
            }
            break;
#ifdef LICH_KING
        case CHAT_MSG_ACHIEVEMENT:
        case CHAT_MSG_GUILD_ACHIEVEMENT:
            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            break;
#endif
        default:
            if (gmMessage)
            {
                data << uint32(senderName.length() + 1);
                data << senderName;
            }

            if (chatType == CHAT_MSG_CHANNEL)
            {
                ASSERT(channelName.length() > 0);
                data << channelName;
            }

            receiverGUIDPos = data.wpos();
            data << uint64(receiverGUID);
            break;
    }

    data << uint32(message.length() + 1);
    data << message;
    data << uint8(chatTag);

#ifdef LICH_KING
    if (chatType == CHAT_MSG_ACHIEVEMENT || chatType == CHAT_MSG_GUILD_ACHIEVEMENT)
        data << uint32(achievementId);
#endif

    return receiverGUIDPos;
}

size_t ChatHandler::BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, WorldObject const* sender, WorldObject const* receiver, std::string const& message,
                                  uint32 achievementId /*= 0*/, std::string const& channelName /*= ""*/, LocaleConstant locale /*= DEFAULT_LOCALE*/)
{
    ObjectGuid senderGUID;
    std::string senderName = "";
    uint8 chatTag = 0;
    bool gmMessage = false;
    ObjectGuid receiverGUID;
    std::string receiverName = "";
    if (sender)
    {
        senderGUID = sender->GetGUID();
        senderName = sender->GetNameForLocaleIdx(locale);
        if (Player const* playerSender = sender->ToPlayer())
        {
            chatTag = playerSender->GetChatTag();

            //TC RBAC gmMessage = playerSender->GetSession()->HasPermission(rbac::RBAC_PERM_COMMAND_GM_CHAT);
            gmMessage = playerSender->IsGMChat();
        }
    }

    if (receiver)
    {
        receiverGUID = receiver->GetGUID();
        receiverName = receiver->GetNameForLocaleIdx(locale);
    }

    return BuildChatPacket(data, chatType, language, senderGUID, receiverGUID, message, chatTag, senderName, receiverName, achievementId, gmMessage, channelName);
}

Player * ChatHandler::GetSelectedPlayer() const
{
    if(!m_session)
        return nullptr;

    ObjectGuid guid  = m_session->GetPlayer()->GetTarget();

    if (guid == 0)
        return m_session->GetPlayer();

    return ObjectAccessor::FindPlayer(guid);
}

Player* ChatHandler::GetSelectedPlayerOrSelf() const
{
    if (!m_session)
        return nullptr;

    ObjectGuid selected = m_session->GetPlayer()->GetTarget();
    if (!selected)
        return m_session->GetPlayer();

    // first try with selected target
    Player* targetPlayer = ObjectAccessor::FindPlayer(selected);
    // if the target is not a player, then return self
    if (!targetPlayer)
        targetPlayer = m_session->GetPlayer();

    return targetPlayer;
}

Unit* ChatHandler::GetSelectedUnit() const
{
    if(!m_session)
        return nullptr;

    ObjectGuid guid = m_session->GetPlayer()->GetTarget();

    if (guid == 0)
        return m_session->GetPlayer();

    return ObjectAccessor::GetUnit(*m_session->GetPlayer(),guid);
}

Creature* ChatHandler::GetSelectedCreature() const
{
    if(!m_session)
        return nullptr;

    return ObjectAccessor::GetCreatureOrPetOrVehicle(*m_session->GetPlayer(),m_session->GetPlayer()->GetTarget());
}

char* ChatHandler::extractKeyFromLink(char* text, char const* linkType, char** something1)
{
    // skip empty
    if(!text)
        return nullptr;

    // skip spaces
    while(*text==' '||*text=='\t'||*text=='\b')
        ++text;

    if(!*text)
        return nullptr;

    // return non link case
    if(text[0]!='|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if(!check)
        return nullptr;                                        // end of data

    char* cLinkType = strtok(nullptr, ":");                    // linktype
    if(!cLinkType)
        return nullptr;                                        // end of data

    if(strcmp(cLinkType,linkType) != 0)
    {
        strtok(nullptr, " ");                                  // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
        SendSysMessage(LANG_WRONG_LINK_TYPE);
        return nullptr;
    }

    char* cKeys = strtok(nullptr, "|");                        // extract keys and values
    char* cKeysTail = strtok(nullptr, "");

    char* cKey = strtok(cKeys, ":|");                       // extract key
    if(something1)
        *something1 = strtok(nullptr, ":|");                   // extract something

    strtok(cKeysTail, "]");                                 // restart scan tail and skip name with possible spaces
    strtok(nullptr, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
    return cKey;
}

char* ChatHandler::extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1)
{
    // skip empty
    if(!text)
        return nullptr;

    // skip spaces
    while(*text==' '||*text=='\t'||*text=='\b')
        ++text;

    if(!*text)
        return nullptr;

    // return non link case
    if(text[0]!='|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if(!check)
        return nullptr;                                        // end of data

    char* cLinkType = strtok(nullptr, ":");                    // linktype
    if(!cLinkType)
        return nullptr;                                        // end of data

    for(int i = 0; linkTypes[i]; ++i)
    {
        if(strcmp(cLinkType,linkTypes[i]) == 0)
        {
            char* cKeys = strtok(nullptr, "|");                // extract keys and values
            char* cKeysTail = strtok(nullptr, "");

            char* cKey = strtok(cKeys, ":|");               // extract key
            if(something1)
                *something1 = strtok(nullptr, ":|");           // extract something

            strtok(cKeysTail, "]");                         // restart scan tail and skip name with possible spaces
            strtok(nullptr, " ");                              // skip link tail (to allow continue strtok(NULL,s) use after return from function
            if(found_idx)
                *found_idx = i;
            return cKey;
        }
    }

    strtok(nullptr, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after return from function
    SendSysMessage(LANG_WRONG_LINK_TYPE);
    return nullptr;
}

GameObject* ChatHandler::GetNearbyGameObject()
{
    if (!m_session)
        return nullptr;

    Player* pl = m_session->GetPlayer();
    GameObject* obj = nullptr;
    Trinity::NearestGameObjectCheck check(*pl);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectCheck> searcher(pl, obj, check);
    Cell::VisitGridObjects(pl, searcher, SIZE_OF_GRIDS);
    return obj;
}

GameObject* ChatHandler::GetObjectFromPlayerMapByDbGuid(ObjectGuid::LowType lowguid)
{
    if (!m_session)
        return nullptr;
    auto bounds = m_session->GetPlayer()->GetMap()->GetGameObjectBySpawnIdStore().equal_range(lowguid);
    if (bounds.first != bounds.second)
        return bounds.first->second;
    return nullptr;
}

Creature* ChatHandler::GetCreatureFromPlayerMapByDbGuid(ObjectGuid::LowType lowguid)
{
    if (!m_session)
        return nullptr;
    // Select the first alive creature or a dead one if not found
    Creature* creature = nullptr;
    auto bounds = m_session->GetPlayer()->GetMap()->GetCreatureBySpawnIdStore().equal_range(lowguid);
    for (auto it = bounds.first; it != bounds.second; ++it)
    {
        creature = it->second;
        if (it->second->IsAlive())
            break;
    }
    return creature;
}

static char const* const spellTalentKeys[] = {
    "Hspell",
    "Htalent",
    nullptr
};

uint32 ChatHandler::extractSpellIdFromLink(char* text)
{
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id,rank|h[name]|h|r
    int type = 0;
    char* rankS = nullptr;
    char* idS = extractKeyFromLink(text,spellTalentKeys,&type,&rankS);
    if(!idS)
        return 0;

    uint32 id = (uint32)atol(idS);

    // spell
    if(type==0)
        return id;

    // talent
    TalentEntry const* talentEntry = sTalentStore.LookupEntry(id);
    if(!talentEntry)
        return 0;

    int32 rank = rankS ? (uint32)atol(rankS) : 0;
    if(rank >= 5)
        return 0;

    if(rank < 0)
        rank = 0;

    return talentEntry->RankID[rank];
}

GameTele const* ChatHandler::extractGameTeleFromLink(char* text)
{
    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    char* cId = extractKeyFromLink(text,"Htele");
    if(!cId)
        return nullptr;

    // id case (explicit or from shift link)
    if(cId[0] >= '0' || cId[0] <= '9')
        if(uint32 id = atoi(cId))
            return sObjectMgr->GetGameTele(id);

    return sObjectMgr->GetGameTele(cId);
}

void ChatHandler::extractOptFirstArg(char* args, char** arg1, char** arg2)
{
    char* p1 = strtok(args, " ");
    char* p2 = strtok(nullptr, " ");

    if (!p2)
    {
        p2 = p1;
        p1 = nullptr;
    }

    if (arg1)
        *arg1 = p1;

    if (arg2)
        *arg2 = p2;
}

char* ChatHandler::extractQuotedArg(char* args)
{
    if (!args || !*args)
        return nullptr;

    if (*args == '"')
        return strtok(args + 1, "\"");
    else
    {
        // skip spaces
        while (*args == ' ')
        {
            args += 1;
            continue;
        }

        // return nullptr if we reached the end of the string
        if (!*args)
            return nullptr;

        // since we skipped all spaces, we expect another token now
        if (*args == '"')
        {
            // return an empty string if there are 2 "" in a row.
            // strtok doesn't handle this case
            if (*(args + 1) == '"')
            {
                strtok(args, " ");
                static char arg[1];
                arg[0] = '\0';
                return arg;
            }
            else
                return strtok(args + 1, "\"");
        }
        else
            return nullptr;
    }
}

std::string const ChatHandler::GetName() const
{
    return m_session->GetPlayer()->GetName();
}

bool ChatHandler::needReportToTarget(Player* chr) const
{
    Player* pl = m_session->GetPlayer();
    return pl != chr && pl->IsVisibleGloballyFor(chr);
}

LocaleConstant ChatHandler::GetSessionDbcLocale() const
{
    return m_session ? m_session->GetSessionDbcLocale() : LOCALE_enUS;
}

bool ChatHandler::HasPermission(uint32 permission) const 
{ 
    return m_session->HasPermission(permission); 
}

std::string ChatHandler::GetNameLink() const 
{ 
    return GetNameLink(m_session->GetPlayer()); 
}

const char *CliHandler::GetTrinityString(int32 entry) const
{
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

bool CliHandler::isAvailable(ChatCommand const& cmd) const
{
    // skip non-console commands in console case
    return cmd.AllowConsole;
}

void CliHandler::SendSysMessage(const char *str, bool escapeCharacters)
{
    m_print(m_callbackArg, str);
    m_print(m_callbackArg, "\r\n");
}

bool CliHandler::ParseCommands(char const* str)
{
    if (!str[0])
        return false;
    // Console allows using commands both with and without leading indicator
    if (str[0] == '.' || str[0] == '!')
        ++str;
    return _ParseCommands(str);
}

std::string CliHandler::GetNameLink() const
{
    return GetTrinityString(LANG_CONSOLE_COMMAND);
}

bool CliHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

LocaleConstant CliHandler::GetSessionDbcLocale() const
{
    return sWorld->GetDefaultDbcLocale();
}

bool ChatHandler::GetPlayerGroupAndGUIDByName(const char* cname, Player* &plr, Group* &group, ObjectGuid &guid, bool offline)
{
    plr  = nullptr;
    guid.Clear();

    if(cname)
    {
        std::string name = cname;
        if(!name.empty())
        {
            if(!normalizePlayerName(name))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }

            plr = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
            if(offline)
                guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
        }
    }

    if(plr)
    {
        group = plr->GetGroup();
        if(!guid || !offline)
            guid = plr->GetGUID();
    }
    else
    {
        if(GetSelectedPlayer())
            plr = GetSelectedPlayer();
        else
            plr = m_session->GetPlayer();

        if(!guid || !offline)
            guid  = plr->GetGUID();
        group = plr->GetGroup();
    }

    return true;
}

std::string ChatHandler::extractPlayerNameFromLink(char* text)
{
    // |color|Hplayer:name|h[name]|h|r
    char* name_str = extractKeyFromLink(text, "Hplayer");
    if (!name_str)
        return "";

    std::string name = name_str;
    if (!normalizePlayerName(name))
        return "";

    return name;
}

bool ChatHandler::extractPlayerTarget(char* args, Player** player, ObjectGuid* player_guid /*=nullptr*/, std::string* player_name /*= NULL*/)
{
    if (args && *args)
    {
        std::string name = extractPlayerNameFromLink(args);
        if (name.empty())
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        Player* pl = ObjectAccessor::FindPlayerByName(name.c_str());

        // if allowed player pointer
        if (player)
            *player = pl;

        // if need guid value from DB (in name case for check player existence)
        ObjectGuid guid = !pl && (player_guid || player_name) ? sCharacterCache->GetCharacterGuidByName(name) : ObjectGuid::Empty;

        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : guid;

        if (player_name)
            *player_name = pl || guid ? name : "";
    }
    else
    {
        Player* pl = GetSelectedPlayer();
        // if allowed player pointer
        if (player)
            *player = pl;
        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : ObjectGuid::Empty;

        if (player_name)
            *player_name = pl ? pl->GetName() : "";
    }

    // some from req. data must be provided (note: name is empty if player not exist)
    if ((!player || !*player) && (!player_guid || !*player_guid) && (!player_name || player_name->empty()))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool AddonChannelCommandHandler::ParseCommands(char const* str)
{
    if (memcmp(str, "TrinityCore\t", 12))
        return false;
    char opcode = str[12];
    if (!opcode) // str[12] is opcode
        return false;
    if (!str[13] || !str[14] || !str[15] || !str[16]) // str[13] through str[16] is 4-character command counter
        return false;
    echo = str + 13;

    switch (opcode)
    {
    case 'p': // p Ping
        SendAck();
        return true;
    case 'h': // h Issue human-readable command
    case 'i': // i Issue command
        if (!str[17])
            return false;
        humanReadable = (opcode == 'h');
        if (_ParseCommands(str + 17)) // actual command starts at str[17]
        {
            if (!hadAck)
                SendAck();
            if (HasSentErrorMessage())
                SendFailed();
            else
                SendOK();
        }
        else
        {
            SendSysMessage(LANG_NO_CMD);
            SendFailed();
        }
        return true;
    default:
        return false;
    }
}

void AddonChannelCommandHandler::Send(std::string const& msg)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_ADDON, GetSession()->GetPlayer(), GetSession()->GetPlayer(), msg);
    GetSession()->SendPacket(&data);
}

void AddonChannelCommandHandler::SendAck() // a Command acknowledged, no body
{
    ASSERT(echo);
    char ack[18] = "TrinityCore\ta";
    memcpy(ack + 13, echo, 4);
    ack[17] = '\0';
    Send(ack);
    hadAck = true;
}

void AddonChannelCommandHandler::SendOK() // o Command OK, no body
{
    ASSERT(echo);
    char ok[18] = "TrinityCore\to";
    memcpy(ok + 13, echo, 4);
    ok[17] = '\0';
    Send(ok);
}

void AddonChannelCommandHandler::SendFailed() // f Command failed, no body
{
    ASSERT(echo);
    char fail[18] = "TrinityCore\tf";
    memcpy(fail + 13, echo, 4);
    fail[17] = '\0';
    Send(fail);
}

// m Command message, message in body
void AddonChannelCommandHandler::SendSysMessage(char const* str, bool escapeCharacters)
{
    ASSERT(echo);
    if (!hadAck)
        SendAck();

    std::string msg = "TrinityCore\tm";
    msg.append(echo, 4);
    std::string body(str);
    if (escapeCharacters)
        boost::replace_all(body, "|", "||");
    size_t pos, lastpos;
    for (lastpos = 0, pos = body.find('\n', lastpos); pos != std::string::npos; lastpos = pos + 1, pos = body.find('\n', lastpos))
    {
        std::string line(msg);
        line.append(body, lastpos, pos - lastpos);
        Send(line);
    }
    msg.append(body, lastpos, pos - lastpos);
    Send(msg);
}
