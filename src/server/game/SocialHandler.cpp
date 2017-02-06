#include "WorldSession.h"
#include "Player.h"
#include "SocialMgr.h"
#include "Language.h"
#include "AccountMgr.h"
#include "CharacterCache.h"

void WorldSession::HandleContactListOpcode( WorldPacket & recvData )
{
    uint32 unk;
    recvData >> unk;
    // TC_LOG_DEBUG("network", "WORLD: Received CMSG_CONTACT_LIST - Unk: %d", unk);
    _player->GetSocial()->SendSocialList();
}

void WorldSession::HandleAddFriendOpcode(WorldPacket& recvData)
{
    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_ADD_FRIEND");

    std::string friendName = GetTrinityString(LANG_FRIEND_IGNORE_UNKNOWN);
    std::string friendNote;

    recvData >> friendName;
    recvData >> friendNote;

    if (!normalizePlayerName(friendName))
        return;

    TC_LOG_DEBUG("network", "WORLD: %s asked to add friend : '%s'",
        GetPlayer()->GetName().c_str(), friendName.c_str());

    FriendsResult friendResult = FRIEND_NOT_FOUND;

    uint32 friendGuid = sCharacterCache->GetCharacterGuidByName(friendName);
    if (friendGuid)
    {
        if (CharacterCacheEntry const* characterInfo = sCharacterCache->GetCharacterCacheByGuid(friendGuid))
        {
            uint32 team = Player::TeamForRace(characterInfo->race);
            uint32 friendAccountId = characterInfo->accountId;

            if (GetSecurity() >= SEC_GAMEMASTER1 || sWorld->getConfig(CONFIG_ALLOW_GM_FRIEND) || sAccountMgr->GetSecurity(friendAccountId) < SEC_GAMEMASTER1)
            {
                if (friendGuid == GetPlayer()->GetGUID())
                    friendResult = FRIEND_SELF;
                else if (GetPlayer()->GetTeam() != team && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND) && GetSecurity() < SEC_GAMEMASTER1)
                    friendResult = FRIEND_ENEMY;
                else if (GetPlayer()->GetSocial()->HasFriend(GUID_LOPART(friendGuid)))
                    friendResult = FRIEND_ALREADY;
                else
                {
                    Player* pFriend = ObjectAccessor::FindPlayer(friendGuid);
                    if (pFriend && pFriend->IsInWorld() && pFriend->IsVisibleGloballyFor(GetPlayer()))
                        friendResult = FRIEND_ADDED_ONLINE;
                    else
                        friendResult = FRIEND_ADDED_OFFLINE;
                    if (!GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(friendGuid), false))
                    {
                        friendResult = FRIEND_LIST_FULL;
                        TC_LOG_DEBUG("network", "WORLD: %s's friend list is full.", GetPlayer()->GetName().c_str());
                    }
                }
                GetPlayer()->GetSocial()->SetFriendNote(GUID_LOPART(friendGuid), friendNote);
            }
        }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), friendResult, GUID_LOPART(friendGuid), false);
}

void WorldSession::HandleDelFriendOpcode( WorldPacket & recvData )
{
    uint64 friendGUID;
    recvData >> friendGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(friendGUID), false);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_REMOVED, GUID_LOPART(friendGUID), false);
}

void WorldSession::HandleAddIgnoreOpcode( WorldPacket & recvData )
{
    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_ADD_IGNORE");

    std::string ignoreName = GetTrinityString(LANG_FRIEND_IGNORE_UNKNOWN);

    recvData >> ignoreName;

    if (!normalizePlayerName(ignoreName))
        return;

    /*TC_LOG_DEBUG("network", "WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName().c_str(), ignoreName.c_str());*/

    FriendsResult ignoreResult;

    ignoreResult = FRIEND_IGNORE_NOT_FOUND;

    uint32 ignoreGuid = sCharacterCache->GetCharacterGuidByName(ignoreName);
    if (ignoreGuid)
    {
        if (ignoreGuid == GetPlayer()->GetGUID())              //not add yourself
            ignoreResult = FRIEND_IGNORE_SELF;
        else if (GetPlayer()->GetSocial()->HasIgnore(ignoreGuid))
            ignoreResult = FRIEND_IGNORE_ALREADY;
        else
        {
            ignoreResult = FRIEND_IGNORE_ADDED;

            // ignore list full
            if (!GetPlayer()->GetSocial()->AddToSocialList(ignoreGuid, true))
                ignoreResult = FRIEND_IGNORE_FULL;
        }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), ignoreResult, ignoreGuid, false);
}

void WorldSession::HandleDelIgnoreOpcode( WorldPacket & recvData )
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_DEL_IGNORE");

    uint64 ignoreGUID;

    recvData >> ignoreGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(ignoreGUID), true);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_IGNORE_REMOVED, GUID_LOPART(ignoreGUID), false);
}

void WorldSession::HandleSetContactNotesOpcode(WorldPacket & recvData)
{
    uint64 guid;
    std::string note;
    recvData >> guid >> note;

//    TC_LOG_DEBUG("network", "WorldSession::HandleSetContactNotesOpcode: Contact: %s, Notes: %s", guid.ToString().c_str(), note.c_str());

    _player->GetSocial()->SetFriendNote(guid, note);
}
