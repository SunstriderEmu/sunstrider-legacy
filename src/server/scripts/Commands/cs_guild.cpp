#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "Guild.h"
#include "GuildMgr.h"

/** \brief GM command level 3 - Create a guild.
 *
 * This command allows a GM (level 3) to create a guild.
 *
 * The "args" parameter contains the name of the guild leader
 * and then the name of the guild.
 *
 */
static bool HandleGuildCreateCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK

    char *lname = strtok ((char*)args, " ");
    char *gname = strtok (nullptr, "");

    if (!lname)
        return false;

    if (!gname)
    {
        handler->SendSysMessage(LANG_INSERT_GUILD_NAME);
        handler->SetSentErrorMessage(true);
        return false;
    }

    std::string guildname = gname;

    Player* player = ObjectAccessor::FindPlayerByName (lname);
    if (!player)
    {
        handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (player->GetGuildId())
    {
        handler->SendSysMessage (LANG_PLAYER_IN_GUILD);
        return true;
    }

    auto guild = new Guild;
    if (!guild->Create(player, guildname))
    {
        delete guild;
        handler->SendSysMessage(LANG_GUILD_NOT_CREATED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    sGuildMgr->AddGuild(guild);
    return true;
}

static bool HandleGuildInviteCommand(const char *args)
{
    ARGS_CHECK

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok (nullptr, "");
    if(!par1 || !par2)
        return false;

    std::string glName = par2;
    Guild* targetGuild = sGuildMgr->GetGuildByName (glName);
    if (!targetGuild)
        return false;

    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        handler->SendSysMessage (LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    ObjectGuid plGuid;
    if (Player* targetPlayer = ObjectAccessor::FindPlayerByName (plName.c_str ()))
        plGuid = targetPlayer->GetGUID ();
    else
        plGuid = sCharacterCache->GetCharacterGuidByName(plName.c_str ());

    if (!plGuid)
    {
        handler->SendSysMessage("Guild not found");
        handler->SetSentErrorMessage(true);
        return false;
    }

    // player's guild membership checked in AddMember before add
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if (!targetGuild->AddMember(trans, plGuid))
    {
        handler->SendSysMessage("Could not add to guild (Is player already in a guild?)");
        handler->SetSentErrorMessage(true);
        return false;
    }
    CharacterDatabase.CommitTransaction(trans);

    return true;
}

static bool HandleGuildUninviteCommand(const char *args)
{
    ARGS_CHECK

    char* par1 = strtok ((char*)args, " ");
    if(!par1)
        return false;

    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        handler->SendSysMessage (LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage (true);
        return false;
    }

    ObjectGuid plGuid;
    uint32 glId = 0;
    if (Player* targetPlayer = ObjectAccessor::FindPlayerByName (plName.c_str ()))
    {
        plGuid = targetPlayer->GetGUID ();
        glId   = targetPlayer->GetGuildId();
    }
    else
    {
        plGuid = sCharacterCache->GetCharacterGuidByName(plName.c_str ());
        glId = Player::GetGuildIdFromCharacterInfo(plGuid);
    }

    if (!plGuid)
    {
        handler->PSendSysMessage("Could not found any player with name %s", plName.c_str());
        return true;
    }
    if (!glId)
    {
        handler->PSendSysMessage("Could not find any guild for player %s", plName.c_str());
        return true;
    }

    Guild* targetGuild = sGuildMgr->GetGuildById (glId);
    if (!targetGuild)
    {
        handler->SendSysMessage("Could not find guild %u", glId);
        return true;
    }

    SQLTransaction trans(nullptr);
    targetGuild->DeleteMember(trans, plGuid, false, true, true);

    return true;
}

static bool HandleGuildRankCommand(const char *args)
{
    ARGS_CHECK

    char* nameStr;
    char* rankStr;
    extractOptFirstArg((char*)args, &nameStr, &rankStr);
    if (!rankStr)
        return false;

    Player* target;
    ObjectGuid targetGuid;
    std::string target_name;
    if (!extractPlayerTarget(nameStr, &target, &targetGuid, &target_name))
        return false;

    ObjectGuid::LowType guildId = target ? target->GetGuildId() : sCharacterCache->GetCharacterGuildIdByGuid(targetGuid);
    if (!guildId)
        return false;

    Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
    if (!targetGuild)
        return false;

    uint8 newRank = uint8(atoi(rankStr));
    SQLTransaction trans(nullptr);
    return targetGuild->ChangeMemberRank(trans, targetGuid, newRank);
}

static bool HandleGuildDeleteCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok(nullptr, " ");
    if (!par1)
        return false;

    std::string gld = par1;

    Guild* targetGuild = nullptr;
    if (gld == "id") {
        if (!par2)
            return false;
        targetGuild = sGuildMgr->GetGuildById(atoi(par2));
    } else {
        targetGuild = sGuildMgr->GetGuildByName (gld);
    }

    if (!targetGuild)
        return false;


    uint32 GMForceGuildId = sWorld->getIntConfig(CONFIG_GM_FORCE_GUILD);
    if (targetGuild->GetId() == GMForceGuildId)
    {
        handler->SendSysMessage("Cannot delete gm guild");
        return true;
    }

    targetGuild->Disband ();
    handler->PSendSysMessage("Guild deleted.");

    return true;
}

static bool HandleGuildRenameCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK
        
    char* guildIdStr = strtok((char*)args, " ");
    if (!guildIdStr)
        return false;
        
    char* newName = strtok(nullptr, "");
    if (!newName)
        return false;
    
    uint32 guildId = atoi(guildIdStr);
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
    {
        handler->PSendSysMessage("Guild (id %u) not found", guildId);
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (sGuildMgr->GetGuildByName(newName))
    {
        handler->PSendSysMessage("Guild %s already exists", newName);
        handler->SetSentErrorMessage(true);
        return false;
    }

    std::string oldName = guild->GetName();

    if (guild->SetName(newName))
    {
        handler->PSendSysMessage("Guild (id %u) renamed from %s to %s", guildId, oldName.c_str(), newName);
        return true;
    }
    else {
        handler->PSendSysMessage("Error while renaming guild %i", guildId);
        handler->SetSentErrorMessage(true);
        return false;
    }

    return true;
}
