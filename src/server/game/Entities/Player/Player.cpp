
#include "Common.h"
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateMask.h"
#include "Player.h"
#include "SkillDiscovery.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "UpdateData.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "MapManager.h"
#include "MapInstanced.h"
#include "InstanceSaveMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Guild.h"
#include "Pet.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "GameTime.h"
#include "Util.h"
#include "CharacterCache.h"
#include "Transport.h"
#include "Weather.h"
#include "BattleGround.h"
#include "BattleGroundAV.h"
#include "BattleGroundMgr.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "ArenaTeam.h"
#include "Chat.h"
#include "Spell.h"
#include "LootItemStorage.h"
#include "SocialMgr.h"
#include "GameEventMgr.h"
#include "Config.h"
#include "InstanceScript.h"
#include "ConditionMgr.h"
#include "SpectatorAddon.h"
#include "ScriptMgr.h"
#include "LogsDatabaseAccessor.h"
#include "Mail.h"
#include "Bag.h"
#include "CharacterCache.h"
#include "UpdateFieldFlags.h"
#include "CharacterDatabase.h"
#include "PlayerTaxi.h"
#include "CinematicMgr.h"
#include "PlayerAntiCheat.h"
#include "SpellHistory.h"
#include "TradeData.h"
#include "GuildMgr.h"
#include "ArenaTeamMgr.h"
#include "PetitionMgr.h"
#include "ReputationMgr.h"

#ifdef PLAYERBOT
#include "PlayerbotAI.h"
#include "GuildTaskMgr.h"
#endif

#include <cmath>
#include <setjmp.h>

#define ZONE_UPDATE_INTERVAL 1000

/*
PLAYER_SKILL_INDEX:
    pair of <skillId, isProfession (0|1)> (<low,high>)
PLAYER_SKILL_VALUE_INDEX:
    pair of <value, maxvalue> (<low, high>)
PLAYER_SKILL_BONUS_INDEX
    pair of ? (<low, high>)
*/
#define PLAYER_SKILL_INDEX(x)       (PLAYER_SKILL_INFO_1_1 + ((x)*3))
#define PLAYER_SKILL_VALUE_INDEX(x) (PLAYER_SKILL_INDEX(x)+1)
#define PLAYER_SKILL_BONUS_INDEX(x) (PLAYER_SKILL_INDEX(x)+2)

#define SKILL_VALUE(x)         PAIR32_LOPART(x)
#define SKILL_MAX(x)           PAIR32_HIPART(x)
#define MAKE_SKILL_VALUE(v, m) MAKE_PAIR32(v,m)

#define SKILL_TEMP_BONUS(x)    int16(PAIR32_LOPART(x))
#define SKILL_PERM_BONUS(x)    int16(PAIR32_HIPART(x))
#define MAKE_SKILL_BONUS(t, p) MAKE_PAIR32(t,p)

#ifdef UNIX
jmp_buf __jmp_env;
void __segv_handler(int)
{
    siglongjmp(__jmp_env, 1);
}
#endif

enum CharacterFlags
{
    CHARACTER_FLAG_NONE                 = 0x00000000,
    CHARACTER_FLAG_UNK1                 = 0x00000001,
    CHARACTER_FLAG_UNK2                 = 0x00000002,
    CHARACTER_LOCKED_FOR_TRANSFER       = 0x00000004,
    CHARACTER_FLAG_UNK4                 = 0x00000008,
    CHARACTER_FLAG_UNK5                 = 0x00000010,
    CHARACTER_FLAG_UNK6                 = 0x00000020,
    CHARACTER_FLAG_UNK7                 = 0x00000040,
    CHARACTER_FLAG_UNK8                 = 0x00000080,
    CHARACTER_FLAG_UNK9                 = 0x00000100,
    CHARACTER_FLAG_UNK10                = 0x00000200,
    CHARACTER_FLAG_HIDE_HELM            = 0x00000400,
    CHARACTER_FLAG_HIDE_CLOAK           = 0x00000800,
    CHARACTER_FLAG_UNK13                = 0x00001000,
    CHARACTER_FLAG_GHOST                = 0x00002000,
    CHARACTER_FLAG_RENAME               = 0x00004000,
    CHARACTER_FLAG_UNK16                = 0x00008000,
    CHARACTER_FLAG_UNK17                = 0x00010000,
    CHARACTER_FLAG_UNK18                = 0x00020000,
    CHARACTER_FLAG_UNK19                = 0x00040000,
    CHARACTER_FLAG_UNK20                = 0x00080000,
    CHARACTER_FLAG_UNK21                = 0x00100000,
    CHARACTER_FLAG_UNK22                = 0x00200000,
    CHARACTER_FLAG_UNK23                = 0x00400000,
    CHARACTER_FLAG_UNK24                = 0x00800000,
    CHARACTER_FLAG_LOCKED_BY_BILLING    = 0x01000000,
    CHARACTER_FLAG_DECLINED             = 0x02000000,
    CHARACTER_FLAG_UNK27                = 0x04000000,
    CHARACTER_FLAG_UNK28                = 0x08000000,
    CHARACTER_FLAG_UNK29                = 0x10000000,
    CHARACTER_FLAG_UNK30                = 0x20000000,
    CHARACTER_FLAG_UNK31                = 0x40000000,
    CHARACTER_FLAG_UNK32                = 0x80000000
};

#ifdef LICH_LING
enum CharacterCustomizeFlags
{
    CHAR_CUSTOMIZE_FLAG_NONE            = 0x00000000,
    CHAR_CUSTOMIZE_FLAG_CUSTOMIZE       = 0x00000001,       // name, gender, etc...
    CHAR_CUSTOMIZE_FLAG_FACTION         = 0x00010000,       // name, gender, faction, etc...
    CHAR_CUSTOMIZE_FLAG_RACE            = 0x00100000        // name, gender, race, etc...
};
#endif
// corpse reclaim times
#define DEATH_EXPIRE_STEP (5*MINUTE)
#define MAX_DEATH_COUNT 3

static uint32 constexpr copseReclaimDelay[MAX_DEATH_COUNT] = { 30, 60, 120 };

//== Player ====================================================

uint32 const MAX_MONEY_AMOUNT = static_cast<uint32>(std::numeric_limits<int32>::max());

Player::Player(WorldSession *session) :
    Unit(true),
    m_bHasDelayedTeleport(false),
    m_bCanDelayTeleport(false),
    m_DelayedOperations(0),
    m_hasMovedInUpdate(false),
    m_seer(this),
    m_needsZoneUpdate(false),
    m_teleportToTestInstanceId(0),
    m_speakTime(0),
    m_speakCount(0),
    m_session(session),
    m_ExtraFlags(0),
    m_comboPoints(0),
    m_usedTalentCount(0),
    m_regenTimerCount(0),
    m_weaponChangeTimer(0),
    m_zoneUpdateId(MAP_INVALID_ZONE),
    m_zoneUpdateTimer(0),
    m_areaUpdateId(0),
    m_spellModTakingSpell(nullptr),
    m_trade(nullptr),
    m_sharedQuestId(0)
{
    m_objectType |= TYPEMASK_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;

    m_valuesCount = PLAYER_END;

    // players always accept
    if(GetSession()->GetSecurity() == SEC_PLAYER/* && !(GetSession()->GetGroupId()) */)
        SetAcceptWhispers(true);

    m_nextSave = sWorld->getConfig(CONFIG_INTERVAL_SAVE);

    clearResurrectRequestData();

    memset(m_items, 0, sizeof(Item*)*PLAYER_SLOTS_COUNT);

    m_social = nullptr;

    // group is initialized in the reference constructor
    SetGroupInvite(nullptr);
    m_groupUpdateMask = 0;
    m_auraUpdateMask = 0;

    m_ControlledByPlayer = true;

    _guildIdInvited = 0;
    _arenaTeamIdInvited = 0;

    m_atLoginFlags = AT_LOGIN_NONE;

    mSemaphoreTeleport_Near = false;
    mSemaphoreTeleport_Far = false;

    m_playerMovingMe = nullptr;

    m_cinematic = 0;
    if (sWorld->getConfig(CONFIG_BETASERVER_ENABLED))
        m_cinematic = true; //no cinematic on beta server, because we alter starting position

    PlayerTalkClass = new PlayerMenu( GetSession() );
    m_currentBuybackSlot = BUYBACK_SLOT_START;

    m_DailyQuestChanged = false;
    m_lastDailyQuestTime = 0;

    for (int & i : m_MirrorTimer)
        i = DISABLED_MIRROR_TIMER;

    m_MirrorTimerFlags = UNDERWATER_NONE;
    m_MirrorTimerFlagsLast = UNDERWATER_NONE;
    m_isInWater = false;
    m_hostileReferenceCheckTimer = 0;
    m_drunkTimer = 0;
    m_drunk = 0;
    m_restTime = 0;
    m_deathTimer = 0;
    m_deathExpireTime = 0;
    m_isRepopPending = false;

    m_swingErrorMsg = 0;

    for (uint8 j = 0; j < PLAYER_MAX_BATTLEGROUND_QUEUES; ++j)
    {
        m_bgBattlegroundQueueID[j].bgQueueTypeId = BATTLEGROUND_QUEUE_NONE;
        m_bgBattlegroundQueueID[j].invitedToInstance = 0;
    }
    m_bgTeam = 0;

    m_logintime = WorldGameTime::GetGameTime();
    m_Last_tick = m_logintime;
    m_WeaponProficiency = 0;
    m_ArmorProficiency = 0;
    m_canParry = false;
    m_canBlock = false;
    m_canDualWield = false;
    m_ammoDPS = 0.0f;

    m_temporaryUnsummonedPetNumber = 0;
    //cache for UNIT_CREATED_BY_SPELL to allow
    //returning reagents for temporarily removed pets
    //when dying/logging out
    m_oldpetspell = 0;

    ////////////////////Rest System/////////////////////
    time_inn_enter=0;
    inn_pos_mapid=0;
    inn_pos_x=0;
    inn_pos_y=0;
    inn_pos_z=0;
    m_rest_bonus=0;
    rest_type=REST_TYPE_NO;
    ////////////////////Rest System/////////////////////

    m_mailsLoaded = false;
    m_mailsUpdated = false;
    unReadMails = 0;
    m_nextMailDelivereTime = 0;

    m_resetTalentsCost = 0;
    m_resetTalentsTime = 0;
    m_itemUpdateQueueBlocked = false;

    for (unsigned char & m_forced_speed_change : m_forced_speed_changes)
        m_forced_speed_change = 0;

    m_stableSlots = 0;

    /////////////////// Instance System /////////////////////

    m_HomebindTimer = 0;
    m_InstanceValid = true;
    m_dungeonDifficulty = DUNGEON_DIFFICULTY_NORMAL;

  /* TC
    for (uint8 i = 0; i < MAX_TALENT_SPECS; ++i)
    {
#ifdef LICH_KING
        for (uint8 g = 0; g < MAX_GLYPH_SLOT_INDEX; ++g)
            m_Glyphs[i][g] = 0;
#endif

        m_talents[i] = new PlayerTalentMap();
    }
    */

    for (uint8 i = 0; i < BASEMOD_END; ++i)
    {
        m_auraBaseFlatMod[i] = 0.0f;
        m_auraBasePctMod[i]  = 1.0f;
    }

    // Honor System
    m_lastHonorUpdateTime = WorldGameTime::GetGameTime();

    // Player summoning
    m_summon_expire = 0;
    m_summon_mapid = 0;
    m_summon_x = 0.0f;
    m_summon_y = 0.0f;
    m_summon_z = 0.0f;
    m_invite_summon = false;

    m_bgAfkReportedTimer = 0;
    m_contestedPvPTimer = 0;

    m_declinedname = nullptr;

    m_isActive = true;

    _activeCheats = CHEAT_NONE;

    for (uint8 i = 0; i < MAX_COMBAT_RATING; i++)
        m_baseRatingValue[i] = 0;

    #ifdef PLAYERBOT
    // playerbot mod
    m_playerbotAI = nullptr;
    m_playerbotMgr = nullptr;
    #endif

    // Experience Blocking
    m_isXpBlocked = false;

    m_kickatnextupdate = false;
    m_swdBackfireDmg = 0;

    m_ConditionErrorMsgId = 0;

    m_lastOpenLockKey = 0;

    _attackersCheckTime = 0;

    m_bPassOnGroupLoot = false;

    spectatorFlag = false;
    spectateCanceled = false;
    spectateFrom = nullptr;

    _lastSpamAlert = 0;
    lastLagReport = 0;

    _cinematicMgr = new CinematicMgr(this);
    m_reputationMgr = new ReputationMgr(this);
}

Player::~Player()
{
    // it must be unloaded already in PlayerLogout and accessed only for loggined player
    //m_social = nullptr;

    if (m_playerMovingMe)
        m_playerMovingMe->ResetActiveMover(true);

    // Note: buy back item already deleted from DB when player was saved
    for(auto & m_item : m_items)
         delete m_item;
    CleanupChannels();

    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
        delete itr->second;

#ifdef LICH_KING
    for (uint8 i = 0; i < MAX_TALENT_SPECS; ++i)
    {
        for (PlayerTalentMap::const_iterator itr = m_talents[i]->begin(); itr != m_talents[i]->end(); ++itr)
            delete itr->second;

        delete m_talents[i];
    }
#endif

    //all mailed items should be deleted, also all mail should be deallocated
    for (auto & itr : m_mail)
        delete itr;

    for (auto & mMitem : mMitems)
        delete mMitem.second;                                //if item is duplicated... then server may crash ... but that item should be deallocated

    delete PlayerTalkClass;

    for(auto & x : ItemSetEff)
        if(x)
            delete x;

    delete m_declinedname;
    delete _cinematicMgr;
    delete m_reputationMgr;

#ifdef PLAYERBOT
    delete m_playerbotAI;
    delete m_playerbotMgr;
#endif

    //TC sWorld->DecreasePlayerCount();
}

void Player::CleanupsBeforeDelete(bool finalCleanup)
{
    if(m_uint32Values)                                      // only for fully created Object
    {
        TradeCancel(false);
        DuelComplete(DUEL_INTERRUPTED);
    }

    Unit::CleanupsBeforeDelete(finalCleanup);

    // clean up player-instance binds, may unload some instance saves
    for (auto & m_boundInstance : m_boundInstances)
        for (auto & itr : m_boundInstance)
            itr.second.save->RemovePlayer(this);
}

bool Player::Create(ObjectGuid::LowType guidlow, CharacterCreateInfo* createInfo)
{
    return Create(guidlow, createInfo->Name, createInfo->Race, createInfo->Class, createInfo->Gender, createInfo->Skin, createInfo->Face, createInfo->HairStyle, createInfo->HairColor, createInfo->FacialHair, createInfo->OutfitId);
}

void Player::SetMapAtCreation(PlayerInfo const* info)
{
    if (sWorld->getConfig(CONFIG_BETASERVER_ENABLED))
        RelocateToBetaZone();
    else
    {
        Relocate(info->positionX, info->positionY, info->positionZ, info->positionO);
        SetMap(sMapMgr->CreateMap(info->mapId, this));
    }

    UpdatePositionData();
}

bool Player::Create(ObjectGuid::LowType guidlow, const std::string& name, uint8 race, uint8 class_, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair, uint8 outfitId)
{
    //FIXME: outfitId not used in player creating

    Object::_Create(guidlow, 0, HighGuid::Player);

    m_name = name;

    PlayerInfo const* info = sObjectMgr->GetPlayerInfo(race, class_);
    if (!info)
    {
        TC_LOG_ERROR("entities.player", "Player have incorrect race/class pair. Can't be loaded.");
        return false;
    }

    if (!ValidateAppearance(race, class_, gender, hairStyle, hairColor, face, facialHair, skin, true))
    {
        TC_LOG_ERROR("entities.player.cheat", "Player::Create: Possible hacking attempt: Account %u tried to create a character named '%s' with invalid appearance attributes - refusing to do so",
            GetSession()->GetAccountId(), m_name.c_str());
        return false;
    }

    for (auto & m_item : m_items)
        m_item = nullptr;

    m_race = race;
    m_class = class_;
    m_gender = gender;
    
    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(class_);
    if(!cEntry)
    {
        TC_LOG_ERROR("entities.player","Class %u not found in DBC (Wrong DBC files?)",class_);
        return false;
    }

    SetMapAtCreation(info);

    uint8 powertype = cEntry->PowerType;

    uint32 unitfield;

    switch(powertype)
    {
        case POWER_ENERGY:
        case POWER_MANA:
            unitfield = 0x00000000;
            break;
        case POWER_RAGE:
            unitfield = 0x00110000;
            break;
        default:
            TC_LOG_ERROR("entities.player","Invalid default powertype %u for player (class %u)",powertype,class_);
            return false;
    }

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_PLAYER_BOUNDING_RADIUS );
    SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_PLAYER_COMBAT_REACH );

    switch(gender)
    {
        case GENDER_FEMALE:
            SetDisplayId(info->displayId_f );
            SetNativeDisplayId(info->displayId_f );
            break;
        case GENDER_MALE:
            SetDisplayId(info->displayId_m );
            SetNativeDisplayId(info->displayId_m );
            break;
        default:
            TC_LOG_ERROR("entities.player","Invalid gender %u for player",gender);
            return false;
            break;
    }

    SetFactionForRace(m_race);

    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE, race);
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, class_);
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, gender);
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_POWER_TYPE, powertype);
    SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield);
#ifndef LICH_KING
    SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_BUFF_LIMIT, UNIT_BYTE2_PLAYER_CONTROLLED_BUFF_LIMIT);
#endif
    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED );
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);               // fix cast time showed in spell tooltip on client

                                                            //-1 is default value
    SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    SetUInt32Value(PLAYER_BYTES, (skin | (face << 8) | (hairStyle << 16) | (hairColor << 24)));
    SetUInt32Value(PLAYER_BYTES_2, (facialHair | (0x00 << 8) | (0x00 << 16) | (0x02 << 24)));
    SetByteValue(PLAYER_BYTES_3, 0, gender);

    SetUInt32Value( PLAYER_GUILDID, 0 );
    SetUInt32Value( PLAYER_GUILDRANK, 0 );
    SetUInt32Value( PLAYER_GUILD_TIMESTAMP, 0 );

    SetUInt64Value( PLAYER_FIELD_KNOWN_TITLES, 0 );        // 0=disabled
    SetUInt32Value( PLAYER_CHOSEN_TITLE, 0 );
    SetUInt32Value( PLAYER_FIELD_KILLS, 0 );
    SetUInt32Value( PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0 );
    SetUInt32Value( PLAYER_FIELD_TODAY_CONTRIBUTION, 0 );
    SetUInt32Value( PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0 );

    // set starting level
    if (GetSession()->GetSecurity() >= SEC_GAMEMASTER1)
        SetUInt32Value (UNIT_FIELD_LEVEL, sWorld->getConfig(CONFIG_START_GM_LEVEL));
    else
        SetUInt32Value (UNIT_FIELD_LEVEL, sWorld->getConfig(CONFIG_START_PLAYER_LEVEL));

    SetUInt32Value (PLAYER_FIELD_COINAGE, sWorld->getConfig(CONFIG_START_PLAYER_MONEY));
    SetUInt32Value (PLAYER_FIELD_HONOR_CURRENCY, sWorld->getConfig(CONFIG_START_HONOR_POINTS));
    SetUInt32Value (PLAYER_FIELD_ARENA_CURRENCY, sWorld->getConfig(CONFIG_START_ARENA_POINTS));

    // start with every map explored
    if(sWorld->getConfig(CONFIG_START_ALL_EXPLORED))
    {
        for (uint8 i=0; i<64; i++)
            SetFlag(PLAYER_EXPLORED_ZONES_1+i,0xFFFFFFFF);
    }

    // Played time
    m_Last_tick = WorldGameTime::GetGameTime();
    m_Played_time[0] = 0;
    m_Played_time[1] = 0;

    // base stats and related field values
    InitStatsForLevel();
    InitTaxiNodesForLevel();
    InitTalentForLevel();
    InitPrimaryProffesions();                               // to max set before any spell added

    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()
    UpdateMaxHealth();                                      // Update max Health (for add bonus from stamina)
    SetFullHealth();
    SetFullPower(POWER_MANA);

    // original spells
    LearnDefaultSkills();
    LearnDefaultSpells(true);

    // original action bar
    std::list<uint16>::const_iterator action_itr[4];
    for(int i=0; i<4; i++)
        action_itr[i] = info->action[i].begin();

    for (; action_itr[0]!=info->action[0].end() && action_itr[1]!=info->action[1].end();)
    {
        uint16 taction[4];
        for(int i=0; i<4 ;i++)
            taction[i] = (*action_itr[i]);

        addActionButton((uint8)taction[0], taction[1], (uint8)taction[2], (uint8)taction[3]);

        for(auto & i : action_itr)
            ++i;
    }

    // original items
    if (CharStartOutfitEntry const* oEntry = GetCharStartOutfitEntry(race, class_, gender))
    {
        for(int j : oEntry->ItemId)
        {
            if(j <= 0)
                continue;

            uint32 item_id = j;

            ItemTemplate const* iProto = sObjectMgr->GetItemTemplate(item_id);
            if(!iProto)
            {
                TC_LOG_ERROR("entities.player","Initial item id %u (race %u class %u) from CharStartOutfit.dbc not listed in `item_template`, ignoring.",item_id,GetRace(),GetClass());
                continue;
            }

            uint32 count = iProto->Stackable;               // max stack by default (mostly 1)
            if (iProto->Class == ITEM_CLASS_CONSUMABLE && iProto->SubClass == ITEM_SUBCLASS_FOOD)
            {
                switch(iProto->Spells[0].SpellCategory)
                {
                    case SPELL_CATEGORY_FOOD:                                // food
                        if(iProto->Stackable > 4)
                            count = 4;
                        break;
                    case SPELL_CATEGORY_DRINK:                                // drink
                        if(iProto->Stackable > 2)
                            count = 2;
                        break;
                }
            }

            StoreNewItemInBestSlots(item_id, count, iProto);
        }
    }

    for (auto item_id_itr = info->item.begin(); item_id_itr!=info->item.end(); ++item_id_itr++)
        StoreNewItemInBestSlots(item_id_itr->item_id, item_id_itr->item_amount);

    //give bags to gms
    if (GetSession()->GetSecurity() > SEC_PLAYER)
        StoreNewItemInBestSlots(23162, 4); //36 slots bags "Foror's Crate of Endless Resist Gear Storage"

    // bags and main-hand weapon must equipped at this moment
    // now second pass for not equipped (offhand weapon/shield if it attempt equipped before main-hand weapon)
    // or ammo not equipped in special bag
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if(Item* pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
        {
            uint16 eDest;
            // equip offhand weapon/shield if it attempt equipped before main-hand weapon
            uint8 msg = CanEquipItem( NULL_SLOT, eDest, pItem, false );
            if( msg == EQUIP_ERR_OK )
            {
                RemoveItem(INVENTORY_SLOT_BAG_0, i,true);
                EquipItem( eDest, pItem, true, false);
            }
            // move other items to more appropriate slots (ammo not equipped in special bag)
            else
            {
                ItemPosCountVec sDest;
                msg = CanStoreItem( NULL_BAG, NULL_SLOT, sDest, pItem, false );
                if( msg == EQUIP_ERR_OK )
                {
                    RemoveItem(INVENTORY_SLOT_BAG_0, i,true);
                    pItem = StoreItem( sDest, pItem, true);
                }

                // if  this is ammo then use it
                uint8 msg2 = CanUseAmmo( pItem->GetTemplate()->ItemId );
                if(msg2 == EQUIP_ERR_OK )
                    SetAmmo( pItem->GetTemplate()->ItemId );
            }
        }
    }
    // all item positions resolved

    //lot of free stuff
    if (sWorld->getConfig(CONFIG_ARENASERVER_ENABLED))
    {
        SetSkill(129,3,375,375); //first aid
        AddSpell(27028,true); //first aid spell
        AddSpell(27033,true); //bandage
        AddSpell(28029,true); //master ench
        SetSkill(333,3,375,375); //max it
        AddSpell(23803,true);//  [Ench. d'arme (Esprit renforcé) frFR]
        AddSpell(34002,true); // [Ench. de brassards (Assaut) frFR]
        AddSpell(25080,true); // [Ench. de gants (Agilité excellente) frFR]
        AddSpell(44383,true); // [Ench. de bouclier (Résilience) frFR]
        AddSpell(34091,true); //mount 280

        LearnAllClassSpells();
    }

    GetThreatManager().Initialize();

    return true;
}

bool Player::StoreNewItemInBestSlots(uint32 titem_id, uint32 titem_amount, ItemTemplate const *proto)
{
    // attempt equip by one
    while(titem_amount > 0)
    {
        uint16 eDest;
        uint8 msg = CanEquipNewItem(NULL_SLOT, eDest, titem_id, false);
        if( msg != EQUIP_ERR_OK )
            break;

        EquipNewItem(eDest, titem_id, true);
        AutoUnequipOffhandIfNeed();
        --titem_amount;
    }

    if(titem_amount == 0)
        return true;                                        // equipped

    // attempt store
    ItemPosCountVec sDest;
    // store in main bag to simplify second pass (special bags can be not equipped yet at this moment)
    InventoryResult msg = CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, sDest, titem_id, titem_amount);
    if (msg == EQUIP_ERR_OK)
    {
        StoreNewItem(sDest, titem_id, true, Item::GenerateItemRandomPropertyId(titem_id));
        return true;                                        // stored
    }

    // item can't be added
    TC_LOG_ERROR("entities.player","STORAGE: Can't equip or store initial item %u for race %u class %u , error msg = %u",titem_id,GetRace(),GetClass(),msg);
    return false;
}

void Player::AutoStoreLoot(uint8 bag, uint8 slot, uint32 loot_id, LootStore const& store, bool broadcast)
{
    Loot loot;
    loot.FillLoot(loot_id, store, this, true);

    uint32 max_slot = loot.GetMaxSlotInLootFor(this);
    for (uint32 i = 0; i < max_slot; ++i)
    {
        LootItem* lootItem = loot.LootItemInSlot(i, this);

        ItemPosCountVec dest;
        InventoryResult msg = CanStoreNewItem(bag, slot, dest, lootItem->itemid, lootItem->count);
        if (msg != EQUIP_ERR_OK && slot != NULL_SLOT)
            msg = CanStoreNewItem(bag, NULL_SLOT, dest, lootItem->itemid, lootItem->count);
        if (msg != EQUIP_ERR_OK && bag != NULL_BAG)
            msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, lootItem->itemid, lootItem->count);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, nullptr, nullptr, lootItem->itemid);
            continue;
        }

        Item* pItem = StoreNewItem(dest, lootItem->itemid, true, lootItem->randomPropertyId);
        SendNewItem(pItem, lootItem->count, false, false, broadcast);
    }
}

void Player::StoreLootItem(uint8 lootSlot, Loot* loot)
{
    NotNormalLootItem* qitem = nullptr;
    NotNormalLootItem* ffaitem = nullptr;
    NotNormalLootItem* conditem = nullptr;

    LootItem* item = loot->LootItemInSlot(lootSlot, this, &qitem, &ffaitem, &conditem);

    if (!item)
    {
        SendEquipError(EQUIP_ERR_ALREADY_LOOTED, nullptr, nullptr);
        return;
    }

    if (!item->AllowedForPlayer(this))
    {
        SendLootRelease(GetLootGUID());
        return;
    }

    // questitems use the blocked field for other purposes
    if (!qitem && item->is_blocked)
    {
        SendLootRelease(GetLootGUID());
        return;
    }

    // dont allow protected item to be looted by someone else
    if (!item->rollWinnerGUID.IsEmpty() && item->rollWinnerGUID != GetGUID())
    {
        SendLootRelease(GetLootGUID());
        return;
    }

    ItemPosCountVec dest;
    InventoryResult msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item->itemid, item->count);
    if (msg == EQUIP_ERR_OK)
    {
        GuidSet looters = item->GetAllowedLooters();
        Item* newitem = StoreNewItem(dest, item->itemid, true, item->randomPropertyId, looters);

        if (qitem)
        {
            qitem->is_looted = true;
            //freeforall is 1 if everyone's supposed to get the quest item.
            if (item->freeforall || loot->GetPlayerQuestItems().size() == 1)
                SendNotifyLootItemRemoved(lootSlot);
            else
                loot->NotifyQuestItemRemoved(qitem->index);
        }
        else
        {
            if (ffaitem)
            {
                //freeforall case, notify only one player of the removal
                ffaitem->is_looted = true;
                SendNotifyLootItemRemoved(lootSlot);
            }
            else
            {
                //not freeforall, notify everyone
                if (conditem)
                    conditem->is_looted = true;
                loot->NotifyItemRemoved(lootSlot);
            }
        }

        //if only one person is supposed to loot the item, then set it to looted
        if (!item->freeforall)
            item->is_looted = true;

        --loot->unlootedCount;

        SendNewItem(newitem, uint32(item->count), false, false, true);
#ifdef LICH_KING
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item->itemid, item->count);
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE, loot->loot_type, item->count);
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM, item->itemid, item->count);
#endif

        // LootItem is being removed (looted) from the container, delete it from the DB.
        if (loot->containerID > 0)
            sLootItemStorage->RemoveStoredLootItemForContainer(loot->containerID, item->itemid, item->count);

    }
    else
        SendEquipError(msg, nullptr, nullptr, item->itemid);
}

uint32 Player::GetEquipedItemsLevelSum()
{
                        /* Head  Neck  Should. Back   Chest  Waist   MH     OH    Ranged Hands  Wrist  Legs   Feet */
    uint16 posTab[13] = { 65280, 65281, 65282, 65294, 65284, 65288, 65295, 65296, 65297, 65289, 65285, 65286, 65287 };
    uint32 levelSum = 0;

    for (unsigned short i : posTab) {
        Item* item = GetItemByPos(i);
        if (!item)
            continue;

        levelSum += item->GetTemplate()->ItemLevel;
    }

    return levelSum;
}

void Player::SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen)
{
    if (int(MaxValue) == DISABLED_MIRROR_TIMER)
    {
        if (int(CurrentValue) != DISABLED_MIRROR_TIMER)
            StopMirrorTimer(Type);
        return;
    }

    WorldPacket data(SMSG_START_MIRROR_TIMER, (21));
    data << (uint32)Type;
    data << CurrentValue;
    data << MaxValue;
    data << Regen;
    data << (uint8)0;
    data << (uint32)0;                                      // spell id
    SendDirectMessage(&data);
}

void Player::StopMirrorTimer(MirrorTimerType Type)
{
    m_MirrorTimer[Type] = DISABLED_MIRROR_TIMER;

    WorldPacket data(SMSG_STOP_MIRROR_TIMER, 4);
    data << (uint32)Type;
    SendDirectMessage(&data);
}

bool Player::IsImmuneToEnvironmentalDamage() const
{
    // check for GM and death state included in isAttackableByAOE
    return !IsTargetableForAttack(false);
}

uint32 Player::EnvironmentalDamage(EnviromentalDamage type, uint32 damage)
{
    if (IsImmuneToEnvironmentalDamage())
        return 0;

    // Absorb, resist some environmental damage type
    uint32 absorb = 0;
    uint32 resist = 0;
    switch (type)
    {
    case DAMAGE_LAVA:
    case DAMAGE_SLIME:
    {
        DamageInfo dmgInfo(this, this, damage, nullptr, type == DAMAGE_LAVA ? SPELL_SCHOOL_MASK_FIRE : SPELL_SCHOOL_MASK_NATURE, DIRECT_DAMAGE, BASE_ATTACK);
        Unit::CalcAbsorbResist(dmgInfo);
        absorb = dmgInfo.GetAbsorb();
        resist = dmgInfo.GetResist();
        damage = dmgInfo.GetDamage();
        break;
    }
    default:
        break;
    }

    Unit::DealDamageMods(this, damage, &absorb);

    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, (21));
    data << uint64(GetGUID());
    data << uint8(type != DAMAGE_FALL_TO_VOID ? type : DAMAGE_FALL);
    data << uint32(damage);
    data << uint32(absorb);
    data << uint32(resist);
    SendMessageToSet(&data, true);

    uint32 final_damage = Unit::DealDamage(this, this, damage, nullptr, SELF_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);

    if (type==DAMAGE_FALL && !IsAlive())                     // DealDamage not apply item durability loss at self damage
    {
        TC_LOG_DEBUG("entities.player", "Player fell to death, loosing 10 percents durability");
        DurabilityLossAll(0.10f,false);
        // durability lost message
        WorldPacket data2(SMSG_DURABILITY_DAMAGE_DEATH, 0);
        SendDirectMessage(&data2);
    }

    return final_damage;
}

int32 Player::getMaxTimer(MirrorTimerType timer)
{
    switch (timer)
    {
        case FATIGUE_TIMER:
            return MINUTE*IN_MILLISECONDS;
        case BREATH_TIMER:
        {
            if (!IsAlive() || HasAuraType(SPELL_AURA_WATER_BREATHING) || IsGameMaster())
                return DISABLED_MIRROR_TIMER;
            int32 UnderWaterTime = MINUTE*IN_MILLISECONDS;
            AuraEffectList const& mModWaterBreathing = GetAuraEffectsByType(SPELL_AURA_MOD_WATER_BREATHING);
            for (auto i : mModWaterBreathing)
                UnderWaterTime = uint32(UnderWaterTime * (100.0f + i->GetAmount()) / 100.0f);
            return UnderWaterTime;
        }
        case FIRE_TIMER:
        {
            if (!IsAlive())
                return DISABLED_MIRROR_TIMER;
            return IN_MILLISECONDS;
        }
        default:
            return 0;
    }
}

void Player::UpdateMirrorTimers()
{
    // Desync flags for update on next HandleDrowning
    if (m_MirrorTimerFlags)
        m_MirrorTimerFlagsLast = ~m_MirrorTimerFlags;
}

void Player::HandleDrowning(uint32 time_diff)
{
    if (!m_MirrorTimerFlags)
        return;

    // In water
    if (m_MirrorTimerFlags & UNDERWATER_INWATER)
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[BREATH_TIMER] == DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimer[BREATH_TIMER] = getMaxTimer(BREATH_TIMER);
            SendMirrorTimer(BREATH_TIMER, m_MirrorTimer[BREATH_TIMER], m_MirrorTimer[BREATH_TIMER], -1);
        }
        else                                                              // If activated - do tick
        {
            m_MirrorTimer[BREATH_TIMER]-=time_diff;
            // Timer limit - need deal damage
            if (m_MirrorTimer[BREATH_TIMER] < 0)
            {
                m_MirrorTimer[BREATH_TIMER]+= 1*IN_MILLISECONDS;
                // Calculate and deal damage
                // TODO: Check this formula
                uint32 damage = GetMaxHealth() / 5 + urand(0, GetLevel()-1);
                EnvironmentalDamage(DAMAGE_DROWNING, damage);
            }
            else if (!(m_MirrorTimerFlagsLast & UNDERWATER_INWATER))      // Update time in client if need
                SendMirrorTimer(BREATH_TIMER, getMaxTimer(BREATH_TIMER), m_MirrorTimer[BREATH_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[BREATH_TIMER] != DISABLED_MIRROR_TIMER)        // Regen timer
    {
        int32 UnderWaterTime = getMaxTimer(BREATH_TIMER);
        // Need breath regen
        m_MirrorTimer[BREATH_TIMER]+=10*time_diff;
        if (m_MirrorTimer[BREATH_TIMER] >= UnderWaterTime || !IsAlive())
            StopMirrorTimer(BREATH_TIMER);
        else if (m_MirrorTimerFlagsLast & UNDERWATER_INWATER)
            SendMirrorTimer(BREATH_TIMER, UnderWaterTime, m_MirrorTimer[BREATH_TIMER], 10);
    }

    // In dark water
    if (m_MirrorTimerFlags & UNDERWATER_INDARKWATER)
    {
        // Fatigue timer not activated - activate it
        if (m_MirrorTimer[FATIGUE_TIMER] == DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimer[FATIGUE_TIMER] = getMaxTimer(FATIGUE_TIMER);
            SendMirrorTimer(FATIGUE_TIMER, m_MirrorTimer[FATIGUE_TIMER], m_MirrorTimer[FATIGUE_TIMER], -1);
        }
        else
        {
            m_MirrorTimer[FATIGUE_TIMER]-=time_diff;
            // Timer limit - need deal damage or teleport ghost to graveyard
            if (m_MirrorTimer[FATIGUE_TIMER] < 0)
            {
                m_MirrorTimer[FATIGUE_TIMER]+= 1*IN_MILLISECONDS;
                if (IsAlive())                                            // Calculate and deal damage
                {
                    uint32 damage = GetMaxHealth() / 5 + urand(0, GetLevel()-1);
                    EnvironmentalDamage(DAMAGE_EXHAUSTED, damage);
                }
                else if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))       // Teleport ghost to graveyard
                    RepopAtGraveyard();
            }
            else if (!(m_MirrorTimerFlagsLast & UNDERWATER_INDARKWATER))
                SendMirrorTimer(FATIGUE_TIMER, getMaxTimer(FATIGUE_TIMER), m_MirrorTimer[FATIGUE_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[FATIGUE_TIMER] != DISABLED_MIRROR_TIMER)       // Regen timer
    {
        int32 DarkWaterTime = getMaxTimer(FATIGUE_TIMER);
        m_MirrorTimer[FATIGUE_TIMER]+=10*time_diff;
        if (m_MirrorTimer[FATIGUE_TIMER] >= DarkWaterTime || !IsAlive())
            StopMirrorTimer(FATIGUE_TIMER);
        else if (m_MirrorTimerFlagsLast & UNDERWATER_INDARKWATER)
            SendMirrorTimer(FATIGUE_TIMER, DarkWaterTime, m_MirrorTimer[FATIGUE_TIMER], 10);
    }

    if (m_MirrorTimerFlags & (UNDERWATER_INLAVA)) //sunstrider: removed UNDERWATER_INSLIME from this check. Slime shouldn't do damage to players except in naxxramas, where it's handled with a special water type applying the spell 28801
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[FIRE_TIMER] == DISABLED_MIRROR_TIMER)
            m_MirrorTimer[FIRE_TIMER] = getMaxTimer(FIRE_TIMER);
        else
        {
            m_MirrorTimer[FIRE_TIMER]-=time_diff;
            if (m_MirrorTimer[FIRE_TIMER] < 0)
            {
                m_MirrorTimer[FIRE_TIMER]+= 1*IN_MILLISECONDS;
                // Calculate and deal damage
                // TODO: Check this formula
                uint32 damage = urand(600, 700);
                //if (m_MirrorTimerFlags & UNDERWATER_INLAVA)
                    EnvironmentalDamage(DAMAGE_LAVA, damage);
                /*else
                    EnvironmentalDamage(DAMAGE_SLIME, damage);*/
            }
        }
    }
    else
        m_MirrorTimer[FIRE_TIMER] = DISABLED_MIRROR_TIMER;

    // Recheck timers flag
    m_MirrorTimerFlags&=~UNDERWATER_EXIST_TIMERS;
    for (int i : m_MirrorTimer)
        if (i != DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimerFlags|=UNDERWATER_EXIST_TIMERS;
            break;
        }
    m_MirrorTimerFlagsLast = m_MirrorTimerFlags;
}

///The player sobers by 256 every 10 seconds
void Player::HandleSobering()
{
    m_drunkTimer = 0;

    uint8 currentDrunkValue = GetDrunkValue();
    uint8 drunk = currentDrunkValue ? --currentDrunkValue : 0;
    SetDrunkValue(drunk);
}

DrunkenState Player::GetDrunkenstateByValue(uint8 value)
{
    if (value >= 90)
        return DRUNKEN_SMASHED;
    if (value >= 50)
        return DRUNKEN_DRUNK;
    if (value)
        return DRUNKEN_TIPSY;

    return DRUNKEN_SOBER;
}

void Player::SetDrunkValue(uint16 newDrunkValue, uint32 itemId)
{
    uint32 oldDrunkenState = Player::GetDrunkenstateByValue(m_drunk);
    if (newDrunkValue > 100)
        newDrunkValue = 100;

    int32 drunkPercent = newDrunkValue; //std::max<int32>(newDrunkValue, GetTotalAuraModifier(SPELL_AURA_MOD_FAKE_INEBRIATE));
    if (drunkPercent)
    {
        m_invisibilityDetect.AddFlag(INVISIBILITY_DRUNK);
        m_invisibilityDetect.SetValue(INVISIBILITY_DRUNK, drunkPercent);
    }
    else if (/* TC !HasAuraType(SPELL_AURA_MOD_FAKE_INEBRIATE) &&*/ !newDrunkValue)
        m_invisibilityDetect.DelFlag(INVISIBILITY_DRUNK);

    m_drunk = newDrunkValue;
    SetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_INEBRIATION, newDrunkValue);
    UpdateObjectVisibility();

    uint32 newDrunkenState = Player::GetDrunkenstateByValue(m_drunk);

    if(newDrunkenState == oldDrunkenState)
        return;

    WorldPacket data(SMSG_CROSSED_INEBRIATION_THRESHOLD, (8+4+4));
    data << uint64(GetGUID());
    data << uint32(newDrunkenState);
    data << uint32(itemId);

    SendMessageToSet(&data, true);
}

void Player::Update( uint32 p_time )
{
    if(!IsInWorld())
        return;

    if (m_kickatnextupdate && m_session) 
    {
        m_kickatnextupdate = false;
        m_session->LogoutPlayer(false);
        return;
    }

    // undelivered mail
    if(m_nextMailDelivereTime && m_nextMailDelivereTime <= WorldGameTime::GetGameTime())
    {
        SendNewMail();
        ++unReadMails;

        // It will be recalculate at mailbox open (for unReadMails important non-0 until mailbox open, it also will be recalculated)
        m_nextMailDelivereTime = 0;
    }

    // Update cinematic location, if 500ms have passed and we're doing a cinematic now.
    _cinematicMgr->m_cinematicDiff += p_time;
    if (_cinematicMgr->m_cinematicCamera && _cinematicMgr->m_activeCinematicCameraId && GetMSTimeDiffToNow(_cinematicMgr->m_lastCinematicCheck) > CINEMATIC_UPDATEDIFF)
    {
        _cinematicMgr->m_lastCinematicCheck = GetMap()->GetGameTimeMS();
        _cinematicMgr->UpdateCinematicLocation(p_time);
    }

    //used to implement delayed far teleports
    SetCanDelayTeleport(true);
    Unit::Update(p_time);
    SetCanDelayTeleport(false);

    time_t now = GetMap()->GetGameTime();

    UpdatePvPFlag(now);

    UpdateContestedPvP(p_time);

    UpdateDuelFlag(now);

    CheckDuelDistance(now);

    UpdateAfkReport(now);

    Unit::AIUpdateTick(p_time);

    // Update items that have just a limited lifetime
    if (now>m_Last_tick)
        UpdateItemDuration(uint32(now- m_Last_tick));

    if (!m_timedquests.empty())
    {
        auto iter = m_timedquests.begin();
        while (iter != m_timedquests.end())
        {
            QuestStatusData& q_status = m_QuestStatus[*iter];
            if( q_status.m_timer <= p_time )
            {
                uint32 quest_id  = *iter;
                ++iter;                                     // current iter will be removed in FailTimedQuest
                FailTimedQuest( quest_id );
            }
            else
            {
                q_status.m_timer -= p_time;
                if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;
                ++iter;
            }
        }
    }

#ifdef LICH_KING
    m_achievementMgr->UpdateTimedAchievements(p_time);
#endif

    if (HasUnitState(UNIT_STATE_MELEE_ATTACKING) && !HasUnitState(UNIT_STATE_CASTING))
    {
        if(Unit *pVictim = GetVictim())
        {
            // default combat reach 10
            // TODO add weapon,skill check

            if (IsAttackReady(BASE_ATTACK))
            {
                if(!IsWithinMeleeRange(pVictim))
                {
                    SetAttackTimer(BASE_ATTACK,100);
                    if(m_swingErrorMsg != 1)                // send single time (client auto repeat)
                    {
                        SendAttackSwingNotInRange();
                        m_swingErrorMsg = 1;
                    }
                }
                //120 degrees of radiant range
                else if (!HasInArc(2 * float(M_PI) / 3, pVictim))
                {
                    SetAttackTimer(BASE_ATTACK,100);
                    if(m_swingErrorMsg != 2)                // send single time (client auto repeat)
                    {
                        SendAttackSwingBadFacingAttack();
                        m_swingErrorMsg = 2;
                    }
                }
                else
                {
                    m_swingErrorMsg = 0;                    // reset swing error state

                    // prevent base and off attack in same time, delay attack at 0.2 sec
                    if(HaveOffhandWeapon())
                       if (GetAttackTimer(OFF_ATTACK) < ATTACK_DISPLAY_DELAY)
                            SetAttackTimer(OFF_ATTACK, ATTACK_DISPLAY_DELAY);

                    AttackerStateUpdate(pVictim, BASE_ATTACK);
                    ResetAttackTimer(BASE_ATTACK);
                }
            }

            if ( HaveOffhandWeapon() && IsAttackReady(OFF_ATTACK))
            {
                if(!IsWithinMeleeRange(pVictim))
                    SetAttackTimer(OFF_ATTACK,100);
                else if (!HasInArc(2 * float(M_PI) / 3, pVictim))
                    SetAttackTimer(OFF_ATTACK,100);
                else
                {
                     // prevent base and off attack in same time, delay attack at 0.2 sec
                    if (GetAttackTimer(BASE_ATTACK) < ATTACK_DISPLAY_DELAY)
                        SetAttackTimer(BASE_ATTACK, ATTACK_DISPLAY_DELAY);

                    // do attack
                    AttackerStateUpdate(pVictim, OFF_ATTACK);
                    ResetAttackTimer(OFF_ATTACK);
                }
            }

            /*Unit *owner = pVictim->GetOwner();
            Unit *u = owner ? owner : pVictim;
            if(u->IsPvP() && (!duel || duel->Opponent != u))
            {
                UpdatePvP(true);
                RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
            }*/
        }
    }

    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
    {
        if(roll_chance_i(3) && GetTimeInnEnter() > 0)       //freeze update
        {
            time_t currTime = WorldGameTime::GetGameTime();
            int time_inn = currTime - GetTimeInnEnter();
            if (time_inn >= 10)                             //freeze update
            {
                float bubble = 0.125 * sWorld->GetRate(RATE_REST_INGAME);
                float extraPerSec = ((float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP) / 72000.0f) * bubble;
                                                            
                //speed collect rest bonus (section/in hour)
                SetRestBonus(GetRestBonus() + time_inn * extraPerSec);
                UpdateInnerTime(currTime);
            }
        }
    }

    if(m_regenTimer > 0)
    {
        if(p_time >= m_regenTimer)
            m_regenTimer = 0;
        else
            m_regenTimer -= p_time;
    }

    if (m_weaponChangeTimer > 0)
    {
        if(p_time >= m_weaponChangeTimer)
            m_weaponChangeTimer = 0;
        else
            m_weaponChangeTimer -= p_time;
    }

    if (m_zoneUpdateTimer > 0)
    {
        if(p_time >= m_zoneUpdateTimer)
        {
            uint32 newzone, newarea;
            GetZoneAndAreaId(newzone, newarea);

            if( m_zoneUpdateId != newzone )
                UpdateZone(newzone, newarea);                        // also update area
            else
            {
                // use area updates as well
                // needed for free far all arenas for example
                if( m_areaUpdateId != newarea )
                    UpdateArea(newarea);

                m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL;
            }
        }
        else
            m_zoneUpdateTimer -= p_time;
    }

    if (IsAlive())
    {
        m_regenTimer += p_time;
        RegenerateAll();
    }

    if (m_deathState == JUST_DIED)
        KillPlayer();

    if(m_nextSave > 0)
    {
        if(p_time >= m_nextSave)
        {
            // m_nextSave reseted in SaveToDB call
            SaveToDB();
            TC_LOG_DEBUG("entities.player","Player '%s' (GUID: %u) saved", GetName().c_str(), GetGUID().GetCounter());
        }
        else
        {
            m_nextSave -= p_time;
        }
    }

    //Handle Water/drowning
    HandleDrowning(p_time);

    // Played time
    if (now > m_Last_tick)
    {
        uint32 elapsed = uint32(now - m_Last_tick);
        m_Played_time[0] += elapsed;                        // Total played time
        m_Played_time[1] += elapsed;                        // Level played time
        m_Last_tick = now;
    }

    if (GetDrunkValue())
    {
        m_drunkTimer += p_time;

        if (m_drunkTimer > 9 * IN_MILLISECONDS)
            HandleSobering();
    }

    // not auto-free ghost from body in instances
    if(m_deathTimer > 0  && !GetBaseMap()->Instanceable())
    {
        if(p_time >= m_deathTimer)
        {
            m_deathTimer = 0;
            BuildPlayerRepop();
            RepopAtGraveyard();
        }
        else
            m_deathTimer -= p_time;
    }

    UpdateEnchantTime(p_time);
    UpdateHomebindTime(p_time);

    // group update
    SendUpdateToOutOfRangeGroupMembers();

    Pet* pet = GetPet();
    if(pet && !IsWithinDistInMap(pet, OWNER_MAX_DISTANCE) && !pet->IsPossessed())
        RemovePet(pet, PET_SAVE_NOT_IN_SLOT, true);

    if (IsAlive())
    {
        if (m_hostileReferenceCheckTimer <= p_time)
        {
            m_hostileReferenceCheckTimer = 1 * SECOND;
            if (!GetMap()->IsDungeon())
                GetCombatManager().EndCombatBeyondRange(GetVisibilityRange(), true);
        }
        else
            m_hostileReferenceCheckTimer -= p_time;
    }

    //we should execute delayed teleports only for alive(!) players
    //because we don't want player's ghost teleported from graveyard
    // sunwell: so we store it to the end of the world and teleport out of the ass after resurrection?
    if (IsHasDelayedTeleport() /* && IsAlive()*/)
        TeleportTo(m_teleport_dest, m_teleport_options);

    #ifdef PLAYERBOT
    if (m_playerbotAI)
       m_playerbotAI->UpdateAI(p_time);
    if (m_playerbotMgr)
       m_playerbotMgr->UpdateAI(p_time);
    #endif
}

void Player::SetDeathState(DeathState s)
{
    uint32 ressSpellId = 0;

    bool cur = IsAlive();

    if(s == JUST_DIED)
    {
        if(!cur)
        {
            TC_LOG_ERROR("entities.player","setDeathState: attempt to kill a dead player %s(%d)", GetName().c_str(), GetGUID().GetCounter());
            return;
        }

        // send spectate addon message
        if (HaveSpectators())
        {
            SpectatorAddonMsg msg;
            msg.SetPlayer(GetName());
            msg.SetStatus(false);
            SendSpectatorAddonMsgToBG(msg);
        }

        // drunken state is cleared on death
        SetDrunkValue(0);
        // lost combo points at any target (targeted combo points clear in Unit::setDeathState)
        ClearComboPoints();

        clearResurrectRequestData();

        //FIXME: is pet dismissed at dying or releasing spirit? if second, add SetDeathState(DEAD) to HandleRepopRequestOpcode and define pet unsummon here with (s == DEAD)
        RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT, true, REMOVE_PET_REASON_PLAYER_DIED);

        // save value before aura remove in Unit::setDeathState
        ressSpellId = GetUInt32Value(PLAYER_SELF_RES_SPELL);

        // passive spell
        if(!ressSpellId)
            ressSpellId = GetResurrectionSpellId();
    }
    Unit::SetDeathState(s);

    // restore resurrection spell id for player after aura remove
    if(s == JUST_DIED && cur && ressSpellId)
        SetUInt32Value(PLAYER_SELF_RES_SPELL, ressSpellId);

    if(IsAlive() && !cur)
    {
        //clear aura case after resurrection by another way (spells will be applied before next death)
        SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);

        // restore default warrior stance
        if(GetClass()== CLASS_WARRIOR)
            CastSpell(this,SPELL_ID_PASSIVE_BATTLE_STANCE, true);
    }
}

//result must be deleted by caller
bool Player::BuildEnumData(PreparedQueryResult result, WorldPacket * p_data, WorldSession const* session)
{
    /* SEE CHAR_SEL_ENUM
               0      1       2        3        4            5             6             7       8        9
    "SELECT c.guid, c.name, c.race, c.class, c.gender, c.playerBytes, c.playerBytes2, c.level, c.zone, c.map,
        10             11            12           13          14            15           16        17         18         19              20
    c.position_x, c.position_y, c.position_z, gm.guildid, c.playerFlags, c.at_login, cp.entry, cp.modelid, cp.level, c.equipmentCache, cb.guid */

    Field *fields = result->Fetch();

    ObjectGuid::LowType guid = fields[0].GetUInt32();
    uint8 plrRace = fields[2].GetUInt8();
    uint8 plrClass = fields[3].GetUInt8();
    uint8 gender = fields[4].GetUInt8();

    PlayerInfo const *info = sObjectMgr->GetPlayerInfo(plrRace, plrClass);
    if (!info)
    {
        TC_LOG_ERROR("entities.player", "Player %u have incorrect race/class pair. Don't build enum.", guid);
        return false;
    }

    *p_data << uint64(ObjectGuid(HighGuid::Player, guid));
    *p_data << fields[1].GetString();                           // name
    *p_data << uint8(plrRace);                                  // race
    *p_data << uint8(plrClass);                                 // class
    *p_data << uint8(gender);                                   // gender

    uint32 playerBytes = fields[5].GetUInt32();
    uint32 playerBytes2 = fields[6].GetUInt32();
    uint32 atLoginFlags = fields[15].GetUInt32();

    uint8 skin = uint8(playerBytes);
    uint8 face = uint8(playerBytes >> 8);
    uint8 hairStyle = uint8(playerBytes >> 16);
    uint8 hairColor = uint8(playerBytes >> 24);
    uint8 facialStyle = uint8(playerBytes2 & 0xFF);

#ifdef LICH_KING
    if (!ValidateAppearance(uint8(plrRace), uint8(plrClass), gender, hairStyle, hairColor, face, facialStyle, skin))
    {
        TC_LOG_ERROR("entities.player.loading", "Player %u has wrong Appearance values (Hair/Skin/Color), forcing recustomize", guid);

        // Make sure customization always works properly - send all zeroes instead
        skin = 0, face = 0, hairStyle = 0, hairColor = 0, facialStyle = 0;

        if (!(atLoginFlags & AT_LOGIN_CUSTOMIZE))
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
            stmt->setUInt16(0, uint16(AT_LOGIN_CUSTOMIZE));
            stmt->setUInt32(1, guid);
            CharacterDatabase.Execute(stmt);
            atLoginFlags |= AT_LOGIN_CUSTOMIZE;
        }
    }
#endif

    *p_data << uint8(skin);
    *p_data << uint8(face);
    *p_data << uint8(hairStyle);
    *p_data << uint8(hairColor);   
    *p_data << uint8(facialStyle);

    *p_data << uint8(fields[7].GetUInt8());                     // level
    *p_data << uint32(fields[8].GetUInt16());                   // zone
    *p_data << uint32(fields[9].GetUInt32());                   // map

    *p_data << fields[10].GetFloat();                           // x
    *p_data << fields[11].GetFloat();                           // y
    *p_data << fields[12].GetFloat();                           // z

    *p_data << uint32(fields[13].GetUInt32());                 // guild id

    uint32 char_flags = 0;
    uint32 playerFlags = fields[14].GetUInt32();
    if (playerFlags & PLAYER_FLAGS_HIDE_HELM)
        char_flags |= CHARACTER_FLAG_HIDE_HELM;
    if (playerFlags & PLAYER_FLAGS_HIDE_CLOAK)
        char_flags |= CHARACTER_FLAG_HIDE_CLOAK;
    if (playerFlags & PLAYER_FLAGS_GHOST)
        char_flags |= CHARACTER_FLAG_GHOST;
    if (atLoginFlags & AT_LOGIN_RENAME)
        char_flags |= CHARACTER_FLAG_RENAME;
    // always send the flag if declined names aren't used
    // to let the client select a default method of declining the name
    if (sWorld->getConfig(CONFIG_DECLINED_NAMES_USED))
        char_flags |= CHARACTER_FLAG_DECLINED;

    *p_data << (uint32)char_flags;                          // character flags

#ifdef LICH_KING
    // character customize flags
    if (atLoginFlags & AT_LOGIN_CUSTOMIZE)
        *p_data << uint32(CHAR_CUSTOMIZE_FLAG_CUSTOMIZE);
    else if (atLoginFlags & AT_LOGIN_CHANGE_FACTION)
        *p_data << uint32(CHAR_CUSTOMIZE_FLAG_FACTION);
    else if (atLoginFlags & AT_LOGIN_CHANGE_RACE)
        *p_data << uint32(CHAR_CUSTOMIZE_FLAG_RACE);
    else
        *p_data << uint32(CHAR_CUSTOMIZE_FLAG_NONE);
#endif

    // First login
    *p_data << uint8(atLoginFlags & AT_LOGIN_FIRST ? 1 : 0);

    // Pets info
    uint32 petDisplayId = 0;
    uint32 petLevel = 0;
    CreatureFamily petFamily = CREATURE_FAMILY_NONE;

    // show pet at selection character in character list only for non-ghost character
    if (result && !(playerFlags & PLAYER_FLAGS_GHOST) && (plrClass == CLASS_WARLOCK || plrClass == CLASS_HUNTER))
    {
        Field* fields2 = result->Fetch();

        uint32 entry = fields2[16].GetUInt32();
        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(entry);
        if (cInfo)
        {
            petDisplayId = fields2[17].GetUInt32();
            petLevel = fields2[18].GetUInt8();
            petFamily = cInfo->family;
        }
    }

    *p_data << (uint32)petDisplayId;
    *p_data << (uint32)petLevel;
    *p_data << (uint32)petFamily;

    Tokens equipCache = StrSplit(fields[19].GetString(), " ");
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; slot++)
    {
        uint32 visualBase = slot * 2;
        uint32 itemId = GetUInt32ValueFromArray(equipCache, visualBase);
        const ItemTemplate *proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
        {
            *p_data << (uint32)0;
            *p_data << (uint8)0;
            *p_data << (uint32)0;
            continue;
        }
        SpellItemEnchantmentEntry const *enchant = nullptr;

        uint32 enchants = GetUInt32ValueFromArray(equipCache, visualBase + 1);
        for (uint8 enchantSlot = PERM_ENCHANTMENT_SLOT; enchantSlot <= TEMP_ENCHANTMENT_SLOT; ++enchantSlot)
        {
            // values stored in 2 uint16
            uint32 enchantId = 0x0000FFFF & (enchants >> enchantSlot * 16);
            if (!enchantId)
                continue;

            enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
            if (enchant)
                break;
        }

        *p_data << (uint32)proto->DisplayInfoID;
        *p_data << (uint8)proto->InventoryType;
        *p_data << (uint32)(enchant ? enchant->aura_id : 0);
    }
#ifdef LICH_KING
    //LK also sends bag
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; slot++)
    {
        *p_data << (uint32)0;
        *p_data << (uint8)0;
        *p_data << (uint32)0;
    }
#else
    //first bag info ?
    *p_data << (uint32)0;
    *p_data << (uint8)0;
    *p_data << (uint32)0;
#endif

    return true;
}


void Player::ToggleAFK()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK);

    // afk player not allowed in battleground
    if(IsAFK() && InBattleground() && !InArena())
        LeaveBattleground();
}

void Player::ToggleDND()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_DND);
}

uint8 Player::GetChatTag() const
{
    uint8 tag = CHAT_TAG_NONE;

    if (IsGMChat())
        tag |= CHAT_TAG_GM;
    if (IsDND())
        tag |= CHAT_TAG_DND;
    if (IsAFK())
        tag |= CHAT_TAG_AFK;
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_DEVELOPER))
        tag |= CHAT_TAG_DEV;

    return tag;
}

void Player::ResetMap()
{
    // this may be called during Map::Update
    // after decrement+unlink, ++m_mapRefIter will continue correctly
    // when the first element of the list is being removed
    // nocheck_prev will return the padding element of the RefManager
    // instead of NULL in the case of prev
    GetMap()->UpdateIteratorBack(this);
    Unit::ResetMap();
    GetMapRef().unlink();
}

void Player::SetMap(Map* map)
{
    Unit::SetMap(map);
    m_mapRef.link(map, this);
}

void Player::SetTeleportingToTest(uint32 instanceId)
{
    m_teleportToTestInstanceId = instanceId;
}

bool Player::TeleportTo(uint32 mapid, float x, float y, float z, float orientation, uint32 options)
{
    if (!MapManager::IsValidMapCoord(mapid, x, y, z, orientation))
    {
        TC_LOG_ERROR("map", "TeleportTo: invalid map (%d) or invalid coordinates (X: %f, Y: %f, Z: %f, O: %f) given when teleporting player (GUID: %u, name: %s, map: %d, X: %f, Y: %f, Z: %f, O: %f).",
            mapid, x, y, z, orientation, GetGUID().GetCounter(), GetName().c_str(), GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        return false;
    }

    if((GetSession()->GetSecurity() < SEC_GAMEMASTER1) && !sWorld->IsAllowedMap(mapid))
    {
        TC_LOG_ERROR("map","Player %s tried to enter a forbidden map", GetName().c_str());
        return false;
    }

    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);

    // don't let enter battlegrounds without assigned battleground id (for example through areatrigger)...
    if (!InBattleground() && mEntry->IsBattlegroundOrArena())
        return false;

    // preparing unsummon pet if lost (we must get pet before teleportation or will not find it later)
    Pet* pet = GetPet();

    TC_LOG_DEBUG("maps", "Player %s is being teleported to map %u", GetName().c_str(), mapid);

    // reset movement flags at teleport, because player will continue move with these flags after teleport
    SetUnitMovementFlags(GetUnitMovementFlags() & MOVEMENTFLAG_MASK_HAS_PLAYER_STATUS_OPCODE);
    DisableSpline();

    if (m_transport)
    {
        if (options & TELE_TO_NOT_LEAVE_TRANSPORT)
            AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
        else
        {
            m_transport->RemovePassenger(this);
            m_transport = nullptr;
            m_movementInfo.transport.Reset();
            m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
        }
    }

    // The player was ported to another map and loses the duel immediately.
    // We have to perform this check before the teleport, otherwise the
    // ObjectAccessor won't find the flag.
    if (duel && GetMapId() != mapid && GetMap()->GetGameObject(GetGuidValue(PLAYER_DUEL_ARBITER)))
        DuelComplete(DUEL_FLED);

    if (GetMapId() == mapid && !(options & TELE_TO_FORCE_RELOAD))
    {
        //lets reset far teleport flag if it wasn't reset during chained teleports
        SetSemaphoreTeleportFar(false);
        //setup delayed teleport flag
        SetDelayedTeleportFlag(IsCanDelayTeleport());
        //if teleport spell is cast in Unit::Update() func
        //then we need to delay it until update process will be finished
        if (IsHasDelayedTeleport())
        {
            SetSemaphoreTeleportNear(true);
            //lets save teleport destination for player
            m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
            m_teleport_options = options;
            return true;
        }

        if (!(options & TELE_TO_NOT_UNSUMMON_PET))
        {
            //same map, only remove pet if out of range for new position
            if(pet && !IsWithinDistInMap(pet, OWNER_MAX_DISTANCE))
                UnsummonPetTemporaryIfAny();
        }

        if (!IsAlive() && options & TELE_REVIVE_AT_TELEPORT)
            ResurrectPlayer(0.5f);

        if (!(options & TELE_TO_NOT_LEAVE_COMBAT))
            CombatStop();

        // this will be used instead of the current location in SaveToDB
        m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
        //reset fall info
        SetFallInformation(0, GetPositionZ());

        // code for finish transfer called in WorldSession::HandleMovementOpcodes()
        // at client packet MSG_MOVE_TELEPORT_ACK
        SetSemaphoreTeleportNear(true);
        // near teleport, triggering send MSG_MOVE_TELEPORT_ACK from client at landing
        if (!GetSession()->PlayerLogout())
        {
            Position oldPos = GetPosition();
            if (HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
#ifdef LICH_KING
                z += GetFloatValue(UNIT_FIELD_HOVERHEIGHT);
#else
                z += DEFAULT_HOVER_HEIGHT;
#endif
            SendTeleportPacket(m_teleport_dest, (options & TELE_TO_TRANSPORT_TELEPORT) != 0);
            if (!IsWithinDist3d(x, y, z, GetMap()->GetVisibilityRange()))
                RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED);
        }
    }
    else
    {
        if (GetSession()->GetClientControl().HasPendingMovementChange())
        {
            SetDelayedTeleportFlag(true);
            SetSemaphoreTeleportFar(true);
            //lets save teleport destination for player
            m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
            m_teleport_options = options;
            return true;
        }

        // far teleport to another map
        Map* oldmap = IsInWorld() ? GetMap() : nullptr;
        // check if we can enter before stopping combat / removing pet / totems / interrupting spells

        // Check enter rights before map getting to avoid creating instance copy for player
        // this check not dependent from map instance copy and same for all instance copies of selected map
        if (sMapMgr->PlayerCannotEnter(mapid, this, false) != Map::CAN_ENTER)
            return false;

        //I think this always returns true. Correct me if I am wrong.
        // If the map is not created, assume it is possible to enter it.
        // It will be created in the WorldPortAck.
        /*
        Map* map = sMapMgr->FindBaseMap(mapid);
        if (!map || map->CanEnter(this))
            */
        {
            //lets reset near teleport flag if it wasn't reset during chained teleports
            SetSemaphoreTeleportNear(false);
            //setup delayed teleport flag
            SetDelayedTeleportFlag(IsCanDelayTeleport());
            //if teleport spell is cast in Unit::Update() func
            //then we need to delay it until update process will be finished
            if (IsHasDelayedTeleport())
            {
                SetSemaphoreTeleportFar(true);
                //lets save teleport destination for player
                m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
                m_teleport_options = options;
                return true;
            }

            SetSelection(ObjectGuid::Empty);

            CombatStop();

            DuelComplete(DUEL_INTERRUPTED);
            ResetContestedPvP();

            // remove player from battleground on far teleport (when changing maps)
            if (Battleground const* bg = GetBattleground())
            {
                // Note: at battleground join battleground id set before teleport
                // and we already will found "current" battleground
                // just need check that this is targeted map or leave
                if (bg->GetMapId() != mapid)
                    LeaveBattleground(false);                   // don't teleport to entry point
            }

            // remove arena spell coldowns/buffs now to also remove pet's cooldowns before it's temporarily unsummoned
            if (mEntry->IsBattleArena() && !IsGameMaster())
            {
                RemoveArenaSpellCooldowns();
                RemoveArenaAuras(false);
                if (pet)
                    pet->RemoveArenaAuras(false);
            }

            // remove pet on map change
            if (pet)
                UnsummonPetTemporaryIfAny();

            // remove all dyn objects
            RemoveAllDynObjects();

            // stop spellcasting
            // not attempt interrupt teleportation spell at caster teleport
            if (!(options & TELE_TO_SPELL))
                if (IsNonMeleeSpellCast(true))
                    InterruptNonMeleeSpells(true);

            //remove auras before removing from map...
            RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING);

            // removing some auras (example spell id 26656) can trigger movement changes. Using this block to ensure that
            // teleports are delayed as long as there are still pending movement changes.
            if (GetSession()->GetClientControl().HasPendingMovementChange())
            {
                SetDelayedTeleportFlag(true);
                SetSemaphoreTeleportFar(true);
                //lets save teleport destination for player
                m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
                m_teleport_options = options;
                return true;
            }
            ASSERT(!GetSession()->GetClientControl().HasPendingMovementChange());

            if (!GetSession()->PlayerLogout())
            {
                // send transfer packets
                WorldPacket data(SMSG_TRANSFER_PENDING, 4 + 4 + 4);
                data << uint32(mapid);
                if (Transport* transport = GetTransport())
                    data << transport->GetEntry() << GetMapId();
                //data << TransferSpellID //optional, not used but seemed to be existing, at least for 4.x client

                SendDirectMessage(&data);
            }

            // remove from old map now
            if (oldmap)
                oldmap->RemovePlayerFromMap(this, false);

            m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
            m_teleport_options = options;
            SetFallInformation(0, GetPositionZ());
            // if the player is saved before worldportack (at logout for example)
            // this will be used instead of the current location in SaveToDB

            if (!GetSession()->PlayerLogout())
            {
                WorldPacket data(SMSG_NEW_WORLD, 4 + 4 + 4 + 4 + 4);
                data << uint32(mapid);
                if (GetTransport())
                    data << m_movementInfo.transport.pos.PositionXYZOStream();
                else
                    data << m_teleport_dest.PositionXYZOStream();

                SendDirectMessage(&data);
                SendSavedInstances();

                RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED);
            }

            // move packet sent by client always after far teleport
            // code for finish transfer to new map called in WorldSession::HandleMoveWorldportAckOpcode at client packet
            SetSemaphoreTeleportFar(true);
        }
        //else
        //    return false;
    }
    GetSession()->anticheat.OnPlayerTeleport(this);
    return true;
}

void Player::AddToWorld()
{
    ///- Do not add/remove the player from the object storage
    ///- It will crash when updating the ObjectAccessor
    ///- The player should only be added when logging in
    Unit::AddToWorld();

    for(int i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
        if(m_items[i])
            m_items[i]->AddToWorld();

    //WR HACK, remove me. Fog of Corruption
    if (HasAura(45717))
        CastSpell(this, 45917, true); //Soul Sever - instakill
}

void Player::RemoveFromWorld()
{
    // cleanup
    if(IsInWorld())
    {
        ///- Release charmed creatures, unsummon totems and remove pets/guardians
        StopCastingCharm();
        StopCastingBindSight();
        UnsummonPetTemporaryIfAny();
        UnsummonAllTotems();
        ClearComboPoints();
        ClearComboPointHolders();
        ObjectGuid lootGuid = GetLootGUID();
        if (lootGuid != 0)
            m_session->DoLootRelease(lootGuid);
        sOutdoorPvPMgr->HandlePlayerLeaveZone(this, m_zoneUpdateId);
    }

    // Remove items from world before self - player must be found in Item::RemoveFromObjectUpdate
    for(int i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
        if(m_items[i])
            m_items[i]->RemoveFromWorld();

    if (isSpectator())
        SetSpectate(false);

    ///- Do not add/remove the player from the object storage
    ///- It will crash when updating the ObjectAccessor
    ///- The player should only be removed when logging out
    Unit::RemoveFromWorld();

    if (m_uint32Values)
    {
        if (WorldObject* viewpoint = GetViewpoint())
        {
            TC_LOG_ERROR("entities.player", "Player::RemoveFromWorld: Player '%s' (%s) has viewpoint (Entry:%u, Type: %u) when removed from world",
                GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str(), viewpoint->GetEntry(), viewpoint->GetTypeId());
            SetViewpoint(viewpoint, false);
        }
    }
}

void Player::RewardRage( uint32 damage, uint32 weaponSpeedHitFactor, bool attacker )
{
    // Testing purposes
    if (m_disabledRegen)
        return;

    float addRage;

    float rageconversion = ((0.0091107836f * GetLevel() * GetLevel()) + 3.225598133f * GetLevel()) + 4.2652911f;
#ifdef LICH_KING
    // Unknown if correct, but lineary adjust rage conversion above level 70
    if (GetLevel() > 70)
        rageconversion += 13.27f * (GetLevel() - 70);
#endif
    if(attacker)
    {
        addRage = (damage / rageconversion * 7.5f + weaponSpeedHitFactor) / 2.0f;

        // talent giving more rage on attack
        AddPct(addRage, GetTotalAuraModifier(SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT));
    }
    else
    {
        addRage = damage / rageconversion * 2.5f;

        // Berserker Rage effect
        if(HasAuraEffect(18499,0))
            addRage *= 2.0f;
    }

    addRage *= sWorld->GetRate(RATE_POWER_RAGE_INCOME);

    ModifyPower(POWER_RAGE, uint32(addRage * 10));
}

void Player::RegenerateAll()
{
    if (m_disabledRegen)
        return;

    m_regenTimerCount += m_regenTimer;

    Regenerate(POWER_ENERGY);

    Regenerate(POWER_MANA);

    static uint32 constexpr regenDelay = 2000;

    if (m_regenTimerCount >= 2000)
    {
        // Not in combat or they have regeneration
        if (!IsInCombat() || HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT) ||
            HasAuraType(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT) || IsPolymorphed())
        {
            RegenerateHealth();
            if (!IsInCombat() && !HasAuraType(SPELL_AURA_INTERRUPT_REGEN))
                Regenerate(POWER_RAGE);
        }

        m_regenTimerCount -= 2000;
    }

    m_regenTimer = 0;
}

void Player::Regenerate(Powers power)
{
    uint32 curValue = GetPower(power);
    uint32 maxValue = GetMaxPower(power);

    float addvalue = 0.0f;

#ifdef LICH_KING
    /// @todo possible use of miscvalueb instead of amount
    if (HasAuraTypeWithValue(SPELL_AURA_PREVENT_REGENERATE_POWER, power))
        return;
#endif

    switch (power)
    {
        case POWER_MANA:
        {
            bool recentCast = IsUnderLastManaUseEffect();
            float ManaIncreaseRate = sWorld->GetRate(RATE_POWER_MANA);

            if (GetLevel() < 15)
                ManaIncreaseRate = sWorld->GetRate(RATE_POWER_MANA) * (2.066f - (GetLevel() * 0.066f));

            if (recentCast)
                addvalue = GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) *  ManaIncreaseRate * 2.00f;  //TC has 0.001f * m_regenTimer; but this is for LK continuous regen
            else
                addvalue = GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) * ManaIncreaseRate * 2.00f; //TC has 0.001f * m_regenTimer; but this is for LK continuous regen
        }   break;
        case POWER_RAGE:                                    // Regenerate rage
        {
            if (!IsInCombat() && !HasAuraType(SPELL_AURA_INTERRUPT_REGEN))
            {
                float RageDecreaseRate = sWorld->GetRate(RATE_POWER_RAGE_LOSS);
                addvalue = -30.0f * RageDecreaseRate;               // 3 rage by tick (TC has 2, it this blizzlike?)
            }
        }   break;
        case POWER_ENERGY:                                  // Regenerate energy (rogue)
            addvalue = 20;  //TC has 0.001f * m_regenTimer; but this is for LK continuous regen
            break;
        case POWER_FOCUS:
        case POWER_HAPPINESS:
        default:
            break;
    }

    // Mana regen calculated in Player::UpdateManaRegen()
    // Exist only for POWER_MANA, POWER_ENERGY, POWER_FOCUS auras
    if(power != POWER_MANA)
    {
        addvalue *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, power);

        // Butchery requires combat for this effect
#ifdef LICH_KING
        if (IsInCombat() || power != POWER_RUNIC_POWER)
#endif
            addvalue += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, power) * ((power != POWER_ENERGY) ? m_regenTimerCount : m_regenTimer) / (5 * IN_MILLISECONDS);
    }

    if (addvalue < 0.0f)
    {
        if (curValue == 0)
            return;
    }
    else if (addvalue > 0.0f)
    {
        if (curValue == maxValue)
            return;
    }

    if (addvalue < 0.0f)
    {
        if (std::fabs(addvalue) > curValue)
            curValue = 0;
        else
            curValue += addvalue;
    }
    else 
    {
        curValue += uint32(addvalue);
        if (curValue > maxValue)
            curValue = maxValue;
    }

    if (m_regenTimerCount >= 2000)
        SetPower(power, curValue);
#ifdef LICH_KING
    else
        UpdateUInt32Value(UNIT_FIELD_POWER1 + power, curValue);
#endif
}

void Player::RegenerateHealth()
{
    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue) return;

    float HealthIncreaseRate = sWorld->GetRate(RATE_HEALTH);

    float addValue = 0.0f;

    // polymorphed case
    if ( IsPolymorphed() )
        addValue = GetMaxHealth() / 3.0f;
    // normal regen case (maybe partly in combat case)
    else if (!IsInCombat() || HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT) )
    {
        addValue = OCTRegenHPPerSpirit()* HealthIncreaseRate;
        if (!IsInCombat())
        {
            addValue *= GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
            addValue += GetTotalAuraModifier(SPELL_AURA_MOD_REGEN) * 0.4f;
        }
        else if(HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
            ApplyPct(addValue, GetTotalAuraModifier(SPELL_AURA_MOD_REGEN_DURING_COMBAT));

        if(!IsStandState())
            addValue *= 1.5;
    }

    // always regeneration bonus (including combat)
    addValue += GetTotalAuraModifier(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT);

    if(addValue < 0)
        addValue = 0;

    ModifyHealth(int32(addValue));
}

void Player::ResetAllPowers()
{
    SetFullHealth();
    switch (GetPowerType())
    {
        case POWER_MANA:
            SetFullPower(POWER_MANA);
            break;
        case POWER_RAGE:
            SetPower(POWER_RAGE, 0);
            break;
        case POWER_ENERGY:
            SetFullPower(POWER_ENERGY);
            break;
#ifdef LICH_KING
        case POWER_RUNIC_POWER:
            SetPower(POWER_RUNIC_POWER, 0);
            break;
#endif
        default:
            break;
    }
}

// this one rechecks weapon auras and stores them in BaseModGroup container
// needed for things like axe specialization applying only to axe weapons in case of dual-wield
void Player::UpdateWeaponDependentCritAuras(WeaponAttackType attackType)
{
    BaseModGroup modGroup;
    switch (attackType)
    {
    case BASE_ATTACK:
        modGroup = CRIT_PERCENTAGE;
        break;
    case OFF_ATTACK:
        modGroup = OFFHAND_CRIT_PERCENTAGE;
        break;
    case RANGED_ATTACK:
        modGroup = RANGED_CRIT_PERCENTAGE;
        break;
    default:
        ABORT();
        break;
    }

    float amount = 0.0f;
    amount += GetTotalAuraModifier(SPELL_AURA_MOD_WEAPON_CRIT_PERCENT, std::bind(&Unit::CheckAttackFitToAuraRequirement, this, attackType, std::placeholders::_1));

#ifdef LICH_KING
    // these auras don't have item requirement (only Combat Expertise in 3.3.5a)
    amount += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
#endif

    SetBaseModFlatValue(modGroup, amount);
}

void Player::UpdateAllWeaponDependentCritAuras()
{
    for (uint8 i = BASE_ATTACK; i < MAX_ATTACK; ++i)
        UpdateWeaponDependentCritAuras(WeaponAttackType(i));
}

void Player::UpdateWeaponDependentAuras(WeaponAttackType attackType)
{
    UpdateWeaponDependentCritAuras(attackType);
    UpdateDamageDoneMods(attackType);
    UpdateDamagePctDoneMods(attackType);
}

void Player::ApplyItemDependentAuras(Item* item, bool apply)
{
    if (apply)
    {
        PlayerSpellMap const& spells = GetSpellMap();
        for (auto itr = spells.begin(); itr != spells.end(); ++itr)
        {
            if (itr->second->state == PLAYERSPELL_REMOVED || itr->second->disabled)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
            if (!spellInfo || !spellInfo->IsPassive() || spellInfo->EquippedItemClass < 0)
                continue;

            if (!HasAura(itr->first) && HasItemFitToSpellRequirements(spellInfo))
                AddAura(itr->first, this);  // no SMSG_SPELL_GO in sniff found
        }
    }
    else
        RemoveItemDependentAurasAndCasts(item);
}

bool Player::CheckAttackFitToAuraRequirement(WeaponAttackType attackType, AuraEffect const* aurEff) const
{
/*    if (!HasWand() && !(aurEff->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL))
        return false;*/

    SpellInfo const* spellInfo = aurEff->GetSpellInfo();
    if (spellInfo->EquippedItemClass == -1)
        return true;

    Item* item = GetWeaponForAttack(attackType, true);
    if (!item || !item->IsFitToSpellRequirements(spellInfo))
        return false;

    return true;
}

bool Player::CanInteractWithQuestGiver(Object* questGiver)
{
    switch (questGiver->GetTypeId())
    {
        case TYPEID_UNIT:
            return GetNPCIfCanInteractWith(questGiver->GetGUID(), UNIT_NPC_FLAG_QUESTGIVER) != nullptr;
        case TYPEID_GAMEOBJECT:
            return GetGameObjectIfCanInteractWith(questGiver->GetGUID(), GAMEOBJECT_TYPE_QUESTGIVER) != nullptr;
        case TYPEID_PLAYER:
            return IsAlive() && questGiver->ToPlayer()->IsAlive();
        case TYPEID_ITEM:
            return IsAlive();
        default:
            break;
    }
    return false;
}

Creature* Player::GetNPCIfCanInteractWith(ObjectGuid guid, uint32 npcflagmask)
{
    // unit checks
    if (!guid)
        return nullptr;

    if (!IsInWorld())
        return nullptr;

    if (IsInFlight())
        return nullptr;

    // exist (we need look pets also for some interaction (quest/etc)
    Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*this, guid);
    if (!creature)
        return nullptr;

    // Deathstate checks
    if (!IsAlive() && !(creature->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_GHOST_VISIBLE || creature->IsSpiritService()))
        return nullptr;

    // alive or spirit healer
    if (!creature->IsAlive() && !(creature->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_CAN_INTERACT_WHILE_DEAD || creature->IsSpiritService()))
        return nullptr;

    // appropriate npc type
    if (npcflagmask && !creature->HasFlag(UNIT_NPC_FLAGS, npcflagmask))
        return nullptr;

    // not allow interaction under control, but allow with own pets
    if (creature->GetCharmerGUID())
        return nullptr;

    // hostile or unfriendly
    if (creature->GetReactionTo(this) <= REP_UNFRIENDLY)
        return nullptr;

    // not unfriendly
    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(creature->GetFaction());
    if(factionTemplate)
    {
        FactionEntry const* faction = sFactionStore.LookupEntry(factionTemplate->faction);
        if( faction->reputationListID >= 0 && GetReputationMgr().GetRank(faction) <= REP_UNFRIENDLY && !HasAuraEffect(29938,0) ) //needed for quest 9410 "A spirit guide"
            return nullptr;
    }

    // not too far, taken from CGGameUI::SetInteractTarget
    if (!creature->IsWithinDistInMap(this, creature->GetCombatReach() + 4.0f))
        return nullptr;

    return creature;
}

GameObject* Player::GetGameObjectIfCanInteractWith(ObjectGuid guid) const
{
    if (GameObject* go = GetMap()->GetGameObject(guid))
    {
        if (go->IsWithinDistInMap(this, go->GetInteractionDistance()))
            return go;

        TC_LOG_DEBUG("maps", "GetGameObjectIfCanInteractWith: GameObject '%s' [GUID: %u] is too far away from player %s [GUID: %u] to be used by him (distance=%f, maximal %f is allowed)", go->GetGOInfo()->name.c_str(),
            go->GetGUID().GetCounter(), GetName().c_str(), GetGUID().GetCounter(), go->GetDistance(this), go->GetInteractionDistance());
    }

    return nullptr;
}

GameObject* Player::GetGameObjectIfCanInteractWith(ObjectGuid guid, GameobjectTypes type) const
{
    if (GameObject* go = GetMap()->GetGameObject(guid))
    {
        if (go->GetGoType() == type)
        {
            if (go->IsWithinDistInMap(this, go->GetInteractionDistance()))
                return go;

            TC_LOG_DEBUG("maps", "GetGameObjectIfCanInteractWith: GameObject '%s' [GUID: %u] is too far away from player %s [GUID: %u] to be used by him (distance=%f, maximal %f is allowed)", (go->GetGOInfo()->name.c_str()),
                go->GetGUID().GetCounter(), this->GetName().c_str(), GetGUID().GetCounter(), go->GetDistance(this), go->GetInteractionDistance());
        }
    }

    return nullptr;
}

bool Player::IsUnderWater() const
{
    return IsInWater() &&
        GetPositionZ() < (GetMap()->GetWaterLevel(GetPositionX(),GetPositionY()) - 2.0f);
}

void Player::SetInWater(bool apply)
{
    if(m_isInWater==apply)
        return;

    //define player in water by opcodes
    //move player's guid into HateOfflineList of those mobs
    //which can't swim and move guid back into ThreatList when
    //on surface.
    //TODO: exist also swimming mobs, and function must be symmetric to enter/leave water
    m_isInWater = apply;

    // remove auras that need water/land
    RemoveAurasWithInterruptFlags(apply ? AURA_INTERRUPT_FLAG_NOT_ABOVEWATER : AURA_INTERRUPT_FLAG_NOT_UNDERWATER);
}

bool Player::IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index, WorldObject const* caster) const
{
    // players are immune to taunt (the aura and the spell effect)
    if (spellInfo->Effects[index].IsAura(SPELL_AURA_MOD_TAUNT))
        return true;
    if (spellInfo->Effects[index].IsEffect(SPELL_EFFECT_ATTACK_ME))
        return true;

    return Unit::IsImmunedToSpellEffect(spellInfo, index, caster);
}

void Player::SetGameMaster(bool on)
{
    if(on)
    {
        m_ExtraFlags |= PLAYER_EXTRA_GM_ON;
        SetFaction(FACTION_FRIENDLY);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);

        if (Pet* pet = GetPet())
            pet->SetFaction(FACTION_FRIENDLY);

        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
        ResetContestedPvP();

        SetPhaseMask(PHASEMASK_ANYWHERE, false);    // see and visible in all phases
        m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GM, GetSession()->GetSecurity());
    }
    else
    {
        SetPhaseMask(PHASEMASK_NORMAL, false);

        m_ExtraFlags &= ~ PLAYER_EXTRA_GM_ON;
        SetFactionForRace(GetRace());
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);

        if (Pet* pet = GetPet())
            pet->SetFaction(GetFaction());

        // restore FFA PvP Server state
        if(sWorld->IsFFAPvPRealm())
            SetFlag(PLAYER_FLAGS,PLAYER_FLAGS_FFA_PVP);

        // restore FFA PvP area state, remove not allowed for GM mounts
        UpdateArea(m_areaUpdateId);

        m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_PLAYER);
    }

    UpdateObjectVisibility();
}

void Player::SetGMVisible(bool on)
{
    uint32 transparence_spell;
    if (GetSession()->GetSecurity() <= SEC_GAMEMASTER1)
        transparence_spell = 37801; //Transparency 25%
    else
        transparence_spell = 37800; //Transparency 50%


    if(on)
    {
        m_ExtraFlags &= ~PLAYER_EXTRA_GM_INVISIBLE;         //remove flag

        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_PLAYER);
      /* // Reapply stealth/invisibility if active or show if not any
        if(HasAuraType(SPELL_AURA_MOD_STEALTH))
            SetVisibility(VISIBILITY_GROUP_STEALTH);
        //else if(HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
        //    SetVisibility(VISIBILITY_GROUP_INVISIBILITY);
        else
            SetVisibility(VISIBILITY_ON); */

        RemoveAurasDueToSpell(transparence_spell);
    }
    else
    {
        m_ExtraFlags |= PLAYER_EXTRA_GM_INVISIBLE;          //add flag

        SetAcceptWhispers(false);
        SetGameMaster(true);

        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, GetSession()->GetSecurity());

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(transparence_spell); //Transparency 50%
        if (spellInfo)
            AddAura(spellInfo->Id, this);
    }
}

bool Player::IsGroupVisibleFor(Player const* p) const
{
    switch(sWorld->getConfig(CONFIG_GROUP_VISIBILITY))
    {
        default: return IsInSameGroupWith(p);
        case 1:  return IsInSameRaidWith(p);
        case 2:  return GetTeam()==p->GetTeam();
    }
}

bool Player::IsInSameGroupWith(Player const* p) const
{
    return  p==this || (GetGroup() != nullptr &&
        GetGroup() == p->GetGroup() &&
        GetGroup()->SameSubGroup(this->ToPlayer(), p->ToPlayer()));
}

///- If the player is invited, remove him. If the group if then only 1 person, disband the group.
void Player::UninviteFromGroup()
{
    Group* group = GetGroupInvite();
    if (!group)
        return;

    group->RemoveInvite(this);

    if (group->IsCreated())
    {
        if (group->GetMembersCount() <= 1) // group has just 1 member => disband
            group->Disband(true);
    }
    else
    {
        if (group->GetInviteeCount() <= 1)
        {
            group->RemoveAllInvites();
            delete group;
        }
    }
}

void Player::RemoveFromGroup(Group* group, ObjectGuid guid, RemoveMethod method /*= GROUP_REMOVEMETHOD_DEFAULT*/, ObjectGuid kicker /*= ObjectGuid::Empty*/, char const* reason /*= nullptr*/)
{
    if (!group)
        return;

    group->RemoveMember(guid, method, kicker, reason);
}

void Player::SendLogXPGain(uint32 GivenXP, Unit* victim, uint32 RestXP)
{
    WorldPacket data(SMSG_LOG_XPGAIN, 21);
    data << uint64(victim ? victim->GetGUID() : 0);         // guid
    data << uint32(GivenXP+RestXP);                         // given experience
    data << uint8(victim ? 0 : 1);                          // 00-kill_xp type, 01-non_kill_xp type
    if(victim)
    {
        data << uint32(GivenXP);                            // experience without rested bonus
        data << float(1);                                   // 1 - none 0 - 100% group bonus output
    }
    data << uint8(0);                                       // new 2.4.0
    SendDirectMessage(&data);
}

void Player::GiveXP(uint32 xp, Unit* victim)
{
    if ( xp < 1 )
        return;

    if(!IsAlive())
        return;

    // Experience Blocking
    if (m_isXpBlocked)
        return;

    uint32 level = GetLevel();

    sScriptMgr->OnGivePlayerXP(this, xp, victim);

    // XP to money conversion processed in Player::RewardQuest
    if(level >= sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        return;

    // handle SPELL_AURA_MOD_XP_PCT auras
    Unit::AuraEffectList const& ModXPPctAuras = GetAuraEffectsByType(SPELL_AURA_MOD_XP_PCT);
    for(auto ModXPPctAura : ModXPPctAuras)
        xp = uint32(xp*(1.0f + ModXPPctAura->GetAmount() / 100.0f));

    Unit::AuraEffectList const& DummyAuras = GetAuraEffectsByType(SPELL_AURA_DUMMY);
    for(auto DummyAura : DummyAuras)
        if(DummyAura->GetId() == 32098 || DummyAura->GetId() == 32096) // Honor Hold's Favor || Thrallmar's Favor 
        {
           uint32 area_id = GetAreaId();
           if(area_id == 3483 || area_id == 3535 || area_id == 3562 || area_id == 3713)
                xp = uint32(xp*(1.0f + 5.0f / 100.0f));
        }


    // XP resting bonus for kill
    uint32 rested_bonus_xp = victim ? GetXPRestBonus(xp) : 0;

    SendLogXPGain(xp, victim, rested_bonus_xp);

    uint32 curXP = GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 newXP = curXP + xp + rested_bonus_xp;

    while( newXP >= nextLvlXP && level < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) )
    {
        newXP -= nextLvlXP;

        if ( level < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) )
            GiveLevel(level + 1);

        level = GetLevel();
        nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    }

    SetUInt32Value(PLAYER_XP, newXP);
}

// Update player to next level
// Current player experience not update (must be update by caller)
void Player::GiveLevel(uint32 level)
{
    uint8 oldLevel = GetLevel();
    if (level == GetLevel())
        return;

    if (Guild* guild = GetGuild())
        guild->UpdateMemberData(this, GUILD_MEMBER_DATA_LEVEL, level);

    PlayerLevelInfo info;
    sObjectMgr->GetPlayerLevelInfo(GetRace(), GetClass(), level, &info);

    PlayerClassLevelInfo classInfo;
    sObjectMgr->GetPlayerClassLevelInfo(GetClass(), level, &classInfo);

    // send levelup info to client
    WorldPacket data(SMSG_LEVELUP_INFO, (4+4+MAX_POWERS*4+MAX_STATS*4));
    data << uint32(level);
    data << uint32(int32(classInfo.basehealth) - int32(GetCreateHealth()));
    // for(int i = 0; i < MAX_POWERS; ++i)                  // Powers loop (0-6)
    data << uint32(int32(classInfo.basemana)   - int32(GetCreateMana()));
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
#ifdef LICH_KING
    data << uint32(0);
    data << uint32(0);
#endif
    // end for
    for(uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)          // Stats loop (0-4)
        data << uint32(int32(info.stats[i]) - GetCreateStat(Stats(i)));

    SendDirectMessage(&data);

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, Trinity::XP::GetXPForLevel(level));

    //update level, max level of skills
    if(GetLevel()!= level)
        m_Played_time[PLAYED_TIME_LEVEL] = 0;                   // Level Played Time reset

    SetLevel(level);
    UpdateSkillsForLevel();

    // save base values (bonuses already included in stored stats
    for(uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetCreateStat(Stats(i), info.stats[i]);

    SetCreateHealth(classInfo.basehealth);
    SetCreateMana(classInfo.basemana);

    InitTalentForLevel();
    InitTaxiNodesForLevel();

    UpdateAllStats();

    if(sWorld->getConfig(CONFIG_ALWAYS_MAXSKILL)) // Max weapon skill when leveling up
        UpdateSkillsToMaxSkillsForLevel();

    // set current level health and mana/energy to maximum after applying all mods.
    SetFullHealth();
    SetFullPower(POWER_MANA);

    // update level to hunter/summon pet
    if (Pet* pet = GetPet())
        pet->SynchronizeLevelWithOwner();

    sScriptMgr->OnPlayerLevelChanged(this, oldLevel);
}

void Player::InitTalentForLevel()
{
    uint32 level = GetLevel();
    // talents base at level diff ( talents = level - 9 but some can be used already)
    if(level < 10)
    {
        // Remove all talent points
        if(m_usedTalentCount > 0)                           // Free any used talents
        {
            ResetTalents(true);
            SetFreeTalentPoints(0);
        }
    }
    else
    {
        uint32 talentPointsForLevel = uint32((level-9)*sWorld->GetRate(RATE_TALENT));
        // if used more that have then reset
        if(m_usedTalentCount > talentPointsForLevel)
        {
            if (GetSession()->GetSecurity() < SEC_GAMEMASTER3)
                ResetTalents(true);
            else
                SetFreeTalentPoints(0);
        }
        // else update amount of free points
        else
            SetFreeTalentPoints(talentPointsForLevel-m_usedTalentCount);
    }
}

void Player::LearnTalent(uint32 talentId, uint32 talentRank)
{
    uint32 CurTalentPoints = GetFreeTalentPoints();

    if (CurTalentPoints == 0)
        return;

    if (talentRank >= MAX_TALENT_RANK)
        return;

    TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);

    if (!talentInfo)
        return;

    TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

    if (!talentTabInfo)
        return;

    // prevent learn talent for different class (cheating)
    if ((GetClassMask() & talentTabInfo->ClassMask) == 0)
        return;

    // prevent skip talent ranks (cheating)
    if (talentRank > 0 && !HasSpell(talentInfo->RankID[talentRank - 1]))
        return;

    // find current max talent rank (0~5)
    uint8 curtalent_maxrank = 0; // 0 = not learned any rank
    for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
    {
        if (talentInfo->RankID[rank] && HasSpell(talentInfo->RankID[rank]))
        {
            curtalent_maxrank = (rank + 1);
            break;
        }
    }

    // we already have same or higher talent rank learned
    if (curtalent_maxrank >= (talentRank + 1))
        return;

    // check if we have enough talent points
    if (CurTalentPoints < (talentRank - curtalent_maxrank + 1))
        return;

    // Check if it requires another talent
    if (talentInfo->DependsOn > 0)
    {
        if (TalentEntry const *depTalentInfo = sTalentStore.LookupEntry(talentInfo->DependsOn))
        {
            bool hasEnoughRank = false;
            for (int i = talentInfo->DependsOnRank; i < MAX_TALENT_RANK; i++)
            {
                if (depTalentInfo->RankID[i] != 0)
                    if (HasSpell(depTalentInfo->RankID[i]))
                        hasEnoughRank = true;
            }
            if (!hasEnoughRank)
                return;
        }
    }

    // Check if it requires spell
    if (talentInfo->DependsOnSpell && !HasSpell(talentInfo->DependsOnSpell))
        return;

    // Find out how many points we have in this field
    uint32 spentPoints = 0;

    uint32 tTab = talentInfo->TalentTab;
    if (talentInfo->Row > 0)
        for (uint32 i = 0; i < sTalentStore.GetNumRows(); i++)          // Loop through all talents.
            if (TalentEntry const* tmpTalent = sTalentStore.LookupEntry(i))                                  // the way talents are tracked
                if (tmpTalent->TalentTab == tTab)
                    for (int j = 0; j < MAX_TALENT_RANK; j++)
                        if (tmpTalent->RankID[j] != 0)
                            if (HasSpell(tmpTalent->RankID[j]))
                                spentPoints += j + 1;

    // not have required min points spent in talent tree
    if (spentPoints < (talentInfo->Row * MAX_TALENT_RANK))
        return;

    // spell not set in talent.dbc
    uint32 spellid = talentInfo->RankID[talentRank];
    if (spellid == 0)
    {
        TC_LOG_ERROR("entities.player", "Talent.dbc have for talent: %u Rank: %u spell id = 0", talentId, talentRank);
        return;
    }
    // Old WR hacks: 
    //Hack for Divine Spirit - talent learns more than one spell
    if (spellid == 14752) {
        if (HasSpellButDisabled(27681))
            LearnSpell(27681, false);
    }
    else if (spellid == 20217) {    // Benediction of Kings
        if (HasSpellButDisabled(25898))
            LearnSpell(25898, false);
    }
    else if (spellid == 20911 || spellid == 20912 || spellid == 20913 || spellid == 20914 || spellid == 27168) {
        if (HasSpellButDisabled(27169))
            LearnSpell(27169, false);
        else if (HasSpellButDisabled(25899))
            LearnSpell(25899, false);
    }

    // already known
    if (HasSpell(spellid))
        return;

    // learn! (other talent ranks will unlearned at learning)
    LearnSpell(spellid, false);
    //TC AddTalent(spellid, m_activeSpec, true);

    TC_LOG_DEBUG("misc", "Player::LearnTalent: TalentID: %u Rank: %u Spell: %u\n", talentId, talentRank, spellid);

    // update free talent points
    SetFreeTalentPoints(CurTalentPoints - (talentRank - curtalent_maxrank + 1)); //always 1 on BC? This is TC logic, probably needed because of the preview system skipping lower ranks
}

void Player::InitStatsForLevel(bool reapplyMods)
{
    if(reapplyMods)                                         //reapply stats values only on .reset stats (level) command
        _RemoveAllStatBonuses();

    PlayerClassLevelInfo classInfo;
    sObjectMgr->GetPlayerClassLevelInfo(GetClass(),GetLevel(),&classInfo);

    PlayerLevelInfo info;
    sObjectMgr->GetPlayerLevelInfo(GetRace(),GetClass(),GetLevel(),&info);

    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) );
    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, Trinity::XP::GetXPForLevel(GetLevel()));

    UpdateSkillsForLevel ();

    // set default cast time multiplier
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    // reset size before reapply auras
    SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);

    // save base values (bonuses already included in stored stats
    for(int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetCreateStat(Stats(i), info.stats[i]);

    for(int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetStat(Stats(i), info.stats[i]);

    SetCreateHealth(classInfo.basehealth);

    //set create powers
    SetCreateMana(classInfo.basemana);

    SetArmor(int32(m_createStats[STAT_AGILITY]*2));

    InitStatBuffMods();

    //reset rating fields values
    for(uint16 index = PLAYER_FIELD_COMBAT_RATING_1; index < PLAYER_FIELD_COMBAT_RATING_1 + MAX_COMBAT_RATING; ++index)
        SetUInt32Value(index, 0);

    SetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS,0);
    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        SetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG+i, 0);
        SetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i, 0);
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+i, 1.00f);
    }

    //reset attack power, damage and attack speed fields
    for (uint8 i = BASE_ATTACK; i < MAX_ATTACK; ++i)
        SetFloatValue(UNIT_FIELD_BASEATTACKTIME + i, float(BASE_ATTACK_TIME));

    SetFloatValue(UNIT_FIELD_MINDAMAGE, 0.0f );
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, 0.0f );
    SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, 0.0f );
    SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, 0.0f );
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, 0.0f );
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, 0.0f );

    SetUInt32Value(UNIT_FIELD_ATTACK_POWER,            0 );
    SetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS,       0 );
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER,0.0f);
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER,     0 );
    SetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,0 );
    SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER,0.0f);

    // Base crit values (will be recalculated in UpdateAllStats() at loading and in _ApplyAllStatBonuses() at reset
    SetFloatValue(PLAYER_CRIT_PERCENTAGE,0.0f);
    SetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE,0.0f);
    SetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE,0.0f);

    // Init spell schools (will be recalculated in UpdateAllStats() at loading and in _ApplyAllStatBonuses() at reset
    for (uint8 i = 0; i < 7; ++i)
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1+i, 0.0f);

    SetFloatValue(PLAYER_PARRY_PERCENTAGE, 0.0f);
    SetFloatValue(PLAYER_BLOCK_PERCENTAGE, 0.0f);
    SetUInt32Value(PLAYER_SHIELD_BLOCK, 0);

    // Dodge percentage
    SetFloatValue(PLAYER_DODGE_PERCENTAGE, 0.0f);

    // set armor (resistance 0) to original value (create_agility*2)
    SetArmor(int32(m_createStats[STAT_AGILITY]*2));
    SetResistanceBuffMods(SPELL_SCHOOL_NORMAL, true, 0.0f);
    SetResistanceBuffMods(SPELL_SCHOOL_NORMAL, false, 0.0f);
    // set other resistance to original value (0)
    for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; i++)
    {
        SetResistance(SpellSchools(i), 0);
        SetResistanceBuffMods(SpellSchools(i), true, 0.0f);
        SetResistanceBuffMods(SpellSchools(i), false, 0.0f);
    }

    SetUInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE,0);
    SetUInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE,0);
    for(int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
    {
        SetFloatValue(UNIT_FIELD_POWER_COST_MODIFIER+i,0.0f);
        SetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+i,0.0f);
    }
    // Init data for form but skip reapply item mods for form
    InitDataForForm(reapplyMods);

    // save new stats
    for (int i = POWER_MANA; i < MAX_POWERS; i++)
        SetMaxPower(Powers(i),  uint32(GetCreatePowers(Powers(i))));

    SetMaxHealth(classInfo.basehealth);                     // stamina bonus will applied later

    // cleanup mounted state (it will set correctly at aura loading if player saved at mount.
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);

    // cleanup unit flags (will be re-applied if need at aura load).
    RemoveFlag( UNIT_FIELD_FLAGS,
        UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_ATTACKABLE_1 |
        UNIT_FLAG_PET_IN_COMBAT  | UNIT_FLAG_SILENCED     | UNIT_FLAG_PACIFIED         |
        UNIT_FLAG_STUNNED | UNIT_FLAG_IN_COMBAT    | UNIT_FLAG_DISARMED         |
        UNIT_FLAG_CONFUSED       | UNIT_FLAG_FLEEING      | UNIT_FLAG_NOT_SELECTABLE   |
        UNIT_FLAG_SKINNABLE      | UNIT_FLAG_MOUNT        | UNIT_FLAG_TAXI_FLIGHT      );
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED );   // must be set

    // cleanup player flags (will be re-applied if need at aura load), to avoid have ghost flag without ghost aura, for example.
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK | PLAYER_FLAGS_DND | PLAYER_FLAGS_GM | PLAYER_FLAGS_GHOST | PLAYER_FLAGS_IN_PVP | PLAYER_FLAGS_FFA_PVP);

    RemoveStandFlags(UNIT_STAND_FLAGS_ALL);                 // one form stealth modified bytes
    //TC RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP | UNIT_BYTE2_FLAG_SANCTUARY);

    // restore if need some important flags
    SetUInt32Value(PLAYER_FIELD_BYTES2, 0 );                // flags empty by default

    if(reapplyMods)                                         //reapply stats values only on .reset stats (level) command
        _ApplyAllStatBonuses();

    // set current level health and mana/energy to maximum after applying all mods.
    SetFullHealth();
    SetFullPower(POWER_MANA);
    SetFullPower(POWER_ENERGY);
    if(GetPower(POWER_RAGE) > GetMaxPower(POWER_RAGE))
        SetFullPower(POWER_RAGE);
    SetFullPower(POWER_FOCUS);
    SetPower(POWER_HAPPINESS, 0);
#ifdef LICH_KING
    SetPower(POWER_RUNIC_POWER, 0);
#endif
}

void Player::SendInitialSpells()
{
    uint16 spellCooldowns = GetSpellHistory()->GetCooldownsSizeForPacket();
    uint16 spellCount = 0;

    WorldPacket data(SMSG_INITIAL_SPELLS, (1 + 2 + 4 * m_spells.size() + 2 + spellCooldowns * (2 + 2 + 2 + 4 + 4)));
    data << uint8(0);

    size_t countPos = data.wpos();
    data << uint16(spellCount);                             // spell count placeholder

    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if(itr->second->state == PLAYERSPELL_REMOVED)
            continue;

        if(!itr->second->active || itr->second->disabled)
            continue;

#ifdef LICH_KING
        data << uint32(itr->first);
#else
        data << uint16(itr->first);
#endif
        data << uint16(0);                                  // it's not slot id

        spellCount +=1;
    }

    data.put<uint16>(countPos,spellCount);                  // write real count value

    GetSpellHistory()->WritePacket<Player>(data);
   
    SendDirectMessage(&data);

    //TC_LOG_DEBUG("entities.player", "CHARACTER: Sent Initial Spells" );
}

void Player::RemoveMail(uint32 id)
{
    for(auto itr = m_mail.begin(); itr != m_mail.end();++itr)
    {
        if ((*itr)->messageID == id)
        {
            //do not delete item, because Player::removeMail() is called when returning mail to sender.
            m_mail.erase(itr);
            return;
        }
    }
}

void Player::SendMailResult(uint32 mailId, MailResponseType mailAction, MailResponseResult mailError, uint32 equipError, ObjectGuid::LowType item_guid, uint32 item_count)
{
    //LK ok
    WorldPacket data(SMSG_SEND_MAIL_RESULT, (4 + 4 + 4 + (mailError == MAIL_ERR_EQUIP_ERROR ? 4 : (mailAction == MAIL_ITEM_TAKEN ? 4 + 4 : 0))));
    data << (uint32) mailId;
    data << (uint32) mailAction;
    data << (uint32) mailError;
    if (mailError == MAIL_ERR_EQUIP_ERROR)
        data << (uint32)equipError;
    else if (mailAction == MAIL_ITEM_TAKEN)
    {
        data << (uint32) item_guid;                         // item guid low?
        data << (uint32) item_count;                        // item count?
    }
    SendDirectMessage(&data);
}

void Player::SendNewMail()
{
    // deliver undelivered mail
    WorldPacket data(SMSG_RECEIVED_MAIL, 4); //LK ok
    data << (uint32) 0;
    SendDirectMessage(&data);
}

void Player::UpdateNextMailTimeAndUnreads()
{
    // calculate next delivery time (min. from non-delivered mails
    // and recalculate unReadMail
    time_t cTime = WorldGameTime::GetGameTime();
    m_nextMailDelivereTime = 0;
    unReadMails = 0;
    for(auto & itr : m_mail)
    {
        if(itr->deliver_time > cTime)
        {
            if(!m_nextMailDelivereTime || m_nextMailDelivereTime > itr->deliver_time)
                m_nextMailDelivereTime = itr->deliver_time;
        }
        else if((itr->checked & MAIL_CHECK_MASK_READ) == 0)
            ++unReadMails;
    }
}

void Player::AddNewMailDeliverTime(time_t deliver_time)
{
    if(deliver_time <= WorldGameTime::GetGameTime())                          // ready now
    {
        ++unReadMails;
        SendNewMail();
    }
    else                                                    // not ready and no have ready mails
    {
        if(!m_nextMailDelivereTime || m_nextMailDelivereTime > deliver_time)
            m_nextMailDelivereTime =  deliver_time;
    }
}

bool Player::AddSpell(uint32 spell_id, bool active, bool learning, bool dependent, bool disabled, bool loading, uint32 fromSkill)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spell_id);
    if (!spellInfo)
    {
        // do character spell book cleanup (all characters)
        if(!IsInWorld() && !learning)                            // spell load case
        {
            TC_LOG_ERROR("entities.player","Player::AddSpell: Non-existed in SpellStore spell #%u request, deleting for all characters in `character_spell`.",spell_id);
            //DeleteSpellFromAllPlayers(spellId);
            CharacterDatabase.PExecute("DELETE FROM character_spell WHERE spell = '%u'",spell_id);
        }
        else
            TC_LOG_ERROR("entities.player","Player::AddSpell: Non-existed in SpellStore spell #%u request.",spell_id);

        return false;
    }

    if(!SpellMgr::IsSpellValid(spellInfo,this,false))
    {
        // do character spell book cleanup (all characters)
        if(!IsInWorld() && !learning)                            // spell load case
        {
            TC_LOG_ERROR("entities.player","Player::AddSpell: Broken spell #%u learning not allowed, deleting for all characters in `character_spell`.",spell_id);
            //DeleteSpellFromAllPlayers(spellId);
            CharacterDatabase.PExecute("DELETE FROM character_spell WHERE spell = '%u'",spell_id);
        }
        else
            TC_LOG_ERROR("entities.player","Player::AddSpell: Broken spell #%u learning not allowed.",spell_id);

        return false;
    }

    PlayerSpellState state = learning ? PLAYERSPELL_NEW : PLAYERSPELL_UNCHANGED;

    bool dependent_set = false;
    bool disabled_case = false;
    bool superceded_old = false;

    auto itr = m_spells.find(spell_id);
    /* TC
    // Remove temporary spell if found to prevent conflicts
    if (itr != m_spells.end() && itr->second->state == PLAYERSPELL_TEMPORARY)
        RemoveTemporarySpell(spell_id);
    else */ if (itr != m_spells.end())
    {
        uint32 next_active_spell_id = 0;
        // fix activate state for non-stackable low rank (and find next spell for !active case)
        if (!spellInfo->IsStackableWithRanks() && spellInfo->IsRanked())
        {
            if (uint32 next = sSpellMgr->GetNextSpellInChain(spell_id))
            {
                if (HasSpell(next))
                {
                    // high rank already known so this must !active
                    active = false;
                    next_active_spell_id = next;
                }
            }
        }

        // not do anything if already known in expected state
        if (itr->second->state != PLAYERSPELL_REMOVED && itr->second->active == active &&
            itr->second->dependent == dependent && itr->second->disabled == disabled)
        {
            if (!IsInWorld() && !learning)                   // explicitly load from DB and then exist in it already and set correctly
                itr->second->state = PLAYERSPELL_UNCHANGED;

            return false;
        }

        // dependent spell known as not dependent, overwrite state
        if (itr->second->state != PLAYERSPELL_REMOVED && !itr->second->dependent && dependent)
        {
            itr->second->dependent = dependent;
            if (itr->second->state != PLAYERSPELL_NEW)
                itr->second->state = PLAYERSPELL_CHANGED;
            dependent_set = true;
        }

        // update active state for known spell
        if(itr->second->active != active && itr->second->state != PLAYERSPELL_REMOVED && !itr->second->disabled)
        {
            itr->second->active = active;

            if (!IsInWorld() && !learning && !dependent_set) // explicitly load from DB and then exist in it already and set correctly
                itr->second->state = PLAYERSPELL_UNCHANGED;
            else if (itr->second->state != PLAYERSPELL_NEW)
                itr->second->state = PLAYERSPELL_CHANGED;

            if (active)
            {
                if (spellInfo->IsPassive() && HandlePassiveSpellLearn(spellInfo))
                    CastSpell(this, spell_id, true);
            }
            else if (IsInWorld())
            {
                if (next_active_spell_id)
                    SendSupercededSpell(spell_id, next_active_spell_id);
                else
                {
                    WorldPacket data(SMSG_REMOVED_SPELL, 4);
                    data << uint32(spell_id);
                    GetSession()->SendPacket(&data);
                }
            }

            return active;                                  // learn (show in spell book if active now)
        }

        if(itr->second->disabled != disabled && itr->second->state != PLAYERSPELL_REMOVED)
        {
            if(itr->second->state != PLAYERSPELL_NEW)
                itr->second->state = PLAYERSPELL_CHANGED;
            itr->second->disabled = disabled;

            if(disabled)
                return false;

            disabled_case = true;
        }
        else switch(itr->second->state)
        {
            case PLAYERSPELL_UNCHANGED:                     // known saved spell
                return false;
            case PLAYERSPELL_REMOVED:                       // re-learning removed not saved spell
            {
                delete itr->second;
                m_spells.erase(itr);
                state = PLAYERSPELL_CHANGED;
                break;                                      // need re-add
            }
            default:                                        // known not saved yet spell (new or modified)
            {
                // can be in case spell loading but learned at some previous spell loading
                if(loading && !learning)
                    itr->second->state = PLAYERSPELL_UNCHANGED;

                return false;
            }
        }
    }

    if(!disabled_case) // skip new spell adding if spell already known (disabled spells case)
    {
        // talent: unlearn all other talent ranks (high and low)
        if(TalentSpellPos const* talentPos = GetTalentSpellPos(spell_id))
        {
            if(TalentEntry const *talentInfo = sTalentStore.LookupEntry( talentPos->talent_id ))
            {
                for(uint32 rankSpellId : talentInfo->RankID)
                {
                    // skip learning spell and no rank spell case
                    if(!rankSpellId || rankSpellId==spell_id)
                        continue;

                    // skip unknown ranks
                    if(!HasSpell(rankSpellId))
                        continue;

                    RemoveSpell(rankSpellId);
                }
            }
        }
        // non talent spell: learn low ranks (recursive call)
        else if(uint32 prev_spell = sSpellMgr->GetPrevSpellInChain(spell_id))
        {
            if (!IsInWorld() || disabled)                    // at spells loading, no output, but allow save
                AddSpell(prev_spell,active,true,false,disabled,false, fromSkill);
            else                                            // at normal learning
                LearnSpell(prev_spell, true, fromSkill);
        }

        auto newspell = new PlayerSpell;
        newspell->state = state;
        newspell->active = active;
        newspell->dependent = dependent;
        newspell->disabled = disabled;

        // replace spells in action bars and spellbook to bigger rank if only one spell rank must be accessible
        if(newspell->active && !newspell->disabled && !SpellMgr::canStackSpellRanks(spellInfo) && sSpellMgr->GetSpellRank(spellInfo->Id) != 0)
        {
            for(auto & m_spell : m_spells)
            {
                if(m_spell.second->state == PLAYERSPELL_REMOVED)
                    continue;

                SpellInfo const *i_spellInfo = sSpellMgr->GetSpellInfo(m_spell.first);
                if(!i_spellInfo)
                    continue;

                if( sSpellMgr->IsRankSpellDueToSpell(spellInfo,m_spell.first) )
                {
                    if(m_spell.second->active)
                    {
                        if(sSpellMgr->IsHighRankOfSpell(spell_id,m_spell.first))
                        {
                            if (IsInWorld())                 // not send spell (re-/over-)learn packets at loading
                                SendSupercededSpell(m_spell.first, spell_id);

                            // mark old spell as disable (SMSG_SUPERCEDED_SPELL replace it in client by new)
                            m_spell.second->active = false;
                            if (m_spell.second->state != PLAYERSPELL_NEW)
                                m_spell.second->state = PLAYERSPELL_CHANGED;
                            superceded_old = true;          // new spell replace old in action bars and spell book.
                        }
                        else if(sSpellMgr->IsHighRankOfSpell(m_spell.first,spell_id))
                        {
                            if (IsInWorld())                 // not send spell (re-/over-)learn packets at loading
                                SendSupercededSpell(spell_id, m_spell.first);

                            // mark new spell as disable (not learned yet for client and will not learned)
                            newspell->active = false;
                            if (newspell->state != PLAYERSPELL_NEW)
                                newspell->state = PLAYERSPELL_CHANGED;
                        }
                    }
                }
            }
        }

        m_spells[spell_id] = newspell;

        // return false if spell disabled
        if (newspell->disabled)
            return false;
    }

    uint32 talentCost = GetTalentSpellCost(spell_id);

    // cast talents with SPELL_EFFECT_LEARN_SPELL (other dependent spells will learned later as not auto-learned)
    // note: all spells with SPELL_EFFECT_LEARN_SPELL isn't passive
    if(!loading && talentCost > 0 && IsSpellHaveEffect(spellInfo,SPELL_EFFECT_LEARN_SPELL) )
    {
        // ignore stance requirement for talent learn spell (stance set for spell only for client spell description show)
        CastSpell(this, spell_id, true);
    }
    // also cast passive spells (including all talents without SPELL_EFFECT_LEARN_SPELL) with additional checks
    else if (spellInfo->IsPassive())
    {
        if (HandlePassiveSpellLearn(spellInfo))
            CastSpell(this, spell_id, true);
    }
    else if( IsSpellHaveEffect(spellInfo,SPELL_EFFECT_SKILL_STEP) )
    {
        CastSpell(this, spell_id, true);
        return false;
    }

    // update used talent points count
    m_usedTalentCount += talentCost;

    // update free primary prof.points (if any, can be none in case GM .learn prof. learning)
    if(uint32 freeProfs = GetFreePrimaryProffesionPoints())
    {
        if(sSpellMgr->IsPrimaryProfessionFirstRankSpell(spell_id))
            SetFreePrimaryProffesions(freeProfs-1);
    }

    // add dependent skills
    SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spell_id);
    if (SpellLearnSkillNode const* spellLearnSkill = sSpellMgr->GetSpellLearnSkill(spell_id))
    {
        // add dependent skills if this spell is not learned from adding skill already
        if (spellLearnSkill->skill != fromSkill)
        {
            uint32 skill_value = GetPureSkillValue(spellLearnSkill->skill);
            uint32 skill_max_value = GetPureMaxSkillValue(spellLearnSkill->skill);

            if (skill_value < spellLearnSkill->value)
                skill_value = spellLearnSkill->value;

            uint32 new_skill_max_value = spellLearnSkill->maxvalue == 0 ? GetMaxSkillValueForLevel() : spellLearnSkill->maxvalue;

            if (skill_max_value < new_skill_max_value)
                skill_max_value = new_skill_max_value;

            SetSkill(spellLearnSkill->skill, spellLearnSkill->step, skill_value, skill_max_value);
        }
    }
    else
    {
        // not ranked skills
        for (auto _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(_spell_idx->second->skillId);
            if(!pSkill)
                continue;

            if (pSkill->id == fromSkill)
                continue;

            if( (_spell_idx->second->AutolearnType == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN && !HasSkill(pSkill->id)) ||
                // poison special case, not have SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN
                (pSkill->id==SKILL_POISONS && _spell_idx->second->max_value==0) ||
                // lockpicking special case, not have SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN
                ((pSkill->id==SKILL_LOCKPICKING
#ifdef LICH_KING
                    // Also added for runeforging. It's already confirmed this happens upon learning for Death Knights, not from character creation.
                    || pSkill->id == SKILL_RUNEFORGING
#endif
                    ) && _spell_idx->second->max_value==0) )
            {
                LearnDefaultSkill(pSkill->id, 0);
            }

#ifdef LICH_KING
            if (pSkill->id == SKILL_MOUNTS && !Has310Flyer(false))
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    if (spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED &&
                        spellInfo->Effects[i].CalcValue() == 310)
                        SetHas310Flyer(true);
#endif
        }
    }

    SpellLearnSpellMapBounds spell_bounds = sSpellMgr->GetSpellLearnSpellMapBounds(spell_id);

    // learn dependent spells
    for (auto itr2 = spell_bounds.first; itr2 != spell_bounds.second; ++itr2)
    {
        if(!itr2->second.autoLearned)
        {
            if (!IsInWorld() /* TC || !itr2->second.active*/)       // at spells loading, no output, but allow save
                AddSpell(itr2->second.spell,true,true,false,false,loading);
            else                                            // at normal learning
                LearnSpell(itr2->second.spell, true);
        }
    }

#ifdef LICH_KING
    if (!GetSession()->PlayerLoading())
    {
        // not ranked skills
        for (SkillLineAbilityMap::const_iterator _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
        {
            UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE, _spell_idx->second->skillId);
            UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS, _spell_idx->second->skillId);
        }

        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL, spellId);
    }
#endif

    // return true (for send learn packet) only if spell active (in case ranked spells) and not replace old spell
    return active && !disabled
#ifdef LICH_KING
        //not sure about LK, but this breaks icon profession icon when learning a new rank on BC
        && !superceded_old
#endif
        ;
}

void Player::LearnSpell(uint32 spell_id, bool dependent, uint32 fromSkill /*= 0*/)
{
    auto itr = m_spells.find(spell_id);

    bool disabled = (itr != m_spells.end()) ? itr->second->disabled : false;
    bool active = disabled ? itr->second->active : true;

    bool learning = AddSpell(spell_id,active, true, dependent, false, false, fromSkill);

    // prevent duplicated entires in spell book
    if (learning && IsInWorld())
    {
        WorldPacket data(SMSG_LEARNED_SPELL, 6);
        data << uint32(spell_id);
#ifdef LICH_KING
        data << uint16(0);
#endif
        GetSession()->SendPacket(&data);
    }

    // learn all disabled higher ranks (recursive)
    if (disabled)
    {
        if (uint32 nextSpell = sSpellMgr->GetNextSpellInChain(spell_id))
        {
            auto iter = m_spells.find(nextSpell);
            if (iter != m_spells.end() && iter->second->disabled)
                LearnSpell(nextSpell, false, fromSkill);
        }

        /* TC
        SpellsRequiringSpellMapBounds spellsRequiringSpell = sSpellMgr->GetSpellsRequiringSpellBounds(spell_id);
        for (SpellsRequiringSpellMap::const_iterator itr2 = spellsRequiringSpell.first; itr2 != spellsRequiringSpell.second; ++itr2)
        {
        PlayerSpellMap::iterator iter2 = m_spells.find(itr2->second);
        if (iter2 != m_spells.end() && iter2->second->disabled)
        LearnSpell(itr2->second, false, fromSkill);
        }
        */
    }
}

void Player::RemoveSpell(uint32 spell_id, bool disabled)
{
    auto itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return;

    if(itr->second->state == PLAYERSPELL_REMOVED || (disabled && itr->second->disabled))
        return;

    // unlearn non talent higher ranks (recursive)
    SpellChainNode const* node = sSpellMgr->GetSpellChainNode(spell_id);
    if (node)
    {
        if(node->next && HasSpell(node->next->Id) && !GetTalentSpellPos(node->next->Id))
            RemoveSpell(node->next->Id,disabled);
    }
    //unlearn spells dependent from recently removed spells
    SpellsRequiringSpellMap const& reqMap = sSpellMgr->GetSpellsRequiringSpell();
    auto itr2 = reqMap.find(spell_id);
    for (uint32 i = reqMap.count(spell_id); i > 0; i--, itr2++)
        RemoveSpell(itr2->second,disabled);

    // re-search, it can be corrupted in prev loop
    itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return;

    // removing
    WorldPacket data(SMSG_REMOVED_SPELL, 4);
    data << uint16(spell_id);
    SendDirectMessage(&data);

    if (disabled)
    {
        itr->second->disabled = disabled;
        if(itr->second->state != PLAYERSPELL_NEW)
            itr->second->state = PLAYERSPELL_CHANGED;
    }
    else
    {
        if(itr->second->state == PLAYERSPELL_NEW)
        {
            delete itr->second;
            m_spells.erase(itr);
        }
        else
            itr->second->state = PLAYERSPELL_REMOVED;
    }

    RemoveAurasDueToSpell(spell_id);

    // remove pet auras
    if(PetAura const* petSpell = sSpellMgr->GetPetAura(spell_id))
        RemovePetAura(petSpell);

    // free talent points
    uint32 talentCosts = GetTalentSpellCost(spell_id);
    if(talentCosts > 0)
    {
        if(talentCosts < m_usedTalentCount)
            m_usedTalentCount -= talentCosts;
        else
            m_usedTalentCount = 0;
    }

    // update free primary prof.points (if not overflow setting, can be in case GM use before .learn prof. learning)
    if(sSpellMgr->IsPrimaryProfessionFirstRankSpell(spell_id))
    {
        uint32 freeProfs = GetFreePrimaryProffesionPoints()+1;
        if(freeProfs <= sWorld->getConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL))
            SetFreePrimaryProffesions(freeProfs);
    }

    // remove dependent skill
    SpellLearnSkillNode const* spellLearnSkill = sSpellMgr->GetSpellLearnSkill(spell_id);
    if(spellLearnSkill)
    {
        uint32 prev_spell = sSpellMgr->GetPrevSpellInChain(spell_id);
        if(!prev_spell)                                     // first rank, remove skill
            SetSkill(spellLearnSkill->skill,0,0,0);
        else
        {
            // search prev. skill setting by spell ranks chain
            SpellLearnSkillNode const* prevSkill = sSpellMgr->GetSpellLearnSkill(prev_spell);
            while(!prevSkill && prev_spell)
            {
                prev_spell = sSpellMgr->GetPrevSpellInChain(prev_spell);
                prevSkill = sSpellMgr->GetSpellLearnSkill(sSpellMgr->GetFirstSpellInChain(prev_spell));
            }

            if(!prevSkill)                                  // not found prev skill setting, remove skill
                SetSkill(spellLearnSkill->skill,0,0,0);
            else                                            // set to prev. skill setting values
            {
                uint32 skill_value = GetPureSkillValue(prevSkill->skill);
                uint32 skill_max_value = GetPureMaxSkillValue(prevSkill->skill);

                if(skill_value >  prevSkill->value)
                    skill_value = prevSkill->value;

                uint32 new_skill_max_value = prevSkill->maxvalue == 0 ? GetMaxSkillValueForLevel() : prevSkill->maxvalue;

                if(skill_max_value > new_skill_max_value)
                    skill_max_value =  new_skill_max_value;

                SetSkill(prevSkill->skill, prevSkill->step, skill_value,skill_max_value);
            }
        }

    }
    else
    {
        // not ranked skills
        SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spell_id);
        for(auto _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(_spell_idx->second->skillId);
            if(!pSkill)
                continue;

            if(_spell_idx->second->AutolearnType == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN ||
                // poison special case, not have SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN
                (pSkill->id==SKILL_POISONS && _spell_idx->second->max_value==0) ||
                // lockpicking special case, not have SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN
                (pSkill->id==SKILL_LOCKPICKING && _spell_idx->second->max_value==0) )
            {
                // not reset skills for professions, class and racial abilities
                if( (pSkill->categoryId==SKILL_CATEGORY_SECONDARY || pSkill->categoryId==SKILL_CATEGORY_PROFESSION) &&
                    (SpellMgr::IsProfessionSkill(pSkill->id) || _spell_idx->second->racemask!=0) )
                    continue;

                if (pSkill->categoryId == SKILL_CATEGORY_CLASS || pSkill->categoryId == SKILL_CATEGORY_WEAPON) // When do we need to reset this? I added this because it made faction-changed characters forget almost all their spells
                    continue;

                SetSkill(pSkill->id, 0, 0, 0 );
            }
        }
    }

    // remove dependent spells
    SpellLearnSpellMapBounds spell_bounds = sSpellMgr->GetSpellLearnSpellMapBounds(spell_id);

    for (auto itr3 = spell_bounds.first; itr3 != spell_bounds.second; ++itr3)
        RemoveSpell(itr3->second.spell, disabled);
}

void Player::RemoveArenaSpellCooldowns(bool removeActivePetCooldowns)
{
    // remove cooldowns on spells that have <= 10/15 min CD (sun: also remove on equality - /cmangos/mangos-tbc/commit/f118d12e93d4136732d61b56d7a9a8a5bc3f4e29)
    // Has to be in sync with SPELL_FAILED_NOT_IN_ARENA check in Spell::CheckCast
    GetSpellHistory()->ResetCooldowns([&](SpellHistory::CooldownStorageType::iterator itr) -> bool
    {
        SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(itr->first);
        return spellInfo->RecoveryTime <= ARENA_MAX_COOLDOWN && spellInfo->CategoryRecoveryTime <= ARENA_MAX_COOLDOWN;
    }, true);

    // pet cooldowns
    if (removeActivePetCooldowns)
        if (Pet* pet = GetPet())
            pet->GetSpellHistory()->ResetAllCooldowns();
}

uint32 Player::ResetTalentsCost() const
{
    // The first time reset costs 1 gold
    if(m_resetTalentsCost < 1*GOLD)
        return 1*GOLD;
    // then 5 gold
    else if(m_resetTalentsCost < 5*GOLD)
        return 5*GOLD;
    // After that it increases in increments of 5 gold
    else if(m_resetTalentsCost < 10*GOLD)
        return 10*GOLD;
    else
    {
        uint32 months = (WorldGameTime::GetGameTime() - m_resetTalentsTime)/MONTH;
        if(months > 0)
        {
            // This cost will be reduced by a rate of 5 gold per month
            int32 new_cost = int32(m_resetTalentsCost) - 5*GOLD*months;
            // to a minimum of 10 gold.
            return (new_cost < 10*GOLD ? 10*GOLD : new_cost);
        }
        else
        {
            // After that it increases in increments of 5 gold
            int32 new_cost = m_resetTalentsCost + 5*GOLD;
            // until it hits a cap of 50 gold.
            if(new_cost > 50*GOLD)
                new_cost = 50*GOLD;
            return new_cost;
        }
    }
}

bool Player::ResetTalents(bool no_cost)
{
    sScriptMgr->OnPlayerTalentsReset(this, no_cost);

    // not need after this call
    if(HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
    {
        m_atLoginFlags = m_atLoginFlags & ~AT_LOGIN_RESET_TALENTS;
        CharacterDatabase.PExecute("UPDATE characters set at_login = at_login & ~ %u WHERE guid ='%u'", uint32(AT_LOGIN_RESET_TALENTS), GetGUID().GetCounter());
    }

    uint32 level = GetLevel();
    uint32 talentPointsForLevel = level < 10 ? 0 : uint32((level-9)*sWorld->GetRate(RATE_TALENT));

    if (m_usedTalentCount == 0)
    {
        SetFreeTalentPoints(talentPointsForLevel);
        return false;
    }

    uint32 cost = 0;

    if(!no_cost && !sWorld->getConfig(CONFIG_NO_RESET_TALENT_COST))
    {
        cost = ResetTalentsCost();

        if (GetMoney() < cost)
        {
            SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, nullptr, 0, 0);
            return false;
        }
    }

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); i++)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);

        if (!talentInfo) continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );

        if(!talentTabInfo)
            continue;

        // unlearn only talents for character class
        // some spell learned by one class as normal spells or know at creation but another class learn it as talent,
        // to prevent unexpected lost normal learned spell skip another class talents
        if( (GetClassMask() & talentTabInfo->ClassMask) == 0 )
            continue;

        for (uint32 j : talentInfo->RankID)
        {
            for(auto itr = GetSpellMap().begin(); itr != GetSpellMap().end();)
            {
                if(itr->second->state == PLAYERSPELL_REMOVED || itr->second->disabled)
                {
                    ++itr;
                    continue;
                }

                // remove learned spells (all ranks)
                uint32 itrFirstId = sSpellMgr->GetFirstSpellInChain(itr->first);

                // unlearn if first rank is talent or learned by talent
                if (itrFirstId == j || sSpellMgr->IsSpellLearnToSpell(j,itrFirstId))
                {
                    RemoveSpell(itr->first,!IsPassiveSpell(itr->first));
                    itr = GetSpellMap().begin();
                    continue;
                }
                else
                    ++itr;
            }
        }
    }

    SetFreeTalentPoints(talentPointsForLevel);

    if(!no_cost)
    {
        ModifyMoney(-(int32)cost);

        m_resetTalentsCost = cost;
        m_resetTalentsTime = WorldGameTime::GetGameTime();
    }

    //FIXME: remove pet before or after unlearn spells? for now after unlearn to allow removing of talent related, pet affecting auras
    RemovePet(nullptr,PET_SAVE_NOT_IN_SLOT, true);

    return true;
}

void Player::SetFreeTalentPoints(uint32 points)
{
    sScriptMgr->OnPlayerFreeTalentPointsChanged(this, points);
    SetUInt32Value(PLAYER_CHARACTER_POINTS1, points);
}

bool Player::_removeSpell(uint16 spell_id)
{
    auto itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        delete itr->second;
        m_spells.erase(itr);
        return true;
    }
    return false;
}

Mail* Player::GetMail(uint32 id)
{
    for(auto & itr : m_mail)
    {
        if (itr->messageID == id)
        {
            return itr;
        }
    }
    return nullptr;
}

void Player::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    for(uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(m_items[i] == nullptr)
            continue;

        m_items[i]->BuildCreateUpdateBlockForPlayer( data, target );
    }

    if(target == this)
    {

        for(uint8 i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if(m_items[i] == nullptr)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer( data, target );
        }
        for(uint8 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
        {
            if(m_items[i] == nullptr)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer( data, target );
        }
    }

    Unit::BuildCreateUpdateBlockForPlayer(data, target);
}

void Player::DestroyForPlayer(Player *target, bool onDeath) const
{
    Unit::DestroyForPlayer(target, onDeath);

    for(uint8 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(m_items[i] == nullptr)
            continue;

        m_items[i]->DestroyForPlayer(target);
    }

    if(target == this)
    {
        for(uint8 i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if(m_items[i] == nullptr)
                continue;

            m_items[i]->DestroyForPlayer(target);
        }
        for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
        {
            if(m_items[i] == nullptr)
                continue;

            m_items[i]->DestroyForPlayer(target);
        }
    }
}

bool Player::HasSpell(uint32 spell) const
{
    auto itr = m_spells.find((uint16)spell);
    return (itr != m_spells.end() && itr->second->state != PLAYERSPELL_REMOVED && !itr->second->disabled);
}

bool Player::HasSpellButDisabled(uint32 spell) const
{
    auto itr = m_spells.find((uint16)spell);
    return (itr != m_spells.end() && itr->second->state != PLAYERSPELL_REMOVED && itr->second->disabled);
}

TrainerSpellState Player::GetTrainerSpellState(TrainerSpell const* trainer_spell) const
{
    if (!trainer_spell)
        return TRAINER_SPELL_RED;

    if (!trainer_spell->spell)
        return TRAINER_SPELL_RED;

    // known spell
    if(HasSpell(trainer_spell->spell))
        return TRAINER_SPELL_GRAY;

    // check race/class requirement
    if(!IsSpellFitByClassAndRace(trainer_spell->spell))
        return TRAINER_SPELL_RED;

    // check level requirement
    if(GetLevel() < trainer_spell->reqlevel)
        return TRAINER_SPELL_RED;

    if(SpellChainNode const* spell_chain = sSpellMgr->GetSpellChainNode(trainer_spell->spell))
    {
        // check prev.rank requirement
        if(spell_chain->prev && !HasSpell(spell_chain->prev->Id))
            return TRAINER_SPELL_RED;
    }

    if(uint32 spell_req = sSpellMgr->GetSpellRequired(trainer_spell->spell))
    {
        // check additional spell requirement
        if(!HasSpell(spell_req))
            return TRAINER_SPELL_RED;
    }

    // check skill requirement
    if(trainer_spell->reqskill && GetBaseSkillValue(trainer_spell->reqskill) < trainer_spell->reqskillvalue)
        return TRAINER_SPELL_RED;

    SpellInfo const* spell = sSpellMgr->GetSpellInfo(trainer_spell->spell);
    if (!spell)
    {
        TC_LOG_ERROR("entities.player", "GetTrainerSpellState could not find spell %u", trainer_spell->spell);
        return TRAINER_SPELL_RED;
    }

    // secondary prof. or not prof. spell
    uint32 skill = spell->Effects[1].MiscValue;

    if(spell->Effects[1].Effect != SPELL_EFFECT_SKILL || !SpellMgr::IsPrimaryProfessionSkill(skill))
        return TRAINER_SPELL_GREEN;

    // check primary prof. limit
    if(sSpellMgr->IsPrimaryProfessionFirstRankSpell(spell->Id) && GetFreePrimaryProffesionPoints() == 0)
        return TRAINER_SPELL_RED;

    return TRAINER_SPELL_GREEN;
}

void Player::LeaveAllArenaTeams(ObjectGuid guid)
{
    CharacterCacheEntry const* characterInfo = sCharacterCache->GetCharacterCacheByGuid(guid);
    if (!characterInfo)
        return;

    for (uint8 i = 0; i < MAX_ARENA_SLOT; ++i)
    {
        uint32 arenaTeamId = characterInfo->arenaTeamId[i];
        if (arenaTeamId != 0)
        {
            ArenaTeam* arenaTeam = sArenaTeamMgr->GetArenaTeamById(arenaTeamId);
            if (arenaTeam)
                arenaTeam->DeleteMember(guid, true);
        }
    }
}

void Player::DeleteOldCharacters()
{
    uint32 keepDays = sWorld->getIntConfig(CONFIG_CHARDELETE_KEEP_DAYS);
    if (!keepDays)
        return;
    if (uint64(keepDays) * DAY > INT_MAX)
    {
        TC_LOG_ERROR("entities.player", "Player::DeleteOldCharacters: Invalid % days, reverting to 1 year", keepDays);
        keepDays = 365;
    }

    TC_LOG_INFO("entities.player", "Player::DeleteOldCharacters: Deleting all characters which have been deleted %u days before...", keepDays);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_OLD_CHARS);
    stmt->setUInt32(0, uint32(WorldGameTime::GetGameTime() - time_t(keepDays * DAY)));
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        TC_LOG_DEBUG("entities.player", "Player::DeleteOldCharacters: Found " UI64FMTD " character(s) to delete", result->GetRowCount());
        do
        {
            Field* fields = result->Fetch();
            Player::DeleteFromDB(ObjectGuid(HighGuid::Player, fields[0].GetUInt32()), fields[1].GetUInt32(), true, true);
        } while (result->NextRow());
    }
}

void Player::DeleteFromDB(ObjectGuid playerguid, uint32 accountId, bool updateRealmChars, bool deleteFinally)
{
    uint32 charDelete_method = deleteFinally ? CHAR_DELETE_REMOVE : CHAR_DELETE_UNLINK;

    ObjectGuid::LowType guid = playerguid.GetCounter();

    CharacterCacheEntry const* characterInfo = sCharacterCache->GetCharacterCacheByGuid(playerguid);
    std::string name;
    if (characterInfo)
        name = characterInfo->name;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if(ObjectGuid::LowType guildId = sCharacterCache->GetCharacterGuildIdByGuid(playerguid))
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))
            guild->DeleteMember(trans, playerguid, false, false, true);

    // remove from arena teams
    LeaveAllArenaTeams(playerguid);

    // the player was uninvited already on logout so just remove from group
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GROUP_MEMBER);
    stmt->setUInt32(0, guid);
    PreparedQueryResult resultGroup = CharacterDatabase.Query(stmt);

    if(resultGroup)
        if (Group* group = sGroupMgr->GetGroupByDbStoreId((*resultGroup)[0].GetUInt32()))
            RemoveFromGroup(group, playerguid);

    // remove signs from petitions (also remove petitions if owner);
    RemovePetitionsAndSigns(trans, playerguid, CHARTER_TYPE_ANY);

    switch (charDelete_method)
    {
        // Completely remove from the database
        case CHAR_DELETE_REMOVE:
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_COD_ITEM_MAIL);
            stmt->setUInt32(0, guid);
            PreparedQueryResult resultMail = CharacterDatabase.Query(stmt);

            if (resultMail)
            {
                do
                {
                    Field* mailFields = resultMail->Fetch();

                    uint32 mail_id = mailFields[0].GetUInt32();
                    uint8 mailType = mailFields[1].GetUInt8();
                    uint16 mailTemplateId = mailFields[2].GetUInt16();
                    uint32 sender = mailFields[3].GetUInt32();
                    std::string subject = mailFields[4].GetString();
                    uint32 itemTextId = mailFields[5].GetUInt32();
                    //std::string body = mailFields[5].GetString();
                    uint32 money = mailFields[6].GetUInt32();
                    bool has_items = mailFields[7].GetBool();

                    //itemTextId can't be reused because it will be deleted with previous mail, create a new one
                    std::string body;
                    if(itemTextId)
                        body = sObjectMgr->GetItemText(itemTextId);

                    // We can return mail now
                    // So firstly delete the old one
                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_BY_ID);
                    stmt->setUInt32(0, mail_id);
                    trans->Append(stmt);

                    // Mail is not from player
                    if (mailType != MAIL_NORMAL)
                    {
                        if (has_items)
                        {
                            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM_BY_ID);
                            stmt->setUInt32(0, mail_id);
                            trans->Append(stmt);
                        }
                        continue;
                    }

                    MailDraft draft(subject, body);
                    if (mailTemplateId)
                        draft = MailDraft(mailTemplateId, false);    // items are already included

                    if (has_items)
                    {
                        // Data needs to be at first place for Item::LoadFromDB
                        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_MAILITEMS);
                        stmt->setUInt32(0, mail_id);
                        PreparedQueryResult resultItems = CharacterDatabase.Query(stmt);
                        if (resultItems)
                        {
                            do
                            {
                                Field* itemFields = resultItems->Fetch();
                                ObjectGuid::LowType item_guidlow = itemFields[48].GetUInt32();
                                uint32 item_template = itemFields[49].GetUInt32();

                                ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(item_template);
                                if (!itemProto)
                                {
                                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_INSTANCE);
                                    stmt->setUInt32(0, item_guidlow);
                                    trans->Append(stmt);
                                    continue;
                                }

                                Item* pItem = NewItemOrBag(itemProto);
                                if (!pItem->LoadFromDB(item_guidlow, playerguid, itemFields, item_template))
                                {
                                    pItem->FSetState(ITEM_REMOVED);
                                    pItem->SaveToDB(trans);              // it also deletes item object!
                                    continue;
                                }

                                draft.AddItem(pItem);
                            } while (resultItems->NextRow());
                        }
                    }

                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM_BY_ID);
                    stmt->setUInt32(0, mail_id);
                    trans->Append(stmt);

                    uint32 pl_account = sCharacterCache->GetCharacterAccountIdByGuid(playerguid);

                    draft.AddMoney(money).SendReturnToSender(pl_account, guid, sender, trans);
                } while (resultMail->NextRow());
            }

            // unsummon and delete for pets in world is not required: player deleted from CLI or character list with not loaded pet.
            // Get guids of character's pets, will deleted in transaction
            QueryResult resultPets = CharacterDatabase.PQuery("SELECT id FROM character_pet WHERE owner = '%u'", guid);

            // NOW we can finally clear other DB data related to character
            if (resultPets)
            {
                do
                {
                    Field *fields3 = resultPets->Fetch();
                    ObjectGuid::LowType petguidlow = fields3[0].GetUInt32();
                    Pet::DeleteFromDB(petguidlow);
                } while (resultPets->NextRow());
            }

            trans->PAppend("DELETE FROM characters WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_declinedname WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_action WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_aura WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_gifts WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_homebind WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_instance WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_inventory WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_queststatus WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_reputation WHERE guid = '%u'", guid);
            trans->PAppend("DELETE FROM character_spell WHERE guid = '%u'", guid);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL_COOLDOWNS);
            stmt->setUInt32(0, guid);
            trans->Append(stmt);

            trans->PAppend("DELETE FROM gm_tickets WHERE playerGuid = '%u'", guid);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_INSTANCE_BY_OWNER);
            stmt->setUInt32(0, guid);
            trans->Append(stmt);

            trans->PAppend("DELETE FROM character_social WHERE guid = '%u' OR friend='%u'", guid, guid);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL);
            stmt->setUInt32(0, guid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEMS);
            stmt->setUInt32(0, guid);
            trans->Append(stmt);

            trans->PAppend("DELETE FROM character_pet WHERE owner = '%u'", guid);
            trans->PAppend("DELETE FROM character_pet_declinedname WHERE owner = '%u'", guid);
            trans->PAppend("DELETE FROM character_skills WHERE guid = '%u'", guid);

            PreparedStatement* stmt2 = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PLAYER_BGDATA);
            stmt2->setUInt32(0, guid);
            trans->Append(stmt2);

        } break;
        // The character gets unlinked from the account, the name gets freed up and appears as deleted ingame
        case CHAR_DELETE_UNLINK:
        {
            PreparedStatement* _stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_DELETE_INFO);
            _stmt->setUInt32(0, guid);
            trans->Append(_stmt);
            break;
        } break;

        default:
            TC_LOG_ERROR("entities.player", "Player::DeleteFromDB: Tried to delete player (%u) with unsupported delete method (%u).",
                playerguid.GetCounter(), charDelete_method);
            return;
    }

    CharacterDatabase.CommitTransaction(trans);

    if(updateRealmChars)
        sWorld->UpdateRealmCharCount(accountId);

    sCharacterCache->DeleteCharacterCacheEntry(playerguid, name);
}

/* Preconditions:
  - a resurrectable corpse must not be loaded for the player (only bones)
  - the player must be in world
*/
void Player::BuildPlayerRepop()
{
#ifdef LICH_KING
    WorldPacket data(SMSG_PRE_RESURRECT, GetPackGUID().size());
    data << GetPackGUID();
    SendMessageToSet(&data, true);
#endif

    if(GetRace() == RACE_NIGHTELF)
        CastSpell(this, 20584, true);                       // auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
    CastSpell(this, 8326, true);                            // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

    // there must be SMSG.STOP_MIRROR_TIMER
    // there we must send 888 opcode

    // the player cannot have a corpse already on current map, only bones which are not returned by GetCorpseLocation
    WorldLocation corpseLocation = GetCorpseLocation();
    if (corpseLocation.GetMapId() == GetMapId())
    {
        TC_LOG_ERROR("entities.player","BuildPlayerRepop: player %s(%d) already has a corpse", GetName().c_str(), GetGUID().GetCounter());
    return;
    }

    // create a corpse and place it at the player's location
    Corpse *corpse = CreateCorpse();
    if(!corpse)
    {
        TC_LOG_ERROR("entities.player","ERROR creating corpse for Player %s [%u]", GetName().c_str(), GetGUID().GetCounter());
        return;
    }
    GetMap()->AddToMap(corpse);

    // convert player body to ghost
    SetDeathState(DEAD);
    SetHealth(1);

    // BG - remove insignia related
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

    int32 corpseReclaimDelay = CalculateCorpseReclaimDelay();

    if (corpseReclaimDelay >= 0)
        SendCorpseReclaimDelay(corpseReclaimDelay);

    // to prevent cheating
    corpse->ResetGhostTime();

    StopMirrorTimers();                                     //disable timers(bars)

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, (float)1.0);   //see radius of death player?

    SetAnimationTier(UnitAnimationTier::Ground);

    // OnPlayerRepop hook
    //sScriptMgr->OnPlayerRepop(this);
}

void Player::ResurrectPlayer(float restore_percent, bool applySickness)
{
    WorldPacket data(SMSG_DEATH_RELEASE_LOC, 4*4);          // remove spirit healer position, LK ok
    data << uint32(-1);
    data << float(0);
    data << float(0);
    data << float(0);
    SendDirectMessage(&data);

    if (!HasUnitState(UNIT_STATE_ROOT))
        SetRooted(false);

    // send spectate addon message
    if (HaveSpectators())
    {
        SpectatorAddonMsg msg;
        msg.SetPlayer(GetName());
        msg.SetStatus(true);
        SendSpectatorAddonMsgToBG(msg);
    }

    // speed change, land walk
    SetRooted(false);

    // remove death flag + set aura
    SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_ANIM_TIER, 0x00);
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_IS_OUT_OF_BOUNDS);
    if(GetRace() == RACE_NIGHTELF)
        RemoveAurasDueToSpell(20584);                       // speed bonuses
    RemoveAurasDueToSpell(8326);                            // SPELL_AURA_GHOST

    SetDeathState(ALIVE);

    m_deathTimer = 0;

    // set health/powers (0- will be set in caller)
    if(restore_percent>0.0f)
    {
        SetHealth(uint32(GetMaxHealth()*restore_percent));
        SetPower(POWER_MANA, uint32(GetMaxPower(POWER_MANA)*restore_percent));
        SetPower(POWER_RAGE, 0);
        SetPower(POWER_ENERGY, uint32(GetMaxPower(POWER_ENERGY)*restore_percent));
    }

    // trigger update zone for alive state zone updates
    uint32 newzone, newarea;
    GetZoneAndAreaId(newzone, newarea);
    UpdateZone(newzone, newarea);
    sOutdoorPvPMgr->HandlePlayerResurrects(this, newzone);

    // update visibility
    UpdateObjectVisibility();

    // some items limited to specific map
    DestroyZoneLimitedItem( true, GetZoneId());

    if(!applySickness)
        return;

    //Characters from level 1-10 are not affected by resurrection sickness.
    //Characters from level 11-19 will suffer from one minute of sickness
    //for each level they are above 10.
    //Characters level 20 and up suffer from ten minutes of sickness.
    int32 startLevel = sWorld->getConfig(CONFIG_DEATH_SICKNESS_LEVEL);

    if(int32(GetLevel()) >= startLevel)
    {
        // set resurrection sickness
        CastSpell(this, SPELL_ID_PASSIVE_RESURRECTION_SICKNESS, true);

        // not full duration
        if(int32(GetLevel()) < startLevel+9)
        {
            int32 delta = (int32(GetLevel()) - startLevel + 1)*MINUTE;
            if(Aura* Aur = GetAura(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS))
                Aur->SetDuration(delta*1000);
        }
    }

    UpdateAreaDependentAuras(GetAreaId());
}

void Player::KillPlayer()
{
    if (IsFlying() && !GetTransport())
        GetMotionMaster()->MoveFall();

    SetRooted(true);

    StopMirrorTimers();                                     //disable timers(bars)

    SetHealth(0); //sun: added this: else we can be in corpse but still have health... this should not be a possible state.
    SetDeathState(CORPSE);

    SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
    ApplyModFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_RELEASE_TIMER, !sMapStore.LookupEntry(GetMapId())->Instanceable());

    // 6 minutes until repop at graveyard
    m_deathTimer = 6 * MINUTE*IN_MILLISECONDS;

    UpdateCorpseReclaimDelay();                             // dependent at use SetDeathPvP() call before kill

    int32 corpseReclaimDelay = CalculateCorpseReclaimDelay();

    if (corpseReclaimDelay >= 0)
        SendCorpseReclaimDelay(corpseReclaimDelay);

    /* Sunwell/Kalecgos: death in spectral realm */
    if (GetMapId() == 580 && GetPositionZ() < -65.f)
        TeleportTo(GetMapId(), GetPositionX(), GetPositionY(), 53.079, GetOrientation());

    // don't create corpse at this moment, player might be falling

    // update visibility
    UpdateObjectVisibility();
}

Corpse* Player::CreateCorpse()
{
    // prevent existence 2 corpse for player
    SpawnCorpseBones();

    uint32 _cfb1, _cfb2;

    auto corpse = new Corpse( (m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH) ? CORPSE_RESURRECTABLE_PVP : CORPSE_RESURRECTABLE_PVE );
    SetPvPDeath(false);

    if(!corpse->Create(GetMap()->GenerateLowGuid<HighGuid::Corpse>(), this))
    {
        delete corpse;
        return nullptr;
    }

    _corpseLocation.WorldRelocate(*this);

    uint8 skin = GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_SKIN_ID);
    uint8 face = GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_FACE_ID);
    uint8 hairstyle = GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_HAIR_STYLE_ID);
    uint8 haircolor = GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_HAIR_COLOR_ID);
    uint8 facialhair = GetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_OFFSET_FACIAL_STYLE);

    _cfb1 = ((0x00) | (GetRace() << 8) | (GetGender() << 16) | (skin << 24));
    _cfb2 = ((face) | (hairstyle << 8) | (haircolor << 16) | (facialhair << 24));

    corpse->SetUInt32Value( CORPSE_FIELD_BYTES_1, _cfb1 );
    corpse->SetUInt32Value( CORPSE_FIELD_BYTES_2, _cfb2 );

    uint32 flags = CORPSE_FLAG_UNK2;
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
        flags |= CORPSE_FLAG_HIDE_HELM;
    if(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK))
        flags |= CORPSE_FLAG_HIDE_CLOAK;
    if(InBattleground() && !InArena())
        flags |= CORPSE_FLAG_LOOTABLE;                      // to be able to remove insignia
    corpse->SetUInt32Value( CORPSE_FIELD_FLAGS, flags );

    corpse->SetUInt32Value( CORPSE_FIELD_DISPLAY_ID, GetNativeDisplayId() );

    corpse->SetUInt32Value( CORPSE_FIELD_GUILD, GetGuildId() );

    uint32 iDisplayID;
    uint16 iIventoryType;
    uint32 _cfi;
    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if(m_items[i])
        {
            iDisplayID = m_items[i]->GetTemplate()->DisplayInfoID;
            iIventoryType = (uint16)m_items[i]->GetTemplate()->InventoryType;

            _cfi =  (uint16(iDisplayID)) | (iIventoryType)<< 24;
            corpse->SetUInt32Value(CORPSE_FIELD_ITEM + i,_cfi);
        }
    }

    // register for player, but not show
    GetMap()->AddCorpse(corpse);

    // we do not need to save corpses for BG/arenas
    if (!GetMap()->IsBattlegroundOrArena())
        corpse->SaveToDB();

    return corpse;
}

void Player::SpawnCorpseBones(bool triggerSave /*= true*/)
{
    _corpseLocation.WorldRelocate();
    if(GetMap()->ConvertCorpseToBones(GetGUID()))
        if (triggerSave && !GetSession()->PlayerLogoutWithSave())          // at logout we will already store the player
            SaveToDB();                                         // prevent loading as ghost without corpse
}

Corpse* Player::GetCorpse() const
{
    return GetMap()->GetCorpseByPlayer(GetGUID());
}

void Player::DurabilityLossAll(double percent, bool inventory)
{
    for(int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        if(Item *pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
            DurabilityLoss(pItem,percent);

    if(inventory)
    {
        // bags not have durability
        // for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)

        for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
            if(Item *pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
                DurabilityLoss(pItem,percent);

        // keys not have durability
        //for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)

        for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            if(Bag* pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
                for(uint32 j = 0; j < pBag->GetBagSize(); j++)
                    if(Item* pItem = GetItemByPos( i, j ))
                        DurabilityLoss(pItem,percent);
    }
}

void Player::DurabilityLoss(Item* item, double percent)
{
    if(!item )
        return;

    uint32 pMaxDurability =  item ->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

    if(!pMaxDurability)
        return;

    uint32 pDurabilityLoss = uint32(pMaxDurability*percent);

    if(pDurabilityLoss < 1 )
        pDurabilityLoss = 1;

    DurabilityPointsLoss(item,pDurabilityLoss);
}

void Player::DurabilityPointsLossAll(int32 points, bool inventory)
{
    for(int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        if(Item *pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
            DurabilityPointsLoss(pItem,points);

    if(inventory)
    {
        // bags not have durability
        // for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)

        for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
            if(Item *pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
                DurabilityPointsLoss(pItem,points);

        // keys not have durability
        //for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)

        for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            if(Bag* pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
                for(uint32 j = 0; j < pBag->GetBagSize(); j++)
                    if(Item* pItem = GetItemByPos( i, j ))
                        DurabilityPointsLoss(pItem,points);
    }
}

void Player::DurabilityPointsLoss(Item* item, int32 points)
{
    int32 pMaxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    int32 pOldDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
    int32 pNewDurability = pOldDurability - points;

    if (pNewDurability < 0)
        pNewDurability = 0;
    else if (pNewDurability > pMaxDurability)
        pNewDurability = pMaxDurability;

    if (pOldDurability != pNewDurability)
    {
        // modify item stats _before_ Durability set to 0 to pass _ApplyItemMods internal check
        if ( pNewDurability == 0 && pOldDurability > 0 && item->IsEquipped())
            _ApplyItemMods(item,item->GetSlot(), false);

        item->SetUInt32Value(ITEM_FIELD_DURABILITY, pNewDurability);

        // modify item stats _after_ restore durability to pass _ApplyItemMods internal check
        if ( pNewDurability > 0 && pOldDurability == 0 && item->IsEquipped())
            _ApplyItemMods(item,item->GetSlot(), true);

        item->SetState(ITEM_CHANGED, this);
    }
}

void Player::DurabilityPointLossForEquipSlot(EquipmentSlots slot)
{
    if(Item *pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, slot ))
        DurabilityPointsLoss(pItem,1);
}

uint32 Player::DurabilityRepairAll(bool cost, float discountMod, bool guildBank)
{
    uint32 TotalCost = 0;
    // equipped, backpack, bags itself
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
        TotalCost += DurabilityRepair(((INVENTORY_SLOT_BAG_0 << 8) | i), cost, discountMod, guildBank);

    // bank, buyback and keys not repaired

    // items in inventory bags
    for(int j = INVENTORY_SLOT_BAG_START; j < INVENTORY_SLOT_BAG_END; j++)
        for(int i = 0; i < MAX_BAG_SIZE; i++)
            TotalCost += DurabilityRepair(((j << 8) | i), cost, discountMod, guildBank);

    return TotalCost;
}

uint32 Player::DurabilityRepair(uint16 pos, bool cost, float discountMod, bool guildBank)
{
    Item* item = GetItemByPos(pos);

    uint32 TotalCost = 0;
    if(!item)
        return TotalCost;

    uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    if(!maxDurability)
        return TotalCost;

    uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

    if(cost)
    {
        uint32 LostDurability = maxDurability - curDurability;
        if(LostDurability>0)
        {
            ItemTemplate const *ditemProto = item->GetTemplate();

            DurabilityCostsEntry const *dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
            if(!dcost)
            {
                TC_LOG_ERROR("entities.player","ERROR: RepairDurability: Wrong item lvl %u", ditemProto->ItemLevel);
                return TotalCost;
            }

            uint32 dQualitymodEntryId = (ditemProto->Quality+1)*2;
            DurabilityQualityEntry const *dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
            if(!dQualitymodEntry)
            {
                TC_LOG_ERROR("entities.player","ERROR: RepairDurability: Wrong dQualityModEntry %u", dQualitymodEntryId);
                return TotalCost;
            }

            uint32 dmultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class,ditemProto->SubClass)];
            uint32 costs = uint32(LostDurability*dmultiplier*double(dQualitymodEntry->quality_mod));

            costs = uint32(costs * discountMod);

            if (costs==0)                                   //fix for ITEM_QUALITY_ARTIFACT
                costs = 1;

            if (guildBank)
            {
                if (GetGuildId() == 0)
                {
                    TC_LOG_DEBUG("entities.player","You are not member of a guild");
                    return TotalCost;
                }

                Guild *guild = sGuildMgr->GetGuildById(GetGuildId());
                if (!guild)
                    return TotalCost;

                if (!guild->HandleMemberWithdrawMoney(GetSession(), costs, true))
                    return TotalCost;

                TotalCost = costs;
            }
            else if (GetMoney() < costs)
            {
                TC_LOG_DEBUG("entities.player","You do not have enough money");
                return TotalCost;
            }
            else
                ModifyMoney( -int32(costs) );
        }
    }

    item->SetUInt32Value(ITEM_FIELD_DURABILITY, maxDurability);
    item->SetState(ITEM_CHANGED, this);

    // reapply mods for total broken and repaired item if equipped
    if(IsEquipmentPos(pos) && !curDurability)
        _ApplyItemMods(item, pos & 255, true);

    return TotalCost;
}

void Player::RepopAtGraveyard()
{
    SetIsRepopPending(false);
    // note: this can be called also when the player is alive
    // for example from WorldSession::HandleMovementOpcodes

    AreaTableEntry const *zone = sAreaTableStore.LookupEntry(GetAreaId());

    bool shouldResurrect = false;
    // Such zones are considered unreachable as a ghost and the player must be automatically revived
    if((!IsAlive() && zone && zone->flags & AREA_FLAG_NEED_FLY) || GetTransport() || GetPositionZ() < GetMap()->GetMinHeight(GetPositionX(), GetPositionY()) || (zone && zone->ID == 2257)) //HACK
    {
        shouldResurrect = true;
        SpawnCorpseBones();
    }

    if (IsInDuelArea())
        return; //stay where we are

    WorldSafeLocsEntry const* ClosestGrave = nullptr;

    // Special handle for battleground maps
    Battleground *bg = GetBattleground();

    if(bg && (bg->GetTypeID() == BATTLEGROUND_AB || bg->GetTypeID() == BATTLEGROUND_EY || bg->GetTypeID() == BATTLEGROUND_AV || bg->GetTypeID() == BATTLEGROUND_WS))
        ClosestGrave = bg->GetClosestGraveYard(GetPositionX(), GetPositionY(), GetPositionZ(), GetTeam());
    else
        ClosestGrave = sObjectMgr->GetClosestGraveYard( GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetTeam() );

    // stop countdown until repop
    m_deathTimer = 0;

    // if no grave found, stay at the current location
    // and don't show spirit healer location
    if(ClosestGrave)
    {
        TeleportTo(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, GetOrientation(), shouldResurrect ? TELE_REVIVE_AT_TELEPORT : 0);
        if(IsDead())                                        // not send if alive, because it used in TeleportTo()
        {
            WorldPacket data(SMSG_DEATH_RELEASE_LOC, 4*4);  // show spirit healer position on minimap
            data << ClosestGrave->map_id;
            data << ClosestGrave->x;
            data << ClosestGrave->y;
            data << ClosestGrave->z;
            SendDirectMessage(&data);
        }
    }
    else if (GetPositionZ() < GetMap()->GetMinHeight(GetPositionX(), GetPositionY()))
        TeleportTo(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ, GetOrientation());

    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_IS_OUT_OF_BOUNDS);
}

void Player::JoinedChannel(Channel *c)
{
    m_channels.push_back(c);
}

void Player::LeftChannel(Channel *c)
{
    m_channels.remove(c);
}

void Player::CleanupChannels()
{
    while(!m_channels.empty())
    {
        Channel* ch = *m_channels.begin();
        m_channels.erase(m_channels.begin());               // remove from player's channel list
        ch->Leave(GetGUID(), false);                 // not send to client, not remove from player's channel list
        if (ChannelMgr* cMgr = channelMgr(GetTeam()))
            cMgr->LeftChannel(ch->GetName());               // deleted channel if empty
    }
}

void Player::UpdateLocalChannels(uint32 newZone )
{
    if (GetSession()->PlayerLoading() && !IsBeingTeleportedFar())
        return;                                              // The client handles it automatically after loading, but not after teleporting

    if(m_channels.empty())
        return;

    AreaTableEntry const* current_area = sAreaTableStore.LookupEntry(newZone);
    if(!current_area)
        return;

    ChannelMgr* cMgr = channelMgr(GetTeam());
    if(!cMgr)
        return;

    std::string current_zone_name = current_area->area_name[GetSession()->GetSessionDbcLocale()];

    for(JoinedChannelsList::iterator i = m_channels.begin(), next; i != m_channels.end(); i = next)
    {
        next = i; ++next;

        // skip non built-in channels
        if(!(*i)->IsConstant())
            continue;

        ChatChannelsEntry const* ch = GetChannelEntryFor((*i)->GetChannelId());
        if(!ch)
            continue;

        if((ch->flags & 4) == 4)                            // global channel without zone name in pattern
            continue;

        //  new channel
        char new_channel_name_buf[100];
        snprintf(new_channel_name_buf,100,ch->pattern[m_session->GetSessionDbcLocale()],current_zone_name.c_str());
        Channel* new_channel = cMgr->GetJoinChannel(new_channel_name_buf,ch->ChannelID);

        if((*i)!=new_channel)
        {
            new_channel->Join(GetGUID(),"");                // will output Changed Channel: N. Name

            // leave old channel
            (*i)->Leave(GetGUID(),false);                   // not send leave channel, it already replaced at client
            std::string name = (*i)->GetName();             // store name, (*i)erase in LeftChannel
            LeftChannel(*i);                                // remove from player's channel list
            cMgr->LeftChannel(name);                        // delete if empty
        }
    }
}

void Player::LeaveLFGChannel()
{
    for(auto & m_channel : m_channels)
    {
        if(m_channel->IsLFG())
        {
            m_channel->Leave(GetGUID());
            break;
        }
    }
}

void Player::UpdateDefense()
{
    uint32 defense_skill_gain = sWorld->getConfig(CONFIG_SKILL_GAIN_DEFENSE);

    if(UpdateSkill(SKILL_DEFENSE,defense_skill_gain))
    {
        // update dependent from defense skill part
        UpdateDefenseBonusesMod();
    }
}

void Player::HandleBaseModFlatValue(BaseModGroup modGroup, float amount, bool apply)
{
    if (modGroup >= BASEMOD_END)
    {
        TC_LOG_ERROR("spells", "Player::HandleBaseModFlatValue: Invalid BaseModGroup/BaseModType (%u/%u) for player '%s' (%s)",
            modGroup, FLAT_MOD, GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str());
        return;
    }

    m_auraBaseFlatMod[modGroup] += apply ? amount : -amount;
    UpdateBaseModGroup(modGroup);
}

void Player::ApplyBaseModPctValue(BaseModGroup modGroup, float pct)
{
    if (modGroup >= BASEMOD_END)
    {
        TC_LOG_ERROR("spells", "Player::ApplyBaseModPctValue: Invalid BaseModGroup/BaseModType (%u/%u) for player '%s' (%s)",
            modGroup, FLAT_MOD, GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str());
        return;
    }

    AddPct(m_auraBasePctMod[modGroup], pct);
    UpdateBaseModGroup(modGroup);
}

void Player::SetBaseModFlatValue(BaseModGroup modGroup, float val)
{
    if (m_auraBaseFlatMod[modGroup] == val)
        return;

    m_auraBaseFlatMod[modGroup] = val;
    UpdateBaseModGroup(modGroup);
}

void Player::SetBaseModPctValue(BaseModGroup modGroup, float val)
{
    if (m_auraBasePctMod[modGroup] == val)
        return;

    m_auraBasePctMod[modGroup] = val;
    UpdateBaseModGroup(modGroup);
}

void Player::UpdateDamageDoneMods(WeaponAttackType attackType)
{
    Unit::UpdateDamageDoneMods(attackType);

    UnitMods unitMod;
    switch (attackType)
    {
    case BASE_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_MAINHAND;
        break;
    case OFF_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_OFFHAND;
        break;
    case RANGED_ATTACK:
        unitMod = UNIT_MOD_DAMAGE_RANGED;
        break;
    default:
        ABORT();
        break;
    }

    float amount = 0.0f;
    Item* item = GetWeaponForAttack(attackType, true);
    if (!item)
        return;

    for (uint8 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
    {
        SpellItemEnchantmentEntry const* enchantmentEntry = sSpellItemEnchantmentStore.LookupEntry(item->GetEnchantmentId(EnchantmentSlot(slot)));
        if (!enchantmentEntry)
            continue;

        for (uint8 i = 0; i < MAX_ITEM_ENCHANTMENT_EFFECTS; ++i)
        {
            switch (enchantmentEntry->type[i])
            {
            case ITEM_ENCHANTMENT_TYPE_DAMAGE:
                amount += enchantmentEntry->amount[i];
                break;
            case ITEM_ENCHANTMENT_TYPE_TOTEM:
                if (GetClass() == CLASS_SHAMAN)
                    amount += enchantmentEntry->amount[i] * item->GetTemplate()->Delay / 1000.0f;
                break;
            default:
                break;
            }
        }
    }

    HandleStatFlatModifier(unitMod, TOTAL_VALUE, amount, true);
}


void Player::UpdateBaseModGroup(BaseModGroup modGroup)
{
    if (!CanModifyStats())
        return;

    switch (modGroup)
    {
    case CRIT_PERCENTAGE:              UpdateCritPercentage(BASE_ATTACK);                          break;
    case RANGED_CRIT_PERCENTAGE:       UpdateCritPercentage(RANGED_ATTACK);                        break;
    case OFFHAND_CRIT_PERCENTAGE:      UpdateCritPercentage(OFF_ATTACK);                           break;
    case SHIELD_BLOCK_VALUE:           UpdateShieldBlockValue();                                   break;
    default: break;
    }
}

float Player::GetBaseModValue(BaseModGroup modGroup, BaseModType modType) const
{
    if (modGroup >= BASEMOD_END || modType >= MOD_END)
    {
        TC_LOG_ERROR("spells", "Player::GetBaseModValue: Invalid BaseModGroup/BaseModType (%u/%u) for player '%s' (%s)",
            modGroup, modType, GetName().c_str(), ObjectGuid( GetGUID()).ToString().c_str());
        return 0.0f;
    }

    return (modType == FLAT_MOD ? m_auraBaseFlatMod[modGroup] : m_auraBasePctMod[modGroup]);
}

float Player::GetTotalBaseModValue(BaseModGroup modGroup) const
{
    if (modGroup >= BASEMOD_END)
    {
        TC_LOG_ERROR("spells", "Player::GetTotalBaseModValue: Invalid BaseModGroup (%u) for player '%s' (%s)",
            modGroup, GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str());
        return 0.0f;
    }

    return m_auraBaseFlatMod[modGroup] * m_auraBasePctMod[modGroup];
}

uint32 Player::GetShieldBlockValue() const
{
    float value = std::max(0.f, (m_auraBaseFlatMod[SHIELD_BLOCK_VALUE] + GetStat(STAT_STRENGTH) * 0.5f - 10) * m_auraBasePctMod[SHIELD_BLOCK_VALUE]);
    return uint32(value);
}

float Player::GetMissPercentageFromDefense() const
{
    float diminishing = 0.0f, nondiminishing = 0.0f;
    // Modify value from defense skill (only bonus from defense rating diminishes)
    nondiminishing += (int32(GetSkillValue(SKILL_DEFENSE)) - int32(GetMaxSkillValueForLevel())) * 0.04f;
    diminishing += (GetRatingBonusValue(CR_DEFENSE_SKILL) * 0.04f);

    // apply diminishing formula to diminishing miss chance
    //return CalculateDiminishingReturns(miss_cap, GetClass(), nondiminishing, diminishing);
    return diminishing;
}

float Player::GetMeleeCritFromAgility()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    GtChanceToMeleeCritBaseEntry const *critBase  = sGtChanceToMeleeCritBaseStore.LookupEntry(pclass-1);
    GtChanceToMeleeCritEntry     const *critRatio = sGtChanceToMeleeCritStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    if (critBase==nullptr || critRatio==nullptr)
        return 0.0f;

    float crit=critBase->base + GetStat(STAT_AGILITY)*critRatio->ratio;
    return crit*100.0f;
}

float Player::GetDodgeFromAgility()
{
    // Table for base dodge values
    float dodge_base[MAX_CLASSES] = {
         0.0075f,   // Warrior
         0.00652f,  // Paladin
        -0.0545f,   // Hunter
        -0.0059f,   // Rogue
         0.03183f,  // Priest
         0.0114f,   // DK
         0.0167f,   // Shaman
         0.034575f, // Mage
         0.02011f,  // Warlock
         0.0f,      // ??
        -0.0187f    // Druid
    };
    // Crit/agility to dodge/agility coefficient multipliers
    float crit_to_dodge[MAX_CLASSES] = {
         1.1f,      // Warrior
         1.0f,      // Paladin
         1.6f,      // Hunter
         2.0f,      // Rogue
         1.0f,      // Priest
         1.0f,      // DK?
         1.0f,      // Shaman
         1.0f,      // Mage
         1.0f,      // Warlock
         0.0f,      // ??
         1.7f       // Druid
    };

    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    // Dodge per agility for most classes equal crit per agility (but for some classes need apply some multiplier)
    GtChanceToMeleeCritEntry  const *dodgeRatio = sGtChanceToMeleeCritStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    if (dodgeRatio==nullptr || pclass > MAX_CLASSES)
        return 0.0f;

    float dodge=dodge_base[pclass-1] + GetStat(STAT_AGILITY) * dodgeRatio->ratio * crit_to_dodge[pclass-1];
    return dodge*100.0f;
}

float Player::GetSpellCritFromIntellect()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    GtChanceToSpellCritBaseEntry const *critBase  = sGtChanceToSpellCritBaseStore.LookupEntry(pclass-1);
    GtChanceToSpellCritEntry     const *critRatio = sGtChanceToSpellCritStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    if (critBase==nullptr || critRatio==nullptr)
        return 0.0f;

    float crit=critBase->base + GetStat(STAT_INTELLECT)*critRatio->ratio;
    return crit*100.0f;
}

//TC has GetRatingMultiplier but it does not work the same way, they use sGtOCTClassCombatRatingScalarStore
float Player::GetRatingCoefficient(CombatRating cr) const
{
    uint32 level = GetLevel();

    if (level > GT_MAX_LEVEL)
        level = GT_MAX_LEVEL;

    GtCombatRatingsEntry const* rating = sGtCombatRatingsStore.LookupEntry(cr*GT_MAX_LEVEL+level-1);
    if (rating == nullptr)
        return 1.0f;                                        // By default use minimum coefficient (not must be called)

    return rating->ratio;
}

float Player::GetRatingBonusValue(CombatRating cr) const
{
#ifdef LICH_KING
    return float(GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + cr)) * GetRatingMultiplier(cr);
#else
    return float(GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + cr)) / GetRatingCoefficient(cr);
#endif
}

uint32 Player::GetMeleeCritDamageReduction(uint32 damage) const
{
    float melee  = GetRatingBonusValue(CR_CRIT_TAKEN_MELEE)*2.0f;
    if (melee>25.0f) melee = 25.0f;
    return uint32 (melee * damage /100.0f);
}

uint32 Player::GetRangedCritDamageReduction(uint32 damage) const
{
    float ranged = GetRatingBonusValue(CR_CRIT_TAKEN_RANGED)*2.0f;
    if (ranged>25.0f) ranged=25.0f;
    return uint32 (ranged * damage /100.0f);
}

uint32 Player::GetSpellCritDamageReduction(uint32 damage) const
{
    float spell = GetRatingBonusValue(CR_CRIT_TAKEN_SPELL)*2.0f;
    // In wow script resilience limited to 25%
    if (spell>25.0f)
        spell = 25.0f;
    return uint32 (spell * damage / 100.0f);
}

uint32 Player::GetDotDamageReduction(uint32 damage) const
{
    float spellDot = GetRatingBonusValue(CR_CRIT_TAKEN_SPELL);
    // Dot resilience not limited (limit it by 100%)
    if (spellDot > 100.0f)
        spellDot = 100.0f;
    return uint32 (spellDot * damage / 100.0f);
}

float Player::GetExpertiseDodgeOrParryReduction(WeaponAttackType attType) const
{
    switch (attType)
    {
        case BASE_ATTACK:
            return GetUInt32Value(PLAYER_EXPERTISE) / 4.0f;
        case OFF_ATTACK:
            return GetUInt32Value(PLAYER_OFFHAND_EXPERTISE) / 4.0f;
        default:
            break;
    }
    return 0.0f;
}

float Player::OCTRegenHPPerSpirit()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    GtOCTRegenHPEntry     const *baseRatio = sGtOCTRegenHPStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    GtRegenHPPerSptEntry  const *moreRatio = sGtRegenHPPerSptStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    if (baseRatio==nullptr || moreRatio==nullptr)
        return 0.0f;

    // Formula from PaperDollFrame script
    float spirit = GetStat(STAT_SPIRIT);
    float baseSpirit = spirit;
    if (baseSpirit>50) baseSpirit = 50;
    float moreSpirit = spirit - baseSpirit;
    float regen = baseSpirit * baseRatio->ratio + moreSpirit * moreRatio->ratio;
    return regen;
}

float Player::OCTRegenMPPerSpirit()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level > GT_MAX_LEVEL) 
        level = GT_MAX_LEVEL;

//    GtOCTRegenMPEntry     const *baseRatio = sGtOCTRegenMPStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    GtRegenMPPerSptEntry const* moreRatio = sGtRegenMPPerSptStore.LookupEntry((pclass - 1) * GT_MAX_LEVEL + level - 1);
    if (moreRatio == nullptr)
        return 0.0f;

    // Formula get from PaperDollFrame script
    float spirit   = GetStat(STAT_SPIRIT);
    float regen    = spirit * moreRatio->ratio;
    return regen;
}

void Player::ApplyRatingMod(CombatRating combatRating, int32 value, bool apply)
{
    float oldRating = m_baseRatingValue[combatRating];
    m_baseRatingValue[combatRating] += (apply ? value : -value);

    // explicit affected values
#ifdef LICH_KING
    float const multiplier = GetRatingMultiplier(combatRating);
    float const oldVal = oldRating * multiplier;
    float const newVal = m_baseRatingValue[combatRating] * multiplier;
#else
    float const coef = GetRatingCoefficient(combatRating);
    float const oldVal = oldRating / coef;
    float const newVal = m_baseRatingValue[combatRating] / coef;
#endif
    switch (combatRating)
    {
        case CR_HASTE_MELEE:
            ApplyAttackTimePercentMod(BASE_ATTACK, oldVal, false);
            ApplyAttackTimePercentMod(OFF_ATTACK, oldVal, false);
            ApplyAttackTimePercentMod(BASE_ATTACK, newVal, true);
            ApplyAttackTimePercentMod(OFF_ATTACK, newVal, true);
            break;
        case CR_HASTE_RANGED:
            ApplyAttackTimePercentMod(RANGED_ATTACK, oldVal, false);
            ApplyAttackTimePercentMod(RANGED_ATTACK, newVal, true);
            break;
        case CR_HASTE_SPELL:
            ApplyCastTimePercentMod(oldVal, false);
            ApplyCastTimePercentMod(newVal, true);
            break;
        default:
            break;
    }

    UpdateRating(combatRating);
}

void Player::UpdateRating(CombatRating cr)
{
    int32 amount = m_baseRatingValue[cr];
    // Apply bonus from SPELL_AURA_MOD_RATING_FROM_STAT
    // stat used stored in miscValueB for this aura
    AuraEffectList const& modRatingFromStat = GetAuraEffectsByType(SPELL_AURA_MOD_RATING_FROM_STAT);
    for (AuraEffect const* aurEff : modRatingFromStat)
        if (aurEff->GetMiscValue() & (1 << cr))
            amount += int32(CalculatePct(GetStat(Stats(aurEff->GetMiscValueB())), aurEff->GetAmount()));

    if (amount < 0)
        amount = 0;
    SetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + cr, uint32(amount));

    bool affectStats = CanModifyStats();

    switch (cr)
    {
    case CR_WEAPON_SKILL:                               // Implemented in Unit::RollMeleeOutcomeAgainst
    case CR_DEFENSE_SKILL:
        UpdateDefenseBonusesMod();
        break;
    case CR_DODGE:
        UpdateDodgePercentage();
        break;
    case CR_PARRY:
        UpdateParryPercentage();
        break;
    case CR_BLOCK:
        UpdateBlockPercentage();
        break;
    case CR_HIT_MELEE:
        UpdateMeleeHitChances();
        break;
    case CR_HIT_RANGED:
        UpdateRangedHitChances();
        break;
    case CR_HIT_SPELL:
        UpdateSpellHitChances();
        break;
    case CR_CRIT_MELEE:
        if (affectStats)
        {
            UpdateCritPercentage(BASE_ATTACK);
            UpdateCritPercentage(OFF_ATTACK);
        }
        break;
    case CR_CRIT_RANGED:
        if (affectStats)
            UpdateCritPercentage(RANGED_ATTACK);
        break;
    case CR_CRIT_SPELL:
        if (affectStats)
            UpdateAllSpellCritChances();
        break;
    case CR_HIT_TAKEN_MELEE:                            // Implemented in Unit::MeleeMissChanceCalc
    case CR_HIT_TAKEN_RANGED:
        break;
    case CR_HIT_TAKEN_SPELL:                            // Implemented in WorldObject::MagicSpellHitResult
        break;
    case CR_CRIT_TAKEN_MELEE:                           // Implemented in Unit::RollMeleeOutcomeAgainst (only for chance to crit)
    case CR_CRIT_TAKEN_RANGED:
        break;
    case CR_CRIT_TAKEN_SPELL:                           // Implemented in Unit::SpellCriticalBonus (only for chance to crit)
        break;
    case CR_HASTE_MELEE:                                // Implemented in Player::ApplyRatingMod
    case CR_HASTE_RANGED:
    case CR_HASTE_SPELL:
        break;
    case CR_WEAPON_SKILL_MAINHAND:                      // Implemented in Unit::RollMeleeOutcomeAgainst
    case CR_WEAPON_SKILL_OFFHAND:
    case CR_WEAPON_SKILL_RANGED:
        break;
    case CR_EXPERTISE:
        if (affectStats)
        {
            UpdateExpertise(BASE_ATTACK);
            UpdateExpertise(OFF_ATTACK);
        }
        break;
#ifdef LICH_KING
    case CR_ARMOR_PENETRATION:
        if (affectStats)
            UpdateArmorPenetration(amount);
        break;
#endif
    }
}

void Player::UpdateAllRatings()
{
    for (uint8 cr = 0; cr < MAX_COMBAT_RATING; ++cr)
        UpdateRating(CombatRating(cr));
}

void Player::SetRegularAttackTime()
{
    for(int i = 0; i < MAX_ATTACK; ++i)
    {
        Item *tmpitem = GetWeaponForAttack(WeaponAttackType(i));
        if(tmpitem && !tmpitem->IsBroken())
        {
            ItemTemplate const *proto = tmpitem->GetTemplate();
            if(proto->Delay)
                SetAttackTime(WeaponAttackType(i), proto->Delay);
            else
                SetAttackTime(WeaponAttackType(i), BASE_ATTACK_TIME);
        }
    }
}

//skill+step, checking for max value
bool Player::UpdateSkill(uint32 skill_id, uint32 step)
{
    if(!skill_id)
        return false;

    if (skill_id == SKILL_FIST_WEAPONS)
        skill_id = SKILL_UNARMED;

    auto itr = mSkillStatus.find(skill_id);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return false;

    uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);
    uint32 data = GetUInt32Value(valueIndex);
    uint32 value = SKILL_VALUE(data);
    uint32 max = SKILL_MAX(data);

    if ((!max) || (!value) || (value >= max))
        return false;

    if (value*512 < max * urand(0,512))
    {
        uint32 new_value = value+step;
        if(new_value > max)
            new_value = max;

        SetUInt32Value(valueIndex,MAKE_SKILL_VALUE(new_value,max));
        if(itr->second.uState != SKILL_NEW)
            itr->second.uState = SKILL_CHANGED;

        return true;
    }

    return false;
}

inline int SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if ( SkillValue >= GrayLevel )
        return sWorld->getConfig(CONFIG_SKILL_CHANCE_GREY)*10;
    if ( SkillValue >= GreenLevel )
        return sWorld->getConfig(CONFIG_SKILL_CHANCE_GREEN)*10;
    if ( SkillValue >= YellowLevel )
        return sWorld->getConfig(CONFIG_SKILL_CHANCE_YELLOW)*10;
    return sWorld->getConfig(CONFIG_SKILL_CHANCE_ORANGE)*10;
}

bool Player::UpdateCraftSkill(uint32 spellid)
{
    SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellid);
    for(auto _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
    {
        if(_spell_idx->second->skillId)
        {
            uint32 SkillValue = GetPureSkillValue(_spell_idx->second->skillId);

            // Alchemy Discoveries here
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(spellid);
            if(spellEntry && spellEntry->Mechanic==MECHANIC_DISCOVERY)
            {
                if(uint32 discoveredSpell = GetSkillDiscoverySpell(_spell_idx->second->skillId, spellid, this))
                    LearnSpell(discoveredSpell, false);
            }

            uint32 craft_skill_gain = sWorld->getConfig(CONFIG_SKILL_GAIN_CRAFTING);

            return UpdateSkillPro(_spell_idx->second->skillId, SkillGainChance(SkillValue,
                _spell_idx->second->max_value,
                (_spell_idx->second->max_value + _spell_idx->second->min_value)/2,
                _spell_idx->second->min_value),
                craft_skill_gain);
        }
    }
    return false;
}

bool Player::UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator )
{
    uint32 gathering_skill_gain = sWorld->getConfig(CONFIG_SKILL_GAIN_GATHERING);

    // For skinning and Mining chance decrease with level. 1-74 - no decrease, 75-149 - 2 times, 225-299 - 8 times
    switch (SkillId)
    {
        case SKILL_HERBALISM:
        case SKILL_LOCKPICKING:
        case SKILL_JEWELCRAFTING:
            return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator,gathering_skill_gain);
        case SKILL_SKINNING:
            if( sWorld->getConfig(CONFIG_SKILL_CHANCE_SKINNING_STEPS)==0)
                return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator,gathering_skill_gain);
            else
                return UpdateSkillPro(SkillId, (SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator) >> (SkillValue/sWorld->getConfig(CONFIG_SKILL_CHANCE_SKINNING_STEPS)), gathering_skill_gain);
        case SKILL_MINING:
            if (sWorld->getConfig(CONFIG_SKILL_CHANCE_MINING_STEPS)==0)
                return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator,gathering_skill_gain);
            else
                return UpdateSkillPro(SkillId, (SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator) >> (SkillValue/sWorld->getConfig(CONFIG_SKILL_CHANCE_MINING_STEPS)),gathering_skill_gain);
    }
    return false;
}

bool Player::UpdateFishingSkill()
{
    uint32 SkillValue = GetPureSkillValue(SKILL_FISHING);

    int32 chance = SkillValue < 75 ? 100 : 2500/(SkillValue-50);

    uint32 gathering_skill_gain = sWorld->getConfig(CONFIG_SKILL_GAIN_GATHERING);

    return UpdateSkillPro(SKILL_FISHING,chance*10,gathering_skill_gain);
}

bool Player::UpdateSkillPro(uint16 SkillId, int32 Chance, uint32 step)
{
    if ( !SkillId )
        return false;

    if(Chance <= 0)                                         // speedup in 0 chance case
        return false;

    auto itr = mSkillStatus.find(SkillId);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return false;

    uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);

    uint32 data = GetUInt32Value(valueIndex);
    uint16 SkillValue = SKILL_VALUE(data);
    uint16 MaxValue   = SKILL_MAX(data);

    if ( !MaxValue || !SkillValue || SkillValue >= MaxValue )
        return false;

    int32 Roll = irand(1,1000);

    if ( Roll <= Chance )
    {
        uint32 new_value = SkillValue+step;
        if(new_value > MaxValue)
            new_value = MaxValue;

        SetUInt32Value(valueIndex,MAKE_SKILL_VALUE(new_value,MaxValue));
        if(itr->second.uState != SKILL_NEW)
            itr->second.uState = SKILL_CHANGED;

        return true;
    }

    return false;
}

void Player::UpdateWeaponSkill(WeaponAttackType attType)
{
    // no skill gain in pvp
    Unit *pVictim = GetVictim();
    if (pVictim && pVictim->IsCharmedOwnedByPlayerOrPlayer())
        return;

    if(IsInFeralForm())
        return;                                             // always maximized SKILL_FERAL_COMBAT in fact

    if(GetShapeshiftForm() == FORM_TREE)
        return;                                             // use weapon but not skill up

    uint32 weapon_skill_gain = sWorld->getConfig(CONFIG_SKILL_GAIN_WEAPON);

    Item* tmpitem = GetWeaponForAttack(attType, true);
    if (!tmpitem && attType == BASE_ATTACK)
    {
        // Keep unarmed & fist weapon skills in sync
        UpdateSkill(SKILL_UNARMED, weapon_skill_gain);
        UpdateSkill(SKILL_FIST_WEAPONS, weapon_skill_gain);
    }
    else if (tmpitem)
    {
        switch (tmpitem->GetTemplate()->SubClass)
        {
        case ITEM_SUBCLASS_WEAPON_FISHING_POLE:
            break;
        case ITEM_SUBCLASS_WEAPON_FIST:
            UpdateSkill(SKILL_UNARMED, weapon_skill_gain);
            [[fallthrough]];
        default:
            UpdateSkill(tmpitem->GetSkill(), weapon_skill_gain);
            break;
        }
    }

    UpdateAllCritPercentages();
}

void Player::UpdateCombatSkills(Unit *pVictim, WeaponAttackType attType, bool defense)
{
    uint32 plevel = GetLevel();                             // if defense than pVictim == attacker
    uint32 greylevel = Trinity::XP::GetGrayLevel(plevel);
    uint32 moblevel = pVictim->GetLevelForTarget(this);
    if(moblevel < greylevel)
        return;

    if (moblevel > plevel + 5)
        moblevel = plevel + 5;

    uint32 lvldif = moblevel - greylevel;
    if(lvldif < 3)
        lvldif = 3;

    uint32 skilldif = 5 * plevel - (defense ? GetBaseDefenseSkillValue() : GetBaseWeaponSkillValue(attType));
    if(skilldif <= 0)
        return;

    float chance = float(3 * lvldif * skilldif) / plevel;
    if(!defense)
        if(GetClass() == CLASS_WARRIOR || GetClass() == CLASS_ROGUE)
            chance += chance * 0.02f * GetStat(STAT_INTELLECT);

    chance = chance < 1.0f ? 1.0f : chance;                 //minimum chance to increase skill is 1%

    if(roll_chance_f(chance))
    {
        if(defense)
            UpdateDefense();
        else
            UpdateWeaponSkill(attType);
    }
    else
        return;
}

void Player::ModifySkillBonus(uint32 skillid,int32 val, bool talent)
{
    SkillStatusMap::const_iterator itr = mSkillStatus.find(skillid);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return;

    uint32 bonusIndex = PLAYER_SKILL_BONUS_INDEX(itr->second.pos);

    uint32 bonus_val = GetUInt32Value(bonusIndex);
    int16 temp_bonus = SKILL_TEMP_BONUS(bonus_val);
    int16 perm_bonus = SKILL_PERM_BONUS(bonus_val);

    if(talent)                                          // permanent bonus stored in high part
        SetUInt32Value(bonusIndex,MAKE_SKILL_BONUS(temp_bonus,perm_bonus+val));
    else
        SetUInt32Value(bonusIndex,MAKE_SKILL_BONUS(temp_bonus+val,perm_bonus));
}

void Player::UpdateSkillsForLevel()
{
    uint16 maxconfskill = sWorld->GetConfigMaxSkillValue();
    uint32 maxSkill = GetMaxSkillValueForLevel();

    bool alwaysMaxSkill = sWorld->getConfig(CONFIG_ALWAYS_MAX_SKILL_FOR_LEVEL);

    for(auto itr = mSkillStatus.begin(); itr != mSkillStatus.end(); ++itr)
    {
        if(itr->second.uState == SKILL_DELETED)
            continue;

        uint32 pskill = itr->first;
        SkillRaceClassInfoEntry const* rcEntry = GetSkillRaceClassInfo(pskill, GetRace(), GetClass());
        if (!rcEntry)
            continue;

        if (GetSkillRangeType(rcEntry) != SKILL_RANGE_LEVEL)
            continue;

        uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);
        uint32 data = GetUInt32Value(valueIndex);
        uint32 max = SKILL_MAX(data);
        uint32 val = SKILL_VALUE(data);

        /// update only level dependent max skill values
        if (max != 1)
        {
            /// maximize skill always
            if (alwaysMaxSkill)
            {
                SetUInt32Value(valueIndex, MAKE_SKILL_VALUE(maxSkill,maxSkill));
                if(itr->second.uState != SKILL_NEW)
                    itr->second.uState = SKILL_CHANGED;
            }
            else if(max != maxconfskill)                    /// update max skill value if current max skill not maximized
            {
                SetUInt32Value(valueIndex, MAKE_SKILL_VALUE(val,maxSkill));
                if(itr->second.uState != SKILL_NEW)
                    itr->second.uState = SKILL_CHANGED;
            }
        }
    }
}

void Player::UpdateSkillsToMaxSkillsForLevel()
{
    for(auto itr = mSkillStatus.begin(); itr != mSkillStatus.end(); ++itr)
    {
        if(itr->second.uState == SKILL_DELETED)
            continue;

        uint32 pskill = itr->first;
        if (SpellMgr::IsProfessionOrRidingSkill(pskill))
            continue;
        uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);
        uint32 data = GetUInt32Value(valueIndex);
        uint32 max = SKILL_MAX(data);

        if (max > 1)
        {
            SetUInt32Value(valueIndex,MAKE_SKILL_VALUE(max,max));
            if(itr->second.uState != SKILL_NEW)
                itr->second.uState = SKILL_CHANGED;
        }
        if (pskill == SKILL_DEFENSE)
            UpdateDefenseBonusesMod();
    }
}

// This functions sets a skill line value (and adds if doesn't exist yet)
// To "remove" a skill line, set it's values to zero
void Player::SetSkill(uint32 id, uint16 step, uint16 newVal, uint16 maxVal)
{
    if(!id)
        return;

    uint16 currVal;
    auto itr = mSkillStatus.find(id);

    // Has skill
    if(itr != mSkillStatus.end() && itr->second.uState != SKILL_DELETED)
    {
        currVal = SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos)));
        if(newVal)
        {
            /* TC
            // if skill value is going down, update enchantments before setting the new value
            if (newVal < currVal)
                UpdateSkillEnchantments(id, currVal, newVal);
            */
            // update step
            SetUInt32Value(PLAYER_SKILL_INDEX(itr->second.pos), MAKE_PAIR32(id, step));
            // update value
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos),MAKE_SKILL_VALUE(newVal,maxVal));
            if(itr->second.uState != SKILL_NEW)
                itr->second.uState = SKILL_CHANGED;
            LearnSkillRewardedSpells(id, newVal);
            /* TC
            // if skill value is going up, update enchantments after setting the new value
            if (newVal > currVal)
            UpdateSkillEnchantments(id, currVal, newVal);
            */
#ifdef LICH_KING
            UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL, id);
            UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL, id);
#endif
        }
        else                                                //remove
        {
            // clear skill fields
            SetUInt32Value(PLAYER_SKILL_INDEX(itr->second.pos),0);
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos),0);
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos),0);

            // mark as deleted or simply remove from map if not saved yet
            if(itr->second.uState != SKILL_NEW)
                itr->second.uState = SKILL_DELETED;
            else
                mSkillStatus.erase(itr);

            // remove spells that depend on this skill when removing the skill
            for (PlayerSpellMap::const_iterator itr_ = m_spells.begin(), next = m_spells.begin(); itr_ != m_spells.end(); itr_ = next)
            {
                ++next;
                if(itr_->second->state == PLAYERSPELL_REMOVED)
                    continue;

                SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(itr_->first);
                for(auto _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
                {
                    if (_spell_idx->second->skillId == id)
                    {
                        // this may remove more than one spell (dependents)
                        RemoveSpell(itr_->first);
                        next = m_spells.begin();
                        break;
                    }
                }
            }
        }
    }
    else if(newVal)                                        //add
    {
        for (int i=0; i < PLAYER_MAX_SKILLS; ++i) {
            if (!GetUInt32Value(PLAYER_SKILL_INDEX(i)))
            {
                SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(id);
                if (!pSkill)
                {
                    TC_LOG_ERROR("entities.player","Skill not found in SkillLineStore: skill #%u", id);
                    return;
                }
                SetUInt32Value(PLAYER_SKILL_INDEX(i), MAKE_PAIR32(id,step));
                SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i),MAKE_SKILL_VALUE(newVal,maxVal));

                // insert new entry or update if not deleted old entry yet
                if(itr != mSkillStatus.end())
                {
                    itr->second.pos = i;
                    itr->second.uState = SKILL_CHANGED;
                }
                else
                    mSkillStatus.insert(SkillStatusMap::value_type(id, SkillStatusData(i, SKILL_NEW)));

                // apply skill bonuses
                SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i),0);

                // temporary bonuses
                AuraEffectList const& mModSkill = GetAuraEffectsByType(SPELL_AURA_MOD_SKILL);
                for (auto j : mModSkill)
                    if (j->GetMiscValue() == int32(id))
                        j->HandleEffect(this, AURA_EFFECT_HANDLE_SKILL, true);

                // permanent bonuses
                AuraEffectList const& mModSkillTalent = GetAuraEffectsByType(SPELL_AURA_MOD_SKILL_TALENT);
                for (auto j : mModSkillTalent)
                    if (j->GetMiscValue() == int32(id))
                        j->HandleEffect(this, AURA_EFFECT_HANDLE_SKILL, true);

                // Learn all spells for skill
                LearnSkillRewardedSpells(id, newVal);
                return;
            }
        }
    }
}

bool Player::HasSkill(uint32 skill) const
{
    if(!skill)
        return false;

    auto itr = mSkillStatus.find(skill);
    return (itr != mSkillStatus.end() && itr->second.uState != SKILL_DELETED);
}

uint16 Player::GetSkillValue(uint32 skill) const
{
    if(!skill)
        return 0;

    auto itr = mSkillStatus.find(skill);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    uint32 bonus = GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos));

    int32 result = int32(SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos))));
    result += SKILL_TEMP_BONUS(bonus);
    result += SKILL_PERM_BONUS(bonus);
    return result < 0 ? 0 : result;
}

uint16 Player::GetMaxSkillValue(uint32 skill) const
{
    if(!skill)
        return 0;

    auto itr = mSkillStatus.find(skill);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    uint32 bonus = GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos));

    int32 result = int32(SKILL_MAX(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos))));
    result += SKILL_TEMP_BONUS(bonus);
    result += SKILL_PERM_BONUS(bonus);
    return result < 0 ? 0 : result;
}

uint16 Player::GetPureMaxSkillValue(uint32 skill) const
{
    if(!skill)
        return 0;

    auto itr = mSkillStatus.find(skill);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_MAX(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos)));
}

uint16 Player::GetBaseSkillValue(uint32 skill) const
{
    if(!skill)
        return 0;

    auto itr = mSkillStatus.find(skill);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    int32 result = int32(SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos))));
    result +=  SKILL_PERM_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos)));
    return result < 0 ? 0 : result;
}

uint16 Player::GetPureSkillValue(uint32 skill) const
{
    if(!skill)
        return 0;

    auto itr = mSkillStatus.find(skill);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos)));
}

int16 Player::GetSkillPermBonusValue(uint32 skill) const
{
    if (!skill)
        return 0;

    auto itr = mSkillStatus.find(skill);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_PERM_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos)));
}

int16 Player::GetSkillTempBonusValue(uint32 skill) const
{
    if (!skill)
        return 0;

    auto itr = mSkillStatus.find(skill);
    if(itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_TEMP_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos)));
}

void Player::SendInitialActionButtons()
{
    TC_LOG_DEBUG("entities.player", "Initializing Action Buttons for '%u'", GetGUID().GetCounter() );

    WorldPacket data(SMSG_ACTION_BUTTONS, (MAX_ACTION_BUTTONS*4));
    for(int button = 0; button < MAX_ACTION_BUTTONS; ++button)
    {
        ActionButtonList::const_iterator itr = m_actionButtons.find(button);
        if(itr != m_actionButtons.end() && itr->second.uState != ACTIONBUTTON_DELETED)
        {
            data << uint16(itr->second.action);
            data << uint8(itr->second.misc);
            data << uint8(itr->second.type);
        }
        else
        {
            data << uint32(0);
        }
    }

    SendDirectMessage( &data );
    TC_LOG_DEBUG("entities.player", "Action Buttons for '%u' Initialized", GetGUID().GetCounter() );
}

void Player::addActionButton(const uint8 button, const uint16 action, const uint8 type, const uint8 misc)
{
    if(button >= MAX_ACTION_BUTTONS)
    {
        TC_LOG_ERROR("entities.player", "Action %u not added into button %u for player %s: button must be < 132", action, button, GetName().c_str() );
        return;
    }

    // check cheating with adding non-known spells to action bar
    if(type==ACTION_BUTTON_SPELL)
    {
        if(!sSpellMgr->GetSpellInfo(action))
        {
            TC_LOG_ERROR("entities.player", "Action %u not added into button %u for player %s: spell not exist", action, button, GetName().c_str() );
            return;
        }

        if(!HasSpell(action))
        {
            TC_LOG_ERROR("entities.player", "Action %u not added into button %u for player %s: player don't known this spell", action, button, GetName().c_str() );
            return;
        }
    }

    auto buttonItr = m_actionButtons.find(button);

    if (buttonItr==m_actionButtons.end())
    {                                                       // just add new button
        m_actionButtons[button] = ActionButton(action,type,misc);
    }
    else
    {                                                       // change state of current button
        ActionButtonUpdateState uState = buttonItr->second.uState;
        buttonItr->second = ActionButton(action,type,misc);
        if (uState != ACTIONBUTTON_NEW) buttonItr->second.uState = ACTIONBUTTON_CHANGED;
    };

    TC_LOG_DEBUG("entities.player", "Player '%u' Added Action '%u' to Button '%u'", GetGUID().GetCounter(), action, button );
}

void Player::removeActionButton(uint8 button)
{
    auto buttonItr = m_actionButtons.find(button);
    if (buttonItr==m_actionButtons.end())
        return;

    if(buttonItr->second.uState==ACTIONBUTTON_NEW)
        m_actionButtons.erase(buttonItr);                   // new and not saved
    else
        buttonItr->second.uState = ACTIONBUTTON_DELETED;    // saved, will deleted at next save

    TC_LOG_DEBUG("entities.player", "Action Button '%u' Removed from Player '%u'", button, GetGUID().GetCounter() );
}

bool Player::UpdatePosition(float x, float y, float z, float orientation, bool teleport)
{
    if (!Unit::UpdatePosition(x, y, z, orientation, teleport))
        return false;

    // Update player zone if needed
    if (m_needsZoneUpdate)
    {
        uint32 newZone, newArea;
        GetZoneAndAreaId(newZone, newArea);
        UpdateZone(newZone, newArea);
        m_needsZoneUpdate = false;
    }

    // group update
    if (GetGroup())
        SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POSITION);

    CheckAreaExploreAndOutdoor();

    return true;
}

void Player::SaveRecallPosition()
{
    m_recall_location.WorldRelocate(*this);
}

void Player::SendMessageToSet(WorldPacket const* data, bool self)
{
    SendMessageToSetInRange(data, GetVisibilityRange(), self);
}

void Player::SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool includeMargin /*= false*/, Player const* skipped_rcvr /*= nullptr*/)
{
    SendMessageToSetInRange(data, dist, self, includeMargin, false, skipped_rcvr);
}

void Player::SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool includeMargin, bool own_team_only, Player const* skipped_rcvr /* = nullptr*/)
{
    if (self)
        GetSession()->SendPacket(data);

    dist += GetCombatReach();
    if(includeMargin)
        dist += VISIBILITY_COMPENSATION; // sunwell: to ensure everyone receives all important packets

    Trinity::MessageDistDeliverer notifier(this, data, dist, own_team_only, skipped_rcvr);
    Cell::VisitWorldObjects(this, notifier, dist);
}

void Player::SendMessageToSet(WorldPacket const* data, Player* skipped_rcvr)
{
    if (skipped_rcvr != this)
        GetSession()->SendPacket(data);

    // we use World::GetMaxVisibleDistance() because i cannot see why not use a distance
    // update: replaced by GetMap()->GetVisibilityRange()
    Trinity::MessageDistDeliverer notifier(this, data, GetVisibilityRange(), false, skipped_rcvr);
    Cell::VisitWorldObjects(this, notifier, GetVisibilityRange());
}

void Player::SendDirectMessage(WorldPacket *data) const
{
    GetSession()->SendPacket(data);
}

void Player::SendCinematicStart(uint32 CinematicSequenceId) const
{
    WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
    data << uint32(CinematicSequenceId);
    SendDirectMessage(&data);
    if (CinematicSequencesEntry const* sequence = sCinematicSequencesStore.LookupEntry(CinematicSequenceId))
        _cinematicMgr->SetActiveCinematicCamera(sequence->cinematicCamera);
}

void Player::SendMovieStart(uint32 MovieId) const
{
#ifdef LICH_KING
    WorldPacket data(SMSG_TRIGGER_MOVIE, 4);
    data << uint32(MovieId);
    SendDirectMessage(&data);
#endif
    //no such packet on BC
}

void Player::CheckAreaExploreAndOutdoor()
{
    if (!IsAlive())
        return;

    if (IsInFlight())
        return;

    if (GetTransport())
        return;

    if (!IsOutdoors())
        RemoveAurasWithAttribute(SPELL_ATTR0_OUTDOORS_ONLY);
    else 
    {
        // Check if we need to reapply outdoor only passive spells
        const PlayerSpellMap& sp_list = GetSpellMap();
        for (const auto & itr : sp_list)
        {
            if (itr.second->state == PLAYERSPELL_REMOVED)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr.first);
            if (!spellInfo || !IsNeedCastSpellAtOutdoor(spellInfo) || HasAuraEffect(itr.first, EFFECT_0))
                continue;

            if (CheckShapeshift(spellInfo, GetShapeshiftForm()) == SPELL_CAST_OK)
                CastSpell(this, itr.first, true);
        }
    }

    uint32 const areaId = GetAreaId();
    AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaId);
    if (!areaEntry)
        return;

    uint32 offset = areaEntry->exploreFlag / 32;

    if (offset >= PLAYER_EXPLORED_ZONES_SIZE)
    {
        TC_LOG_ERROR("entities.player", "Player::CheckAreaExploreAndOutdoor: Wrong zone %u in map data for (X: %f Y: %f) point to field PLAYER_EXPLORED_ZONES_1 + %u ( %u must be < %u ).",
            areaEntry->exploreFlag, GetPositionX(), GetPositionY(), offset, offset, PLAYER_EXPLORED_ZONES_SIZE);
        return;
    }

    uint32 val = (uint32)(1 << (areaEntry->exploreFlag % 32));
    uint32 currFields = GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    if( !(currFields & val) )
    {
        SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

#ifdef LICH_KING
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA);
#endif

        if (areaEntry->area_level > 0)
        {
            if (GetLevel() >= sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                SendExplorationExperience(areaId,0);
            }
            else
            {
                int32 diff = int32(GetLevel()) - areaEntry->area_level;
                uint32 XP = 0;
                if (diff < -5)
                {
                    if (hasCustomXpRate())
                        XP = uint32(sObjectMgr->GetBaseXP(GetLevel()+5)*m_customXp);
                    else
                        XP = uint32(sObjectMgr->GetBaseXP(GetLevel()+5)*sWorld->GetRate(RATE_XP_EXPLORE));
                }
                else if (diff > 5)
                {
                    int32 exploration_percent = (100-((diff-5)*5));
                    if (exploration_percent < 0)
                        exploration_percent = 0;

                    if (hasCustomXpRate())
                        XP = uint32(sObjectMgr->GetBaseXP(areaEntry->area_level)*exploration_percent/100*m_customXp);
                    else
                        XP = uint32(sObjectMgr->GetBaseXP(areaEntry->area_level)*exploration_percent/100*sWorld->GetRate(RATE_XP_EXPLORE));
                }
                else
                {
                    if (hasCustomXpRate())
                        XP = uint32(sObjectMgr->GetBaseXP(areaEntry->area_level)*m_customXp);
                    else
                        XP = uint32(sObjectMgr->GetBaseXP(areaEntry->area_level)*sWorld->GetRate(RATE_XP_EXPLORE));
                }

                GiveXP( XP, nullptr );
                SendExplorationExperience(areaId,XP);
            }
            TC_LOG_DEBUG("entities.player","PLAYER: Player %u discovered a new area: %u", GetGUID().GetCounter(), areaId);
        }
    }
}

uint32 Player::TeamForRace(uint8 race)
{
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    if(!rEntry)
    {
        TC_LOG_ERROR("entities.player","Race %u not found in DBC: wrong DBC files?",uint32(race));
        return ALLIANCE;
    }

    switch(rEntry->TeamID)
    {
        case 7: return ALLIANCE;
        case 1: return HORDE;
    }

    TC_LOG_ERROR("entities.player","Race %u have wrong team id %u in DBC: wrong DBC files?",uint32(race),rEntry->TeamID);
    return ALLIANCE;
}

void Player::SetFactionForRace(uint8 race)
{
    m_team = TeamForRace(race);

    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    SetFaction(rEntry ? rEntry->FactionID : 0);
}

int32 Player::GetReputation(uint32 factionentry) const
{
    return GetReputationMgr().GetReputation(sFactionStore.LookupEntry(factionentry));
}

ReputationRank Player::GetReputationRank(uint32 faction) const
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction);
    return GetReputationMgr().GetRank(factionEntry);
}

//Calculate total reputation percent player gain with quest/creature level
int32 Player::CalculateReputationGain(ReputationSource source, uint32 creatureOrQuestLevel, int32 rep, int32 faction, bool noQuestBonus)
{
    float percent = 100.0f;

    float repMod = noQuestBonus ? 0.0f : float(GetTotalAuraModifier(SPELL_AURA_MOD_REPUTATION_GAIN));

    // faction specific auras only seem to apply to kills
    if (source == REPUTATION_SOURCE_KILL)
        repMod += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_FACTION_REPUTATION_GAIN, faction);

    percent += rep > 0 ? repMod : -repMod;

    float rate;
    switch (source)
    {
    case REPUTATION_SOURCE_KILL:
        rate = 0.2f; //sWorld->getRate(RATE_REPUTATION_LOWLEVEL_KILL);
        break;
    case REPUTATION_SOURCE_QUEST:
    case REPUTATION_SOURCE_DAILY_QUEST:
    case REPUTATION_SOURCE_WEEKLY_QUEST:
    case REPUTATION_SOURCE_MONTHLY_QUEST:
    case REPUTATION_SOURCE_REPEATABLE_QUEST:
        rate = 0.2f; // sWorld->getRate(RATE_REPUTATION_LOWLEVEL_QUEST);
        break;
    case REPUTATION_SOURCE_SPELL:
    default:
        rate = 1.0f;
        break;
    }

    if (rate != 1.0f && creatureOrQuestLevel <= Trinity::XP::GetGrayLevel(GetLevel()))
        percent *= rate;

    if (percent <= 0.0f)
        return 0;

    /* TC
    // Multiply result with the faction specific rate
    if (RepRewardRate const* repData = sObjectMgr->GetRepRewardRate(faction))
    {
        float repRate = 0.0f;
        switch (source)
        {
        case REPUTATION_SOURCE_KILL:
            repRate = repData->creatureRate;
            break;
        case REPUTATION_SOURCE_QUEST:
            repRate = repData->questRate;
            break;
        case REPUTATION_SOURCE_DAILY_QUEST:
            repRate = repData->questDailyRate;
            break;
        case REPUTATION_SOURCE_WEEKLY_QUEST:
            repRate = repData->questWeeklyRate;
            break;
        case REPUTATION_SOURCE_MONTHLY_QUEST:
            repRate = repData->questMonthlyRate;
            break;
        case REPUTATION_SOURCE_REPEATABLE_QUEST:
            repRate = repData->questRepeatableRate;
            break;
        case REPUTATION_SOURCE_SPELL:
            repRate = repData->spellRate;
            break;
        }

        // for custom, a rate of 0.0 will totally disable reputation gain for this faction/type
        if (repRate <= 0.0f)
            return 0;

        percent *= repRate;
    }
    */
    percent *= sWorld->GetRate(RATE_REPUTATION_GAIN);

#ifdef LICH_KING
    if (source != REPUTATION_SOURCE_SPELL && GetsRecruitAFriendBonus(false))
        percent *= 1.0f + sWorld->getRate(RATE_REPUTATION_RECRUIT_A_FRIEND_BONUS);
#endif

    return CalculatePct(rep, percent);
}

//Calculates how many reputation points player gains in victim's enemy factions
void Player::RewardReputation(Unit* victim, float rate)
{
    if (!victim || victim->GetTypeId() == TYPEID_PLAYER)
        return;

    if (victim->ToCreature()->IsReputationGainDisabled())
        return;

    ReputationOnKillEntry const* Rep = sObjectMgr->GetReputationOnKilEntry(victim->ToCreature()->GetCreatureTemplate()->Entry);
    if (!Rep)
        return;

    uint32 ChampioningFaction = 0;
#ifdef LICH_KING
    if (GetChampioningFaction())
    {
        // support for: Championing - http://www.wowwiki.com/Championing
        Map const* map = GetMap();
        if (map->IsNonRaidDungeon())
            if (LFGDungeonEntry const* dungeon = GetLFGDungeon(map->GetId(), map->GetDifficulty()))
                if (dungeon->reclevel == 80)
                    ChampioningFaction = GetChampioningFaction();
    }
#endif

    Unit::AuraEffectList const& DummyAuras = GetAuraEffectsByType(SPELL_AURA_DUMMY);
    for (auto DummyAura : DummyAuras)
        if (DummyAura->GetId() == 32098 || DummyAura->GetId() == 32096)
        {
            uint32 area_id = GetAreaId();
            if (area_id == 3483 || area_id == 3535 || area_id == 3562 || area_id == 3713)
                rate *= 1.25f;
        }

    uint32 team = GetTeam();

    if (Rep->RepFaction1 && (!Rep->TeamDependent || team == ALLIANCE))
    {
        int32 donerep1 = CalculateReputationGain(REPUTATION_SOURCE_KILL, victim->GetLevel(), Rep->RepValue1, ChampioningFaction ? ChampioningFaction : Rep->RepFaction1);
        donerep1 = int32(donerep1 * rate);

        FactionEntry const* factionEntry1 = sFactionStore.LookupEntry(ChampioningFaction ? ChampioningFaction : Rep->RepFaction1);
        uint32 current_reputation_rank1 = GetReputationMgr().GetRank(factionEntry1);
        if (factionEntry1)
            GetReputationMgr().ModifyReputation(factionEntry1, donerep1, current_reputation_rank1 > Rep->ReputationMaxCap1);
    }

    if (Rep->RepFaction2 && (!Rep->TeamDependent || team == HORDE))
    {
        int32 donerep2 = CalculateReputationGain(REPUTATION_SOURCE_KILL, victim->GetLevel(), Rep->RepValue2, ChampioningFaction ? ChampioningFaction : Rep->RepFaction2);
        donerep2 = int32(donerep2 * rate);

        FactionEntry const* factionEntry2 = sFactionStore.LookupEntry(ChampioningFaction ? ChampioningFaction : Rep->RepFaction2);
        uint32 current_reputation_rank2 = GetReputationMgr().GetRank(factionEntry2);
        if (factionEntry2)
            GetReputationMgr().ModifyReputation(factionEntry2, donerep2, current_reputation_rank2 > Rep->ReputationMaxCap2);
    }
}

//Calculate how many reputation points player gain with the quest
void Player::RewardReputation(Quest const* quest)
{
    for (uint8 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        if (!quest->RewardFactionId[i])
            continue;

        int32 rep = 0;
        bool noQuestBonus = false;

#ifdef LICH_KING
        if (quest->RewardFactionValueIdOverride[i])
        {
            rep = quest->RewardFactionValueIdOverride[i] / 100;
            noQuestBonus = true;
        }
        else
        {
            uint32 row = ((quest->RewardFactionValueId[i] < 0) ? 1 : 0) + 1;
            if (QuestFactionRewEntry const* questFactionRewEntry = sQuestFactionRewardStore.LookupEntry(row))
            {
                uint32 field = abs(quest->RewardFactionValueId[i]);
                rep = questFactionRewEntry->QuestRewFactionValue[field];
            }
        }
#else
        rep = quest->RewardRepValue[i];
#endif

        if (!rep)
            continue;

        if (quest->IsDaily())
            rep = CalculateReputationGain(REPUTATION_SOURCE_DAILY_QUEST, GetQuestLevel(quest), rep, quest->RewardFactionId[i], noQuestBonus);
        else if (quest->IsWeekly())
            rep = CalculateReputationGain(REPUTATION_SOURCE_WEEKLY_QUEST, GetQuestLevel(quest), rep, quest->RewardFactionId[i], noQuestBonus);
        else if (quest->IsMonthly())
            rep = CalculateReputationGain(REPUTATION_SOURCE_MONTHLY_QUEST, GetQuestLevel(quest), rep, quest->RewardFactionId[i], noQuestBonus);
        else if (quest->IsRepeatable())
            rep = CalculateReputationGain(REPUTATION_SOURCE_REPEATABLE_QUEST, GetQuestLevel(quest), rep, quest->RewardFactionId[i], noQuestBonus);
        else
            rep = CalculateReputationGain(REPUTATION_SOURCE_QUEST, GetQuestLevel(quest), rep, quest->RewardFactionId[i], noQuestBonus);

        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(quest->RewardFactionId[i]))
            GetReputationMgr().ModifyReputation(factionEntry, rep);
    }
}

void Player::UpdateArenaFields()
{
    /* arena calcs go here */
}

void Player::UpdateHonorFields()
{
    /// called when rewarding honor and at each save
    uint64 now = WorldGameTime::GetGameTime();
    uint64 today = uint64(WorldGameTime::GetGameTime() / DAY) * DAY;

    if(m_lastHonorUpdateTime < today)
    {
        uint64 yesterday = today - DAY;

        uint16 kills_today = PAIR32_LOPART(GetUInt32Value(PLAYER_FIELD_KILLS));

        // update yesterday's contribution
        if(m_lastHonorUpdateTime >= yesterday )
        {
            SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));

            // this is the first update today, reset today's contribution
            SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
            SetUInt32Value(PLAYER_FIELD_KILLS, MAKE_PAIR32(0,kills_today));
        }
        else
        {
            // no honor/kills yesterday or today, reset
            SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);
            SetUInt32Value(PLAYER_FIELD_KILLS, 0);
        }
    }

    m_lastHonorUpdateTime = now;
}

///Calculate the amount of honor gained based on the victim
///and the size of the group for which the honor is divided
///An exact honor value can also be given (overriding the calcs)
bool Player::RewardHonor(Unit *uVictim, uint32 groupsize, float honor, bool pvptoken)
{
    // do not reward honor in arenas, but enable onkill spellproc
    if(InArena())
    {
        if(!uVictim || uVictim == this || uVictim->GetTypeId() != TYPEID_PLAYER)
            return false;

        if( GetBGTeam() == (uVictim->ToPlayer())->GetBGTeam() )
            return false;

        return true;
    }

    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    if(GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return false;

    ObjectGuid victim_guid;
    uint32 victim_rank = 0;

    // need call before fields update to have chance move yesterday data to appropriate fields before today data change.
    UpdateHonorFields();

    // do not reward honor in arenas, but return true to enable onkill spellproc
    if(InBattleground() && GetBattleground() && GetBattleground()->IsArena())
        return true;

    if(honor <= 0)
    {
        if(!uVictim || uVictim == this || uVictim->HasAuraType(SPELL_AURA_NO_PVP_CREDIT))
            return false;

        victim_guid = uVictim->GetGUID();

        if( uVictim->GetTypeId() == TYPEID_PLAYER )
        {
            Player *pVictim = uVictim->ToPlayer();

            if( GetTeam() == pVictim->GetTeam() && !sWorld->IsFFAPvPRealm() )
                return false;

            float f = 1;                                    //need for total kills (?? need more info)
            uint32 k_grey = 0;
            uint32 k_level = GetLevel();
            uint32 v_level = pVictim->GetLevel();

            {
                // PLAYER_CHOSEN_TITLE VALUES DESCRIPTION
                //  [0]      Just name
                //  [1..14]  Alliance honor titles and player name
                //  [15..28] Horde honor titles and player name
                //  [29..38] Other title and player name
                //  [39+]    Nothing
                uint32 victim_title = pVictim->GetUInt32Value(PLAYER_CHOSEN_TITLE);
                                                            // Get Killer titles, CharTitlesEntry::bit_index

                // Ranks:
                //  title[1..14]  -> rank[5..18]
                //  title[15..28] -> rank[5..18]
                //  title[other]  -> 0
                if (victim_title == 0)
                    victim_guid = ObjectGuid::Empty;                        // Don't show HK: <rank> message, only log.
                else if (victim_title < HKRANKMAX)
                    victim_rank = victim_title + 4;
                else if (victim_title < (2*HKRANKMAX-1))
                    victim_rank = victim_title - (HKRANKMAX-1) + 4;
                else
                    victim_guid = ObjectGuid::Empty;                        // Don't show HK: <rank> message, only log.
            }

            if(k_level <= 5)
                k_grey = 0;
            else if( k_level <= 39 )
                k_grey = k_level - 5 - k_level/10;
            else
                k_grey = k_level - 1 - k_level/5;

            if(v_level<=k_grey)
                return false;

            float diff_level = (k_level == k_grey) ? 1 : ((float(v_level) - float(k_grey)) / (float(k_level) - float(k_grey)));

            int32 v_rank =1;                                //need more info

            honor = ((f * diff_level * (190 + v_rank*10))/6);
            honor *= ((float)k_level) / 70.0f;              //factor of dependence on levels of the killer

            // count the number of playerkills in one day
            ApplyModUInt32Value(PLAYER_FIELD_KILLS, 1, true);
            // and those in a lifetime
            ApplyModUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 1, true);

            UpdateKnownPvPTitles();
        }
        else
        {
            Creature *cVictim = uVictim->ToCreature();

            if (!cVictim->isRacialLeader())
                return false;

            honor = 100;                                    // ??? need more info
            victim_rank = 19;                               // HK: Leader
        }
    }

    if (uVictim != nullptr)
    {
        honor *= sWorld->GetRate(RATE_HONOR);

        if(groupsize > 1)
            honor /= groupsize;

        honor *= (((float)urand(8,12))/10);                 // approx honor: 80% - 120% of real honor
    }

    // honor - for show honor points in log
    // victim_guid - for show victim name in log
    // victim_rank [1..4]  HK: <dishonored rank>
    // victim_rank [5..19] HK: <alliance\horde rank>
    // victim_rank [0,20+] HK: <>
    WorldPacket data(SMSG_PVP_CREDIT,4+8+4);
    data << (uint32) honor;
    data << (uint64) victim_guid;
    data << (uint32) victim_rank;

    SendDirectMessage(&data);

    // add honor points
    ModifyHonorPoints(int32(honor));

    ApplyModUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, uint32(honor), true);

    if( sWorld->getConfig(CONFIG_PVP_TOKEN_ENABLE) && pvptoken )
    {
        if(!uVictim || uVictim == this || uVictim->HasAuraType(SPELL_AURA_NO_PVP_CREDIT))
            return true;

        if(uVictim->GetTypeId() == TYPEID_PLAYER)
        {
            // Check if allowed to receive it in current map
            uint8 MapType = sWorld->getConfig(CONFIG_PVP_TOKEN_MAP_TYPE);
            if( (MapType == 1 && !InBattleground() && !HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP))
                || (MapType == 2 && !HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP))
                || (MapType == 3 && !InBattleground()) )
                return true;

            uint32 ZoneId = sWorld->getConfig(CONFIG_PVP_TOKEN_ZONE_ID);
            if (ZoneId && m_zoneUpdateId != ZoneId)
                return true;

            uint32 noSpaceForCount = 0;
            uint32 itemId = sWorld->getConfig(CONFIG_PVP_TOKEN_ID);
            int32 count = sWorld->getConfig(CONFIG_PVP_TOKEN_COUNT);

            // check space and find places
            ItemPosCountVec dest;
            uint8 msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
            if( msg != EQUIP_ERR_OK )   // convert to possible store amount
                count = noSpaceForCount;

            if( count == 0 || dest.empty()) // can't add any
            {
                // -- TODO: Send to mailbox if no space
                ChatHandler(this).PSendSysMessage("You don't have any space in your bags for a token.");
                return true;
            }

            Item* item = StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
            SendNewItem(item,count,true,false);
            ChatHandler(this).PSendSysMessage("You have been awarded a token for slaying another player.");
        }
    }

    return true;
}

void Player::SetHonorPoints(uint32 value)
{
    if (value > sWorld->getIntConfig(CONFIG_MAX_HONOR_POINTS))
        value = sWorld->getIntConfig(CONFIG_MAX_HONOR_POINTS);
    SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, value);
#ifdef LICH_KING
    if (value)
        AddKnownCurrency(ITEM_HONOR_POINTS_ID);
#endif
}

void Player::SetArenaPoints(uint32 value)
{
    if (value > sWorld->getIntConfig(CONFIG_MAX_ARENA_POINTS))
        value = sWorld->getIntConfig(CONFIG_MAX_ARENA_POINTS);
    SetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, value);
#ifdef LICH_KING
    if (value)
        AddKnownCurrency(ITEM_ARENA_POINTS_ID);
#endif
}

void Player::ModifyHonorPoints(int32 value, SQLTransaction trans)
{
    int32 newValue = int32(GetHonorPoints()) + value;
    if (newValue < 0)
        newValue = 0;

    SetHonorPoints(uint32(newValue));

    if (trans)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_HONOR_POINTS);
        stmt->setUInt32(0, newValue);
        stmt->setUInt32(1, GetGUID().GetCounter());
        trans->Append(stmt);
    }
}

void Player::ModifyArenaPoints(int32 value, SQLTransaction trans)
{
    int32 newValue = int32(GetArenaPoints()) + value;
    if (newValue < 0)
        newValue = 0;
    SetArenaPoints(uint32(newValue));

    if (trans)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_ARENA_POINTS);
        stmt->setUInt32(0, newValue);
        stmt->setUInt32(1, GetGUID().GetCounter());
        trans->Append(stmt);
    }
}

Guild* Player::GetGuild() const
{
    uint32 guildId = GetGuildId();
    return guildId ? sGuildMgr->GetGuildById(guildId) : nullptr;
}

uint32 Player::GetZoneIdFromDB(ObjectGuid guid)
{
    std::ostringstream ss;

    ss << "SELECT zone FROM characters WHERE guid='" << guid.GetCounter() << "'";
    QueryResult result = CharacterDatabase.Query( ss.str().c_str() );
    if (!result)
        return 0;
    Field* fields = result->Fetch();
    uint32 zone = fields[0].GetUInt32();

    if (!zone)
    {
        // stored zone is zero, use generic and slow zone detection
        ss.str("");
        ss<<"SELECT map,position_x,position_y,position_z FROM characters WHERE guid='"<<guid.GetCounter()<<"'";
        result = CharacterDatabase.Query(ss.str().c_str());
        if( !result )
            return 0;
        fields = result->Fetch();
        uint32 map  = fields[0].GetUInt32();
        float posx = fields[1].GetFloat();
        float posy = fields[2].GetFloat();
        float posz = fields[3].GetFloat();

        zone = sMapMgr->GetZoneId(map,posx,posy,posz);

        ss.str("");
        ss << "UPDATE characters SET zone='"<<zone<<"' WHERE guid='"<<guid.GetCounter()<<"'";
        CharacterDatabase.Execute(ss.str().c_str());
    }

    return zone;
}

uint32 Player::GetLevelFromStorage(ObjectGuid guid)
{
    // Get data from global storage
    if (CharacterCacheEntry const* playerData = sCharacterCache->GetCharacterCacheByGuid(guid.GetCounter()))
        return playerData->level;

    return 0;
}

void Player::UpdatePvPState(bool onlyFFA)
{
    if (!pvpInfo.IsInNoPvPArea && !IsGameMaster()
        && (pvpInfo.IsInFFAPvPArea || sWorld->IsFFAPvPRealm()))
    {
        if (!IsFFAPvP())
        {
#ifdef LICH_KING
            SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
            for (ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
                (*itr)->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
#else

            SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
            //ON BC controlled creatures FFA is handled by the client depending on owner?
#endif
        }
    }
    else if (IsFFAPvP())
    {
#ifdef LICH_KING
        RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
        for (ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
            (*itr)->RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
#else
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
        //ON BC controlled creatures FFA is handled by the client depending on owner?
#endif
    }

    if (onlyFFA)
        return;

    if (pvpInfo.IsHostile)                               // in hostile area
    {
        if (!IsPvP() || pvpInfo.endTimer)
            UpdatePvP(true, true);
    }
    else                                                    // in friendly area
    {
        if (IsPvP() && !HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP) && !pvpInfo.endTimer)
            pvpInfo.endTimer = WorldGameTime::GetGameTime();                  // start toggle-off
    }
}

void Player::UpdateArea(uint32 newArea)
{
    // FFA_PVP flags are area and not zone id dependent
    // so apply them accordingly
    m_areaUpdateId    = newArea;

    AreaTableEntry const* area = sAreaTableStore.LookupEntry(newArea);

    bool oldFFAPvPArea = pvpInfo.IsInFFAPvPArea;
    pvpInfo.IsInFFAPvPArea = (area && ((area->flags & AREA_FLAG_ARENA) || (sWorld->IsZoneFFA(area->ID)) || (area->ID == 3775))); //last one is hack
    UpdatePvPState(true);

    // check if we were in ffa arena and we left
    if (oldFFAPvPArea && !pvpInfo.IsInFFAPvPArea)
        ValidateAttackersAndOwnTarget();

    UpdateAreaDependentAuras(newArea);

    pvpInfo.IsInNoPvPArea = false;
    if (area && area->IsSanctuary())    // in sanctuary
    {
#ifdef LICH_KING
        SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_SANCTUARY);
#else
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY);
#endif
        pvpInfo.IsInNoPvPArea = true;
        if (!duel)
            CombatStopWithPets();
    }
    else
#ifdef LICH_KING
        RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_SANCTUARY);
#else
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY);
#endif

}

void Player::UpdateZone(uint32 newZone, uint32 newArea)
{
    if (!IsInWorld())
        return;

    if (sWorld->getConfig(CONFIG_ARENASERVER_ENABLED) //bring back the escapers !
        && newZone != 616  //Hyjal arena zone
        && newZone != 406 // zone pvp
        && GetAreaId() != 19 //Zul Gurub arena zone
        && !InBattleground()
        && !IsBeingTeleported()
        && !IsGameMaster())
    {
        TeleportToArenaZone(ShouldGoToSecondaryArenaZone());
        return;
    }

    uint32 const oldZoneId = m_zoneUpdateId;
    m_zoneUpdateId = newZone;
    m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL;

    GetMap()->UpdatePlayerZoneStats(oldZoneId, newZone);

    // inform outdoor pvp
    if (oldZoneId != m_zoneUpdateId)
    {
        sOutdoorPvPMgr->HandlePlayerLeaveZone(this, oldZoneId);
#ifdef LICH_KING
        sBattlefieldMgr->HandlePlayerLeaveZone(this, oldZoneId);
        sBattlefieldMgr->HandlePlayerEnterZone(this, newZone);
#endif
    }

    // zone changed, so area changed as well, update it
    UpdateArea(GetAreaId());

    AreaTableEntry const* zone = sAreaTableStore.LookupEntry(newZone);
    if (!zone)
        return;


    if (sWorld->getConfig(CONFIG_WEATHER))
    {
        if(Weather *wth = sWorld->FindWeather(zone->ID))
            wth->SendWeatherUpdateToPlayer(this);
        else if (!sWorld->AddWeather(zone->ID))
            // send fine weather packet to remove old zone's weather
            Weather::SendFineWeatherUpdateToPlayer(this);
    }

    // in PvP, any not controlled zone (except zone->team == 6, default case)
    // in PvE, only opposition team capital
    switch (zone->team)
    {
    case AREATEAM_ALLY:
        pvpInfo.IsInHostileArea = GetTeam() != ALLIANCE && (sWorld->IsPvPRealm() || zone->flags & AREA_FLAG_CAPITAL);
        break;
    case AREATEAM_HORDE:
        pvpInfo.IsInHostileArea = GetTeam() != HORDE && (sWorld->IsPvPRealm() || zone->flags & AREA_FLAG_CAPITAL);
        break;
    case AREATEAM_NONE:
        // overwrite for battlegrounds, maybe batter some zone flags but current known not 100% fit to this
        pvpInfo.IsInHostileArea = sWorld->IsPvPRealm() || InBattleground() 
#ifdef LICH_KING
            || zone->flags & AREA_FLAG_WINTERGRASP
#endif
            ;
        break;
    default:                                            // 6 in fact
        pvpInfo.IsInHostileArea = false;
        break;
    }

    // Treat players having a quest flagging for PvP as always in hostile area
    pvpInfo.IsHostile = pvpInfo.IsInHostileArea
#ifdef LICH_KING
        || HasPvPForcingQuest()
#endif
        ;

    if (zone->flags & AREA_FLAG_CAPITAL)                     // in capital city
    {
        if (!pvpInfo.IsHostile || zone->IsSanctuary())
            SetRestType(REST_TYPE_IN_CITY);

        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        InnEnter(WorldGameTime::GetGameTime(), GetMapId(), 0, 0, 0);

        pvpInfo.IsInNoPvPArea = true;
    }
    else                                                    // anywhere else
    {
        if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))     // but resting (walk from city or maybe in tavern or leave tavern recently)
        {
            if (GetRestType() == REST_TYPE_IN_TAVERN)          // has been in tavern. Is still in?
            {
                if (GetMapId() != GetInnPosMapId() || sqrt((GetPositionX() - GetInnPosX())*(GetPositionX() - GetInnPosX()) + (GetPositionY() - GetInnPosY())*(GetPositionY() - GetInnPosY()) + (GetPositionZ() - GetInnPosZ())*(GetPositionZ() - GetInnPosZ())) > 40)
                {
                    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
                    SetRestType(REST_TYPE_NO);
                }
            }
            else   // not in tavern (leave city then)
            {
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
                SetRestType(REST_TYPE_NO);
            }
        }
    }

    UpdatePvPState();

    // remove items with area/map limitations (delete only for alive player to allow back in ghost mode)
    // if player resurrected at teleport this will be applied in resurrect code
    if (IsAlive())
        DestroyZoneLimitedItem(true, newZone);

    // check some item equip limitations (in result lost CanTitanGrip at talent reset, for example)
    AutoUnequipOffhandIfNeed();

    // recent client version not send leave/join channel packets for built-in local channels
    UpdateLocalChannels(newZone);

    // group update
    if (GetGroup())
        SetGroupUpdateFlag(GROUP_UPDATE_FLAG_ZONE);

    UpdateZoneDependentAuras(newZone);
    sScriptMgr->OnPlayerUpdateZone(this, newZone, newArea);

    if (oldZoneId != newZone)
    {
        sOutdoorPvPMgr->HandlePlayerEnterZone(this, m_zoneUpdateId);
#ifdef LICH_KING
        sBattlefieldMgr->HandlePlayerEnterZone(this, newZone);
#endif
        SendInitWorldStates(newZone, newArea);              // only if really enters to new zone, not just area change, works strange...
        if (Guild* guild = GetGuild())
            guild->UpdateMemberData(this, GUILD_MEMBER_DATA_ZONEID, newZone);
    }
}

//If players are too far way of duel flag... then player loose the duel
void Player::CheckDuelDistance(time_t currTime)
{
    if(!duel)
        return;

    ObjectGuid duelFlagGUID = GetGuidValue(PLAYER_DUEL_ARBITER);
    GameObject* obj = ObjectAccessor::GetGameObject(*this, duelFlagGUID);
    if(!obj)
        return;

    if (!duel->OutOfBoundsTime)
    {
        if(!IsWithinDistInMap(obj, 65))
        {
            duel->OutOfBoundsTime = currTime + 10;

            WorldPacket data(SMSG_DUEL_OUTOFBOUNDS, 0);
            SendDirectMessage(&data);
        }
    }
    else
    {
        if(IsWithinDistInMap(obj, 65))
        {
            duel->OutOfBoundsTime = 0;

            WorldPacket data(SMSG_DUEL_INBOUNDS, 0);
            SendDirectMessage(&data);
        }
        else if(currTime >= (duel->OutOfBoundsTime))
        {
            DuelComplete(DUEL_FLED);
        }
    }
}

bool Player::IsOutdoorPvPActive()
{
    return (IsAlive() && !HasInvisibilityAura() && !HasStealthAura() && (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP) || sWorld->IsPvPRealm())  && !HasUnitMovementFlag(MOVEMENTFLAG_PLAYER_FLYING) && !IsInFlight());
}

void Player::DuelComplete(DuelCompleteType type)
{
    // duel not requested
    if(!duel)
        return;

    // Check if DuelComplete() has been called already up in the stack and in that case don't do anything else here
    if (duel->State == DUEL_STATE_COMPLETED)
        return;

    Player* opponent = duel->Opponent;
    duel->State = DUEL_STATE_COMPLETED;
    opponent->duel->State = DUEL_STATE_COMPLETED;

    TC_LOG_DEBUG("entities.unit", "Player::DuelComplete: Player '%s' (%s), Opponent: '%s' (%s)",
        GetName().c_str(), GetGUID().ToString().c_str(), opponent->GetName().c_str(), opponent->GetGUID().ToString().c_str());

    WorldPacket data(SMSG_DUEL_COMPLETE, (1));
    data << uint8((type != DUEL_INTERRUPTED) ? 1 : 0);
    SendDirectMessage(&data);
    if (opponent->GetSession())
        opponent->SendDirectMessage(&data);

    if(type != DUEL_INTERRUPTED)
    {
        data.Initialize(SMSG_DUEL_WINNER, (1+20));          // we guess size
        data << (uint8)((type==DUEL_WON) ? 0 : 1);          // 0 = just won; 1 = fled
        data << opponent->GetName();
        data << GetName();
        SendMessageToSet(&data,true);
    }

    sScriptMgr->OnPlayerDuelEnd(opponent, this, type);

    switch (type)
    {
    case DUEL_FLED:
        // if initiator and opponent are on the same team
        // or initiator and opponent are not PvP enabled, forcibly stop attacking
        if (GetTeam() == opponent->GetTeam())
        {
            AttackStop();
            opponent->AttackStop();
        }
        else
        {
            if (!IsPvP())
                AttackStop();
            if (!opponent->IsPvP())
                opponent->AttackStop();
        }
        break;
    case DUEL_WON:
#ifdef LICH_KING
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL, 1);
        opponent->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL, 1);

        // Credit for quest Death's Challenge
        if (GetClass() == CLASS_DEATH_KNIGHT && opponent->GetQuestStatus(12733) == QUEST_STATUS_INCOMPLETE)
            opponent->CastSpell(opponent, 52994, true);

        // Honor points after duel (the winner) - ImpConfig
        if (uint32 amount = sWorld->getIntConfig(CONFIG_HONOR_AFTER_DUEL))
            opponent->RewardHonor(nullptr, 1, amount);
#endif

        break;
    default:
        break;
    }

    // cool-down duel spell
    /*data.Initialize(SMSG_SPELL_COOLDOWN, 17);

    data<<GetGUID();
    data<<uint8(0x0);

    data<<(uint32)7266;
    data<<uint32(0x0);
    SendDirectMessage(&data);
    data.Initialize(SMSG_SPELL_COOLDOWN, 17);
    data<<duel->Opponent->GetGUID();
    data<<uint8(0x0);
    data<<(uint32)7266;
    data<<uint32(0x0);
    duel->Opponent->SendDirectMessage(&data);*/

#ifdef LICH_KING
    // Victory emote spell
    if (type != DUEL_INTERRUPTED)
        opponent->CastSpell(opponent, 52852, true);
#endif

    //Remove Duel Flag object
    GameObject* obj = GetMap()->GetGameObject(GetGuidValue(PLAYER_DUEL_ARBITER));
    if(obj)
        duel->Initiator->RemoveGameObject(obj, true);

    /* remove auras */
    AuraApplicationMap &itsAuras = opponent->GetAppliedAuras();
    for (AuraApplicationMap::iterator i = itsAuras.begin(); i != itsAuras.end();)
    {
        Aura const* aura = i->second->GetBase();
        if (!i->second->IsPositive() && aura->GetCasterGUID() == GetGUID() && aura->GetApplyTime() >= duel->StartTime)
            opponent->RemoveAura(i);
        else
            ++i;
    }

    AuraApplicationMap &myAuras = GetAppliedAuras();
    for (AuraApplicationMap::iterator i = myAuras.begin(); i != myAuras.end();)
    {
        Aura const* aura = i->second->GetBase();
        if (!i->second->IsPositive() && aura->GetCasterGUID() == opponent->GetGUID() && aura->GetApplyTime() >= duel->StartTime)
            RemoveAura(i);
        else
            ++i;
    }

    // cleanup combo points

    if (Unit* comboTarget = ObjectAccessor::GetUnit(*this, GetComboTarget()))
        if (comboTarget->GetControllingPlayer() == opponent)
            ClearComboPoints();

    if (Unit* opponentComboTarget = ObjectAccessor::GetUnit(*this, opponent->GetComboTarget()))
        if (opponentComboTarget->GetControllingPlayer() == this)
            opponent->ClearComboPoints();

    // Refresh in PvPZone
    if(IsInDuelArea())
    {
        SetFullHealth();
        if(Pet* pet = GetPet())
            pet->SetHealth(pet->GetMaxHealth());
        if(GetPowerType() == POWER_MANA || GetClass() == CLASS_DRUID)
            SetPower(POWER_MANA,GetMaxPower(POWER_MANA));

        opponent->SetHealth(opponent->GetMaxHealth());
        if(Pet* pet = opponent->GetPet())
            pet->SetHealth(pet->GetMaxHealth());
        if(opponent->GetPowerType() == POWER_MANA || GetClass() == CLASS_DRUID)
            opponent->SetPower(POWER_MANA, opponent->GetMaxPower(POWER_MANA));
    }

    //cleanups
    SetGuidValue(PLAYER_DUEL_ARBITER, ObjectGuid::Empty);
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);
    opponent->SetGuidValue(PLAYER_DUEL_ARBITER, ObjectGuid::Empty);
    opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    opponent->duel.reset(nullptr);
    duel.reset(nullptr);
}

void Player::SendDuelCountdown(uint32 counter)
{
    WorldPacket data(SMSG_DUEL_COUNTDOWN, 4);
    data << uint32(counter);                                // seconds
    SendDirectMessage(&data);
}

//---------------------------------------------------------//

void Player::_ApplyItemMods(Item *item, uint8 slot,bool apply, bool updateItemAuras /*= true*/)
{
    if(slot >= INVENTORY_SLOT_BAG_END || !item)
        return;

    // not apply mods for broken item
    if(item->IsBroken() && apply)
        return;

    ItemTemplate const *proto = item->GetTemplate();

    if(!proto)
        return;

    //TC_LOG_DEBUG("entities.player","applying mods for item %u ",item->GetGUID().GetCounter());

    _ApplyItemBonuses(proto,slot,apply);

    if( slot==EQUIPMENT_SLOT_RANGED )
        _ApplyAmmoBonuses();

    ApplyItemEquipSpell(item,apply);
    if (updateItemAuras)
    {
        ApplyItemDependentAuras(item, apply);

        WeaponAttackType const attackType = Player::GetAttackBySlot(slot);
        if (attackType != MAX_ATTACK)
            UpdateWeaponDependentAuras(attackType);
    }

    ApplyEnchantment(item, apply);

    if(proto->Socket[0].Color)                              //only (un)equipping of items with sockets can influence metagems, so no need to waste time with normal items
        CorrectMetaGemEnchants(slot, apply);
}

void Player::_ApplyItemBonuses(ItemTemplate const *proto,uint8 slot,bool apply)
{
    if(slot >= INVENTORY_SLOT_BAG_END || !proto)
        return;

    for (auto i : proto->ItemStat)
    {
        float val = float (i.ItemStatValue);

        if(val==0)
            continue;

        switch (i.ItemStatType)
        {
            case ITEM_MOD_MANA:
                HandleStatFlatModifier(UNIT_MOD_MANA, BASE_VALUE, float(val), apply);
                break;
            case ITEM_MOD_HEALTH:                           // modify HP
                HandleStatFlatModifier(UNIT_MOD_HEALTH, BASE_VALUE, float(val), apply);
                break;
            case ITEM_MOD_AGILITY:                          // modify agility
                HandleStatFlatModifier(UNIT_MOD_STAT_AGILITY, BASE_VALUE, float(val), apply);
                UpdateStatBuffMod(STAT_AGILITY);
                break;
            case ITEM_MOD_STRENGTH:                         //modify strength
                HandleStatFlatModifier(UNIT_MOD_STAT_STRENGTH, BASE_VALUE, float(val), apply);
                UpdateStatBuffMod(STAT_STRENGTH);
                break;
            case ITEM_MOD_INTELLECT:                        //modify intellect
                HandleStatFlatModifier(UNIT_MOD_STAT_INTELLECT, BASE_VALUE, float(val), apply);
                UpdateStatBuffMod(STAT_INTELLECT);
                break;
            case ITEM_MOD_SPIRIT:                           //modify spirit
                HandleStatFlatModifier(UNIT_MOD_STAT_SPIRIT, BASE_VALUE, float(val), apply);
                UpdateStatBuffMod(STAT_SPIRIT);
                break;
            case ITEM_MOD_STAMINA:                          //modify stamina
                HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA, BASE_VALUE, float(val), apply);
                UpdateStatBuffMod(STAT_STAMINA);
                break;
            case ITEM_MOD_DEFENSE_SKILL_RATING:
                ApplyRatingMod(CR_DEFENSE_SKILL, int32(val), apply);
                break;
            case ITEM_MOD_DODGE_RATING:
                ApplyRatingMod(CR_DODGE, int32(val), apply);
                break;
            case ITEM_MOD_PARRY_RATING:
                ApplyRatingMod(CR_PARRY, int32(val), apply);
                break;
            case ITEM_MOD_BLOCK_RATING:
                ApplyRatingMod(CR_BLOCK, int32(val), apply);
                break;
            case ITEM_MOD_HIT_MELEE_RATING:
                ApplyRatingMod(CR_HIT_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_HIT_RANGED_RATING:
                ApplyRatingMod(CR_HIT_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_HIT_SPELL_RATING:
                ApplyRatingMod(CR_HIT_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_MELEE_RATING:
                ApplyRatingMod(CR_CRIT_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_RANGED_RATING:
                ApplyRatingMod(CR_CRIT_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_SPELL_RATING:
                ApplyRatingMod(CR_CRIT_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HIT_TAKEN_MELEE_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_HIT_TAKEN_RANGED_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_HIT_TAKEN_SPELL_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_MELEE_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_RANGED_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_SPELL_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_MELEE_RATING:
                ApplyRatingMod(CR_HASTE_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_RANGED_RATING:
                ApplyRatingMod(CR_HASTE_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_SPELL_RATING:
                ApplyRatingMod(CR_HASTE_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HIT_RATING:
                ApplyRatingMod(CR_HIT_MELEE, int32(val), apply);
                ApplyRatingMod(CR_HIT_RANGED, int32(val), apply);
#ifdef LICH_KING
                //Patch 3.0.2 : Hit Rating, Critical Strike Rating, and Haste Rating now modify both melee attacks and spells.
                ApplyRatingMod(CR_HIT_SPELL, int32(val), apply);
#endif
                break;
            case ITEM_MOD_CRIT_RATING:
                ApplyRatingMod(CR_CRIT_MELEE, int32(val), apply);
                ApplyRatingMod(CR_CRIT_RANGED, int32(val), apply);
#ifdef LICH_KING
                //Patch 3.0.2 : Hit Rating, Critical Strike Rating, and Haste Rating now modify both melee attacks and spells.
                ApplyRatingMod(CR_CRIT_SPELL, int32(val), apply);
#endif
                break;
            case ITEM_MOD_HIT_TAKEN_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_MELEE, int32(val), apply);
                ApplyRatingMod(CR_HIT_TAKEN_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_MELEE, int32(val), apply);
                ApplyRatingMod(CR_CRIT_TAKEN_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_RESILIENCE_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_MELEE, int32(val), apply);
                ApplyRatingMod(CR_CRIT_TAKEN_RANGED, int32(val), apply);
                ApplyRatingMod(CR_CRIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_RATING:
                ApplyRatingMod(CR_HASTE_MELEE, int32(val), apply);
                ApplyRatingMod(CR_HASTE_RANGED, int32(val), apply);
#ifdef LICH_KING
                //Patch 3.0.2 : Hit Rating, Critical Strike Rating, and Haste Rating now modify both melee attacks and spells.
                ApplyRatingMod(CR_HASTE_SPELL, int32(val), apply);
#endif
                break;
            case ITEM_MOD_EXPERTISE_RATING:
                ApplyRatingMod(CR_EXPERTISE, int32(val), apply);
                break;
        }
    }

    uint32 armor = proto->Armor;
    if (armor)
    {
        UnitModifierFlatType modType = TOTAL_VALUE;
        if (proto->Class == ITEM_CLASS_ARMOR)
        {
            // Why this distinction? Why does armor from a weapon or trinket is handled differently?
            switch (proto->SubClass)
            {
            case ITEM_SUBCLASS_ARMOR_CLOTH:
            case ITEM_SUBCLASS_ARMOR_LEATHER:
            case ITEM_SUBCLASS_ARMOR_MAIL:
            case ITEM_SUBCLASS_ARMOR_PLATE:
            case ITEM_SUBCLASS_ARMOR_SHIELD:
                modType = BASE_VALUE;
                break;
            }
        }
        HandleStatFlatModifier(UNIT_MOD_ARMOR, modType, float(armor), apply);
    }

    // Add armor bonus from ArmorDamageModifier if > 0
    /* sun: 
    For BC: The "green" armor seems to point to the fact that the item has high base armor for its level and quality, 
    and not that even more armor is added beyond what is shown.
    - This green value (ArmorDamageModifier) is not shown anywhere in the client so green armor would mean "unknown hidden bonus armor" if it was added
    - This handling has probably changed the way this is handled. Note that LK client has changed the way it is showned and explicitely give the bonus armor value besides the armor.
    - Corecraft also ignore this bonus armor
    - Patch 3.0.8 (2009-01-20): Bonus armor beyond the base armor of an item will no longer be multiplied by any talents or by the bonuses of  [Bear Form],  [Dire Bear Form], or  [Frost Presence].
    - http://www.wowhead.com/forums&topic=64127/green-numbers-for-armor 
   */
#ifdef LICH_KING
    if (proto->ArmorDamageModifier > 0)
        HandleStatFlatModifier(UNIT_MOD_ARMOR, TOTAL_VALUE, float(proto->ArmorDamageModifier), apply);
#endif

    if (proto->Block)
        HandleBaseModFlatValue(SHIELD_BLOCK_VALUE, float(proto->Block), apply);

    if (proto->HolyRes)
        HandleStatFlatModifier(UNIT_MOD_RESISTANCE_HOLY, BASE_VALUE, float(proto->HolyRes), apply);

    if (proto->FireRes)
        HandleStatFlatModifier(UNIT_MOD_RESISTANCE_FIRE, BASE_VALUE, float(proto->FireRes), apply);

    if (proto->NatureRes)
        HandleStatFlatModifier(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(proto->NatureRes), apply);

    if (proto->FrostRes)
        HandleStatFlatModifier(UNIT_MOD_RESISTANCE_FROST, BASE_VALUE, float(proto->FrostRes), apply);

    if (proto->ShadowRes)
        HandleStatFlatModifier(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(proto->ShadowRes), apply);

    if (proto->ArcaneRes)
        HandleStatFlatModifier(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(proto->ArcaneRes), apply);

    WeaponAttackType attType = Player::GetAttackBySlot(slot);
    if (attType != MAX_ATTACK)
        _ApplyWeaponDamage(slot, proto, apply);

#ifdef LICH_KING
    // Druids get feral AP bonus from weapon dps (also use DPS from ScalingStatValue)
    if (getClass() == CLASS_DRUID)
    {
        int32 dpsMod = 0;
        int32 feral_bonus = 0;
        if (ssv)
        {
            dpsMod = ssv->getDPSMod(proto->ScalingStatValue);
            feral_bonus += ssv->getFeralBonus(proto->ScalingStatValue);
        }

        feral_bonus += proto->getFeralBonus(dpsMod);
        if (feral_bonus)
            ApplyFeralAPBonus(feral_bonus, apply);
    }
#endif
}

void Player::_ApplyWeaponDamage(uint8 slot, ItemTemplate const* proto, bool apply)
{
    WeaponAttackType attType = Player::GetAttackBySlot(slot);
    if (!IsInFeralForm() && apply && !CanUseAttackType(attType))
        return;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
    {
        float minDamage = proto->Damage[i].DamageMin;
        float maxDamage = proto->Damage[i].DamageMax;

    #ifdef LICH_KING
        // If set dpsMod in ScalingStatValue use it for min (70% from average), max (130% from average) damage
        if (ssv && i == 0)
        {
            int32 extraDPS = ssv->getDPSMod(proto->ScalingStatValue);
            if (extraDPS)
            {
                float average = extraDPS * proto->Delay / 1000.0f;
                minDamage = 0.7f * average;
                maxDamage = 1.3f * average;
            }
        }
    #endif

        if (apply)
        {
            if (minDamage > 0)
                SetBaseWeaponDamage(attType, MINDAMAGE, minDamage, i);

            if (maxDamage > 0)
                SetBaseWeaponDamage(attType, MAXDAMAGE, maxDamage, i);
        }
    }

    if (!apply)
    {
        for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            SetBaseWeaponDamage(attType, MINDAMAGE, 0.f, i);
            SetBaseWeaponDamage(attType, MAXDAMAGE, 0.f, i);
        }

        if (attType == BASE_ATTACK)
        {
            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, BASE_MINDAMAGE);
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, BASE_MAXDAMAGE);
        }
    }

    if (proto->Delay && !IsInFeralForm())
    {
        SetAttackTime(attType, apply ? proto->Delay : BASE_ATTACK_TIME);
    }

    // No need to modify any physical damage for ferals as it is calculated from stats only
    if (IsInFeralForm())
        return;

    if (CanModifyStats() && (GetWeaponDamageRange(attType, MAXDAMAGE) || proto->Delay))
        UpdateDamagePhysical(attType);
}

SpellSchoolMask Player::GetMeleeDamageSchoolMask(WeaponAttackType attackType /*= BASE_ATTACK*/, uint8 damageIndex /*= 0*/) const
{
    if (Item const* weapon = GetWeaponForAttack(attackType, true))
        return SpellSchoolMask(1 << weapon->GetTemplate()->Damage[damageIndex].DamageType);

    return SPELL_SCHOOL_MASK_NORMAL;
}

void Player::ApplyItemEquipSpell(Item *item, bool apply, bool form_change)
{
    if(!item)
        return;

    ItemTemplate const *proto = item->GetTemplate();
    if(!proto)
        return;

    for (const auto & spellData : proto->Spells)
    {
        // no spell
        if(!spellData.SpellId )
            continue;

        // wrong triggering type
        if(apply && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_EQUIP)
            continue;

        // check if it is valid spell
        SpellInfo const* spellproto = sSpellMgr->GetSpellInfo(spellData.SpellId);
        if(!spellproto)
            continue;

        ApplyEquipSpell(spellproto,item,apply,form_change);
    }
}

void Player::ApplyEquipSpell(SpellInfo const* spellInfo, Item* item, bool apply, bool form_change)
{
    if(apply)
    {
        // Cannot be used in this stance/form
        if(CheckShapeshift(spellInfo, GetShapeshiftForm()) != SPELL_CAST_OK)
            return;

        if(form_change)                                     // check aura active state from other form
        {
            AuraApplicationMapBounds range = GetAppliedAuras().equal_range(spellInfo->Id);
            for (AuraApplicationMap::const_iterator itr = range.first; itr != range.second; ++itr)
                if (!item || itr->second->GetBase()->GetCastItemGUID() == item->GetGUID())
                    return;
        }

        TC_LOG_DEBUG("entities.player","WORLD: cast %s Equip spellId - %i", (item ? "item" : "itemset"), spellInfo->Id);

        CastSpell(this, spellInfo->Id, item);
    }
    else
    {
        if(form_change)                                     // check aura compatibility
        {
            // Cannot be used in this stance/form
            if(CheckShapeshift(spellInfo, GetShapeshiftForm()) == SPELL_CAST_OK)
                return;                                     // and remove only not compatible at form change
        }

        if (item)
            RemoveAurasDueToItemSpell(spellInfo->Id, item->GetGUID());  // un-apply all spells, not only at-equipped
        else
            RemoveAurasDueToSpell(spellInfo->Id);           // un-apply spell (item set case)
    }
}

void Player::UpdateEquipSpellsAtFormChange()
{
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i] && !m_items[i]->IsBroken() && CanUseAttackType(GetAttackBySlot(i)))
        {
            ApplyItemEquipSpell(m_items[i], false, true);     // remove spells that not fit to form
            ApplyItemEquipSpell(m_items[i], true, true);      // add spells that fit form but not active
        }
    }

    // item set bonuses not dependent from item broken state
    for(auto eff : ItemSetEff)
    {
        if(!eff)
            continue;

        for(auto spellInfo : eff->spells)
        {
            if(!spellInfo)
                continue;

            ApplyEquipSpell(spellInfo, nullptr, false, true);       // remove spells that not fit to form
            ApplyEquipSpell(spellInfo, nullptr, true, true);        // add spells that fit form but not active
        }
    }
}

void Player::CastItemCombatSpell(DamageInfo const& damageInfo)
{
    //windrunner rule
    if (damageInfo.GetSpellInfo() && (damageInfo.GetSpellInfo()->Attributes & SPELL_ATTR0_STOP_ATTACK_TARGET))
        return;

    Unit* target = damageInfo.GetVictim();
    if (!target || !target->IsAlive() || target == this)
        return;

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        // If usable, try to cast item spell
        if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (!item->IsBroken() && CanUseAttackType(damageInfo.GetAttackType()))
            {
                if (ItemTemplate const* proto = item->GetTemplate())
                {
                    // Additional check for weapons
                    if (proto->Class == ITEM_CLASS_WEAPON)
                    {
                        // offhand item cannot proc from main hand hit etc
                        EquipmentSlots slot;
                        switch (damageInfo.GetAttackType())
                        {
                        case BASE_ATTACK:
                            slot = EQUIPMENT_SLOT_MAINHAND;
                            break;
                        case OFF_ATTACK:
                            slot = EQUIPMENT_SLOT_OFFHAND;
                            break;
                        case RANGED_ATTACK:
                            slot = EQUIPMENT_SLOT_RANGED;
                            break;
                        default:
                            slot = EQUIPMENT_SLOT_END;
                            break;
                        }
                        if (slot != i)
                            continue;
                        // Check if item is useable (forms or disarm)
                        if (damageInfo.GetAttackType() == BASE_ATTACK)
                            if (!IsUseEquipedWeapon(true) && !IsInFeralForm())
                                continue;
                    }

                    CastItemCombatSpell(damageInfo, item, proto);
                }
            }
        }
    }
}

void Player::CastItemCombatSpell(DamageInfo const& damageInfo, Item* item, ItemTemplate const* proto)
{
    // Can do effect if any damage done to target
    // for done procs allow normal + critical + absorbs by default
    bool canTrigger = (damageInfo.GetHitMask() & (PROC_HIT_NORMAL | PROC_HIT_CRITICAL | PROC_HIT_ABSORB)) != 0;
    if (canTrigger)
    {
        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            _Spell const& spellData = proto->Spells[i];

            // no spell
            if (spellData.SpellId <= 0)
                continue;

            // wrong triggering type
            if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_CHANCE_ON_HIT)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellData.SpellId);
            if (!spellInfo)
            {
                TC_LOG_ERROR("entities.player.items", "Player::CastItemCombatSpell: Player '%s' (%s) cast unknown item spell (ID: %i)",
                    GetName().c_str(), GetGUID().ToString().c_str(), spellData.SpellId);
                continue;
            }

            // not allow proc extra attack spell at extra attack
            if (m_extraAttacks && spellInfo->HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS))
                return;

            float chance = (float)spellInfo->ProcChance;

            if (spellData.SpellPPMRate)
            {
                uint32 WeaponSpeed = GetAttackTime(damageInfo.GetAttackType());
                chance = GetPPMProcChance(WeaponSpeed, spellData.SpellPPMRate, spellInfo);
            }
            else if (chance > 100.0f)
                chance = GetWeaponProcChance();

            if (roll_chance_f(chance) /* TC && sScriptMgr->OnCastItemCombatSpell(this, damageInfo.GetVictim(), spellInfo, item) */)
                CastSpell(damageInfo.GetVictim(), spellInfo->Id, item);
        }
    }

    // item combat enchantments
    for (uint8 e_slot = 0; e_slot < MAX_ENCHANTMENT_SLOT; ++e_slot)
    {
        uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(e_slot));
        SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!pEnchant)
            continue;

        for (uint8 s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
        {
            if (pEnchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
            {
                { //old WR hack
                    uint32 FTSpellId = 0;
                    switch (pEnchant->spellid[s])
                    {
                    // Flametongue Weapon
                    case 10400:  FTSpellId = 8026; break; // Rank 1
                    case 15567: FTSpellId = 8028; break; // Rank 2
                    case 15568: FTSpellId = 8029; break; // Rank 3
                    case 15569: FTSpellId = 10445; break; // Rank 4
                    case 16311: FTSpellId = 16343; break; // Rank 5
                    case 16312: FTSpellId = 16344; break; // Rank 6
                    case 16313: FTSpellId = 25488; break; // Rank 7
                    }
                    if (FTSpellId)
                        CastSpell(damageInfo.GetVictim(), FTSpellId, item);
                }

                continue;
            }

            SpellEnchantProcEntry const* entry = sSpellMgr->GetSpellEnchantProcEvent(enchant_id);
            if (entry && entry->HitMask)
            {
                // Check hit/crit/dodge/parry requirement
                if ((entry->HitMask & damageInfo.GetHitMask()) == 0)
                    continue;
            }
            else
            {
                // Can do effect if any damage done to target
                // for done procs allow normal + critical + absorbs by default
                if (!canTrigger)
                    continue;
            }

            // check if enchant procs only on white hits
            if (entry && (entry->AttributesMask & ENCHANT_PROC_ATTR_WHITE_HIT) && damageInfo.GetSpellInfo())
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(pEnchant->spellid[s]);
            if (!spellInfo)
            {
                TC_LOG_ERROR("entities.player.items", "Player::CastItemCombatSpell: Player '%s' (%s) cast unknown spell (EnchantID: %u, SpellID: %i), ignoring",
                    GetName().c_str(), GetGUID().ToString().c_str(), pEnchant->ID, pEnchant->spellid[s]);
                continue;
            }
            //Old sunstrider rules: do not allow proc windfury totem from yellow attacks except for attacks on next swing
            {
                if (damageInfo.GetSpellInfo()
                    && !damageInfo.GetSpellInfo()->IsNextMeleeSwingSpell()
                    && spellInfo->SpellFamilyName == SPELLFAMILY_SHAMAN
                    && spellInfo->SpellFamilyFlags & 0x200000000LL)
                    continue;

                // not allow proc extra attack spell at extra attack
                if (m_extraAttacks && IsSpellHaveEffect(spellInfo, SPELL_EFFECT_ADD_EXTRA_ATTACKS))
                    continue;
            }

            float chance = pEnchant->amount[s] != 0 ? float(pEnchant->amount[s]) : GetWeaponProcChance();
            if (entry)
            {
                if (entry->ProcsPerMinute)
                    chance = GetPPMProcChance(proto->Delay, entry->ProcsPerMinute, spellInfo);
                else if (entry->Chance)
                    chance = entry->Chance;
            }

            // Apply spell mods
            ApplySpellMod(pEnchant->spellid[s], SPELLMOD_CHANCE_OF_SUCCESS, chance);

            // Shiv has 100% chance to apply the poison
            if (FindCurrentSpellBySpellId(5938) && e_slot == TEMP_ENCHANTMENT_SLOT)
                chance = 100.0f;

            if (roll_chance_f(chance))
            {
                Unit* target = spellInfo->IsPositive() ? this : damageInfo.GetVictim();

                CastSpellExtraArgs args(item);
                // reduce effect values if enchant is limited
                if (entry && (entry->AttributesMask & ENCHANT_PROC_ATTR_LIMIT_60) && target->GetLevel() > 60)
                {
                    int32 const lvlDifference = target->GetLevel() - 60;
                    int32 const lvlPenaltyFactor = 4; // 4% lost effectiveness per level

                    int32 const effectPct = std::max(0, 100 - (lvlDifference * lvlPenaltyFactor));

                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        if (spellInfo->Effects[i].IsEffect())
                            args.AddSpellMod(static_cast<SpellValueMod>(SPELLVALUE_BASE_POINT0 + i), CalculatePct(spellInfo->Effects[i].CalcValue(this), effectPct));
                }
                CastSpell(target, spellInfo->Id, args);
            }
        }
    }
}

bool Player::CastItemUseSpell(Item* item, SpellCastTargets const& targets, uint8 cast_count, uint32 glyphIndex)
{
#ifdef LICH_KING
    --"todo glyphIndex";
#endif

    ItemTemplate const* proto = item->GetTemplate();
    // special learning case
    if (proto->Spells[0].SpellId == SPELL_ID_GENERIC_LEARN)
    {
        uint32 learning_spell_id = item->GetTemplate()->Spells[1].SpellId;

        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(SPELL_ID_GENERIC_LEARN);
        if (!spellInfo)
        {
            TC_LOG_ERROR("entities.player", "Item (Entry: %u) in have wrong spell id %u, ignoring ", proto->ItemId, SPELL_ID_GENERIC_LEARN);
            SendEquipError(EQUIP_ERR_NONE, item, nullptr);
            return false;
        }

        auto spell = new Spell(this, spellInfo, TRIGGERED_NONE);
        spell->m_CastItem = item;
        spell->m_cast_count = cast_count;               //set count of casts
        spell->SetSpellValue(SPELLVALUE_BASE_POINT0, learning_spell_id);
        uint32 res = spell->prepare(targets);
        return res == SPELL_CAST_OK;
    }

    // use triggered flag only for items with many spell casts and for not first cast
    int count = 0;

    std::list<Spell*> pushSpells;
    for (const auto & spellData : item->GetTemplate()->Spells)
    {
        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
            continue;

        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellData.SpellId);
        if (!spellInfo)
        {
            TC_LOG_ERROR("entities.player", "Item (Entry: %u) in have wrong spell id %u, ignoring ", proto->ItemId, spellData.SpellId);
            continue;
        }

        auto spell = new Spell(this, spellInfo, (count > 0) ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);
        spell->m_CastItem = item;
        spell->m_cast_count = cast_count;               //set count of casts
        spell->InitExplicitTargets(targets);

        // sunwell: dont allow to cast such spells, it may happen that spell possess 2 spells, one for players and one for items / gameobjects
        // sunwell: if first one is cast on player, it may be deleted thus resulting in crash because second spell has saved pointer to the item
        // sunwell: there is one problem with scripts which wont be loaded at the moment of call
        SpellCastResult result = spell->CheckCast(true);
        if (result != SPELL_CAST_OK)
        {
            spell->SendCastResult(result);
            delete spell;
            continue;
        }

        pushSpells.push_back(spell);

        ++count;
    }


    bool castedOneSpell = false;
    // sunwell: send all spells in one go, prevents crash because container is not set
    for (std::list<Spell*>::const_iterator itr = pushSpells.begin(); itr != pushSpells.end(); ++itr)
    {
        uint32 res = (*itr)->prepare(targets);
        if(res == SPELL_CAST_OK)
            castedOneSpell = true;
    }

    return castedOneSpell;
}

void Player::_RemoveAllItemMods()
{
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            ItemTemplate const *proto = m_items[i]->GetTemplate();
            if(!proto)
                continue;

            // item set bonuses not dependent from item broken state
            if(proto->ItemSet)
                RemoveItemsSetItem(this,proto);

            if(m_items[i]->IsBroken())
                continue;

            ApplyItemEquipSpell(m_items[i], false);
            ApplyEnchantment(m_items[i], false);
        }
    }

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            if (m_items[i]->IsBroken() || !CanUseAttackType(GetAttackBySlot(i)))
                continue;
            ItemTemplate const *proto = m_items[i]->GetTemplate();
            if(!proto)
                continue;

            ApplyItemDependentAuras(m_items[i], false);
            _ApplyItemBonuses(proto,i, false);

            if (i == EQUIPMENT_SLOT_RANGED)
                _ApplyAmmoBonuses();
        }
    }
}

void Player::_ApplyAllItemMods()
{
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            if(m_items[i]->IsBroken())
                continue;

            ItemTemplate const *proto = m_items[i]->GetTemplate();
            if(!proto)
                continue;

            ApplyItemDependentAuras(m_items[i], true);
            _ApplyItemBonuses(proto, i, true);

            WeaponAttackType const attackType = Player::GetAttackBySlot(i);
            if (attackType != MAX_ATTACK)
                UpdateWeaponDependentAuras(attackType);

            if (i == EQUIPMENT_SLOT_RANGED)
                _ApplyAmmoBonuses();

        }
    }

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(m_items[i])
        {
            ItemTemplate const *proto = m_items[i]->GetTemplate();
            if(!proto)
                continue;

            // item set bonuses not dependent from item broken state
            if(proto->ItemSet)
                AddItemsSetItem(this,m_items[i]);

            if (m_items[i]->IsBroken() || !CanUseAttackType(GetAttackBySlot(i)))
                continue;

            ApplyItemEquipSpell(m_items[i],true);
            ApplyEnchantment(m_items[i], true);
        }
    }
}

void Player::_ApplyAmmoBonuses()
{
    // check ammo
    uint32 ammo_id = GetUInt32Value(PLAYER_AMMO_ID);
    if(!ammo_id)
        return;

    float currentAmmoDPS;

    ItemTemplate const *ammo_proto = sObjectMgr->GetItemTemplate( ammo_id );
    if( !ammo_proto || ammo_proto->Class!=ITEM_CLASS_PROJECTILE || !CheckAmmoCompatibility(ammo_proto))
        currentAmmoDPS = 0.0f;
    else
        currentAmmoDPS = (ammo_proto->Damage[0].DamageMin + ammo_proto->Damage[0].DamageMax) / 2;

    if(currentAmmoDPS == GetAmmoDPS())
        return;

    m_ammoDPS = currentAmmoDPS;

    if(CanModifyStats())
        UpdateDamagePhysical(RANGED_ATTACK);
}

bool Player::CheckAmmoCompatibility(const ItemTemplate *ammo_proto) const
{
    if(!ammo_proto)
        return false;

    // check ranged weapon
    Item *weapon = GetWeaponForAttack( RANGED_ATTACK );
    if(!weapon  || weapon->IsBroken() )
        return false;

    ItemTemplate const* weapon_proto = weapon->GetTemplate();
    if(!weapon_proto || weapon_proto->Class!=ITEM_CLASS_WEAPON )
        return false;

    // check ammo ws. weapon compatibility
    switch(weapon_proto->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            if(ammo_proto->SubClass!=ITEM_SUBCLASS_ARROW)
                return false;
            break;
        case ITEM_SUBCLASS_WEAPON_GUN:
            if(ammo_proto->SubClass!=ITEM_SUBCLASS_BULLET)
                return false;
            break;
        default:
            return false;
    }

    return true;
}

/*  If in a battleground a player dies, and an enemy removes the insignia, the player's bones is lootable
    Called by remove insignia spell effect    */
void Player::RemovedInsignia(Player* looterPlr)
{
    if (!GetBattlegroundId())
        return;

    // If not released spirit, do it !
    if(m_deathTimer > 0)
    {
        m_deathTimer = 0;
        BuildPlayerRepop();
        RepopAtGraveyard();
    }

    _corpseLocation.WorldRelocate();

    // We have to convert player corpse to bones, not to be able to resurrect there
    // SpawnCorpseBones isn't handy, 'cos it saves player while he in BG
    Corpse *bones = GetMap()->ConvertCorpseToBones(GetGUID(),true);
    if (!bones)
        return;

    // Now we must make bones lootable, and send player loot
    bones->SetFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);

    // We store the level of our player in the gold field
    // We retrieve this information at Player::SendLoot()
    bones->loot.gold = GetLevel();
    bones->lootRecipient = looterPlr;
    looterPlr->SendLoot(bones->GetGUID(), LOOT_INSIGNIA);
}

/*Loot type MUST be
1-corpse, go
2-skinning
3-Fishing
*/

void Player::SendLootRelease( ObjectGuid guid )
{
    WorldPacket data( SMSG_LOOT_RELEASE_RESPONSE, (8+1) );
    data << uint64(guid) << uint8(1);
    SendDirectMessage( &data );
}

void Player::SendLootError(ObjectGuid guid, LootError error)
{
    WorldPacket data(SMSG_LOOT_RESPONSE, 10);
    data << uint64(guid);
    data << uint8(LOOT_NONE);
    data << uint8(error);
    SendDirectMessage(&data);
}

void Player::SendLoot(ObjectGuid guid, LootType loot_type)
{
    Loot* loot = nullptr;
    PermissionTypes permission = ALL_PERMISSION;

    // release old loot
    if(ObjectGuid lguid = GetLootGUID())
        GetSession()->DoLootRelease(lguid);

    if (guid.IsGameObject())
    {
        GameObject *go = ObjectAccessor::GetGameObject(*this, guid);

        auto shouldLootRelease = [this](GameObject* go, LootType lootType) -> bool
        {
            // not check distance for GO in case owned GO (fishing bobber case, for example)
            // And permit out of range GO with no owner in case fishing hole
            if (!go)
                return true;

            if (lootType == LOOT_SKINNING)
            {
                // Disarm Trap
                if (!go->IsWithinDistInMap(this, 20.f))
                    return true;
            }
            else
            {
                //check interaction distance if not fishing
                if (lootType != LOOT_FISHINGHOLE 
                    && ( (lootType != LOOT_FISHING 
#ifdef LICH_KING
                        && lootType != LOOT_FISHING_JUNK
#endif
                        ) 
                        || go->GetOwnerGUID() != GetGUID()) 
                    && !go->IsWithinDistInMap(this, INTERACTION_DISTANCE))
                    return true;

                if (lootType == LOOT_CORPSE && go->GetRespawnTime() && go->isSpawnedByDefault())
                    return true;
            }

            return false;
        };

        if (shouldLootRelease(go, loot_type))
        {
            SendLootRelease(guid);
            return;
        }

        loot = &go->loot;

        // loot was generated and respawntime has passed since then, allow to recreate loot
        // to avoid bugs, this rule covers spawned gameobjects only
        if (go->isSpawnedByDefault() && go->getLootState() == GO_ACTIVATED && !go->loot.isLooted() && go->GetLootGenerationTime() + go->GetRespawnDelay() < WorldGameTime::GetGameTime())
            go->SetLootState(GO_READY);

        if(go->getLootState() == GO_READY)
        {
            uint32 lootid = go->GetGOInfo()->GetLootId();
            if (Battleground* bg = GetBattleground())
                if (!bg->CanActivateGO(go->GetEntry(), GetTeam()))
                {
                    SendLootRelease(guid);
                    return;
                }

            if(lootid)
            {
                loot->clear();

                Group* group = GetGroup();
                bool groupRules = (group && go->GetGOInfo()->type == GAMEOBJECT_TYPE_CHEST && go->GetGOInfo()->chest.groupLootRules);

                // check current RR player and get next if necessary
                if (groupRules)
                    group->UpdateLooterGuid(go, true);

                loot->FillLoot(lootid, LootTemplates_Gameobject, this, !groupRules, false, go->GetLootMode());
                go->SetLootGenerationTime();

                // get next RR player (for next loot)
                if (groupRules && !go->loot.empty())
                    group->UpdateLooterGuid(go);
            }

            /*TC, but not used in their db as of writing (2018-02-19)
            if (go->GetLootMode() > 0)
                //if (GameObjectTemplateAddon const* addon = go->GetTemplateAddon())
                    loot->generateMoneyLoot(addon->mingold, addon->maxgold);
                    */

            if(loot_type == LOOT_FISHING)
                go->getFishLoot(loot, this);
#ifdef LICH_KING
            else if (loot_type == LOOT_FISHING_JUNK)
                go->getFishLootJunk(loot, this);
#endif

            if (go->GetGOInfo()->type == GAMEOBJECT_TYPE_CHEST && go->GetGOInfo()->chest.groupLootRules)
            {
                if (Group* group = GetGroup())
                {
                    switch (group->GetLootMethod())
                    {
                    case GROUP_LOOT:
                        // GroupLoot: rolls items over threshold. Items with quality < threshold, round robin
                        group->GroupLoot(loot, go);
                        break;
                    case NEED_BEFORE_GREED:
                        group->NeedBeforeGreed(loot, go);
                        break;
                    case MASTER_LOOT:
                        group->MasterLoot(loot, go);
                        break;
                    default:
                        break;
                    }
                }
            }

            go->SetLootState(GO_ACTIVATED, this);
        }

        if (go->getLootState() == GO_ACTIVATED)
        {
            if (Group* group = GetGroup())
            {
                switch (group->GetLootMethod())
                {
                case MASTER_LOOT:
                    permission = group->GetMasterLooterGuid() == GetGUID() ? MASTER_PERMISSION : RESTRICTED_PERMISSION;
                    break;
                case FREE_FOR_ALL:
                    permission = ALL_PERMISSION;
                    break;
                case ROUND_ROBIN:
                    permission = ROUND_ROBIN_PERMISSION;
                    break;
                default:
                    permission = GROUP_PERMISSION;
                    break;
                }
            }
            else
                permission = ALL_PERMISSION;
        }
    }
    else if (guid.IsItem())
    {
        Item *item = GetItemByGuid( guid );

        if (!item)
        {
            SendLootRelease(guid);
            return;
        }

        permission = OWNER_PERMISSION;
        loot = &item->loot;

        // Store container id
        loot->containerID = item->GetGUID().GetCounter();

        // If item doesn't already have loot, attempt to load it. If that
        // fails then this is first time opening, generate loot
        if (!item->m_lootGenerated && !sLootItemStorage->LoadStoredLoot(item, this))
        {
            item->m_lootGenerated = true;
            loot->clear();

            switch (loot_type)
            {
            case LOOT_DISENCHANTING:
                loot->FillLoot(item->GetTemplate()->DisenchantID, LootTemplates_Disenchant, this, true);
                break;
            case LOOT_PROSPECTING:
                loot->FillLoot(item->GetEntry(), LootTemplates_Prospecting, this, true);
                break;
#ifdef LICH_KING
            case LOOT_MILLING:
                loot->FillLoot(item->GetEntry(), LootTemplates_Milling, this, true);
                break;
#endif
            default:
                loot->generateMoneyLoot(item->GetTemplate()->MinMoneyLoot, item->GetTemplate()->MaxMoneyLoot);
                loot->FillLoot(item->GetEntry(), LootTemplates_Item, this, true, loot->gold != 0);

                // Force save the loot and money items that were just rolled
                //  Also saves the container item ID in Loot struct (not to DB)
                if (loot->gold > 0 || loot->unlootedCount > 0)
                    sLootItemStorage->AddNewStoredLoot(loot, this);

                break;
            }
        }
    }
    else if (guid.IsCorpse())                          // remove insignia
    {
        Corpse *bones = ObjectAccessor::GetCorpse(*this, guid);

        if (!bones || !((loot_type == LOOT_CORPSE) || (loot_type == LOOT_INSIGNIA)) || (bones->GetType() != CORPSE_BONES) )
        {
            SendLootRelease(guid);
            return;
        }

        loot = &bones->loot;

        if (loot->loot_type == LOOT_NONE)
        {
            uint32 pLevel = bones->loot.gold;
            bones->loot.clear();

            // For AV Achievement
            if (Battleground* bg = GetBattleground())
            {
                if (bg->GetTypeID(/*true*/) == BATTLEGROUND_AV)
                    loot->FillLoot(PLAYER_CORPSE_LOOT_ENTRY, LootTemplates_Creature, this, true);
            }
#ifdef LICH_KING
            // For wintergrasp Quests
            else if (GetZoneId() == AREA_WINTERGRASP)
                loot->FillLoot(PLAYER_CORPSE_LOOT_ENTRY, LootTemplates_Creature, this, true);
#endif

            // It may need a better formula
            // Now it works like this: lvl10: ~6copper, lvl70: ~9silver
            bones->loot.gold = uint32(urand(50, 150) * 0.016f * std::pow(float(pLevel) / 5.76f, 2.5f) * sWorld->GetRate(RATE_DROP_MONEY));
        }

        if (bones->lootRecipient != this)
            permission = NONE_PERMISSION;
        else
            permission = OWNER_PERMISSION;
    }
    else
    {
        Creature *creature = ObjectAccessor::GetCreature(*this, guid);

        // must be in range and creature must be alive for pickpocket and must be dead for another loot
        if (!creature || creature->IsAlive()!=(loot_type == LOOT_PICKPOCKETING) || !creature->IsWithinDistInMap(this,INTERACTION_DISTANCE))
        {
            SendLootRelease(guid);
            return;
        }

        if(loot_type == LOOT_PICKPOCKETING && IsFriendlyTo(creature))
        {
            SendLootRelease(guid);
            return;
        }

        loot = &creature->loot;

        if(loot_type == LOOT_PICKPOCKETING)
        {
            if (loot->loot_type != LOOT_PICKPOCKETING)
            {
                if (creature->CanGeneratePickPocketLoot())
                {
                    creature->StartPickPocketRefillTimer();
                    loot->clear();

                    if (uint32 lootid = creature->GetCreatureTemplate()->pickpocketLootId)
                        loot->FillLoot(lootid, LootTemplates_Pickpocketing, this, true);

                    // Generate extra money for pick pocket loot
                    const uint32 a = urand(0, creature->GetLevel() / 2);
                    const uint32 b = urand(0, GetLevel() / 2);
                    loot->gold = uint32(10 * (a + b) * sWorld->GetRate(RATE_DROP_MONEY));
                }
                else {
                    SendLootError(guid, LOOT_ERROR_ALREADY_PICKPOCKETED);
                    return;
                } // else - still has pickpocket loot generated & not fully taken
            }
        }
        else
        {
            // exploit fix
            if (!creature->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE))
            {
                SendLootError(guid, LOOT_ERROR_DIDNT_KILL);
                return;
            }

            // the player whose group may loot the corpse
            Player* recipient = creature->GetLootRecipient();
            Group* recipientGroup = creature->GetLootRecipientGroup();
            if (!recipient && !recipientGroup)
            {
                SendLootError(guid, LOOT_ERROR_DIDNT_KILL);
                return;
            }

            if (loot->loot_type == LOOT_NONE)
            {
                // for creature, loot is filled when creature is killed.
                if (Group* group = creature->GetLootRecipientGroup())
                {
                    switch (group->GetLootMethod())
                    {
                    case GROUP_LOOT:
                        // GroupLoot: rolls items over threshold. Items with quality < threshold, round robin
                        group->GroupLoot(loot, creature);
                        break;
                    case NEED_BEFORE_GREED:
                        group->NeedBeforeGreed(loot, creature);
                        break;
                    case MASTER_LOOT:
                        group->MasterLoot(loot, creature);
                        break;
                    default:
                        break;
                    }
                }
            }

            // if loot is already skinning loot then don't do anything else
            if (loot->loot_type == LOOT_SKINNING)
            {
                loot_type = LOOT_SKINNING;
                permission = creature->GetLootRecipientGUID() == GetGUID() ? OWNER_PERMISSION : NONE_PERMISSION;
            }
            else if (loot_type == LOOT_SKINNING)
            {
                loot->clear();
                loot->FillLoot(creature->GetCreatureTemplate()->SkinLootId, LootTemplates_Skinning, this, true);
                permission = OWNER_PERMISSION;

                // Set new loot recipient
                creature->SetLootRecipient(this, false);
            }
            // set group rights only for loot_type != LOOT_SKINNING
            else
            {
                if (creature->GetLootRecipientGroup())
                {
                    Group* group = GetGroup();
                    if (group == creature->GetLootRecipientGroup())
                    {
                        switch (group->GetLootMethod())
                        {
                        case MASTER_LOOT:
                            permission = group->GetMasterLooterGuid() == GetGUID() ? MASTER_PERMISSION : RESTRICTED_PERMISSION;
                            break;
                        case FREE_FOR_ALL:
                            permission = ALL_PERMISSION;
                            break;
                        case ROUND_ROBIN:
                            permission = ROUND_ROBIN_PERMISSION;
                            break;
                        default:
                            permission = GROUP_PERMISSION;
                            break;
                        }
                    }
                    else
                        permission = NONE_PERMISSION;
                }
                else if (creature->GetLootRecipient() == this)
                    permission = OWNER_PERMISSION;
                else
                    permission = NONE_PERMISSION;
            }
        }
    }

    auto sendLootType = loot_type; //sun, store real loot type in loot object, we just alter type for sending here (we need to know real loot type in the rest of the code)
    // LOOT_INSIGNIA and LOOT_FISHINGHOLE unsupported by client
    switch (loot_type)
    {
#ifndef LICH_KING
    case LOOT_PICKPOCKETING:
    case LOOT_DISENCHANTING:
    case LOOT_PROSPECTING:
#endif
    case LOOT_INSIGNIA:    sendLootType = LOOT_SKINNING; break;
    case LOOT_FISHINGHOLE: sendLootType = LOOT_FISHING; break;
#ifdef LICH_KING
    case LOOT_FISHING_JUNK: sendLootType = LOOT_FISHING; break;
#endif
    default: break;
    }

    // need know merged fishing/corpse loot type for achievements
    loot->loot_type = loot_type;

    if (permission != NONE_PERMISSION)
    {
        SetLootGUID(guid);

        WorldPacket data(SMSG_LOOT_RESPONSE, (9 + 50));           // we guess size

        data << uint64(guid);
        data << uint8(sendLootType);
        data << LootView(*loot, this, permission);

        SendDirectMessage(&data);

        // add 'this' player as one of the players that are looting 'loot'
        loot->AddLooter(GetGUID());

        if (loot_type == LOOT_CORPSE && !guid.IsItem())
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
    }
    else
        SendLootError(GetLootGUID(), LOOT_ERROR_DIDNT_KILL);
}

void Player::SendNotifyLootMoneyRemoved()
{
    WorldPacket data(SMSG_LOOT_CLEAR_MONEY, 0);
    SendDirectMessage(&data);
}

void Player::SendNotifyLootItemRemoved(uint8 lootSlot)
{
    WorldPacket data(SMSG_LOOT_REMOVED, 1);
    data << uint8(lootSlot);
    SendDirectMessage( &data );
}

void Player::SendUpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
    data << Field;
    data << Value;
    SendDirectMessage(&data);
}

void Player::SendInitWorldStates(uint32 zoneid, uint32 areaid)
{
    // data depends on zoneid/mapid...
    Battleground* bg = GetBattleground();
    uint32 mapid = GetMapId();
#ifdef LICH_KING
    InstanceScript* instance = GetInstanceScript();
    Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(zoneid);
#endif

    WorldPacket data(SMSG_INIT_WORLD_STATES, (4+4+4+2+(12*8)));
    data << uint32(mapid);                                  // mapid
    data << uint32(zoneid);                                 // zone id
    data << uint32(areaid);                                 // area id, new 2.1.0
    size_t countPos = data.wpos();
    data << uint16(0);                                      // count of uint64 blocks
    //from mac leak : next fields are called ClientWorldStateInfo
    /*data << uint32(0x8d8) << uint32(0x0);                   // 1
    data << uint32(0x8d7) << uint32(0x0);                   // 2
    data << uint32(0x8d6) << uint32(0x0);                   // 3
    data << uint32(0x8d5) << uint32(0x0);                   // 4
    data << uint32(0x8d4) << uint32(0x0);                   // 5
    data << uint32(0x8d3) << uint32(0x0);                   // 6*/
#ifdef LICH_KING
                                                            // 7 1 - Arena season in progress, 0 - end of season
    data << uint32(0xC77) << uint32(sWorld->getBoolConfig(CONFIG_ARENA_SEASON_IN_PROGRESS));
    // 8 Arena season id
    data << uint32(0xF3D) << uint32(sWorld->getIntConfig(CONFIG_ARENA_SEASON_ID));
#else
    data << uint32(3191) << uint32(sWorld->getConfig(CONFIG_ARENA_SEASON));
#endif

    switch(zoneid)
    {
        case 139:                                           // Eastern Plaguelands
        case 3483:                                          // Hellfire Peninsula
        case 3521:                                          // Zangarmarsh
        case 3519:                                          // Terokkar Forest
        case 3518:                                          // Nagrand
        case 1377:                                          // Silithus
            if(OutdoorPvP * pvp = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(zoneid))
                pvp->FillInitialWorldStates(data);

            // dunno about these... aq opening event maybe?
            /* for 1377:
            data << uint32(2322) << uint32(0x0); // 10 sandworm N
            data << uint32(2323) << uint32(0x0); // 11 sandworm S
            data << uint32(2324) << uint32(0x0); // 12 sandworm SW
            data << uint32(2325) << uint32(0x0); // 13 sandworm E
            */
            break;
        case 2597:                                          // AV
            if (bg && bg->GetTypeID() == BATTLEGROUND_AV)
                bg->FillInitialWorldStates(data);
            break;
        case 3277:                                          // WS
            if (bg && bg->GetTypeID() == BATTLEGROUND_WS)
                bg->FillInitialWorldStates(data);
            break;
        case 3358:                                          // AB
            if (bg && bg->GetTypeID() == BATTLEGROUND_AB)
                bg->FillInitialWorldStates(data);
            break;
        case 3820:                                          // EY
            if (bg && bg->GetTypeID() == BATTLEGROUND_EY)
                bg->FillInitialWorldStates(data);
            break;
        case 3698:                                          // Nagrand Arena
            if (bg && bg->GetTypeID() == BATTLEGROUND_NA)
                bg->FillInitialWorldStates(data);
            break;
        case 3702:                                          // Blade's Edge Arena
            if (bg && bg->GetTypeID() == BATTLEGROUND_BE)
                bg->FillInitialWorldStates(data);
            break;
        case 3968:                                          // Ruins of Lordaeron
            if (bg && bg->GetTypeID() == BATTLEGROUND_RL)
                bg->FillInitialWorldStates(data);
            break;
    }

    /*mangos if (InstanceData* instanceData = GetMap()->GetInstanceData())
        instanceData->FillInitialWorldStates(data, count, zoneid, areaid);*/

    uint16 length = (data.wpos() - countPos) / 8;
    data.put<uint16>(countPos, length);

    SendDirectMessage(&data);
#ifdef LICH_KING
    SendBGWeekendWorldStates();
    SendBattlefieldWorldStates();
#endif
}

uint32 Player::GetXPRestBonus(uint32 xp)
{
    uint32 rested_bonus = (uint32)GetRestBonus();           // xp for each rested bonus

    if(rested_bonus > xp)                                   // max rested_bonus == xp or (r+x) = 200% xp
        rested_bonus = xp;

    SetRestBonus(GetRestBonus() - rested_bonus);

    TC_LOG_DEBUG("entities.player","Player gain %u xp (+ %u Rested Bonus). Rested points=%f",xp+rested_bonus,rested_bonus,GetRestBonus());
    return rested_bonus;
}

void Player::SetBindPoint(ObjectGuid guid)
{
    WorldPacket data(SMSG_BINDER_CONFIRM, 8);
    data << uint64(guid);
    SendDirectMessage( &data );
}

void Player::SendTalentWipeConfirm(ObjectGuid guid)
{
    WorldPacket data(MSG_TALENT_WIPE_CONFIRM, (8+4));
    data << uint64(guid);
    uint32 cost = sWorld->getConfig(CONFIG_NO_RESET_TALENT_COST) ? 0 : ResetTalentsCost();
    data << cost;
    SendDirectMessage( &data );
}

void Player::SendPetSkillWipeConfirm()
{
    Pet* pet = GetPet();
    if(!pet)
        return;
    WorldPacket data(SMSG_PET_UNLEARN_CONFIRM, (8+4));
    data << pet->GetGUID();
    data << uint32(pet->ResetTalentsCost());
    SendDirectMessage( &data );
}

/*********************************************************/
/***                    STORAGE SYSTEM                 ***/
/*********************************************************/

void Player::SetVirtualItemSlot( uint8 i, Item* item)
{
    assert(i < 3);
    if (i < 2 && item)
    {
        if(!item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
            return;
        uint32 charges = item->GetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT);
        if (charges == 0)
            return;
        if (charges > 1)
            item->SetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT,charges-1);
        else if (charges <= 1)
        {
            ApplyEnchantment(item,TEMP_ENCHANTMENT_SLOT,false);
            item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
        }
    }
}

void Player::SetSheath( SheathState sheathed )
{
    switch (sheathed)
    {
        case SHEATH_STATE_UNARMED:                          // no prepared weapon
            SetVirtualItemSlot(0,nullptr);
            SetVirtualItemSlot(1,nullptr);
            SetVirtualItemSlot(2,nullptr);
            break;
        case SHEATH_STATE_MELEE:                            // prepared melee weapon
        {
            SetVirtualItemSlot(0,GetWeaponForAttack(BASE_ATTACK,true));
            SetVirtualItemSlot(1,GetWeaponForAttack(OFF_ATTACK,true));
            SetVirtualItemSlot(2,nullptr);
        };  break;
        case SHEATH_STATE_RANGED:                           // prepared ranged weapon
            SetVirtualItemSlot(0,nullptr);
            SetVirtualItemSlot(1,nullptr);
            SetVirtualItemSlot(2,GetWeaponForAttack(RANGED_ATTACK,true));
            break;
        default:
            SetVirtualItemSlot(0,nullptr);
            SetVirtualItemSlot(1,nullptr);
            SetVirtualItemSlot(2,nullptr);
            break;
    }
    Unit::SetSheath(sheathed); // this must visualize Sheath changing for other players...
}

uint8 Player::FindEquipSlot( ItemTemplate const* proto, uint32 slot, bool swap ) const
{
    uint8 pClass = GetClass();

    uint8 slots[4];
    slots[0] = NULL_SLOT;
    slots[1] = NULL_SLOT;
    slots[2] = NULL_SLOT;
    slots[3] = NULL_SLOT;
    switch( proto->InventoryType )
    {
        case INVTYPE_HEAD:
            slots[0] = EQUIPMENT_SLOT_HEAD;
            break;
        case INVTYPE_NECK:
            slots[0] = EQUIPMENT_SLOT_NECK;
            break;
        case INVTYPE_SHOULDERS:
            slots[0] = EQUIPMENT_SLOT_SHOULDERS;
            break;
        case INVTYPE_BODY:
            slots[0] = EQUIPMENT_SLOT_BODY;
            break;
        case INVTYPE_CHEST:
            slots[0] = EQUIPMENT_SLOT_CHEST;
            break;
        case INVTYPE_ROBE:
            slots[0] = EQUIPMENT_SLOT_CHEST;
            break;
        case INVTYPE_WAIST:
            slots[0] = EQUIPMENT_SLOT_WAIST;
            break;
        case INVTYPE_LEGS:
            slots[0] = EQUIPMENT_SLOT_LEGS;
            break;
        case INVTYPE_FEET:
            slots[0] = EQUIPMENT_SLOT_FEET;
            break;
        case INVTYPE_WRISTS:
            slots[0] = EQUIPMENT_SLOT_WRISTS;
            break;
        case INVTYPE_HANDS:
            slots[0] = EQUIPMENT_SLOT_HANDS;
            break;
        case INVTYPE_FINGER:
            slots[0] = EQUIPMENT_SLOT_FINGER1;
            slots[1] = EQUIPMENT_SLOT_FINGER2;
            break;
        case INVTYPE_TRINKET:
            slots[0] = EQUIPMENT_SLOT_TRINKET1;
            slots[1] = EQUIPMENT_SLOT_TRINKET2;
            break;
        case INVTYPE_CLOAK:
            slots[0] =  EQUIPMENT_SLOT_BACK;
            break;
        case INVTYPE_WEAPON:
        {
            slots[0] = EQUIPMENT_SLOT_MAINHAND;

            // suggest offhand slot only if know dual wielding
            // (this will be replace mainhand weapon at auto equip instead unwonted "you don't known dual wielding" ...
            if(CanDualWield())
                slots[1] = EQUIPMENT_SLOT_OFFHAND;
        }
        break;
        case INVTYPE_SHIELD:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_RANGED:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_2HWEAPON:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            break;
        case INVTYPE_TABARD:
            slots[0] = EQUIPMENT_SLOT_TABARD;
            break;
        case INVTYPE_WEAPONMAINHAND:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            break;
        case INVTYPE_WEAPONOFFHAND:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_HOLDABLE:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_THROWN:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_RANGEDRIGHT:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_BAG:
            slots[0] = INVENTORY_SLOT_BAG_1;
            slots[1] = INVENTORY_SLOT_BAG_2;
            slots[2] = INVENTORY_SLOT_BAG_3;
            slots[3] = INVENTORY_SLOT_BAG_4;
            break;
        case INVTYPE_RELIC:
        {
            switch(proto->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_LIBRAM:
                    if (pClass == CLASS_PALADIN)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_IDOL:
                    if (pClass == CLASS_DRUID)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_TOTEM:
                    if (pClass == CLASS_SHAMAN)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_MISC:
                    if (pClass == CLASS_WARLOCK)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
            }
            break;
        }
        default :
            return NULL_SLOT;
    }

    if( slot != NULL_SLOT )
    {
        if( swap || !GetItemByPos( INVENTORY_SLOT_BAG_0, slot ) )
        {
            for (unsigned char i : slots)
            {
                if ( i == slot )
                    return slot;
            }
        }
    }
    else
    {
        // search free slot at first
        for (unsigned char _slot : slots)
        {
            if (_slot != NULL_SLOT && !GetItemByPos( INVENTORY_SLOT_BAG_0, _slot) )
            {
                // in case 2hand equipped weapon offhand slot empty but not free
                if(_slot == EQUIPMENT_SLOT_OFFHAND)
                {
                    Item* mainItem = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND );
                    if(!mainItem || mainItem->GetTemplate()->InventoryType != INVTYPE_2HWEAPON)
                        return _slot;
                }
                else
                    return _slot;
            }
        }

        // if not found free and can swap return first appropriate from used
        for (unsigned char _slot : slots)
        {
            if (_slot != NULL_SLOT && swap)
                return _slot;
        }
    }

    // no free position
    return NULL_SLOT;
}

InventoryResult Player::CanUnequipItems( uint32 item, uint32 count ) const
{
    Item *pItem;
    uint32 tempcount = 0;

    InventoryResult res = EQUIP_ERR_OK;

    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            InventoryResult ires = CanUnequipItem(INVENTORY_SLOT_BAG_0 << 8 | i, false);
            if(ires==EQUIP_ERR_OK)
            {
                tempcount += pItem->GetCount();
                if (tempcount >= count)
                    return EQUIP_ERR_OK;
            }
            else
                res = ires;
        }
    }
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return EQUIP_ERR_OK;
        }
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return EQUIP_ERR_OK;
        }
    }
    Bag *pBag;
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for(uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                pItem = GetItemByPos( i, j );
                if (pItem && pItem->GetEntry() == item)
                {
                    tempcount += pItem->GetCount();
                    if (tempcount >= count)
                        return EQUIP_ERR_OK;
                }
            }
        }
    }

    // not found req. item count and have unequippable items
    return res;
}

Item* Player::GetFirstItem(uint32 item) const
{
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
            return pItem;
    }
    return nullptr;
}

uint32 Player::GetItemCount( uint32 item, bool inBankAlso, Item* skipItem ) const
{
    uint32 count = 0;
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem != skipItem && pItem->GetEntry() == item)
            count += pItem->GetCount();
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem != skipItem && pItem->GetEntry() == item)
            count += pItem->GetCount();
    }
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
            count += pBag->GetItemCount(item,skipItem);
    }

    if(skipItem && skipItem->GetTemplate()->GemProperties)
    {
        for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
        {
            Item *pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
            if (pItem && pItem != skipItem && pItem->GetTemplate()->Socket[0].Color)
                count += pItem->GetGemCountWithID(item);
        }
    }

    if(inBankAlso)
    {
        for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem != skipItem && pItem->GetEntry() == item)
                count += pItem->GetCount();
        }
        for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pBag)
                count += pBag->GetItemCount(item,skipItem);
        }

        if (skipItem && skipItem->GetTemplate()->GemProperties)
        {
            for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
            {
                Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pItem && pItem != skipItem && pItem->GetTemplate()->Socket[0].Color)
                    count += pItem->GetGemCountWithID(item);
            }
        }
    }

    return count;
}

Item* Player::GetItemByGuid(ObjectGuid guid) const
{
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetGUID() == guid)
            return pItem;
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetGUID() == guid)
            return pItem;
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetGUID() == guid)
                    return pItem;
            }
        }
    }
    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetGUID() == guid)
                    return pItem;
            }
        }
    }

    return nullptr;
}

Item* Player::GetItemByPos( uint16 pos ) const
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    return GetItemByPos(bag, slot);
}

Item* Player::GetItemByPos(uint8 bag, uint8 slot) const
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot < BANK_SLOT_BAG_END || (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END)))
        return m_items[slot];
    else if ((bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
        || (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END))
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
            return pBag->GetItemByPos(slot);
    }
    return nullptr;
}

//Does additional check for disarmed weapons
Item* Player::GetUseableItemByPos(uint8 bag, uint8 slot) const
{
    if (!CanUseAttackType(GetAttackBySlot(slot)))
        return nullptr;
    return GetItemByPos(bag, slot);
}

Bag* Player::GetBagByPos(uint8 bag) const
{
    if ((bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
        || (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END))
        if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
            return item->ToBag();
    return nullptr;
}

Item* Player::GetWeaponForAttack(WeaponAttackType attackType, bool useable) const
{
    uint16 slot;
    switch (attackType)
    {
        case BASE_ATTACK:   slot = EQUIPMENT_SLOT_MAINHAND; break;
        case OFF_ATTACK:    slot = EQUIPMENT_SLOT_OFFHAND;  break;
        case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED;   break;
        default: return nullptr;
    }

    Item* item;
    if (useable)
        item = GetUseableItemByPos(INVENTORY_SLOT_BAG_0, slot);
    else
        item = GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!item || item->GetTemplate()->Class != ITEM_CLASS_WEAPON)
        return nullptr;

    if(!useable)
        return item;

    if( item->IsBroken() || !IsUseEquipedWeapon(attackType==BASE_ATTACK) ) //TC IsInFeralForm())
        return nullptr;

    return item;
}

bool Player::HasMainWeapon() const
{
    return bool(GetWeaponForAttack(BASE_ATTACK, true));
}

Item* Player::GetShield(bool useable) const
{
    Item* item;
    if (useable)
        item = GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    else
        item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

    if (!item || item->GetTemplate()->Class != ITEM_CLASS_ARMOR)
        return nullptr;

    if(!useable)
        return item;

    if( item->IsBroken())
        return nullptr;

    return item;
}

WeaponAttackType Player::GetAttackBySlot( uint8 slot )
{
    switch(slot)
    {
        case EQUIPMENT_SLOT_MAINHAND: return BASE_ATTACK;
        case EQUIPMENT_SLOT_OFFHAND:  return OFF_ATTACK;
        case EQUIPMENT_SLOT_RANGED:   return RANGED_ATTACK;
        default:                      return MAX_ATTACK;
    }
}

bool Player::HasBankBagSlot( uint8 slot ) const
{
    uint32 maxslot = GetByteValue(PLAYER_BYTES_2, 2) + BANK_SLOT_BAG_START;
    if( slot < maxslot )
        return true;
    return false;
}

bool Player::IsInventoryPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && slot == NULL_SLOT)
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END))
        return true;
    if (bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END))
        return true;
    return false;
}

bool Player::IsEquipmentPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot < EQUIPMENT_SLOT_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END))
        return true;
    return false;
}

bool Player::IsBankPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
        return true;
    if (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END)
        return true;
    return false;
}

bool Player::IsBagPos(uint16 pos)
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
        return true;
    return false;
}

bool Player::IsValidPos(uint8 bag, uint8 slot) const
{
    // post selected
    if(bag == NULL_BAG)
        return true;

    if (bag == INVENTORY_SLOT_BAG_0)
    {
        // any post selected
        if (slot == NULL_SLOT)
            return true;

        // equipment
        if (slot < EQUIPMENT_SLOT_END)
            return true;

        // bag equip slots
        if (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END)
            return true;

        // backpack slots
        if (slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END)
            return true;

        // keyring slots
        if (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END)
            return true;

        // bank main slots
        if (slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END)
            return true;

        // bank bag slots
        if (slot < BANK_SLOT_BAG_END)
            return true;

        return false;
    }

    // bag content slots
    if (bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
    {
        Bag* pBag = (Bag*)GetItemByPos (INVENTORY_SLOT_BAG_0, bag);
        if(!pBag)
            return false;

        // any post selected
        if (slot == NULL_SLOT)
            return true;

        return slot < pBag->GetBagSize();
    }

    // bank bag content slots
    if (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END)
    {
        Bag* pBag = (Bag*)GetItemByPos (INVENTORY_SLOT_BAG_0, bag);
        if(!pBag)
            return false;

        // any post selected
        if (slot == NULL_SLOT)
            return true;

        return slot < pBag->GetBagSize();
    }

    // where this?
    return false;
}


bool Player::HasItemCount(uint32 item, uint32 count, bool inBankAlso) const
{
    if (count == 0)
        return true;

    uint32 tempcount = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return true;
        }
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return true;
        }
    }
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = GetItemByPos(i, j);
                if (pItem && pItem->GetEntry() == item)
                {
                    tempcount += pItem->GetCount();
                    if (tempcount >= count)
                        return true;
                }
            }
        }
    }

    if (inBankAlso)
    {
        for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem->GetEntry() == item)
            {
                tempcount += pItem->GetCount();
                if (tempcount >= count)
                    return true;
            }
        }
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    Item* pItem = GetItemByPos(i, j);
                    if (pItem && pItem->GetEntry() == item)
                    {
                        tempcount += pItem->GetCount();
                        if (tempcount >= count)
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

uint32 Player::GetEmptyBagSlotsCount() const
{
    uint32 freeSlots = 0;

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++) 
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!pItem)
            ++freeSlots;
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++) 
    {
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i)) 
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++) 
            {
                Item* pItem = GetItemByPos(i, j);
                if (!pItem)
                    ++freeSlots;
            }
        }
    }

    return freeSlots;
}


InventoryResult Player::CanTakeMoreSimilarItems(Item* pItem, uint32* itemLimitCategory /*= nullptr*/) const
{
    return _CanTakeMoreSimilarItems(pItem->GetEntry(), pItem->GetCount(), pItem, nullptr, itemLimitCategory);
}

Item* Player::GetItemOrItemWithGemEquipped(uint32 item) const
{
    Item *pItem;
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
            return pItem;
    }

    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(item);
    if (pProto && pProto->GemProperties)
    {
        for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        {
            pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem->GetTemplate()->Socket[0].Color)
            {
                if (pItem->GetGemCountWithID(item) > 0)
                    return pItem;
            }
        }
    }

    return nullptr;
}

//itemLimitCategory unused on BC
InventoryResult Player::_CanTakeMoreSimilarItems(uint32 entry, uint32 count, Item* pItem, uint32* no_space_count, uint32* /*itemLimitCategory = nullptr*/) const
{
    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(entry);
    if (!pProto)
    {
        if(no_space_count)
            *no_space_count = count;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    if (pItem && pItem->m_lootGenerated)
        return EQUIP_ERR_ALREADY_LOOTED;

    // no maximum
    if(pProto->MaxCount == 0)
        return EQUIP_ERR_OK;

    uint32 curcount = GetItemCount(pProto->ItemId,true,pItem);

    if (curcount + count > pProto->MaxCount)
    {
        if(no_space_count)
            *no_space_count = count +curcount - pProto->MaxCount;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    return EQUIP_ERR_OK;
}

bool Player::HasItemTotemCategory( uint32 TotemCategory ) const
{
    Item *pItem;
    for (uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i) 
    {
        pItem = GetUseableItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && IsTotemCategoryCompatibleWith(pItem->GetTemplate()->TotemCategory,TotemCategory, pItem))
            return true;
    }

    for (uint8 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i) 
    {
        pItem = GetUseableItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && IsTotemCategoryCompatibleWith(pItem->GetTemplate()->TotemCategory,TotemCategory, pItem))
            return true;
    }

    for(uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i) 
    {
        if (Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j) 
            {
                pItem = GetUseableItemByPos(i, j);
                if (pItem && IsTotemCategoryCompatibleWith(pItem->GetTemplate()->TotemCategory,TotemCategory, pItem))
                    return true;
            }
        }
    }
    return false;
}

InventoryResult Player::_CanStoreItem_InSpecificSlot( uint8 bag, uint8 slot, ItemPosCountVec &dest, ItemTemplate const *pProto, uint32& count, bool swap, Item* pSrcItem ) const
{
    Item* pItem2 = GetItemByPos( bag, slot );

    // ignore move item (this slot will be empty at move)
    if(pItem2==pSrcItem)
        pItem2 = nullptr;

    uint32 need_space;

    // empty specific slot - check item fit to slot
    if( !pItem2 || swap )
    {
        if( bag == INVENTORY_SLOT_BAG_0 )
        {
            // keyring case
            if(slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_START+GetMaxKeyringSize() && !(pProto->BagFamily & BAG_FAMILY_MASK_KEYS))
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            // prevent cheating
            if((slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END) || slot >= PLAYER_SLOT_END)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
        }
        else
        {
            Bag* pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, bag );
            if( !pBag )
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            ItemTemplate const* pBagProto = pBag->GetTemplate();
            if( !pBagProto )
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            if( !ItemCanGoIntoBag(pProto,pBagProto) )
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
        }

        // non empty stack with space
        need_space = pProto->Stackable;
    }
    // non empty slot, check item type
    else
    {
        // check item type
        if(pItem2->GetEntry() != pProto->ItemId)
            return EQUIP_ERR_ITEM_CANT_STACK;

        // check free space
        if(pItem2->GetCount() >= pProto->Stackable)
            return EQUIP_ERR_ITEM_CANT_STACK;

        need_space = pProto->Stackable - pItem2->GetCount();
    }

    if(need_space > count)
        need_space = count;

    ItemPosCount newPosition = ItemPosCount((bag << 8) | slot, need_space);
    if(!newPosition.isContainedIn(dest))
    {
        dest.push_back(newPosition);
        count -= need_space;
    }
    return EQUIP_ERR_OK;
}

InventoryResult Player::_CanStoreItem_InBag(uint8 bag, ItemPosCountVec &dest, ItemTemplate const *pProto, uint32& count, bool merge, bool non_specialized, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const
{
    // skip specific bag already processed in first called _CanStoreItem_InBag
    if (bag == skip_bag)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
    if (!pBag)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    ItemTemplate const* pBagProto = pBag->GetTemplate();
    if (!pBagProto)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    // specialized bag mode or non-specilized
    if (non_specialized != (pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass == ITEM_SUBCLASS_CONTAINER))
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    if (!ItemCanGoIntoBag(pProto, pBagProto))
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    for (uint32 j = 0; j < pBag->GetBagSize(); j++)
    {
        // skip specific slot already processed in first called _CanStoreItem_InSpecificSlot
        if (j == skip_slot)
            continue;

        Item* pItem2 = GetItemByPos(bag, j);

        // ignore move item (this slot will be empty at move)
        if (pItem2 == pSrcItem)
            pItem2 = nullptr;

        // if merge skip empty, if !merge skip non-empty
        if ((pItem2 != nullptr) != merge)
            continue;

        if (pItem2)
        {
            if (pItem2->GetEntry() == pProto->ItemId && pItem2->GetCount() < pProto->Stackable)
            {
                uint32 need_space = pProto->Stackable - pItem2->GetCount();
                if (need_space > count)
                    need_space = count;

                ItemPosCount newPosition = ItemPosCount((bag << 8) | j, need_space);
                if (!newPosition.isContainedIn(dest))
                {
                    dest.push_back(newPosition);
                    count -= need_space;

                    if (count == 0)
                        return EQUIP_ERR_OK;
                }
            }
        }
        else
        {
            uint32 need_space = pProto->Stackable;
            if (need_space > count)
                need_space = count;

            ItemPosCount newPosition = ItemPosCount((bag << 8) | j, need_space);
            if (!newPosition.isContainedIn(dest))
            {
                dest.push_back(newPosition);
                count -= need_space;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
        }
    }
    return EQUIP_ERR_OK;
}

InventoryResult Player::_CanStoreItem_InInventorySlots(uint8 slot_begin, uint8 slot_end, ItemPosCountVec &dest, ItemTemplate const *pProto, uint32& count, bool merge, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const
{
    for (uint32 j = slot_begin; j < slot_end; j++)
    {
        // skip specific slot already processed in first called _CanStoreItem_InSpecificSlot
        if (INVENTORY_SLOT_BAG_0 == skip_bag && j == skip_slot)
            continue;

        Item* pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, j);

        // ignore move item (this slot will be empty at move)
        if (pItem2 == pSrcItem)
            pItem2 = nullptr;

        // if merge skip empty, if !merge skip non-empty
        if ((pItem2 != nullptr) != merge)
            continue;

        if (pItem2)
        {
            if (pItem2->GetEntry() == pProto->ItemId && pItem2->GetCount() < pProto->Stackable)
            {
                uint32 need_space = pProto->Stackable - pItem2->GetCount();
                if (need_space > count)
                    need_space = count;
                ItemPosCount newPosition = ItemPosCount((INVENTORY_SLOT_BAG_0 << 8) | j, need_space);
                if (!newPosition.isContainedIn(dest))
                {
                    dest.push_back(newPosition);
                    count -= need_space;

                    if (count == 0)
                        return EQUIP_ERR_OK;
                }
            }
        }
        else
        {
            uint32 need_space = pProto->Stackable;
            if (need_space > count)
                need_space = count;

            ItemPosCount newPosition = ItemPosCount((INVENTORY_SLOT_BAG_0 << 8) | j, need_space);
            if (!newPosition.isContainedIn(dest))
            {
                dest.push_back(newPosition);
                count -= need_space;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
        }
    }
    return EQUIP_ERR_OK;
}

InventoryResult Player::_CanStoreItem(uint8 bag, uint8 slot, ItemPosCountVec &dest, uint32 entry, uint32 count, Item *pItem, bool swap, uint32* no_space_count, ItemTemplate const* pProto) const
{
    if (pItem && !pProto)
        pProto = pItem->GetTemplate();
    if (!pProto)
        pProto = sObjectMgr->GetItemTemplate(entry);
    if (!pProto)
    {
        if (no_space_count)
            *no_space_count = count;
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;
    }

    if (pItem && pItem->IsBindedNotWith(this))
    {
        if (no_space_count)
            *no_space_count = count;
        return EQUIP_ERR_DONT_OWN_THAT_ITEM;
    }

    // Healthstones check
    /*static uint32 const itypes[6][3] = {
    { 5512,19004,19005},                        // Minor Healthstone
    { 5511,19006,19007},                        // Lesser Healthstone
    { 5509,19008,19009},                        // Healthstone
    { 5510,19010,19011},                        // Greater Healthstone
    { 9421,19012,19013},                        // Major Healthstone
    {22103,22104,22105}                         // Master Healthstone
    };
    bool isHealthstone = false;
    for (uint8 i = 0; i < 6 && !isHealthstone; i++) {
    for (uint8 j = 0; j < 3 && !isHealthstone; j++) {
    if (itypes[i][j] == entry)
    isHealthstone = true;
    }
    }
    if (isHealthstone) {
    for (uint8 i = 0; i < 6; i++) {
    for (uint8 j = 0; j < 3; j++) {
    if (HasItemCount(itypes[i][j], 1, true))
    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }
    }
    }*/

    // check count of items (skip for auto move for same player from bank)
    uint32 no_similar_count = 0;                            // can't store this amount similar items
    InventoryResult res = _CanTakeMoreSimilarItems(entry, count, pItem, &no_similar_count);
    if (res != EQUIP_ERR_OK)
    {
        if (count == no_similar_count)
        {
            if (no_space_count)
                *no_space_count = no_similar_count;
            return res;
        }
        count -= no_similar_count;
    }

    // in specific slot
    if (bag != NULL_BAG && slot != NULL_SLOT)
    {
        if (!IsValidPos(bag, slot))
            return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;
        res = _CanStoreItem_InSpecificSlot(bag, slot, dest, pProto, count, swap, pItem);
        if (res != EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

    // not specific slot or have space for partly store only in specific slot

    // in specific bag
    if (bag != NULL_BAG)
    {
        // search stack in bag for merge to
        if (pProto->Stackable > 1)
        {
            if (bag == INVENTORY_SLOT_BAG_0)               // inventory
            {
                res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_END, dest, pProto, count, true, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }

                res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
            else                                            // equipped bag
            {
                // we need check 2 time (specialized/non_specialized), use NULL_BAG to prevent skipping bag
                res = _CanStoreItem_InBag(bag, dest, pProto, count, true, false, pItem, NULL_BAG, slot);
                if (res != EQUIP_ERR_OK)
                    res = _CanStoreItem_InBag(bag, dest, pProto, count, true, true, pItem, NULL_BAG, slot);

                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
        }

        // search free slot in bag for place to
        if (bag == INVENTORY_SLOT_BAG_0)                   // inventory
        {
            // search free slot - keyring case
            if (pProto->BagFamily & BAG_FAMILY_MASK_KEYS)
            {
                uint32 keyringSize = GetMaxKeyringSize();
                res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_START + keyringSize, dest, pProto, count, false, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }

            res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
        else                                                // equipped bag
        {
            res = _CanStoreItem_InBag(bag, dest, pProto, count, false, false, pItem, NULL_BAG, slot);
            if (res != EQUIP_ERR_OK)
                res = _CanStoreItem_InBag(bag, dest, pProto, count, false, true, pItem, NULL_BAG, slot);

            if (res != EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // not specific bag or have space for partly store only in specific bag

    // search stack for merge to
    if (pProto->Stackable > 1)
    {
        res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_END, dest, pProto, count, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }

        res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }

        if (pProto->BagFamily)
        {
            for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            {
                res = _CanStoreItem_InBag(i, dest, pProto, count, true, false, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                    continue;

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, true, true, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // search free slot - special bag case
    if (pProto->BagFamily)
    {
        if (pProto->BagFamily & BAG_FAMILY_MASK_KEYS)
        {
            uint32 keyringSize = GetMaxKeyringSize();
            res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_START + keyringSize, dest, pProto, count, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, false, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // search free slot
    res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
    if (res != EQUIP_ERR_OK)
    {
        if (no_space_count)
            *no_space_count = count + no_similar_count;
        return res;
    }

    if (count == 0)
    {
        if (no_similar_count == 0)
            return EQUIP_ERR_OK;

        if (no_space_count)
            *no_space_count = count + no_similar_count;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        res = _CanStoreItem_InBag(i, dest, pProto, count, false, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
            continue;

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

    if (no_space_count)
        *no_space_count = count + no_similar_count;

    return EQUIP_ERR_INVENTORY_FULL;
}

Item* Player::AddItem(uint32 itemId, uint32 count)
{
    uint32 noSpaceForCount = 0;
    ItemPosCountVec dest;
    InventoryResult msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK)
        count -= noSpaceForCount;

    if (count == 0 || dest.empty())
    {
        /// @todo Send to mailbox if no space
        ChatHandler(GetSession()).PSendSysMessage("You don't have any space in your bags.");
        return nullptr;
    }

    Item* item = StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
    if (item)
        SendNewItem(item, count, true, false);
    else
        return nullptr;

    return item;
}

//////////////////////////////////////////////////////////////////////////
InventoryResult Player::CanStoreItems(std::vector<Item*> const& items, uint32 count, uint32* itemLimitCategory) const
{
    Item* pItem2;

    // fill space table
    int inv_slot_items[INVENTORY_SLOT_ITEM_END-INVENTORY_SLOT_ITEM_START];
    int inv_bags[INVENTORY_SLOT_BAG_END-INVENTORY_SLOT_BAG_START][MAX_BAG_SIZE];
    int inv_keys[KEYRING_SLOT_END-KEYRING_SLOT_START];

    memset(inv_slot_items,0,sizeof(int)*(INVENTORY_SLOT_ITEM_END-INVENTORY_SLOT_ITEM_START));
    memset(inv_bags,0,sizeof(int)*(INVENTORY_SLOT_BAG_END-INVENTORY_SLOT_BAG_START)*MAX_BAG_SIZE);
    memset(inv_keys,0,sizeof(int)*(KEYRING_SLOT_END-KEYRING_SLOT_START));

    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );

        if (pItem2 && !pItem2->IsInTrade())
        {
            inv_slot_items[i-INVENTORY_SLOT_ITEM_START] = pItem2->GetCount();
        }
    }

    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );

        if (pItem2 && !pItem2->IsInTrade())
        {
            inv_keys[i-KEYRING_SLOT_START] = pItem2->GetCount();
        }
    }

    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(Bag* pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
        {
            for(uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                pItem2 = GetItemByPos( i, j );
                if (pItem2 && !pItem2->IsInTrade())
                {
                    inv_bags[i-INVENTORY_SLOT_BAG_START][j] = pItem2->GetCount();
                }
            }
        }
    }

    // check free space for all items
    for (int k=0;k<count;k++)
    {
        Item * pItem = items[k];

        // no item
        if (!pItem)  continue;

        ItemTemplate const *pProto = pItem->GetTemplate();

        // strange item
        if( !pProto )
            return EQUIP_ERR_ITEM_NOT_FOUND;

        // item it 'bind'
        if(pItem->IsBindedNotWith(this))
            return EQUIP_ERR_DONT_OWN_THAT_ITEM;

        Bag *pBag;
        ItemTemplate const *pBagProto;

        // item is 'one item only'
        InventoryResult res = CanTakeMoreSimilarItems(pItem, itemLimitCategory);
        if(res != EQUIP_ERR_OK)
            return res;

        // search stack for merge to
        if( pProto->Stackable > 1 )
        {
            bool b_found = false;

            for(int t = KEYRING_SLOT_START; t < KEYRING_SLOT_END; t++)
            {
                pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, t );
                if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_keys[t-KEYRING_SLOT_START] + pItem->GetCount() <= pProto->Stackable )
                {
                    inv_keys[t-KEYRING_SLOT_START] += pItem->GetCount();
                    b_found = true;
                    break;
                }
            }
            if (b_found) continue;

            for(int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; t++)
            {
                pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, t );
                if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_slot_items[t-INVENTORY_SLOT_ITEM_START] + pItem->GetCount() <= pProto->Stackable )
                {
                    inv_slot_items[t-INVENTORY_SLOT_ITEM_START] += pItem->GetCount();
                    b_found = true;
                    break;
                }
            }
            if (b_found) continue;

            for(int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
            {
                pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, t );
                if( pBag )
                {
                    for(uint32 j = 0; j < pBag->GetBagSize(); j++)
                    {
                        pItem2 = GetItemByPos( t, j );
                        if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_bags[t-INVENTORY_SLOT_BAG_START][j] + pItem->GetCount() <= pProto->Stackable )
                        {
                            inv_bags[t-INVENTORY_SLOT_BAG_START][j] += pItem->GetCount();
                            b_found = true;
                            break;
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // special bag case
        if( pProto->BagFamily )
        {
            bool b_found = false;
            if(pProto->BagFamily & BAG_FAMILY_MASK_KEYS)
            {
                uint32 keyringSize = GetMaxKeyringSize();
                for(uint32 t = KEYRING_SLOT_START; t < KEYRING_SLOT_START+keyringSize; ++t)
                {
                    if( inv_keys[t-KEYRING_SLOT_START] == 0 )
                    {
                        inv_keys[t-KEYRING_SLOT_START] = 1;
                        b_found = true;
                        break;
                    }
                }
            }

            if (b_found) continue;

            for(int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
            {
                pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, t );
                if( pBag )
                {
                    pBagProto = pBag->GetTemplate();

                    // not plain container check
                    if( pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER) &&
                        ItemCanGoIntoBag(pProto,pBagProto) )
                    {
                        for(uint32 j = 0; j < pBag->GetBagSize(); j++)
                        {
                            if( inv_bags[t-INVENTORY_SLOT_BAG_START][j] == 0 )
                            {
                                inv_bags[t-INVENTORY_SLOT_BAG_START][j] = 1;
                                b_found = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // search free slot
        bool b_found = false;
        for(int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; t++)
        {
            if( inv_slot_items[t-INVENTORY_SLOT_ITEM_START] == 0 )
            {
                inv_slot_items[t-INVENTORY_SLOT_ITEM_START] = 1;
                b_found = true;
                break;
            }
        }
        if (b_found) continue;

        // search free slot in bags
        for(int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
        {
            pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, t );
            if( pBag )
            {
                pBagProto = pBag->GetTemplate();

                // special bag already checked
                if( pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER))
                    continue;

                for(uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    if( inv_bags[t-INVENTORY_SLOT_BAG_START][j] == 0 )
                    {
                        inv_bags[t-INVENTORY_SLOT_BAG_START][j] = 1;
                        b_found = true;
                        break;
                    }
                }
            }
        }

        // no free slot found?
        if (!b_found)
            return EQUIP_ERR_INVENTORY_FULL;
    }

    return EQUIP_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
InventoryResult Player::CanEquipNewItem( uint8 slot, uint16 &dest, uint32 item, bool swap) const
{
    dest = 0;
    Item *pItem = Item::CreateItem(item, 1, this);
    if (pItem)
    {
        InventoryResult result = CanEquipItem(slot, dest, pItem, swap);
        delete pItem;
        return result;
    }

    return EQUIP_ERR_ITEM_NOT_FOUND;
}

InventoryResult Player::CanEquipItem( uint8 slot, uint16 &dest, Item *pItem, bool swap, bool not_loading ) const
{
    dest = 0;
    if( pItem )
    {
        ItemTemplate const *pProto = pItem->GetTemplate();
        if( pProto )
        {
            if(pItem->IsBindedNotWith(this))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            // check count of items (skip for auto move for same player from bank)
            InventoryResult res = CanTakeMoreSimilarItems(pItem);
            if(res != EQUIP_ERR_OK)
                return res;

            // check this only in game
            if(not_loading)
            {
                // May be here should be more stronger checks; STUNNED checked
                // ROOT, CONFUSED, DISTRACTED, FLEEING this needs to be checked.
                if (HasUnitState(UNIT_STATE_STUNNED))
                    return EQUIP_ERR_YOU_ARE_STUNNED;

                if (IsCharmed())
                    return EQUIP_ERR_CANT_DO_RIGHT_NOW; // @todo is this the correct error?

                // do not allow equipping gear except weapons, offhands, projectiles, relics in
                // - combat
                // - in-progress arenas
                if( !pProto->CanChangeEquipStateInCombat() )
                {
                    if( IsInCombat() )
                        return EQUIP_ERR_NOT_IN_COMBAT;

                    if(Battleground* bg = GetBattleground())
                        if( bg->IsArena() && bg->GetStatus() == STATUS_IN_PROGRESS )
                            return EQUIP_ERR_NOT_DURING_ARENA_MATCH;
                }

                if(IsInCombat()&& pProto->Class == ITEM_CLASS_WEAPON && m_weaponChangeTimer != 0)
                    return EQUIP_ERR_CANT_DO_RIGHT_NOW;         // maybe exist better err
#ifdef LICH_KING
                if(IsNonMeleeSpellCast(false))
                    return EQUIP_ERR_CANT_DO_RIGHT_NOW;
#endif
            }

            uint8 eslot = FindEquipSlot( pProto, slot, swap );
            if( eslot == NULL_SLOT )
                return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

            InventoryResult msg = CanUseItem( pItem , not_loading );
            if( msg != EQUIP_ERR_OK )
                return msg;
            if( !swap && GetItemByPos( INVENTORY_SLOT_BAG_0, eslot ) )
                return EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE;

            // check unique-equipped on item
            if (pProto->Flags & ITEM_FLAG_UNIQUE_EQUIPPED)
            {
                // there is an equip limit on this item
                Item* tItem = GetItemOrItemWithGemEquipped(pProto->ItemId);
                if (tItem && (!swap || tItem->GetSlot() != eslot ) )
                    return EQUIP_ERR_ITEM_UNIQUE_EQUIPABLE;
            }

            // check unique-equipped on gems
            for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
            {
                uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if(!enchant_id)
                    continue;
                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if(!enchantEntry)
                    continue;

                ItemTemplate const* pGem = sObjectMgr->GetItemTemplate(enchantEntry->GemID);
                if(pGem && (pGem->Flags & ITEM_FLAG_UNIQUE_EQUIPPED))
                {
                    Item* tItem = GetItemOrItemWithGemEquipped(enchantEntry->GemID);
                    if(tItem && (!swap || tItem->GetSlot() != eslot ))
                        return EQUIP_ERR_ITEM_UNIQUE_EQUIPABLE;
                }
            }

            // check unique-equipped special item classes
            if (pProto->Class == ITEM_CLASS_QUIVER)
            {
                for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                {
                    if( Item* pBag = GetItemByPos( INVENTORY_SLOT_BAG_0, i ) )
                    {
                        if( ItemTemplate const* pBagProto = pBag->GetTemplate() )
                        {
                            if( pBagProto->Class==pProto->Class && (!swap || pBag->GetSlot() != eslot ) )
                            {
                                if(pBagProto->SubClass == ITEM_SUBCLASS_AMMO_POUCH)
                                    return EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH;
                                else
                                    return EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER;
                            }
                        }
                    }
                }
            }

            uint32 type = pProto->InventoryType;

            if(eslot == EQUIPMENT_SLOT_OFFHAND)
            {
                if( type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND )
                {
                    if(!CanDualWield())
                        return EQUIP_ERR_CANT_DUAL_WIELD;
                }

                Item *mainItem = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND );
                if(mainItem)
                {
                    if(mainItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                        return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED;
                }
            }

            // equip two-hand weapon case (with possible unequip 2 items)
            if( type == INVTYPE_2HWEAPON )
            {
                if(eslot != EQUIPMENT_SLOT_MAINHAND)
                    return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

                // offhand item must can be stored in inventory for offhand item and it also must be unequipped
                Item *offItem = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND );
                ItemPosCountVec off_dest;
                if( offItem && (!not_loading ||
                    CanUnequipItem(uint16(INVENTORY_SLOT_BAG_0) << 8 | EQUIPMENT_SLOT_OFFHAND,false) !=  EQUIP_ERR_OK ||
                    CanStoreItem( NULL_BAG, NULL_SLOT, off_dest, offItem, false ) !=  EQUIP_ERR_OK ) )
                    return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_INVENTORY_FULL;
            }
            dest = ((INVENTORY_SLOT_BAG_0 << 8) | eslot);
            return EQUIP_ERR_OK;
        }
    }
    if( !swap )
        return EQUIP_ERR_ITEM_NOT_FOUND;
    else
        return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

InventoryResult Player::CanUnequipItem( uint16 pos, bool swap ) const
{
    // Applied only to equipped items and bank bags
    if(!IsEquipmentPos(pos) && !IsBagPos(pos))
        return EQUIP_ERR_OK;

    Item* pItem = GetItemByPos(pos);

    // Applied only to existed equipped item
    if( !pItem )
        return EQUIP_ERR_OK;

    ItemTemplate const *pProto = pItem->GetTemplate();
    if( !pProto )
        return EQUIP_ERR_ITEM_NOT_FOUND;

    if (IsCharmed())
        return EQUIP_ERR_CANT_DO_RIGHT_NOW; // @todo is this the correct error?

    // do not allow unequipping gear except weapons, offhands, projectiles, relics in
    // - combat
    // - in-progress arenas
    if( !pProto->CanChangeEquipStateInCombat() )
    {
        if( IsInCombat() )
            return EQUIP_ERR_NOT_IN_COMBAT;

        if(Battleground* bg = GetBattleground())
            if( bg->IsArena() && bg->GetStatus() == STATUS_IN_PROGRESS )
                return EQUIP_ERR_NOT_DURING_ARENA_MATCH;
    }

    if(!swap && pItem->IsBag() && !((Bag*)pItem)->IsEmpty())
        return EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS;

    return EQUIP_ERR_OK;
}

InventoryResult Player::CanBankItem( uint8 bag, uint8 slot, ItemPosCountVec &dest, Item *pItem, bool swap, bool not_loading ) const
{
    if (!pItem)
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;

    uint32 count = pItem->GetCount();

    ItemTemplate const* pProto = pItem->GetTemplate();
    if (!pProto)
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;

    if (pItem->IsBindedNotWith(this))
        return EQUIP_ERR_DONT_OWN_THAT_ITEM;

    // check count of items (skip for auto move for same player from bank)
    InventoryResult res = CanTakeMoreSimilarItems(pItem);
    if (res != EQUIP_ERR_OK)
        return res;

    // in specific slot
    if (bag != NULL_BAG && slot != NULL_SLOT)
    {
        if (pProto->InventoryType == INVTYPE_BAG)
        {
            Bag* pBag = static_cast<Bag*>(pItem);
            if (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END)
            {
                if (!HasBankBagSlot(slot))
                    return EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT;
                InventoryResult canUse = CanUseItem(pItem, not_loading);
                if(canUse != EQUIP_ERR_OK)
                    return canUse;
            }
            else
            {
                if (!pBag->IsEmpty())
                    return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
            }
        }
        else
        {
            if( slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END )
                return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }

        res = _CanStoreItem_InSpecificSlot(bag,slot,dest,pProto,count,swap,pItem);
        if (res != EQUIP_ERR_OK)
            return res;

        if (count == 0)
            return EQUIP_ERR_OK;
    }

    // not specific slot or have space for partly store only in specific slot

    // in specific bag
    if (bag != NULL_BAG)
    {
        if (pProto->InventoryType == INVTYPE_BAG)
        {
            if (!pItem->ToBag()->IsEmpty())
                return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
        }

        // search stack in bag for merge to
        if (pProto->Stackable > 1)
        {
            if (bag == INVENTORY_SLOT_BAG_0)
            {
                res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                    return res;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
            else
            {
                res = _CanStoreItem_InBag(bag, dest, pProto, count, true, false, pItem, NULL_BAG, slot);
                if (res != EQUIP_ERR_OK)
                    res = _CanStoreItem_InBag(bag, dest, pProto, count, true, true, pItem, NULL_BAG, slot);

                if (res != EQUIP_ERR_OK)
                    return res;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
        }

        // search free slot in bag
        if (bag == INVENTORY_SLOT_BAG_0)
        {
            res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                return res;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
        else
        {
            res = _CanStoreItem_InBag(bag, dest, pProto, count, false, false, pItem, NULL_BAG, slot);
            if (res != EQUIP_ERR_OK)
                res = _CanStoreItem_InBag(bag, dest, pProto, count, false, true, pItem, NULL_BAG, slot);

            if (res != EQUIP_ERR_OK)
                return res;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }

    // not specific bag or have space for partly store only in specific bag

    // search stack for merge to
    if (pProto->Stackable > 1)
    {
        // in slots
        res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
            return res;

        if (count == 0)
            return EQUIP_ERR_OK;

        // in special bags
        if (pProto->BagFamily)
        {
            for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
            {
                res = _CanStoreItem_InBag(i, dest, pProto, count, true, false, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                    continue;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
        }

        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, true, true, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }

    // search free place in special bag
    if (pProto->BagFamily)
    {
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, false, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }

    // search free space
    res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
    if (res != EQUIP_ERR_OK)
        return res;

    if (count == 0)
        return EQUIP_ERR_OK;

    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        res = _CanStoreItem_InBag(i, dest, pProto, count, false, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
            continue;

        if (count == 0)
            return EQUIP_ERR_OK;
    }

    return EQUIP_ERR_BANK_FULL;
}

InventoryResult Player::CanUseItem( Item *pItem, bool not_loading ) const
{
    if( pItem )
    {
        if( !IsAlive() && not_loading )
            return EQUIP_ERR_YOU_ARE_DEAD;
        //if( isStunned() )
        //    return EQUIP_ERR_YOU_ARE_STUNNED;
        ItemTemplate const *pProto = pItem->GetTemplate();
        if( pProto )
        {
            if (pItem->IsBindedNotWith(this))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;
            if( (pProto->AllowableClass & GetClassMask()) == 0 || (pProto->AllowableRace & GetRaceMask()) == 0 )
                return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
            if( pItem->GetSkill() != 0  )
            {
                if( GetSkillValue( pItem->GetSkill() ) == 0 )
                    return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            }
            if( pProto->RequiredSkill != 0  )
            {
                if( GetSkillValue( pProto->RequiredSkill ) == 0 )
                    return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
                else if( GetSkillValue( pProto->RequiredSkill ) < pProto->RequiredSkillRank )
                    return EQUIP_ERR_ERR_CANT_EQUIP_SKILL;
            }
            if( pProto->RequiredSpell != 0 && !HasSpell( pProto->RequiredSpell ) )
                return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            if( pProto->RequiredReputationFaction && uint32(GetReputationRank(pProto->RequiredReputationFaction)) < pProto->RequiredReputationRank )
                return EQUIP_ERR_CANT_EQUIP_REPUTATION;
            if( GetLevel() < pProto->RequiredLevel )
                return EQUIP_ERR_CANT_EQUIP_LEVEL_I;
            return EQUIP_ERR_OK;
        }
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

bool Player::CanUseItem( ItemTemplate const *pProto )
{
    // Used by group, function NeedBeforeGreed, to know if a prototype can be used by a player

    if( pProto )
    {
        if( (pProto->AllowableClass & GetClassMask()) == 0 || (pProto->AllowableRace & GetRaceMask()) == 0 )
            return false;
        if( pProto->RequiredSkill != 0  )
        {
            if( GetSkillValue( pProto->RequiredSkill ) == 0 )
                return false;
            else if( GetSkillValue( pProto->RequiredSkill ) < pProto->RequiredSkillRank )
                return false;
        }
        if( pProto->RequiredSpell != 0 && !HasSpell( pProto->RequiredSpell ) )
            return false;
        if( GetLevel() < pProto->RequiredLevel )
            return false;
        return true;
    }
    return false;
}

InventoryResult Player::CanUseAmmo( uint32 item ) const
{
    if( !IsAlive() )
        return EQUIP_ERR_YOU_ARE_DEAD;
    //if( isStunned() )
    //    return EQUIP_ERR_YOU_ARE_STUNNED;
    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate( item );
    if( pProto )
    {
        if( pProto->InventoryType!= INVTYPE_AMMO )
            return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
        if( (pProto->AllowableClass & GetClassMask()) == 0 || (pProto->AllowableRace & GetRaceMask()) == 0 )
            return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
        if( pProto->RequiredSkill != 0  )
        {
            if( GetSkillValue( pProto->RequiredSkill ) == 0 )
                return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            else if( GetSkillValue( pProto->RequiredSkill ) < pProto->RequiredSkillRank )
                return EQUIP_ERR_ERR_CANT_EQUIP_SKILL;
        }
        if( pProto->RequiredSpell != 0 && !HasSpell( pProto->RequiredSpell ) )
            return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
        /*if( GetReputation() < pProto->RequiredReputation )
        return EQUIP_ERR_CANT_EQUIP_REPUTATION;
        */
        if( GetLevel() < pProto->RequiredLevel )
            return EQUIP_ERR_CANT_EQUIP_LEVEL_I;

        // Requires No Ammo
        if(GetDummyAura(46699))
            return EQUIP_ERR_BAG_FULL6;

        return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

void Player::SetAmmo( uint32 item )
{
    if(!item)
        return;

    // already set
    if (GetUInt32Value(PLAYER_AMMO_ID) == item)
        return;

    // check ammo
    InventoryResult msg = CanUseAmmo(item);
    if (msg != EQUIP_ERR_OK)
    {
        SendEquipError(msg, nullptr, nullptr, item);
        return;
    }

    SetUInt32Value(PLAYER_AMMO_ID, item);

    _ApplyAmmoBonuses();
}

void Player::RemoveAmmo()
{
    SetUInt32Value(PLAYER_AMMO_ID, 0);

    m_ammoDPS = 0.0f;

    if(CanModifyStats())
        UpdateDamagePhysical(RANGED_ATTACK);
}

//Make sure the player has remaining space before calling this
// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::StoreNewItem(ItemPosCountVec const& dest, uint32 item, bool update, int32 randomPropertyId, GuidSet const& allowedLooters)
{
    uint32 count = 0;
    for(auto itr : dest)
        count += itr.count;

    Item *pItem = Item::CreateItem(item, count, this);
    if (pItem)
    {
        ItemAddedQuestCheck( item, count );
        if(randomPropertyId)
            pItem->SetItemRandomProperties(randomPropertyId);
        pItem = StoreItem( dest, pItem, update );
    }

    if (item == 31088)  // HACK: Tainted Core
        SetRooted(true);

    // If purple equipable item, save inventory immediately
    if (pItem && pItem->GetTemplate()->Quality >= ITEM_QUALITY_EPIC &&
        (pItem->GetTemplate()->Class == ITEM_CLASS_WEAPON || pItem->GetTemplate()->Class == ITEM_CLASS_ARMOR || pItem->GetTemplate()->Class == ITEM_CLASS_MISC)) {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        SaveInventoryAndGoldToDB(trans);
        CharacterDatabase.CommitTransaction(trans);
    }

    return pItem;
}

Item* Player::StoreItem( ItemPosCountVec const& dest, Item* pItem, bool update )
{
    if (!pItem)
        return nullptr;

    Item* lastItem = pItem;

    for(auto itr = dest.begin(); itr != dest.end(); )
    {
        uint16 pos = itr->pos;
        uint32 count = itr->count;

        ++itr;

        if(itr == dest.end())
        {
            lastItem = _StoreItem(pos,pItem,count,false,update);
            break;
        }

        lastItem = _StoreItem(pos,pItem,count,true,update);
    }

    return lastItem;
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::_StoreItem(uint16 pos, Item *pItem, uint32 count, bool clone, bool update)
{
    if( !pItem )
        return nullptr;

    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;

    Item *pItem2 = GetItemByPos( bag, slot );

    if (!pItem2)
    {
        if(clone)
            pItem = pItem->CloneItem(count,this);
        else
            pItem->SetCount(count);

        if(!pItem)
            return nullptr;

        if( pItem->GetTemplate()->Bonding == BIND_WHEN_PICKED_UP ||
            pItem->GetTemplate()->Bonding == BIND_QUEST_ITEM ||
            (pItem->GetTemplate()->Bonding == BIND_WHEN_EQUIPED && IsBagPos(pos)) )
            pItem->SetBinding( true );

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            m_items[slot] = pItem;
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2)), pItem->GetGUID());
            pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
            pItem->SetUInt64Value(ITEM_FIELD_OWNER, GetGUID());

            pItem->SetSlot(slot);
            pItem->SetContainer(nullptr);

            if (IsInWorld() && update)
            {
                pItem->AddToWorld();
                pItem->SendUpdateToPlayer(this);
            }

            pItem->SetState(ITEM_CHANGED, this);
        }
        else
        {
            Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (pBag)
            {
                pBag->StoreItem(slot, pItem, update);
                if (IsInWorld() && update)
                {
                    pItem->AddToWorld();
                    pItem->SendUpdateToPlayer(this);
                }
                pItem->SetState(ITEM_CHANGED, this);
                pBag->SetState(ITEM_CHANGED, this);
            }
        }

        AddEnchantmentDurations(pItem);
        AddItemDurations(pItem);

        return pItem;
    }
    else
    {
        if( pItem2->GetTemplate()->Bonding == BIND_WHEN_PICKED_UP ||
            pItem2->GetTemplate()->Bonding == BIND_QUEST_ITEM ||
            (pItem2->GetTemplate()->Bonding == BIND_WHEN_EQUIPED && IsBagPos(pos)) )
            pItem2->SetBinding(true);

        pItem2->SetCount(pItem2->GetCount() + count);
        if (IsInWorld() && update)
            pItem2->SendUpdateToPlayer(this);

        if (!clone)
        {
            // delete item (it not in any slot currently)
            if (IsInWorld() && update)
            {
                pItem->RemoveFromWorld();
                pItem->DestroyForPlayer(this);
            }

            RemoveEnchantmentDurations(pItem);
            RemoveItemDurations(pItem);

            pItem->SetOwnerGUID(GetGUID());                 // prevent error at next SetState in case trade/mail/buy from vendor
            pItem->SetState(ITEM_REMOVED, this);
        }
        // AddItemDurations(pItem2); - pItem2 already have duration listed for player
        AddEnchantmentDurations(pItem2);

        pItem2->SetState(ITEM_CHANGED, this);

        return pItem2;
    }
}

Item* Player::EquipNewItem(uint16 pos, uint32 item, bool update)
{
    Item *pItem = Item::CreateItem(item, 1, this);
    if (pItem)
    {
        ItemAddedQuestCheck( item, 1 );
        Item * retItem = EquipItem( pos, pItem, update );

        return retItem;
    }
    return nullptr;
}

Item* Player::EquipItem(uint16 pos, Item *pItem, bool update, bool interruptSpells)
{
    if(pItem)
    {
#ifndef LICH_KING
        if (interruptSpells && IsNonMeleeSpellCast(false))
            InterruptNonMeleeSpells(false);
#endif

        AddEnchantmentDurations(pItem);
        AddItemDurations(pItem);

        uint8 bag = pos >> 8;
        uint8 slot = pos & 255;

        Item* pItem2 = GetItemByPos(bag, slot);

        if(!pItem2)
        {
            VisualizeItem(slot, pItem);

            if(IsAlive())
            {
                // item set bonuses applied only at equip and removed at unequip, and still active for broken items
                ItemTemplate const* pProto = ASSERT_NOTNULL(pItem->GetTemplate());
                if(pProto->ItemSet)
                    AddItemsSetItem(this,pItem);

                _ApplyItemMods(pItem, slot, true);

                if(IsInCombat()&& pProto->Class == ITEM_CLASS_WEAPON && m_weaponChangeTimer == 0)
                {
                    uint32 cooldownSpell = SPELL_ID_WEAPON_SWITCH_COOLDOWN_1_5s;

                    if (GetClass() == CLASS_ROGUE)
                        cooldownSpell = SPELL_ID_WEAPON_SWITCH_COOLDOWN_1_0s;

                    SpellInfo const* spellProto = sSpellMgr->GetSpellInfo(cooldownSpell);

                    if (!spellProto)
                        TC_LOG_ERROR("entities.player","Weapon switch cooldown spell %u couldn't be found in Spell.dbc", cooldownSpell);
                    else
                    {
                        m_weaponChangeTimer = spellProto->StartRecoveryTime;

                        WorldPacket data(SMSG_SPELL_COOLDOWN, 8+1+4);
                        data << uint64(GetGUID());
                        data << uint8(1);
                        data << uint32(cooldownSpell);
                        data << uint32(0);
                        SendDirectMessage(&data);
                    }
                }
            }

            if (IsInWorld() && update)
            {
                pItem->AddToWorld();
                pItem->SendUpdateToPlayer( this );
            }

            ApplyEquipCooldown(pItem);

            if( slot == EQUIPMENT_SLOT_MAINHAND )
                UpdateExpertise(BASE_ATTACK);
            else if( slot == EQUIPMENT_SLOT_OFFHAND )
                UpdateExpertise(OFF_ATTACK);

#ifdef LICH_KING
            switch (slot)
            {
            case EQUIPMENT_SLOT_MAINHAND:
            case EQUIPMENT_SLOT_OFFHAND:
            case EQUIPMENT_SLOT_RANGED:
                RecalculateRating(CR_ARMOR_PENETRATION);
            default:
                break;
            }
#endif
        }
        else
        {
            pItem2->SetCount( pItem2->GetCount() + pItem->GetCount() );
            if( IsInWorld() && update )
                pItem2->SendUpdateToPlayer( this );

            // delete item (it not in any slot currently)
            //pItem->DeleteFromDB();
            if( IsInWorld() && update )
            {
                pItem->RemoveFromWorld();
                pItem->DestroyForPlayer( this );
            }

            RemoveEnchantmentDurations(pItem);
            RemoveItemDurations(pItem);

            pItem->SetOwnerGUID(GetGUID());                 // prevent error at next SetState in case trade/mail/buy from vendor
            pItem->SetState(ITEM_REMOVED, this);
            pItem2->SetState(ITEM_CHANGED, this);

            ApplyEquipCooldown(pItem2);

            return pItem2;
        }
    }

#ifdef LICH_KING
    if (slot == EQUIPMENT_SLOT_MAINHAND || slot == EQUIPMENT_SLOT_OFFHAND)
        CheckTitanGripPenalty();

    // only for full equip instead adding to stack
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, pItem->GetEntry());
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, slot, pItem->GetEntry());
#endif

    return pItem;
}

void Player::QuickEquipItem( uint16 pos, Item *pItem)
{
    if( pItem )
    {
        AddEnchantmentDurations(pItem);
        AddItemDurations(pItem);

        uint8 slot = pos & 255;
        VisualizeItem( slot, pItem);

        if( IsInWorld() )
        {
            pItem->AddToWorld();
            pItem->SendUpdateToPlayer( this );
        }
#ifdef LICH_KING
        if (slot == EQUIPMENT_SLOT_MAINHAND || slot == EQUIPMENT_SLOT_OFFHAND)
            CheckTitanGripPenalty();

        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, pItem->GetEntry());
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, slot, pItem->GetEntry());
#endif
    }
}

void Player::SetVisibleItemSlot(uint8 slot, Item* pItem)
{
    // PLAYER_VISIBLE_ITEM_i_CREATOR    // Size: 2
    // PLAYER_VISIBLE_ITEM_i_0          // Size: 12
    //    entry                         //      Size: 1
    //    inspected enchantments        //      Size: 6
    //    ?                             //      Size: 5
    // PLAYER_VISIBLE_ITEM_i_PROPERTIES // Size: 1 (property,suffix factor)
    // PLAYER_VISIBLE_ITEM_i_PAD        // Size: 1
    //                                  //     = 16

    if(pItem)
    {
        SetGuidValue(PLAYER_VISIBLE_ITEM_1_CREATOR + (slot * MAX_VISIBLE_ITEM_OFFSET), pItem->GetGuidValue(ITEM_FIELD_CREATOR));

        int visibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(visibleBase + 0, pItem->GetEntry());

        for(int i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            SetUInt32Value(visibleBase + 1 + i, pItem->GetEnchantmentId(EnchantmentSlot(i)));

        // Use SetInt16Value to prevent set high part to FFFF for negative value
        SetInt16Value( PLAYER_VISIBLE_ITEM_1_PROPERTIES + (slot * MAX_VISIBLE_ITEM_OFFSET), 0, pItem->GetItemRandomPropertyId());
        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (slot * MAX_VISIBLE_ITEM_OFFSET), pItem->GetItemSuffixFactor());
    }
    else
    {
        SetGuidValue(PLAYER_VISIBLE_ITEM_1_CREATOR + (slot * MAX_VISIBLE_ITEM_OFFSET), ObjectGuid::Empty);

        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(VisibleBase + 0, 0);

        for(int i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            SetUInt32Value(VisibleBase + 1 + i, 0);

        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 0 + (slot * MAX_VISIBLE_ITEM_OFFSET), 0);
        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (slot * MAX_VISIBLE_ITEM_OFFSET), 0);
    }
}

void Player::VisualizeItem( uint8 slot, Item *pItem)
{
    if(!pItem)
        return;

    // check also  BIND_WHEN_PICKED_UP and BIND_QUEST_ITEM for .additem or .additemset case by GM (not binded at adding to inventory)
    if(pItem->GetTemplate()->Bonding == BIND_WHEN_EQUIPED || pItem->GetTemplate()->Bonding == BIND_WHEN_PICKED_UP || pItem->GetTemplate()->Bonding == BIND_QUEST_ITEM)
        pItem->SetBinding( true );

    m_items[slot] = pItem;
    SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2)), pItem->GetGUID());
    pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
    pItem->SetUInt64Value(ITEM_FIELD_OWNER, GetGUID());
    pItem->SetSlot(slot);
    pItem->SetContainer(nullptr);

    if(slot < EQUIPMENT_SLOT_END)
        SetVisibleItemSlot(slot,pItem);

    pItem->SetState(ITEM_CHANGED, this);
}

void Player::RemoveItem( uint8 bag, uint8 slot, bool update )
{
    // note: removeitem does not actually change the item
    // it only takes the item out of storage temporarily
    // note2: if removeitem is to be used for delinking
    // the item must be removed from the player's updatequeue

    Item *pItem = GetItemByPos( bag, slot );
    if( pItem )
    {
        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            if (slot < INVENTORY_SLOT_BAG_END)
            {
                // item set bonuses applied only at equip and removed at unequip, and still active for broken items
                ItemTemplate const* pProto = ASSERT_NOTNULL(pItem->GetTemplate());
                if(pProto->ItemSet)
                    RemoveItemsSetItem(this, pProto);

                _ApplyItemMods(pItem, slot, false, update);

                // remove held enchantments
                if ( slot == EQUIPMENT_SLOT_MAINHAND )
                {
                    if (pItem->GetItemSuffixFactor())
                    {
                        pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_3);
                        pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_4);
                    }
                    else
                    {
                        pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_0);
                        pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_1);
                    }
                    UpdateExpertise(BASE_ATTACK);
                }
                else if (slot == EQUIPMENT_SLOT_OFFHAND)
                    UpdateExpertise(OFF_ATTACK);
#ifdef LICH_KING
                // update armor penetration - passive auras may need it
                switch (slot)
                {
                    case EQUIPMENT_SLOT_MAINHAND:
                    case EQUIPMENT_SLOT_OFFHAND:
                    case EQUIPMENT_SLOT_RANGED:
                        RecalculateRating(CR_ARMOR_PENETRATION);
                    default:
                        break;
                }
#endif
            }

            m_items[slot] = nullptr;
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            if (slot < EQUIPMENT_SLOT_END)
                SetVisibleItemSlot(slot,nullptr);
        }
        else
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (pBag)
                pBag->RemoveItem(slot, update);
        }
        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, ObjectGuid::Empty);
        // pItem->SetUInt64Value( ITEM_FIELD_OWNER, 0 ); not clear owner at remove (it will be set at store). This used in mail and auction code
        pItem->SetSlot( NULL_SLOT );
        if (IsInWorld() && update)
            pItem->SendUpdateToPlayer(this);

        if (slot == EQUIPMENT_SLOT_MAINHAND)
            UpdateExpertise(BASE_ATTACK);
        else if (slot == EQUIPMENT_SLOT_OFFHAND)
            UpdateExpertise(OFF_ATTACK);
    }
}

// Common operation need to remove item from inventory without delete in trade, auction, guild bank, mail....
void Player::MoveItemFromInventory(uint8 bag, uint8 slot, bool update)
{
    if(Item* it = GetItemByPos(bag,slot))
    {
        ItemRemovedQuestCheck(it->GetEntry(),it->GetCount());
        RemoveItem( bag,slot,update);
        it->RemoveItemFromUpdateQueueOf(this);
        if(it->IsInWorld())
        {
            it->RemoveFromWorld();
            it->DestroyForPlayer( this );
        }
    }
}

// Common operation need to add item from inventory without delete in trade, guild bank, mail....
void Player::MoveItemToInventory(ItemPosCountVec const& dest, Item* pItem, bool update, bool in_characterInventoryDB)
{
    // update quest counters
    ItemAddedQuestCheck(pItem->GetEntry(),pItem->GetCount());

    // store item
    Item* pLastItem = StoreItem( dest, pItem, update);

    // only set if not merged to existed stack (pItem can be deleted already but we can compare pointers any way)
    if(pLastItem==pItem)
    {
        // update owner for last item (this can be original item with wrong owner
        if(pLastItem->GetOwnerGUID() != GetGUID())
            pLastItem->SetOwnerGUID(GetGUID());

        // if this original item then it need create record in inventory
        // in case trade we already have item in other player inventory
        pLastItem->SetState(in_characterInventoryDB ? ITEM_CHANGED : ITEM_NEW, this);
    }
}

void Player::RemoveItemDependentAurasAndCasts(Item* pItem)
{
    for (AuraMap::iterator itr = m_ownedAuras.begin(); itr != m_ownedAuras.end();)
    {
        Aura* aura = itr->second;

        // skip not self applied auras
        SpellInfo const* spellInfo = aura->GetSpellInfo();
        if (aura->GetCasterGUID() != GetGUID())
        {
            ++itr;
            continue;
        }

        // skip if not item dependent or have alternative item
        if (HasItemFitToSpellRequirements(spellInfo, pItem))
        {
            ++itr;
            continue;
        }

        // no alt item, remove aura, restart check
        RemoveOwnedAura(itr);
    }

    // currently cast spells can be dependent from item
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; ++i)
        if (Spell* spell = GetCurrentSpell(CurrentSpellTypes(i)))
            if (spell->getState() != SPELL_STATE_DELAYED && !HasItemFitToSpellRequirements(spell->m_spellInfo, pItem))
                InterruptSpell(CurrentSpellTypes(i));
}

void Player::DestroyItem(uint8 bag, uint8 slot, bool update)
{
    Item *pItem = GetItemByPos(bag, slot);
    if (pItem)
    {
        // start from destroy contained items (only equipped bag can have its)
        if (pItem->IsBag() && pItem->IsEquipped())          // this also prevent infinity loop if empty bag stored in bag==slot
        {
            for (int i = 0; i < MAX_BAG_SIZE; i++)
                DestroyItem(slot, i, update);
        }


        if (pItem->GetEntry() == 31088)      // HACK: Vashj Tainted Core
            SetRooted(false);

        if(pItem->HasFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_WRAPPED))
            CharacterDatabase.PExecute("DELETE FROM character_gifts WHERE item_guid = '%u'", pItem->GetGUID().GetCounter());

        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        ItemRemovedQuestCheck(pItem->GetEntry(), pItem->GetCount());

        ItemTemplate const* pProto = pItem->GetTemplate();
        if (bag == INVENTORY_SLOT_BAG_0)
        {
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            // equipment and equipped bags can have applied bonuses
            if (slot < INVENTORY_SLOT_BAG_END)
            {
                // item set bonuses applied only at equip and removed at unequip, and still active for broken items
                if(pProto && pProto->ItemSet)
                    RemoveItemsSetItem(this,pProto);

                _ApplyItemMods(pItem, slot, false);
            }

            if (slot < EQUIPMENT_SLOT_END)
            {
                // update expertise and armor penetration - passive auras may need it
#ifdef LICH_KING
                switch (slot)
                {
                case EQUIPMENT_SLOT_MAINHAND:
                case EQUIPMENT_SLOT_OFFHAND:
                case EQUIPMENT_SLOT_RANGED:
                    RecalculateRating(CR_ARMOR_PENETRATION);
                default:
                    break;
                }
#endif

                if (slot == EQUIPMENT_SLOT_MAINHAND)
                    UpdateExpertise(BASE_ATTACK);
                else if (slot == EQUIPMENT_SLOT_OFFHAND)
                    UpdateExpertise(OFF_ATTACK);

                // equipment visual show
                SetVisibleItemSlot(slot,nullptr);
            }

            m_items[slot] = nullptr;
        }
        else if(Bag* pBag = GetBagByPos(bag))
            pBag->RemoveItem(slot, update);

        // Delete rolled money / loot from db.
        // MUST be done before RemoveFromWorld() or GetTemplate() fails
        if (pProto->Flags & ITEM_FLAG_HAS_LOOT)
            sLootItemStorage->RemoveStoredLootForContainer(pItem->GetGUID().GetCounter());

        if (IsInWorld() && update)
        {
            pItem->RemoveFromWorld();
            pItem->DestroyForPlayer(this);
        }

        //pItem->SetOwnerGUID(0);
        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, ObjectGuid::Empty);
        pItem->SetSlot(NULL_SLOT);
        pItem->SetState(ITEM_REMOVED, this);
    }
}

void Player::DestroyItemCount( uint32 item, uint32 count, bool update, bool unequip_check, bool inBankAlso)
{
    Item *pItem;
    uint32 remcount = 0;

    // in inventory
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                // all items in inventory can unequipped
                remcount += pItem->GetCount();
                DestroyItem( INVENTORY_SLOT_BAG_0, i, update);

                if(remcount >=count)
                    return;
            }
            else
            {
                //pProto = pItem->GetTemplate(); //not used
                ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() & update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                // all keys can be unequipped
                remcount += pItem->GetCount();
                DestroyItem( INVENTORY_SLOT_BAG_0, i, update);

                if(remcount >=count)
                    return;
            }
            else
            {
                //pProto = pItem->GetTemplate(); //not used
                ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() & update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }

    // in inventory bags
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if(Bag *pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
        {
            for(uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                pItem = pBag->GetItemByPos(j);
                if( pItem && pItem->GetEntry() == item )
                {
                    // all items in bags can be unequipped
                    if( pItem->GetCount() + remcount <= count )
                    {
                        remcount += pItem->GetCount();
                        DestroyItem( i, j, update );

                        if(remcount >=count)
                            return;
                    }
                    else
                    {
                        //pProto = pItem->GetTemplate(); //not used
                        ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                        pItem->SetCount( pItem->GetCount() - count + remcount );
                        if( IsInWorld() && update )
                            pItem->SendUpdateToPlayer( this );
                        pItem->SetState(ITEM_CHANGED, this);
                        return;
                    }
                }
            }
        }
    }

    // in equipment and bag list
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->GetEntry() == item )
        {
            if( pItem->GetCount() + remcount <= count )
            {
                if(!unequip_check || CanUnequipItem(INVENTORY_SLOT_BAG_0 << 8 | i,false) == EQUIP_ERR_OK )
                {
                    remcount += pItem->GetCount();
                    DestroyItem( INVENTORY_SLOT_BAG_0, i, update);

                    if(remcount >=count)
                        return;
                }
            }
            else
            {
                //pProto = pItem->GetTemplate(); //not used
                ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                pItem->SetCount( pItem->GetCount() - count + remcount );
                if( IsInWorld() & update )
                    pItem->SendUpdateToPlayer( this );
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }

    if(inBankAlso)
    {
        for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            Item* _pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
            if(_pItem && _pItem->GetEntry() == item )
            {
                if(_pItem->GetCount() + remcount <= count )
                {
                    // all items in inventory can unequipped
                    remcount += _pItem->GetCount();
                    DestroyItem( INVENTORY_SLOT_BAG_0, i, update);

                    if(remcount >=count)
                        return;
                }
                else
                {
                    //pProto = pItem->GetTemplate(); //not used
                    ItemRemovedQuestCheck(_pItem->GetEntry(), count - remcount );
                    _pItem->SetCount(_pItem->GetCount() - count + remcount );
                    if( IsInWorld() & update )
                        _pItem->SendUpdateToPlayer( this );
                    _pItem->SetState(ITEM_CHANGED, this);
                    return;
                }
            }
        }
        for(int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if(Bag* pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
            {
                for(uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    pItem = pBag->GetItemByPos(j);
                    if( pItem && pItem->GetEntry() == item )
                    {
                        // all items in bags can be unequipped
                        if( pItem->GetCount() + remcount <= count )
                        {
                            remcount += pItem->GetCount();
                            DestroyItem( i, j, update );

                            if(remcount >=count)
                                return;
                        }
                        else
                        {
                            //pProto = pItem->GetTemplate(); //not used
                            ItemRemovedQuestCheck( pItem->GetEntry(), count - remcount );
                            pItem->SetCount( pItem->GetCount() - count + remcount );
                            if( IsInWorld() && update )
                                pItem->SendUpdateToPlayer( this );
                            pItem->SetState(ITEM_CHANGED, this);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void Player::DestroyZoneLimitedItem( bool update, uint32 new_zone )
{
    // in inventory
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(),new_zone) )
            DestroyItem( INVENTORY_SLOT_BAG_0, i, update);
    }
    for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item* pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(),new_zone) )
            DestroyItem( INVENTORY_SLOT_BAG_0, i, update);
    }

    // in inventory bags
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag* pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if( pBag )
        {
            for(uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if( pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(),new_zone) )
                    DestroyItem( i, j, update);
            }
        }
    }

    // in equipment and bag list
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(), new_zone))
            DestroyItem(INVENTORY_SLOT_BAG_0, i, update);
    }
}

void Player::DestroyConjuredItems(bool update)
{
    // used when entering arena
    // destroys all conjured items
    // in inventory
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->IsConjuredConsumable())
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

    // in inventory bags
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        if(Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for(uint32 j = 0; j < pBag->GetBagSize(); j++)
                if(Item* pItem = pBag->GetItemByPos(j))
                    if (pItem->IsConjuredConsumable())
                        DestroyItem( i, j, update);

    // in equipment and bag list
    for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->IsConjuredConsumable())
                DestroyItem( INVENTORY_SLOT_BAG_0, i, update);
}

void Player::DestroyItemCount(Item* pItem, uint32 &count, bool update)
{
    if(!pItem)
        return;

    TC_LOG_DEBUG("entities.player.items", "STORAGE: DestroyItemCount item (GUID: %u, Entry: %u) count = %u", pItem->GetGUID().GetCounter(), pItem->GetEntry(), count);

    if (pItem->GetCount() <= count)
    {
        count -= pItem->GetCount();

        DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), update);
    }
    else
    {
        ItemRemovedQuestCheck(pItem->GetEntry(), count);
        pItem->SetCount(pItem->GetCount() - count);
        count = 0;
        if (IsInWorld() & update)
            pItem->SendUpdateToPlayer(this);
        pItem->SetState(ITEM_CHANGED, this);
    }
}

void Player::SwapItems(uint32 item1, uint32 item2)
{
    uint32 count = GetItemCount(item1, true);
    if (count != 0) 
    {
        DestroyItemCount(item1, count, true, false, true);
        ItemPosCountVec dest;
        uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item2, count);
        if (msg == EQUIP_ERR_OK)
            StoreNewItem(dest, item2, count, true);
        else 
        {
            if (Item* newItem = Item::CreateItem(item2, count, this)) 
            {
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                newItem->SaveToDB(trans);

                MailSender sender(MAIL_NORMAL, GetGUID().GetCounter(), MAIL_STATIONERY_GM);

                std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                MailDraft(subject, {})
                    .AddItem(newItem)
                    .SendMailTo(trans, MailReceiver(this, GetGUID().GetCounter()), sender, MAIL_CHECK_MASK_COPIED);

                CharacterDatabase.CommitTransaction(trans);
            }
        }
    }
}

void Player::SplitItem( uint16 src, uint16 dst, uint32 count )
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item *pSrcItem = GetItemByPos(srcbag, srcslot);
    if (!pSrcItem)
    {
        SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pSrcItem, nullptr);
        return;
    }

    // not let split all items (can be only at cheating)
    if (pSrcItem->GetCount() == count)
    {
        SendEquipError(EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, nullptr);
        return;
    }

    // not let split more existed items (can be only at cheating)
    if (pSrcItem->GetCount() < count)
    {
        SendEquipError(EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT, pSrcItem, nullptr);
        return;
    }

    if (pSrcItem->m_lootGenerated)                           // prevent split looting item (item
    {
        //best error message found for attempting to split while looting
        SendEquipError(EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, nullptr);
        return;
    }

    Item *pNewItem = pSrcItem->CloneItem(count, this);
    if (!pNewItem)
    {
        SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pSrcItem, nullptr);
        return;
    }

    if (IsInventoryPos(dst))
    {
        // change item amount before check (for unique max count check)
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        ItemPosCountVec dest;
        uint8 msg = CanStoreItem(dstbag, dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, nullptr);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        StoreItem(dest, pNewItem, true);
    }
    else if (IsBankPos(dst))
    {
        // change item amount before check (for unique max count check)
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        ItemPosCountVec dest;
        uint8 msg = CanBankItem(dstbag, dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, nullptr);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        BankItem(dest, pNewItem, true);
    }
    else if (IsEquipmentPos(dst))
    {
        // change item amount before check (for unique max count check), provide space for splitted items
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        uint16 dest;
        uint8 msg = CanEquipItem(dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, nullptr);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        EquipItem(dest, pNewItem, true);
        AutoUnequipOffhandIfNeed();
    }
}

void Player::SwapItem( uint16 src, uint16 dst )
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item *pSrcItem = GetItemByPos( srcbag, srcslot );
    Item *pDstItem = GetItemByPos( dstbag, dstslot );

    if (!pSrcItem)
        return;

    if (!IsAlive())
    {
        SendEquipError(EQUIP_ERR_YOU_ARE_DEAD, pSrcItem, pDstItem);
        return;
    }

    if (pSrcItem->m_lootGenerated)                           // prevent swap looting item
    {
        //best error message found for attempting to swap while looting
        SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, pSrcItem, nullptr);
        return;
    }

    // check unequip potability for equipped items and bank bags
    if (IsEquipmentPos(src) || IsBagPos(src))
    {
        // bags can be swapped with empty bag slots
        uint8 msg = CanUnequipItem(src, !IsBagPos(src) || IsBagPos(dst));
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, pSrcItem, pDstItem);
            return;
        }
    }

    // prevent put equipped/bank bag in self
    if (IsBagPos(src) && srcslot == dstbag)
    {
        SendEquipError(EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG, pSrcItem, pDstItem);
        return;
    }

    // prevent equipping bag in the same slot from its inside
    if (IsBagPos(dst) && srcbag == dstslot)
    {
        SendEquipError(EQUIP_ERR_ITEMS_CANT_BE_SWAPPED, pSrcItem, pDstItem);
        return;
    }

    if (!pDstItem)
    {
        if (IsInventoryPos(dst))
        {
            ItemPosCountVec dest;
            uint8 msg = CanStoreItem(dstbag, dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, nullptr);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            StoreItem(dest, pSrcItem, true);
        }
        else if (IsBankPos(dst))
        {
            ItemPosCountVec dest;
            uint8 msg = CanBankItem(dstbag, dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, nullptr);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            BankItem(dest, pSrcItem, true);
        }
        else if (IsEquipmentPos(dst))
        {
            uint16 dest;
            uint8 msg = CanEquipItem(dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, nullptr);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            EquipItem(dest, pSrcItem, true);
            AutoUnequipOffhandIfNeed();
        }
    }
    else                                                    // if (!pDstItem)
    {
        if (pDstItem->m_lootGenerated)                       // prevent swap looting item
        {
            //best error message found for attempting to swap while looting
            SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, pDstItem, nullptr);
            return;
        }

        // check unequip potability for equipped items and bank bags
        if (IsEquipmentPos(dst) || IsBagPos(dst))
        {
            // bags can be swapped with empty bag slots
            uint8 msg = CanUnequipItem(dst, !IsBagPos(dst) || IsBagPos(src));
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, pDstItem);
                return;
            }
        }

        // attempt merge to / fill target item
        {
            uint8 msg;
            ItemPosCountVec sDest;
            uint16 eDest = 0;
            if (IsInventoryPos(dst))
                msg = CanStoreItem(dstbag, dstslot, sDest, pSrcItem, false);
            else if (IsBankPos(dst))
                msg = CanBankItem(dstbag, dstslot, sDest, pSrcItem, false);
            else if (IsEquipmentPos(dst))
                msg = CanEquipItem(dstslot, eDest, pSrcItem, false);
            else
                return;

            // can be merge/fill
            if (msg == EQUIP_ERR_OK)
            {
                if (pSrcItem->GetCount() + pDstItem->GetCount() <= pSrcItem->GetTemplate()->Stackable)
                {
                    RemoveItem(srcbag, srcslot, true);

                    if (IsInventoryPos(dst))
                        StoreItem(sDest, pSrcItem, true);
                    else if (IsBankPos(dst))
                        BankItem(sDest, pSrcItem, true);
                    else if (IsEquipmentPos(dst))
                    {
                        EquipItem(eDest, pSrcItem, true);
                        AutoUnequipOffhandIfNeed();
                    }
                }
                else
                {
                    pSrcItem->SetCount(pSrcItem->GetCount() + pDstItem->GetCount() - pSrcItem->GetTemplate()->Stackable);
                    pDstItem->SetCount(pSrcItem->GetTemplate()->Stackable);
                    pSrcItem->SetState(ITEM_CHANGED, this);
                    pDstItem->SetState(ITEM_CHANGED, this);
                    if (IsInWorld())
                    {
                        pSrcItem->SendUpdateToPlayer(this);
                        pDstItem->SendUpdateToPlayer(this);
                    }
                }
                return;
            }
        }

        // impossible merge/fill, do real swap
        uint8 msg = EQUIP_ERR_OK;

        // check src->dest move possibility
        ItemPosCountVec sDest;
        uint16 eDest = 0;
        if (IsInventoryPos(dst))
            msg = CanStoreItem(dstbag, dstslot, sDest, pSrcItem, true);
        else if (IsBankPos(dst))
            msg = CanBankItem(dstbag, dstslot, sDest, pSrcItem, true);
        else if (IsEquipmentPos(dst))
        {
            msg = CanEquipItem(dstslot, eDest, pSrcItem, true);
            if (msg == EQUIP_ERR_OK)
                msg = CanUnequipItem(eDest, true);
        }

        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, pSrcItem, pDstItem);
            return;
        }

        // check dest->src move possibility
        ItemPosCountVec sDest2;
        uint16 eDest2 = 0;
        if (IsInventoryPos(src))
            msg = CanStoreItem(srcbag, srcslot, sDest2, pDstItem, true);
        else if (IsBankPos(src))
            msg = CanBankItem(srcbag, srcslot, sDest2, pDstItem, true);
        else if (IsEquipmentPos(src))
        {
            msg = CanEquipItem(srcslot, eDest2, pDstItem, true);
            if (msg == EQUIP_ERR_OK)
                msg = CanUnequipItem(eDest2, true);
        }

        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, pDstItem, pSrcItem);
            return;
        }

        // now do moves, remove...
        RemoveItem(dstbag, dstslot, false);
        RemoveItem(srcbag, srcslot, false);

        // add to dest
        if (IsInventoryPos(dst))
            StoreItem(sDest, pSrcItem, true);
        else if (IsBankPos(dst))
            BankItem(sDest, pSrcItem, true);
        else if (IsEquipmentPos(dst))
            EquipItem(eDest, pSrcItem, true);

        // add to src
        if (IsInventoryPos(src))
            StoreItem(sDest2, pDstItem, true);
        else if (IsBankPos(src))
            BankItem(sDest2, pDstItem, true);
        else if (IsEquipmentPos(src))
            EquipItem(eDest2, pDstItem, true);

        AutoUnequipOffhandIfNeed();
    }
}

void Player::AddItemToBuyBackSlot(Item *pItem)
{
    if (pItem)
    {
        uint32 slot = m_currentBuybackSlot;
        // if current back slot non-empty search oldest or free
        if (m_items[slot])
        {
            uint32 oldest_time = GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1);
            uint32 oldest_slot = BUYBACK_SLOT_START;

            for (uint32 i = BUYBACK_SLOT_START + 1; i < BUYBACK_SLOT_END; ++i)
            {
                // found empty
                if (!m_items[i])
                {
                    slot = i;
                    break;
                }

                uint32 i_time = GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + i - BUYBACK_SLOT_START);

                if (oldest_time > i_time)
                {
                    oldest_time = i_time;
                    oldest_slot = i;
                }
            }

            // find oldest
            slot = oldest_slot;
        }

        RemoveItemFromBuyBackSlot(slot, true);

        m_items[slot] = pItem;
        time_t base = WorldGameTime::GetGameTime();
        uint32 etime = uint32(base - m_logintime + (30 * 3600));
        uint32 eslot = slot - BUYBACK_SLOT_START;

        SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, pItem->GetGUID());
        ItemTemplate const *pProto = pItem->GetTemplate();
        if (pProto)
            SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, pProto->SellPrice * pItem->GetCount());
        else
            SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, (uint32)etime);

        // move to next (for non filled list is move most optimized choice)
        if (m_currentBuybackSlot < BUYBACK_SLOT_END - 1)
            ++m_currentBuybackSlot;
    }
}

Item* Player::GetItemFromBuyBackSlot(uint32 slot)
{
    if (slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END)
        return m_items[slot];
    return nullptr;
}

void Player::RemoveItemFromBuyBackSlot(uint32 slot, bool del)
{
    if (slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END)
    {
        Item *pItem = m_items[slot];
        if (pItem)
        {
            pItem->RemoveFromWorld();
            if (del)
            {
                if (ItemTemplate const* itemTemplate = pItem->GetTemplate())
                    if (itemTemplate->Flags & ITEM_FLAG_HAS_LOOT)
                        sLootItemStorage->RemoveStoredLootForContainer(pItem->GetGUID().GetCounter());

                pItem->SetState(ITEM_REMOVED, this);
            }
        }

        m_items[slot] = nullptr;

        uint32 eslot = slot - BUYBACK_SLOT_START;
        SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, 0);

        // if current backslot is filled set to now free slot
        if (m_items[m_currentBuybackSlot])
            m_currentBuybackSlot = slot;
    }
}

void Player::SendEquipError(uint8 msg, Item* pItem, Item* pItem2 /*= nullptr*/, uint32 itemid /*= 0*/) const
{
    WorldPacket data(SMSG_INVENTORY_CHANGE_FAILURE, (msg == EQUIP_ERR_CANT_EQUIP_LEVEL_I ? 22 : (msg == EQUIP_ERR_OK ? 1 : 18)));
    data << uint8(msg);
    if(msg != EQUIP_ERR_OK)
    {
        data << uint64(pItem ? pItem->GetGUID() : ObjectGuid::Empty);
        data << uint64(pItem2 ? pItem2->GetGUID() : ObjectGuid::Empty);
        data << uint8(0);                                  // bag type subclass, used with  
                                                           // EQUIP_ERR_EVENT_AUTOEQUIP_BIND_CONFIRM and
                                                           // EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG2
        switch(msg)
        {
        case EQUIP_ERR_CANT_EQUIP_LEVEL_I:
#ifdef LICH_KING
        case EQUIP_ERR_PURCHASE_LEVEL_TOO_LOW:
#endif
        {
            ItemTemplate const* proto = pItem ? pItem->GetTemplate() : sObjectMgr->GetItemTemplate(itemid);
            data << uint32(proto ? proto->RequiredLevel : 0);
            break;
        }
#ifdef LICH_KING
        case EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED:
        case EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED:
        case EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED:
        {
            ItemTemplate const* proto = pItem ? pItem->GetTemplate() : sObjectMgr->GetItemTemplate(itemid);
            data << uint32(proto ? proto->ItemLimitCategory : 0);
            break;
        }  
#endif
        default:
            break;
        }
    }
    SendDirectMessage(&data);
}

void Player::SendBuyError(uint8 msg, Creature* pCreature, uint32 item, uint32 param)
{
    WorldPacket data(SMSG_BUY_FAILED, (8 + 4 + 4 + 1));
    data << uint64(pCreature ? pCreature->GetGUID() : 0);
    data << uint32(item);
    if (param > 0)
        data << uint32(param);
    data << uint8(msg);
    SendDirectMessage(&data);
}

void Player::SendSellError(uint8 msg, Creature* pCreature, ObjectGuid guid, uint32 param)
{
    WorldPacket data(SMSG_SELL_ITEM, (8 + 8 + (param ? 4 : 0) + 1));  // last check 2.0.10
    data << uint64(pCreature ? pCreature->GetGUID() : 0);
    data << uint64(guid);
    if (param > 0)
        data << uint32(param);
    data << uint8(msg);
    SendDirectMessage(&data);
}

void Player::TradeCancel(bool sendback)
{
    if (m_trade)
    {
        Player* trader = m_trade->GetTrader();

        // send yellow "Trade canceled" message to both traders
        if(sendback)
            GetSession()->SendCancelTrade();

        GetSession()->SendCancelTrade();

        // cleanup
        delete m_trade;
        m_trade = nullptr;
        delete trader->m_trade;
        trader->m_trade = nullptr;
    }
}

void Player::UpdateItemDuration(uint32 time, bool realtimeonly)
{
    if(m_itemDuration.empty())
        return;

    for(auto itr = m_itemDuration.begin();itr != m_itemDuration.end(); )
    {
        Item* item = *itr;
        ++itr;                                              // current element can be erased in UpdateDuration

        if (realtimeonly && (item->GetTemplate()->Duration < 0))
            item->UpdateDuration(this,time);
    }
}

void Player::UpdateEnchantTime(uint32 time)
{
    for(EnchantDurationList::iterator itr = m_enchantDuration.begin(),next;itr != m_enchantDuration.end();itr=next)
    {
        assert(itr->item);
        next=itr;
        if(!itr->item->GetEnchantmentId(itr->slot))
        {
            next = m_enchantDuration.erase(itr);
        }
        else if(itr->leftduration <= time)
        {
            ApplyEnchantment(itr->item,itr->slot,false,false);
            itr->item->ClearEnchantment(itr->slot);
            next = m_enchantDuration.erase(itr);
        }
        else if(itr->leftduration > time)
        {
            itr->leftduration -= time;
            ++next;
        }
    }
}

void Player::AddEnchantmentDurations(Item *item)
{
    for(int x=0;x<MAX_ENCHANTMENT_SLOT;++x)
    {
        if(!item->GetEnchantmentId(EnchantmentSlot(x)))
            continue;

        uint32 duration = item->GetEnchantmentDuration(EnchantmentSlot(x));
        if( duration > 0 )
            AddEnchantmentDuration(item,EnchantmentSlot(x),duration);
    }
}

void Player::RemoveEnchantmentDurations(Item *item)
{
    for(auto itr = m_enchantDuration.begin();itr != m_enchantDuration.end();)
    {
        if(itr->item == item)
        {
            // save duration in item
            item->SetEnchantmentDuration(EnchantmentSlot(itr->slot),itr->leftduration);
            itr = m_enchantDuration.erase(itr);
        }
        else
            ++itr;
    }
}

void Player::RemoveAllEnchantments(EnchantmentSlot slot, bool arena)
{
    // remove enchantments from equipped items first to clean up the m_enchantDuration list
    for(EnchantDurationList::iterator itr = m_enchantDuration.begin(),next;itr != m_enchantDuration.end();itr=next)
    {
        next = itr;
        if(itr->slot==slot)
        {
            if(arena && itr->item)
            {
                uint32 enchant_id = itr->item->GetEnchantmentId(slot);
                if(enchant_id)
                {
                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if(pEnchant && pEnchant->aura_id == ITEM_ENCHANTMENT_AURAID_POISON)
                    {
                        ++next;
                        continue;
                    }
                }
            }
            if(itr->item && itr->item->GetEnchantmentId(slot))
            {
                // remove from stats
                ApplyEnchantment(itr->item, slot, false, false);
                // remove visual
                itr->item->ClearEnchantment(slot);
            }
            // remove from update list
            next = m_enchantDuration.erase(itr);
        }
        else
            ++next;
    }

    // remove enchants from inventory items
    // NOTE: no need to remove these from stats, since these aren't equipped
    // in inventory
    for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!pItem)
            continue;
        uint32 enchant_id = pItem->GetEnchantmentId(slot);
        if (enchant_id) 
        {
            SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (arena && pEnchant && pEnchant->aura_id == ITEM_ENCHANTMENT_AURAID_POISON)
                continue;

            pItem->ClearEnchantment(slot);
        }
    }

    // in inventory bags
    for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for(uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (!pItem)
                    continue;
                uint32 enchant_id = pItem->GetEnchantmentId(slot);
                if (enchant_id) {
                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if (arena && pEnchant && pEnchant->aura_id == ITEM_ENCHANTMENT_AURAID_POISON)
                        continue;

                    pItem->ClearEnchantment(slot);
                }
            }
        }
    }
}

// duration == 0 will remove item enchant
void Player::AddEnchantmentDuration(Item *item,EnchantmentSlot slot,uint32 duration)
{
    if(!item)
        return;

    if(slot >= MAX_ENCHANTMENT_SLOT)
        return;

    for(auto itr = m_enchantDuration.begin();itr != m_enchantDuration.end();++itr)
    {
        if(itr->item == item && itr->slot == slot)
        {
            itr->item->SetEnchantmentDuration(itr->slot,itr->leftduration);
            m_enchantDuration.erase(itr);
            break;
        }
    }
    if(duration > 0 )
    {
        GetSession()->SendItemEnchantTimeUpdate(GetGUID(), item->GetGUID(),slot,uint32(duration/1000));
        m_enchantDuration.push_back(EnchantDuration(item,slot,duration));
    }
}

void Player::ApplyEnchantment(Item *item,bool apply)
{
    for(uint32 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
        ApplyEnchantment(item, EnchantmentSlot(slot), apply);
}

void Player::ApplyEnchantment(Item *item,EnchantmentSlot slot,bool apply, bool apply_dur, bool ignore_condition)
{
    if(!item)
        return;

    if(!item->IsEquipped())
        return;

    if(slot >= MAX_ENCHANTMENT_SLOT)
        return;

    uint32 enchant_id = item->GetEnchantmentId(slot);
    if(!enchant_id)
        return;

    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!pEnchant)
        return;

    if(!ignore_condition && pEnchant->EnchantmentCondition && !(this->ToPlayer())->EnchantmentFitsRequirements(pEnchant->EnchantmentCondition, -1))
        return;

    for (int s=0; s<3; s++)
    {
        uint32 enchant_display_type = pEnchant->type[s];
        uint32 enchant_amount = pEnchant->amount[s];
        uint32 enchant_spell_id = pEnchant->spellid[s];

        switch(enchant_display_type)
        {
            case ITEM_ENCHANTMENT_TYPE_NONE:
                break;
            case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
                // processed in Player::CastItemCombatSpell
                break;
            case ITEM_ENCHANTMENT_TYPE_DAMAGE:
            {
                WeaponAttackType const attackType = Player::GetAttackBySlot(item->GetSlot());
                if (attackType != MAX_ATTACK)
                    UpdateDamageDoneMods(attackType);
            } break;
            case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
                if(enchant_spell_id)
                {
                    // Hack, Flametongue Weapon
                    if (enchant_spell_id==10400 || enchant_spell_id==15567 ||
                        enchant_spell_id==15568 || enchant_spell_id==15569 ||
                        enchant_spell_id==16311 || enchant_spell_id==16312 ||
                        enchant_spell_id==16313)
                    {
                        // processed in Player::CastItemCombatSpell
                        break;
                    }
                    else if(apply)
                    {
                        int32 basepoints = 0;
                        // Random Property Exist - try found basepoints for spell (basepoints depends from item suffix factor)
                        if (item->GetItemRandomPropertyId())
                        {
                            ItemRandomSuffixEntry const *item_rand = sItemRandomSuffixStore.LookupEntry(abs(item->GetItemRandomPropertyId()));
                            if (item_rand)
                            {
                                // Search enchant_amount
                                for (int k=0; k<3; k++)
                                {
                                    if(item_rand->enchant_id[k] == enchant_id)
                                    {
                                        basepoints = int32((item_rand->prefix[k]*item->GetItemSuffixFactor()) / 10000 );
                                        break;
                                    }
                                }
                            }
                        }
                        CastSpellExtraArgs args;
                        args.TriggerFlags = TRIGGERED_FULL_MASK;
                        if (basepoints) // Cast custom spell vs all equal basepoints getted from enchant_amount
                        {
                            args.AddSpellMod(SPELLVALUE_BASE_POINT0, int32(basepoints));
                            args.AddSpellMod(SPELLVALUE_BASE_POINT1, int32(basepoints));
                            args.AddSpellMod(SPELLVALUE_BASE_POINT2, int32(basepoints));
                        }
                        args.SetCastItem(item);
                        CastSpell(this, enchant_spell_id, args);
                    }
                    else
                        RemoveAurasDueToItemSpell(enchant_spell_id, item->GetGUID());
                }
                break;
            case ITEM_ENCHANTMENT_TYPE_RESISTANCE:
                if (!enchant_amount)
                {
                    ItemRandomSuffixEntry const *item_rand = sItemRandomSuffixStore.LookupEntry(abs(item->GetItemRandomPropertyId()));
                    if(item_rand)
                    {
                        for (int k=0; k<3; k++)
                        {
                            if(item_rand->enchant_id[k] == enchant_id)
                            {
                                enchant_amount = uint32((item_rand->prefix[k]*item->GetItemSuffixFactor()) / 10000 );
                                break;
                            }
                        }
                    }
                }

                HandleStatFlatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + enchant_spell_id), TOTAL_VALUE, float(enchant_amount), apply);
                break;
            case ITEM_ENCHANTMENT_TYPE_STAT:
            {
                if (!enchant_amount)
                {
                    ItemRandomSuffixEntry const *item_rand_suffix = sItemRandomSuffixStore.LookupEntry(abs(item->GetItemRandomPropertyId()));
                    if(item_rand_suffix)
                    {
                        for (int k=0; k<3; k++)
                        {
                            if(item_rand_suffix->enchant_id[k] == enchant_id)
                            {
                                enchant_amount = uint32((item_rand_suffix->prefix[k]*item->GetItemSuffixFactor()) / 10000 );
                                break;
                            }
                        }
                    }
                }

                switch (enchant_spell_id)
                {
                    case ITEM_MOD_AGILITY:
                        HandleStatFlatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, float(enchant_amount), apply);
                        UpdateStatBuffMod(STAT_AGILITY);
                        break;
                    case ITEM_MOD_STRENGTH:
                        HandleStatFlatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, float(enchant_amount), apply);
                        UpdateStatBuffMod(STAT_STRENGTH);
                        break;
                    case ITEM_MOD_INTELLECT:
                        HandleStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, float(enchant_amount), apply);
                        UpdateStatBuffMod(STAT_INTELLECT);
                        break;
                    case ITEM_MOD_SPIRIT:
                        HandleStatFlatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, float(enchant_amount), apply);
                        UpdateStatBuffMod(STAT_SPIRIT);
                        break;
                    case ITEM_MOD_STAMINA:
                        HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, float(enchant_amount), apply);
                        UpdateStatBuffMod(STAT_STAMINA);
                        break;
                    case ITEM_MOD_DEFENSE_SKILL_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_DEFENSE_SKILL, enchant_amount, apply);
                        break;
                    case  ITEM_MOD_DODGE_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_DODGE, enchant_amount, apply);
                        break;
                    case ITEM_MOD_PARRY_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_PARRY, enchant_amount, apply);
                        break;
                    case ITEM_MOD_BLOCK_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_BLOCK, enchant_amount, apply);
                        break;
                    case ITEM_MOD_HIT_MELEE_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_MELEE, enchant_amount, apply);
                        break;
                    case ITEM_MOD_HIT_RANGED_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_RANGED, enchant_amount, apply);
                        break;
                    case ITEM_MOD_HIT_SPELL_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_SPELL, enchant_amount, apply);
                        break;
                    case ITEM_MOD_CRIT_MELEE_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                        break;
                    case ITEM_MOD_CRIT_RANGED_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                        break;
                    case ITEM_MOD_CRIT_SPELL_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                        break;
//                    Values from ITEM_STAT_MELEE_HA_RATING to ITEM_MOD_HASTE_RANGED_RATING are never used
//                    in Enchantments
//                    case ITEM_MOD_HIT_TAKEN_MELEE_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_TAKEN_MELEE, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_HIT_TAKEN_RANGED_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_TAKEN_RANGED, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_HIT_TAKEN_SPELL_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_TAKEN_SPELL, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_CRIT_TAKEN_MELEE_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_CRIT_TAKEN_RANGED_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_CRIT_TAKEN_SPELL_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_HASTE_MELEE_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_HASTE_MELEE, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_HASTE_RANGED_RATING:
//                        (this->ToPlayer())->ApplyRatingMod(CR_HASTE_RANGED, enchant_amount, apply);
//                        break;
                    case ITEM_MOD_HASTE_SPELL_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_HASTE_SPELL, enchant_amount, apply);
                        break;
                    case ITEM_MOD_HIT_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_MELEE, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_RANGED, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_HIT_SPELL, enchant_amount, apply);
                        break;
                    case ITEM_MOD_CRIT_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                        break;
//                    Values ITEM_MOD_HIT_TAKEN_RATING and ITEM_MOD_CRIT_TAKEN_RATING are never used in Enchantment
//                    case ITEM_MOD_HIT_TAKEN_RATING:
//                          (this->ToPlayer())->ApplyRatingMod(CR_HIT_TAKEN_MELEE, enchant_amount, apply);
//                          (this->ToPlayer())->ApplyRatingMod(CR_HIT_TAKEN_RANGED, enchant_amount, apply);
//                          (this->ToPlayer())->ApplyRatingMod(CR_HIT_TAKEN_SPELL, enchant_amount, apply);
//                        break;
//                    case ITEM_MOD_CRIT_TAKEN_RATING:
//                          (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
//                          (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
//                          (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
//                        break;
                    case ITEM_MOD_RESILIENCE_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                        break;
                    case ITEM_MOD_HASTE_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_HASTE_MELEE, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_HASTE_RANGED, enchant_amount, apply);
                        (this->ToPlayer())->ApplyRatingMod(CR_HASTE_SPELL, enchant_amount, apply);
                        break;
                    case ITEM_MOD_EXPERTISE_RATING:
                        (this->ToPlayer())->ApplyRatingMod(CR_EXPERTISE, enchant_amount, apply);
                        break;
#ifdef LICH_KING
                    case ITEM_MOD_ATTACK_POWER:
                        HandleStatFlatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(enchant_amount), apply);
                        HandleStatFlatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u ATTACK_POWER", enchant_amount);
                        break;
                    case ITEM_MOD_RANGED_ATTACK_POWER:
                        HandleStatFlatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u RANGED_ATTACK_POWER", enchant_amount);
                        break;
                        //                        case ITEM_MOD_FERAL_ATTACK_POWER:
                        //                            ApplyFeralAPBonus(enchant_amount, apply);
                        //                            TC_LOG_DEBUG("entities.player.items", "+ %u FERAL_ATTACK_POWER", enchant_amount);
                        //                            break;
                    case ITEM_MOD_MANA_REGENERATION:
                        ApplyManaRegenBonus(enchant_amount, apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u MANA_REGENERATION", enchant_amount);
                        break;
                    case ITEM_MOD_ARMOR_PENETRATION_RATING:
                        ApplyRatingMod(CR_ARMOR_PENETRATION, enchant_amount, apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u ARMOR PENETRATION", enchant_amount);
                        break;
                    case ITEM_MOD_SPELL_POWER:
                        ApplySpellPowerBonus(enchant_amount, apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u SPELL_POWER", enchant_amount);
                        break;
                    case ITEM_MOD_HEALTH_REGEN:
                        ApplyHealthRegenBonus(enchant_amount, apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u HEALTH_REGENERATION", enchant_amount);
                        break;
                    case ITEM_MOD_SPELL_PENETRATION:
                        ApplySpellPenetrationBonus(enchant_amount, apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u SPELL_PENETRATION", enchant_amount);
                        break;
                    case ITEM_MOD_BLOCK_VALUE:
                        HandleBaseModFlatValue(SHIELD_BLOCK_VALUE, float(enchant_amount), apply);
                        TC_LOG_DEBUG("entities.player.items", "+ %u BLOCK_VALUE", enchant_amount);
                        break;
                    case ITEM_MOD_SPELL_HEALING_DONE:   // deprecated
                    case ITEM_MOD_SPELL_DAMAGE_DONE:    // deprecated
#endif
                    default:
                        break;
                }
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_TOTEM:               // Shaman Rockbiter Weapon
            {
                if(GetClass() == CLASS_SHAMAN)
                {
                    switch(item->GetSlot())
                    {
                        case EQUIPMENT_SLOT_MAINHAND:
                        case EQUIPMENT_SLOT_OFFHAND:
                        {
                            UnitMods mod = item->GetSlot() == EQUIPMENT_SLOT_MAINHAND ? UNIT_MOD_DAMAGE_MAINHAND : UNIT_MOD_DAMAGE_OFFHAND;
                            float addValue = float(enchant_amount * item->GetTemplate()->Delay / 1000.0f);
                            HandleStatFlatModifier(mod, TOTAL_VALUE, addValue, apply);
                            break;
                        }
                        default:
                            break;
                    }
                }
                break;
            }
            default:
                TC_LOG_ERROR("entities.player","Unknown item enchantment display type: %d",enchant_display_type);
                break;
        }                                                   /*switch(enchant_display_type)*/
    }                                                       /*for*/

    // visualize enchantment at player and equipped items
    if(slot < MAX_INSPECTED_ENCHANTMENT_SLOT)
    {
        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (item->GetSlot() * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(VisibleBase + 1 + slot, apply? item->GetEnchantmentId(slot) : 0);
    }

    if(apply_dur)
    {
        if(apply)
        {
            // set duration
            uint32 duration = item->GetEnchantmentDuration(slot);
            if(duration > 0)
                AddEnchantmentDuration(item,slot,duration);
        }
        else
        {
            // duration == 0 will remove EnchantDuration
            AddEnchantmentDuration(item,slot,0);
        }
    }
}

void Player::SendEnchantmentDurations()
{
    for(auto & itr : m_enchantDuration)
    {
        GetSession()->SendItemEnchantTimeUpdate(GetGUID(), itr.item->GetGUID(),itr.slot,uint32(itr.leftduration)/1000);
    }
}

void Player::SendItemDurations()
{
    for(auto & itr : m_itemDuration)
    {
        itr->SendTimeUpdate(this);
    }
}

void Player::SendNewItem(Item *item, uint32 count, bool received, bool created, bool broadcast, bool sendChatMessage)
{
    if(!item)                                               // prevent crash
        return;
                                                            // last check 2.0.10
    WorldPacket data( SMSG_ITEM_PUSH_RESULT, (8+4+4+4+1+4+4+4+4+4) );
    data << GetGUID();                                      // player GUID
    data << uint32(received);                               // 0=looted, 1=from npc
    data << uint32(created);                                // 0=received, 1=created
    data << uint32(sendChatMessage);                        // bool print message to chat
    data << (uint8)item->GetBagSlot();                      // bagslot
                                                            // item slot, but when added to stack: 0xFFFFFFFF
    data << (uint32) ((item->GetCount()==count) ? item->GetSlot() : -1);
    data << uint32(item->GetEntry());                       // item id
    data << uint32(item->GetItemSuffixFactor());            // SuffixFactor
    data << uint32(item->GetItemRandomPropertyId());        // random item property id
    data << uint32(count);                                  // count of items
    data << GetItemCount(item->GetEntry());                 // count of items in inventory

    if (broadcast && GetGroup())
        GetGroup()->BroadcastPacket(&data, true);
    else
        SendDirectMessage(&data);
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

void Player::PrepareQuestMenu(ObjectGuid guid)
{
    Object *pObject;
    QuestRelations* pObjectQR;
    QuestRelations* pObjectQIR;
    Creature *pCreature = ObjectAccessor::GetCreature(*this, guid);
    if( pCreature )
    {
        pObject = (Object*)pCreature;
        pObjectQR  = &sObjectMgr->mCreatureQuestRelations;
        pObjectQIR = &sObjectMgr->mCreatureQuestInvolvedRelations;
    }
    else
    {
        GameObject *pGameObject = ObjectAccessor::GetGameObject(*this, guid);
        if( pGameObject )
        {
            pObject = (Object*)pGameObject;
            pObjectQR  = &sObjectMgr->mGOQuestRelations;
            pObjectQIR = &sObjectMgr->mGOQuestInvolvedRelations;
        }
        else
            return;
    }

    QuestMenu &qm = PlayerTalkClass->GetQuestMenu();
    qm.ClearMenu();

    for(QuestRelations::const_iterator i = pObjectQIR->lower_bound(pObject->GetEntry()); i != pObjectQIR->upper_bound(pObject->GetEntry()); ++i)
    {
        uint32 quest_id = i->second;
        QuestStatus status = GetQuestStatus( quest_id );
        if ( status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus( quest_id ) )
            qm.AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if ( status == QUEST_STATUS_INCOMPLETE)
            qm.AddMenuItem(quest_id, DIALOG_STATUS_INCOMPLETE);
        //else if (status == QUEST_STATUS_AVAILABLE)
        //    qm.AddMenuItem(quest_id, DIALOG_STATUS_CHAT);
    }

    for(QuestRelations::const_iterator i = pObjectQR->lower_bound(pObject->GetEntry()); i != pObjectQR->upper_bound(pObject->GetEntry()); ++i)
    {
        uint32 quest_id = i->second;
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(quest_id);
        if(!pQuest)
            continue;

        QuestStatus status = GetQuestStatus( quest_id );

        if (pQuest->IsAutoComplete() && CanTakeQuest(pQuest, false))
            qm.AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if ( status == QUEST_STATUS_NONE && CanTakeQuest( pQuest, false ) ) {
            if (pCreature && pCreature->GetQuestPoolId()) {
                if (!sWorld->IsQuestInAPool(quest_id)) {
                    qm.AddMenuItem(quest_id, DIALOG_STATUS_CHAT);
                    continue;
                }
                // Quest is in a pool, check if it's current
                if (sWorld->GetCurrentQuestForPool(pCreature->GetQuestPoolId()) != quest_id)
                    continue;
                else
                    qm.AddMenuItem(quest_id, DIALOG_STATUS_CHAT);
            }
            else    // No quest pool, just add it
                qm.AddMenuItem(quest_id, DIALOG_STATUS_AVAILABLE);
        }
    }
}

void Player::SendPreparedQuest(ObjectGuid guid)
{
    QuestMenu& questMenu = PlayerTalkClass->GetQuestMenu();
    if (questMenu.Empty())
        return;

    QuestMenuItem const& qmi0 = questMenu.GetItem(0);
    uint32 status = qmi0.QuestIcon;

    // single element case
    if (questMenu.GetMenuItemCount() == 1)
    {
        // Auto open -- maybe also should verify there is no greeting
        uint32 questId = qmi0.QuestId;
        if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
        {
            if (status == DIALOG_STATUS_REWARD_REP && !GetQuestRewardStatus(questId))
                PlayerTalkClass->SendQuestGiverRequestItems(quest, guid, CanRewardQuest(quest, false), true);
            else if (status == DIALOG_STATUS_INCOMPLETE)
                PlayerTalkClass->SendQuestGiverRequestItems(quest, guid, false, true);
            // Send completable on repeatable quest if player don't have quest
            else if (quest->IsRepeatable() && !quest->IsDaily())
                PlayerTalkClass->SendQuestGiverRequestItems(quest, guid, CanCompleteRepeatableQuest(quest), true);
            else
                PlayerTalkClass->SendQuestGiverQuestDetails(quest, guid, true);
        }
    }
    // multiply entries
    else
    {
        QEmote qe;
        qe._Delay = 0;
        qe._Emote = 0;
        std::string title = "";

        // need pet case for some quests
        Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*this, guid);
        if (creature)
        {
            uint32 textid = GetGossipTextId(creature);
            GossipText const* gossiptext = sObjectMgr->GetGossipText(textid);
            if (!gossiptext)
            {
                qe._Delay = 0;                              //TEXTEMOTE_MESSAGE;              //zyg: player emote
                qe._Emote = 0;                              //TEXTEMOTE_HELLO;                //zyg: NPC emote
                title.clear();
            }
            else
            {
                qe = gossiptext->Options[0].Emotes[0];

                if(!gossiptext->Options[0].Text_0.empty())
                {
                    title = gossiptext->Options[0].Text_0;

                    LocaleConstant localeConstant = GetSession()->GetSessionDbLocaleIndex();
                    if (localeConstant != LOCALE_enUS)
                        if (NpcTextLocale const* nl = sObjectMgr->GetNpcTextLocale(textid))
                            ObjectMgr::GetLocaleString(nl->Text_0[0], localeConstant, title);
                }
                else
                {
                    title = gossiptext->Options[0].Text_1;

                    LocaleConstant localeConstant = GetSession()->GetSessionDbLocaleIndex();
                    if (localeConstant != LOCALE_enUS)
                        if (NpcTextLocale const* nl = sObjectMgr->GetNpcTextLocale(textid))
                            ObjectMgr::GetLocaleString(nl->Text_1[0], localeConstant, title);
                }
            }
        }
        PlayerTalkClass->SendQuestGiverQuestList( qe, title, guid );
    }
}

bool Player::IsActiveQuest(uint32 quest_id) const
{
    auto itr = m_QuestStatus.find(quest_id);

    return itr != m_QuestStatus.end() && itr->second.Status != QUEST_STATUS_NONE;
}

Quest const* Player::GetNextQuest(ObjectGuid guid, Quest const *pQuest)
{
    Object *pObject;
    QuestRelations* pObjectQR;

    Creature *pCreature = ObjectAccessor::GetCreature(*this, guid);
    if( pCreature )
    {
        pObject = (Object*)pCreature;
        pObjectQR  = &sObjectMgr->mCreatureQuestRelations;
        //pObjectQIR = &sObjectMgr->mCreatureQuestInvolvedRelations;
    }
    else
    {
        GameObject *pGameObject = ObjectAccessor::GetGameObject(*this, guid);
        if( pGameObject )
        {
            pObject = (Object*)pGameObject;
            pObjectQR  = &sObjectMgr->mGOQuestRelations;
            //pObjectQIR = &sObjectMgr->mGOQuestInvolvedRelations;
        }
        else
            return nullptr;
    }

    uint32 nextQuestID = pQuest->GetNextQuestInChain();
    for(QuestRelations::const_iterator itr = pObjectQR->lower_bound(pObject->GetEntry()); itr != pObjectQR->upper_bound(pObject->GetEntry()); ++itr)
    {
        if (itr->second == nextQuestID)
            return sObjectMgr->GetQuestTemplate(nextQuestID);
    }

    return nullptr;
}

bool Player::CanSeeStartQuest( Quest const *pQuest )
{
    if( SatisfyQuestRace( pQuest, false ) && SatisfyQuestSkillOrClass( pQuest, false ) &&
        SatisfyQuestExclusiveGroup( pQuest, false ) && SatisfyQuestReputation( pQuest, false ) &&
        SatisfyQuestPreviousQuest( pQuest, false ) && SatisfyQuestNextChain( pQuest, false ) &&
        SatisfyQuestPrevChain( pQuest, false ) && SatisfyQuestDay( pQuest, false ) )
    {
        return GetLevel() + sWorld->getConfig(CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF) >= pQuest->GetMinLevel();
    }

    return false;
}

bool Player::CanTakeQuest( Quest const *pQuest, bool msg )
{
    return SatisfyQuestStatus( pQuest, msg ) && SatisfyQuestExclusiveGroup( pQuest, msg )
        && SatisfyQuestRace( pQuest, msg ) && SatisfyQuestLevel( pQuest, msg )
        && SatisfyQuestSkillOrClass( pQuest, msg ) && SatisfyQuestReputation( pQuest, msg )
        && SatisfyQuestPreviousQuest( pQuest, msg ) && SatisfyQuestTimed( pQuest, msg )
        && SatisfyQuestNextChain( pQuest, msg ) && SatisfyQuestPrevChain( pQuest, msg )
        && SatisfyQuestDay( pQuest, msg )
        && SatisfyQuestConditions(pQuest, msg);;
}

bool Player::CanAddQuest( Quest const *pQuest, bool msg )
{
    if( !SatisfyQuestLog( msg ) )
        return false;

    uint32 srcitem = pQuest->GetSrcItemId();
    if( srcitem > 0 )
    {
        uint32 count = pQuest->GetSrcItemCount();
        ItemPosCountVec dest;
        uint8 _msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, srcitem, count );

        // player already have max number (in most case 1) source item, no additional item needed and quest can be added.
        if(_msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS )
            return true;
        else if(_msg != EQUIP_ERR_OK )
        {
            SendEquipError(_msg, nullptr, nullptr );
            return false;
        }
    }
    return true;
}

bool Player::CanCompleteQuest( uint32 quest_id )
{
    if( quest_id )
    {
        QuestStatusData& q_status = m_QuestStatus[quest_id];
        if( q_status.Status == QUEST_STATUS_COMPLETE )
            return false;                                   // not allow re-complete quest

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest_id);

        if(!qInfo)
            return false;

        // auto complete quest
        if (qInfo->IsAutoComplete() && CanTakeQuest(qInfo, false))
            return true;

        if ( q_status.Status == QUEST_STATUS_INCOMPLETE )
        {

            if ( qInfo->HasFlag( QUEST_TRINITY_FLAGS_DELIVER ) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if( qInfo->RequiredItemCount[i]!= 0 && q_status.ItemCount[i] < qInfo->RequiredItemCount[i] )
                        return false;
                }
            }

            if ( qInfo->HasFlag(QUEST_TRINITY_FLAGS_KILL_OR_CAST | QUEST_TRINITY_FLAGS_SPEAKTO) )
            {
                for(int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if( qInfo->RequiredNpcOrGo[i] == 0 )
                        continue;

                    if( qInfo->RequiredNpcOrGoCount[i] != 0 && q_status.CreatureOrGOCount[i] < qInfo->RequiredNpcOrGoCount[i] )
                        return false;
                }
            }

            if ( qInfo->HasFlag( QUEST_TRINITY_FLAGS_EXPLORATION_OR_EVENT ) && !q_status.Explored )
                return false;

            if ( qInfo->HasFlag( QUEST_TRINITY_FLAGS_TIMED ) && q_status.m_timer == 0 )
                return false;

            if ( qInfo->GetRewOrReqMoney() < 0 )
            {
                if ( GetMoney() < uint32(-qInfo->GetRewOrReqMoney()) )
                    return false;
            }

            uint32 repFacId = qInfo->GetRepObjectiveFaction();
            if ( repFacId && GetReputation(repFacId) < qInfo->GetRepObjectiveValue() )
                return false;

            return true;
        }
    }
    return false;
}

bool Player::CanCompleteRepeatableQuest( Quest const *pQuest )
{
    // Solve problem that player don't have the quest and try complete it.
    // if repeatable she must be able to complete event if player don't have it.
    // Seem that all repeatable quest are DELIVER Flag so, no need to add more.
    if( !CanTakeQuest(pQuest, false) )
        return false;

    if (pQuest->HasFlag( QUEST_TRINITY_FLAGS_DELIVER) )
        for(int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++)
            if( pQuest->RequiredItemId[i] && pQuest->RequiredItemCount[i] && !HasItemCount(pQuest->RequiredItemId[i],pQuest->RequiredItemCount[i]) )
                return false;

    if( !CanRewardQuest(pQuest, false) )
        return false;

    return true;
}

bool Player::CanRewardQuest( Quest const *pQuest, bool msg )
{
    // not auto complete quest and not completed quest (only cheating case, then ignore without message)
    if(!pQuest->IsAutoComplete() && GetQuestStatus(pQuest->GetQuestId()) != QUEST_STATUS_COMPLETE)
        return false;

    // daily quest can't be rewarded (25 daily quest already completed)
    if(!SatisfyQuestDay(pQuest,true))
        return false;

    // rewarded and not repeatable quest (only cheating case, then ignore without message)
    if(GetQuestRewardStatus(pQuest->GetQuestId()))
        return false;

    // prevent receive reward with quest items in bank
    if ( pQuest->HasFlag( QUEST_TRINITY_FLAGS_DELIVER ) )
    {
        for(int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++)
        {
            if( pQuest->RequiredItemCount[i]!= 0 &&
                GetItemCount(pQuest->RequiredItemId[i]) < pQuest->RequiredItemCount[i] )
            {
                if(msg)
                    SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr );
                return false;
            }
        }
    }

    // prevent receive reward with low money and GetRewOrReqMoney() < 0
    if(pQuest->GetRewOrReqMoney() < 0 && GetMoney() < uint32(-pQuest->GetRewOrReqMoney()) )
        return false;

    return true;
}

bool Player::CanRewardQuest( Quest const *pQuest, uint32 reward, bool msg )
{
    // prevent receive reward with quest items in bank or for not completed quest
    if(!CanRewardQuest(pQuest,msg))
        return false;

    if ( pQuest->GetRewardChoiceItemsCount() > 0 )
    {
        if( pQuest->RewardChoiceItemId[reward] )
        {
            ItemPosCountVec dest;
            uint8 res = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pQuest->RewardChoiceItemId[reward], pQuest->RewardChoiceItemCount[reward] );
            if( res != EQUIP_ERR_OK )
            {
                SendEquipError( res, nullptr, nullptr );
                return false;
            }
        }
    }

    if ( pQuest->GetRewardItemsCount() > 0 )
    {
        for (uint32 i = 0; i < pQuest->GetRewardItemsCount(); ++i)
        {
            if( pQuest->RewardItemId[i] )
            {
                ItemPosCountVec dest;
                uint8 res = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pQuest->RewardItemId[i], pQuest->RewardItemIdCount[i] );
                if( res != EQUIP_ERR_OK )
                {
                    SendEquipError( res, nullptr, nullptr );
                    return false;
                }
            }
        }
    }

    return true;
}

void Player::AddQuest(Quest const *pQuest, Object* questGiver)
{
    uint16 log_slot = FindQuestSlot( 0 );
    assert(log_slot < MAX_QUEST_LOG_SIZE);

    uint32 quest_id = pQuest->GetQuestId();

    // if not exist then created with set uState==NEW and rewarded=false
    QuestStatusData& questStatusData = m_QuestStatus[quest_id];
    if (questStatusData.uState != QUEST_NEW)
        questStatusData.uState = QUEST_CHANGED;

    // check for repeatable quests status reset
    questStatusData.Status = QUEST_STATUS_INCOMPLETE;
    questStatusData.Explored = false;

    if ( pQuest->HasFlag( QUEST_TRINITY_FLAGS_DELIVER ) )
    {
        for(uint32 & i : questStatusData.ItemCount)
            i = 0;
    }

    if ( pQuest->HasFlag(QUEST_TRINITY_FLAGS_KILL_OR_CAST | QUEST_TRINITY_FLAGS_SPEAKTO) )
    {
        for(uint32 & i : questStatusData.CreatureOrGOCount)
            i = 0;
    }

    GiveQuestSourceItem(pQuest);
    AdjustQuestRequiredItemCount(pQuest);

    if (pQuest->GetRepObjectiveFaction())
        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(pQuest->GetRepObjectiveFaction()))
            GetReputationMgr().SetVisible(factionEntry);

#ifdef LICH_KING
    if (pQuest->GetRepObjectiveFaction2())
        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(pQuest->GetRepObjectiveFaction2()))
            GetReputationMgr().SetVisible(factionEntry);
#endif

    uint32 qtime = 0;
    if (pQuest->HasFlag(QUEST_TRINITY_FLAGS_TIMED))
    {
        uint32 limittime = pQuest->GetLimitTime();

        // shared timed quest
        if(questGiver && questGiver->GetTypeId()==TYPEID_PLAYER)
            limittime = (questGiver->ToPlayer())->getQuestStatusMap()[quest_id].m_timer / 1000;

        AddTimedQuest( quest_id );
        questStatusData.m_timer = limittime * 1000;
        qtime = static_cast<uint32>(WorldGameTime::GetGameTime()) + limittime;
    }
    else
        questStatusData.m_timer = 0;

    SetQuestSlot(log_slot, quest_id, qtime);

    //starting initial quest script
    if(questGiver && pQuest->GetQuestStartScript()!=0)
        GetMap()->ScriptsStart(sQuestStartScripts, pQuest->GetQuestStartScript(), questGiver, this);

    UpdateForQuestWorldObjects();
}

void Player::AddQuestAndCheckCompletion(Quest const* quest, Object* questGiver)
{
    AddQuest(quest, questGiver);

    if (CanCompleteQuest(quest->GetQuestId()))
        CompleteQuest(quest->GetQuestId());

    if (!questGiver)
        return;

    switch (questGiver->GetTypeId())
    {
        case TYPEID_UNIT:
            PlayerTalkClass->ClearMenus();
            questGiver->ToCreature()->AI()->QuestAccept(this, quest);
            break;
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
        {
            Item* item = (Item*)questGiver;
            sScriptMgr->OnQuestAccept(this, item, quest);

            // destroy not required for quest finish quest starting item
            bool destroyItem = true;
            for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
            {
                if (quest->RequiredItemId[i] == item->GetEntry() && item->GetTemplate()->MaxCount > 0)
                {
                    destroyItem = false;
                    break;
                }
            }

            if (destroyItem)
                DestroyItem(item->GetBagSlot(), item->GetSlot(), true);

            break;
        }
        case TYPEID_GAMEOBJECT:
            PlayerTalkClass->ClearMenus();
            questGiver->ToGameObject()->AI()->QuestAccept(this, quest);
            break;
        default:
            break;
    }
}

void Player::CompleteQuest( uint32 quest_id )
{
    if( quest_id )
    {
        SetQuestStatus( quest_id, QUEST_STATUS_COMPLETE );

        uint16 log_slot = FindQuestSlot( quest_id );
        if( log_slot < MAX_QUEST_LOG_SIZE)
            SetQuestSlotState(log_slot,QUEST_STATE_COMPLETE);

        if(Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest_id))
            if( qInfo->HasFlag(QUEST_FLAGS_AUTO_REWARDED) )
                RewardQuest(qInfo,0,this,false);
    }
}

void Player::IncompleteQuest( uint32 quest_id )
{
    if( quest_id )
    {
        SetQuestStatus( quest_id, QUEST_STATUS_INCOMPLETE );

        uint16 log_slot = FindQuestSlot( quest_id );
        if( log_slot < MAX_QUEST_LOG_SIZE)
            RemoveQuestSlotState(log_slot,QUEST_STATE_COMPLETE);
    }
}

void Player::RewardQuest(Quest const *pQuest, uint32 reward, Object* questGiver, bool announce)
{
    //this THING should be here to protect code from quest, which cast on player far teleport as a reward
    //should work fine, cause far teleport will be executed in Player::Update()
    SetCanDelayTeleport(true);

    uint32 quest_id = pQuest->GetQuestId();

    for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++ )
    {
        if ( pQuest->RequiredItemId[i] )
            DestroyItemCount( pQuest->RequiredItemId[i], pQuest->RequiredItemCount[i], true);
    }

    //if( qInfo->HasSpecialFlag( QUEST_FLAGS_TIMED ) )
    //    SetTimedQuest( 0 );
    m_timedquests.erase(pQuest->GetQuestId());

    if ( pQuest->GetRewardItemsCount() > 0 )
    {
        for (uint32 i=0; i < pQuest->GetRewardItemsCount(); ++i)
        {
            if( pQuest->RewardItemId[i] )
            {
                ItemPosCountVec dest;
                if( CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, pQuest->RewardItemId[i], pQuest->RewardItemIdCount[i] ) == EQUIP_ERR_OK )
                {
                    Item* item = StoreNewItem( dest, pQuest->RewardItemId[i], true);
#ifdef LICH_KING
                    SendNewItem(item, pQuest->RewardItemIdCount[i], true, false, false, false);
#else
                    SendNewItem(item, pQuest->RewardItemIdCount[i], true, false, false, true);
#endif
                }
            }
        }
    }

    if (pQuest->GetRewardChoiceItemsCount() > 0)
    {
        if (pQuest->RewardChoiceItemId[reward])
        {
            ItemPosCountVec dest;
            if (CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pQuest->RewardChoiceItemId[reward], pQuest->RewardChoiceItemCount[reward]) == EQUIP_ERR_OK)
            {
                Item* item = StoreNewItem(dest, pQuest->RewardChoiceItemId[reward], true);
#ifdef LICH_KING
                SendNewItem(item, pQuest->RewardChoiceItemCount[reward], true, false, false, false);
#else
                SendNewItem(item, pQuest->RewardChoiceItemCount[reward], true, false, false, true);
#endif
            }
        }
    }

    if( pQuest->GetRewSpellCast() > 0 )
        CastSpell( this, pQuest->GetRewSpellCast(), true);
    else if( pQuest->GetRewSpell() > 0)
        CastSpell( this, pQuest->GetRewSpell(), true);

    uint16 log_slot = FindQuestSlot( quest_id );
    if( log_slot < MAX_QUEST_LOG_SIZE)
        SetQuestSlot(log_slot,0);

    QuestStatusData& q_status = m_QuestStatus[quest_id];

    // Not give XP in case already completed once repeatable quest
    uint32 XP = 0;
    if (hasCustomXpRate())
        XP = q_status.Rewarded ? 0 : uint32(pQuest->XPValue( this )*m_customXp);
    else
        XP = q_status.Rewarded ? 0 : uint32(pQuest->XPValue( this )*sWorld->GetRate(RATE_XP_QUEST));

    if ( GetLevel() < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) )
        GiveXP( XP , nullptr );
    else
        ModifyMoney( int32(pQuest->GetRewMoneyMaxLevel() * sWorld->GetRate(RATE_DROP_MONEY)) );

    // Give player extra money if GetRewOrReqMoney > 0 and get ReqMoney if negative
    ModifyMoney( pQuest->GetRewOrReqMoney() );

        // honor reward
    if(pQuest->GetRewHonorableKills())
    RewardHonor(nullptr, 0, Trinity::Honor::hk_honor_at_level(GetLevel(), pQuest->GetRewHonorableKills()));

    RewardReputation(pQuest);

    // title reward
    if(pQuest->GetCharTitleId())
    {
        if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(pQuest->GetCharTitleId()))
            SetTitle(titleEntry,true);
    }

    // Send reward mail
    if (uint32 mail_template_id = pQuest->GetRewMailTemplateId())
    {
        /// @todo Poor design of mail system
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        if (uint32 questMailSender = pQuest->GetRewMailSenderEntry())
            MailDraft(mail_template_id).SendMailTo(trans, this, questMailSender, MAIL_CHECK_MASK_HAS_BODY, pQuest->GetRewMailDelaySecs());
        else
            MailDraft(mail_template_id).SendMailTo(trans, this, questGiver, MAIL_CHECK_MASK_HAS_BODY, pQuest->GetRewMailDelaySecs());
        CharacterDatabase.CommitTransaction(trans);
    }

    if(pQuest->IsDaily())
        SetDailyQuestStatus(quest_id);

    if ( !pQuest->IsRepeatable() )
        SetQuestStatus(quest_id, QUEST_STATUS_COMPLETE);
    else
        SetQuestStatus(quest_id, QUEST_STATUS_NONE);

    q_status.Rewarded = true;

    if(announce)
        SendQuestReward( pQuest, XP, questGiver );

    if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

    //lets remove flag for delayed teleports
    SetCanDelayTeleport(false);
}

void Player::FailQuest( uint32 questId )
{
    if(!questId)
        return;

    // Already complete quests shouldn't turn failed.
    if (GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        return;

    SetQuestStatus(questId, QUEST_STATUS_FAILED);

    uint16 log_slot = FindQuestSlot( questId );
    if( log_slot < MAX_QUEST_LOG_SIZE)
    {
        SetQuestSlotTimer(log_slot, 1 );
        SetQuestSlotState(log_slot,QUEST_STATE_FAIL);
    }
    SendQuestFailed( questId );
}

void Player::AbandonQuest(uint32 questId)
{
    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
            if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->RequiredItemId[i]))
                if (quest->RequiredItemCount[i] > 0 && itemTemplate->Bonding == BIND_QUEST_ITEM)
                    DestroyItemCount(quest->RequiredItemId[i], quest->RequiredItemCount[i], true);

#ifdef LICH_KING
        for (uint8 i = 0; i < QUEST_SOURCE_ITEM_IDS_COUNT; ++i)
            if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(quest->ItemDrop[i]))
                if (quest->ItemDropQuantity[i] > 0 && itemTemplate->Bonding == BIND_QUEST_ITEM)
                    DestroyItemCount(quest->ItemDrop[i], 9999, true);
#endif
    }
}

void Player::FailTimedQuest( uint32 quest_id )
{
    if( quest_id )
    {
        QuestStatusData& q_status = m_QuestStatus[quest_id];

        if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;
        q_status.m_timer = 0;

        IncompleteQuest( quest_id );

        uint16 log_slot = FindQuestSlot( quest_id );
        if( log_slot < MAX_QUEST_LOG_SIZE)
        {
            SetQuestSlotTimer(log_slot, 1 );
            SetQuestSlotState(log_slot,QUEST_STATE_FAIL);
        }
        SendQuestTimerFailed( quest_id );
    }
}

bool Player::SatisfyQuestSkillOrClass( Quest const* qInfo, bool msg )
{
    int32 ZoneOrSort   = qInfo->GetZoneOrSort();
    int32 skillOrClass = qInfo->GetSkillOrClass();

    // skip zone ZoneOrSort and 0 case skillOrClass
    if( ZoneOrSort >= 0 && skillOrClass == 0 )
        return true;

    int32 questSort = -ZoneOrSort;
    uint8 reqSortClass = ClassByQuestSort(questSort);

    // check class sort cases in ZoneOrSort
    if( reqSortClass != 0 && GetClass() != reqSortClass)
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
        return false;
    }

    // check class
    if( skillOrClass < 0 )
    {
        uint32 reqClass = -int32(skillOrClass);
        if((GetClassMask() & reqClass) == 0)
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
    }
    // check skill
    else if( skillOrClass > 0 )
    {
        uint32 reqSkill = skillOrClass;
        if( GetSkillValue( reqSkill ) < qInfo->GetRequiredSkillValue() )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
    }

    return true;
}

bool Player::SatisfyQuestLevel( Quest const* qInfo, bool msg )
{
    if( GetLevel() < qInfo->GetMinLevel() )
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
        return false;
    }
    return true;
}

bool Player::SatisfyQuestLog( bool msg )
{
    // exist free slot
    if( FindQuestSlot(0) < MAX_QUEST_LOG_SIZE )
        return true;

    if( msg )
    {
        WorldPacket data( SMSG_QUESTLOG_FULL, 0 );
        SendDirectMessage( &data );
    }
    return false;
}

bool Player::SatisfyQuestPreviousQuest( Quest const* qInfo, bool msg )
{
    // No previous quest (might be first quest in a series)
    if( qInfo->prevQuests.empty())
        return true;

    for(int prevQuest : qInfo->prevQuests)
    {
        uint32 prevId = abs(prevQuest);

        auto i_prevstatus = m_QuestStatus.find( prevId );
        Quest const* qPrevInfo = sObjectMgr->GetQuestTemplate(prevId);

        if( qPrevInfo && i_prevstatus != m_QuestStatus.end() )
        {
            // If any of the positive previous quests completed, return true
            if( prevQuest > 0 && i_prevstatus->second.Rewarded )
            {
                // skip one-from-all exclusive group
                if(qPrevInfo->GetExclusiveGroup() >= 0)
                    return true;

                // each-from-all exclusive group ( < 0)
                // can be start if only all quests in prev quest exclusive group completed and rewarded
                auto iter = sObjectMgr->mExclusiveQuestGroups.lower_bound(qPrevInfo->GetExclusiveGroup());
                auto end  = sObjectMgr->mExclusiveQuestGroups.upper_bound(qPrevInfo->GetExclusiveGroup());

                assert(iter!=end);                          // always must be found if qPrevInfo->ExclusiveGroup != 0

                for(; iter != end; ++iter)
                {
                    uint32 exclude_Id = iter->second;

                    // skip checked quest id, only state of other quests in group is interesting
                    if(exclude_Id == prevId)
                        continue;

                    auto i_exstatus = m_QuestStatus.find( exclude_Id );

                    // alternative quest from group also must be completed and rewarded(reported)
                    if( i_exstatus == m_QuestStatus.end() || !i_exstatus->second.Rewarded )
                    {
                        if( msg )
                            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
                        return false;
                    }
                }
                return true;
            }
            // If any of the negative previous quests active, return true
            if( prevQuest < 0 && (i_prevstatus->second.Status == QUEST_STATUS_INCOMPLETE
                || (i_prevstatus->second.Status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(prevId))))
            {
                // skip one-from-all exclusive group
                if(qPrevInfo->GetExclusiveGroup() >= 0)
                    return true;

                // each-from-all exclusive group ( < 0)
                // can be start if only all quests in prev quest exclusive group active
                auto iter = sObjectMgr->mExclusiveQuestGroups.lower_bound(qPrevInfo->GetExclusiveGroup());
                auto end  = sObjectMgr->mExclusiveQuestGroups.upper_bound(qPrevInfo->GetExclusiveGroup());

                assert(iter!=end);                          // always must be found if qPrevInfo->ExclusiveGroup != 0

                for(; iter != end; ++iter)
                {
                    uint32 exclude_Id = iter->second;

                    // skip checked quest id, only state of other quests in group is interesting
                    if(exclude_Id == prevId)
                        continue;

                    auto i_exstatus = m_QuestStatus.find( exclude_Id );

                    // alternative quest from group also must be active
                    if( i_exstatus == m_QuestStatus.end() ||
                        (i_exstatus->second.Status != QUEST_STATUS_INCOMPLETE &&
                        (i_prevstatus->second.Status != QUEST_STATUS_COMPLETE || GetQuestRewardStatus(prevId))) )
                    {
                        if( msg )
                            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
                        return false;
                    }
                }
                return true;
            }
        }
    }

    // Has only positive prev. quests in non-rewarded state
    // and negative prev. quests in non-active state
    if( msg )
        SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );

    return false;
}

bool Player::SatisfyQuestRace( Quest const* qInfo, bool msg )
{
    uint32 reqraces = qInfo->GetRequiredRaces();
    if ( reqraces == 0 )
        return true;
    if( (reqraces & GetRaceMask()) == 0 )
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_QUEST_FAILED_WRONG_RACE );
        return false;
    }
    return true;
}

bool Player::SatisfyQuestReputation( Quest const* qInfo, bool msg )
{
    uint32 fIdMin = qInfo->GetRequiredMinRepFaction();      //Min required rep
    if(fIdMin && GetReputation(fIdMin) < qInfo->GetRequiredMinRepValue())
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
        return false;
    }

    uint32 fIdMax = qInfo->GetRequiredMaxRepFaction();      //Max required rep
    if(fIdMax && GetReputation(fIdMax) >= qInfo->GetRequiredMaxRepValue())
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
        return false;
    }

    return true;
}

bool Player::SatisfyQuestStatus(Quest const* qInfo, bool msg)
{
    if (GetQuestStatus(qInfo->GetQuestId()) == QUEST_STATUS_REWARDED)
    {
        if (msg)
        {
            SendCanTakeQuestResponse(INVALIDREASON_QUEST_ALREADY_DONE);
            TC_LOG_DEBUG("misc", "Player::SatisfyQuestStatus: Sent QUEST_STATUS_REWARDED (QuestID: %u) because player '%s' (%u) quest status is already REWARDED.",
                    qInfo->GetQuestId(), GetName().c_str(), GetGUID().GetCounter());
        }
        return false;
    }

    auto itr = m_QuestStatus.find(qInfo->GetQuestId());
    if  ( itr != m_QuestStatus.end() && itr->second.Status != QUEST_STATUS_NONE )
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_QUEST_ALREADY_ON );
        return false;
    }
    return true;
}

bool Player::SatisfyQuestConditions(Quest const* qInfo, bool msg)
{
    if (!sConditionMgr->IsObjectMeetingNotGroupedConditions(CONDITION_SOURCE_TYPE_QUEST_ACCEPT, qInfo->GetQuestId(), this))
    {
        if (msg)
        {
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
            TC_LOG_DEBUG("misc", "SatisfyQuestConditions: Sent INVALIDREASON_DONT_HAVE_REQ (questId: %u) because player does not meet conditions.", qInfo->GetQuestId());
        }
        TC_LOG_DEBUG("condition", "Player::SatisfyQuestConditions: conditions not met for quest %u", qInfo->GetQuestId());
        return false;
    }
    return true;
}

bool Player::SatisfyQuestTimed( Quest const* qInfo, bool msg )
{
    if ( (find(m_timedquests.begin(), m_timedquests.end(), qInfo->GetQuestId()) != m_timedquests.end()) && qInfo->HasFlag(QUEST_TRINITY_FLAGS_TIMED) )
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_QUEST_ONLY_ONE_TIMED );
        return false;
    }
    return true;
}

bool Player::SatisfyQuestExclusiveGroup( Quest const* qInfo, bool msg )
{
    // non positive exclusive group, if > 0 then can be start if any other quest in exclusive group already started/completed
    if(qInfo->GetExclusiveGroup() <= 0)
        return true;

    auto iter = sObjectMgr->mExclusiveQuestGroups.lower_bound(qInfo->GetExclusiveGroup());
    auto end  = sObjectMgr->mExclusiveQuestGroups.upper_bound(qInfo->GetExclusiveGroup());

    assert(iter!=end);                                      // always must be found if qInfo->ExclusiveGroup != 0

    for(; iter != end; ++iter)
    {
        uint32 exclude_Id = iter->second;

        // skip checked quest id, only state of other quests in group is interesting
        if(exclude_Id == qInfo->GetQuestId())
            continue;

        // not allow have daily quest if daily quest from exclusive group already recently completed
        Quest const* Nquest = sObjectMgr->GetQuestTemplate(exclude_Id);
        if( !SatisfyQuestDay(Nquest, false) )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }

        auto i_exstatus = m_QuestStatus.find( exclude_Id );

        // alternative quest already started or completed
        if( i_exstatus != m_QuestStatus.end()
            && (i_exstatus->second.Status == QUEST_STATUS_COMPLETE || i_exstatus->second.Status == QUEST_STATUS_INCOMPLETE) )
        {
            if( msg )
                SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
            return false;
        }
    }
    return true;
}

bool Player::SatisfyQuestNextChain( Quest const* qInfo, bool msg )
{
    if(!qInfo->GetNextQuestInChain())
        return true;

    // next quest in chain already started or completed
    auto itr = m_QuestStatus.find( qInfo->GetNextQuestInChain() );
    if( itr != m_QuestStatus.end()
        && (itr->second.Status == QUEST_STATUS_COMPLETE || itr->second.Status == QUEST_STATUS_INCOMPLETE) )
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
        return false;
    }

    // check for all quests further up the chain
    // only necessary if there are quest chains with more than one quest that can be skipped
    //return SatisfyQuestNextChain( qInfo->GetNextQuestInChain(), msg );
    return true;
}

bool Player::SatisfyQuestPrevChain( Quest const* qInfo, bool msg )
{
    // No previous quest in chain
    if( qInfo->prevChainQuests.empty())
        return true;

    for(uint32 prevId : qInfo->prevChainQuests)
    {
        auto i_prevstatus = m_QuestStatus.find( prevId );

        if( i_prevstatus != m_QuestStatus.end() )
        {
            // If any of the previous quests in chain active, return false
            if( i_prevstatus->second.Status == QUEST_STATUS_INCOMPLETE
                || (i_prevstatus->second.Status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(prevId)))
            {
                if( msg )
                    SendCanTakeQuestResponse( INVALIDREASON_DONT_HAVE_REQ );
                return false;
            }
        }

        // check for all quests further down the chain
        // only necessary if there are quest chains with more than one quest that can be skipped
        //if( !SatisfyQuestPrevChain( prevId, msg ) )
        //    return false;
    }

    // No previous quest in chain active
    return true;
}

bool Player::SatisfyQuestDay( Quest const* qInfo, bool msg )
{
    if(!qInfo->IsDaily())
        return true;

    bool have_slot = false;
    for(uint32 quest_daily_idx = 0; quest_daily_idx < PLAYER_MAX_DAILY_QUESTS; ++quest_daily_idx)
    {
        uint32 id = GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx);
        if(qInfo->GetQuestId()==id)
            return false;

        if(!id)
            have_slot = true;
    }

    if(!have_slot)
    {
        if( msg )
            SendCanTakeQuestResponse( INVALIDREASON_DAILY_QUESTS_REMAINING );
        return false;
    }

    return true;
}

bool Player::GiveQuestSourceItem( Quest const *pQuest )
{
    uint32 srcitem = pQuest->GetSrcItemId();
    if( srcitem > 0 )
    {
        uint32 count = pQuest->GetSrcItemCount();
        if( count <= 0 )
            count = 1;

        ItemPosCountVec dest;
        uint8 msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, srcitem, count );
        if( msg == EQUIP_ERR_OK )
        {
            Item * item = StoreNewItem(dest, srcitem, true);
            SendNewItem(item, count, true, false);
            return true;
        }
        // player already have max amount required item, just report success
        else if( msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS )
            return true;
        else
            SendEquipError( msg, nullptr, nullptr );
        return false;
    }

    return true;
}

bool Player::TakeQuestSourceItem( uint32 quest_id, bool msg )
{
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest_id);
    if( qInfo )
    {
        uint32 srcitem = qInfo->GetSrcItemId();
        if( srcitem > 0 )
        {
            uint32 count = qInfo->GetSrcItemCount();
            if( count <= 0 )
                count = 1;

            // exist one case when destroy source quest item not possible:
            // non un-equippable item (equipped non-empty bag, for example)
            uint8 res = CanUnequipItems(srcitem,count);
            if(res != EQUIP_ERR_OK)
            {
                if(msg)
                    SendEquipError( res, nullptr, nullptr );
                return false;
            }

            DestroyItemCount(srcitem, count, true, true, true);
        }
    }
    return true;
}

bool Player::GetQuestRewardStatus( uint32 quest_id ) const
{
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest_id);
    if( qInfo )
    {
        // for repeatable quests: rewarded field is set after first reward only to prevent getting XP more than once
        auto itr = m_QuestStatus.find( quest_id );
        if( itr != m_QuestStatus.end() && itr->second.Status != QUEST_STATUS_NONE
            && !qInfo->IsRepeatable() )
            return itr->second.Rewarded;

        return false;
    }
    return false;
}

QuestStatus Player::GetQuestStatus( uint32 quest_id ) const
{
    if( quest_id )
    {
        auto itr = m_QuestStatus.find( quest_id );
        if( itr != m_QuestStatus.end() )
            return itr->second.Status;
    }

    if (Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest_id))
    {
        /* NYI
        if (qInfo->IsSeasonal() && !qInfo->IsRepeatable())
        {
            uint16 eventId = sGameEventMgr->GetEventIdForQuest(qInfo);
            auto seasonalQuestItr = m_seasonalquests.find(eventId);
            if (seasonalQuestItr == m_seasonalquests.end() || seasonalQuestItr->second.find(quest_id) == seasonalQuestItr->second.end())
                return QUEST_STATUS_NONE;
        }
        */

        if (!qInfo->IsRepeatable() && IsQuestRewarded(quest_id))
            return QUEST_STATUS_REWARDED;
    }

    return QUEST_STATUS_NONE;
}

bool Player::CanShareQuest(uint32 quest_id) const
{
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest_id);
    if( qInfo && qInfo->HasFlag(QUEST_FLAGS_SHARABLE) )
    {
        auto itr = m_QuestStatus.find( quest_id );
        if( itr != m_QuestStatus.end() )
        {
            // in pool and not currently available (wintergrasp weekly, dalaran weekly) - can't share
            /*
            if (sPoolMgr->IsPartOfAPool<Quest>(quest_id) && !sPoolMgr->IsSpawnedObject<Quest>(quest_id))
            {
                SendPushToPartyResponse(this, QUEST_PARTY_MSG_CANT_BE_SHARED_TODAY);
                return false;
            }
            TC */

            return true;
        }
    }
    return false;
}

void Player::SetQuestStatus( uint32 questId, QuestStatus status )
{
    uint32 zone = 0, area = 0;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
    if( qInfo )
    {
        if( status == QUEST_STATUS_NONE || status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE )
        {
            if( qInfo->HasFlag( QUEST_TRINITY_FLAGS_TIMED ) )
                m_timedquests.erase(qInfo->GetQuestId());
        }

        QuestStatusData& q_status = m_QuestStatus[questId];

        q_status.Status = status;
        if (q_status.uState != QUEST_NEW)
            q_status.uState = QUEST_CHANGED;
    }

    SpellAreaForQuestMapBounds saBounds = sSpellMgr->GetSpellAreaForQuestMapBounds(questId);
    if (saBounds.first != saBounds.second)
    {
        GetZoneAndAreaId(zone, area);

        for (auto itr = saBounds.first; itr != saBounds.second; ++itr)
            if (itr->second->autocast && itr->second->IsFitToRequirements(this, zone, area))
                if (!HasAura(itr->second->spellId))
                    CastSpell(this, itr->second->spellId, TRIGGERED_FULL_MASK);
    }

    saBounds = sSpellMgr->GetSpellAreaForQuestEndMapBounds(questId);
    if (saBounds.first != saBounds.second)
    {
        if (!zone || !area)
            GetZoneAndAreaId(zone, area);

        for (auto itr = saBounds.first; itr != saBounds.second; ++itr)
            if (!itr->second->IsFitToRequirements(this, zone, area))
                RemoveAurasDueToSpell(itr->second->spellId);
    }

    UpdateForQuestWorldObjects();
}

// not used in TrinIty, but used in scripting code
uint32 Player::GetReqKillOrCastCurrentCount(uint32 quest_id, int32 entry)
{
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest_id);
    if( !qInfo )
        return 0;

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
        if ( qInfo->RequiredNpcOrGo[j] == entry )
            return m_QuestStatus[quest_id].CreatureOrGOCount[j];

    return 0;
}

void Player::AdjustQuestRequiredItemCount( Quest const* pQuest )
{
    if ( pQuest->HasFlag( QUEST_TRINITY_FLAGS_DELIVER ) )
    {
        for(int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; i++)
        {
            uint32 reqitemcount = pQuest->RequiredItemCount[i];
            if( reqitemcount != 0 )
            {
                uint32 quest_id = pQuest->GetQuestId();
                uint32 curitemcount = GetItemCount(pQuest->RequiredItemId[i],true);

                QuestStatusData& q_status = m_QuestStatus[quest_id];
                q_status.ItemCount[i] = std::min(curitemcount, reqitemcount);
                if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;
            }
        }
    }
}

uint16 Player::FindQuestSlot( uint32 quest_id ) const
{
    for ( uint16 i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
        if ( GetQuestSlotQuestId(i) == quest_id )
            return i;

    return MAX_QUEST_LOG_SIZE;
}

void Player::AreaExploredOrEventHappens( uint32 questId )
{
    if( questId )
    {
        uint16 log_slot = FindQuestSlot( questId );
        if( log_slot < MAX_QUEST_LOG_SIZE)
        {
            QuestStatusData& q_status = m_QuestStatus[questId];

            if(!q_status.Explored)
            {
                q_status.Explored = true;
                if (q_status.uState != QUEST_NEW)
                    q_status.uState = QUEST_CHANGED;
            }
        }
        if( CanCompleteQuest( questId ) )
            CompleteQuest( questId );
    }
}

//not used in Trinityd, function for external script library
void Player::GroupEventHappens( uint32 questId, WorldObject const* pEventObject )
{
    if( Group *pGroup = GetGroup() )
    {
        for(GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player *pGroupGuy = itr->GetSource();

            // for any leave or dead (with not released body) group member at appropriate distance
            if( pGroupGuy && pGroupGuy->IsAtGroupRewardDistance(pEventObject) && !pGroupGuy->GetCorpse() )
                pGroupGuy->AreaExploredOrEventHappens(questId);
        }
    }
    else
        AreaExploredOrEventHappens(questId);
}

void Player::ItemAddedQuestCheck( uint32 entry, uint32 count )
{
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if ( questid == 0 )
            continue;

        QuestStatusData& q_status = m_QuestStatus[questid];

        if ( q_status.Status != QUEST_STATUS_INCOMPLETE )
            continue;

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid);
        if( !qInfo || !qInfo->HasFlag( QUEST_TRINITY_FLAGS_DELIVER ) )
            continue;

        for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; j++)
        {
            uint32 reqitem = qInfo->RequiredItemId[j];
            if ( reqitem == entry )
            {
                uint32 reqitemcount = qInfo->RequiredItemCount[j];
                uint32 curitemcount = q_status.ItemCount[j];
                if ( curitemcount < reqitemcount )
                {
                    uint32 additemcount = ( curitemcount + count <= reqitemcount ? count : reqitemcount - curitemcount);
                    q_status.ItemCount[j] += additemcount;
                    if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                    SendQuestUpdateAddItem( qInfo, j, additemcount );
                }
                if (CanCompleteQuest(questid))
                    CompleteQuest(questid);
                return;
            }
        }
    }
    UpdateForQuestWorldObjects();
}

void Player::ItemRemovedQuestCheck( uint32 entry, uint32 count )
{
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if(!questid)
            continue;
        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid);
        if ( !qInfo )
            continue;
        if( !qInfo->HasFlag( QUEST_TRINITY_FLAGS_DELIVER ) )
            continue;

        for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; j++)
        {
            uint32 reqitem = qInfo->RequiredItemId[j];
            if ( reqitem == entry )
            {
                QuestStatusData& q_status = m_QuestStatus[questid];

                uint32 reqitemcount = qInfo->RequiredItemCount[j];
                uint32 curitemcount;
                if( q_status.Status != QUEST_STATUS_COMPLETE )
                    curitemcount = q_status.ItemCount[j];
                else
                    curitemcount = GetItemCount(entry,true);
                if ( curitemcount < reqitemcount + count )
                {
                    uint32 remitemcount = ( curitemcount <= reqitemcount ? count : count + reqitemcount - curitemcount);
                    q_status.ItemCount[j] = curitemcount - remitemcount;
                    if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                    IncompleteQuest( questid );
                }
                return;
            }
        }
    }
    UpdateForQuestWorldObjects();
}

void Player::KilledMonsterCredit(uint32 entry, ObjectGuid guid, uint32 questId)
{
    uint32 addkillcount = 1;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if(!questid)
            continue;

        if (questId && questid != questId)
            continue;

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid);
        if( !qInfo )
            continue;
        // just if !ingroup || !noraidgroup || raidgroup
        QuestStatusData& q_status = m_QuestStatus[questid];
        if( q_status.Status == QUEST_STATUS_INCOMPLETE && (!GetGroup() || !GetGroup()->isRaidGroup() || qInfo->GetType() == QUEST_TYPE_RAID))
        {
            if( qInfo->HasFlag( QUEST_TRINITY_FLAGS_KILL_OR_CAST) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    // skip GO activate objective or none
                    if(qInfo->RequiredNpcOrGo[j] <=0)
                        continue;

                    // skip Cast at creature objective
                    if(qInfo->ReqSpell[j] !=0 )
                        continue;

                    uint32 reqkill = qInfo->RequiredNpcOrGo[j];

                    if ( reqkill == entry )
                    {
                        uint32 reqkillcount = qInfo->RequiredNpcOrGoCount[j];
                        uint32 curkillcount = q_status.CreatureOrGOCount[j];
                        if ( curkillcount < reqkillcount )
                        {
                            q_status.CreatureOrGOCount[j] = curkillcount + addkillcount;
                            if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreatureOrGo( qInfo, guid, j, curkillcount, addkillcount);
                        }
                        if ( CanCompleteQuest( questid ) )
                            CompleteQuest( questid );

                        // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                        continue;
                    }
                }
            }
        }
    }
}

void Player::ActivatedGO(uint32 entry, ObjectGuid guid)
{
    uint32 addkillcount = 1;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if(!questid)
            continue;

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid);
        if( !qInfo )
            continue;
        // just if !ingroup || !noraidgroup || raidgroup
        QuestStatusData& q_status = m_QuestStatus[questid];
        if( q_status.Status == QUEST_STATUS_INCOMPLETE && (!GetGroup() || !GetGroup()->isRaidGroup() || qInfo->GetType() == QUEST_TYPE_RAID))
        {
            if( qInfo->HasFlag( QUEST_TRINITY_FLAGS_KILL_OR_CAST) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    // skip GO activate objective or none
                    if(qInfo->RequiredNpcOrGo[j] >= 0)
                        continue;

                    // skip Cast at creature objective
                    if(qInfo->ReqSpell[j] !=0 )
                        continue;

                    int32 reqkill = qInfo->RequiredNpcOrGo[j];

                    if ( -reqkill == int32(entry) )
                    {
                        uint32 reqkillcount = qInfo->RequiredNpcOrGoCount[j];
                        uint32 curkillcount = q_status.CreatureOrGOCount[j];
                        if ( curkillcount < reqkillcount )
                        {
                            q_status.CreatureOrGOCount[j] = curkillcount + addkillcount;
                            if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreatureOrGo( qInfo, guid, j, curkillcount, addkillcount);
                        }
                        if ( CanCompleteQuest( questid ) )
                            CompleteQuest( questid );

                        // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                        continue;
                    }
                }
            }
        }
    }
}

void Player::CastedCreatureOrGO( uint32 entry, ObjectGuid guid, uint32 spell_id )
{
    bool isCreature = guid.IsCreature();
    if (!guid)
        isCreature = true;

    uint32 addCastCount = 1;
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if(!questid)
            continue;

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid);
        if ( !qInfo  )
            continue;

        QuestStatusData& q_status = m_QuestStatus[questid];

        if ( q_status.Status == QUEST_STATUS_INCOMPLETE )
        {
            if( qInfo->HasFlag( QUEST_TRINITY_FLAGS_KILL_OR_CAST ) )
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    // skip kill creature objective (0) or wrong spell casts
                    if(qInfo->ReqSpell[j] != spell_id )
                        continue;

                    uint32 reqTarget = 0;

                    if(isCreature)
                    {
                        // creature activate objectives
                        if(qInfo->RequiredNpcOrGo[j] > 0)
                            // checked at quest_template loading
                            reqTarget = qInfo->RequiredNpcOrGo[j];
                    }
                    else
                    {
                        // GO activate objective
                        if(qInfo->RequiredNpcOrGo[j] < 0)
                            // checked at quest_template loading
                            reqTarget = - qInfo->RequiredNpcOrGo[j];
                    }

                    // other not this creature/GO related objectives
                    if( reqTarget != entry )
                        continue;

                    uint32 reqCastCount = qInfo->RequiredNpcOrGoCount[j];
                    uint32 curCastCount = q_status.CreatureOrGOCount[j];
                    if ( curCastCount < reqCastCount )
                    {
                        q_status.CreatureOrGOCount[j] = curCastCount + addCastCount;
                        if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                        SendQuestUpdateAddCreatureOrGo( qInfo, guid, j, curCastCount, addCastCount);
                    }

                    if ( CanCompleteQuest( questid ) )
                        CompleteQuest( questid );

                    // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                    break;
                }
            }
        }
    }
}

void Player::TalkedToCreature( uint32 entry, ObjectGuid guid )
{
    //here was quest objectives "speak to validation", but was removed since it broke some quests and the few ones that were using this were fixed otherwise
}

void Player::MoneyChanged( uint32 count )
{
    for( int i = 0; i < MAX_QUEST_LOG_SIZE; i++ )
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid);
        if( qInfo && qInfo->GetRewOrReqMoney() < 0 )
        {
            QuestStatusData& q_status = m_QuestStatus[questid];

            if( q_status.Status == QUEST_STATUS_INCOMPLETE )
            {
                if(int32(count) >= -qInfo->GetRewOrReqMoney())
                {
                    if ( CanCompleteQuest( questid ) )
                        CompleteQuest( questid );
                }
            }
            else if( q_status.Status == QUEST_STATUS_COMPLETE )
            {
                if(int32(count) < -qInfo->GetRewOrReqMoney())
                    IncompleteQuest( questid );
            }
        }
    }
}

void Player::ReputationChanged(FactionEntry const* factionEntry)
{
    for (uint8 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        if (uint32 questid = GetQuestSlotQuestId(i))
        {
            if (Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid))
            {
                if (qInfo->GetRepObjectiveFaction() == factionEntry->ID)
                {
                    QuestStatusData& q_status = m_QuestStatus[questid];
                    if (q_status.Status == QUEST_STATUS_INCOMPLETE)
                    {
                        if (GetReputationMgr().GetReputation(factionEntry) >= qInfo->GetRepObjectiveValue())
                            if (CanCompleteQuest(questid))
                                CompleteQuest(questid);
                    }
                    else if (q_status.Status == QUEST_STATUS_COMPLETE)
                    {
                        if (GetReputationMgr().GetReputation(factionEntry) < qInfo->GetRepObjectiveValue())
                            IncompleteQuest(questid);
                    }
                }
            }
        }
    }
}

bool Player::HasQuestForItem( uint32 itemid ) const
{
    // Workaround for quests 7810/7838
    if (itemid == 18706)
        return true;
    for(const auto & m_QuestStatu : m_QuestStatus)
    {
        QuestStatusData const& q_status = m_QuestStatu.second;

        if (q_status.Status == QUEST_STATUS_INCOMPLETE)
        {
            Quest const* qinfo = sObjectMgr->GetQuestTemplate(m_QuestStatu.first);
            if(!qinfo)
                continue;

            // hide quest if player is in raid-group and quest is no raid quest
            if(GetGroup() && GetGroup()->isRaidGroup() && qinfo->GetType() != QUEST_TYPE_RAID)
                if(!InBattleground()) //there are two ways.. we can make every bg-quest a raidquest, or add this code here.. i don't know if this can be exploited by other quests, but i think all other quests depend on a specific area.. but keep this in mind, if something strange happens later
                    continue;

            // There should be no mixed ReqItem/ReqSource drop
            // This part for ReqItem drop
            for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; j++)
            {
                if(itemid == qinfo->RequiredItemId[j] && q_status.ItemCount[j] < qinfo->RequiredItemCount[j] )
                    return true;
            }

            // This part - for ReqSource
            for (int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; j++)
            {
                // examined item is a source item
                if (qinfo->RequiredSourceItemId[j] == itemid && qinfo->ReqSourceRef[j] > 0 && qinfo->ReqSourceRef[j] <= QUEST_OBJECTIVES_COUNT)
                {
                    uint32 idx = qinfo->ReqSourceRef[j]-1;

                    // total count of created ReqItems and SourceItems is less than RequiredItemCount
                    if(qinfo->RequiredItemId[idx] != 0 &&
                        q_status.ItemCount[idx] * qinfo->RequiredSourceItemCount[j] + GetItemCount(itemid,true) < qinfo->RequiredItemCount[idx] * qinfo->RequiredSourceItemCount[j])
                        return true;

                    // total count of casted ReqCreatureOrGOs and SourceItems is less than RequiredNpcOrGoCount
                    if (qinfo->RequiredNpcOrGo[idx] != 0)
                    {
                        if(q_status.CreatureOrGOCount[idx] * qinfo->RequiredSourceItemCount[j] + GetItemCount(itemid,true) < qinfo->RequiredNpcOrGoCount[idx] * qinfo->RequiredSourceItemCount[j])
                            return true;
                    }
                    // spell with SPELL_EFFECT_QUEST_COMPLETE or SPELL_EFFECT_SEND_EVENT (with script) case
                    else if(qinfo->ReqSpell[idx] != 0)
                    {
                        // not casted and need more reagents/item for use.
                        if(!q_status.Explored && GetItemCount(itemid,true) < qinfo->RequiredSourceItemCount[j])
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

void Player::SendQuestComplete( uint32 quest_id )
{
    if( quest_id )
    {
        WorldPacket data( SMSG_QUESTUPDATE_COMPLETE, 4 );
        data << uint32(quest_id);
        SendDirectMessage( &data );
    }
}

void Player::SendQuestReward( Quest const *pQuest, uint32 xp, Object * questGiver )
{
    uint32 questid = pQuest->GetQuestId();
    sGameEventMgr->HandleQuestComplete(questid);
    WorldPacket data( SMSG_QUESTGIVER_QUEST_COMPLETE, (4+4+4+4+4+4+pQuest->GetRewardItemsCount()*8) );
    data << questid;
    data << uint32(0x03);

    if ( GetLevel() < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) )
    {
        data << xp;
        data << uint32(pQuest->GetRewOrReqMoney());
    }
    else
    {
        data << uint32(0);
        data << uint32(pQuest->GetRewOrReqMoney() + int32(pQuest->GetRewMoneyMaxLevel() * sWorld->GetRate(RATE_DROP_MONEY)));
    }
    data << uint32(0);                                      // new 2.3.0, HonorPoints?
    data << uint32( pQuest->GetRewardItemsCount() );           // max is 5

    for (uint32 i = 0; i < pQuest->GetRewardItemsCount(); ++i)
    {
        if ( pQuest->RewardItemId[i] > 0 )
            data << pQuest->RewardItemId[i] << pQuest->RewardItemIdCount[i];
        else
            data << uint32(0) << uint32(0);
    }
    SendDirectMessage( &data );

    if (pQuest->GetQuestCompleteScript() != 0)
        GetMap()->ScriptsStart(sQuestEndScripts, pQuest->GetQuestCompleteScript(), questGiver, this);
}

void Player::SendQuestFailed( uint32 quest_id )
{
    if( quest_id )
    {
        WorldPacket data( SMSG_QUESTGIVER_QUEST_FAILED, 4 );
        data << quest_id;
        SendDirectMessage( &data );
    }
}

void Player::SendQuestTimerFailed( uint32 quest_id )
{
    if( quest_id )
    {
        WorldPacket data( SMSG_QUESTUPDATE_FAILEDTIMER, 4 );
        data << quest_id;
        SendDirectMessage( &data );
    }
}

void Player::SendCanTakeQuestResponse( uint32 msg )
{
    WorldPacket data( SMSG_QUESTGIVER_QUEST_INVALID, 4 );
    data << uint32(msg);
    SendDirectMessage( &data );
}

void Player::SendQuestConfirmAccept(const Quest* pQuest, Player* pReceiver)
{
    if (pReceiver) {
        std::string strTitle = pQuest->GetTitle();

        LocaleConstant loc_idx = pReceiver->GetSession()->GetSessionDbcLocale();
        if (loc_idx != DEFAULT_LOCALE)
        {
            if (QuestLocale const* pLocale = sObjectMgr->GetQuestLocale(pQuest->GetQuestId()))
                if (pLocale->Title.size() > loc_idx && !pLocale->Title[loc_idx].empty())
                    strTitle = pLocale->Title[loc_idx];
        }

        WorldPacket data(SMSG_QUEST_CONFIRM_ACCEPT, (4 + strTitle.size() + 8)); // LK OK
        data << uint32(pQuest->GetQuestId());
        data << strTitle;
        data << uint64(GetGUID());
        pReceiver->SendDirectMessage(&data);
    }
}

void Player::SendPushToPartyResponse( Player *pPlayer, uint32 msg )
{
    if( pPlayer )
    {
        WorldPacket data( MSG_QUEST_PUSH_RESULT, (8+1) );
        data << uint64(pPlayer->GetGUID());
        data << uint8(msg);                                 // valid values: 0-8
        SendDirectMessage( &data );
    }
}

void Player::SendQuestUpdateAddItem( Quest const* pQuest, uint32 item_idx, uint32 count )
{
    WorldPacket data( SMSG_QUESTUPDATE_ADD_ITEM, (4+4) );
    data << pQuest->RequiredItemId[item_idx];
    data << count;
    SendDirectMessage( &data );
}

void Player::SendQuestUpdateAddCreatureOrGo( Quest const* pQuest, ObjectGuid guid, uint32 creatureOrGO_idx, uint32 old_count, uint32 add_count )
{
    assert(old_count + add_count < 256 && "mob/GO count store in 8 bits 2^8 = 256 (0..256)");

    int32 entry = pQuest->RequiredNpcOrGo[ creatureOrGO_idx ];
    if (entry < 0)
        // client expected gameobject template id in form (id|0x80000000)
        entry = (-entry) | 0x80000000;

    WorldPacket data( SMSG_QUESTUPDATE_ADD_KILL, (4*4+8) );
    data << uint32(pQuest->GetQuestId());
    data << uint32(entry);
    data << uint32(old_count + add_count);
    data << uint32(pQuest->RequiredNpcOrGoCount[ creatureOrGO_idx ]);
    data << uint64(guid);
    SendDirectMessage(&data);

    uint16 log_slot = FindQuestSlot( pQuest->GetQuestId() );
    if( log_slot < MAX_QUEST_LOG_SIZE)
        SetQuestSlotCounter(log_slot,creatureOrGO_idx,GetQuestSlotCounter(log_slot,creatureOrGO_idx)+add_count);
}

/*********************************************************/
/***                   LOAD SYSTEM                     ***/
/*********************************************************/

void Player::_LoadDeclinedNames(PreparedQueryResult result)
{
    if(!result)
        return;

    if(m_declinedname)
        delete m_declinedname;

    m_declinedname = new DeclinedName;
    Field *fields = result->Fetch();
    for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        m_declinedname->name[i] = fields[i].GetString();
}

void Player::_LoadArenaTeamInfo(PreparedQueryResult result)
{
    // arenateamid, played_week, played_season, PersonalRating
    memset((void*)&m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1], 0, sizeof(uint32)*18);
    if (!result)
        return;

    do
    {
        Field *fields = result->Fetch();

        uint32 arenateamid     = fields[0].GetUInt32();
        uint32 played_week     = fields[1].GetUInt16();
        uint32 played_season   = fields[2].GetUInt16();
        uint32 PersonalRating  = fields[3].GetUInt16();

        ArenaTeam* aTeam = sArenaTeamMgr->GetArenaTeamById(arenateamid);
        if(!aTeam)
        {
            TC_LOG_ERROR("entities.player","FATAL: couldn't load arenateam %u", arenateamid);
            continue;
        }
        uint8  arenaSlot = aTeam->GetSlot();

        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6]     = arenateamid;      // TeamID
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 1] = ((aTeam->GetCaptain() == GetGUID()) ? (uint32)0 : (uint32)1); // Captain 0, member 1
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 2] = played_week;      // Played Week
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 3] = played_season;    // Played Season
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 4] = 0;                // Unk
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 5] = PersonalRating;  // Personal Rating

    }while (result->NextRow());
}

void Player::_LoadBGData(PreparedQueryResult result)
{
    if (!result)
        return;

    Field* fields = result->Fetch();
    // Expecting only one row
    //        0           1     2      3      4      5      6          7          8        9
    // SELECT instanceId, team, joinX, joinY, joinZ, joinO, joinMapId, taxiStart, taxiEnd, mountSpell FROM character_battleground_data WHERE guid = ?

    m_bgData.bgInstanceID = fields[0].GetUInt32();
    m_bgData.bgTeam = fields[1].GetUInt16();
    m_bgData.joinPos = WorldLocation(fields[6].GetUInt16(),    // Map
        fields[2].GetFloat(),     // X
        fields[3].GetFloat(),     // Y
        fields[4].GetFloat(),     // Z
        fields[5].GetFloat());    // Orientation
    m_bgData.taxiPath[0] = fields[7].GetUInt32();
    m_bgData.taxiPath[1] = fields[8].GetUInt32();
    m_bgData.mountSpell = fields[9].GetUInt32();
}

bool Player::LoadPositionFromDB(uint32& mapid, float& x,float& y,float& z,float& o, bool& in_flight, ObjectGuid guid)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map,taxi_path FROM characters WHERE guid = '%u'",guid.GetCounter());
    if(!result)
        return false;

    Field *fields = result->Fetch();

    x = fields[0].GetFloat();
    y = fields[1].GetFloat();
    z = fields[2].GetFloat();
    o = fields[3].GetFloat();
    mapid = fields[4].GetUInt32();
    in_flight = !fields[5].GetString().empty();

    return true;
}

bool Player::LoadValuesArrayFromDB(Tokens& data, ObjectGuid guid)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT data FROM characters WHERE guid='%u'",guid.GetCounter());
    if( !result )
        return false;

    Field *fields = result->Fetch();

    data = StrSplit(fields[0].GetString(), " ");

    return true;
}

uint32 Player::GetUInt32ValueFromArray(Tokens const& data, uint16 index)
{
    if(index >= data.size())
        return 0;

    return (uint32)atoi(data[index].c_str());
}

float Player::GetFloatValueFromArray(Tokens const& data, uint16 index)
{
    float result;
    uint32 temp = Player::GetUInt32ValueFromArray(data,index);
    memcpy(&result, &temp, sizeof(result));

    return result;
}

uint32 Player::GetUInt32ValueFromDB(uint16 index, ObjectGuid guid)
{
    Tokens data;
    if(!LoadValuesArrayFromDB(data,guid))
        return 0;

    return GetUInt32ValueFromArray(data,index);
}

float Player::GetFloatValueFromDB(uint16 index, ObjectGuid guid)
{
    float result;
    uint32 temp = Player::GetUInt32ValueFromDB(index, guid);
    memcpy(&result, &temp, sizeof(result));

    return result;
}

bool Player::LoadFromDB( uint32 guid, SQLQueryHolder *holder )
{
    PreparedQueryResult result = holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_FROM);

    Object::_Create( guid, 0, HighGuid::Player );

    if(!result)
    {
        TC_LOG_ERROR("entities.player","ERROR: Player (GUID: %u) not found in table `characters`, can't load. ",guid);
        return false;
    }

    Field *fields = result->Fetch();

    uint32 dbAccountId = fields[LOAD_DATA_ACCOUNT].GetUInt32();

    // check if the character's account in the db and the logged in account match.
    // player should be able to load/delete character only with correct account!
    if( dbAccountId != GetSession()->GetAccountId() )
    {
        TC_LOG_ERROR("entities.player","ERROR: Player (GUID: %u) loading from wrong account (is: %u, should be: %u)",guid,GetSession()->GetAccountId(),dbAccountId);
        return false;
    }

    m_name = fields[LOAD_DATA_NAME].GetString();

    // check name limitations
    if(!ObjectMgr::CheckPlayerName(m_name) || (GetSession()->GetSecurity() == SEC_PLAYER && sObjectMgr->IsReservedName(m_name)))
    {
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid ='%u'", uint32(AT_LOGIN_RENAME),guid);
        return false;
    }

    // overwrite possible wrong/corrupted guid
    SetGuidValue(OBJECT_FIELD_GUID, ObjectGuid(HighGuid::Player, guid));

    // cleanup inventory related item value fields (its will be filled correctly in _LoadInventory)
    for(uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), 0 );
        SetVisibleItemSlot(slot,nullptr);

        delete m_items[slot];
        m_items[slot] = nullptr;
    }

    m_race = fields[LOAD_DATA_RACE].GetUInt8();
    m_class = fields[LOAD_DATA_CLASS].GetUInt8();
    m_gender = fields[LOAD_DATA_GENDER].GetUInt8();
    //Need to call it to initialize m_team (m_team can be calculated from m_race)
    //Other way is to saves m_team into characters table.
    SetFactionForRace(m_race);

    // Override some data fields
    SetUInt32Value(UNIT_FIELD_LEVEL, fields[LOAD_DATA_LEVEL].GetUInt8());
    SetUInt32Value(PLAYER_XP, fields[LOAD_DATA_XP].GetUInt32());
    SetUInt32Value(PLAYER_FIELD_COINAGE, fields[LOAD_DATA_MONEY].GetUInt32());
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE, m_race);
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, m_class);
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, m_gender);

    // check if race/class combination is valid
    PlayerInfo const* info = sObjectMgr->GetPlayerInfo(GetRace(), GetClass());
    if (!info)
    {
        TC_LOG_ERROR("entities.player", "Player::LoadFromDB: Player (%u) has wrong race/class (%u/%u), can't load.", guid, GetRace(), GetClass());
        return false;
    }

#ifndef LICH_KING
    SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_BUFF_LIMIT, UNIT_BYTE2_PLAYER_CONTROLLED_BUFF_LIMIT);
#endif
    //SetByteValue(PLAYER_BYTES_3, 0, m_gender);
    SetUInt32Value(PLAYER_BYTES, fields[LOAD_DATA_PLAYERBYTES].GetUInt32());   // PlayerBytes
    SetUInt32Value(PLAYER_BYTES_2, fields[LOAD_DATA_PLAYERBYTES2].GetUInt32()); // PlayerBytes2
    SetUInt32Value(PLAYER_FLAGS, fields[LOAD_DATA_PLAYERFLAGS].GetUInt32());   // PlayerFlags
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK);
    SetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_GENDER, fields[LOAD_DATA_GENDER].GetUInt8());
    SetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_INEBRIATION, fields[LOAD_DATA_DRUNK].GetUInt8());
    SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fields[LOAD_DATA_WATCHED_FACTION].GetUInt32());
    SetUInt32Value(PLAYER_AMMO_ID, fields[LOAD_DATA_AMMOID].GetUInt32());
    SetByteValue(PLAYER_FIELD_BYTES, 2, fields[LOAD_DATA_ACTIONBARS].GetUInt8());
    _LoadIntoDataField(fields[LOAD_DATA_EXPLOREDZONES].GetString(), PLAYER_EXPLORED_ZONES_1, 128);
    _LoadIntoDataField(fields[LOAD_DATA_KNOWNTITLES].GetString(), PLAYER_FIELD_KNOWN_TITLES, 2);

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_PLAYER_BOUNDING_RADIUS);
    SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f);
    //SetFloatValue(UNIT_FIELD_HOVERHEIGHT, 1.0f);

    // update money limits
    if(GetMoney() > MAX_MONEY_AMOUNT)
        SetMoney(MAX_MONEY_AMOUNT);

    // Override NativeDisplayId in case of race/faction change
    switch (m_gender) {
    case GENDER_FEMALE:
        SetDisplayId(info->displayId_f);
        SetNativeDisplayId(info->displayId_f);
        break;
    case GENDER_MALE:
        SetDisplayId(info->displayId_m);
        SetNativeDisplayId(info->displayId_m);
        break;
    default:
        TC_LOG_ERROR("entities.player","Invalid gender %u for player", m_gender);
        return false;
    }

    // load home bind and check in same time class/race pair, it used later for restore broken positions
    if (!_LoadHomeBind(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_HOME_BIND)))
    {
        return false;
    }

    InitPrimaryProffesions();                               // to max set before any spell loaded

    // init saved position, and fix it later if problematic
    uint32 transGUIDLow = fields[LOAD_DATA_TRANSGUID].GetUInt32();
    uint32 mapId = fields[LOAD_DATA_MAP].GetUInt32();
    uint32 instanceId = fields[LOAD_DATA_INSTANCE_ID].GetUInt32();

    MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
    Map* map = nullptr;

    if(sWorld->getConfig(CONFIG_ARENASERVER_ENABLED) && sWorld->getConfig(CONFIG_ARENASERVER_PLAYER_REPARTITION_THRESHOLD))
    {
        RelocateToArenaZone(ShouldGoToSecondaryArenaZone());
        map = GetMap();
    } else {
        Relocate(fields[LOAD_DATA_POSX].GetFloat(),fields[LOAD_DATA_POSY].GetFloat(),fields[LOAD_DATA_POSZ].GetFloat(),fields[LOAD_DATA_ORIENTATION].GetFloat());
        SetFallInformation(0, fields[LOAD_DATA_POSZ].GetFloat());
    }

    SetDifficulty(Difficulty(fields[LOAD_DATA_DUNGEON_DIFF].GetUInt8()));                  // may be changed in _LoadGroup


    // Experience Blocking
    m_isXpBlocked = fields[LOAD_DATA_XP_BLOCKED].GetUInt8();

    m_customXp = fields[LOAD_DATA_CUSTOM_XP].GetDouble();
    // Check value
    if (m_customXp < 1.0f || m_customXp > sWorld->GetRate(RATE_XP_KILL))
        m_customXp = 0;

    _LoadGroup(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_GROUP));

    _LoadArenaTeamInfo(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_ARENA_INFO));

    uint32 arena_currency = fields[LOAD_DATA_ARENAPOINTS].GetUInt32();
    if (arena_currency > sWorld->getConfig(CONFIG_MAX_ARENA_POINTS))
        arena_currency = sWorld->getConfig(CONFIG_MAX_ARENA_POINTS);

    SetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, arena_currency);

    // check arena teams integrity
    for(uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
    {
        uint32 arena_team_id = GetArenaTeamId(arena_slot);
        if(!arena_team_id)
            continue;

        if(ArenaTeam * at = sArenaTeamMgr->GetArenaTeamById(arena_team_id))
            if(at->HaveMember(GetGUID()))
                continue;

        // arena team not exist or not member, cleanup fields
        for(int j =0; j < 6; ++j)
            SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arena_slot * 6 + j, 0);
    }

    SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, fields[LOAD_DATA_TOTALHONORPOINTS].GetUInt32());
    SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, fields[LOAD_DATA_TODAYHONORPOINTS].GetUInt32());
    SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, fields[LOAD_DATA_YESTERDAYHONORPOINTS].GetUInt32());
    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, fields[LOAD_DATA_TOTALKILLS].GetUInt32());
    SetUInt16Value(PLAYER_FIELD_KILLS, 0, fields[LOAD_DATA_TODAYKILLS].GetUInt16());
    SetUInt16Value(PLAYER_FIELD_KILLS, 1, fields[LOAD_DATA_YESTERDAYKILLS].GetUInt16());

    _LoadBoundInstances(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_BOUND_INSTANCES));
    _LoadBGData(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_BG_DATA));

    GetSession()->SetPlayer(this);

#define RelocateToHomebind(){ mapId = m_homebindMapId; instanceId = 0; Relocate(m_homebindX, m_homebindY, m_homebindZ); }

    bool player_at_bg = false;

    if(!mapEntry || !IsPositionValid())
    {
        TC_LOG_ERROR("entities.player","Player (guidlow %d) have invalid coordinates (X: %f Y: %f Z: %f O: %f). Teleport to default race/class locations.",guid,GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());
        RelocateToHomebind();
        transGUIDLow = 0;
        m_movementInfo.transport.pos.Relocate(0.0,0.0,0.0,0.0f);
    }

    // Player was saved in Arena or Bg
    else if (mapEntry->IsBattlegroundOrArena())
    {
        Battleground* currentBg = nullptr;
        if (m_bgData.bgInstanceID)                                                //saved in Battleground
            currentBg = sBattlegroundMgr->GetBattleground(m_bgData.bgInstanceID, BATTLEGROUND_TYPE_NONE);

        player_at_bg = currentBg && currentBg->IsPlayerInBattleground(GetGUID());

        if (player_at_bg && currentBg->GetStatus() != STATUS_WAIT_LEAVE)
        {
            map = currentBg->GetBgMap();

            BattlegroundQueueTypeId bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(currentBg->GetTypeID(), currentBg->GetArenaType());
            AddBattlegroundQueueId(bgQueueTypeId);

            m_bgData.bgTypeID = currentBg->GetTypeID();

            //join player to battleground group
            currentBg->EventPlayerLoggedIn(this);

            SetInviteForBattlegroundQueueType(bgQueueTypeId, currentBg->GetInstanceID());
        }
        // Bg was not found - go to Entry Point
        else
        {
            // leave bg
            if (player_at_bg)
            {
                player_at_bg = false;
                currentBg->RemovePlayerAtLeave(GetGUID(), false, true);
            }

            // Do not look for instance if bg not found
            WorldLocation const& _loc = GetBattlegroundEntryPoint();
            mapId = _loc.GetMapId();
            instanceId = 0;

            // Db field type is type int16, so it can never be MAPID_INVALID
            //if (mapId == MAPID_INVALID) -- code kept for reference
            if (int16(mapId) == int16(-1)) // Battleground Entry Point not found (???)
            {
                TC_LOG_ERROR("entities.player", "Player::LoadFromDB: Player (%u) was in BG in database, but BG was not found and entry point was invalid! Teleport to default race/class locations.",
                    guid);
                RelocateToHomebind();
            }
            else
                Relocate(&_loc);

            // We are not in BG anymore
            m_bgData.bgInstanceID = 0;
        }
    }

    std::string taxi_nodes = fields[LOAD_DATA_TAXI_PATH].GetString();

    if (transGUIDLow)
    {
        ObjectGuid transGUID = ObjectGuid(HighGuid::Mo_Transport, 0, transGUIDLow);
        Transport* transport = nullptr;
        if (Transport* go = HashMapHolder<MotionTransport>::Find(transGUID))
            transport = go;

        /* fixme, loading for static transport is probably broken right now. THis needs to be moved later when map is loaded
        if (!transGO) // sunwell: if not MotionTransport, look for StaticTransport
        {
#ifdef LICH_KING
            transGUID = MAKE_NEW_GUID(transGUIDLow, 0, HighGuid::Transport);
#else
            transGUID = MAKE_NEW_GUID(transGUIDLow, 0, HighGuid::GameObject);
#endif
            transGO = GetMap()->GetGameObject(transGUID);
        }
        */
        if (transport)
            if (transport->IsInWorld() && transport->FindMap()) // sunwell: must be on map, for one world tick transport is not in map and has old GetMapId(), player would be added to old map and to the transport, multithreading crashfix
                m_transport = transport;

        if (m_transport)
        {
            float x = fields[LOAD_DATA_TRANSX].GetFloat(), y = fields[LOAD_DATA_TRANSY].GetFloat(), z = fields[LOAD_DATA_TRANSZ].GetFloat(), o = fields[LOAD_DATA_TRANSO].GetFloat();
            m_movementInfo.transport.guid = transGUID;
            m_movementInfo.transport.pos.Relocate(x, y, z, o);
            m_transport->CalculatePassengerPosition(x, y, z, &o);

            if (!Trinity::IsValidMapCoord(x, y, z, o) || std::fabs(m_movementInfo.transport.pos.GetPositionX()) > 75.0f || std::fabs(m_movementInfo.transport.pos.GetPositionY()) > 75.0f || std::fabs(m_movementInfo.transport.pos.GetPositionZ()) > 75.0f)
            {
                m_transport = nullptr;
                m_movementInfo.transport.Reset();
                m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
                RelocateToHomebind();
            }
            else
            {
                Relocate(x, y, z, o);
                mapId = m_transport->GetMapId();
                m_transport->AddPassenger(this);
                AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
            }
        }
        else
        {
            bool fixed = false;
            if (mapEntry->Instanceable())
                if (AreaTrigger const* at = sObjectMgr->GetMapEntranceTrigger(GetMapId()))
                {
                    fixed = true;
                    Relocate(at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
                }
            if (!fixed)
                RelocateToHomebind();
        }
    }

    // currently we do not support taxi in instance
    else if (!taxi_nodes.empty())
    {
        instanceId = 0;

        // Not finish taxi flight path
#ifdef LICH_KING
        if (m_bgData.HasTaxiPath())
        {
            for (int i = 0; i < 2; ++i)
                m_taxi.AddTaxiDestination(m_bgData.taxiPath[i]);
        }
        else
#endif
        if (!m_taxi.LoadTaxiDestinationsFromString(taxi_nodes, GetTeam()))
        {
            // problems with taxi path loading
            TaxiNodesEntry const* nodeEntry = nullptr;
            if (uint32 node_id = m_taxi.GetTaxiSource())
                nodeEntry = sTaxiNodesStore.LookupEntry(node_id);

            if (!nodeEntry)                                      // don't know taxi start node, teleport to homebind
            {
                TC_LOG_ERROR("entities.player", "Player::LoadFromDB: Player (%s) has wrong data in taxi destination list, teleport to homebind.", ObjectGuid(GetGUID()).ToString().c_str());
                RelocateToHomebind();
            }
            else                                                // has start node, teleport to it
            {
                TC_LOG_ERROR("entities.player", "Player::LoadFromDB: Player (%s) has too short taxi destination list, teleport to original node.", ObjectGuid(GetGUID()).ToString().c_str());
                mapId = nodeEntry->map_id;
                Relocate(nodeEntry->x, nodeEntry->y, nodeEntry->z, 0.0f);
            }
            m_taxi.ClearTaxiDestinations();
        }

        if (uint32 node_id = m_taxi.GetTaxiSource())
        {
            // save source node as recall coord to prevent recall and fall from sky
            TaxiNodesEntry const* nodeEntry = sTaxiNodesStore.LookupEntry(node_id);
            if (nodeEntry && nodeEntry->map_id == GetMapId())
            {
                ASSERT(nodeEntry);                                  // checked in m_taxi.LoadTaxiDestinationsFromString
                mapId = nodeEntry->map_id;
                Relocate(nodeEntry->x, nodeEntry->y, nodeEntry->z, 0.0f);
            }

            // flight will started later
        }
    }

    // In some old saves players' instance id are not correctly ordered
    // This fixes the crash. But it is not needed for a new db
    if (InstanceSave *pSave = GetInstanceSave(mapId))
        if (pSave->GetInstanceId() != GetInstanceId())
            instanceId = pSave->GetInstanceId();

    // Map could be changed before
    mapEntry = sMapStore.LookupEntry(mapId);

    // NOW player must have valid map
    // load the player's map here if it's not already loaded
    if (!map)
        map = sMapMgr->CreateMap(mapId, this, instanceId);
    AreaTrigger const* areaTrigger = nullptr;
    bool check = false;

    if (!map)
    {
        areaTrigger = sObjectMgr->GetGoBackTrigger(mapId);
        check = true;
    }
    else if (map->IsDungeon()) // if map is dungeon...
    {
        if (Map::EnterState denyReason = ((InstanceMap*)map)->CannotEnter(this)) // ... and can't enter map, then look for entry point.
        {
            switch (denyReason)
            {
            case Map::CANNOT_ENTER_DIFFICULTY_UNAVAILABLE:
#ifdef LICH_KING
                SendTransferAborted(map->GetId(), TRANSFER_ABORT_DIFFICULTY, map->GetDifficulty());
#else
                SendTransferAborted(map->GetId(), TRANSFER_ABORT_DIFFICULTY2); //on BC unavailable difficulty will always be heroic
#endif
                break;
            case Map::CANNOT_ENTER_INSTANCE_BIND_MISMATCH:
                //TC ChatHandler(GetSession()).PSendSysMessage(GetSession()->GetTrinityString(LANG_INSTANCE_BIND_MISMATCH), map->GetMapName());
                ChatHandler(GetSession()).PSendSysMessage("You are already locked to %s.", map->GetMapName());
                break;
            case Map::CANNOT_ENTER_TOO_MANY_INSTANCES:
                SendTransferAborted(map->GetId(), TRANSFER_ABORT_TOO_MANY_INSTANCES);
                break;
            case Map::CANNOT_ENTER_MAX_PLAYERS:
                SendTransferAborted(map->GetId(), TRANSFER_ABORT_MAX_PLAYERS);
                break;
            case Map::CANNOT_ENTER_ZONE_IN_COMBAT:
                SendTransferAborted(map->GetId(), TRANSFER_ABORT_ZONE_IN_COMBAT);
                break;
            default:
                break;
            }
            areaTrigger = sObjectMgr->GetGoBackTrigger(mapId);
            check = true;
        }
        else if (instanceId && !sInstanceSaveMgr->GetInstanceSave(instanceId)) // ... and instance is reseted then look for entrance.
        {
            areaTrigger = sObjectMgr->GetMapEntranceTrigger(mapId);
            check = true;
        }
    }

    if (check) // in case of special event when creating map...
    {
        if (areaTrigger) // ... if we have an areatrigger, then relocate to new map/coordinates.
        {
            Relocate(areaTrigger->target_X, areaTrigger->target_Y, areaTrigger->target_Z, GetOrientation());
            if (mapId != areaTrigger->target_mapId)
            {
                mapId = areaTrigger->target_mapId;
                map = sMapMgr->CreateMap(mapId, this);
            }
        }
        else
        {
            TC_LOG_ERROR("entities.player", "Player::LoadFromDB: Player '%s' (%s) Map: %u, X: %f, Y: %f, Z: %f, O: %f. Areatrigger not found.",
                m_name.c_str(), ObjectGuid(HighGuid::Player, guid).ToString().c_str(), mapId, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
            RelocateToHomebind();
            map = nullptr;
        }
    }

    if (!map)
    {
        mapId = info->mapId;
        Relocate(info->positionX, info->positionY, info->positionZ, 0.0f);
        map = sMapMgr->CreateMap(mapId, this);
        if (!map)
        {
            TC_LOG_ERROR("entities.player", "Player::LoadFromDB: Player '%s' (%s) Map: %u, X: %f, Y: %f, Z: %f, O: %f. Invalid default map coordinates or instance couldn't be created.",
                m_name.c_str(), ObjectGuid(HighGuid::Player, guid).ToString().c_str(), mapId, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
            return false;
        }
    }

    SetMap(map);
    UpdatePositionData();
#ifdef LICH_KING
    StoreRaidMapDifficulty();
#endif

    // now that map position is determined, check instance validity
    if (!CheckInstanceValidity(true) /*&& !IsInstanceLoginGameMasterException()*/) //sun, removed second condition, don't want gm treated differently from player when gm mode is off
        m_InstanceValid = false;

    // if the player is in an instance and it has been reset in the meantime teleport him to the entrance
    if (GetInstanceId() && !sInstanceSaveMgr->GetInstanceSave(GetInstanceId()))
    {
        AreaTrigger const* at = sObjectMgr->GetMapEntranceTrigger(GetMapId());
        if (at)
            Relocate(at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
        else if (!map->IsBattlegroundOrArena())
            TC_LOG_ERROR("entities.player","Player %s(GUID: %u) logged in to a reset instance (map: %u) and there is no area-trigger leading to this map. Thus he can't be ported back to the entrance. This _might_ be an exploit attempt.",
                    GetName().c_str(), GetGUID().GetCounter(), GetMapId());
    }

    SaveRecallPosition();


    // randomize first save time in range [CONFIG_INTERVAL_SAVE] around [CONFIG_INTERVAL_SAVE]
    // this must help in case next save after mass player load after server startup
    m_nextSave = urand(m_nextSave / 2, m_nextSave * 3 / 2);

    time_t now = map->GetGameTime();
    time_t logoutTime = time_t(fields[LOAD_DATA_LOGOUT_TIME].GetUInt32());

    // since last logout (in seconds)
    uint64 time_diff = uint64(now - logoutTime);

    // set value, including drunk invisibility detection
    // calculate sobering. after 15 minutes logged out, the player will be sober again
    uint8 newDrunkValue = 0;
    if (time_diff < uint32(GetDrunkValue()) * 9)
        newDrunkValue = GetDrunkValue() - time_diff / 9;

    SetDrunkValue(newDrunkValue);

    m_cinematic = fields[LOAD_DATA_CINEMATIC].GetUInt8();
    m_Played_time[0]= fields[LOAD_DATA_TOTALTIME].GetUInt32();
    m_Played_time[1]= fields[LOAD_DATA_LEVELTIME].GetUInt32();

    m_resetTalentsCost = fields[LOAD_DATA_RESETTALENTS_COST].GetUInt32();
    m_resetTalentsTime = time_t(fields[LOAD_DATA_RESETTALENTS_TIME].GetUInt32());

    // reserve some flags
    uint32 old_safe_flags = GetUInt32Value(PLAYER_FLAGS) & ( PLAYER_FLAGS_HIDE_CLOAK | PLAYER_FLAGS_HIDE_HELM );

    if( HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM) )
        SetUInt32Value(PLAYER_FLAGS, 0 | old_safe_flags);

    m_taxi.LoadTaxiMask(fields[LOAD_DATA_TAXIMASK].GetCString());          // must be before InitTaxiNodesForLevel

    uint32 extraflags = fields[LOAD_DATA_EXTRA_FLAGS].GetUInt16();

    m_stableSlots = fields[LOAD_DATA_STABLE_SLOTS].GetUInt8();
    if(m_stableSlots > 2)
    {
        TC_LOG_ERROR("entities.player","Player can have not more 2 stable slots, but have in DB %u",uint32(m_stableSlots));
        m_stableSlots = 2;
    }

    m_atLoginFlags = fields[LOAD_DATA_AT_LOGIN].GetUInt32();

    // Honor system
    // Update Honor kills data
    m_lastHonorUpdateTime = logoutTime;
    UpdateHonorFields();

    m_deathExpireTime = (time_t)fields[LOAD_DATA_DEATH_EXPIRE_TIME].GetUInt64();
    if(m_deathExpireTime > now+MAX_DEATH_COUNT*DEATH_EXPIRE_STEP)
        m_deathExpireTime = now+MAX_DEATH_COUNT*DEATH_EXPIRE_STEP-1;

    // clear channel spell data (if saved at channel spell casting)
    SetChannelObjectGuid(ObjectGuid::Empty);
    SetUInt32Value(UNIT_CHANNEL_SPELL, 0);

    // clear charm/summon related fields
    SetOwnerGUID(ObjectGuid::Empty);
    SetCreatorGUID(ObjectGuid::Empty);
    SetGuidValue(PLAYER_FARSIGHT, ObjectGuid::Empty);
    SetGuidValue(UNIT_FIELD_CHARMEDBY, ObjectGuid::Empty);
    SetGuidValue(UNIT_FIELD_CHARM, ObjectGuid::Empty);
    SetGuidValue(UNIT_FIELD_SUMMON, ObjectGuid::Empty);

    // reset some aura modifiers before aura apply
    SetUInt32Value(PLAYER_TRACK_CREATURES, 0 );
    SetUInt32Value(PLAYER_TRACK_RESOURCES, 0 );

    _LoadSkills(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_SKILLS));
    UpdateSkillsForLevel(); //update skills after load, to make sure they are correctly update at player load

    // make sure the unit is considered out of combat for proper loading
    ClearInCombat();

    // make sure the unit is considered not in duel for proper loading
    SetGuidValue(PLAYER_DUEL_ARBITER, ObjectGuid::Empty);
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    // reset stats before loading any modifiers
    InitStatsForLevel();
    InitTaxiNodesForLevel();

    // After InitStatsForLevel(), or PLAYER_NEXT_LEVEL_XP is 0 and rest bonus too
    m_rest_bonus = fields[LOAD_DATA_REST_BONUS].GetFloat();
    //speed collect rest bonus in offline, in logout, far from tavern, city (section/in hour)
    float bubble0 = 0.031;
    //speed collect rest bonus in offline, in logout, in tavern, city (section/in hour)
    float bubble1 = 0.125;

    if((int32)fields[LOAD_DATA_LOGOUT_TIME].GetUInt32() > 0)
    {
        float bubble = fields[LOAD_DATA_IS_LOGOUT_RESTING].GetUInt8() > 0
            ? bubble1*sWorld->GetRate(RATE_REST_OFFLINE_IN_TAVERN_OR_CITY)
            : bubble0*sWorld->GetRate(RATE_REST_OFFLINE_IN_WILDERNESS);

        SetRestBonus(GetRestBonus()+ time_diff*((float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP)/72000)*bubble);
    }
    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()

    //mails are loaded only when needed ;-) - when player in game click on mailbox.
    //_LoadMail();

    _LoadAuras(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_AURAS), time_diff);

    // add ghost flag (must be after aura load: PLAYER_FLAGS_GHOST set in aura)
    if( HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST) )
        m_deathState = DEAD;

    UpdateDisplayPower();
    _LoadSpells(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_SPELLS));

    // after spell load
    InitTalentForLevel();
    LearnDefaultSkills();
    LearnDefaultSpells();

    // after spell load, learn rewarded spell if need also
    _LoadQuestStatus(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_QUEST_STATUS));
    _LoadDailyQuestStatus(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_DAILY_QUEST_STATUS));

    // must be before inventory (some items required reputation check)
    m_reputationMgr->LoadFromDB(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_REPUTATION));

    _LoadInventory(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_INVENTORY), time_diff);

    // update items with duration and realtime
    UpdateItemDuration(time_diff, true);

    _LoadActions(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_ACTIONS));

    // unread mails and next delivery time, actual mails not loaded
    _LoadMailInit(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_MAIL_COUNT), holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_MAIL_DATE));

    m_social = sSocialMgr->LoadFromDB(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_SOCIAL_LIST), GetGUID().GetCounter());

    // check PLAYER_CHOSEN_TITLE compatibility with PLAYER_FIELD_KNOWN_TITLES
    // note: PLAYER_FIELD_KNOWN_TITLES updated at quest status loaded
    uint32 curTitle = fields[LOAD_DATA_CHOSEN_TITLE].GetUInt32();

    if (curTitle && !HasTitle(curTitle))
        curTitle = 0;

    SetUInt32Value(PLAYER_CHOSEN_TITLE, curTitle);

    // has to be called after last Relocate() in Player::LoadFromDB
    SetFallInformation(0, GetPositionZ());

    GetSpellHistory()->LoadFromDB<Player>(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_SPELL_COOLDOWNS));

    uint32 savedHealth = fields[LOAD_DATA_HEALTH].GetUInt32();
    if (!savedHealth)
        m_deathState = CORPSE;

    // Spell code allow apply any auras to dead character in load time in aura/spell/item loading
    // Do now before stats re-calculation cleanup for ghost state unexpected auras
    if(!IsAlive())
        RemoveAllAurasOnDeath();

    //apply all stat bonuses from items and auras
    SetCanModifyStats(true);
    UpdateAllStats();

    // restore remembered power/health values (but not more max values)
    SetHealth(savedHealth);
    for(uint32 i = 0; i < MAX_POWERS; ++i) 
    {
        uint32 savedPower = fields[LOAD_DATA_POWER1+i].GetUInt32();
        SetPower(static_cast<Powers>(i), savedPower);
    }

    // GM state
    if(GetSession()->GetSecurity() > SEC_PLAYER /*|| GetSession()->GetGroupId()*/) // gmlevel > 0 or is in a gm group
    {
        switch(sWorld->getConfig(CONFIG_GM_LOGIN_STATE))
        {
            default:
            case 0:                      break;             // disable
            case 1: SetGameMaster(true); break;             // enable
            case 2:                                         // save state
                if(extraflags & PLAYER_EXTRA_GM_ON)
                    SetGameMaster(true);
                break;
        }

        switch(sWorld->getConfig(CONFIG_GM_VISIBLE_STATE))
        {
            default:
            case 0: SetGMVisible(false); break;             // invisible
            case 1:                      break;             // visible
            case 2:                                         // save state
                if(extraflags & PLAYER_EXTRA_GM_INVISIBLE)
                    SetGMVisible(false);
                break;
        }

        switch(sWorld->getConfig(CONFIG_GM_CHAT))
        {
            default:
            case 0:                  break;                 // disable
            case 1: SetGMChat(true); break;                 // enable
            case 2:                                         // save state
                if(extraflags & PLAYER_EXTRA_GM_CHAT)
                    SetGMChat(true);
                break;
        }

        switch(sWorld->getConfig(CONFIG_GM_WISPERING_TO))
        {
            default:
            case 0:                          break;         // disable
            case 1: SetAcceptWhispers(true); break;         // enable
            case 2:                                         // save state
                if(extraflags & PLAYER_EXTRA_ACCEPT_WHISPERS)
                    SetAcceptWhispers(true);
                break;
        }
    }

    _LoadDeclinedNames(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_DECLINED_NAMES));

    return true;
}

uint32 Player::GetMoney() const
{
    return GetUInt32Value (PLAYER_FIELD_COINAGE);
}

bool Player::HasEnoughMoney(int32 amount) const
{
    if (amount > 0)
        return (GetMoney() >= (uint32) amount);
    return true;
}

void Player::SetMoney( uint32 value )
{
    SetUInt32Value (PLAYER_FIELD_COINAGE, value);
    MoneyChanged( value );
}

bool Player::ModifyMoney(int32 amount, bool sendError /*= true*/)
{
    if (!amount)
        return true;

    sScriptMgr->OnPlayerMoneyChanged(this, amount);

    if (amount < 0)
        SetMoney (GetMoney() > uint32(-amount) ? GetMoney() + amount : 0);
    else
    {
        if (GetMoney() < MAX_MONEY_AMOUNT - static_cast<uint32>(amount))
            SetMoney(GetMoney() + amount);
        else
        {
           // sScriptMgr->OnPlayerMoneyLimit(this, amount);

            if (sendError)
                SendEquipError(EQUIP_ERR_TOO_MUCH_GOLD, nullptr, nullptr);
            return false;
        }
    }

    return true;
}

bool Player::IsAllowedToLoot(Creature const* creature) const
{
    if(creature->IsDead() && !creature->IsDamageEnoughForLootingAndReward())
        return false;

    const Loot* loot = &creature->loot;
    if (loot->isLooted()) // nothing to loot or everything looted.
        return false;

    /* TC, not tested
    if (!loot->hasItemForAll() && !loot->hasItemFor(this)) // no loot in creature for this player
        return false;
        */

    /*if (loot->loot_type == LOOT_SKINNING)
        return creature->GetLootRecipientGUID() == GetGUID(); */

    Group const* thisGroup = GetGroup();
    if (!thisGroup)
        return this == creature->GetLootRecipient();
    else if (thisGroup != creature->GetLootRecipientGroup())
        return false;
    else //same group
    {
        //check if player was present at creature death if worldboss
        if(creature->IsWorldBoss())
            if(!creature->HadPlayerInThreatListAtDeath(this->GetGUID()))
            {
                TC_LOG_DEBUG("misc", "Player %u tried to loot creature %u, was in the right group but wasn't present in creature threat list.", this->GetGUID().GetCounter(), creature->GetGUID().GetCounter());
                return false;
            }
    }

    switch (thisGroup->GetLootMethod())
    {
        case MASTER_LOOT:
        case FREE_FOR_ALL:
            return true;
        case ROUND_ROBIN:
            // may only loot if the player is the loot roundrobin player
            // or if there are free/quest/conditional item for the player
            if (loot->roundRobinPlayer == 0 || loot->roundRobinPlayer == GetGUID())
                return true;

            return loot->hasItemFor(this);
        case GROUP_LOOT:
        case NEED_BEFORE_GREED:
            // may only loot if the player is the loot roundrobin player
            // or item over threshold (so roll(s) can be launched)
            // or if there are free/quest/conditional item for the player
            if (loot->roundRobinPlayer == 0 || loot->roundRobinPlayer == GetGUID())
                return true;

            if (loot->hasOverThresholdItem())
                return true;

            return loot->hasItemFor(this);
    }

    return false;
}

void Player::_LoadActions(PreparedQueryResult result)
{
    m_actionButtons.clear();

    //QueryResult result = CharacterDatabase.PQuery("SELECT button,action,type,misc FROM character_action WHERE guid = '%u' ORDER BY button",GetGUID().GetCounter());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint8 button = fields[0].GetUInt8();

            addActionButton(button, fields[1].GetUInt16(), fields[2].GetUInt8(), fields[3].GetUInt8());

            m_actionButtons[button].uState = ACTIONBUTTON_UNCHANGED;
        }
        while( result->NextRow() );
    }
}

void Player::_LoadAuras(PreparedQueryResult result, uint32 timediff)
{
    // all aura related fields
    for(int i = UNIT_FIELD_AURA; i <= UNIT_FIELD_AURASTATE; ++i)
        SetUInt32Value(i, 0);

    if(result)
    {
        do
        {
            int32 damage[3];
            int32 baseDamage[3];
            Field *fields = result->Fetch();
            ObjectGuid caster_guid = ObjectGuid(fields[0].GetUInt64());
            ObjectGuid itemGuid(fields[1].GetUInt64());
            uint32 spellid = fields[2].GetUInt32();
            uint8 effmask = fields[3].GetUInt8();
            uint8 recalculatemask = fields[4].GetUInt8();
            uint8 stackcount = fields[5].GetUInt8();
            damage[0] = fields[6].GetInt32();
            damage[1] = fields[7].GetInt32();
            damage[2] = fields[8].GetInt32();
            baseDamage[0] = fields[9].GetInt32();
            baseDamage[1] = fields[10].GetInt32();
            baseDamage[2] = fields[11].GetInt32();
            int32 maxduration = fields[12].GetInt32();
            int32 remaintime = fields[13].GetInt32();
            uint8 remaincharges = fields[14].GetUInt8();
            float critChance = fields[15].GetFloat();
            bool applyResilience = fields[16].GetBool();

            if(spellid == SPELL_ARENA_PREPARATION || spellid == SPELL_PREPARATION)
            {
               if(Battleground const *bg = GetBattleground())
                   if(bg->GetStatus() == STATUS_IN_PROGRESS)
                       continue;
            }

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if(!spellInfo)
            {
                TC_LOG_ERROR("entities.player","Unknown aura (spellid %u), ignore.",spellid);
                continue;
            }

            if(effmask > MAX_EFFECT_MASK)
            {
                TC_LOG_ERROR("entities.player","Invalid effect mask (spellid %u, effmask %u), ignore.", spellid, effmask);
                continue;
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && ((!spellInfo->IsPositive() && spellInfo->Id != 15007) || spellInfo->HasAttribute(SPELL_ATTR4_EXPIRE_OFFLINE))) // Resurrection sickness should not fade while logged out
            {
                if(remaintime  <= int32(timediff))
                    continue;

                remaintime -= timediff;
            }

            // prevent wrong values of remaincharges
            if (spellInfo->ProcCharges)
            {
                // we have no control over the order of applying auras and modifiers allow auras
                // to have more charges than value in SpellInfo
                if (remaincharges <= 0/* || remaincharges > spellproto->procCharges*/)
                    remaincharges = spellInfo->ProcCharges;
            }
            else
                remaincharges = 0;

            //do not load single target auras (unless they were cast by the player)
            if (caster_guid != GetGUID() && spellInfo->IsSingleTarget())
                continue;

            // Do not load SPELL_AURA_MOD_DETAUNT auras
            if (spellInfo->HasAura(SPELL_AURA_MOD_DETAUNT))
                continue;

            AuraCreateInfo createInfo(spellInfo, effmask, this);
            createInfo
                .SetCasterGUID(caster_guid)
                .SetCastItemGUID(itemGuid)
                .SetBaseAmount(baseDamage);

            if (Aura* aura = Aura::TryCreate(createInfo))
            {
                if (!aura->CanBeSaved())
                {
                    aura->Remove();
                    continue;
                }

                aura->SetLoadedState(maxduration, remaintime, remaincharges, stackcount, recalculatemask, critChance, applyResilience, &damage[0]);
                aura->ApplyForTargets();
                TC_LOG_DEBUG("entities.player", "Player::_LoadAuras: Added aura (SpellID: %u, EffectMask: %u) to player '%s (%s)",
                    spellInfo->Id, effmask, GetName().c_str(), GetGUID().ToString().c_str());
            }
        }
        while( result->NextRow() );
    }

    if(m_class == CLASS_WARRIOR)
        CastSpell(this, SPELL_ID_PASSIVE_BATTLE_STANCE, true);
}

void Player::LoadCorpse(PreparedQueryResult result)
{
    if( IsAlive() || HasAtLoginFlag(AT_LOGIN_RESURRECT))
        SpawnCorpseBones(false);
    else
    {
        if (!HasAtLoginFlag(AT_LOGIN_RESURRECT))
            ResurrectPlayer(0.5f);
        else if (result)
        {
            Field* fields = result->Fetch();
            _corpseLocation.WorldRelocate(fields[0].GetUInt16(), fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            ApplyModFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_RELEASE_TIMER, !sMapStore.LookupEntry(_corpseLocation.GetMapId())->Instanceable() );
        }
    }

    RemoveAtLoginFlag(AT_LOGIN_RESURRECT);
}

Item* Player::_LoadItem(SQLTransaction& trans, uint32 zoneId, uint32 timeDiff, Field* fields)
{
    Item* item = nullptr;
    uint32 startIndex = CHAR_SEL_ITEM_INSTANCE_FIELDS_COUNT + 2;
    ObjectGuid::LowType itemGuid = fields[startIndex++].GetUInt32();
    uint32 itemEntry = fields[startIndex++].GetUInt32();
    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry))
    {
        bool remove = false;
        item = NewItemOrBag(proto);
        if (item->LoadFromDB(itemGuid, GetGUID(), fields, itemEntry))
        {
            //PreparedStatement* stmt;

            // Do not allow to have item limited to another map/zone in alive state
            if (IsAlive() && item->IsLimitedToAnotherMapOrZone(GetMapId(), zoneId))
            {
                TC_LOG_DEBUG("entities.player.loading", "Player::_LoadInventory: player (GUID: %u, name: '%s', map: %u) has item (GUID: %u, entry: %u) limited to another map (%u). Deleting item.",
                    GetGUID().GetCounter(), GetName().c_str(), GetMapId(), item->GetGUID().GetCounter(), item->GetEntry(), zoneId);
                remove = true;
            }
            // "Conjured items disappear if you are logged out for more than 15 minutes"
            else if (timeDiff > 15 * MINUTE && proto->Flags & ITEM_FLAG_CONJURED)
            {
                TC_LOG_DEBUG("entities.player.loading", "Player::_LoadInventory: player (GUID: %u, name: '%s', diff: %u) has conjured item (GUID: %u, entry: %u) with expired lifetime (15 minutes). Deleting item.",
                    GetGUID().GetCounter(), GetName().c_str(), timeDiff, item->GetGUID().GetCounter(), item->GetEntry());
                remove = true;
            }
#ifdef LICH_KING
            else if (item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_REFUNDABLE))
            {
                if (item->GetPlayedTime() > (2 * HOUR))
                {
                    TC_LOG_DEBUG("entities.player.loading", "Player::_LoadInventory: player (GUID: %u, name: '%s') has item (GUID: %u, entry: %u) with expired refund time (%u). Deleting refund data and removing refundable flag.",
                        GetGUID().GetCounter(), GetName().c_str(), item->GetGUID().GetCounter(), item->GetEntry(), item->GetPlayedTime());

                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_REFUND_INSTANCE);
                    stmt->setUInt32(0, item->GetGUID().GetCounter());
                    trans->Append(stmt);

                    item->RemoveFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_REFUNDABLE);
                }
                else
                {
                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ITEM_REFUNDS);
                    stmt->setUInt32(0, item->GetGUID().GetCounter());
                    stmt->setUInt32(1, GetGUID().GetCounter());
                    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
                    {
                        item->SetRefundRecipient((*result)[0].GetUInt32());
                        item->SetPaidMoney((*result)[1].GetUInt32());
                        item->SetPaidExtendedCost((*result)[2].GetUInt16());
                        AddRefundReference(item->GetGUID());
                    }
                    else
                    {
                        TC_LOG_DEBUG("entities.player.loading", "Player::_LoadInventory: player (GUID: %u, name: '%s') has item (GUID: %u, entry: %u) with refundable flags, but without data in item_refund_instance. Removing flag.",
                            GetGUID().GetCounter(), GetName().c_str(), item->GetGUID().GetCounter(), item->GetEntry());
                        item->RemoveFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_REFUNDABLE);
                    }
                }
            }
            else if (item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_BOP_TRADEABLE))
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ITEM_BOP_TRADE);
                stmt->setUInt32(0, item->GetGUID().GetCounter());
                if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
                {
                    std::string strGUID = (*result)[0].GetString();
                    Tokenizer GUIDlist(strGUID, ' ');
                    GuidSet looters;
                    for (Tokenizer::const_iterator itr = GUIDlist.begin(); itr != GUIDlist.end(); ++itr)
                        looters.insert(ObjectGuid::Create<HighGuid::Player>(uint32(strtoul(*itr, nullptr, 10))));

                    if (looters.size() > 1 && item->GetTemplate()->GetMaxStackSize() == 1 && item->IsSoulBound())
                    {
                        item->SetSoulboundTradeable(looters);
                        AddTradeableItem(item);
                    }
                    else
                        item->ClearSoulboundTradeable(this);
                }
                else
                {
                    TC_LOG_DEBUG("entities.player.loading", "Player::_LoadInventory: player (GUID: %u, name: '%s') has item (GUID: %u, entry: %u) with ITEM_FLAG_BOP_TRADEABLE flag, but without data in item_soulbound_trade_data. Removing flag.",
                        GetGUID().GetCounter(), GetName().c_str(), item->GetGUID().GetCounter(), item->GetEntry());
                    item->RemoveFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_BOP_TRADEABLE);
                }
            }
            else if (proto->HolidayId)
            {
                remove = true;
                GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();
                GameEventMgr::ActiveEvents const& activeEventsList = sGameEventMgr->GetActiveEventList();
                for (GameEventMgr::ActiveEvents::const_iterator itr = activeEventsList.begin(); itr != activeEventsList.end(); ++itr)
                {
                    if (uint32(events[*itr].holiday_id) == proto->HolidayId)
                    {
                        remove = false;
                        break;
                    }
                }
            }
#endif
        }
        else
        {
            TC_LOG_ERROR("entities.player", "Player::_LoadInventory: player (GUID: %u, name: '%s') has a broken item (GUID: %u, entry: %u) in inventory. Deleting item.",
                GetGUID().GetCounter(), GetName().c_str(), itemGuid, itemEntry);
            remove = true;
        }
        // Remove item from inventory if necessary
        if (remove)
        {
            Item::DeleteFromInventoryDB(trans, itemGuid);
            item->FSetState(ITEM_REMOVED);
            item->SaveToDB(trans);                           // it also deletes item object!
            item = nullptr;
        }
    }
    else
    {
        TC_LOG_ERROR("entities.player", "Player::_LoadInventory: player (GUID: %u, name: '%s') has an unknown item (entry: %u) in inventory. Deleting item.",
            GetGUID().GetCounter(), GetName().c_str(), itemEntry);
        Item::DeleteFromInventoryDB(trans, itemGuid);
        Item::DeleteFromDB(trans, itemGuid);
    }
    return item;
}

void Player::_LoadInventory(PreparedQueryResult result, uint32 timeDiff)
{
    //NOTE: the "order by `bag`" is important because it makes sure
    //the bagMap is filled before items in the bags are loaded
    //NOTE2: the "order by `slot`" is needed because mainhand weapons are (wrongly?)
    //expected to be equipped before offhand items (TODO: fixme)

    if (result)
    {
        uint32 zoneId = GetZoneId();

        std::map<ObjectGuid::LowType, Bag*> bagMap;             // fast guid lookup for bags
        std::map<ObjectGuid::LowType, Item*> invalidBagMap;     // fast guid lookup for bags
        std::list<Item*> problematicItems;

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        // prevent items from being added to the queue when stored
        m_itemUpdateQueueBlocked = true;
        do
        {
            Field *fields = result->Fetch();
            if (Item* item = _LoadItem(trans, zoneId, timeDiff, fields))
            {
                uint32 startIndex = CHAR_SEL_ITEM_INSTANCE_FIELDS_COUNT;
                ObjectGuid::LowType bagGuid = fields[startIndex++].GetUInt32();
                uint8 slot = fields[startIndex++].GetUInt8();

                InventoryResult err = EQUIP_ERR_OK;
                if (!bagGuid)
                {
                    // the item is not in a bag
                    item->SetContainer(nullptr);
                    item->SetSlot(slot);

                    if (IsInventoryPos(INVENTORY_SLOT_BAG_0, slot))
                    {
                        ItemPosCountVec dest;
                        err = CanStoreItem(INVENTORY_SLOT_BAG_0, slot, dest, item, false);
                        if (err == EQUIP_ERR_OK)
                            item = StoreItem(dest, item, true);
                    }
                    else if (IsEquipmentPos(INVENTORY_SLOT_BAG_0, slot))
                    {
                        uint16 dest;
                        err = CanEquipItem(slot, dest, item, false, false);
                        if (err == EQUIP_ERR_OK)
                            QuickEquipItem(dest, item);
                    }
                    else if (IsBankPos(INVENTORY_SLOT_BAG_0, slot))
                    {
                        ItemPosCountVec dest;
                        err = CanBankItem(INVENTORY_SLOT_BAG_0, slot, dest, item, false, false);
                        if (err == EQUIP_ERR_OK)
                            item = BankItem(dest, item, true);
                    }

                    // Remember bags that may contain items in them
                    if (err == EQUIP_ERR_OK)
                    {
                        if (IsBagPos(item->GetPos()))
                            if (Bag* pBag = item->ToBag())
                                bagMap[item->GetGUID().GetCounter()] = pBag;
                    }
                    else
                        if (IsBagPos(item->GetPos()))
                            if (item->IsBag())
                                invalidBagMap[item->GetGUID().GetCounter()] = item;
                }
                else
                {
                    item->SetSlot(NULL_SLOT);
                    // Item is in the bag, find the bag
                    std::map<ObjectGuid::LowType, Bag*>::iterator itr = bagMap.find(bagGuid);
                    if (itr != bagMap.end())
                    {
                        ItemPosCountVec dest;
                        err = CanStoreItem(itr->second->GetSlot(), slot, dest, item);
                        if (err == EQUIP_ERR_OK)
                            item = StoreItem(dest, item, true);
                    }
                    else if (invalidBagMap.find(bagGuid) != invalidBagMap.end())
                    {
                        std::map<ObjectGuid::LowType, Item*>::iterator invalidBagItr = invalidBagMap.find(bagGuid);
                        if (std::find(problematicItems.begin(), problematicItems.end(), invalidBagItr->second) != problematicItems.end())
                            err = EQUIP_ERR_INT_BAG_ERROR;
                    }
                    else
                    {
                        TC_LOG_ERROR("entities.player", "Player::_LoadInventory: Player '%s' (%s) has item (%s, entry: %u) which doesnt have a valid bag (Bag %u, slot: %u). Possible cheat?",
                            GetName().c_str(), GetGUID().ToString().c_str(), item->GetGUID().ToString().c_str(), item->GetEntry(), bagGuid, slot);
                        item->DeleteFromInventoryDB(trans);
                        delete item;
                        continue;
                    }
                }

                // item's state may have changed after stored
                if (err == EQUIP_ERR_OK)
                    item->SetState(ITEM_UNCHANGED, this);
                else
                {
                    TC_LOG_ERROR("entities.player", "Player::_LoadInventory: Player '%s' (%s) has item (%s, entry: %u) which can't be loaded into inventory (Bag %u, slot: %u) by reason %u. Item will be sent by mail.",
                        GetName().c_str(), GetGUID().ToString().c_str(), item->GetGUID().ToString().c_str(), item->GetEntry(), bagGuid, slot, uint32(err));
                    item->DeleteFromInventoryDB(trans);
                    problematicItems.push_back(item);
                }
            } 
        } while (result->NextRow());

        m_itemUpdateQueueBlocked = false;

        // send by mail problematic items
        while(!problematicItems.empty())
        {
            std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);

            MailDraft draft(subject, "There were problems with equipping item(s).");
            for (uint8 i = 0; !problematicItems.empty() && i < MAX_MAIL_ITEMS; ++i)
            {
                draft.AddItem(problematicItems.front());
                problematicItems.pop_front();
            }
            draft.SendMailTo(trans, this, MailSender(this, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_COPIED);
        }
        if(trans->GetSize())
            CharacterDatabase.CommitTransaction(trans);
    }
    //if(IsAlive())
    _ApplyAllItemMods();
}

// load mailed item which should receive current player
void Player::_LoadMailedItems(Mail *mail)
{
    // data needs to be at first place for Item::LoadFromDB
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_MAILITEMS);
    stmt->setUInt32(0, mail->messageID);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        uint32 startIndex = CHAR_SEL_ITEM_INSTANCE_FIELDS_COUNT;
        ObjectGuid::LowType itemGuid = fields[startIndex++].GetUInt32();
        uint32 itemTemplate = fields[startIndex++].GetUInt32();

        mail->AddItem(itemGuid, itemTemplate);

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemTemplate);

        if (!proto)
        {
            TC_LOG_ERROR("entities.player", "Player '%s' (%s) has unknown item_template in mailed items (GUID: %u, Entry: %u) in mail (%u), deleted.",
                GetName().c_str(), GetGUID().ToString().c_str(), itemGuid, itemTemplate, mail->messageID);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_MAIL_ITEM);
            stmt->setUInt32(0, itemGuid);
            CharacterDatabase.Execute(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_INSTANCE);
            stmt->setUInt32(0, itemGuid);
            CharacterDatabase.Execute(stmt);
            continue;
        }

        Item* item = NewItemOrBag(proto);

        if (!item->LoadFromDB(itemGuid, ObjectGuid(HighGuid::Player, fields[startIndex++].GetUInt32()), fields, itemTemplate))
        {
            TC_LOG_ERROR("entities.player", "Player::_LoadMailedItems: Item (GUID: %u) in mail (%u) doesn't exist, deleted from mail.", itemGuid, mail->messageID);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM);
            stmt->setUInt32(0, itemGuid);
            CharacterDatabase.Execute(stmt);

            item->FSetState(ITEM_REMOVED);

            SQLTransaction temp = SQLTransaction(nullptr);
            item->SaveToDB(temp);                               // it also deletes item object !
            continue;
        }

        AddMItem(item);
    } while (result->NextRow());
}

void Player::_LoadMailInit(PreparedQueryResult resultUnread, PreparedQueryResult resultDelivery)
{
    //set a count of unread mails
    //QueryResult resultMails = CharacterDatabase.PQuery("SELECT COUNT(id) FROM mail WHERE receiver = '%u' AND (checked & 1)=0 AND deliver_time <= '" UI64FMTD "'", playerGuid.GetCounter(),(uint64)cTime);
    if (resultUnread)
    {
        Field *fieldMail = resultUnread->Fetch();
        unReadMails = fieldMail[0].GetUInt64();
    }

    // store nearest delivery time (it > 0 and if it < current then at next player update SendNewMaill will be called)
    //resultMails = CharacterDatabase.PQuery("SELECT MIN(deliver_time) FROM mail WHERE receiver = '%u' AND (checked & 1)=0", playerGuid.GetCounter());
    if (resultDelivery)
    {
        Field *fieldMail = resultDelivery->Fetch();
        m_nextMailDelivereTime = (time_t)fieldMail[0].GetUInt64();
    }
}

void Player::_LoadMail()
{
    m_mail.clear();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_MAIL);
    stmt->setUInt32(0, GetGUID().GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            Mail* m = new Mail;

            m->messageID = fields[0].GetUInt32();
            m->messageType = fields[1].GetUInt8();
            m->sender = fields[2].GetUInt32();
            m->receiver = fields[3].GetUInt32();
            m->subject = fields[4].GetString();
            //m->body = fields[5].GetString();
            m->itemTextId = fields[5].GetUInt32();
            bool has_items = fields[6].GetBool();
            m->expire_time = time_t(fields[7].GetUInt32());
            m->deliver_time = time_t(fields[8].GetUInt32());
            m->money = fields[9].GetUInt32();
            m->COD = fields[10].GetUInt32();
            m->checked = fields[11].GetUInt8();
            m->stationery = fields[12].GetUInt8();
            m->mailTemplateId = fields[13].GetInt16();

            if (m->mailTemplateId && !sMailTemplateStore.LookupEntry(m->mailTemplateId))
            {
                TC_LOG_ERROR("entities.player", "Player::_LoadMail: Mail (%u) has nonexistent MailTemplateId (%u), remove at load", m->messageID, m->mailTemplateId);
                m->mailTemplateId = 0;
            }

            m->state = MAIL_STATE_UNCHANGED;

            if (has_items)
                _LoadMailedItems(m);

            m_mail.push_back(m);
        } while (result->NextRow());
    }
    m_mailsLoaded = true;
}

void Player::LoadPet()
{
    //fixme: the pet should still be loaded if the player is not in world
    // just not added to the map
    if(IsInWorld())
    {
        Pet* pet = new Pet(this);
        if(!pet->LoadPetFromDB(this,0,0,true))
            delete pet;
    }
}

void Player::_LoadQuestStatus(PreparedQueryResult result)
{
    m_QuestStatus.clear();

    uint32 slot = 0;

    ////                                                     0      1       2         3         4      5          6          7          8          9           10          11          12
    //QueryResult result = CharacterDatabase.PQuery("SELECT quest, status, rewarded, explored, timer, mobcount1, mobcount2, mobcount3, mobcount4, itemcount1, itemcount2, itemcount3, itemcount4 FROM character_queststatus WHERE guid = '%u'", GetGUID().GetCounter());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 quest_id = fields[0].GetUInt32();
                                                            // used to be new, no delete?
            Quest const* pQuest = sObjectMgr->GetQuestTemplate(quest_id);
            if( pQuest )
            {
                // find or create
                QuestStatusData& questStatusData = m_QuestStatus[quest_id];

                uint32 qstatus = fields[1].GetUInt32();
                if(qstatus < MAX_QUEST_STATUS)
                    questStatusData.Status = QuestStatus(qstatus);
                else
                {
                    questStatusData.Status = QUEST_STATUS_NONE;
                    TC_LOG_ERROR("entities.player","Player %s have invalid quest %d status (%d), replaced by QUEST_STATUS_NONE(0).",GetName().c_str(),quest_id,qstatus);
                }

                questStatusData.Rewarded = ( fields[2].GetUInt8() > 0 );
                questStatusData.Explored = ( fields[3].GetUInt8() > 0 );

                time_t quest_time = time_t(fields[4].GetUInt64());

                if( pQuest->HasFlag( QUEST_TRINITY_FLAGS_TIMED ) && !GetQuestRewardStatus(quest_id) &&  questStatusData.Status != QUEST_STATUS_NONE )
                {
                    AddTimedQuest( quest_id );

                    if (quest_time <= WorldGameTime::GetGameTime())
                        questStatusData.m_timer = 1;
                    else
                        questStatusData.m_timer = (quest_time - WorldGameTime::GetGameTime()) * 1000;
                }
                else
                    quest_time = 0;

                questStatusData.CreatureOrGOCount[0] = fields[5].GetUInt32();
                questStatusData.CreatureOrGOCount[1] = fields[6].GetUInt32();
                questStatusData.CreatureOrGOCount[2] = fields[7].GetUInt32();
                questStatusData.CreatureOrGOCount[3] = fields[8].GetUInt32();
                questStatusData.ItemCount[0] = fields[9].GetUInt32();
                questStatusData.ItemCount[1] = fields[10].GetUInt32();
                questStatusData.ItemCount[2] = fields[11].GetUInt32();
                questStatusData.ItemCount[3] = fields[12].GetUInt32();

                questStatusData.uState = QUEST_UNCHANGED;

                // add to quest log
                if (slot < MAX_QUEST_LOG_SIZE && questStatusData.Status != QUEST_STATUS_NONE)
                {
                    SetQuestSlot(slot, quest_id, uint32(quest_time)); // cast can't be helped

                    if(questStatusData.Status == QUEST_STATUS_COMPLETE)
                        SetQuestSlotState(slot,QUEST_STATE_COMPLETE);
                    else if (questStatusData.Status == QUEST_STATUS_FAILED)
                        SetQuestSlotState(slot, QUEST_STATE_FAIL);

                    for(uint8 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
                        if(questStatusData.CreatureOrGOCount[idx])
                            SetQuestSlotCounter(slot,idx,questStatusData.CreatureOrGOCount[idx]);

                    ++slot;
                }

                if(questStatusData.Rewarded)
                {
                    // learn rewarded spell if unknown
                    learnQuestRewardedSpells(pQuest);

                    // set rewarded title if any
                    if(pQuest->GetCharTitleId())
                    {
                        if(CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(pQuest->GetCharTitleId()))
                            SetTitle(titleEntry,false,false);
                    }
                }
            }
        }
        while( result->NextRow() );
    }

    // clear quest log tail
    for ( uint16 i = slot; i < MAX_QUEST_LOG_SIZE; ++i )
        SetQuestSlot(i,0);
}

void Player::_LoadDailyQuestStatus(PreparedQueryResult result)
{
    for(uint32 quest_daily_idx = 0; quest_daily_idx < PLAYER_MAX_DAILY_QUESTS; ++quest_daily_idx)
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,0);

    //QueryResult result = CharacterDatabase.PQuery("SELECT quest,time FROM character_queststatus_daily WHERE guid = '%u'", GetGUID().GetCounter());

    if(result)
    {
        uint32 quest_daily_idx = 0;

        do
        {
            if(quest_daily_idx >= PLAYER_MAX_DAILY_QUESTS)  // max amount with exist data in query
            {
                TC_LOG_ERROR("entities.player","Player (GUID: %u) have more 25 daily quest records in `charcter_queststatus_daily`",GetGUID().GetCounter());
                break;
            }

            Field *fields = result->Fetch();

            uint32 quest_id = fields[0].GetUInt32();

            // save _any_ from daily quest times (it must be after last reset anyway)
            m_lastDailyQuestTime = (time_t)fields[1].GetUInt64();

            Quest const* pQuest = sObjectMgr->GetQuestTemplate(quest_id);
            if( !pQuest )
                continue;

            SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,quest_id);
            ++quest_daily_idx;
        }
        while( result->NextRow() );
    }

    m_DailyQuestChanged = false;
}

void Player::_LoadSpells(PreparedQueryResult result)
{
    //QueryResult result = CharacterDatabase.PQuery("SELECT spell, active, disabled FROM character_spell WHERE guid = '%u'",GetGUID().GetCounter());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            AddSpell(fields[0].GetUInt32(), fields[1].GetBool(), false, false, fields[2].GetBool(), true);
        }
        while( result->NextRow() );
    }
}

void Player::_LoadGroup(PreparedQueryResult result)
{
    //QueryResult* result = CharacterDatabase.PQuery("SELECT guid FROM group_member WHERE memberGuid=%u", GetGUID().GetCounter());
    if(result)
    {
        if (Group* group = sGroupMgr->GetGroupByDbStoreId((*result)[0].GetUInt32()))
        {
            uint8 subgroup = group->GetMemberGroup(GetGUID());
            SetGroup(group, subgroup);
            if(GetLevel() >= LEVELREQUIREMENT_HEROIC)
            {
                // the group leader may change the instance difficulty while the player is offline
                SetDifficulty(group->GetDifficulty(), false);
            }
        }
    }

    if (!GetGroup() || !GetGroup()->IsLeader(GetGUID()))
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GROUP_LEADER);
}

void Player::_LoadBoundInstances(PreparedQueryResult result)
{
    for(auto & m_boundInstance : m_boundInstances)
        m_boundInstance.clear();

    Group *group = GetGroup();

    //QueryResult result = CharacterDatabase.PQuery("SELECT id, permanent, map, difficulty, resettime FROM character_instance LEFT JOIN instance ON instance = id WHERE guid = '%u'", m_guid.GetCounter());
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            bool perm = fields[1].GetBool();
            uint32 mapId = fields[2].GetUInt16();
            uint32 instanceId = fields[0].GetUInt32();
            uint8 difficulty = fields[3].GetUInt8();
            time_t resetTime = (time_t)fields[4].GetUInt32();
            // the resettime for normal instances is only saved when the InstanceSave is unloaded
            // so the value read from the DB may be wrong here but only if the InstanceSave is loaded
            // and in that case it is not used

            bool deleteInstance = false;

            MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
            std::string mapname = mapEntry ? mapEntry->name[sWorld->GetDefaultDbcLocale()] : "Unknown";

            if (!mapEntry || !mapEntry->IsDungeon())
            {
                TC_LOG_ERROR("entities.player", "_LoadBoundInstances: player %s(%d) has bind to not existed or not dungeon map %d (%s)", GetName().c_str(), GetGUID().GetCounter(), mapId, mapname.c_str());
                deleteInstance = true;
            }
            else if (difficulty >= MAX_DIFFICULTY)
            {
                TC_LOG_ERROR("entities.player", "_LoadBoundInstances: player %s(%d) has bind to not existed difficulty %d instance for map %u (%s)", GetName().c_str(), GetGUID().GetCounter(), difficulty, mapId, mapname.c_str());
                deleteInstance = true;
            }
            else
            {
                /*
                MapDifficulty const* mapDiff = GetMapDifficultyData(mapId, Difficulty(difficulty));
                if (!mapDiff)
                {
                    TC_LOG_ERROR("entities.player", "_LoadBoundInstances: player %s(%d) has bind to not existed difficulty %d instance for map %u (%s)", GetName().c_str(), GetGUID().GetCounter(), difficulty, mapId, mapname.c_str());
                    deleteInstance = true;
                }
                else*/ if (!perm && group)
                {
                    TC_LOG_ERROR("entities.player", "_LoadBoundInstances: player %s(%d) is in group %d but has a non-permanent character bind to map %d (%s), %d, %d", GetName().c_str(), GetGUID().GetCounter(), group->GetLowGUID(), mapId, mapname.c_str(), instanceId, difficulty);
                    deleteInstance = true;
                }
            }

            if (deleteInstance)
            {
                CharacterDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%d' AND instance = '%d'", GetGUID().GetCounter(), instanceId);
                continue;
            }

            // since non permanent binds are always solo bind, they can always be reset
            InstanceSave *save = sInstanceSaveMgr->AddInstanceSave(mapId, instanceId, Difficulty(difficulty), resetTime, !perm, true);
            if(save)
                BindToInstance(save, perm, true);

        } while(result->NextRow());
    }
}

InstancePlayerBind* Player::GetBoundInstance(uint32 mapid, Difficulty difficulty)
{
    // some instances only have one difficulty
    MapDifficulty const* mapDiff = GetDownscaledMapDifficultyData(mapid, difficulty);
    if (!mapDiff)
        return nullptr;

    auto itr = m_boundInstances[difficulty].find(mapid);
    if(itr != m_boundInstances[difficulty].end())
        return &itr->second;

    return nullptr;
}

InstanceSave * Player::GetInstanceSave(uint32 mapid, bool /*raid*/)
{
    InstancePlayerBind *pBind = GetBoundInstance(mapid, GetDifficulty());
    InstanceSave *pSave = pBind ? pBind->save : nullptr;
    if (!pBind || !pBind->perm)
    {
        if (Group *group = GetGroup())
            if (InstanceGroupBind *groupBind = group->GetBoundInstance(GetDifficulty(), mapid))
                pSave = groupBind->save;
    }
    return pSave;
}

void Player::UnbindInstance(uint32 mapid, Difficulty difficulty, bool unload)
{
    auto itr = m_boundInstances[difficulty].find(mapid);
    UnbindInstance(itr, difficulty, unload);
}

void Player::UnbindInstance(BoundInstancesMap::iterator &itr, Difficulty difficulty, bool unload)
{
    if(itr != m_boundInstances[difficulty].end())
    {
        if(!unload)
            CharacterDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u' AND instance = '%u'", GetGUID().GetCounter(), itr->second.save->GetInstanceId());

#ifdef LICH_KING
        if (itr->second.perm)
        GetSession()->SendCalendarRaidLockout(itr->second.save, false);
#endif

        itr->second.save->RemovePlayer(this);               // save can become invalid
        itr = m_boundInstances[difficulty].erase(itr);
    }
}

InstancePlayerBind* Player::BindToInstance(InstanceSave *save, bool permanent, bool load)
{
    if(save)
    {
        InstancePlayerBind& bind = m_boundInstances[save->GetDifficulty()][save->GetMapId()];
        if(bind.save)
        {
            // update the save when the group kills a boss
            if(permanent != bind.perm || save != bind.save)
                if(!load) CharacterDatabase.PExecute("UPDATE character_instance SET instance = '%u', permanent = '%u' WHERE guid = '%u' AND instance = '%u'", save->GetInstanceId(), permanent, GetGUID().GetCounter(), bind.save->GetInstanceId());
        }
        else
            if(!load) CharacterDatabase.PExecute("REPLACE INTO character_instance (guid, instance, permanent) VALUES ('%u', '%u', '%u')", GetGUID().GetCounter(), save->GetInstanceId(), permanent);

        if(bind.save != save)
        {
            if(bind.save)
                bind.save->RemovePlayer(this);

            save->AddPlayer(this);
        }

        if(permanent)
            save->SetCanReset(false);

        bind.save = save;
        bind.perm = permanent;

        if (!load)
            TC_LOG_DEBUG("maps", "Player::BindToInstance: %s(%d) is now bound to map %d, instance %d, difficulty %d", GetName().c_str(), GetGUID().GetCounter(), save->GetMapId(), save->GetInstanceId(), save->GetDifficulty());

        sScriptMgr->OnPlayerBindToInstance(this, save->GetDifficulty(), save->GetMapId(), permanent);

        return &bind;
    }
    else
        return nullptr;
}

void Player::SendRaidInfo()
{
    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 4);

    uint32 counter = 0, i;
    for(i = 0; i < MAX_DIFFICULTY; i++)
        for (auto & itr : m_boundInstances[i])
            if(itr.second.perm)
                counter++;

    data << counter;

    time_t now = WorldGameTime::GetGameTime();

    for(i = 0; i < MAX_DIFFICULTY; i++)
    {
        for (auto & itr : m_boundInstances[i])
        {
            if(itr.second.perm)
            {
                InstanceSave *save = itr.second.save;
                uint32 nextReset = save->GetResetTime() - now;

                data << (save->GetMapId());
#ifdef LICH_KING
                data << uint32(save->GetDifficulty());                     // difficulty
                data << uint64(save->GetInstanceId());                     // instance id
                data << uint8(bind.extendState != EXTEND_STATE_EXPIRED);   // expired = 0
                data << uint8(bind.extendState == EXTEND_STATE_EXTENDED);  // extended = 1
                time_t nextReset = save->GetResetTime();
                if (bind.extendState == EXTEND_STATE_EXTENDED) {
                    nextReset = sInstanceSaveMgr->GetSubsequentResetTime(save->GetMapId(), save->GetDifficulty(), save->GetResetTime());
                }
                data << uint32(nextReset);                // reset time
#else
                data << uint32(nextReset);
                data << uint32(save->GetInstanceId());
                data << uint32(counter);
                counter--;
#endif
            }
        }
    }
    SendDirectMessage(&data);
}

/*
- called on every successful teleportation to a map
*/
void Player::SendSavedInstances()
{
    bool hasBeenSaved = false;
    WorldPacket data;

    for(auto & m_boundInstance : m_boundInstances)
    {
        for (auto & itr : m_boundInstance)
        {
            if(itr.second.perm)                                // only permanent binds are sent
            {
                hasBeenSaved = true;
                break;
            }
        }
    }

    //Send opcode 811. true or false means, whether you have current raid/heroic instances
    data.Initialize(SMSG_UPDATE_INSTANCE_OWNERSHIP);
    data << uint32(hasBeenSaved);
    SendDirectMessage(&data);

    if(!hasBeenSaved)
        return;

    for(auto & m_boundInstance : m_boundInstances)
    {
        for (auto & itr : m_boundInstance)
        {
            if(itr.second.perm)
            {
                data.Initialize(SMSG_UPDATE_LAST_INSTANCE);
                data << uint32(itr.second.save->GetMapId());
                SendDirectMessage(&data);
            }
        }
    }
}

bool Player::Satisfy(AccessRequirement const *ar, uint32 target_map, bool report)
{
    if(!IsGameMaster() && ar)
    {
        uint32 LevelMin = 0;
        if(GetLevel() < ar->levelMin && !sWorld->getConfig(CONFIG_INSTANCE_IGNORE_LEVEL))
            LevelMin = ar->levelMin;

        uint32 LevelMax = 0;
        if(ar->levelMax >= ar->levelMin && GetLevel() > ar->levelMax && !sWorld->getConfig(CONFIG_INSTANCE_IGNORE_LEVEL))
            LevelMax = ar->levelMax;

        uint32 missingItem = 0;
        if(ar->item)
        {
            if(!HasItemCount(ar->item, 1) &&
                (!ar->item2 || !HasItemCount(ar->item2, 1)))
                missingItem = ar->item;
        }
        else if(ar->item2 && !HasItemCount(ar->item2, 1))
            missingItem = ar->item2;

        uint32 missingKey = 0;
        uint32 missingHeroicQuest = 0;
        if(GetDifficulty() == DUNGEON_DIFFICULTY_HEROIC)
        {
            if(ar->heroicKey)
            {
                if(!HasItemCount(ar->heroicKey, 1) &&
                    (!ar->heroicKey2 || !HasItemCount(ar->heroicKey2, 1)))
                    missingKey = ar->heroicKey;
            }
            else if(ar->heroicKey2 && !HasItemCount(ar->heroicKey2, 1))
                missingKey = ar->heroicKey2;

            if(ar->heroicQuest && !GetQuestRewardStatus(ar->heroicQuest))
                missingHeroicQuest = ar->heroicQuest;
        }

        uint32 missingQuest = 0;
        if(ar->quest && !GetQuestRewardStatus(ar->quest))
            missingQuest = ar->quest;

        if(LevelMin || LevelMax || missingItem || missingKey || missingQuest || missingHeroicQuest)
        {
            if(report)
            {
                if(missingItem)
                    GetSession()->SendAreaTriggerMessage(GetSession()->GetTrinityString(LANG_LEVEL_MINREQUIRED_AND_ITEM), ar->levelMin, GetSession()->GetLocalizedItemName(missingItem).c_str());
                else if(missingKey)
                    SendTransferAborted(target_map, TRANSFER_ABORT_DIFFICULTY2);
                else if(missingHeroicQuest)
                    ChatHandler(this).SendSysMessage(ar->heroicQuestFailedText);
                else if(missingQuest)
                    ChatHandler(this).SendSysMessage(ar->questFailedText);
                else if(LevelMin)
                    GetSession()->SendAreaTriggerMessage(GetSession()->GetTrinityString(LANG_LEVEL_MINREQUIRED), LevelMin);
            }
            return false;
        }
    }
    return true;
}

bool Player::CheckInstanceValidity(bool /*isLogin*/)
{
    // game masters' instances are always valid
    if (IsGameMaster())
        return true;

    // non-instances are always valid
    Map* map = FindMap();
    if (!map || !map->IsDungeon())
        return true;

    // raid instances require the player to be in a raid group to be valid
    if (map->IsRaid() && !sWorld->getBoolConfig(CONFIG_INSTANCE_IGNORE_RAID))
        if (!GetGroup() || !GetGroup()->isRaidGroup())
            return false;

    if (Group* group = GetGroup())
    {
        // check if player's group is bound to this instance
        InstanceGroupBind* bind = group->GetBoundInstance(map->GetDifficulty(), map->GetId());
        if (!bind || !bind->save || bind->save->GetInstanceId() != map->GetInstanceId())
            return false;

        Map::PlayerList const& players = map->GetPlayers();
        if (!players.isEmpty())
            for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            {
                if (Player* otherPlayer = it->GetSource())
                {
                    if (otherPlayer->IsGameMaster())
                        continue;
                    if (!otherPlayer->m_InstanceValid) // ignore players that currently have a homebind timer active
                        continue;
                    if (group != otherPlayer->GetGroup())
                        return false;
                }
            }
    }
    else
    {
        // instance is invalid if we are not grouped and there are other players
        if (map->GetPlayersCountExceptGMs() > 1)
            return false;

        // check if the player is bound to this instance
        InstancePlayerBind* bind = GetBoundInstance(map->GetId(), map->GetDifficulty());
        if (!bind || !bind->save || bind->save->GetInstanceId() != map->GetInstanceId())
            return false;
    }

    return true;
}

bool Player::_LoadHomeBind(PreparedQueryResult result)
{
    PlayerInfo const *info = sObjectMgr->GetPlayerInfo(GetRace(), GetClass());
    if (!info)
    {
        TC_LOG_ERROR("entities.player","Player have incorrect race/class pair. Can't be loaded.");
        return false;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    bool ok = false;
    //QueryResult result = CharacterDatabase.PQuery("SELECT map,zone,position_x,position_y,position_z FROM character_homebind WHERE guid = '%u'", playerGuid.GetCounter());
    if (result)
    {
        Field *fields = result->Fetch();
        m_homebindMapId = fields[0].GetUInt32();
        m_homebindAreaId = fields[1].GetUInt32();
        m_homebindX = fields[2].GetFloat();
        m_homebindY = fields[3].GetFloat();
        m_homebindZ = fields[4].GetFloat();

        // accept saved data only for valid position (and non instanceable)
        if( MapManager::IsValidMapCoord(m_homebindMapId,m_homebindX,m_homebindY,m_homebindZ) &&
            !sMapStore.LookupEntry(m_homebindMapId)->Instanceable() )
        {
            ok = true;
        }
        else
            trans->PAppend("DELETE FROM character_homebind WHERE guid = '%u'", GetGUID().GetCounter());
    }

    if(!ok)
    {
        if (sWorld->getConfig(CONFIG_BETASERVER_ENABLED))
        {
            float o;
            GetBetaZoneCoord(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ, o);
            m_homebindAreaId = sMapMgr->GetAreaId(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ);
        }
        else {
            m_homebindMapId = info->mapId;
            m_homebindAreaId = info->areaId;
            m_homebindX = info->positionX;
            m_homebindY = info->positionY;
            m_homebindZ = info->positionZ;
        }

        trans->PAppend("INSERT INTO character_homebind (guid,map,zone,position_x,position_y,position_z) VALUES ('%u', '%u', '%u', '%f', '%f', '%f')", GetGUID().GetCounter(), m_homebindMapId, (uint32)m_homebindAreaId, m_homebindX, m_homebindY, m_homebindZ);
    }
    if(trans->GetSize())
        CharacterDatabase.CommitTransaction(trans);

    TC_LOG_DEBUG("entities.player","Setting player home position: mapid is: %u, zoneid is %u, X is %f, Y is %f, Z is %f",
        m_homebindMapId, m_homebindAreaId, m_homebindX, m_homebindY, m_homebindZ);

    return true;
}

/*********************************************************/
/***                   SAVE SYSTEM                     ***/
/*********************************************************/

void Player::SaveToDB(bool create /*=false*/)
{
    // delay auto save at any saves (manual, in code, or autosave)
    m_nextSave = sWorld->getConfig(CONFIG_INTERVAL_SAVE);

    //lets allow only players in world to be saved
    if (IsBeingTeleportedFar())
    {
        ScheduleDelayedOperation(DELAYED_SAVE_PLAYER);
        return;
    }

    // first save/honor gain after midnight will also update the player's honor fields
    UpdateHonorFields();

    if (!create)
        sScriptMgr->OnPlayerSave(this); 

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt = nullptr;
    uint8 index = 0;

    if (create)
    {
        //! Insert query
        /// @todo: Filter out more redundant fields that can take their default value at player create
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER);
        stmt->setUInt32(index++, GetGUID().GetCounter());
        stmt->setUInt32(index++, GetSession()->GetAccountId());
        stmt->setString(index++, GetName());
        stmt->setUInt8(index++, GetRace());
        stmt->setUInt8(index++, GetClass());
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_GENDER));   // save gender from PLAYER_BYTES_3, UNIT_BYTES_0 changes with every transform effect
        stmt->setUInt8(index++, GetLevel());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_XP));
        stmt->setUInt32(index++, GetMoney());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_BYTES));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_BYTES_2));
        /*stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_SKIN_ID));
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_FACE_ID));
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_HAIR_STYLE_ID));
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES, PLAYER_BYTES_OFFSET_HAIR_COLOR_ID));
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_OFFSET_FACIAL_STYLE));
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_OFFSET_BANK_BAG_SLOTS));
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_OFFSET_REST_STATE));*/
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FLAGS));
        stmt->setUInt16(index++, (uint16)GetMapId());
        stmt->setUInt32(index++, (uint32)GetInstanceId());
        stmt->setUInt8(index++, (uint8(GetDungeonDifficulty()) | uint8(GetRaidDifficulty()) << 4));
        stmt->setFloat(index++, finiteAlways(GetPositionX()));
        stmt->setFloat(index++, finiteAlways(GetPositionY()));
        stmt->setFloat(index++, finiteAlways(GetPositionZ()));
        stmt->setFloat(index++, finiteAlways(GetOrientation()));
        stmt->setFloat(index++, finiteAlways(GetTransOffsetX()));
        stmt->setFloat(index++, finiteAlways(GetTransOffsetY()));
        stmt->setFloat(index++, finiteAlways(GetTransOffsetZ()));
        stmt->setFloat(index++, finiteAlways(GetTransOffsetO()));
        ObjectGuid::LowType transLowGUID = 0;
        if (GetTransport())
            transLowGUID = GetTransport()->GetGUID().GetCounter();
        stmt->setUInt32(index++, transLowGUID);

        std::ostringstream ss;
        ss << m_taxi;
        stmt->setString(index++, ss.str());
        stmt->setUInt8(index++, m_cinematic);
        stmt->setUInt32(index++, m_Played_time[PLAYED_TIME_TOTAL]);
        stmt->setUInt32(index++, m_Played_time[PLAYED_TIME_LEVEL]);
        stmt->setFloat(index++, finiteAlways(m_rest_bonus));
        stmt->setUInt32(index++, uint32(WorldGameTime::GetGameTime()));
        stmt->setUInt8(index++, (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ? 1 : 0));
        //save, far from tavern/city
        //save, but in tavern/city
        stmt->setUInt32(index++, m_resetTalentsCost);
        stmt->setUInt32(index++, uint32(m_resetTalentsTime));
        stmt->setUInt16(index++, (uint16)m_ExtraFlags);
        stmt->setUInt8(index++, m_stableSlots);
        stmt->setUInt16(index++, (uint16)m_atLoginFlags);
        stmt->setUInt16(index++, GetZoneId());
        stmt->setUInt32(index++, uint32(m_deathExpireTime));

        ss.str("");
        ss << m_taxi.SaveTaxiDestinationsToString();

        stmt->setString(index++, ss.str());
        stmt->setUInt32(index++, GetArenaPoints());
        stmt->setUInt32(index++, GetHonorPoints());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
        stmt->setUInt16(index++, GetUInt16Value(PLAYER_FIELD_KILLS, 0));
        stmt->setUInt16(index++, GetUInt16Value(PLAYER_FIELD_KILLS, 1));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_CHOSEN_TITLE));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX));
        stmt->setUInt8(index++, GetDrunkValue());
        stmt->setUInt32(index++, GetHealth());

        for (uint32 i = 0; i < MAX_POWERS; ++i)
            stmt->setUInt32(index++, GetPower(Powers(i)));

        stmt->setUInt32(index++, GetSession()->GetLatency());

        ss.str("");
        for (uint32 i = 0; i < PLAYER_EXPLORED_ZONES_SIZE; ++i)
            ss << GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + i) << ' ';
        stmt->setString(index++, ss.str());

        ss.str("");
        // cache equipment...
#ifdef LICH_KING
        for (uint32 i = 0; i < EQUIPMENT_SLOT_END * 2; ++i)
            ss << GetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + i) << ' ';

        // ...and bags for enum opcode
        for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                ss << item->GetEntry();
            else
                ss << '0';
            ss << " 0 ";
        }
#else
        for (uint32 i = 0; i < 304; ++i) {
            if (i % 16 == 2 || i % 16 == 3) //save only PLAYER_VISIBLE_ITEM_*_0 + PLAYER_VISIBLE_ITEM_*_PROPERTIES
                ss << GetUInt32Value(PLAYER_VISIBLE_ITEM_1_CREATOR + i) << " ";
        }
#endif

        stmt->setString(index++, ss.str());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_AMMO_ID));

        ss.str("");
#ifdef LICH_KING
        for (uint32 i = 0; i < KNOWN_TITLES_SIZE * 2; ++i)
            ss << GetUInt32Value(PLAYER__FIELD_KNOWN_TITLES + i) << ' ';
#else
        for (uint32 i = 0; i < 2; ++i)
            ss << GetUInt32Value(PLAYER_FIELD_KNOWN_TITLES + i) << " ";
#endif
        stmt->setString(index++, ss.str());
        stmt->setUInt8(index++, GetByteValue(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_ACTION_BAR_TOGGLES));
        stmt->setUInt8(index++, m_isXpBlocked);
    }
    else
    {
        /*PrepareStatement(CHAR_UPD_CHARACTER, "UPDATE characters SET name=?,race=?,class=?,gender=?,level=?,xp=?,money=?,playerBytes=?,playerBytes2=?,playerFlags=?,"
            "map=?,instance_id=?,dungeon_difficulty=?,position_x=?,position_y=?,position_z=?,orientation=?,trans_x=?,trans_y=?,trans_z=?,trans_o=?,transguid=?,taximask=?,cinematic=?,totaltime=?,leveltime=?,rest_bonus=?,"
            "logout_time=?,is_logout_resting=?,resettalents_cost=?,resettalents_time=?,extra_flags=?,stable_slots=?,at_login=?,zone=?,death_expire_time=?,taxi_path=?,"
            "arenapoints=?,totalHonorPoints=?,todayHonorPoints=?,yesterdayHonorPoints=?,totalKills=?,todayKills=?,yesterdayKills=?,chosenTitle=?,"
            "watchedFaction=?,drunk=?,health=?,power1=?,power2=?,power3=?,power4=?,power5=?,latency=?,exploredZones=?,"
            "equipmentCache=?,ammoId=?,knownTitles=?,actionBars=?,online=?,xp_blocked=? WHERE guid=?", CONNECTION_ASYNC);*/
        // Update query
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER);
        stmt->setString(index++, GetName());
        stmt->setUInt8(index++, GetRace());
        stmt->setUInt8(index++, GetClass());
        stmt->setUInt8(index++, GetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_GENDER));   // save gender from PLAYER_BYTES_3, UNIT_BYTES_0 changes with every transform effect
        stmt->setUInt8(index++, GetLevel());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_XP));
        stmt->setUInt32(index++, GetMoney());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_BYTES));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_BYTES_2)); //sun: keep this field for reskin case!
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FLAGS));

        if (!IsBeingTeleported())
        {
            stmt->setUInt16(index++, (uint16)GetMapId());
            stmt->setUInt32(index++, (uint32)GetInstanceId());
            stmt->setUInt8(index++, (uint8(GetDungeonDifficulty()) | uint8(GetRaidDifficulty()) << 4));
            stmt->setFloat(index++, finiteAlways(GetPositionX()));
            stmt->setFloat(index++, finiteAlways(GetPositionY()));
            stmt->setFloat(index++, finiteAlways(GetPositionZ()));
            stmt->setFloat(index++, finiteAlways(GetOrientation()));
        }
        else
        {
            stmt->setUInt16(index++, (uint16)GetTeleportDest().GetMapId());
            stmt->setUInt32(index++, (uint32)0);
            stmt->setUInt8(index++, (uint8(GetDungeonDifficulty()) | uint8(GetRaidDifficulty()) << 4));
            stmt->setFloat(index++, finiteAlways(GetTeleportDest().GetPositionX()));
            stmt->setFloat(index++, finiteAlways(GetTeleportDest().GetPositionY()));
            stmt->setFloat(index++, finiteAlways(GetTeleportDest().GetPositionZ()));
            stmt->setFloat(index++, finiteAlways(GetTeleportDest().GetOrientation()));
        }

        stmt->setFloat(index++, finiteAlways(GetTransOffsetX()));
        stmt->setFloat(index++, finiteAlways(GetTransOffsetY()));
        stmt->setFloat(index++, finiteAlways(GetTransOffsetZ()));
        stmt->setFloat(index++, finiteAlways(GetTransOffsetO()));
        ObjectGuid::LowType transLowGUID = 0;
        if (GetTransport())
            transLowGUID = GetTransport()->GetGUID().GetCounter();
        stmt->setUInt32(index++, transLowGUID);

        std::ostringstream ss;
        ss << m_taxi;
        stmt->setString(index++, ss.str());
        stmt->setUInt8(index++, m_cinematic);
        stmt->setUInt32(index++, m_Played_time[PLAYED_TIME_TOTAL]);
        stmt->setUInt32(index++, m_Played_time[PLAYED_TIME_LEVEL]);
        stmt->setFloat(index++, finiteAlways(m_rest_bonus));
        stmt->setUInt32(index++, uint32(WorldGameTime::GetGameTime()));
        stmt->setUInt8(index++, (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ? 1 : 0));
        //save, far from tavern/city
        //save, but in tavern/city
        stmt->setUInt32(index++, m_resetTalentsCost);
        stmt->setUInt32(index++, uint32(m_resetTalentsTime));
        stmt->setUInt16(index++, (uint16)m_ExtraFlags);
        stmt->setUInt8(index++, m_stableSlots);
        stmt->setUInt16(index++, (uint16)m_atLoginFlags);
        stmt->setUInt16(index++, GetZoneId());
        stmt->setUInt32(index++, uint32(m_deathExpireTime));

        ss.str("");
        ss << m_taxi.SaveTaxiDestinationsToString();

        stmt->setString(index++, ss.str());
        stmt->setUInt32(index++, GetArenaPoints());
        stmt->setUInt32(index++, GetHonorPoints());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
        stmt->setUInt16(index++, GetUInt16Value(PLAYER_FIELD_KILLS, 0));
        stmt->setUInt16(index++, GetUInt16Value(PLAYER_FIELD_KILLS, 1));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_CHOSEN_TITLE));
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX));
        stmt->setUInt8(index++, GetDrunkValue());
        stmt->setUInt32(index++, GetHealth());

        for (uint32 i = 0; i < MAX_POWERS; ++i)
            stmt->setUInt32(index++, GetPower(Powers(i)));

        stmt->setUInt32(index++, GetSession()->GetLatency());

        ss.str("");
        for (uint32 i = 0; i < PLAYER_EXPLORED_ZONES_SIZE; ++i)
            ss << GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + i) << ' ';
        stmt->setString(index++, ss.str());

        ss.str("");
        // cache equipment...
#ifdef LICH_KING
        for (uint32 i = 0; i < EQUIPMENT_SLOT_END * 2; ++i)
            ss << GetUInt32Value(PLAYER_VISIBLE_ITEM_1_ENTRYID + i) << ' ';

        // ...and bags for enum opcode
        for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                ss << item->GetEntry();
            else
                ss << '0';
            ss << " 0 ";
        }
#else
        for (uint32 i = 0; i < 304; ++i) {
            if (i % 16 == 2 || i % 16 == 3) //save only PLAYER_VISIBLE_ITEM_*_0 + PLAYER_VISIBLE_ITEM_*_PROPERTIES
                ss << GetUInt32Value(PLAYER_VISIBLE_ITEM_1_CREATOR + i) << " ";
        }
#endif
        stmt->setString(index++, ss.str());
        stmt->setUInt32(index++, GetUInt32Value(PLAYER_AMMO_ID));

        ss.str("");
#ifdef LICH_KING
        for (uint32 i = 0; i < KNOWN_TITLES_SIZE * 2; ++i)
            ss << GetUInt32Value(PLAYER__FIELD_KNOWN_TITLES + i) << ' ';
#else
        for (uint32 i = 0; i < 2; ++i)
            ss << GetUInt32Value(PLAYER_FIELD_KNOWN_TITLES + i) << " ";
#endif

        stmt->setString(index++, ss.str());
        stmt->setUInt8(index++, GetByteValue(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_ACTION_BAR_TOGGLES));
        stmt->setUInt8(index++, m_isXpBlocked);
        stmt->setUInt8(index++, IsInWorld() && !GetSession()->PlayerLogout() ? 1 : 0);
        stmt->setUInt32(index++, GetGUID().GetCounter());
    }

    trans->Append(stmt);

    if(m_mailsUpdated)                                     //save mails only when needed
        _SaveMail(trans);
    
    _SaveBGData(trans);
    _SaveInventory(trans);
    _SaveQuestStatus(trans);
    _SaveDailyQuestStatus(trans);
    _SaveSpells(trans);
    GetSpellHistory()->SaveToDB<Player>(trans);
    _SaveActions(trans);
    _SaveAuras(trans);
    _SaveSkills(trans);
    m_reputationMgr->SaveToDB(trans);
    GetSession()->SaveTutorialsData(trans);                 // changed only while character in game

    WorldSession* session = GetSession(); //This player object won't exist anymore when executing the callback, so extract session in a variable to capture
    GetSession()->GetQueryProcessor().AddQuery(CharacterDatabase.CommitTransaction(trans).WithCallback([session, create]() -> void
    {
        //sun: Moved CHAR_CREATE_SUCCESS here, we need to ensure character is created before sending it.
        //     (else client may end up with a char enum without the newly created character)
        if (create)
            session->SendCharCreate(CHAR_CREATE_SUCCESS);
    }));
   
    // save pet (hunter pet level and experience and all type pets health/mana).
    if(Pet* pet = GetPet())
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
}

// fast save function for item/money cheating preventing - save only inventory and money state
void Player::SaveInventoryAndGoldToDB(SQLTransaction trans)
{
    _SaveInventory(trans);
    SaveGoldToDB(trans);
}

void Player::SaveGoldToDB(SQLTransaction trans)
{
    trans->PAppend("UPDATE characters SET money = '%u' WHERE guid = '%u'", GetMoney(), GetGUID().GetCounter());
}

void Player::_SaveActions(SQLTransaction trans)
{
    for(auto itr = m_actionButtons.begin(); itr != m_actionButtons.end(); )
    {
        switch (itr->second.uState)
        {
            case ACTIONBUTTON_NEW:
                trans->PAppend("INSERT INTO character_action (guid,button,action,type,misc) VALUES ('%u', '%u', '%u', '%u', '%u')",
                    GetGUID().GetCounter(), (uint32)itr->first, (uint32)itr->second.action, (uint32)itr->second.type, (uint32)itr->second.misc );
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
                break;
            case ACTIONBUTTON_CHANGED:
                trans->PAppend("UPDATE character_action  SET action = '%u', type = '%u', misc= '%u' WHERE guid= '%u' AND button= '%u' ",
                    (uint32)itr->second.action, (uint32)itr->second.type, (uint32)itr->second.misc, GetGUID().GetCounter(), (uint32)itr->first );
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
                break;
            case ACTIONBUTTON_DELETED:
                trans->PAppend("DELETE FROM character_action WHERE guid = '%u' and button = '%u'", GetGUID().GetCounter(), (uint32)itr->first );
                m_actionButtons.erase(itr++);
                break;
            default:
                ++itr;
                break;
        };
    }
}

void Player::_SaveAuras(SQLTransaction trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_AURA);
    stmt->setUInt32(0, GetGUID().GetCounter());
    trans->Append(stmt);

    for (AuraMap::const_iterator itr = m_ownedAuras.begin(); itr != m_ownedAuras.end(); ++itr)
    {
        if (!itr->second->CanBeSaved())
            continue;

        Aura* aura = itr->second;

        int32 damage[MAX_SPELL_EFFECTS];
        int32 baseDamage[MAX_SPELL_EFFECTS];
        uint8 effMask = 0;
        uint8 recalculateMask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (AuraEffect const* effect = aura->GetEffect(i))
            {
                baseDamage[i] = effect->GetBaseAmount();
                damage[i] = effect->GetAmount();
                effMask |= 1 << i;
                if (effect->CanBeRecalculated())
                    recalculateMask |= 1 << i;
            }
            else
            {
                baseDamage[i] = 0;
                damage[i] = 0;
            }
        }

        uint8 index = 0;
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_AURA);
        stmt->setUInt32(index++, GetGUID().GetCounter());
        stmt->setUInt64(index++, itr->second->GetCasterGUID().GetRawValue());
        stmt->setUInt64(index++, itr->second->GetCastItemGUID().GetRawValue());
        stmt->setUInt32(index++, itr->second->GetId());
        stmt->setUInt8(index++, effMask);
        stmt->setUInt8(index++, recalculateMask);
        stmt->setUInt8(index++, itr->second->GetStackAmount());
        stmt->setInt32(index++, damage[0]);
        stmt->setInt32(index++, damage[1]);
        stmt->setInt32(index++, damage[2]);
        stmt->setInt32(index++, baseDamage[0]);
        stmt->setInt32(index++, baseDamage[1]);
        stmt->setInt32(index++, baseDamage[2]);
        stmt->setInt32(index++, itr->second->GetMaxDuration());
        stmt->setInt32(index++, itr->second->GetDuration());
        stmt->setUInt8(index++, itr->second->GetCharges());
        stmt->setFloat(index++, itr->second->GetCritChance());
        stmt->setBool(index++, itr->second->CanApplyResilience());
        trans->Append(stmt);
    }
}

void Player::_SaveBGData(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PLAYER_BGDATA);
    stmt->setUInt32(0, GetGUID().GetCounter());
    trans->Append(stmt);
    /* guid, bgInstanceID, bgTeam, x, y, z, o, map, taxi[0], taxi[1], mountSpell */
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PLAYER_BGDATA);
    stmt->setUInt32(0, GetGUID().GetCounter());
    stmt->setUInt32(1, m_bgData.bgInstanceID);
    stmt->setUInt16(2, m_bgData.bgTeam);
    stmt->setFloat(3, m_bgData.joinPos.GetPositionX());
    stmt->setFloat(4, m_bgData.joinPos.GetPositionY());
    stmt->setFloat(5, m_bgData.joinPos.GetPositionZ());
    stmt->setFloat(6, m_bgData.joinPos.GetOrientation());
    stmt->setUInt16(7, m_bgData.joinPos.GetMapId());
    stmt->setUInt16(8, m_bgData.taxiPath[0]);
    stmt->setUInt16(9, m_bgData.taxiPath[1]);
    stmt->setUInt16(10, m_bgData.mountSpell);
    trans->Append(stmt);
}

void Player::_SaveInventory(SQLTransaction trans)
{
    PreparedStatement* stmt;
    // force items in buyback slots to new state
    // and remove those that aren't already
    for (uint8 i = BUYBACK_SLOT_START; i < BUYBACK_SLOT_END; i++)
    {
        Item *item = m_items[i];
        if (!item)
            continue;

        if (item->GetState() == ITEM_NEW)
        {
            if (ItemTemplate const* itemTemplate = item->GetTemplate())
                if (itemTemplate->Flags & ITEM_FLAG_HAS_LOOT)
                    sLootItemStorage->RemoveStoredLootForContainer(item->GetGUID().GetCounter());

            continue;
        }

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_INVENTORY_BY_ITEM);
        stmt->setUInt32(0, item->GetGUID().GetCounter());
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_INSTANCE);
        stmt->setUInt32(0, item->GetGUID().GetCounter());
        trans->Append(stmt);

        m_items[i]->FSetState(ITEM_NEW);

        if (ItemTemplate const* itemTemplate = item->GetTemplate())
            if (itemTemplate->Flags & ITEM_FLAG_HAS_LOOT)
                sLootItemStorage->RemoveStoredLootForContainer(item->GetGUID().GetCounter());
    }

    // update enchantment durations
    for(auto & itr : m_enchantDuration)
    {
        itr.item->SetEnchantmentDuration(itr.slot,itr.leftduration);
    }

    // if no changes
    if (m_itemUpdateQueue.empty()) return;

    // do not save if the update queue is corrupt
    bool error = false;
    for(auto item : m_itemUpdateQueue)
    {
        if(!item || item->GetState() == ITEM_REMOVED) continue;
        Item *test = GetItemByPos( item->GetBagSlot(), item->GetSlot());

        if (test == nullptr)
        {
            TC_LOG_ERROR("entities.player","POSSIBLE ITEM DUPLICATION ATTEMPT: Player(GUID: %u Name: %s)::_SaveInventory - the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have an item at that position!", GetGUID().GetCounter(), GetName().c_str(), item->GetBagSlot(), item->GetSlot(), item->GetGUID().GetCounter());
            error = true;
        }
        else if (test != item)
        {
            TC_LOG_ERROR("entities.player","Player(GUID: %u Name: %s)::_SaveInventory - the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the item with guid %d is there instead!", GetGUID().GetCounter(), GetName().c_str(), item->GetBagSlot(), item->GetSlot(), item->GetGUID().GetCounter(), test->GetGUID().GetCounter());
            error = true;
        }
    }

    if (error)
    {
        TC_LOG_ERROR("entities.player","Player::_SaveInventory - one or more errors occurred save aborted!");
        ChatHandler(this).SendSysMessage(LANG_ITEM_SAVE_FAILED);
        m_itemUpdateQueue.clear();
        return;
    }

    for(auto item : m_itemUpdateQueue)
    {
        if(!item) continue;

        Bag *container = item->GetContainer();
        ObjectGuid::LowType bag_guid = container ? container->GetGUID().GetCounter() : 0;

        switch(item->GetState())
        {
            case ITEM_NEW:
                trans->PAppend("INSERT INTO character_inventory (guid,bag,slot,item,item_template) VALUES ('%u', '%u', '%u', '%u', '%u')", GetGUID().GetCounter(), bag_guid, item->GetSlot(), item->GetGUID().GetCounter(), item->GetEntry());
                break;
            case ITEM_CHANGED:
                trans->PAppend("UPDATE character_inventory SET guid='%u', bag='%u', slot='%u', item_template='%u' WHERE item='%u'", GetGUID().GetCounter(), bag_guid, item->GetSlot(), item->GetEntry(), item->GetGUID().GetCounter());
                break;
            case ITEM_REMOVED:
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_INVENTORY_BY_ITEM);
                stmt->setUInt32(0, item->GetGUID().GetCounter());
                trans->Append(stmt);
                break;
            case ITEM_UNCHANGED:
                break;
        }

        item->SaveToDB(trans);                                   // item have unchanged inventory record and can be save standalone
    }
    m_itemUpdateQueue.clear();
}

void Player::_SaveMail(SQLTransaction trans)
{
    if (!m_mailsLoaded)
        return;

    PreparedStatement* stmt;

    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        Mail* m = (*itr);
        if (m->state == MAIL_STATE_CHANGED)
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_MAIL);
            stmt->setUInt8(0, uint8(m->HasItems() ? 1 : 0));
            stmt->setUInt32(1, uint32(m->expire_time));
            stmt->setUInt32(2, uint32(m->deliver_time));
            stmt->setUInt32(3, m->money);
            stmt->setUInt32(4, m->COD);
            stmt->setUInt8(5, uint8(m->checked));
            stmt->setUInt32(6, m->messageID);

            trans->Append(stmt);

            if (!m->removedItems.empty())
            {
                for (std::vector<uint32>::iterator itr2 = m->removedItems.begin(); itr2 != m->removedItems.end(); ++itr2)
                {
                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM);
                    stmt->setUInt32(0, *itr2);
                    trans->Append(stmt);
                }
                m->removedItems.clear();
            }
            m->state = MAIL_STATE_UNCHANGED;
        }
        else if (m->state == MAIL_STATE_DELETED)
        {
            if (m->HasItems())
            {
                for (MailItemInfoVec::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
                {
                    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_INSTANCE);
                    stmt->setUInt32(0, itr2->item_guid);
                    trans->Append(stmt);
                }
            }
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_BY_ID);
            stmt->setUInt32(0, m->messageID);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_MAIL_ITEM_BY_ID);
            stmt->setUInt32(0, m->messageID);
            trans->Append(stmt);
        }
    }

    //deallocate deleted mails...
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end();)
    {
        if ((*itr)->state == MAIL_STATE_DELETED)
        {
            Mail* m = *itr;
            m_mail.erase(itr);
            delete m;
            itr = m_mail.begin();
        }
        else
            ++itr;
    }

    m_mailsUpdated = false;
}

void Player::_SaveQuestStatus(SQLTransaction trans)
{
    for(auto & m_QuestStatu : m_QuestStatus)
    {
        switch (m_QuestStatu.second.uState)
        {
            case QUEST_NEW :
                trans->PAppend("INSERT INTO character_queststatus (guid,quest,status,rewarded,explored,timer,mobcount1,mobcount2,mobcount3,mobcount4,itemcount1,itemcount2,itemcount3,itemcount4) "
                    "VALUES ('%u', '%u', '%u', '%u', '%u', '" UI64FMTD "', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')",
                    GetGUID().GetCounter(), m_QuestStatu.first, m_QuestStatu.second.Status, m_QuestStatu.second.Rewarded, m_QuestStatu.second.Explored, uint64(m_QuestStatu.second.m_timer / 1000 + WorldGameTime::GetGameTime()), m_QuestStatu.second.CreatureOrGOCount[0], m_QuestStatu.second.CreatureOrGOCount[1], m_QuestStatu.second.CreatureOrGOCount[2], m_QuestStatu.second.CreatureOrGOCount[3], m_QuestStatu.second.ItemCount[0], m_QuestStatu.second.ItemCount[1], m_QuestStatu.second.ItemCount[2], m_QuestStatu.second.ItemCount[3]);
                break;
            case QUEST_CHANGED :
                trans->PAppend("UPDATE character_queststatus SET status = '%u',rewarded = '%u',explored = '%u',timer = '" UI64FMTD "',mobcount1 = '%u',mobcount2 = '%u',mobcount3 = '%u',mobcount4 = '%u',itemcount1 = '%u',itemcount2 = '%u',itemcount3 = '%u',itemcount4 = '%u'  WHERE guid = '%u' AND quest = '%u' ",
                    m_QuestStatu.second.Status, m_QuestStatu.second.Rewarded, m_QuestStatu.second.Explored, uint64(m_QuestStatu.second.m_timer / 1000 + WorldGameTime::GetGameTime()), m_QuestStatu.second.CreatureOrGOCount[0], m_QuestStatu.second.CreatureOrGOCount[1], m_QuestStatu.second.CreatureOrGOCount[2], m_QuestStatu.second.CreatureOrGOCount[3], m_QuestStatu.second.ItemCount[0], m_QuestStatu.second.ItemCount[1], m_QuestStatu.second.ItemCount[2], m_QuestStatu.second.ItemCount[3], GetGUID().GetCounter(), m_QuestStatu.first );
                break;
            case QUEST_UNCHANGED:
                break;
        };
        m_QuestStatu.second.uState = QUEST_UNCHANGED;
    }
}

void Player::_SaveDailyQuestStatus(SQLTransaction trans)
{
    if(!m_DailyQuestChanged)
        return;

    m_DailyQuestChanged = false;

    // save last daily quest time for all quests: we need only mostly reset time for reset check anyway
    trans->PAppend("DELETE FROM character_queststatus_daily WHERE guid = '%u'",GetGUID().GetCounter());
    for(uint32 quest_daily_idx = 0; quest_daily_idx < PLAYER_MAX_DAILY_QUESTS; ++quest_daily_idx)
        if(GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx))
            trans->PAppend("INSERT INTO character_queststatus_daily (guid,quest,time) VALUES ('%u', '%u','" UI64FMTD "')",
                GetGUID().GetCounter(), GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx),uint64(m_lastDailyQuestTime));
}

void Player::_SaveSkills(SQLTransaction trans)
{
    for( auto itr = mSkillStatus.begin(); itr != mSkillStatus.end(); )
    {
        if(itr->second.uState == SKILL_UNCHANGED)
        {
            ++itr;
            continue;
        }

        if(itr->second.uState == SKILL_DELETED)
        {
            trans->PAppend("DELETE FROM character_skills WHERE guid = '%u' AND skill = '%u' ", GetGUID().GetCounter(), itr->first );
            mSkillStatus.erase(itr++);
            continue;
        }

        uint32 valueData = GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos));
        uint16 value = SKILL_VALUE(valueData);
        uint16 max = SKILL_MAX(valueData);

        switch (itr->second.uState)
        {
            case SKILL_NEW:
                trans->PAppend("INSERT INTO character_skills (guid, skill, value, max) VALUES ('%u', '%u', '%u', '%u')",
                    GetGUID().GetCounter(), itr->first, value, max);
                break;
            case SKILL_CHANGED:
                trans->PAppend("UPDATE character_skills SET value = '%u',max = '%u'WHERE guid = '%u' AND skill = '%u' ",
                    value, max, GetGUID().GetCounter(), itr->first );
                break;
            default:
                break;
        };
        itr->second.uState = SKILL_UNCHANGED;

        ++itr;
    }
}

void Player::_SaveSpells(SQLTransaction trans)
{
    for (PlayerSpellMap::const_iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        ++next;
        if (itr->second->state == PLAYERSPELL_REMOVED || itr->second->state == PLAYERSPELL_CHANGED)
            trans->PAppend("DELETE FROM character_spell WHERE guid = '%u' and spell = '%u'", GetGUID().GetCounter(), itr->first);

        // add only changed/new not dependent spells
        if ((!itr->second->dependent && itr->second->state == PLAYERSPELL_NEW) || itr->second->state == PLAYERSPELL_CHANGED)
            trans->PAppend("INSERT INTO character_spell (guid,spell,active,disabled) VALUES ('%u','%u','%u','%u')", GetGUID().GetCounter(), itr->first, uint32(itr->second->active), uint32(itr->second->disabled));

        if (itr->second->state == PLAYERSPELL_REMOVED)
            _removeSpell(itr->first);
        else
            itr->second->state = PLAYERSPELL_UNCHANGED;
    }
}

/*********************************************************/
/***               FLOOD FILTER SYSTEM                 ***/
/*********************************************************/

void Player::UpdateSpeakTime()
{
    // ignore chat spam protection for GMs in any mode
    if(GetSession()->GetSecurity() > SEC_PLAYER)
        return;

    time_t current = time (nullptr);
    if(m_speakTime > current)
    {
        uint32 max_count = sWorld->getConfig(CONFIG_CHATFLOOD_MESSAGE_COUNT);
        if(!max_count)
            return;

        ++m_speakCount;
        if(m_speakCount >= max_count)
        {
            // prevent overwrite mute time, if message send just before mutes set, for example.
            time_t new_mute = current + sWorld->getConfig(CONFIG_CHATFLOOD_MUTE_TIME);
            if(GetSession()->m_muteTime < new_mute)
                GetSession()->m_muteTime = new_mute;

            m_speakCount = 0;
        }
    }
    else
        m_speakCount = 0;

    m_speakTime = current + sWorld->getConfig(CONFIG_CHATFLOOD_MESSAGE_DELAY);
}

bool Player::CanSpeak() const
{
    return  GetSession()->m_muteTime <= time (nullptr);
}

/*********************************************************/
/***              LOW LEVEL FUNCTIONS:Notifiers        ***/
/*********************************************************/

void Player::SendAttackSwingNotInRange()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTINRANGE, 0);
    SendDirectMessage( &data );
}

// SQLTransaction here?
void Player::SavePositionInDB(uint32 mapid, float x,float y,float z,float o, uint32 zone, ObjectGuid guid)
{
    std::ostringstream ss;
    ss << "UPDATE characters SET position_x='"<<x<<"',position_y='"<<y
        << "',position_z='"<<z<<"',orientation='"<<o<<"',map='"<<mapid
        << "',zone='"<<zone<<"',trans_x='0',trans_y='0',trans_z='0',"
        << "transguid='0',taxi_path='' WHERE guid='"<< guid.GetCounter() <<"'";

    CharacterDatabase.Execute(ss.str().c_str());
}

void Player::SaveDataFieldToDB()
{
    std::ostringstream ss;
    ss<<"UPDATE characters SET data='";

    for(uint16 i = 0; i < m_valuesCount; i++ )
    {
        ss << GetUInt32Value(i) << " ";
    }
    ss<<"' WHERE guid='"<< GetGUID().GetCounter() <<"'";

    CharacterDatabase.Execute(ss.str().c_str());
}

bool Player::SaveValuesArrayInDB(Tokens const& tokens, ObjectGuid guid)
{
    std::ostringstream ss2;
    ss2<<"UPDATE characters SET data='";
    int i=0;
    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter, ++i)
    {
        ss2<<tokens[i]<<" ";
    }
    ss2<<"' WHERE guid='"<< guid.GetCounter() <<"'";

    CharacterDatabase.Execute(ss2.str().c_str());

    return true;
}

void Player::SetUInt32ValueInArray(Tokens& tokens,uint16 index, uint32 value)
{
    char buf[11];
    snprintf(buf,11,"%u",value);

    if(index >= tokens.size())
        return;

    tokens[index] = buf;
}

void Player::SetUInt32ValueInDB(uint16 index, uint32 value, ObjectGuid guid)
{
    Tokens tokens;
    if(!LoadValuesArrayFromDB(tokens,guid))
        return;

    if(index >= tokens.size())
        return;

    char buf[11];
    snprintf(buf,11,"%u",value);
    tokens[index] = buf;

    SaveValuesArrayInDB(tokens,guid);
}

void Player::SetFloatValueInDB(uint16 index, float value, ObjectGuid guid)
{
    uint32 temp;
    memcpy(&temp, &value, sizeof(value));
    Player::SetUInt32ValueInDB(index, temp, guid);
}

void Player::SendAttackSwingNotStanding()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTSTANDING, 0);
    SendDirectMessage( &data );
}

void Player::SendAttackSwingDeadTarget()
{
    WorldPacket data(SMSG_ATTACKSWING_DEADTARGET, 0);
    SendDirectMessage( &data );
}

void Player::SendAttackSwingCantAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_CANT_ATTACK, 0);
    SendDirectMessage( &data );
}

void Player::SendAttackSwingCancelAttack()
{
    WorldPacket data(SMSG_CANCEL_COMBAT, 0);
    SendDirectMessage( &data );
}

void Player::SendAttackSwingBadFacingAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_BADFACING, 0);
    SendDirectMessage( &data );
}

void Player::SendAutoRepeatCancel()
{
#ifdef LICH_KING
    WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, target->GetPackGUID().size());
    data << target->GetPackGUID(); // may be it's target guid
    SendMessageToSet(&data, true);
#else
    WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, 0);
    SendDirectMessage( &data );
#endif
}

void Player::SendFeignDeathResisted() const
{
#ifdef LICH_KING
    WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 9);
    data << target->GetGUID();
    data << uint8(0); //or 1
#else
    WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 0);
#endif
    GetSession()->SendPacket(&data);
}

void Player::PlaySound(uint32 Sound, bool OnlySelf)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << Sound;
    if (OnlySelf)
        SendDirectMessage( &data );
    else
        SendMessageToSet( &data, true );
}

void Player::SendExplorationExperience(uint32 Area, uint32 Experience)
{
    // LK ok
    WorldPacket data( SMSG_EXPLORATION_EXPERIENCE, 8 );
    data << Area;
    data << Experience;
    SendDirectMessage(&data);
}

void Player::SendDungeonDifficulty(bool IsInGroup)
{
    //LK ok
    uint8 val = 0x00000001;
    WorldPacket data(MSG_SET_DUNGEON_DIFFICULTY, 12);
    data << (uint32)GetDifficulty();
    data << uint32(val);
    data << uint32(IsInGroup);
    SendDirectMessage(&data);
}

#ifdef LICH_KING
void Player::SendRaidDifficulty(bool IsInGroup, int32 forcedDifficulty)
{
    uint8 val = 0x00000001;
    WorldPacket data(MSG_SET_RAID_DIFFICULTY, 12);
    data << uint32(forcedDifficulty == -1 ? GetRaidDifficulty() : forcedDifficulty);
    data << uint32(val);
    data << uint32(IsInGroup);
    SendDirectMessage(&data);
}
#endif

void Player::SendResetFailedNotify(uint32 mapid)
{
    //LK ok
    WorldPacket data(SMSG_RESET_FAILED_NOTIFY, 4);
    data << uint32(mapid);
    SendDirectMessage(&data);
}

/// Reset all solo instances and optionally send a message on success for each
void Player::ResetInstances(uint8 method, bool isRaid)
{
    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_JOIN

    // we assume that when the difficulty changes, all instances that can be reset will be
    uint8 dif = GetDifficulty();

    for (auto itr = m_boundInstances[dif].begin(); itr != m_boundInstances[dif].end();)
    {
        InstanceSave *p = itr->second.save;
        const MapEntry *entry = sMapStore.LookupEntry(itr->first);
        if(!entry || !p->CanReset() || entry->MapID == 580)
        {
            ++itr;
            continue;
        }

        if(method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if(dif == DUNGEON_DIFFICULTY_HEROIC || entry->map_type == MAP_RAID)
            {
                ++itr;
                continue;
            }
        }

#ifndef LICH_KING
        if (method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            if (entry->map_type == MAP_RAID) {
                ++itr;
                continue;
            }
        }
#endif

        // if the map is loaded, reset it
        Map *map = sMapMgr->FindMap(p->GetMapId(), p->GetInstanceId());
        if(map && map->IsDungeon())
            if(!((InstanceMap*)map)->Reset(method))
            {
                ++itr;
                continue;
            }

        // since this is a solo instance there should not be any players inside
        if(method == INSTANCE_RESET_ALL || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
            SendResetInstanceSuccess(p->GetMapId());

        p->DeleteFromDB();
        m_boundInstances[dif].erase(itr++);

        // the following should remove the instance save from the manager and delete it as well
        p->RemovePlayer(this);
    }
}

void Player::SendResetInstanceSuccess(uint32 MapId)
{
    //LK ok
    WorldPacket data(SMSG_INSTANCE_RESET, 4);
    data << MapId;
    SendDirectMessage(&data);
}

void Player::SendResetInstanceFailed(uint32 reason, uint32 MapId)
{
    //lk ok
    /*reasons for instance reset failure:
    // 0: There are players inside the instance.
    // 1: There are players offline in your party.
    // 2>: There are players in your party attempting to zone into an instance.
    */
    WorldPacket data(SMSG_INSTANCE_RESET_FAILED, 4);
    data << reason;
    data << MapId;
    SendDirectMessage(&data);
}

/*********************************************************/
/***              Update timers                        ***/
/*********************************************************/

///checks the 15 afk reports per 5 minutes limit
void Player::UpdateAfkReport(time_t currTime)
{
    if(m_bgAfkReportedTimer <= currTime)
    {
        m_bgAfkReportedCount = 0;
        m_bgAfkReportedTimer = currTime+5*MINUTE;
    }
}

void Player::UpdateContestedPvP(uint32 diff)
{
    if(!m_contestedPvPTimer||IsInCombat())
        return;
    if(m_contestedPvPTimer <= diff)
    {
        ResetContestedPvP();
    }
    else
        m_contestedPvPTimer -= diff;
}

void Player::UpdatePvPFlag(time_t currTime)
{
    if(!IsPvP())
        return;
    if(!IsInDuelArea() && (!pvpInfo.endTimer || currTime < (pvpInfo.endTimer + 300)))
        return;

    UpdatePvP(false);
}

void Player::SetContestedPvP(Player* attackedPlayer)
{
    if (attackedPlayer && (attackedPlayer == this || (duel && duel->Opponent == attackedPlayer)))
        return;

    SetContestedPvPTimer(30000);
    if (!HasUnitState(UNIT_STATE_ATTACK_PLAYER))
    {
        AddUnitState(UNIT_STATE_ATTACK_PLAYER);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
        // call MoveInLineOfSight for nearby contested guards
        Trinity::AIRelocationNotifier notifier(*this);
        Cell::VisitWorldObjects(this, notifier, GetVisibilityRange());
    }
    for (Unit* unit : m_Controlled)
    {
        if (!unit->HasUnitState(UNIT_STATE_ATTACK_PLAYER))
        {
            unit->AddUnitState(UNIT_STATE_ATTACK_PLAYER);
            Trinity::AIRelocationNotifier notifier(*unit);
            Cell::VisitWorldObjects(this, notifier, GetVisibilityRange());
        }
    }
}

void Player::ResetContestedPvP()
{
    ClearUnitState(UNIT_STATE_ATTACK_PLAYER);
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
    m_contestedPvPTimer = 0;
}

void Player::UpdateDuelFlag(time_t currTime)
{
    if (!duel || duel->State != DUEL_STATE_COUNTDOWN || duel->StartTime > currTime)
        return;

    sScriptMgr->OnPlayerDuelStart(this, duel->Opponent);

    SetUInt32Value(PLAYER_DUEL_TEAM, 1);
    duel->Opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 2);

    duel->State = DUEL_STATE_IN_PROGRESS;
    duel->Opponent->duel->State = DUEL_STATE_IN_PROGRESS;
}


Pet* Player::GetPet() const
{
    if (ObjectGuid pet_guid = GetPetGUID())
    {
        if (!pet_guid.IsPet())
            return nullptr;

        Pet* pet = ObjectAccessor::GetPet(*this, pet_guid);

        if (!pet)
            return nullptr;

        if (IsInWorld())
            return pet;

        // there may be a guardian in this slot
        //TC_LOG_ERROR("entities.player", "Player::GetPet: Pet %u does not exist.", pet_guid.GetCounter());
        //const_cast<Player*>(this)->SetPetGUID(0);
    }

    return nullptr;
}

Pet* Player::SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 duration)
{
    Pet* pet = new Pet(this, petType);

    if (petType == SUMMON_PET && pet->LoadPetFromDB(this, entry))
    {
        // Remove Demonic Sacrifice auras (known pet)
        auto const& auraClassScripts = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (auto itr = auraClassScripts.begin(); itr != auraClassScripts.end();)
        {
            if ((*itr)->GetMiscValue() == 2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }

        if (duration > 0)
            pet->SetDuration(duration);

        return nullptr;
    }

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if (!entry)
    {
        delete pet;
        return nullptr;
    }

    pet->Relocate(x, y, z, ang);

    if (!pet->IsPositionValid())
    {
        TC_LOG_ERROR("misc", "ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)", pet->GetGUID().GetCounter(), pet->GetEntry(), pet->GetPositionX(), pet->GetPositionY());
        delete pet;
        return nullptr;
    }

    Map *map = GetMap();
    uint32 pet_number = sObjectMgr->GeneratePetNumber();
    if (!pet->Create(map->GenerateLowGuid<HighGuid::Pet>(), map, GetPhaseMask(), entry, pet_number))
    {
        TC_LOG_ERROR("misc", "no such creature entry %u", entry);
        delete pet;
        return nullptr;
    }

    pet->SetCreatorGUID(GetGUID());
    pet->SetFaction(GetFaction());

    // this enables pet details window (Shift+P)
    pet->GetCharmInfo()->SetPetNumber(pet_number, false);

    pet->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
    pet->InitStatsForLevel(GetLevel());

    SetMinion(pet, true);

    switch (petType)
    {
    case SUMMON_PET:
        // this enables pet details window (Shift+P)
        pet->GetCharmInfo()->SetPetNumber(pet_number, true);
        pet->SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, CLASS_MAGE);
        pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 1000);
        pet->SetFullHealth();
        pet->SetPower(POWER_MANA, pet->GetMaxPower(POWER_MANA));
        break;
    default:
        break;
    }

    map->AddToMap(pet->ToCreature(), true);

    switch (petType)
    {
    case SUMMON_PET:
        pet->InitPetCreateSpells();
        //TC pet->InitTalentForLevel();
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        PetSpellInitialize();
        break;
    default:
        break;
    }

    if (petType == SUMMON_PET)
    {
        // Remove Demonic Sacrifice auras (known pet)
        auto const& auraClassScripts = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (auto itr = auraClassScripts.begin(); itr != auraClassScripts.end();)
        {
            if ((*itr)->GetMiscValue() == 2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }
    }

    if (duration > 0)
        pet->SetDuration(duration);

    return pet;
}

void Player::RemovePet(Pet* pet, PetSaveMode mode, bool returnreagent, RemovePetReason reason)
{
    if(!pet)
        pet = GetPet();

    if(returnreagent && (pet || m_temporaryUnsummonedPetNumber))
    {
        //returning of reagents only for players, so best done here
        uint32 spellId = pet ? pet->GetUInt32Value(UNIT_CREATED_BY_SPELL) : m_oldpetspell;
        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);

        if(spellInfo)
        {
            for(uint32 i = 0; i < 7; ++i)
            {
                if(spellInfo->Reagent[i] > 0)
                {
                    ItemPosCountVec dest;                   //for succubus, voidwalker, felhunter and felguard credit soulshard when despawn reason other than death (out of range, logout)
                    uint8 msg = CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, spellInfo->Reagent[i], spellInfo->ReagentCount[i] );
                    if( msg == EQUIP_ERR_OK )
                    {
                        Item* item = StoreNewItem( dest, spellInfo->Reagent[i], true);
                        if(IsInWorld())
                            SendNewItem(item,spellInfo->ReagentCount[i],true,false);
                    }
                }
            }
        }
        m_temporaryUnsummonedPetNumber = 0;
    }

    if(!pet || !pet->IsInWorld() || pet->GetOwnerGUID()!=GetGUID())
        return;

    pet->CombatStop();

    if(returnreagent)
    {
        switch(pet->GetEntry())
        {
            //warlock pets except imp are removed(?) when logging out
            case 1860:
            case 1863:
            case 417:
            case 17252:
                mode = PET_SAVE_NOT_IN_SLOT;
                break;
        }
    }

    pet->SavePetToDB(mode);

    SetMinion(pet, false);

    pet->AddObjectToRemoveList();
    pet->m_removed = true;

    if(pet->isControlled())
    {
        //LK ok
        WorldPacket data(SMSG_PET_SPELLS, 8);
        data << uint64(0);
        SendDirectMessage(&data);

        if(GetGroup())
            SetGroupUpdateFlag(GROUP_UPDATE_PET);
    }
}

void Player::StopCastingCharm(Aura* except /*= nullptr*/)
{
#ifdef LICH_KING
    if (IsGhouled())
    {
        RemoveGhoul();
        return;
    }
#endif

    Unit* charm = GetCharmed();
    if (!charm)
        return;

    if (charm->GetTypeId() == TYPEID_UNIT)
    {
        if (charm->ToCreature()->HasUnitTypeMask(UNIT_MASK_PUPPET))
            static_cast<Puppet*>(charm)->UnSummon();
#ifdef LICH_KING
        else if (charm->IsVehicle())
            ExitVehicle();
#endif
    }
    if (GetCharmedGUID())
        charm->RemoveCharmAuras(except);

    if (GetCharmedGUID())
    {
        TC_LOG_FATAL("entities.player", "Player::StopCastingCharm: Player '%s' (%s) is not able to uncharm unit (%s)", GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str(), ObjectGuid(GetCharmedGUID()).ToString().c_str());
        if (!ObjectGuid(charm->GetCharmerGUID()).IsEmpty())
        {
            TC_LOG_FATAL("entities.player", "Player::StopCastingCharm: Charmed unit has charmer %s", ObjectGuid(charm->GetCharmerGUID()).ToString().c_str());
            ABORT();
        }

        SetCharm(charm, false);
    }
}

void Player::Say(std::string const& text, Language language, WorldObject const* /*= nullptr*/)
{
    std::string _text(text);
   // sScriptMgr->OnPlayerChat(this, CHAT_MSG_SAY, language, _text);

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_SAY, language, this, this, _text);
    SendMessageToSetInRange(&data, sWorld->getConfig(CONFIG_LISTEN_RANGE_SAY), true);
}

void Player::Yell(std::string const& text, Language language, WorldObject const* /*= nullptr*/)
{
    std::string _text(text);
  //  sScriptMgr->OnPlayerChat(this, CHAT_MSG_YELL, language, _text);

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_YELL, language, this, this, _text);
    SendMessageToSetInRange(&data, sWorld->getConfig(CONFIG_LISTEN_RANGE_YELL), true);
}

void Player::TextEmote(std::string const& text, WorldObject const* /*= nullptr*/, bool /*= false*/)
{
    std::string _text(text);
   // sScriptMgr->OnPlayerChat(this, CHAT_MSG_EMOTE, LANG_UNIVERSAL, _text);

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_EMOTE, LANG_UNIVERSAL, this, this, _text);
    SendMessageToSetInRange(&data, sWorld->getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), true, /* !GetSession()->HasPermission(rbac::RBAC_PERM_TWO_SIDE_INTERACTION_CHAT) */ !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT));
}


void Player::Whisper(std::string const& text, Language language, Player* target, bool /*= false*/)
{
    ASSERT(target);

    bool isAddonMessage = (language == LANG_ADDON);

    if (!isAddonMessage)                                   // if not addon data
        language = LANG_UNIVERSAL;                          // whispers should always be readable

   // sScriptMgr->OnPlayerChat(this, CHAT_MSG_WHISPER, language, _text, target);

    // when player you are whispering to is dnd, he cannot receive your message, unless you are in gm mode
    if(!target->IsDND() || IsGameMaster())
    {
        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, language, this, target, text);
        target->SendDirectMessage(&data);

        // Also send message to sender. Do not send for addon messages
        if (language != LANG_ADDON) {
            ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER_INFORM, language, target, target, text);
            SendDirectMessage(&data);
        }
    }
    else
    {
        // announce to player that player he is whispering to is dnd and cannot receive his message
        ChatHandler(this).PSendSysMessage(LANG_PLAYER_DND, target->GetName().c_str(), target->dndMsg.c_str());
    }

    if(!IsAcceptWhispers() && !IsGameMaster() && !target->IsGameMaster())
    {
        SetAcceptWhispers(true);
        ChatHandler(this).SendSysMessage(LANG_COMMAND_WHISPERON);
    }

    // announce to player that player he is whispering to is afk
    if(target->IsAFK() && language != LANG_ADDON)
        ChatHandler(this).PSendSysMessage(LANG_PLAYER_AFK, target->GetName().c_str(), target->afkMsg.c_str());

    // if player whisper someone, auto turn of dnd to be able to receive an answer
    if(IsDND() && !target->IsGameMaster())
        ToggleDND();
}

void Player::PetSpellInitialize()
{
    Pet* pet = GetPet();
    if (!pet)
        return;

    // first line + actionbar + spellcount + spells + last adds
    WorldPacket data(SMSG_PET_SPELLS, 8+4+1+1+2+1+4*MAX_UNIT_ACTION_BAR_INDEX+1);

    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        TC_LOG_ERROR("entities.pet", "Pet has not charmInfo at PetSpellInitialize time, this should never happen!");
        return;
    }
                                                        //16
    data << (uint64)pet->GetGUID();
#ifdef LICH_KING
    data << uint16(pet->GetCreatureTemplate()->family);         // creature family (required for pet talents)
#endif
    data << uint32(pet->GetDuration());
    data << uint8(pet->GetReactState());
    data << uint8(charmInfo->GetCommandState());
    data << uint16(0); // Flags, mostly unknown

    charmInfo->BuildActionBar(&data);

    // spells count
    size_t spellsCountPos = data.wpos();
    uint8 addlist = 0;
    data << uint8(addlist);                             //placeholder

    if (pet->IsPermanentPetFor(this))
    {
        for (auto& m_spell : pet->m_spells)
        {
            if(m_spell.second.state == PETSPELL_REMOVED)
                continue;

            data << uint32(MAKE_UNIT_ACTION_BUTTON(m_spell.first, m_spell.second.active));
            ++addlist;
        }
    }
    data.put<uint8>(spellsCountPos, addlist);

    //Cooldowns
    pet->GetSpellHistory()->WritePacket<Pet>(data);

    SendDirectMessage(&data);
}

void Player::SendRemoveControlBar() const
{
    //LK OK
    WorldPacket data(SMSG_PET_SPELLS, 8);
    data << uint64(0);
    GetSession()->SendPacket(&data);
}

void Player::PossessSpellInitialize()
{
    Unit* charm = GetCharmed();

    if(!charm)
        return;

    CharmInfo *charmInfo = charm->GetCharmInfo();

    if(!charmInfo)
    {
        TC_LOG_ERROR("entities.player","Player::PossessSpellInitialize(): charm (%s) has no charminfo!", charm->GetGUID().ToString().c_str());
        return;
    }

    WorldPacket data(SMSG_PET_SPELLS, 8+4+1+1+2+1+4*MAX_UNIT_ACTION_BAR_INDEX+1+1);
                                                            //16
    data << (uint64)charm->GetGUID();
#ifdef LICH_KING
    data << uint16(0); //Family
#endif
    data << uint32(0); //duration
    data << uint8(0); //react state
    data << uint8(0); //command state
    data << uint16(0); //flags

    charmInfo->BuildActionBar(&data);

    data << uint8(0); // spells count
    data << uint8(0);                                       // cooldowns count
    //data << uint32(0x6010) << uint64(0);                    // for each cooldown (bc format, dunno about LK)

    SendDirectMessage(&data);
}

void Player::CharmSpellInitialize()
{
    Unit* charm = GetFirstControlled();
    if(!charm)
        return;

    CharmInfo *charmInfo = charm->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("entities.player","Player::CharmSpellInitialize(): the player's charm (%u) has no charminfo!", charm->GetGUID().GetCounter());
        return;
    }

    WorldPacket data(SMSG_PET_SPELLS, 8+4+1+1+2+1+4*MAX_UNIT_ACTION_BAR_INDEX+1);

    data << (uint64)charm->GetGUID();
#ifdef LICH_KING
    data << uint16(0); //pet family
#endif
    data << uint32(0); //duration

    if (charm->GetTypeId() != TYPEID_PLAYER)
        data << uint8((charm->ToCreature())->GetReactState()) << uint8(charmInfo->GetCommandState());
    else
        data << uint8(0) << uint8(0);

    data << uint16(0); //flags

    charmInfo->BuildActionBar(&data);

    size_t spellsCountPos = data.wpos();
    uint8 addlist = 0;
    data << uint8(addlist);                             //placeholder
    for(uint32 i = 0; i < MAX_SPELL_CHARM; ++i)
    {
        CharmSpellInfo* cspell = charmInfo->GetCharmSpell(i);
        if (cspell->GetAction())
        {
            data << uint32(cspell->packedData);
            addlist++;
        }
    }
    data.put<uint8>(spellsCountPos, addlist);

    data << uint8(0);                                       // cooldowns count
    //data << uint32(0x6010) << uint64(0);                    // for each cooldown (bc format, dunno about LK)

    SendDirectMessage(&data);
}

bool Player::IsAffectedBySpellmod(SpellInfo const *spellInfo, SpellModifier *mod, Spell const* spell)
{
    if (!mod || !spellInfo)
        return false;

    // First time this aura applies a mod to us and is out of charges
    if (spell && mod->ownerAura->IsUsingCharges() && !mod->ownerAura->GetCharges() && spell->m_appliedMods.find(mod->ownerAura) == spell->m_appliedMods.end())
        return false;

    // +duration to infinite duration spells making them limited
    if (mod->op == SPELLMOD_DURATION && spellInfo->GetDuration() == -1)
        return false;

    // mod crit to spells that can't crit
    if (mod->op == SPELLMOD_CRITICAL_CHANCE && !spellInfo->HasAttribute(SPELL_ATTR0_CU_CAN_CRIT))
        return false;

    return spellInfo->IsAffectedBySpellMod(mod);
}

template <class T>
void Player::ApplySpellMod(uint32 spellId, SpellModOp op, T &basevalue, Spell* spell)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return;

    float totalmul = 1.0f;
    int32 totalflat = 0;

    auto calculateSpellMod = [&](SpellModifier* mod)
    {
        switch (mod->type)
        {
        case SPELLMOD_FLAT:
            totalflat += mod->value;
            break;
        case SPELLMOD_PCT:
            // special case (skip > 10sec spell casts for instant cast setting)
            if (op == SPELLMOD_CASTING_TIME && mod->value <= -100 && basevalue >= T(10000))
                return;

            totalmul += CalculatePct(1.0f, mod->value);
            break;
        }

        Player::ApplyModToSpell(mod, spell);
    };

    // Drop charges for triggering spells instead of triggered ones
    if (m_spellModTakingSpell)
        spell = m_spellModTakingSpell;

    SpellModifier* chargedMod = nullptr;
    for (SpellModifier* mod : m_spellMods[op])
    {
        if (!IsAffectedBySpellmod(spellInfo, mod, spell))
            continue;

        if (mod->ownerAura->IsUsingCharges())
        {
            if (!chargedMod || (chargedMod->ownerAura->GetSpellInfo()->Priority < mod->ownerAura->GetSpellInfo()->Priority))
                chargedMod = mod;
            continue;
        }

        calculateSpellMod(mod);
    }

    if (chargedMod)
        calculateSpellMod(chargedMod);

    basevalue = T(float(basevalue + totalflat) * totalmul);
}

template TC_GAME_API void Player::ApplySpellMod(uint32 spellId, SpellModOp op, int32& basevalue, Spell* spell);
template TC_GAME_API void Player::ApplySpellMod(uint32 spellId, SpellModOp op, uint32& basevalue, Spell* spell);
template TC_GAME_API void Player::ApplySpellMod(uint32 spellId, SpellModOp op, float& basevalue, Spell* spell);

void Player::AddSpellMod(SpellModifier*& mod, bool apply)
{
    //TC_LOG_DEBUG("spells", "Player::AddSpellMod: Player '%s' (%s), SpellID: %d", GetName().c_str(), GetGUID().ToString().c_str(), mod->spellId);
    uint16 Opcode = (mod->type == SPELLMOD_FLAT) ? SMSG_SET_FLAT_SPELL_MODIFIER : SMSG_SET_PCT_SPELL_MODIFIER;

#ifdef LICH_KING
    flag96 modMask;
    for (uint8 i = 0; i < 3; ++i)
    {
        for (uint32 eff = 0; eff < 32; ++eff)
        {
            modMask[i] = uint32(1) << eff;
            if ((mod->mask & modMask))
            {
                int32 val = 0;
                for (SpellModifier* spellMod : m_spellMods[mod->op])
                {
                    if (spellMod->type == mod->type && (spellMod->mask & modMask))
                        val += spellMod->value;
                }
                val += apply ? mod->value : -(mod->value);
                WorldPacket data(Opcode, (1 + 1 + 4));
                data << uint8(eff + 32 * i);
                data << uint8(mod->op);
                data << int32(val);
                SendDirectMessage(&data);
            }
        }

        modMask[i] = 0;
    }

#else
    //almost same copde, only diff is that we're working with a uint64 modMask
    for (uint32 eff = 0; eff < 64; ++eff)
    {
        uint64 modMask = uint64(1) << eff;
        if ((mod->mask & modMask))
        {
            int32 val = 0;
            for (SpellModifier* spellMod : m_spellMods[mod->op])
            {
                if (spellMod->type == mod->type && (spellMod->mask & modMask))
                    val += spellMod->value;
            }
            val += apply ? mod->value : -(mod->value);
            WorldPacket data(Opcode, (1 + 1 + 4));
            data << uint8(eff);
            data << uint8(mod->op);
            data << int32(val);
            SendDirectMessage(&data);
        }
    }
#endif
 
    if (apply)
        m_spellMods[mod->op].insert(mod);
    else
        m_spellMods[mod->op].erase(mod);
}

void Player::ApplyModToSpell(SpellModifier* mod, Spell* spell)
{
    if (!spell)
        return;

    // don't do anything with no charges
    if (mod->ownerAura->IsUsingCharges() && !mod->ownerAura->GetCharges())
        return;

    // register inside spell, proc system uses this to drop charges
    spell->m_appliedMods.insert(mod->ownerAura);
}

void Player::SetSpellModTakingSpell(Spell* spell, bool apply)
{
    if (apply && m_spellModTakingSpell != nullptr)
        return;

    if (!apply && (!m_spellModTakingSpell || m_spellModTakingSpell != spell))
        return;

    m_spellModTakingSpell = apply ? spell : nullptr;
}

// send Proficiency
void Player::SendProficiency(uint8 pr1, uint32 pr2)
{
    WorldPacket data(SMSG_SET_PROFICIENCY, 8);
    data << uint8(pr1) << uint32(pr2);
    GetSession()->SendPacket (&data);
}

void Player::RemovePetitionsAndSigns(SQLTransaction trans, ObjectGuid guid, CharterTypes type)
{
    sPetitionMgr->RemoveSignaturesBySignerAndType(trans, guid, type);
    sPetitionMgr->RemovePetitionsByOwnerAndType(trans, guid, type);
}

void Player::SetRestBonus (float rest_bonus_new)
{
    // Prevent resting on max level
    if(GetLevel() >= sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        rest_bonus_new = 0;

    if(rest_bonus_new < 0)
        rest_bonus_new = 0;

    float rest_bonus_max = (float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP)*1.5/2;
    if(rest_bonus_new > rest_bonus_max)
        m_rest_bonus = rest_bonus_max;
    else
        m_rest_bonus = rest_bonus_new;

    // update data for client
    if(m_rest_bonus>10)
        SetByteValue(PLAYER_BYTES_2, 3, 0x01);              // Set Reststate = Rested
    else if(m_rest_bonus<=1)
        SetByteValue(PLAYER_BYTES_2, 3, 0x02);              // Set Reststate = Normal

    //RestTickUpdate
    SetUInt32Value(PLAYER_REST_STATE_EXPERIENCE, uint32(m_rest_bonus));
}


bool Player::ActivateTaxiPathTo(std::vector<uint32> const& nodes, Creature* npc /*= nullptr*/, uint32 spellid /*= 0*/)
{
    if (nodes.size() < 2)
        return false;

    // not let cheating with start flight in time of logout process || while in combat || has type state: stunned || has type state: root
    if (GetSession()->isLogingOut() || IsInCombat() || HasUnitState(UNIT_STATE_STUNNED) || HasUnitState(UNIT_STATE_ROOT))
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERBUSY);
        return false;
    }

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return false;

    // taximaster case
    if (npc)
    {
        // not let cheating with start flight mounted
        if (IsMounted())
        {
            GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERALREADYMOUNTED);
            return false;
        }

        if (IsInDisallowedMountForm())
        {
            GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERSHAPESHIFTED);
            return false;
        }

        // not let cheating with start flight in time of logout process || if casting not finished || while in combat || if not use Spell's with EffectSendTaxi
        if (IsNonMeleeSpellCast(false))
        {
            GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERBUSY);
            return false;
        }
    }
    // cast case or scripted call case
    else
    {
        RemoveAurasByType(SPELL_AURA_MOUNTED);

        if (IsInDisallowedMountForm())
            RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        if (Spell* spell = GetCurrentSpell(CURRENT_GENERIC_SPELL))
            if (spell->m_spellInfo->Id != spellid)
                InterruptSpell(CURRENT_GENERIC_SPELL, false);

        InterruptSpell(CURRENT_AUTOREPEAT_SPELL, false);

        if (Spell* spell = GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            if (spell->m_spellInfo->Id != spellid)
                InterruptSpell(CURRENT_CHANNELED_SPELL, true);
    }

    uint32 sourcenode = nodes[0];

    // starting node too far away (cheat?)
    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(sourcenode);
    if (!node)
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXINOSUCHPATH);
        return false;
    }

    // Prepare to flight start now

    // stop combat at start taxi flight if any
    CombatStop();

    StopCastingCharm();
    StopCastingBindSight();

    // stop trade (client cancel trade at taxi map open but cheating tools can be used for reopen it)
    TradeCancel(true);

    // clean not finished taxi path if any
    m_taxi.ClearTaxiDestinations();

    // 0 element current node
    m_taxi.AddTaxiDestination(sourcenode);

    // fill destinations path tail
    uint32 sourcepath = 0;
    uint32 totalcost = 0;
    uint32 firstcost = 0;

    uint32 prevnode = sourcenode;
    uint32 lastnode = 0;

    for (uint32 i = 1; i < nodes.size(); ++i)
    {
        uint32 path, cost;

        lastnode = nodes[i];
        sObjectMgr->GetTaxiPath(prevnode, lastnode, path, cost);

        if (!path)
        {
            m_taxi.ClearTaxiDestinations();
            return false;
        }

        totalcost += cost;
        if (i == 1)
            firstcost = cost;

        if (prevnode == sourcenode)
            sourcepath = path;

        m_taxi.AddTaxiDestination(lastnode);

        prevnode = lastnode;
    }

    // get mount model (in case non taximaster (npc == NULL) allow more wide lookup)
    //
    // Hack-Fix for Alliance not being able to use Acherus taxi. There is
    // only one mount ID for both sides. Probably not good to use 315 in case DBC nodes
    // change but I couldn't find a suitable alternative. OK to use class because only DK
    // can use this taxi.
    uint32 mount_display_id = sObjectMgr->GetTaxiMountDisplayId(sourcenode, GetTeam(), npc == nullptr);

    //HACKS
    switch(spellid)
    {
        case 31606:       //Stormcrow Amulet
            AreaExploredOrEventHappens(9718);
            mount_display_id = 17447;
            break;
        case 45071:      //Quest - Sunwell Daily - Dead Scar Bombing Run
        case 45113:      //Quest - Sunwell Daily - Ship Bombing Run
        case 45353:      //Quest - Sunwell Daily - Ship Bombing Run Return
            mount_display_id = 22840;
            break;
        case 34905:      //Stealth Flight
            mount_display_id = 6851;
            break;
        case 41533:      //Fly of the Netherwing
        case 41540:      //Fly of the Netherwing
            mount_display_id = 23468;
            break;
    }

    // in spell case allow 0 model
    if ((mount_display_id == 0 && spellid == 0) || sourcepath == 0)
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXIUNSPECIFIEDSERVERERROR);
        m_taxi.ClearTaxiDestinations();
        return false;
    }

    uint32 money = GetMoney();

     if (npc)
    {
        float discount = GetReputationPriceDiscount(npc);
        totalcost = uint32(ceil(totalcost * discount));
        firstcost = uint32(ceil(firstcost * discount));
        m_taxi.SetFlightMasterFactionTemplateId(npc->GetFaction());
    }
    else
        m_taxi.SetFlightMasterFactionTemplateId(0);

    if (money < totalcost)
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXINOTENOUGHMONEY);
        m_taxi.ClearTaxiDestinations();
        return false;
    }

    //Checks and preparations done, DO FLIGHT
#ifdef LICH_KING
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN, 1);
#endif

    // prevent stealth flight
    //RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);

    if (sWorld->getBoolConfig(CONFIG_INSTANT_TAXI))
    {
        TaxiNodesEntry const* lastPathNode = sTaxiNodesStore.LookupEntry(nodes[nodes.size() - 1]);
        ASSERT(lastPathNode);
        m_taxi.ClearTaxiDestinations();
        ModifyMoney(-(int32)totalcost);
#ifdef LICH_KING
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING, totalcost);
#endif
        TeleportTo(lastPathNode->map_id, lastPathNode->x, lastPathNode->y, lastPathNode->z, GetOrientation());
        return false;
    }
    else
    {
        ModifyMoney(-(int32)firstcost);
#ifdef LICH_KING
        UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING, firstcost);
#endif
        GetSession()->SendActivateTaxiReply(ERR_TAXIOK);
        GetSession()->SendDoFlight(mount_display_id, sourcepath);
    }

    return true;
}

bool Player::ActivateTaxiPathTo(uint32 taxi_path_id, uint32 spellid /*= 0*/)
{
    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(taxi_path_id);
    if (!entry)
        return false;

    std::vector<uint32> nodes;

    nodes.resize(2);
    nodes[0] = entry->from;
    nodes[1] = entry->to;

    return ActivateTaxiPathTo(nodes, nullptr, spellid);
}

void Player::FinishTaxiFlight()
{
    if (!IsInFlight())
        return;

    GetMotionMaster()->Remove(FLIGHT_MOTION_TYPE);
    m_taxi.ClearTaxiDestinations(); // not destinations, clear source node
}

void Player::CleanupAfterTaxiFlight()
{
    m_taxi.ClearTaxiDestinations(); // not destinations, clear source node
    Dismount();
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_TAXI_FLIGHT);
}

void Player::ContinueTaxiFlight()
{
    uint32 sourceNode = m_taxi.GetTaxiSource();
    if (!sourceNode)
        return;

    TC_LOG_DEBUG("entities.unit", "WORLD: Restart character %u taxi flight", GetGUID().GetCounter());

    uint32 mountDisplayId = sObjectMgr->GetTaxiMountDisplayId(sourceNode, GetTeam(), true);
    if (!mountDisplayId)
        return;

    uint32 path = m_taxi.GetCurrentTaxiPath();

    // search appropriate start path node
    uint32 startNode = 0;

    TaxiPathNodeList const& nodeList = sTaxiPathNodesByPath[path];

    float distPrev = MAP_SIZE*MAP_SIZE;
    float distNext = GetExactDistSq(nodeList[0]->LocX, nodeList[0]->LocY, nodeList[0]->LocZ);

    for (uint32 i = 1; i < nodeList.size(); ++i)
    {
        TaxiPathNodeEntry const* node = nodeList[i];
        TaxiPathNodeEntry const* prevNode = nodeList[i - 1];

        // skip nodes at another map
        if (node->MapID != GetMapId())
            continue;

        distPrev = distNext;

        distNext = GetExactDistSq(node->LocX, node->LocY, node->LocZ);

        float distNodes =
            (node->LocX - prevNode->LocX)*(node->LocX - prevNode->LocX) +
            (node->LocY - prevNode->LocY)*(node->LocY - prevNode->LocY) +
            (node->LocZ - prevNode->LocZ)*(node->LocZ - prevNode->LocZ);

        if (distNext + distPrev < distNodes)
        {
            startNode = i;
            break;
        }
    }

    GetSession()->SendDoFlight(mountDisplayId, path, startNode);
}

void Player::InitDataForForm(bool reapplyMods)
{
    SpellShapeshiftEntry const* ssEntry = sSpellShapeshiftStore.LookupEntry(GetShapeshiftForm());
    if(ssEntry && ssEntry->attackSpeed)
    {
        SetAttackTime(BASE_ATTACK,ssEntry->attackSpeed);
        SetAttackTime(OFF_ATTACK,ssEntry->attackSpeed);
        SetAttackTime(RANGED_ATTACK, BASE_ATTACK_TIME);
    }
    else
        SetRegularAttackTime();

    UpdateDisplayPower();

    // update auras at form change, ignore this at mods reapply (.reset stats/etc) when form not change.
    if (!reapplyMods)
        UpdateEquipSpellsAtFormChange();

    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);
}

// Return true is the bought item has a max count to force refresh of window by caller
bool Player::BuyItemFromVendorSlot(ObjectGuid vendorguid, uint32 /*vendorslot*/, uint32 item, uint8 count, uint8 bag, uint8 slot)
{
    // cheating attempt
    if(count < 1) count = 1;

    // cheating attempt
    if (slot > MAX_BAG_SIZE && slot != NULL_SLOT)
        return false;

    if(!IsAlive())
        return false;

    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate( item );
    if( !pProto )
    {
        SendBuyError( BUY_ERR_CANT_FIND_ITEM, nullptr, item, 0);
        return false;
    }

    if (!(pProto->AllowableClass & GetClassMask()) && pProto->Bonding == BIND_WHEN_PICKED_UP && !IsGameMaster())
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, nullptr, item, 0);
        return false;
    }

    Creature *pCreature = GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
    if (!pCreature)
    {
        TC_LOG_DEBUG("network", "WORLD: BuyItemFromVendorSlot - %u not found or you can't interact with him.", vendorguid.GetCounter());
        SendBuyError( BUY_ERR_DISTANCE_TOO_FAR, nullptr, item, 0);
        return false;
    }

    if (!sConditionMgr->IsObjectMeetingVendorItemConditions(pCreature->GetEntry(), item, this, pCreature))
    {
        TC_LOG_DEBUG("condition", "BuyItemFromVendorSlot: conditions not met for creature entry %u item %u", pCreature->GetEntry(), item);
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    VendorItemData const* vItems = pCreature->GetVendorItems();
    if(!vItems || vItems->Empty())
    {
        SendBuyError( BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    size_t vendor_slot = vItems->FindItemSlot(item);
    if(vendor_slot >= vItems->GetItemCount())
    {
        SendBuyError( BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    VendorItem const* crItem = vItems->m_items[vendor_slot];

    // check current item amount if it limited
    if( crItem->maxcount != 0 )
    {
        if(pCreature->GetVendorItemCurrentCount(crItem) < pProto->BuyCount * count )
        {
            SendBuyError( BUY_ERR_ITEM_ALREADY_SOLD, pCreature, item, 0);
            return false;
        }
    }

    if( uint32(GetReputationRank(pProto->RequiredReputationFaction)) < pProto->RequiredReputationRank)
    {
        SendBuyError( BUY_ERR_REPUTATION_REQUIRE, pCreature, item, 0);
        return false;
    }

    if(crItem->ExtendedCost)
    {
        ItemExtendedCostEntry const* iece = sObjectMgr->GetItemExtendedCost(crItem->ExtendedCost);
        if(!iece)
        {
            TC_LOG_ERROR("entities.player","Item %u have wrong ExtendedCost field value %u", pProto->ItemId, crItem->ExtendedCost);
            return false;
        }

        // honor points price
        if(GetHonorPoints() < (iece->reqhonorpoints * count))
        {
            SendEquipError(EQUIP_ERR_NOT_ENOUGH_HONOR_POINTS, nullptr, nullptr);
            return false;
        }

        // arena points price
        if(GetArenaPoints() < (iece->reqarenapoints * count))
        {
            SendEquipError(EQUIP_ERR_NOT_ENOUGH_ARENA_POINTS, nullptr, nullptr);
            return false;
        }

        // item base price
        for (uint8 i = 0; i < 5; ++i)
        {
            if(iece->reqitem[i] && !HasItemCount(iece->reqitem[i], (iece->reqitemcount[i] * count)))
            {
                SendEquipError(EQUIP_ERR_VENDOR_MISSING_TURNINS, nullptr, nullptr);
                return false;
            }
        }

        // check for personal arena rating requirement
        if( GetMaxPersonalArenaRatingRequirement() < iece->reqpersonalarenarating )
        {
            // probably not the proper equip err
            SendEquipError(EQUIP_ERR_CANT_EQUIP_RANK,nullptr,nullptr);
            return false;
        }
    }

    uint32 price  = pProto->BuyPrice * count;

    // reputation discount
    price = uint32(floor(price * GetReputationPriceDiscount(pCreature)));

    if( GetMoney() < price )
    {
        SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, item, 0);
        return false;
    }

    if( IsInventoryPos( bag, slot ) || (bag == NULL_BAG && slot == NULL_SLOT) )
    {
        ItemPosCountVec dest;
        uint8 msg = CanStoreNewItem( bag, slot, dest, item, pProto->BuyCount * count, nullptr, pProto );
        if( msg != EQUIP_ERR_OK )
        {
            SendEquipError( msg, nullptr, nullptr );
            return false;
        }

        ModifyMoney( -(int32)price );
        if(crItem->ExtendedCost)                            // case for new honor system
        {
            ItemExtendedCostEntry const* iece = sObjectMgr->GetItemExtendedCost(crItem->ExtendedCost);
            if(iece->reqhonorpoints)
                ModifyHonorPoints( - int32(iece->reqhonorpoints * count));
            if(iece->reqarenapoints)
                ModifyArenaPoints( - int32(iece->reqarenapoints * count));
            for (uint8 i = 0; i < 5; ++i)
            {
                if(iece->reqitem[i])
                    DestroyItemCount(iece->reqitem[i], (iece->reqitemcount[i] * count), true);
            }
        }

        if(Item *it = StoreNewItem( dest, item, true, 0))
        {
            uint32 new_count = pCreature->UpdateVendorItemCurrentCount(crItem,pProto->BuyCount * count);

            WorldPacket data(SMSG_BUY_ITEM, (8+4+4+4));
            data << pCreature->GetGUID();
            data << (uint32)(vendor_slot+1);                // numbered from 1 at client
            data << (uint32)(crItem->maxcount > 0 ? new_count : 0xFFFFFFFF);
            data << (uint32)count;
            SendDirectMessage(&data);

            SendNewItem(it, pProto->BuyCount*count, true, false, false);

            LogsDatabaseAccessor::BuyOrSellItemToVendor(LogsDatabaseAccessor::TRANSACTION_BUY, this, it, pCreature);
        }
    }
    else if( IsEquipmentPos( bag, slot ) )
    {
        if(pProto->BuyCount * count != 1)
        {
            SendEquipError( EQUIP_ERR_ITEM_CANT_BE_EQUIPPED, nullptr, nullptr );
            return false;
        }

        uint16 dest;
        uint8 msg = CanEquipNewItem(slot, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError( msg, nullptr, nullptr );
            return false;
        }

        ModifyMoney(-(int32)price);
        if(crItem->ExtendedCost)                            // case for new honor system
        {
            ItemExtendedCostEntry const* iece = sObjectMgr->GetItemExtendedCost(crItem->ExtendedCost);
            if(iece->reqhonorpoints)
                ModifyHonorPoints(-int32(iece->reqhonorpoints));
            if(iece->reqarenapoints)
                ModifyArenaPoints(-int32(iece->reqarenapoints));
            for (uint8 i = 0; i < 5; ++i)
            {
                if(iece->reqitem[i])
                    DestroyItemCount(iece->reqitem[i], iece->reqitemcount[i], true);
            }
        }

        if(Item *it = EquipNewItem( dest, item, true ))
        {
            uint32 new_count = pCreature->UpdateVendorItemCurrentCount(crItem,pProto->BuyCount * count);

            WorldPacket data(SMSG_BUY_ITEM, (8+4+4+4));
            data << pCreature->GetGUID();
            data << (uint32)(vendor_slot+1);                // numbered from 1 at client
            data << (uint32)(crItem->maxcount > 0 ? new_count : 0xFFFFFFFF);
            data << (uint32)count;
            SendDirectMessage(&data);

            SendNewItem(it, pProto->BuyCount*count, true, false, false);

            LogsDatabaseAccessor::BuyOrSellItemToVendor(LogsDatabaseAccessor::TRANSACTION_BUY, this, it, pCreature);

            AutoUnequipOffhandIfNeed();
        }
    }
    else
    {
        SendEquipError( EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT, nullptr, nullptr );
        return false;
    }

    return crItem->maxcount!=0;
}

uint32 Player::GetMaxPersonalArenaRatingRequirement()
{
    // returns the maximal personal arena rating that can be used to purchase items requiring this condition
    // the personal rating of the arena team must match the required limit as well
    // so return max[in arenateams](min(personalrating[teamtype], teamrating[teamtype]))
    uint32 max_personal_rating = 0;
    for(int i = 0; i < MAX_ARENA_SLOT; ++i)
    {
        if(ArenaTeam * at = sArenaTeamMgr->GetArenaTeamById(GetArenaTeamId(i)))
        {
            uint32 p_rating = GetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (i * 6) + 5);
            uint32 t_rating = at->GetRating();
            p_rating = p_rating<t_rating? p_rating : t_rating;
            if(max_personal_rating < p_rating)
                max_personal_rating = p_rating;
        }
    }
    return max_personal_rating;
}

void Player::UpdateHomebindTime(uint32 time)
{
    // GMs never get homebind timer online
    if (m_InstanceValid || IsGameMaster())
    {
        if(m_HomebindTimer)                                 // instance valid, but timer not reset
        {
            // hide reminder
            WorldPacket data(SMSG_RAID_GROUP_ONLY, 4+4);
            data << uint32(0);
            data << uint32(0);
            SendDirectMessage(&data);
        }
        // instance is valid, reset homebind timer
        m_HomebindTimer = 0;
    }
    else if (m_HomebindTimer > 0)
    {
        if (time >= m_HomebindTimer)
        {
            // teleport to homebind location
            TeleportTo(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ, GetOrientation());
        }
        else
            m_HomebindTimer -= time;
    }
    else
    {
        // instance is invalid, start homebind timer
        m_HomebindTimer = 60000;
        // send message to player
        WorldPacket data(SMSG_RAID_GROUP_ONLY, 4+4);
        data << m_HomebindTimer;
        data << uint32(1);
        SendDirectMessage(&data);
    }
}

void Player::UpdatePvP(bool state, bool ovrride)
{
    if(!state || ovrride)
    {
        SetPvP(state);
        for (ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
            (*itr)->SetPvP(state);

        pvpInfo.endTimer = 0;
    }
    else
    {
        if(pvpInfo.endTimer != 0)
            pvpInfo.endTimer = WorldGameTime::GetGameTime();
        else
        {
            SetPvP(state);
            for (ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
                (*itr)->SetPvP(state);
        }
    }
}

//slot to be excluded while counting
bool Player::EnchantmentFitsRequirements(uint32 enchantmentcondition, int8 slot)
{
    if(!enchantmentcondition)
        return true;

    SpellItemEnchantmentConditionEntry const *Condition = sSpellItemEnchantmentConditionStore.LookupEntry(enchantmentcondition);

    if(!Condition)
        return true;

    uint8 curcount[4] = {0, 0, 0, 0};

    //counting current equipped gem colors
    for(uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if(i == slot)
            continue;
        Item *pItem2 = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
        if(pItem2 && pItem2->GetTemplate()->Socket[0].Color)
        {
            for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
            {
                uint32 enchant_id = pItem2->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if(!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if(!enchantEntry)
                    continue;

                uint32 gemid = enchantEntry->GemID;
                if(!gemid)
                    continue;

                ItemTemplate const* gemProto = sObjectMgr->GetItemTemplate(gemid);
                if(!gemProto)
                    continue;

                GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
                if(!gemProperty)
                    continue;

                uint8 GemColor = gemProperty->color;

                for(uint8 b = 0, tmpcolormask = 1; b < 4; b++, tmpcolormask <<= 1)
                {
                    if(tmpcolormask & GemColor)
                        ++curcount[b];
                }
            }
        }
    }

    bool activate = true;

    for(int i = 0; i < 5; i++)
    {
        if(!Condition->Color[i])
            continue;

        uint32 _cur_gem = curcount[Condition->Color[i] - 1];

        // if have <CompareColor> use them as count, else use <value> from Condition
        uint32 _cmp_gem = Condition->CompareColor[i] ? curcount[Condition->CompareColor[i] - 1]: Condition->Value[i];

        switch(Condition->Comparator[i])
        {
            case 2:                                         // requires less <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem < _cmp_gem) ? true : false;
                break;
            case 3:                                         // requires more <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem > _cmp_gem) ? true : false;
                break;
            case 5:                                         // requires at least <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem >= _cmp_gem) ? true : false;
                break;
        }
    }

    return activate;
}

void Player::CorrectMetaGemEnchants(uint8 exceptslot, bool apply)
{
                                                            //cycle all equipped items
    for(uint32 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        //enchants for the slot being socketed are handled by Player::ApplyItemMods
        if(slot == exceptslot)
            continue;

        Item* pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, slot );

        if(!pItem || !pItem->GetTemplate()->Socket[0].Color)
            continue;

        for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
        {
            uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if(!enchant_id)
                continue;

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if(!enchantEntry)
                continue;

            uint32 condition = enchantEntry->EnchantmentCondition;
            if(condition)
            {
                                                            //was enchant active with/without item?
                bool wasactive = EnchantmentFitsRequirements(condition, apply ? exceptslot : -1);
                                                            //should it now be?
                if(wasactive ^ EnchantmentFitsRequirements(condition, apply ? -1 : exceptslot))
                {
                    // ignore item gem conditions
                                                            //if state changed, (dis)apply enchant
                    ApplyEnchantment(pItem,EnchantmentSlot(enchant_slot),!wasactive,true,true);
                }
            }
        }
    }
}

                                                            //if false -> then toggled off if was on| if true -> toggled on if was off AND meets requirements
void Player::ToggleMetaGemsActive(uint8 exceptslot, bool apply)
{
    //cycle all equipped items
    for(int slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        //enchants for the slot being socketed are handled by WorldSession::HandleSocketOpcode(WorldPacket& recvData)
        if(slot == exceptslot)
            continue;

        Item *pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, slot );

        if(!pItem || !pItem->GetTemplate()->Socket[0].Color)   //if item has no sockets or no item is equipped go to next item
            continue;

        //cycle all (gem)enchants
        for(uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
        {
            uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if(!enchant_id)                                 //if no enchant go to next enchant(slot)
                continue;

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if(!enchantEntry)
                continue;

            //only metagems to be (de)activated, so only enchants with condition
            uint32 condition = enchantEntry->EnchantmentCondition;
            if(condition)
                ApplyEnchantment(pItem,EnchantmentSlot(enchant_slot), apply);
        }
    }
}

void Player::LeaveBattleground(bool teleportToEntryPoint)
{
    if(Battleground *bg = GetBattleground())
    {
        if (bg->isSpectator(GetGUID()))
            return;

        bool need_debuf = bg->isBattleground() && !IsGameMaster() && ((bg->GetStatus() == STATUS_IN_PROGRESS) || (bg->GetStatus() == STATUS_WAIT_JOIN)) && sWorld->getConfig(CONFIG_BATTLEGROUND_CAST_DESERTER) && !sWorld->IsShuttingDown();

        if(bg->IsArena() && bg->isRated() && bg->GetStatus() == STATUS_WAIT_JOIN) //if game has not end then make sure that personal raiting is decreased
        {
            //decrease private rating here
            Team Loser = (Team)bg->GetPlayerTeam(GetGUID());
            Team Winner = Loser == ALLIANCE ? HORDE : ALLIANCE;
            ArenaTeam* WinnerTeam = sArenaTeamMgr->GetArenaTeamById(bg->GetArenaTeamIdForTeam(Winner));
            ArenaTeam* LoserTeam = sArenaTeamMgr->GetArenaTeamById(bg->GetArenaTeamIdForTeam(Loser));
            if (WinnerTeam && LoserTeam)
                LoserTeam->MemberLost(this, WinnerTeam->GetStats().Rating);
        }
        if (bg->GetTypeID() == BATTLEGROUND_WS) {
            RemoveAurasDueToSpell(46392);
            RemoveAurasDueToSpell(46393);
        }
        bg->RemovePlayerAtLeave(GetGUID(), teleportToEntryPoint, true);

        // call after remove to be sure that player resurrected for correct cast
        if(need_debuf)
        {
            //lets check if player was teleported from BG and schedule delayed Deserter spell cast
            if (IsBeingTeleportedFar())
            {
                ScheduleDelayedOperation(DELAYED_SPELL_CAST_DESERTER);
                return;
            }

            CastSpell(this, 26013, true);                   // Deserter
        }
    }
}

void Player::SetBattlegroundEntryPoint()
{
    // Taxi path store
    if (!m_taxi.empty())
    {
        m_bgData.mountSpell = 0;
        m_bgData.taxiPath[0] = m_taxi.GetTaxiSource();
        m_bgData.taxiPath[1] = m_taxi.GetTaxiDestination();

        // On taxi we don't need check for dungeon
        m_bgData.joinPos = WorldLocation(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    }
    else
    {
        m_bgData.ClearTaxiPath();

        // Mount spell id storing
        if (IsMounted())
        {
            AuraEffectList const& auras = GetAuraEffectsByType(SPELL_AURA_MOUNTED);
            if (!auras.empty())
                m_bgData.mountSpell = (*auras.begin())->GetId();
        }
        else
            m_bgData.mountSpell = 0;

        // If map is dungeon find linked graveyard
        if (GetMap()->IsDungeon())
        {
            if (WorldSafeLocsEntry const* entry = sObjectMgr->GetClosestGraveYard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetTeam()))
                m_bgData.joinPos = WorldLocation(entry->map_id, entry->x, entry->y, entry->z, 0.0f);
            else
                TC_LOG_ERROR("entities.player", "Player::SetBattlegroundEntryPoint: Dungeon (MapID: %u) has no linked graveyard, setting home location as entry point.", GetMapId());
        }
        // If new entry point is not BG or arena set it
        else if (!GetMap()->IsBattlegroundOrArena())
            m_bgData.joinPos = WorldLocation(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    }

    if (m_bgData.joinPos.m_mapId == MAPID_INVALID) // In error cases use homebind position
        m_bgData.joinPos = WorldLocation(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ, 0.0f);
}

bool Player::TeleportToBGEntryPoint()
{
      if (m_bgData.joinPos.m_mapId == MAPID_INVALID)
        return false;

    ScheduleDelayedOperation(DELAYED_BG_MOUNT_RESTORE);
    ScheduleDelayedOperation(DELAYED_BG_TAXI_RESTORE);
    ScheduleDelayedOperation(DELAYED_BG_GROUP_RESTORE);
    return TeleportTo(m_bgData.joinPos);
}

bool Player::CanJoinToBattleground(Battleground const* bg) const
{
    // check Deserter debuff
    if(GetDummyAura(26013))
        return false;

    /* TC
      if (bg->isArena() && !GetSession()->HasPermission(rbac::RBAC_PERM_JOIN_ARENAS))
        return false;

    if (bg->IsRandom() && !GetSession()->HasPermission(rbac::RBAC_PERM_JOIN_RANDOM_BG))
        return false;

    if (!GetSession()->HasPermission(rbac::RBAC_PERM_JOIN_NORMAL_BG))
        return false;
    */

    return true;
}

bool Player::CanReportAfkDueToLimit()
{
    // a player can complain about 15 people per 5 minutes
    if(m_bgAfkReportedCount >= 15)
        return false;
    ++m_bgAfkReportedCount;
    return true;
}

///This player has been blamed to be inactive in a battleground
void Player::ReportedAfkBy(Player* reporter)
{
    Battleground *bg = GetBattleground();
    if(!bg || bg != reporter->GetBattleground() || GetTeam() != reporter->GetTeam() || bg->GetStatus() != STATUS_IN_PROGRESS)
        return;

    // check if player has 'Idle' or 'Inactive' debuff
    if(m_bgData.bgAfkReporter.find(reporter->GetGUID().GetCounter()) == m_bgData.bgAfkReporter.end() && !HasAuraEffect(SPELL_AURA_PLAYER_IDLE, 0) && !HasAuraEffect(SPELL_AURA_PLAYER_INACTIVE,0) && reporter->CanReportAfkDueToLimit())
    {
        m_bgData.bgAfkReporter.insert(reporter->GetGUID().GetCounter());
        // 3 players have to complain to apply debuff
        if(m_bgData.bgAfkReporter.size() >= 3)
        {
            // cast 'Idle' spell
            CastSpell(this, SPELL_AURA_PLAYER_IDLE, true);
            m_bgData.bgAfkReporter.clear();
        }
    }
}

bool Player::IsNeverVisible() const
{
    if (Unit::IsNeverVisible())
        return true;

    if (GetSession()->PlayerLogout() || GetSession()->PlayerLoading())
        return true;

    return false;
}

bool Player::CanNeverSee(WorldObject const* obj) const
{
    if (obj->ToUnit())
    {
        //can never see opposite playters in arena while in prep
        Player const* targetPlayer = obj->ToUnit()->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (targetPlayer && targetPlayer->InArena() && !IsInSameRaidWith(targetPlayer) && targetPlayer->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION))
            return true;
    }

    return WorldObject::CanNeverSee(obj);
}

bool Player::CanAlwaysSee(WorldObject const* obj) const
{
    // Always can see self
    if (GetSession()->GetClientControl().GetAllowedActiveMover() == obj)
        return true;

    if (ObjectGuid guid = GetGuidValue(PLAYER_FARSIGHT))
        if (obj->GetGUID() == guid)
            return true;

    return false;
}

bool Player::IsAlwaysDetectableFor(WorldObject const* seer) const
{
    if (Unit::IsAlwaysDetectableFor(seer))
        return true;

    if (duel && duel->State != DUEL_STATE_CHALLENGED && duel->Opponent == seer)
        return false;

    if (const Player* seerPlayer = seer->ToPlayer())
        if (IsGroupVisibleFor(seerPlayer))
            return true;

    return false;
}

bool Player::IsVisibleGloballyFor( Player* u ) const
{
    if(!u)
        return false;

    // Always can see self
    if (u==this)
        return true;

    // Visible units, always are visible for all players
    if (IsVisible())
        return true;

    // -- Only gm's from this point, custom sunstrider rules

    //Rank >=2 GMs can always see everyone
    if (u->GetSession()->GetSecurity() >= SEC_GAMEMASTER2)
       return true;

    //Rank1 GM can see everyone except higher GMs
    if (u->GetSession()->GetSecurity() == SEC_GAMEMASTER1 && GetSession()->GetSecurity() <= SEC_GAMEMASTER1)
       return true;

    //But GM's can still see others GM's if in same group
    if(u->GetSession()->GetSecurity() >= SEC_GAMEMASTER1 && IsInSameGroupWith(u))
        return true;

    return false;
}

void Player::UpdateTriggerVisibility()
{
    if (m_clientGUIDs.empty())
        return;

    if (!IsInWorld())
        return;

    UpdateData udata;
    WorldPacket packet;
    for (auto itr = m_clientGUIDs.begin(); itr != m_clientGUIDs.end(); ++itr)
    {
        if (itr->IsCreatureOrVehicle())
        {
            Creature* creature = GetMap()->GetCreature(*itr);
            // Update fields of triggers, transformed units or unselectable units (values dependent on GM state)
            if (!creature || (!creature->IsTrigger() && !creature->HasAuraType(SPELL_AURA_TRANSFORM) && !creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
                continue;

            creature->SetFieldNotifyFlag(UF_FLAG_PUBLIC);
            creature->BuildValuesUpdateBlockForPlayer(&udata, this);
            creature->RemoveFieldNotifyFlag(UF_FLAG_PUBLIC);
        }
        else if (itr->IsGameObject())
        {
            GameObject* go = GetMap()->GetGameObject(*itr);
            if (!go)
                continue;

            go->SetFieldNotifyFlag(UF_FLAG_PUBLIC);
            go->BuildValuesUpdateBlockForPlayer(&udata, this);
            go->RemoveFieldNotifyFlag(UF_FLAG_PUBLIC);
        }
    }

    if (!udata.HasData())
        return;

    udata.BuildPacket(&packet, false);
    GetSession()->SendPacket(&packet);
}

void Player::SendInitialVisiblePackets(Unit* target)
{
    SendAuraDurationsForTarget(target);

    if (target->IsAlive())
    {
        if (target->HasUnitState(UNIT_STATE_MELEE_ATTACKING) && target->GetVictim())
            target->SendMeleeAttackStart(target->GetVictim());
    }
}

template<class T>
inline void BeforeVisibilityDestroy(T* /*t*/, Player* /*p*/) { }

template<>
inline void BeforeVisibilityDestroy<Creature>(Creature* t, Player* p)
{
    if (p->GetPetGUID() == t->GetGUID() && t->IsPet())
        t->ToPet()->Remove(PET_SAVE_NOT_IN_SLOT, true);
}

void Player::UpdateVisibilityOf(WorldObject* target)
{
    if(HaveAtClient(target))
    {
        if (!CanSeeOrDetect(target, false, true))
        {
            if (target->GetTypeId() == TYPEID_UNIT)
                BeforeVisibilityDestroy<Creature>(target->ToCreature(), this);

            target->DestroyForPlayer(this);
            m_clientGUIDs.erase(target->GetGUID());

            //TC_LOG_DEBUG("debug.grid","Object %u (Type: %u) out of range for player %u. Distance = %f",target->GetGUID().GetCounter(),target->GetTypeId(),GetGUID().GetCounter(),GetDistance(target));
        }
    }
    else
    {
        if (CanSeeOrDetect(target, false, true))
        {
            target->SendUpdateToPlayer(this);
            m_clientGUIDs.insert(target->GetGUID());

            //TC_LOG_DEBUG("debug.grid","Object %u (Type: %u) is visible now for player %u. Distance = %f",target->GetGUID().GetCounter(),target->GetTypeId(),GetGUID().GetCounter(),GetDistance(target));

            // target aura duration for caster show only if target exist at caster client
            // send data at target visibility change (adding to client)
            if(target->isType(TYPEMASK_UNIT))
                SendInitialVisiblePackets(static_cast<Unit*>(target));
        }
    }
}

template<class T>
inline void UpdateVisibilityOf_helper(GuidUnorderedSet& s64, T* target, std::set<Unit*>& /*v*/)
{
    s64.insert(target->GetGUID());
}

template<>
inline void UpdateVisibilityOf_helper(GuidUnorderedSet& s64, GameObject* target, std::set<Unit*>& /*v*/)
{
    if(!target->IsTransport())
        s64.insert(target->GetGUID());
}

template<>
inline void UpdateVisibilityOf_helper(GuidUnorderedSet& s64, Creature* target, std::set<Unit*>& v)
{
    s64.insert(target->GetGUID());
    v.insert(target);
}

template<>
inline void UpdateVisibilityOf_helper(GuidUnorderedSet& s64, Player* target, std::set<Unit*>& v)
{
    s64.insert(target->GetGUID());
    v.insert(target);
}


template<class T>
void Player::UpdateVisibilityOf(T* target, UpdateData& data, std::set<Unit*>& visibleNow)
{
    if(!target)
        return;

    if(HaveAtClient(target))
    {
        if (!CanSeeOrDetect(target, false, true))
        {
            BeforeVisibilityDestroy<T>(target, this);

            target->BuildOutOfRangeUpdateBlock(&data);
            m_clientGUIDs.erase(target->GetGUID());

            //TC_LOG_DEBUG("debug.grid","Object %u (Type: %u, Entry: %u) is out of range for player %u. Distance = %f",target->GetGUID().GetCounter(),target->GetTypeId(),target->GetEntry(),GetGUID().GetCounter(),GetDistance(target));
        }
    }
    else //if(visibleNow.size() < 30)
    {
        if (CanSeeOrDetect(target, false, true))
        {
            target->BuildCreateUpdateBlockForPlayer(&data, this);
            UpdateVisibilityOf_helper(m_clientGUIDs, target, visibleNow);

            //TC_LOG_DEBUG("debug.grid", "Object %u (Type: %u) is visible now for player %u. Distance = %f", target->GetGUID().GetCounter(), target->GetTypeId(), GetGUID().GetCounter(), GetDistance(target));
        }
    }
}

template void Player::UpdateVisibilityOf(Player*        target, UpdateData& data, std::set<Unit*>& visibleNow);
template void Player::UpdateVisibilityOf(Creature*      target, UpdateData& data, std::set<Unit*>& visibleNow);
template void Player::UpdateVisibilityOf(Corpse*        target, UpdateData& data, std::set<Unit*>& visibleNow);
template void Player::UpdateVisibilityOf(GameObject*    target, UpdateData& data, std::set<Unit*>& visibleNow);
template void Player::UpdateVisibilityOf(DynamicObject* target, UpdateData& data, std::set<Unit*>& visibleNow);

void Player::UpdateObjectVisibility(bool forced)
{
    if (!forced)
        AddToNotify(NOTIFY_VISIBILITY_CHANGED);
    else
    {
        Unit::UpdateObjectVisibility(true);
        UpdateVisibilityForPlayer();
    }
}

void Player::UpdateVisibilityForPlayer()
{
    // updates visibility of all objects around point of view for current player
    Trinity::VisibleNotifier notifier(*this);
    Cell::VisitAllObjects(m_seer, notifier, GetSightRange());
    notifier.SendToSelf();   // send gathered data
}

void Player::InitPrimaryProffesions()
{
    SetFreePrimaryProffesions(sWorld->getConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL));
}

bool Player::IsQuestRewarded(uint32 quest_id) const
{
    /* TC
    return m_RewardedQuests.find(quest_id) != m_RewardedQuests.end();
    */

    for (auto itr : m_QuestStatus)
    {
        if (itr.first != quest_id)
            continue;
        return itr.second.Rewarded;
    }

    return false;
}

Unit* Player::GetSelectedUnit() const
{
    if (ObjectGuid selectionGUID = GetGuidValue(UNIT_FIELD_TARGET))
        return ObjectAccessor::GetUnit(*this, selectionGUID);
    return nullptr;
}

Player* Player::GetSelectedPlayer() const
{
    if (ObjectGuid selectionGUID = GetGuidValue(UNIT_FIELD_TARGET))
        return ObjectAccessor::FindConnectedPlayer(selectionGUID);
    return nullptr;
}

void Player::AddComboPoints(Unit* target, int8 count, bool forceCurrent /* = false */) // forceCurrent: forces combo add on current combo target (fixes rogue's Setup)
{
    if (!count)
        return;

    // without combo points lost (duration checked in aura)
    RemoveAurasByType(SPELL_AURA_RETAIN_COMBO_POINTS);

    if (target->GetGUID() == m_comboTarget)
        m_comboPoints += count;
    else if (!forceCurrent || !m_comboTarget) { // Accept this only if not force current or no current combo target
        if (m_comboTarget) {
            if (Unit* _target = ObjectAccessor::GetUnit(*this, m_comboTarget))
                _target->RemoveComboPointHolder(GetGUID().GetCounter());
        }

        m_comboTarget = target->GetGUID();
        m_comboPoints = count;

        target->AddComboPointHolder(GetGUID().GetCounter());
    }

    if (m_comboPoints > 5) m_comboPoints = 5;
    if (m_comboPoints < 0) m_comboPoints = 0;

    SendComboPoints();
}

void Player::ClearComboPoints(uint32 spellId)
{
    if (!m_comboTarget)
        return;

    // without combopoints lost (duration checked in aura)
    RemoveAurasByType(SPELL_AURA_RETAIN_COMBO_POINTS);

    m_comboPoints = 0;

    SendComboPoints();

    if (Unit* target = ObjectAccessor::GetUnit(*this, m_comboTarget))
        target->RemoveComboPointHolder(GetGUID().GetCounter());

    m_comboTarget = ObjectGuid::Empty;

    //handle Ruthlessness - shouldn't proc on Deadly Throw
    if (spellId != 26679 && spellId != 48673) {
        if (HasSpell(14156) /*rank 1, 20%*/ || HasSpell(14160) /*Rank 2, 40%*/ || HasSpell(14161) /*Rank 3, 60% */)
        {
            uint32 procChance = urand(1, 100);
            if ((HasSpell(14161) && procChance <= 60) || (HasSpell(14160) && procChance <= 40) || (HasSpell(14156) && procChance <= 20))
            {
                if (this->GetVictim())
                    AddComboPoints(this->GetVictim(), 1);
            }
        }
    }
}

void Player::SendComboPoints()
{
    if (m_cleanupDone)
        return;

    Unit* combotarget = ObjectAccessor::GetUnit(*this, m_comboTarget);
    if (combotarget)
    {
        WorldPacket data;
        if (GetSession()->GetClientControl().GetActiveMover() != this)
            return; //no combo point from pet/charmed creatures

        data.Initialize(SMSG_UPDATE_COMBO_POINTS, combotarget->GetPackGUID().size() + 1);
        data << combotarget->GetPackGUID();
        data << uint8(m_comboPoints);
        SendDirectMessage(&data);
    }
}

void Player::SetGroup(Group *group, int8 subgroup)
{
    if(group == nullptr) m_group.unlink();
    else
    {
        // never use SetGroup without a subgroup unless you specify NULL for group
        assert(subgroup >= 0);
        m_group.link(group, this);
        m_group.setSubGroup((uint8)subgroup);
    }
}

void Player::SendInitialPacketsBeforeAddToMap()
{
    WorldPacket data(SMSG_SET_REST_START, 4);
    data << uint32(0);                                      // unknown, may be rest state time or experience
    SendDirectMessage(&data);

    // Homebind
    data.Initialize(SMSG_BINDPOINTUPDATE, 5*4);
    data << m_homebindX << m_homebindY << m_homebindZ;
    data << (uint32) m_homebindMapId;
    data << (uint32) m_homebindAreaId;
    SendDirectMessage(&data);

    // SMSG_SET_PROFICIENCY
    // SMSG_UPDATE_AURA_DURATION

#ifndef LICH_KING
    //LK send those at session opening
    GetSession()->SendTutorialsData();
#endif

    SendInitialSpells();

    data.Initialize(SMSG_SEND_UNLEARN_SPELLS, 4);
    data << uint32(0);                                      // count, for(count) uint32;
    SendDirectMessage(&data);

    SendInitialActionButtons();
    m_reputationMgr->SendInitialReputations();
    uint32 newzone, newarea;
    GetZoneAndAreaId(newzone, newarea);
    UpdateZone(newzone, newarea);

    // set fly flag if in fly form or taxi flight to prevent visually drop at ground in showup moment
    if(HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || IsInFlight())
        AddUnitMovementFlag(MOVEMENTFLAG_PLAYER_FLYING);

    GetSession()->GetClientControl().InitActiveMover(this);
}

void Player::SendInitialPacketsAfterAddToMap()
{
    UpdateVisibilityForPlayer();

    // update zone
    uint32 newzone, newarea;
    GetZoneAndAreaId(newzone, newarea);
    UpdateZone(newzone, newarea);                            // also call SendInitWorldStates();

    GetSession()->ResetTimeSync();
    GetSession()->SendTimeSync();

    CastSpell(this, 836, true);                             // LOGINEFFECT

    // set some aura effects that send packet to player client after add player to map
    // SendMessageToSet not send it to player not it map, only for aura that not changed anything at re-apply
    // same auras state lost at far teleport, send it one more time in this case also
    static AuraType constexpr auratypes[] =
    {
        SPELL_AURA_MOD_FEAR,     SPELL_AURA_TRANSFORM,                         SPELL_AURA_WATER_WALK,
        SPELL_AURA_FEATHER_FALL, SPELL_AURA_HOVER,                             SPELL_AURA_SAFE_FALL,
        SPELL_AURA_FLY,          SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED, SPELL_AURA_NONE
    };
    for(AuraType const* itr = &auratypes[0]; itr && itr[0] != SPELL_AURA_NONE; ++itr)
    {
        Unit::AuraEffectList const& auraList = GetAuraEffectsByType(*itr);
        if(!auraList.empty())
            auraList.front()->HandleEffect(this, AURA_EFFECT_HANDLE_SEND_FOR_CLIENT, true);
    }

    if(HasAuraType(SPELL_AURA_MOD_STUN))
        SetRooted(true);

    // manual send package (have code in HandleEffect(true,true); that don't must be re-applied.
    if(HasAuraType(SPELL_AURA_MOD_ROOT))
        SetRooted(true);

    //SendAurasForTarget(this);
    SendEnchantmentDurations();                             // must be after add to map
    SendItemDurations();                                    // must be after add to map

#ifdef LICH_KING
                                                            // raid downscaling - send difficulty to player
    if (GetMap()->IsRaid())
    {
        if (GetMap()->GetDifficulty() != GetRaidDifficulty())
        {
            StoreRaidMapDifficulty();
            SendRaidDifficulty(GetGroup() != NULL, GetStoredRaidDifficulty());
        }
    }
    else if (GetRaidDifficulty() != GetStoredRaidDifficulty())
        SendRaidDifficulty(GetGroup() != NULL);
#endif
}

void Player::SendSupercededSpell(uint32 oldSpell, uint32 newSpell) const
{
    //LK ok
    WorldPacket data(SMSG_SUPERCEDED_SPELL, 8);
    data << uint32(oldSpell) << uint32(newSpell);
    GetSession()->SendPacket(&data);
}

bool Player::ValidateAppearance(uint8 race, uint8 class_, uint8 gender, uint8 hairID, uint8 hairColor, uint8 faceID, uint8 facialHair, uint8 skinColor, bool create /*=false*/)
{
    auto validateCharSection = [class_, create](CharSectionsEntry const* entry) -> bool
    {
        if (!entry)
            return false;

#ifdef LICH_KING
        // Check Death Knight exclusive
        if (class_ != CLASS_DEATH_KNIGHT && entry->HasFlag(SECTION_FLAG_DEATH_KNIGHT))
            return false;

        // Character creation/customize has some limited sections (as opposed to barbershop)
        if (create && !entry->HasFlag(SECTION_FLAG_PLAYER))
            return false;
#endif

        return true;
    };

    // For Skin type is always 0
    CharSectionsEntry const* skinEntry = GetCharSectionEntry(race, SECTION_TYPE_SKIN, gender, 0, skinColor);
    if (!validateCharSection(skinEntry))
        return false;

    // Skin Color defined as Face color, too
    CharSectionsEntry const* faceEntry = GetCharSectionEntry(race, SECTION_TYPE_FACE, gender, faceID, skinColor);
    if (!validateCharSection(faceEntry))
        return false;

    // Check Hair
    CharSectionsEntry const* hairEntry = GetCharSectionEntry(race, SECTION_TYPE_HAIR, gender, hairID, hairColor);
    if (!validateCharSection(hairEntry))
        return false;

    // These combinations don't have an entry of Type SECTION_TYPE_FACIAL_HAIR, exclude them from that check
    bool const excludeCheck = (race == RACE_TAUREN) || (race == RACE_DRAENEI) || (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD_PLAYER);
    if (!excludeCheck)
    {
        CharSectionsEntry const* facialHairEntry = GetCharSectionEntry(race, SECTION_TYPE_FACIAL_HAIR, gender, facialHair, hairColor);
        if (!validateCharSection(facialHairEntry))
            return false;
    }

    CharacterFacialHairStylesEntry const* entry = GetCharFacialHairEntry(race, gender, facialHair);
    if (!entry)
        return false;

    return true;
}

void Player::SendUpdateToOutOfRangeGroupMembers()
{
    if (m_groupUpdateMask == GROUP_UPDATE_FLAG_NONE)
        return;
    if(Group* group = GetGroup())
        group->UpdatePlayerOutOfRange(this);

    m_groupUpdateMask = GROUP_UPDATE_FLAG_NONE;
    m_auraUpdateMask = 0;
    if(Pet *pet = GetPet())
        pet->ResetAuraUpdateMask();
}

void Player::SendTransferAborted(uint32 mapid, uint16 reason)
{
    WorldPacket data(SMSG_TRANSFER_ABORTED, 4 + 2);
    data << uint32(mapid);
#ifdef LICH_KING
    data << uint8(0x1 /* TRANSFER_ABORT_ERROR*/); //TODO: enum is offset on LK here, need to find a good way to convert it
#else
    data << uint16(reason);                                 // transfer abort reason
#endif
    SendDirectMessage(&data);
}

void Player::SendInstanceResetWarning(uint32 mapid, uint32 time)
{
    // type of warning, based on the time remaining until reset
    uint32 type;
    if(time > 3600)
        type = RAID_INSTANCE_WELCOME;
    else if(time > 900)
        type = RAID_INSTANCE_WARNING_HOURS;
    else if(time > 300)
        type = RAID_INSTANCE_WARNING_MIN;
    else
        type = RAID_INSTANCE_WARNING_MIN_SOON;
    WorldPacket data(SMSG_RAID_INSTANCE_MESSAGE, 4+4+4);
    data << uint32(type);
    data << uint32(mapid);
    data << uint32(time);
    SendDirectMessage(&data);
}

void Player::ApplyEquipCooldown( Item * pItem )
{
    if (pItem->GetTemplate()->Flags & ITEM_FLAG_NO_EQUIP_COOLDOWN)
        return;

    std::chrono::steady_clock::time_point now = WorldGameTime::GetGameTimeSteadyPoint();
    for(const auto & spellData : pItem->GetTemplate()->Spells)
    {
        // no spell
        if( !spellData.SpellId )
            continue;

        // apply proc cooldown to equip auras if we have any
        if (spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_EQUIP)
        {
            SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(spellData.SpellId);
            if (!procEntry)
                continue;

            if (Aura* itemAura = GetAura(spellData.SpellId, GetGUID(), pItem->GetGUID()))
                itemAura->AddProcCooldown(now + procEntry->Cooldown);
            continue;
        }

        // wrong triggering type (note: ITEM_SPELLTRIGGER_ON_NO_DELAY_USE not have cooldown)
        if( spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE )
            continue;

        GetSpellHistory()->AddCooldown(spellData.SpellId, pItem->GetEntry(), std::chrono::seconds(30));

        WorldPacket data(SMSG_ITEM_COOLDOWN, 12);
        data << pItem->GetGUID();
        data << uint32(spellData.SpellId);
        SendDirectMessage(&data);
    }
}

void Player::resetSpells()
{
    // not need after this call
    if(HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
    {
        m_atLoginFlags = m_atLoginFlags & ~AT_LOGIN_RESET_SPELLS;
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login & ~ %u WHERE guid ='%u'", uint32(AT_LOGIN_RESET_SPELLS), GetGUID().GetCounter());
    }

    // make full copy of map (spells removed and marked as deleted at another spell remove
    // and we can't use original map for safe iterative with visit each spell at loop end
    PlayerSpellMap smap = GetSpellMap();

    for(PlayerSpellMap::const_iterator iter = smap.begin();iter != smap.end(); ++iter)
        RemoveSpell(iter->first);                           // only iter->first can be accessed, object by iter->second can be deleted already

    LearnDefaultSkills();
    LearnDefaultSpells();
    learnQuestRewardedSpells();
}

void Player::LearnDefaultSkills()
{
    // learn default race/class skills
    PlayerInfo const* info = sObjectMgr->GetPlayerInfo(GetRace(), GetClass());
    ASSERT(info);
    for (PlayerCreateInfoSkills::const_iterator itr = info->skills.begin(); itr != info->skills.end(); ++itr)
    {
        uint32 skillId = itr->SkillId;
        if (HasSkill(skillId))
            continue;

        LearnDefaultSkill(skillId, itr->Rank);
    }
}

void Player::LearnDefaultSkill(uint32 skillId, uint16 rank)
{
    SkillRaceClassInfoEntry const* rcInfo = GetSkillRaceClassInfo(skillId, GetRace(), GetClass());
    if (!rcInfo)
        return;

    //TC_LOG_DEBUG("entities.player.loading", "PLAYER (Class: %u Race: %u): Adding initial skill, id = %u", uint32(GetClass()), uint32(GetRace()), skillId);

    switch (GetSkillRangeType(rcInfo))
    {
    case SKILL_RANGE_LANGUAGE:
        SetSkill(skillId, 0, 300, 300);
        break;
    case SKILL_RANGE_LEVEL:
    {
        uint16 skillValue = 1;
        uint16 maxValue = GetMaxSkillValueForLevel();
        if (rcInfo->Flags & SKILL_FLAG_ALWAYS_MAX_VALUE)
            skillValue = maxValue;
#ifdef LICH_KING
        else if (GetClass() == CLASS_DEATH_KNIGHT)
            skillValue = std::min(std::max<uint16>({ 1, uint16((GetLevel() - 1) * 5) }), maxValue);
#endif
        else if (skillId == SKILL_FIST_WEAPONS)
            skillValue = std::max<uint16>(1, GetSkillValue(SKILL_UNARMED));
        else if (skillId == SKILL_LOCKPICKING)
            skillValue = std::max<uint16>(1, GetSkillValue(SKILL_LOCKPICKING));

        SetSkill(skillId, 0, skillValue, maxValue);
    }
        break;
    case SKILL_RANGE_MONO:
        SetSkill(skillId, 0, 1, 1);
        break;
    case SKILL_RANGE_RANK:
    {
        if (!rank)
            break;

        SkillTiersEntry const* tier = sSkillTiersStore.LookupEntry(rcInfo->SkillTier);
        uint16 maxValue = tier->MaxSkill[std::max<int32>(rank - 1, 0)];
        uint16 skillValue = 1;
        if (rcInfo->Flags & SKILL_FLAG_ALWAYS_MAX_VALUE)
            skillValue = maxValue;
        else if (GetClass() == CLASS_DEATH_KNIGHT)
            skillValue = std::min(std::max<uint16>({ uint16(1), uint16((GetLevel() - 1) * 5) }), maxValue);

        TC_LOG_ERROR("entities.player", "Player::LearnDefaultSkill called with NYI SKILL_RANGE_RANK");
        SetSkill(skillId, rank, skillValue, maxValue);
    }
        break;
    default:
        break;
    }
}

void Player::LearnDefaultSpells(bool loading)
{
    // learn default race/class spells
    PlayerInfo const *info = sObjectMgr->GetPlayerInfo(GetRace(),GetClass());
    ASSERT(info);
    std::list<CreateSpellPair>::const_iterator spell_itr;
    for (spell_itr = info->spell.begin(); spell_itr!=info->spell.end(); ++spell_itr)
    {
        uint16 tspell = spell_itr->first;
        if (tspell)
        {
            if(loading || !spell_itr->second)               // not care about passive spells or loading case
                AddSpell(tspell,spell_itr->second);
            else                                            // but send in normal spell in game learn case
                LearnSpell(tspell, false);
        }
    }
}

void Player::learnQuestRewardedSpells(Quest const* quest)
{
    uint32 spell_id = quest->GetRewSpellCast();

    // skip quests without rewarded spell
    if( !spell_id )
        return;

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spell_id);
    if(!spellInfo)
        return;

    // check learned spells state
    bool found = false;
    for(const auto & Effect : spellInfo->Effects)
    {
        //skip spells with effect SPELL_EFFECT_TRADE_SKILL, these are skill spec and shouldn't be learned again when unlearned
        uint32 triggerSpell = Effect.TriggerSpell;
        if(SpellInfo const* _spellInfo = sSpellMgr->GetSpellInfo(triggerSpell))
            if(_spellInfo->Effects[0].Effect == SPELL_EFFECT_TRADE_SKILL)
                continue;

        if(Effect.Effect == SPELL_EFFECT_LEARN_SPELL && !HasSpell(Effect.TriggerSpell))
        {
            found = true;
            break;
        }
    }

    // skip quests with not teaching spell or already known spell
    if(!found)
        return;

    // prevent learn non first rank unknown profession and second specialization for same profession)
    uint32 learned_0 = spellInfo->Effects[0].TriggerSpell;
    if( sSpellMgr->GetSpellRank(learned_0) > 1 && !HasSpell(learned_0) )
    {
        // not have first rank learned (unlearned prof?)
        uint32 first_spell = sSpellMgr->GetFirstSpellInChain(learned_0);
        if( !HasSpell(first_spell) )
            return;

        SpellInfo const *learnedInfo = sSpellMgr->GetSpellInfo(learned_0);
        if(!learnedInfo)
            return;

        // specialization
        if(learnedInfo->Effects[0].Effect==SPELL_EFFECT_TRADE_SKILL && learnedInfo->Effects[1].Effect==0)
        {
            // search other specialization for same prof
            for(PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
            {
                if(itr->second->state == PLAYERSPELL_REMOVED || itr->first==learned_0)
                    continue;

                SpellInfo const *itrInfo = sSpellMgr->GetSpellInfo(itr->first);
                if(!itrInfo)
                    return;

                // compare only specializations
                if(itrInfo->Effects[0].Effect!=SPELL_EFFECT_TRADE_SKILL || itrInfo->Effects[1].Effect!=0)
                    continue;

                // compare same chain spells
                if(sSpellMgr->GetFirstSpellInChain(itr->first) != first_spell)
                    continue;

                // now we have 2 specialization, learn possible only if found is lesser specialization rank
                if(!sSpellMgr->IsHighRankOfSpell(learned_0,itr->first))
                    return;
            }
        }
    }

    CastSpell( this, spell_id, true);
}

void Player::learnQuestRewardedSpells()
{
    // learn spells received from quest completing
    for(QuestStatusMap::const_iterator itr = m_QuestStatus.begin(); itr != m_QuestStatus.end(); ++itr)
    {
        // skip no rewarded quests
        if(!itr->second.Rewarded)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(itr->first);
        if( !quest )
            continue;

        learnQuestRewardedSpells(quest);
    }
}

void Player::LearnSkillRewardedSpells(uint32 skillId, uint32 skillValue)
{
    uint32 raceMask  = GetRaceMask();
    uint32 classMask = GetClassMask();
    for (uint32 j=0; j<sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* ability = sSkillLineAbilityStore.LookupEntry(j);
        if (!ability || ability->skillId != skillId)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ability->spellId);
        if (!spellInfo)
            continue;

        if (ability->AutolearnType != SKILL_LINE_ABILITY_LEARNED_ON_SKILL_VALUE && ability->AutolearnType != SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN)
            continue;

        // Check race if set
        if (ability->racemask && !(ability->racemask & raceMask))
            continue;

        // Check class if set
        if (ability->classmask && !(ability->classmask & classMask))
            continue;

        // still necessary ?
        if (spellInfo->Effects[0].Effect == SPELL_EFFECT_SUMMON) // these values seems wrong in the dbc. See spells 19804, 13166, 13258, 4073, 12749
            continue;

        // need unlearn spell
        if (skillValue < ability->req_skill_value && ability->AutolearnType == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_VALUE)
            RemoveSpell(ability->spellId);
        // need learn spell
        else
        {
            uint32 addSpellID = ability->spellId;
#ifdef LICH_KING
            // sunstrider: Apparently this has changed since BC... Our 21084 is not learned by default (AutolearnType is 0)
            // used to avoid double Seal of Righteousness on paladins, it's the only player spell which has both spell and forward spell in auto learn
            if (ability->AutolearnType == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN && ability->forward_spellid)
            {
                bool skipCurrent = false;
                auto bounds = sSpellMgr->GetSkillLineAbilityMapBounds(ability->forward_spellid);
                for (auto itr = bounds.first; itr != bounds.second; ++itr)
                {
                    if (itr->second->AutolearnType == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN && skillValue >= itr->second->req_skill_value)
                    {
                        skipCurrent = true;
                        break;
                    }
                }

                if (skipCurrent)
                    continue;
            }
#else
            if (ability->spellId == 20154) // The second Seal of Righteousness that shouldn't be added
                addSpellID = ability->forward_spellid;
#endif

            if (!IsInWorld())
                AddSpell(addSpellID, true, true, true, false, false, ability->skillId);
            else
                LearnSpell(addSpellID, true, ability->skillId);
        }
    }
}

void Player::LearnAllClassProficiencies()
{
    std::vector<uint32> weaponAndArmorSkillsList = { 196,197,198,199,200,201,202,227,264,266,1180,2567,5011,15590, //weapons
                                                     8737, 750, 8737, 9116, 674 }; //armors & shield & dual wield

    for (auto itr : weaponAndArmorSkillsList)
    {
        // known spell
        if (HasSpell(itr))
            continue;

        //exception : skip dual wield for shaman (this only comes with spec)
        if (GetClass() == CLASS_SHAMAN && itr == 674)
            continue;

        //a bit hacky: lets transform our spells into trainer spell to be able to use GetTrainerSpellState
        TrainerSpell trainerSpell;
        trainerSpell.reqlevel = 0;
        trainerSpell.reqskill = 0;
        trainerSpell.reqskillvalue = 0;
        trainerSpell.spell = itr;
        trainerSpell.spellcost = 0;

        if (GetTrainerSpellState(&trainerSpell) != TRAINER_SPELL_GREEN)
            continue;

        LearnSpell(trainerSpell.spell, false);
    }
}

//Hacky way, learn from a designated trainer for each class
void Player::LearnAllClassSpells()
{
    uint8 playerClass = GetClass();
    uint32 classMaster;
    switch(playerClass)
    {
    case CLASS_WARRIOR:      classMaster = 5479;  break;
    case CLASS_PALADIN:      classMaster = 928;   break;
    case CLASS_HUNTER:       classMaster = 5515;  break;
    case CLASS_ROGUE:        classMaster = 918;   break;
    case CLASS_PRIEST:       classMaster = 5489;  break;
    case CLASS_SHAMAN:       classMaster = 20407; break;
    case CLASS_MAGE:         classMaster = 5498;  break;
    case CLASS_WARLOCK:      classMaster = 5495;  break;
    case CLASS_DRUID:        classMaster = 5504;  break;
    default: return;
    }

    switch(GetClass())
    {
        case CLASS_SHAMAN:
        {
            //those totems are learned from quests
            LearnSpell(8071, false); //stoneskin totem
            LearnSpell(3599, false); //incendiary totem
            LearnSpell(5394, false); //healing totem
        }
        break;
        case CLASS_DRUID: //only 1 form seems to appear in the form bar until reconnexion
            if(GetLevel() >= 10)
            {
                LearnSpell(9634, false); //bear
                LearnSpell(6807, false); //maul rank 1
            }
            if(GetLevel() >= 20)
                LearnSpell(768, false); //cat
            if(GetLevel() >= 26)
                LearnSpell(1066, false); //aqua
            if(GetLevel() >= 30)
                LearnSpell(783, false); //travel
            break;
        case CLASS_HUNTER:
        {
            CastSpell(this,5300, true); //learn some pet related spells
            LearnSpell(883, false); //call pet
            LearnSpell(2641, false);//dismiss pet
            LearnSpell(1515, false); //taming spell
            //pet spells
            uint32 spellsId [119] = {5149,883,1515,6991,2641,982,17254,737,17262,24424,26184,3530,26185,35303,311,26184,17263,7370,35299,35302,17264,1749,231,2441,23111,2976,23111,17266,2981,17262,24609,2976,26094,2982,298,1747,17264,24608,26189,24454,23150,24581,2977,1267,1748,26065,24455,1751,17265,23146,17267,23112,17265,2310,23100,24451,175,24607,2315,2981,24641,25013,25014,17263,3667,24584,3667,2975,23146,25015,1749,26185,1750,35388,17266,24607,25016,23149,24588,23149,295,27361,26202,35306,2619,2977,16698,3666,3666,24582,23112,26202,1751,16698,24582,17268,24599,24589,25017,35391,3489,28343,35307,27347,27349,353,24599,35324,27347,35348,27348,17268,27348,27346,24845,27361,2751,24632,35308 };
            for (uint32 i : spellsId)
                AddSpell(i,true);
            break;
        }
        case CLASS_PALADIN:
            //Pala mounts
            if (GetTeam() == ALLIANCE) {
                AddSpell(23214, true); //60
                AddSpell(13819, true); //40
            } else {
                AddSpell(34767, true); //60
                AddSpell(34769, true); //40
            }
            break;
        case CLASS_WARLOCK:
            AddSpell(5784, true); //mount 40
            AddSpell(23161, true); //mount 60
            AddSpell(688, true); //imp
            AddSpell(697, true); //void walker
            AddSpell(712, true); //Succubus
            AddSpell(691, true); //Fel Hunter
            break;
        default:
            break;
    }

    {
        //class specific spells/skills from recuperation data
        int faction = (GetTeam() == ALLIANCE) ? 1 : 2;
        QueryResult query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE classe = %u AND (faction = %u OR faction = 0)", GetClass(), faction);
        if (query) {
            do {
                Field* fields = query->Fetch();
                std::string tempstr = fields[0].GetString();

                {
                    std::vector<std::string> v, vline;
                    std::vector<std::string>::iterator i;

                    int cutAt;
                    while ((cutAt = tempstr.find_first_of(";")) != tempstr.npos) {
                        if (cutAt > 0) {
                            vline.push_back(tempstr.substr(0, cutAt));
                        }
                        tempstr = tempstr.substr(cutAt + 1);
                    }

                    if (tempstr.length() > 0) {
                        vline.push_back(tempstr);
                    }

                    for (i = vline.begin(); i != vline.end(); i++) {
                        v.clear();
                        tempstr = *i;
                        while ((cutAt = tempstr.find_first_of(" ")) != tempstr.npos) {
                            if (cutAt > 0) {
                                v.push_back(tempstr.substr(0, cutAt));
                            }
                            tempstr = tempstr.substr(cutAt + 1);
                        }

                        if (tempstr.length() > 0) {
                            v.push_back(tempstr);
                        }

                        if (v[0] == "learn") {
                            uint32 spell = atol(v[1].c_str());
                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
                            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer())) {
                                continue;
                            }

                            if (!HasSpell(spell))
                                AddSpell(spell, true);
                        }
                        else if (v[0] == "setskill") {
                            /* skill, v[1] == skill ID */
                            int32 skill = atoi(v[1].c_str());
                            if (skill <= 0) {
                                continue;
                            }

                            int32 maxskill = 375;

                            SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
                            if (!sl) {
                                continue;
                            }

                            if (!GetSkillValue(skill)) {
                                continue;
                            }

                            SetSkill(skill, 3, 375, maxskill);
                        }
                    }
                }
            } while (query->NextRow());

        }
        else {
            TC_LOG_ERROR("entities.player", "Player creation : failed to get data from recups_data to add initial spells/skills");
        }
    }

    TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(classMaster);
    if(!trainer_spells)
    {
        TC_LOG_ERROR("entities.player","Player::LearnAllClassSpell - No trainer spells for entry %u.",playerClass);
        return;
    }

    //the spells in trainer list aren't in the right order, so some spells won't be learned. The i loop is a ugly hack to fix this.
    for(int i = 0; i < 15; i++)
    {
        for(auto itr : trainer_spells->spellList)
        {
            if(GetTrainerSpellState(itr) != TRAINER_SPELL_GREEN)
                continue;

            LearnSpell(itr->spell, false);
        }
    }
}

void Player::DoPack58(uint8 step)
{
    if(step == PACK58_STEP1)
    {
        uint32 destroyCount;
        GiveLevel(58);
        InitTalentForLevel();
        SetUInt32Value(PLAYER_XP,0);
        LearnSpell(33388, false); //mount 75
        uint32 mountid = 0;
        switch(GetRace())
        {
        case RACE_HUMAN:           mountid = 5656; break;
        case RACE_ORC:             mountid = 1132; break;
        case RACE_DWARF:           mountid = 5873; break;
        case RACE_NIGHTELF:        mountid = 8629; break;
        case RACE_UNDEAD_PLAYER:   mountid = 13332; break;
        case RACE_TAUREN:          mountid = 15277; break;
        case RACE_GNOME:           mountid = 13322; break;
        case RACE_TROLL:           mountid = 8592; break;
        case RACE_BLOODELF:        mountid = 28927; break;
        case RACE_DRAENEI:         mountid = 28481; break;
        }
        StoreNewItemInBestSlots(mountid, 1);
        LearnAllClassProficiencies();

        uint32 addBags = 4;

        //give totems to shamans
        switch(GetClass())
        {
            case CLASS_SHAMAN:
            {
                uint32 totemsId[4] = {5176,5177,5175,5178};
                for(uint32 i : totemsId)
                    StoreNewItemInBestSlots(i, 1);
                break;
            }
            case CLASS_ROGUE:
            {
                StoreNewItemInBestSlots(5060, 1); //thieve tools
                break;
            }
            case CLASS_WARLOCK:
            {
                StoreNewItemInBestSlots(6265, 20); //Soul shard
                StoreNewItemInBestSlots(21872, 1); //Ebon Shadowbag
                addBags = 3;
                break;
            }
            case CLASS_HUNTER:
            {
                RemoveAmmo();
                destroyCount = 10000;
                DestroyItemCount(2512, destroyCount, true); //base arrows
                destroyCount = 10000;
                DestroyItemCount(2516, destroyCount, true); //base bullets
                destroyCount = 1;
                SwapItem(65299, 65317); //hacky hacky, try removing quiver if any found at first bag pos. Will fail if not empty.

                StoreNewItemInBestSlots(19319, 1); //Harpy Hide Quiver
                StoreNewItemInBestSlots(18042, 1400); //Thorium Headed Arrow (lvl 52)
                SetAmmo(18042);
                addBags = 3;
                break;
            }
            break;
        }

        destroyCount = 2;
        DestroyItemCount(159, destroyCount, true); // Destroy starting water

        StoreNewItemInBestSlots(21841, addBags); //netherweave bags

        StoreNewItemInBestSlots(27854, 20); //food
        StoreNewItemInBestSlots(8766, 20); //drink

        LearnAllClassSpells();
        UpdateSkillsToMaxSkillsForLevel();

        //relocate homebind + some taxis
        WorldLocation loc;
        uint32 area_id;
        if (Player::TeamForRace(GetRace()) == ALLIANCE)
        {
            loc = WorldLocation(0, -8866.468750, 671.831238, 97.903374, 2.154216);
            area_id = 1519; // Stormwind

            m_taxi.SetTaximaskNode(TAXI_STORMWIND);
            m_taxi.SetTaximaskNode(TAXI_IRONFORGE);
            m_taxi.SetTaximaskNode(TAXI_SOUTHSHORE);
            m_taxi.SetTaximaskNode(TAXI_BOOTY_BAY_ALLIANCE);
            m_taxi.SetTaximaskNode(TAXI_AUBERDINE);
            m_taxi.SetTaximaskNode(TAXI_RUTHERAN);
            m_taxi.SetTaximaskNode(TAXI_ASTRANAAR);
            m_taxi.SetTaximaskNode(TAXI_THERAMODE);
            m_taxi.SetTaximaskNode(TAXI_GADGETZAN_ALLIANCE);
            m_taxi.SetTaximaskNode(TAXI_HINTERLANDS);
            m_taxi.SetTaximaskNode(TAXI_STRANGLETHORN_REBEL_CAMP);
            m_taxi.SetTaximaskNode(TAXI_EVERLOOK_ALLIANCE); 
            m_taxi.SetTaximaskNode(TAXI_FEATHERMOON);
            m_taxi.SetTaximaskNode(TAXI_NETHERGARDE_KEEP);
        } else {
            loc = WorldLocation(1, 1632.54, -4440.77, 15.4584, 1.0637);
            area_id = 1637; // Orgrimmar

            m_taxi.SetTaximaskNode(TAXI_ORGRIMMAR);
            m_taxi.SetTaximaskNode(TAXI_UNDERCITY);
            m_taxi.SetTaximaskNode(TAXI_HAMMERFALL);
            m_taxi.SetTaximaskNode(TAXI_BOOTY_BAY_HORDE);
            m_taxi.SetTaximaskNode(TAXI_KARGATH);
            m_taxi.SetTaximaskNode(TAXI_GROMGOL);
            m_taxi.SetTaximaskNode(TAXI_THUNDERBLUFF);
            m_taxi.SetTaximaskNode(TAXI_ORGRIMMAR);
            m_taxi.SetTaximaskNode(TAXI_THOUSAND_NEEDLES);
            m_taxi.SetTaximaskNode(TAXI_GADGETZAN_HORDE);
            m_taxi.SetTaximaskNode(TAXI_EVERLOOK_HORDE);
            m_taxi.SetTaximaskNode(TAXI_SPLINTERTREE);
            m_taxi.SetTaximaskNode(TAXI_TAURAJO);
            m_taxi.SetTaximaskNode(TAXI_SILVERMOON);
            m_taxi.SetTaximaskNode(TAXI_TRAQUILIEN);
            m_taxi.SetTaximaskNode(TAXI_STONARD);
        }
        SetHomebind(loc, area_id);

        // Also give some money
        ModifyMoney(250 * GOLD);
    } else {
        for(int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        {
            Item* currentItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
            if(!currentItem)
                continue;

            DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
        }
        uint8 packType;
        switch(step)
        {
        case PACK58_MELEE: packType = PACK58_TYPE_MELEE; break;
        case PACK58_HEAL:  packType = PACK58_TYPE_HEAL; break;
        case PACK58_TANK:  packType = PACK58_TYPE_TANK; break;
        case PACK58_MAGIC: packType = PACK58_TYPE_MAGIC; break;
        }

        QueryResult result = WorldDatabase.PQuery("SELECT item, count FROM pack58 WHERE class = %u and type = %u", GetClass(), packType);

        uint32 count = 0;
        if(result)
        {
            do
            {
                count++;
                Field *fields = result->Fetch();
                uint32 itemid = fields[0].GetUInt32();
                uint32 _count = fields[1].GetUInt32();
                if(!itemid || !_count)
                    continue;

                StoreNewItemInBestSlots(itemid, _count);
            }
            while( result->NextRow() );
        }
        if(count == 0)
            TC_LOG_ERROR("entities.player","DoPack58 : no item for given class (%u) & type (%u)", GetClass(), packType);
    }
    SaveToDB();
}

void Player::SendAuraDurationsForTarget(Unit* target)
{
    if (!target || target->GetVisibleAuras()->empty())                  // speedup things
        return;

    /*! Blizz sends certain movement packets sometimes even before CreateObject
    These movement packets are usually found in SMSG_COMPRESSED_MOVES
    */
    if (target->HasAuraType(SPELL_AURA_FEATHER_FALL))
        target->SetFeatherFall(true);

    if (target->HasAuraType(SPELL_AURA_WATER_WALK))
        target->SetWaterWalking(true);

    if (target->HasAuraType(SPELL_AURA_HOVER))
        target->SetHover(true);

#ifdef LICH_KING
    WorldPacket data(SMSG_AURA_UPDATE_ALL);
    data << target->GetPackGUID();
    Unit::VisibleAuraMap const* visibleAuras = target->GetVisibleAuras();
    for (Unit::VisibleAuraMap::const_iterator itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        AuraApplication * auraApp = itr->second;
        auraApp->BuildUpdatePacket(data, false);
    }
    SendDirectMessage(&data);
#else
    Unit::VisibleAuraMap const* visibleAuras = target->GetVisibleAuras();
    for (Unit::VisibleAuraMap::const_iterator itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        AuraApplication * auraApp = itr->second;
        auraApp->SendAuraDurationForCaster(this);
    }
#endif
}

void Player::SetDailyQuestStatus( uint32 quest_id )
{
    for(uint32 quest_daily_idx = 0; quest_daily_idx < PLAYER_MAX_DAILY_QUESTS; ++quest_daily_idx)
    {
        if(!GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx))
        {
            SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,quest_id);
            m_lastDailyQuestTime = WorldGameTime::GetGameTime();              // last daily quest time
            m_DailyQuestChanged = true;
            break;
        }
    }
}

bool Player::IsDailyQuestDone(uint32 quest_id) const
{
    bool found = false;
    if (sObjectMgr->GetQuestTemplate(quest_id))
    {
        for (uint32 quest_daily_idx = 0; quest_daily_idx < PLAYER_MAX_DAILY_QUESTS; ++quest_daily_idx)
        {
            if (GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1 + quest_daily_idx) == quest_id)
            {
                found = true;
                break;
            }
        }
    }

    return found;
}

void Player::ResetDailyQuestStatus()
{
    for(uint32 quest_daily_idx = 0; quest_daily_idx < PLAYER_MAX_DAILY_QUESTS; ++quest_daily_idx)
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,0);

    // DB data deleted in caller
    m_DailyQuestChanged = false;
    m_lastDailyQuestTime = 0;
}

Battleground* Player::GetBattleground() const
{
    if(GetBattlegroundId()==0)
        return nullptr;

    return sBattlegroundMgr->GetBattleground(GetBattlegroundId(), m_bgData.bgTypeID);
}


bool Player::InBattlegroundQueue(bool ignoreArena) const
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        if (m_bgBattlegroundQueueID[i].bgQueueTypeId != BATTLEGROUND_QUEUE_NONE)
            if (!ignoreArena || (m_bgBattlegroundQueueID[i].bgQueueTypeId != BATTLEGROUND_QUEUE_2v2 &&
                m_bgBattlegroundQueueID[i].bgQueueTypeId != BATTLEGROUND_QUEUE_3v3 &&
                m_bgBattlegroundQueueID[i].bgQueueTypeId != BATTLEGROUND_QUEUE_5v5))
                return true;
    return false;
}

BattlegroundQueueTypeId Player::GetBattlegroundQueueTypeId(uint32 index) const
{
    return m_bgBattlegroundQueueID[index].bgQueueTypeId;
}

uint32 Player::GetBattlegroundQueueIndex(BattlegroundQueueTypeId bgQueueTypeId) const
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        if (m_bgBattlegroundQueueID[i].bgQueueTypeId == bgQueueTypeId)
            return i;
    return PLAYER_MAX_BATTLEGROUND_QUEUES;
}

bool Player::IsInvitedForBattlegroundQueueType(BattlegroundQueueTypeId bgQueueTypeId) const
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        if (m_bgBattlegroundQueueID[i].bgQueueTypeId == bgQueueTypeId)
            return m_bgBattlegroundQueueID[i].invitedToInstance != 0;
    return false;
}

bool Player::InBattlegroundQueueForBattlegroundQueueType(BattlegroundQueueTypeId bgQueueTypeId) const
{
    return GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES;
}

void Player::SetBattlegroundId(uint32 val, BattlegroundTypeId bgTypeId)
{
    m_bgData.bgInstanceID = val;
    m_bgData.bgTypeID = bgTypeId;
}

uint32 Player::AddBattlegroundQueueId(BattlegroundQueueTypeId val)
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        if (m_bgBattlegroundQueueID[i].bgQueueTypeId == BATTLEGROUND_QUEUE_NONE || m_bgBattlegroundQueueID[i].bgQueueTypeId == val)
        {
            m_bgBattlegroundQueueID[i].bgQueueTypeId = val;
            m_bgBattlegroundQueueID[i].invitedToInstance = 0;
            return i;
        }
    }
    return PLAYER_MAX_BATTLEGROUND_QUEUES;
}

bool Player::HasFreeBattlegroundQueueId() const
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        if (m_bgBattlegroundQueueID[i].bgQueueTypeId == BATTLEGROUND_QUEUE_NONE)
            return true;
    return false;
}

void Player::RemoveBattlegroundQueueId(BattlegroundQueueTypeId val)
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        if (m_bgBattlegroundQueueID[i].bgQueueTypeId == val)
        {
            m_bgBattlegroundQueueID[i].bgQueueTypeId = BATTLEGROUND_QUEUE_NONE;
            m_bgBattlegroundQueueID[i].invitedToInstance = 0;
            return;
        }
    }
}

void Player::SetInviteForBattlegroundQueueType(BattlegroundQueueTypeId bgQueueTypeId, uint32 instanceId)
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        if (m_bgBattlegroundQueueID[i].bgQueueTypeId == bgQueueTypeId)
            m_bgBattlegroundQueueID[i].invitedToInstance = instanceId;
}

bool Player::IsInvitedForBattlegroundInstance(uint32 instanceId) const
{
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        if (m_bgBattlegroundQueueID[i].invitedToInstance == instanceId)
            return true;
    return false;
}

bool Player::InArena() const
{
    Battleground *bg = GetBattleground();
    if(!bg || !bg->IsArena())
        return false;

    return true;
}

bool Player::GetBGAccessByLevel(BattlegroundTypeId bgTypeId) const
{
    // get a template bg instead of running one
    Battleground *bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if(!bg)
        return false;

    if(GetLevel() < bg->GetMinLevel() || GetLevel() > bg->GetMaxLevel())
        return false;

    return true;
}

float Player::GetReputationPriceDiscount(Creature const* creature) const
{
    return GetReputationPriceDiscount(creature->GetFactionTemplateEntry());
}

float Player::GetReputationPriceDiscount(FactionTemplateEntry const* factionTemplate) const
{
    if (!factionTemplate || !factionTemplate->faction)
        return 1.0f;

    ReputationRank rank = GetReputationRank(factionTemplate->faction);
    if (rank <= REP_NEUTRAL)
        return 1.0f;

    return 1.0f - 0.05f* (rank - REP_NEUTRAL);
}

Player* Player::GetTrader() const
{
    return m_trade ? m_trade->GetTrader() : nullptr;
}

bool Player::HandlePassiveSpellLearn(SpellInfo const* spellInfo)
{
    // note: form passives activated with shapeshift spells be implemented by HandleShapeshiftBoosts instead of spell_learn_spell
    // talent dependent passives activated at form apply have proper stance data
    ShapeshiftForm form = GetShapeshiftForm();
    bool need_cast = (!spellInfo->Stances || (form && (spellInfo->Stances & (UI64LIT(1) << (form - 1)))) ||
        (!form && spellInfo->HasAttribute(SPELL_ATTR2_NOT_NEED_SHAPESHIFT)));

    // Check EquippedItemClass
    // passive spells which apply aura and have an item requirement are to be added manually, instead of casted
    if (spellInfo->EquippedItemClass >= 0)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].IsAura())
            {
                if (!HasAura(spellInfo->Id) && HasItemFitToSpellRequirements(spellInfo))
                    AddAura(spellInfo->Id, this);
                return false;
            }
        }
    }

    //Check CasterAuraStates
    return need_cast && (!spellInfo->CasterAuraState || HasAuraState(AuraStateType(spellInfo->CasterAuraState)));
}


/* Warning : This is wrong for some spells such as draenei racials or paladin mount skills/spells */
bool Player::IsSpellFitByClassAndRace( uint32 spell_id ) const
{
    uint32 racemask  = GetRaceMask();
    uint32 classmask = GetClassMask();

    SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spell_id);
    for(auto _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
    {
        // skip wrong race skills
        if( _spell_idx->second->racemask && (_spell_idx->second->racemask & racemask) == 0)
            return false;

        // skip wrong class skills
        if( _spell_idx->second->classmask && (_spell_idx->second->classmask & classmask) == 0)
            return false;
    }
    return true;
}

bool Player::HasQuestForGO(int32 GOId)
{
    for(auto & m_QuestStatu : m_QuestStatus)
    {
        QuestStatusData qs=m_QuestStatu.second;
        if (qs.Status == QUEST_STATUS_INCOMPLETE)
        {
            Quest const* qinfo = sObjectMgr->GetQuestTemplate(m_QuestStatu.first);
            if(!qinfo)
                continue;

            if(GetGroup() && GetGroup()->isRaidGroup() && qinfo->GetType() != QUEST_TYPE_RAID)
                continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                if (qinfo->RequiredNpcOrGo[j]>=0)         //skip non GO case
                    continue;

                if((-1)*GOId == qinfo->RequiredNpcOrGo[j] && qs.CreatureOrGOCount[j] < qinfo->RequiredNpcOrGoCount[j])
                    return true;
            }
        }
    }
    return false;
}

void Player::UpdateForQuestWorldObjects()
{
    if(m_clientGUIDs.empty())
        return;

    if (!IsInWorld())
        return;

    UpdateData udata;
    WorldPacket packet;
    for(ObjectGuid m_clientGUID : m_clientGUIDs)
    {
        if(m_clientGUID.IsGameObject())
        {
            if (GameObject* obj = ObjectAccessor::GetGameObject(*this, m_clientGUID))
                obj->BuildValuesUpdateBlockForPlayer(&udata, this);
        }   
#ifdef LICH_KING
        else if (itr->IsCreatureOrVehicle())
        {
            Creature* obj = ObjectAccessor::GetCreatureOrPetOrVehicle(*this, *itr);
            if (!obj)
                continue;

            // check if this unit requires quest specific flags
            if (!obj->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK))
                continue;

            SpellClickInfoMapBounds clickPair = sObjectMgr->GetSpellClickInfoMapBounds(obj->GetEntry());
            for (SpellClickInfoContainer::const_iterator _itr = clickPair.first; _itr != clickPair.second; ++_itr)
            {
                //! This code doesn't look right, but it was logically converted to condition system to do the exact
                //! same thing it did before. It definitely needs to be overlooked for intended functionality.
                if (ConditionContainer const* conds = sConditionMgr->GetConditionsForSpellClickEvent(obj->GetEntry(), _itr->second.spellId))
                {
                    bool buildUpdateBlock = false;
                    for (ConditionContainer::const_iterator jtr = conds->begin(); jtr != conds->end() && !buildUpdateBlock; ++jtr)
                        if ((*jtr)->ConditionType == CONDITION_QUESTREWARDED || (*jtr)->ConditionType == CONDITION_QUESTTAKEN || (*jtr)->ConditionType == CONDITION_QUEST_COMPLETE)
                            buildUpdateBlock = true;

                    if (buildUpdateBlock)
                    {
                        obj->BuildValuesUpdateBlockForPlayer(&udata, this);
                        break;
                    }
                }
            }
        }
#endif
    }
    udata.BuildPacket(&packet, false);
    SendDirectMessage(&packet);
}

void Player::SetSummonPoint(uint32 mapid, float x, float y, float z)
{
    m_summon_expire = WorldGameTime::GetGameTime() + MAX_PLAYER_SUMMON_DELAY;
    m_summon_mapid = mapid;
    m_summon_x = x;
    m_summon_y = y;
    m_summon_z = z;
    m_invite_summon = true;
}

void Player::SummonIfPossible(bool agree)
{
    // expire and auto declined
    if(m_summon_expire < WorldGameTime::GetGameTime())
        return;

    if(!agree)
    {
        m_summon_expire = 0;
        return;
    }

    // stop taxi flight at summon
    FinishTaxiFlight();

    // drop flag at summon
    if(Battleground *bg = GetBattleground())
        bg->EventPlayerDroppedFlag(this);

    m_summon_expire = 0;

    TeleportTo(m_summon_mapid, m_summon_x, m_summon_y, m_summon_z,GetOrientation());
    m_invite_summon = false;
}

void Player::UpdateSummonExpireTime() 
{ 
    m_summon_expire = WorldGameTime::GetGameTime() + MAX_PLAYER_SUMMON_DELAY; 
}

void Player::RemoveItemDurations( Item *item )
{
    for(auto itr = m_itemDuration.begin();itr != m_itemDuration.end(); ++itr)
    {
        if(*itr==item)
        {
            m_itemDuration.erase(itr);
            break;
        }
    }
}

void Player::AddItemDurations( Item *item )
{
    if(item->GetUInt32Value(ITEM_FIELD_DURATION))
    {
        m_itemDuration.push_back(item);
        item->SendTimeUpdate(this);
    }
}

void Player::AutoUnequipOffhandIfNeed()
{
    Item *offItem = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND );
    if(!offItem)
        return;

    Item *mainItem = GetItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND );

    if(!mainItem || mainItem->GetTemplate()->InventoryType != INVTYPE_2HWEAPON)
        return;

    ItemPosCountVec off_dest;
    uint8 off_msg = CanStoreItem( NULL_BAG, NULL_SLOT, off_dest, offItem, false );
    if( off_msg == EQUIP_ERR_OK )
    {
        RemoveItem(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND, true);
        StoreItem( off_dest, offItem, true );
    }
    else
    {
        TC_LOG_ERROR("entities.player","Player::EquipItem: Can's store offhand item at 2hand item equip for player (GUID: %u).",GetGUID().GetCounter());
    }
}

OutdoorPvP * Player::GetOutdoorPvP() const
{
    return sOutdoorPvPMgr->GetOutdoorPvPToZoneId(GetZoneId());
}

bool Player::HasItemFitToSpellRequirements(SpellInfo const* spellInfo, Item const* ignoreItem) const
{
    if(spellInfo->EquippedItemClass < 0)
        return true;

    //these need to be excepted else client wont properly show weapon/armor skills
    for(const auto & Effect : spellInfo->Effects)
        if(Effect.Effect == SPELL_EFFECT_PROFICIENCY)
            return true;

    // scan other equipped items for same requirements (mostly 2 daggers/etc)
    // for optimize check 2 used cases only
    switch(spellInfo->EquippedItemClass)
    {
        case ITEM_CLASS_WEAPON:
        {
            for(int i= EQUIPMENT_SLOT_MAINHAND; i < EQUIPMENT_SLOT_TABARD; ++i)
                if(Item *item = GetUseableItemByPos( INVENTORY_SLOT_BAG_0, i ))
                    if(item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                        return true;
            break;
        }
        case ITEM_CLASS_ARMOR:
        {
            // most used check: shield only
            if (spellInfo->EquippedItemSubClassMask & ((1 << ITEM_SUBCLASS_ARMOR_BUCKLER) | (1 << ITEM_SUBCLASS_ARMOR_SHIELD)))
            {
                if (Item* item = GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                    if (item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                        return true;
 
                // special check to filter things like Shield Wall, the aura is not permanent and must stay even without required item
                if (!spellInfo->IsPassive())
                {
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        if (spellInfo->Effects[i].IsAura())
                            return true;
                }
            }
            // tabard not have dependent spells
            for(int i= EQUIPMENT_SLOT_START; i< EQUIPMENT_SLOT_MAINHAND; ++i)
                if(Item *item = GetUseableItemByPos( INVENTORY_SLOT_BAG_0, i ))
                    if(item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                        return true;

            // ranged slot can have some armor subclasses
            if(Item *item = GetUseableItemByPos( INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
                if(item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                    return true;

            break;
        }
        default:
            TC_LOG_ERROR("entities.player","HasItemFitToSpellRequirements: Not handled spell requirement for item class %u",spellInfo->EquippedItemClass);
            break;
    }

    return false;
}

uint32 Player::GetResurrectionSpellId()
{
    // search priceless resurrection possibilities
    uint32 prio = 0;
    uint32 spell_id = 0;
    AuraEffectList const& dummyAuras = GetAuraEffectsByType(SPELL_AURA_DUMMY);
    for(auto dummyAura : dummyAuras)
    {
        // Soulstone Resurrection                           // prio: 3 (max, non death persistent)
        if (prio < 2 && dummyAura->GetSpellInfo()->HasVisual(99) && dummyAura->GetSpellInfo()->SpellIconID == 92)
        {
            switch(dummyAura->GetId())
            {
                case 20707: spell_id =  3026; break;        // rank 1
                case 20762: spell_id = 20758; break;        // rank 2
                case 20763: spell_id = 20759; break;        // rank 3
                case 20764: spell_id = 20760; break;        // rank 4
                case 20765: spell_id = 20761; break;        // rank 5
                case 27239: spell_id = 27240; break;        // rank 6
                default:
                    TC_LOG_ERROR("spell","Unhandled spell %u: S.Resurrection",dummyAura->GetId());
                    continue;
            }

            prio = 3;
        }
        // Twisting Nether                                  // prio: 2 (max)
        else if(dummyAura->GetId()==23701 && roll_chance_i(10))
        {
            prio = 2;
            spell_id = 23700;
        }
    }

    // Reincarnation (passive spell)                        // prio: 1
    if(prio < 1 && HasSpell(20608) && !GetSpellHistory()->HasCooldown(21169) && HasItemCount(17030,1))
        spell_id = 21169;

    return spell_id;
}

void Player::OfflineResurrect(ObjectGuid const& guid, SQLTransaction& trans)
{
    Corpse::DeleteFromDB(guid, trans);
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, uint16(AT_LOGIN_RESURRECT));
    stmt->setUInt64(1, guid.GetCounter());
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

bool Player::RewardPlayerAndGroupAtKill(Unit* pVictim)
{
    bool PvP = pVictim->IsCharmedOwnedByPlayerOrPlayer();

    // prepare data for near group iteration (PvP and !PvP cases)
    uint32 xp = 0;
    bool honored_kill = false;

    if(Group *pGroup = GetGroup())
    {
        uint32 count = 0;
        uint32 sum_level = 0;
        Player* member_with_max_level = nullptr;
        Player* not_gray_member_with_max_level = nullptr;

        pGroup->GetDataForXPAtKill(pVictim,count,sum_level,member_with_max_level,not_gray_member_with_max_level);

        if(member_with_max_level)
        {
            // PvP kills doesn't yield experience
            // also no XP gained if there is no member below gray level
            xp = (PvP || !not_gray_member_with_max_level) ? 0 : Trinity::XP::Gain(not_gray_member_with_max_level, pVictim);

            /// skip in check PvP case (for speed, not used)
            bool is_raid = PvP ? false : sMapStore.LookupEntry(GetMapId())->IsRaid() && pGroup->isRaidGroup();
            bool is_dungeon = PvP ? false : sMapStore.LookupEntry(GetMapId())->IsDungeon();
            float group_rate = Trinity::XP::xp_in_group_rate(count,is_raid);

            for(GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* pGroupGuy = itr->GetSource();
                if(!pGroupGuy)
                    continue;

                if(!pGroupGuy->IsAtGroupRewardDistance(pVictim))
                    continue;                               // member (alive or dead) or his corpse at req. distance

                // honor can be in PvP and !PvP (racial leader) cases (for alive)
                if(pGroupGuy->IsAlive() && pGroupGuy->RewardHonor(pVictim,count, -1, true) && pGroupGuy==this)
                    honored_kill = true;

                // xp and reputation only in !PvP case
                if(!PvP)
                {
                    float rate = group_rate * float(pGroupGuy->GetLevel()) / sum_level;

                    // if is in dungeon then all receive full reputation at kill
                    // rewarded any alive/dead/near_corpse group member
                    pGroupGuy->RewardReputation(pVictim,is_dungeon ? 1.0f : rate);

                    // XP updated only for alive group member
                    if(pGroupGuy->IsAlive() && not_gray_member_with_max_level &&
                       pGroupGuy->GetLevel() <= not_gray_member_with_max_level->GetLevel())
                    {
                        uint32 itr_xp = (member_with_max_level == not_gray_member_with_max_level) ? uint32(xp*rate) : uint32((xp*rate/2)+1);

                        pGroupGuy->GiveXP(itr_xp, pVictim);
                        if (Pet* pet = pGroupGuy->GetPet())
                        {
                            // TODO: Pets need to get exp based on their level diff to the target, not the owners.
                            // the whole RewardGroupAtKill needs a rewrite to match up with this anyways:
                            // http://wowwiki.wikia.com/wiki/Formulas:Mob_XP?oldid=228414
                            pet->GivePetXP((float)itr_xp / 1.5);
                        }
                    }

                    // quest objectives updated only for alive group member or dead but with not released body
                    if(pGroupGuy->IsAlive()|| !pGroupGuy->GetCorpse())
                    {
                        // normal creature (not pet/etc) can be only in !PvP case
                        if(pVictim->GetTypeId()==TYPEID_UNIT)
                            pGroupGuy->KilledMonsterCredit(pVictim->GetEntry(), pVictim->GetGUID());
                    }
                }
            }
        }
    }
    else                                                    // if (!pGroup)
    {
        xp = PvP ? 0 : Trinity::XP::Gain(this, pVictim);

        // honor can be in PvP and !PvP (racial leader) cases
        if(RewardHonor(pVictim,1, -1, true))
            honored_kill = true;

        // xp and reputation only in !PvP case
        if(!PvP)
        {
            RewardReputation(pVictim,1);
            GiveXP(xp, pVictim);

            if(Pet* pet = GetPet())
                pet->GivePetXP(xp);

            // normal creature (not pet/etc) can be only in !PvP case
            if(pVictim->GetTypeId()==TYPEID_UNIT)
                KilledMonsterCredit(pVictim->GetEntry(),pVictim->GetGUID());
        }
    }

    #ifdef PLAYERBOT
    sGuildTaskMgr.CheckKillTask(this, pVictim);
    #endif

    return xp || honored_kill;
}

void Player::RewardPlayerAndGroupAtEvent(uint32 creature_id, WorldObject* pRewardSource)
{
    if (!pRewardSource)
        return;

    ObjectGuid creature_guid = (pRewardSource->GetTypeId() == TYPEID_UNIT) ? pRewardSource->GetGUID() : ObjectGuid::Empty;

    // prepare data for near group iteration
    if (Group *pGroup = GetGroup())
    {
        for (GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* pGroupGuy = itr->GetSource();
            if (!pGroupGuy)
                continue;

            if (!pGroupGuy->IsAtGroupRewardDistance(pRewardSource))
                continue;                               // member (alive or dead) or his corpse at req. distance

            // quest objectives updated only for alive group member or dead but with not released body
            if (pGroupGuy->IsAlive()|| !pGroupGuy->GetCorpse())
                pGroupGuy->KilledMonsterCredit(creature_id, creature_guid);
        }
    }
    else                                                    // if (!pGroup)
        KilledMonsterCredit(creature_id, creature_guid);
}

bool Player::IsAtGroupRewardDistance(WorldObject const* pRewardSource) const
{
    const WorldObject* player = GetCorpse();
    if(!player || IsAlive())
        player = this;

    if(player->GetMapId() != pRewardSource->GetMapId() || player->GetInstanceId() != pRewardSource->GetInstanceId())
        return false;

    return pRewardSource->GetDistance(player) <= sWorld->getConfig(CONFIG_GROUP_XP_DISTANCE);
}

// Used in triggers for check "Only to targets that grant experience or honor" req
bool Player::IsHonorOrXPTarget(Unit* victim) const
{
    uint8 v_level = victim->GetLevel();
    uint8 k_grey = Trinity::XP::GetGrayLevel(GetLevel());

    // Victim level less gray level
    if (v_level <= k_grey /* TC && !sWorld->getIntConfig(CONFIG_MIN_CREATURE_SCALED_XP_RATIO)*/)
        return false;

    if (Creature const* creature = victim->ToCreature())
    {
        if (!creature->CanGiveExperience())
            return false;
    }
    return true;
}

uint32 Player::GetBaseWeaponSkillValue (WeaponAttackType attType) const
{
    Item* item = GetWeaponForAttack(attType,true);

    // unarmed only with base attack
    if(attType != BASE_ATTACK && !item)
        return 0;

    // weapon skill or (unarmed for base attack)
    uint32  skill = (item && item->GetSkill() != SKILL_FIST_WEAPONS) ? item->GetSkill() : uint32(SKILL_UNARMED);
    return GetBaseSkillValue(skill);
}

void Player::RessurectUsingRequestData()
{
    TeleportTo(m_resurrectMap, m_resurrectX, m_resurrectY, m_resurrectZ, GetOrientation());

    if (IsBeingTeleported())
    {
        ScheduleDelayedOperation(DELAYED_RESURRECT_PLAYER);
        return;
    }

    ResurrectPlayer(0.0f,false);

    if(GetMaxHealth() > m_resurrectHealth)
        SetHealth(m_resurrectHealth);
    else
        SetHealth(GetMaxHealth());

    if(GetMaxPower(POWER_MANA) > m_resurrectMana)
        SetPower(POWER_MANA, m_resurrectMana);
    else
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));

    SetPower(POWER_RAGE, 0);

    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));

    SpawnCorpseBones();
}

void Player::UpdateZoneDependentAuras( uint32 newZone )
{
    // remove new continent flight forms
    if( !IsGameMaster() &&
        GetVirtualMapForMapAndZone(GetMapId(),newZone) != 530)
    {
        RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
        RemoveAurasByType(SPELL_AURA_FLY);
    }

    // Some spells applied at enter into zone (with subzones)
    // Human Illusion
    // NOTE: these are removed by RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP);
    if ( newZone == 2367 )                                  // Old Hillsbrad Foothills
    {
        uint32 spellid = 0;
        // all horde races
        if( GetTeam() == HORDE )
            spellid = GetGender() == GENDER_FEMALE ? 35481 : 35480;
        // and some alliance races
        else if( GetRace() == RACE_NIGHTELF || GetRace() == RACE_DRAENEI )
            spellid = GetGender() == GENDER_FEMALE ? 35483 : 35482;

        if(spellid && !HasAuraEffect(spellid,0) )
            CastSpell(this,spellid, true);
    }

    // Some spells applied at enter into zone (with subzones), aura removed in UpdateAreaDependentAuras that called always at zone->area update
    SpellAreaForAreaMapBounds saBounds = sSpellMgr->GetSpellAreaForAreaMapBounds(newZone);
    for (auto itr = saBounds.first; itr != saBounds.second; ++itr)
        if (itr->second->autocast && itr->second->IsFitToRequirements(this, newZone, 0))
            if (!HasAura(itr->second->spellId))
                CastSpell(this, itr->second->spellId, true);
}

void Player::UpdateAreaDependentAuras( uint32 newArea )
{
    // remove auras from spells with area limitations
    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        // use m_zoneUpdateId for speed: UpdateArea called from UpdateZone or instead UpdateZone in both cases m_zoneUpdateId up-to-date
        if (iter->second->GetSpellInfo()->CheckLocation(GetMapId(), m_zoneUpdateId, newArea, this, false) != SPELL_CAST_OK)
            RemoveOwnedAura(iter);
        else
            ++iter;
    }

    // unmount if enter in this subzone
    if(newArea == 35)
        RemoveAurasByType(SPELL_AURA_MOUNTED);
    // Dragonmaw Illusion
    else if( newArea == 3759 || newArea == 3966 || newArea == 3939 || newArea == 3965 )
    {
        //if( GetDummyAura(40214) )
        if (GetReputationRank(1015) >= REP_NEUTRAL)
        {
            if( !HasAuraEffect(42016,0) ) 
            {
                CastSpell(this,42016, true);
                CastSpell(this,40216, true);
            }
        }
    }

    // some auras applied at subzone enter
    SpellAreaForAreaMapBounds saBounds = sSpellMgr->GetSpellAreaForAreaMapBounds(newArea);
    for (auto itr = saBounds.first; itr != saBounds.second; ++itr)
        if (itr->second->autocast && itr->second->IsFitToRequirements(this, m_zoneUpdateId, newArea))
            if (!HasAura(itr->second->spellId))
                CastSpell(this, itr->second->spellId, TRIGGERED_FULL_MASK);
}

uint32 Player::GetCorpseReclaimDelay(bool pvp) const
{
    if(pvp)
    {
        if(!sWorld->getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP))
            return copseReclaimDelay[0];
    }
    else if(!sWorld->getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE) )
        return 0;

    time_t now = GetMap()->GetGameTime();
    // 0..2 full period
    // should be ceil(x)-1 but not floor(x)
    uint32 count = (now < m_deathExpireTime - 1) ? (m_deathExpireTime - 1 - now)/DEATH_EXPIRE_STEP : 0;
    return copseReclaimDelay[count];
}

void Player::UpdateCorpseReclaimDelay()
{
    bool pvp = m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH;

    if( (pvp && !sWorld->getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP)) ||
        (!pvp && !sWorld->getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE)) )
        return;

    time_t now = GetMap()->GetGameTime();
    if(now < m_deathExpireTime)
    {
        // full and partly periods 1..3
        uint32 count = (m_deathExpireTime - now)/DEATH_EXPIRE_STEP +1;
        if(count < MAX_DEATH_COUNT)
            m_deathExpireTime = now+(count+1)*DEATH_EXPIRE_STEP;
        else
            m_deathExpireTime = now+MAX_DEATH_COUNT*DEATH_EXPIRE_STEP;
    }
    else
        m_deathExpireTime = now+DEATH_EXPIRE_STEP;
}



int32 Player::CalculateCorpseReclaimDelay(bool load) const
{
    Corpse* corpse = GetCorpse();

    if (load && !corpse)
        return -1;

    bool pvp = corpse ? corpse->GetType() == CORPSE_RESURRECTABLE_PVP : (m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH) != 0;

    uint32 delay;

    if (load)
    {
        if (corpse->GetGhostTime() > m_deathExpireTime)
            return -1;

        uint64 count = 0;

        if ((pvp && sWorld->getBoolConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP)) ||
            (!pvp && sWorld->getBoolConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE)))
        {
            count = (m_deathExpireTime - corpse->GetGhostTime()) / DEATH_EXPIRE_STEP;

            if (count >= MAX_DEATH_COUNT)
                count = MAX_DEATH_COUNT - 1;
        }

        time_t expected_time = corpse->GetGhostTime() + copseReclaimDelay[count];
        time_t now = GetMap()->GetGameTime();

        if (now >= expected_time)
            return -1;

        delay = expected_time - now;
    }
    else
        delay = GetCorpseReclaimDelay(pvp);

    return delay * IN_MILLISECONDS;
}

void Player::SendCorpseReclaimDelay(uint32 delay)
{
    //! corpse reclaim delay 30 * 1000ms or longer at often deaths
    WorldPacket data(SMSG_CORPSE_RECLAIM_DELAY, 4);
    data << uint32(delay);
    SendDirectMessage( &data );
}

Player* Player::GetNextRandomRaidMember(float radius) const
{
    Group const* pGroup = GetGroup();
    if(!pGroup)
        return nullptr;

    std::vector<Player*> nearMembers;
    nearMembers.reserve(pGroup->GetMembersCount());

    for(GroupReference const* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* Target = itr->GetSource();

        // IsHostileTo check duel and controlled by enemy
        if( Target && Target != this && IsWithinDistInMap(Target, radius) &&
            !Target->HasInvisibilityAura() && !IsHostileTo(Target) )
            nearMembers.push_back(Target);
    }

    if (nearMembers.empty())
        return nullptr;

    uint32 randTarget = urand(0,nearMembers.size()-1);
    return nearMembers[randTarget];
}

PartyResult Player::CanUninviteFromGroup() const
{
    const Group* grp = GetGroup();
    if(!grp)
        return PARTY_RESULT_YOU_NOT_IN_GROUP;

    if(!grp->IsLeader(GetGUID()) && !grp->IsAssistant(GetGUID()))
        return PARTY_RESULT_YOU_NOT_LEADER;

    if(InBattleground())
        return PARTY_RESULT_INVITE_RESTRICTED;

    return PARTY_RESULT_OK;
}

void Player::SetBattlegroundRaid(Group* group, int8 subgroup)
{
    // we must move references from m_group to m_originalGroup
    SetOriginalGroup(GetGroup(), GetSubGroup());

    m_group.unlink();
    m_group.link(group, this);
    m_group.setSubGroup((uint8)subgroup);
}

void Player::RemoveFromBattlegroundRaid()
{
    // remove existing reference
    m_group.unlink();
    if (Group* group = GetOriginalGroup())
    {
        m_group.link(group, this);
        m_group.setSubGroup(GetOriginalSubGroup());
    }
    SetOriginalGroup(nullptr);
}

void Player::SetOriginalGroup(Group *group, int8 subgroup)
{
    if (group == nullptr)
        m_originalGroup.unlink();
    else
    {
        // never use SetOriginalGroup without a subgroup unless you specify NULL for group
        assert(subgroup >= 0);
        m_originalGroup.link(group, this);
        m_originalGroup.setSubGroup((uint8)subgroup);
    }
}

void Player::ProcessTerrainStatusUpdate(ZLiquidStatus status, Optional<LiquidData> const& liquidData, bool /*updateCreatureLiquid*/)
{
    if (IsFlying() || IsInFlight()) //sun: also check IsInFlight 
    {
        m_MirrorTimerFlags &= ~(UNDERWATER_INWATER | UNDERWATER_INLAVA | UNDERWATER_INSLIME | UNDERWATER_INDARKWATER);
        return;
    }

    // process liquid auras using generic unit code
    Unit::ProcessTerrainStatusUpdate(status, liquidData);

    // player specific logic for mirror timers
    if (status && liquidData)
    {
        // Breath bar state (under water in any liquid type)
        if (liquidData->type_flags & MAP_ALL_LIQUIDS)
        {
            if (status & LIQUID_MAP_UNDER_WATER)
                m_MirrorTimerFlags |= UNDERWATER_INWATER;
            else
                m_MirrorTimerFlags &= ~UNDERWATER_INWATER;
        }

        // Fatigue bar state (if not on flight path or transport)
        if ((liquidData->type_flags & MAP_LIQUID_TYPE_DARK_WATER) && !IsInFlight() && !GetTransport())
            m_MirrorTimerFlags |= UNDERWATER_INDARKWATER;
        else
            m_MirrorTimerFlags &= ~UNDERWATER_INDARKWATER;

        // Lava state (any contact)
        if (liquidData->type_flags & MAP_LIQUID_TYPE_MAGMA)
        {
            if (status & MAP_LIQUID_STATUS_IN_CONTACT)
                m_MirrorTimerFlags |= UNDERWATER_INLAVA;
            else
                m_MirrorTimerFlags &= ~UNDERWATER_INLAVA;
        }

        // Slime state (any contact)
        if (liquidData->type_flags & MAP_LIQUID_TYPE_SLIME)
        {
            if (status & MAP_LIQUID_STATUS_IN_CONTACT)
                m_MirrorTimerFlags |= UNDERWATER_INSLIME;
            else
                m_MirrorTimerFlags &= ~UNDERWATER_INSLIME;
        }
    }
    else
        m_MirrorTimerFlags &= ~(UNDERWATER_INWATER | UNDERWATER_INLAVA | UNDERWATER_INSLIME | UNDERWATER_INDARKWATER);
}

void Player::AtExitCombat()
{
    Unit::AtExitCombat();
#ifdef LICH_KING
    UpdatePotionCooldown();
    
    if (GetClass() == CLASS_DEATH_KNIGHT)
        for (uint8 i = 0; i < MAX_RUNES; ++i)
        {
            SetRuneTimer(i, 0xFFFFFFFF);
            SetLastRuneGraceTimer(i, 0);
        }
#endif
}

void Player::SetCanParry( bool value )
{
    if(m_canParry == value)
        return;

    m_canParry = value;
    UpdateParryPercentage();
}

void Player::SetCanBlock( bool value )
{
    if(m_canBlock==value)
        return;

    m_canBlock = value;
    UpdateBlockPercentage();
}

bool ItemPosCount::isContainedIn(ItemPosCountVec const& vec) const
{
    for(auto itr : vec)
        if(itr.pos == pos)
            return true;
    return false;
}

//***********************************
//-------------TRINITY---------------
//***********************************

void Player::HandleFall(MovementInfo const& movementInfo)
{
    // calculate total z distance of the fall
    float z_diff = m_lastFallZ - movementInfo.pos.GetPositionZ();
    //TC_LOG_DEBUG("zDiff = %f", z_diff);

    //Players with low fall distance, Feather Fall or physical immunity (charges used) are ignored
    // 14.57 can be calculated by resolving damageperc formula below to 0
    if (z_diff >= 14.57f && !IsDead() && !IsGameMaster() &&
        !HasAuraType(SPELL_AURA_HOVER) && !HasAuraType(SPELL_AURA_FEATHER_FALL) &&
        !HasAuraType(SPELL_AURA_FLY) && !IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))
    {
        //Safe fall, fall height reduction
        int32 safe_fall = GetTotalAuraModifier(SPELL_AURA_SAFE_FALL);

        float damageperc = 0.018f*(z_diff-safe_fall)-0.2426f;

        if (damageperc > 0)
        {
            uint32 damage = (uint32)(damageperc * GetMaxHealth()*sWorld->GetRate(RATE_DAMAGE_FALL));

            if (GetCommandStatus(CHEAT_GOD))
                damage = 0;

            float height = movementInfo.pos.m_positionZ;
            UpdateGroundPositionZ(movementInfo.pos.m_positionX, movementInfo.pos.m_positionY, height);

            if (damage > 0)
            {
                //Prevent fall damage from being more than the player maximum health
                if (damage > GetMaxHealth())
                    damage = GetMaxHealth();

                // Gust of Wind
                if (HasAura(43621))
                    damage = GetMaxHealth()/2;

                EnvironmentalDamage(DAMAGE_FALL, damage);
            }

            //Z given by moveinfo, LastZ, FallTime, WaterZ, MapZ, Damage, Safefall reduction
            TC_LOG_DEBUG("entities.player", "FALLDAMAGE z=%f sz=%f pZ=%f FallTime=%d mZ=%f damage=%d SF=%d", movementInfo.pos.GetPositionZ(), height, GetPositionZ(), movementInfo.fallTime, height, damage, safe_fall);
        }
    }
}

void Player::StopCastingBindSight(Aura* except /* = nullptr*/)
{
    if(WorldObject* target = GetViewpoint())
    {
        if(target->isType(TYPEMASK_UNIT))
        {
            ((Unit*)target)->RemoveAurasByType(SPELL_AURA_BIND_SIGHT, GetGUID(), except);
            ((Unit*)target)->RemoveAurasByType(SPELL_AURA_MOD_POSSESS, GetGUID(), except);
            ((Unit*)target)->RemoveAurasByType(SPELL_AURA_MOD_POSSESS_PET, GetGUID(), except);
        }
    }
}

bool Player::CanUseBattlegroundObject(GameObject* gameobject)
{
    // It is possible to call this method with a null pointer, only skipping faction check.
    if (gameobject)
    {
        FactionTemplateEntry const* playerFaction = GetFactionTemplateEntry();
        FactionTemplateEntry const* faction = sFactionTemplateStore.LookupEntry(gameobject->GetFaction());

        if (playerFaction && faction && !playerFaction->IsFriendlyTo(*faction))
            return false;
    }

    return ( //InBattleground() &&                            // in battleground - not need, check in other cases
             //!IsMounted() &&                                  // not mounted
             !IsTotalImmune() &&                              // not totally immuned
             !HasStealthAura() &&                             // not stealthed
             !HasInvisibilityAura() &&                        // not invisible
             !HasAuraEffect(SPELL_RECENTLY_DROPPED_FLAG, 0) &&      // can't pickup
             IsAlive()                                        // live player
           );
}

bool Player::isAllowedToTakeBattlegroundBase()
{
    return ( !HasStealthAura() &&                             // not stealthed
             !HasInvisibilityAura() &&                        // not invisible
             IsAlive()                                        // live player
           );
}

bool Player::HasTitle(uint32 bitIndex) const
{
    if (bitIndex > 128)
        return false;

    uint32 fieldIndexOffset = bitIndex/32;
    uint32 flag = 1 << (bitIndex%32);
    return HasFlag(PLAYER_FIELD_KNOWN_TITLES+fieldIndexOffset, flag);
}

void Player::SetTitle(CharTitlesEntry const* title, bool notify, bool setCurrentTitle)
{
    uint32 fieldIndexOffset = title->bit_index/32;
    uint32 flag = 1 << (title->bit_index%32);
    SetFlag(PLAYER_FIELD_KNOWN_TITLES+fieldIndexOffset, flag);

    if(notify)
        GetSession()->SendTitleEarned(title->bit_index, true);

    if(setCurrentTitle)
        SetUInt32Value(PLAYER_CHOSEN_TITLE, title->bit_index);
}


void Player::RemoveTitle(CharTitlesEntry const* title, bool notify)
{
    uint32 fieldIndexOffset = title->bit_index/32;
    uint32 flag = 1 << (title->bit_index%32);
    RemoveFlag(PLAYER_FIELD_KNOWN_TITLES+fieldIndexOffset, flag);

    if(notify)
        GetSession()->SendTitleEarned(title->bit_index, false);

    if(GetUInt32Value(PLAYER_CHOSEN_TITLE) == title->bit_index)
        SetUInt32Value(PLAYER_CHOSEN_TITLE, 0);
}

bool Player::HasLevelInRangeForTeleport() const
{
    uint8 minLevel = sConfigMgr->GetIntDefault("Teleporter.MinLevel", 1);
    uint8 MaxLevel = sConfigMgr->GetIntDefault("Teleporter.MaxLevel", 255);

    return (GetLevel() >= minLevel && GetLevel() <= MaxLevel);
}

/*-----------------------TRINITY--------------------------*/
bool Player::IsTotalImmune()
{
    AuraEffectList const& immune = GetAuraEffectsByType(SPELL_AURA_SCHOOL_IMMUNITY);

    uint32 immuneMask = 0;
    for (AuraEffectList::const_iterator itr = immune.begin(); itr != immune.end(); ++itr)
    {
        immuneMask |= (*itr)->GetMiscValue();
        if (immuneMask & SPELL_SCHOOL_MASK_ALL)            // total immunity
            return true;
    }
    return false;
}

void Player::SetHomebind(WorldLocation const& loc, uint32 area_id)
{
    m_homebindMapId = loc.m_mapId;
    m_homebindAreaId = area_id;
    m_homebindX = loc.m_positionX;
    m_homebindY = loc.m_positionY;
    m_homebindZ = loc.m_positionZ;

    // update sql homebind
    CharacterDatabase.PExecute("UPDATE character_homebind SET map = '%u', zone = '%u', position_x = '%f', position_y = '%f', position_z = '%f' WHERE guid = '%u'",
        m_homebindMapId, m_homebindAreaId, m_homebindX, m_homebindY, m_homebindZ, GetGUID().GetCounter());
}

void Player::_LoadSkills(PreparedQueryResult result)
{
    //                                                           0      1      2
    // SetPQuery(PLAYER_LOGIN_QUERY_LOAD_SKILLS,          "SELECT skill, value, max FROM character_skills WHERE guid = '%u'", m_guid.GetCounter());

    uint32 count = 0;
    std::unordered_map<uint32, uint32> loadedSkillValues;
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 skill    = fields[0].GetUInt32();
            uint16 value    = fields[1].GetUInt32();
            uint16 max      = fields[2].GetUInt32();

            SkillRaceClassInfoEntry const* rcEntry = GetSkillRaceClassInfo(skill, GetRace(), GetClass());
            if(!rcEntry)
            {
                TC_LOG_ERROR("entities.player","Character %u has skill %u that does not exist, or is not compatible with it's race/class.", GetGUID().GetCounter(), skill);
                continue;
            }

            // set fixed skill ranges
            switch(GetSkillRangeType(rcEntry))
            {
                case SKILL_RANGE_LANGUAGE:                      // 300..300
                    value = max = 300;
                    break;
                case SKILL_RANGE_MONO:                          // 1..1, grey monolite bar
                    value = max = 1;
                    break;
                default:
                    break;
            }
            if(value == 0)
            {
                TC_LOG_ERROR("entities.player","Character %u has skill %u with value 0. Will be deleted.", GetGUID().GetCounter(), skill);
                CharacterDatabase.PExecute("DELETE FROM character_skills WHERE guid = '%u' AND skill = '%u' ", GetGUID().GetCounter(), skill );
                continue;
            }

            //step == profession tier ?
            uint16 skillStep = 0;
            if (SkillTiersEntry const* skillTier = sSkillTiersStore.LookupEntry(rcEntry->SkillTier))
            {
                for (uint32 i = 0; i < MAX_SKILL_STEP; ++i)
                {
                    if (skillTier->MaxSkill[skillStep] == max)
                    {
                        skillStep = i + 1;
                        break;
                    }
                }
            }

            SetUInt32Value(PLAYER_SKILL_INDEX(count), MAKE_PAIR32(skill, skillStep));

            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(count),MAKE_SKILL_VALUE(value, max));
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(count),0);

            mSkillStatus.insert(SkillStatusMap::value_type(skill, SkillStatusData(count, SKILL_UNCHANGED)));
            loadedSkillValues[skill] = value;

            ++count;

            if(count >= PLAYER_MAX_SKILLS)                      // client limit
            {
                TC_LOG_ERROR("entities.player","Character %u has more than %u skills.", GetGUID().GetCounter(), uint32(PLAYER_MAX_SKILLS));
                break;
            }
        } while (result->NextRow());
    }

    // Learn skill rewarded spells after all skills have been loaded to prevent learning a skill from them before its loaded with proper value from DB
    for (auto& skill : loadedSkillValues)
        LearnSkillRewardedSpells(skill.first, skill.second);

    //fill the rest with 0's
    for (; count < PLAYER_MAX_SKILLS; ++count)
    {
        SetUInt32Value(PLAYER_SKILL_INDEX(count), 0);
        SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(count),0);
        SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(count),0);
    }

    if (HasSkill(SKILL_FIST_WEAPONS))
        SetSkill(SKILL_FIST_WEAPONS, 0, GetSkillValue(SKILL_UNARMED), GetMaxSkillValueForLevel());
}

uint32 Player::GetTotalAccountPlayedTime()
{
    uint32 accountId = m_session->GetAccountId();

    QueryResult result = CharacterDatabase.PQuery("SELECT SUM(totaltime) FROM characters WHERE account = %u", accountId);

    if (!result)
        return 0;

    Field* fields = result->Fetch();

    uint32 totalPlayer = fields[0].GetUInt32();;

    return totalPlayer;
}

bool Player::IsMainFactionForRace(uint32 race, uint32 factionId)
{
    switch (race) {
    case RACE_HUMAN:
        return factionId ==  72;
    case RACE_ORC:
        return factionId == 76;
    case RACE_DWARF:
        return factionId == 47;
    case RACE_NIGHTELF:
        return factionId == 69;
    case RACE_UNDEAD_PLAYER:
        return factionId == 68;
    case RACE_TAUREN:
        return factionId == 81;
    case RACE_GNOME:
        return factionId == 54;
    case RACE_TROLL:
        return factionId == 530;
    case RACE_BLOODELF:
        return factionId == 911;
    case RACE_DRAENEI:
        return factionId == 930;
    default:
        break;
    }

    return false;
}

uint32 Player::GetMainFactionForRace(uint32 race)
{
    switch (race) {
    case RACE_HUMAN:
        return 72;
    case RACE_ORC:
        return 76;
    case RACE_DWARF:
        return 47;
    case RACE_NIGHTELF:
        return 69;
    case RACE_UNDEAD_PLAYER:
        return 68;
    case RACE_TAUREN:
        return 81;
    case RACE_GNOME:
        return 54;
    case RACE_TROLL:
        return 530;
    case RACE_BLOODELF:
        return 911;
    case RACE_DRAENEI:
        return 930;
    default:
        break;
    }

    return 0;
}

uint32 Player::GetNewFactionForRaceChange(uint32 oldRace, uint32 newRace, uint32 factionId)
{
    // Main faction case
    if (IsMainFactionForRace(oldRace, factionId))
        return GetMainFactionForRace(newRace);

    // Default case
    switch (factionId) {
    case 47:    // Ironforge
        return 530;
    case 54:    // Gnomeregan
    case 68:    // Undercity
    case 69:    // Darnassus
    case 72:    // Stormwind
    case 76:    // Orgrimmar
    case 81:    // Thunder Bluff
    case 530:   // Darkspear Trolls
    case 911:   // Silvermoon
    case 930:   // Exodar
    default:
        break;
    }

    return factionId;
}

void Player::UnsummonPetTemporaryIfAny()
{
    Pet* pet = GetPet();
    if (!pet)
        return;

    if (!m_temporaryUnsummonedPetNumber && pet->isControlled() && !pet->isTempSummoned())
    {
        m_temporaryUnsummonedPetNumber = pet->GetCharmInfo()->GetPetNumber();
        m_oldpetspell = pet->GetUInt32Value(UNIT_CREATED_BY_SPELL);
    }

    RemovePet(pet, PET_SAVE_AS_CURRENT);
}

void Player::setCommentator(bool on)
{
    if (on)
    {
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_COMMENTATOR | PLAYER_FLAGS_COMMENTATOR_UBER);

        WorldPacket data(SMSG_COMMENTATOR_STATE_CHANGED, 8 + 1);
        data << uint64(GetGUID());
        data << uint8(1);
        SendMessageToSet(&data, true);
    }
    else
    {
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_COMMENTATOR | PLAYER_FLAGS_COMMENTATOR_UBER);

        WorldPacket data(SMSG_COMMENTATOR_STATE_CHANGED, 8 + 1);
        data << uint64(GetGUID());
        data << uint8(0);
        SendMessageToSet(&data, true);
    }
}

void Player::SetSpectate(bool on)
{
    if (on)
    {
        SetSpeedRate(MOVE_RUN, 2.5);
        spectatorFlag = true;

        SetFaction(FACTION_FRIENDLY);

        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);

        CombatStopWithPets();
        if (Pet* pet = GetPet())
        {
            RemovePet(pet, PET_SAVE_AS_CURRENT);
        }
        SetTemporaryUnsummonedPetNumber(0);
        UnsummonPetTemporaryIfAny();
        RemoveAllControlled();

        ResetContestedPvP();

        SetDisplayId(10045);

        SetVisible(false);
    }
    else
    {
        SetFactionForRace(GetRace());

        if (spectateFrom)
            spectateFrom->RemovePlayerFromVision(this);

        // restore FFA PvP Server state
        if(sWorld->IsFFAPvPRealm())
            SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);

        // restore FFA PvP area state, remove not allowed for GM mounts
        UpdateArea(m_areaUpdateId);

        spectateCanceled = false;
        spectatorFlag = false;
        SetDisplayId(GetNativeDisplayId());
        UpdateSpeed(MOVE_RUN);

        if (!(m_ExtraFlags & PLAYER_EXTRA_GM_INVISIBLE)) //don't reset gm visibility
            SetVisible(true);
    }

    UpdateObjectVisibility();
}

bool Player::HaveSpectators() const
{
    if (Battleground *bg = GetBattleground())
    {
        if (bg->isSpectator(GetGUID()))
            return false;

        if (bg->IsArena())
        {
            if (bg->GetStatus() != STATUS_IN_PROGRESS)
                return false;

            return bg->HaveSpectators();
        }
    }

    return false;
}

void Player::SendDataForSpectator()
{
    Battleground *bGround = GetBattleground();
    if (!bGround)
        return;

    if (!bGround->isSpectator(GetGUID()))
        return;

    if (bGround->GetStatus() != STATUS_IN_PROGRESS)
        return;

    for (auto itr = bGround->GetPlayers().begin(); itr != bGround->GetPlayers().end(); ++itr)
        if (Player* tmpPlayer = ObjectAccessor::FindPlayer(itr->first))
        {
            if (bGround->isSpectator(tmpPlayer->GetGUID()))
                continue;

            uint32 tmpID = bGround->GetPlayerTeam(tmpPlayer->GetGUID());

            // generate addon massage
            std::string pName = tmpPlayer->GetName();
            std::string tName = "";

            if (Player *target = tmpPlayer->GetSelectedPlayer())
            {
                if (!bGround->isSpectator(target->GetGUID()))
                    tName = target->GetName();
            }

            SpectatorAddonMsg msg;
            msg.SetPlayer(pName);
            if (tName != "")
                msg.SetTarget(tName);
            msg.SetStatus(tmpPlayer->IsAlive());
            msg.SetClass(tmpPlayer->GetClass());
            msg.SetCurrentHP(tmpPlayer->GetHealth());
            msg.SetMaxHP(tmpPlayer->GetMaxHealth());
            Powers powerType = tmpPlayer->GetPowerType();
            msg.SetMaxPower(tmpPlayer->GetMaxPower(powerType));
            msg.SetCurrentPower(tmpPlayer->GetPower(powerType));
            msg.SetPowerType(powerType);
            msg.SetTeam(tmpID);
            msg.SendPacket(GetGUID());
        }
}

void Player::SendSpectatorAddonMsgToBG(SpectatorAddonMsg msg)
{
    if (!HaveSpectators())
        return;

    if (Battleground *bg = GetBattleground())
        bg->SendSpectateAddonsMsg(msg);
}

void Player::UpdateKnownPvPTitles()
{
    uint32 bit_index;
    uint32 honor_kills = GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);

    for (int i = HKRANK01; i != HKRANKMAX; ++i)
    {
        uint32 checkTitle = i;
        checkTitle += ((GetTeam() == ALLIANCE) ? (HKRANKMAX-1) : 0);
        if(CharTitlesEntry const* tEntry = sCharTitlesStore.LookupEntry(checkTitle))
        {
            bit_index = tEntry->bit_index;
            if (HasTitle(bit_index))
                RemoveTitle(tEntry);
        }

        if (honor_kills >= sWorld->pvp_ranks[i])
        {
            uint32 new_title = i;
            new_title += ((GetTeam() == ALLIANCE) ? 0 : (HKRANKMAX-1));

            if(CharTitlesEntry const* tEntry = sCharTitlesStore.LookupEntry(new_title))
            {
                bit_index = tEntry->bit_index;
                if (!HasTitle(bit_index))
                {
                    SetFlag64(PLAYER_FIELD_KNOWN_TITLES,uint64(1) << bit_index);

                    GetSession()->SendTitleEarned(bit_index, true);
                }
            }
        }
    }
}

void Player::addSpamReport(ObjectGuid reporterGUID, std::string message)
{
    // Add the new report
    time_t now = time(nullptr);
    SpamReport spr;
    spr.time = now;
    spr.reporterGUID = reporterGUID;
    spr.message = std::move(message);

    _spamReports[reporterGUID.GetCounter()] = spr;

    // Trash expired reports according to world config
    uint32 period = sWorld->getConfig(CONFIG_SPAM_REPORT_PERIOD);
    for (auto itr = _spamReports.begin(); itr != _spamReports.end(); ++itr) {
        if (itr->second.time < (now - period)) {
            _spamReports.erase(itr);
            itr = _spamReports.begin();
        }
    }

    // If we reported that spammer a little while ago, there's no need to do it again
    if (_lastSpamAlert > (now - sWorld->getConfig(CONFIG_SPAM_REPORT_COOLDOWN)))
        return;

    // Oooh, you little spammer!
    if (_spamReports.size() >= sWorld->getConfig(CONFIG_SPAM_REPORT_THRESHOLD)) {
        //sIRCMgr->onReportSpam(GetName(), GetGUID().GetCounter());
        _lastSpamAlert = now;
    }
}

void Player::RemoveAllCurrentPetAuras()
{
    if(Pet* pet = GetPet())
        pet->RemoveAllAuras();
     else
        CharacterDatabase.PQuery("DELETE FROM pet_aura WHERE guid IN ( SELECT id FROM character_pet WHERE owner = %u AND slot = %u )", GetGUID().GetCounter(), PET_SAVE_NOT_IN_SLOT);
}

void Player::GetBetaZoneCoord(uint32& map, float& x, float& y, float& z, float& o)
{
    bool set = false;
    QueryResult query = WorldDatabase.PQuery("SELECT position_x, position_y, position_z, orientation, map FROM game_tele WHERE name = 'BETA Landing Zone'");
    if (query) {
        Field* fields = query->Fetch();
        if (fields)
        {
            x = fields[0].GetFloat();
            y = fields[1].GetFloat();
            z = fields[2].GetFloat();
            o = fields[3].GetFloat();
            map = fields[4].GetUInt16();
            set = true;
        }
    }

    if (!set) //default values
    {
        TC_LOG_ERROR("entities.player", "GetBetaZoneCoord(...) : DB values not found, using default values");
        map = 0; x = -11785; y = -3171; z = -29; o = 3.7;
    }
}

void Player::GetArenaZoneCoord(bool secondary, uint32& map, float& x, float& y, float& z, float& o)
{
    bool set = false;
    QueryResult query = WorldDatabase.PQuery("SELECT position_x, position_y, position_z, orientation, map FROM game_tele WHERE name = \"%s\"", secondary ? "pvpzone2" : "pvpzone");
    if (query) {
        Field* fields = query->Fetch();
        if(fields)
        {
            x = fields[0].GetFloat();
            y = fields[1].GetFloat();
            z = fields[2].GetFloat();
            o = fields[3].GetFloat();
            map = fields[4].GetUInt16();
            set = true;
        }
    }

    if(!set) //default values
    {
        TC_LOG_ERROR("entities.player","GetArenaZoneCoord(...) : DB values not found, using default values");
        if(!secondary) { //hyjal area
           map = 1;x = 4717.020020;y = -1973.829956;z = 1087.079956;o = 0.068669;
        } else { //ZG Area
           map = 0;x = -12248.573242;y = -1679.274902;z = 130.267273;o = 3.024384;
        }
    }
}

void Player::RelocateToArenaZone(bool secondary)
{
    float x, y, z, o;
    uint32 mapid;
    GetArenaZoneCoord(secondary, mapid, x, y, z, o);
    SetFallInformation(0, z);
    Map* map = sMapMgr->CreateBaseMap(mapid);
    SetMap(map);
    Relocate(x, y, z, o);
}

void Player::RelocateToBetaZone()
{
    float x, y, z, o;
    uint32 map_id;
    GetBetaZoneCoord(map_id, x, y, z, o);
    Map* map = sMapMgr->CreateBaseMap(map_id);
    SetMap(map);
    Relocate(x, y, z, o);
}

void Player::TeleportToArenaZone(bool secondary)
{
    float x, y, z, o;
    uint32 map;
    GetArenaZoneCoord(secondary, map, x, y, z, o);
    TeleportTo(map, x, y, z, o, TELE_TO_GM_MODE);
}

void Player::TeleportToBetaZone()
{
    float x, y, z, o;
    uint32 map;
    GetBetaZoneCoord(map, x, y, z, o);
    TeleportTo(map, x, y, z, o, TELE_TO_GM_MODE);
}

/* true if the player threshold is reached and there is more player in the main zone than the secondary */
bool Player::ShouldGoToSecondaryArenaZone()
{
    uint32 onlinePlayers = sWorld->GetActiveSessionCount();
    uint32 repartitionTheshold = sWorld->getConfig(CONFIG_ARENASERVER_PLAYER_REPARTITION_THRESHOLD);

    if(repartitionTheshold && onlinePlayers > repartitionTheshold)
    {
        float x,y,z,o;
        uint32 mainMapId, secMapId;
        GetArenaZoneCoord(false, mainMapId, x, y, z, o);
        GetArenaZoneCoord(true, secMapId, x, y, z, o);
        Map* mainMap = sMapMgr->FindBaseMap(mainMapId);
        Map* secMap = sMapMgr->FindBaseMap(secMapId);
        if(mainMap && secMap && mainMap->GetPlayers().getSize() > secMap->GetPlayers().getSize())
                return true;
    }

    return false;
}

bool Player::IsInDuelArea() const
{
    return m_ExtraFlags & PLAYER_EXTRA_DUEL_AREA;
}

void Player::SetDifficulty(Difficulty dungeon_difficulty, bool sendToPlayer, bool asGroup)
{
    ASSERT(uint32(dungeon_difficulty) < MAX_DIFFICULTY);
    m_dungeonDifficulty = dungeon_difficulty;
    if (sendToPlayer)
        SendDungeonDifficulty(asGroup);
}

void Player::UpdateArenaTitleForRank(uint8 rank, bool add)
{
    CharTitlesEntry const* titleForRank = sWorld->getArenaLeaderTitle(rank);
    if(!titleForRank)
    {
        TC_LOG_ERROR("entities.player","UpdateArenaTitleForRank : No title for rank %u",rank);
        return;
    }

    if(add)
    {
        if(!HasTitle(titleForRank))
            SetTitle(titleForRank,true,true);
    } else {
        if(HasTitle(titleForRank))
            RemoveTitle(titleForRank);
    }
}

uint8 Player::GetGladiatorRank() const
{
    for(auto itr : sWorld->confGladiators)
    {
        uint32 myguid = GetGUID().GetCounter();
        if(itr.playerguid == myguid)
            return itr.rank;
    }
    return 0;
}

void Player::UpdateGladiatorTitle(uint8 rank)
{
    for(uint8 i = 1; i <= MAX_GLADIATORS_RANK; i++)
    {
        CharTitlesEntry const* titleForRank = sWorld->getGladiatorTitle(i);
        if(!titleForRank)
        {
            TC_LOG_ERROR("misc","UpdateGladiatorTitle : No title for rank %u",i);
            return;
        }
        if(i == rank)
        {
            if(!HasTitle(titleForRank))
                SetTitle(titleForRank,true,true);
        } else {
            if(HasTitle(titleForRank))
                RemoveTitle(titleForRank);
        }
    }
}

void Player::UpdateArenaTitles()
{
    //update gladiator titles first
    UpdateGladiatorTitle(GetGladiatorRank());

    //if interseason, leaders are defined in conf file
    if(sWorld->getConfig(CONFIG_ARENA_SEASON) == 0)
    {
        uint32 guid = GetGUID().GetCounter();
        for(uint8 rank = 1; rank <= 3; rank++)
        {
            bool add = false;
            //check if present in bracket
            for(uint8 i = (rank-1)*4; i < rank*4; i++) // 0-3 4-7 8-11
            {
                add = (guid == sWorld->confStaticLeaders[i]);
                if(add == true) break; //found in current range and title added, we're done here
            }
            UpdateArenaTitleForRank(rank,add);
        }
        return;
    }

    //else, normal case :
    CharacterCacheEntry const* playerData = sCharacterCache->GetCharacterCacheByGuid(GetGUID().GetCounter());
    if (!playerData)
        return;

    uint32 teamid = playerData->arenaTeamId[0];
    std::vector<ArenaTeam*> firstTeams = sWorld->getArenaLeaderTeams();

    bool hasRank[3];
    for(bool & i : hasRank)
        i = false;

    for(auto itr : firstTeams)
    {
        if(itr == nullptr)
            continue;

        uint8 rank = itr->GetRank();
        if(rank > 3)
        {
            TC_LOG_ERROR("misc","UpdateArenaTitles() : found a team with rank > 3, skipping");
            continue;
        }
        if(hasRank[rank-1]) //we already found a suitable team for this rank, don't erase it
            continue;

        bool sameTeam = itr->GetId() == teamid;
        bool closeRating = false;

        ArenaTeam * at = sArenaTeamMgr->GetArenaTeamById(teamid);
        if(at)
            if(ArenaTeamMember* member = at->GetMember(GetGUID()))
                closeRating = ((int)at->GetStats().Rating - (int)member->PersonalRating) < 100; //no more than 100 under the team rating

        hasRank[rank-1] = sameTeam && closeRating;
    }

    //Real title update
    for(uint8 i = 1; i <= 3; i++)
        UpdateArenaTitleForRank(i,hasRank[i-1]);

    /*
    // Rare case but if there is less than 3 first teams, still need to remove remaining titles
    for(uint8 i = rank; i <= 3; i++)
        UpdateArenaTitleForRank(i,false);
    */
}

void Player::SetFlying(bool apply)
{
    GetSession()->anticheat.OnPlayerSetFlying(this, apply);

    Unit::SetFlying(apply);
}

void Player::ProcessDelayedOperations()
{
    if (m_DelayedOperations == 0)
        return;

    if (m_DelayedOperations & DELAYED_RESURRECT_PLAYER)
    {
        ResurrectPlayer(0.0f, false);

        SetHealth(m_resurrectHealth);
        SetPower(POWER_MANA, m_resurrectMana);

        SetPower(POWER_RAGE, 0);
        SetFullPower(POWER_ENERGY);

        SpawnCorpseBones();
    }

    if (m_DelayedOperations & DELAYED_SAVE_PLAYER)
        SaveToDB();

    if (m_DelayedOperations & DELAYED_SPELL_CAST_DESERTER)
        CastSpell(this, 26013, TRIGGERED_FULL_MASK);               // Deserter

    if (m_DelayedOperations & DELAYED_BG_MOUNT_RESTORE)
    {
        if (m_bgData.mountSpell)
        {
            CastSpell(this, m_bgData.mountSpell, true);
            m_bgData.mountSpell = 0;
        }
    }

    if (m_DelayedOperations & DELAYED_BG_TAXI_RESTORE)
    {
        if (m_bgData.HasTaxiPath())
        {
            m_taxi.AddTaxiDestination(m_bgData.taxiPath[0]);
            m_taxi.AddTaxiDestination(m_bgData.taxiPath[1]);
            m_bgData.ClearTaxiPath();

            ContinueTaxiFlight();
        }
    }

    if (m_DelayedOperations & DELAYED_BG_GROUP_RESTORE)
    {
        if (Group* g = GetGroup())
            g->SendUpdateToPlayer(GetGUID());
    }

    //we have executed ALL delayed ops, so clear the flag
    m_DelayedOperations = 0;
}

void Player::UpdateFallInformationIfNeed(MovementInfo const& minfo, uint16 opcode)
{
    if (m_lastFallTime >= minfo.fallTime || m_lastFallZ <= minfo.pos.GetPositionZ() || opcode == MSG_MOVE_FALL_LAND)
        SetFallInformation(minfo.fallTime, minfo.pos.GetPositionZ());
}

void Player::SetFallInformation(uint32 time, float z)
{
    m_lastFallTime = time;
    m_lastFallZ = z;
}

void Player::SetViewpoint(WorldObject* target, bool apply)
{
    if (apply)
    {
        TC_LOG_TRACE("entities.player", "Player::SetViewpoint: Player '%s' (%s) creates seer (Entry: %u, TypeId: %u).",
            GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str(), target->GetEntry(), target->GetTypeId());

        if (!AddGuidValue(PLAYER_FARSIGHT, target->GetGUID()))
        {
            TC_LOG_FATAL("entities.player", "Player::SetViewpoint: Player '%s' (%s) cannot add new viewpoint!", GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str());
            return;
        }

        // farsight dynobj or puppet may be very far away
        UpdateVisibilityOf(target);

        if (target->isType(TYPEMASK_UNIT)
#ifdef LICH_KING
            && target != GetVehicleBase()
#endif
            )
            static_cast<Unit*>(target)->AddPlayerToVision(this);

        SetSeer(target);
    }
    else
    {
        TC_LOG_TRACE("entities.player", "Player::SetViewpoint: Player %s removed seer", GetName().c_str());

        if (!RemoveGuidValue(PLAYER_FARSIGHT, target->GetGUID()))
        {
            TC_LOG_FATAL("entities.player", "Player::SetViewpoint: Player '%s' (%s) cannot remove current viewpoint!", GetName().c_str(), ObjectGuid(GetGUID()).ToString().c_str());
            return;
        }

        if (target->isType(TYPEMASK_UNIT)
#ifdef LICH_KING
            && target != GetVehicleBase()
#endif
            )
            static_cast<Unit*>(target)->RemovePlayerFromVision(this);

        //must immediately set seer back otherwise may crash
        SetSeer(this);

        //WorldPacket data(SMSG_CLEAR_FAR_SIGHT_IMMEDIATE, 0);
        //GetSession()->SendPacket(&data);
    }

#ifdef LICH_KING
    // HACK: Make sure update for PLAYER_FARSIGHT is received before SMSG_PET_SPELLS to properly hide "Release spirit" dialog
    if (target->GetTypeId() == TYPEID_UNIT && static_cast<Unit*>(target)->HasUnitTypeMask(UNIT_MASK_MINION) && static_cast<Minion*>(target)->IsRisenAlly())
    {
        if (apply)
        {
            UpdateDataMapType update_players;
            UpdatePlayerSet player_set; //only there for performance, avoid recreating it at each BuildUpdate call
            BuildUpdate(update_players, player_set);
            WorldPacket packet;
            for (UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
            {
                iter->second.BuildPacket(&packet);
                iter->first->GetSession()->SendPacket(&packet);
                packet.clear();
            }
        }
        else
        {
            m_deathTimer = 6 * MINUTE * IN_MILLISECONDS;

            // Reset "Release spirit" timer clientside
            WorldPacket data(SMSG_FORCED_DEATH_UPDATE);
            SendDirectMessage(&data);
        }
    }
#endif
}

WorldObject* Player::GetViewpoint() const
{
    if (ObjectGuid guid = GetGuidValue(PLAYER_FARSIGHT))
        return static_cast<WorldObject*>(ObjectAccessor::GetObjectByTypeMask(*this, guid, TYPEMASK_SEER));
    return nullptr;
}

void Player::ResummonPetTemporaryUnSummonedIfAny()
{
    if (!m_temporaryUnsummonedPetNumber)
        return;

    // not resummon in not appropriate state
    if (IsPetNeedBeTemporaryUnsummoned())
        return;

    if (GetMinionGUID())
        return;

    Pet* newPet = new Pet(this);
    if (!newPet->LoadPetFromDB(this, 0, m_temporaryUnsummonedPetNumber, true))
        delete newPet;

    m_temporaryUnsummonedPetNumber = 0;
}

bool Player::IsPetNeedBeTemporaryUnsummoned() const
{
    return !IsInWorld() || !IsAlive() || IsMounted() /*+in flight*/;
}

/*********************************************************/
/***                    GOSSIP SYSTEM                  ***/
/*********************************************************/

void Player::PrepareGossipMenu(WorldObject* source, uint32 menuId /*= 0*/, bool showQuests /*= false*/)
{
    PlayerMenu* menu = PlayerTalkClass;
    menu->ClearMenus();

    menu->GetGossipMenu().SetMenuId(menuId);

    GossipMenuItemsMapBounds menuItemBounds = sObjectMgr->GetGossipMenuItemsMapBounds(menuId);

    // if default menuId and no menu options exist for this, use options from default options
    if (menuItemBounds.first == menuItemBounds.second && menuId == GetDefaultGossipMenuForSource(source))
        menuItemBounds = sObjectMgr->GetGossipMenuItemsMapBounds(GENERIC_OPTIONS_MENU);

    uint32 npcflags = 0;

    if (source->GetTypeId() == TYPEID_UNIT)
    {
        npcflags = source->GetUInt32Value(UNIT_NPC_FLAGS);
        if (showQuests && npcflags & UNIT_NPC_FLAG_QUESTGIVER)
            PrepareQuestMenu(source->GetGUID());
    }
    else if (source->GetTypeId() == TYPEID_GAMEOBJECT)
        if (showQuests && source->ToGameObject()->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
            PrepareQuestMenu(source->GetGUID());

    for (auto itr = menuItemBounds.first; itr != menuItemBounds.second; ++itr)
    {
        bool canTalk = true;
        if (!sConditionMgr->IsObjectMeetToConditions(this, source, itr->second.Conditions))
            continue;

        if (Creature* creature = source->ToCreature())
        {
            if (!(itr->second.OptionNpcflag & npcflags))
                continue;

            switch (itr->second.OptionType)
            {
                case GOSSIP_OPTION_ARMORER:
                    canTalk = false;                       // added in special mode
                    break;
                case GOSSIP_OPTION_SPIRITHEALER:
                    if (!IsDead())
                        canTalk = false;
                    break;
                case GOSSIP_OPTION_VENDOR:
                {
                    VendorItemData const* vendorItems = creature->GetVendorItems();
                    if (!vendorItems || vendorItems->Empty())
                    {
                        TC_LOG_ERROR("sql.sql", "Creature %s (Entry: %u GUID: %u DB GUID: %u) has UNIT_NPC_FLAG_VENDOR set but has an empty trading item list.", creature->GetName().c_str(), creature->GetEntry(), creature->GetGUID().GetCounter(), creature->GetSpawnId());
                        canTalk = false;
                    }
                    break;
                }
#ifdef LICH_KING
                case GOSSIP_OPTION_LEARNDUALSPEC:
                    if (!(GetSpecsCount() == 1 && creature->isCanTrainingAndResetTalentsOf(this) && !(GetLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))))
                        canTalk = false;
                    break;
#endif
                case GOSSIP_OPTION_UNLEARNTALENTS:
                    if (!creature->canResetTalentsOf(this))
                        canTalk = false;
                    break;
                case GOSSIP_OPTION_UNLEARNPETTALENTS:
                    if (!GetPet() || GetPet()->getPetType() != HUNTER_PET || GetPet()->m_spells.size() <= 1 || creature->GetCreatureTemplate()->trainer_type != TRAINER_TYPE_PETS || creature->GetCreatureTemplate()->trainer_class != CLASS_HUNTER)
                        canTalk = false;
                    break;
                case GOSSIP_OPTION_TAXIVENDOR:
                    if (GetSession()->SendLearnNewTaxiNode(creature))
                        return;
                    break;
                case GOSSIP_OPTION_BATTLEFIELD:
                    if (!creature->isCanInteractWithBattleMaster(this, false))
                        canTalk = false;
                    break;
                case GOSSIP_OPTION_STABLEPET:
                    if (GetClass() != CLASS_HUNTER)
                        canTalk = false;
                    break;
                case GOSSIP_OPTION_QUESTGIVER:
                    canTalk = false;
                    break;
                case GOSSIP_OPTION_TRAINER:
                    if (GetClass() != creature->GetCreatureTemplate()->trainer_class && creature->GetCreatureTemplate()->trainer_type == TRAINER_TYPE_CLASS)
                        TC_LOG_ERROR("sql.sql", "GOSSIP_OPTION_TRAINER:: Player %s (GUID: %u) request wrong gossip menu: %u with wrong class: %u at Creature: %s (Entry: %u, Trainer Class: %u)",
                            GetName().c_str(), GetGUID().GetCounter(), menu->GetGossipMenu().GetMenuId(), GetClass(), creature->GetName().c_str(), creature->GetEntry(), creature->GetCreatureTemplate()->trainer_class);

                    [[fallthrough]];
                case GOSSIP_OPTION_GOSSIP:
                case GOSSIP_OPTION_SPIRITGUIDE:
                case GOSSIP_OPTION_INNKEEPER:
                case GOSSIP_OPTION_BANKER:
                case GOSSIP_OPTION_PETITIONER:
                case GOSSIP_OPTION_TABARDDESIGNER:
                case GOSSIP_OPTION_AUCTIONEER:
                    break;                                  // no checks
                case GOSSIP_OPTION_OUTDOORPVP:
                    if (!sOutdoorPvPMgr->CanTalkTo(this, creature, itr->second))
                        canTalk = false;
                    break;
                default:
                    TC_LOG_ERROR("sql.sql", "Creature entry %u has unknown gossip option %u for menu %u", creature->GetEntry(), itr->second.OptionType, itr->second.MenuId);
                    canTalk = false;
                    break;
            }
        }
        else if (GameObject* go = source->ToGameObject())
        {
            switch (itr->second.OptionType)
            {
                case GOSSIP_OPTION_GOSSIP:
                    if (go->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER && go->GetGoType() != GAMEOBJECT_TYPE_GOOBER)
                        canTalk = false;
                    break;
                default:
                    canTalk = false;
                    break;
            }
        }

        if (canTalk)
        {
            std::string strOptionText, strBoxText;

            BroadcastText const* optionBroadcastText = sObjectMgr->GetBroadcastText(itr->second.OptionBroadcastTextId);
            BroadcastText const* boxBroadcastText = sObjectMgr->GetBroadcastText(itr->second.BoxBroadcastTextId);

            LocaleConstant locale = GetSession()->GetSessionDbLocaleIndex();

            if (optionBroadcastText)
                strOptionText = optionBroadcastText->GetText(locale, GetGender());
            else
                strOptionText = itr->second.OptionText;

            if (boxBroadcastText)
                strBoxText = boxBroadcastText->GetText(locale, GetGender());
            else
                strBoxText = itr->second.BoxText;

            if (locale != DEFAULT_LOCALE)
            {
                if (!optionBroadcastText)
                {
                    /// Find localizations from database.
                    if (GossipMenuItemsLocale const* gossipMenuLocale = sObjectMgr->GetGossipMenuItemsLocale(menuId, itr->second.OptionIndex))
                        ObjectMgr::GetLocaleString(gossipMenuLocale->OptionText, locale, strOptionText);
                }

                if (!boxBroadcastText)
                {
                    /// Find localizations from database.
                    if (GossipMenuItemsLocale const* gossipMenuLocale = sObjectMgr->GetGossipMenuItemsLocale(menuId, itr->second.OptionIndex))
                        ObjectMgr::GetLocaleString(gossipMenuLocale->BoxText, locale, strBoxText);
                }
            }

            menu->GetGossipMenu().AddMenuItem(itr->second.OptionIndex, itr->second.OptionIcon, strOptionText, 0, itr->second.OptionType, strBoxText, itr->second.BoxMoney, itr->second.BoxCoded);
            menu->GetGossipMenu().AddGossipMenuItemData(itr->second.OptionIndex, itr->second.ActionMenuId, itr->second.ActionPoiId);
        }
    }
}

void Player::SendPreparedGossip(WorldObject* source)
{
    if (!source)
        return;

    if (source->GetTypeId() == TYPEID_UNIT)
    {
        // in case no gossip flag and quest menu not empty, open quest menu (client expect gossip menu with this flag)
        if (!source->ToCreature()->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP) && !PlayerTalkClass->GetQuestMenu().Empty())
        {
            SendPreparedQuest(source->GetGUID());
            return;
        }
    }
    else if (source->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        // probably need to find a better way here
        if (!PlayerTalkClass->GetGossipMenu().GetMenuId() && !PlayerTalkClass->GetQuestMenu().Empty())
        {
            SendPreparedQuest(source->GetGUID());
            return;
        }
    }

    // in case non empty gossip menu (that not included quests list size) show it
    // (quest entries from quest menu will be included in list)

    uint32 textId = GetGossipTextId(source);

    if (uint32 menuId = PlayerTalkClass->GetGossipMenu().GetMenuId())
        textId = GetGossipTextId(menuId, source);

    PlayerTalkClass->SendGossipMenuTextID(textId, source->GetGUID());
}

void Player::OnGossipSelect(WorldObject* source, uint32 gossipListId, uint32 menuId)
{
    GossipMenu& gossipMenu = PlayerTalkClass->GetGossipMenu();

    // if not same, then something funky is going on
    if (menuId != gossipMenu.GetMenuId())
        return;

    GossipMenuItem const* item = gossipMenu.GetItem(gossipListId);
    if (!item)
        return;

    uint32 gossipOptionId = item->OptionType;
    ObjectGuid guid = source->GetGUID();

    if (source->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        if (gossipOptionId > GOSSIP_OPTION_QUESTGIVER)
        {
            TC_LOG_ERROR("entities.player", "Player guid %u request invalid gossip option for GameObject entry %u", GetGUID().GetCounter(), source->GetEntry());
            return;
        }
    }

    GossipMenuItemData const* menuItemData = gossipMenu.GetItemData(gossipListId);
    if (!menuItemData)
        return;

    int32 cost = int32(item->BoxMoney);
    if (!HasEnoughMoney(cost))
    {
        SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, nullptr, 0, 0);
        PlayerTalkClass->SendCloseGossip();
        return;
    }

    switch (gossipOptionId)
    {
        case GOSSIP_OPTION_GOSSIP:
        {
            if (menuItemData->GossipActionPoi)
                PlayerTalkClass->SendPointOfInterest(menuItemData->GossipActionPoi);

            if (menuItemData->GossipActionMenuId)
            {
                PrepareGossipMenu(source, menuItemData->GossipActionMenuId);
                SendPreparedGossip(source);
            }

            break;
        }
        case GOSSIP_OPTION_OUTDOORPVP:
            sOutdoorPvPMgr->HandleGossipOption(this, source->GetGUID(), gossipListId);
            break;
        case GOSSIP_OPTION_SPIRITHEALER:
            if (IsDead())
                source->ToCreature()->CastSpell(source, 17251, GetGUID());
            break;
        case GOSSIP_OPTION_QUESTGIVER:
            PrepareQuestMenu(guid);
            SendPreparedQuest(guid);
            break;
        case GOSSIP_OPTION_VENDOR:
        case GOSSIP_OPTION_ARMORER:
            GetSession()->SendListInventory(guid);
            break;
        case GOSSIP_OPTION_STABLEPET:
            GetSession()->SendStablePet(guid);
            break;
        case GOSSIP_OPTION_TRAINER:
            GetSession()->SendTrainerList(guid);
            break;
       /* case GOSSIP_OPTION_LEARNDUALSPEC:
            if (GetSpecsCount() == 1 && getLevel() >= sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))
            {
                // Cast spells that teach dual spec
                // Both are also ImplicitTarget self and must be cast by player
                CastSpell(this, 63680, TRIGGERED_FULL_MASK, NULL, NULL, GetGUID());
                CastSpell(this, 63624, TRIGGERED_FULL_MASK, NULL, NULL, GetGUID());

                // Should show another Gossip text with "Congratulations..."
                PlayerTalkClass->SendCloseGossip();
            }
            break;
           */
        case GOSSIP_OPTION_UNLEARNTALENTS:
            PlayerTalkClass->SendCloseGossip();
            SendTalentWipeConfirm(guid);
            break;
        case GOSSIP_OPTION_UNLEARNPETTALENTS:
            PlayerTalkClass->SendCloseGossip();
            SendPetSkillWipeConfirm();
            break;
        case GOSSIP_OPTION_TAXIVENDOR:
            GetSession()->SendTaxiMenu(source->ToCreature());
            break;
        case GOSSIP_OPTION_INNKEEPER:
            PlayerTalkClass->SendCloseGossip();
            SetBindPoint(guid);
            break;
        case GOSSIP_OPTION_BANKER:
            GetSession()->SendShowBank(guid);
            break;
        case GOSSIP_OPTION_PETITIONER:
            PlayerTalkClass->SendCloseGossip();
            GetSession()->SendPetitionShowList(guid);
            break;
        case GOSSIP_OPTION_TABARDDESIGNER:
            PlayerTalkClass->SendCloseGossip();
            GetSession()->SendTabardVendorActivate(guid);
            break;
        case GOSSIP_OPTION_AUCTIONEER:
            GetSession()->SendAuctionHello(guid, source->ToCreature());
            break;
        case GOSSIP_OPTION_SPIRITGUIDE:
            PrepareGossipMenu(source);
            SendPreparedGossip(source);
            break;
        case GOSSIP_OPTION_BATTLEFIELD:
        {
            BattlegroundTypeId bgTypeId = sObjectMgr->GetBattleMasterBG(source->GetEntry());

            if (bgTypeId == BATTLEGROUND_TYPE_NONE)
            {
                TC_LOG_ERROR("entities.player", "a user (guid %u) requested battlegroundlist from a npc who is no battlemaster", GetGUID().GetCounter());
                return;
            }

            GetSession()->SendBattleGroundList(guid, bgTypeId);
            break;
        }
    }

    ModifyMoney(-cost);
}

uint32 Player::GetGossipTextId(WorldObject* source) const
{
    if (!source)
        return DEFAULT_GOSSIP_MESSAGE;

    return GetGossipTextId(GetDefaultGossipMenuForSource(source), source);
}

uint32 Player::GetGossipTextId(uint32 menuId, WorldObject* source) const
{
    uint32 textId = DEFAULT_GOSSIP_MESSAGE;

    if (!menuId)
        return textId;

    GossipMenusMapBounds menuBounds = sObjectMgr->GetGossipMenusMapBounds(menuId);

    for (auto itr = menuBounds.first; itr != menuBounds.second; ++itr)
    {
        if (sConditionMgr->IsObjectMeetToConditions(const_cast<Player*>(this), source, itr->second.conditions))
            textId = itr->second.text_id;
    }

    return textId;
}

uint32 Player::GetDefaultGossipMenuForSource(WorldObject* source)
{
    switch (source->GetTypeId())
    {
    case TYPEID_UNIT:
        // return menu for this particular guid if any
        if (uint32 menuId = sObjectMgr->GetNpcGossipMenu(source->ToCreature()->GetSpawnId()))
            return menuId;

        // else return menu from creature template
        return source->ToCreature()->GetCreatureTemplate()->GossipMenuId;
    case TYPEID_GAMEOBJECT:
        return source->ToGameObject()->GetGOInfo()->GetGossipMenuId();
    default:
        break;
    }

    return 0;
}

uint32 Player::GetGuildIdFromCharacterInfo(ObjectGuid::LowType guid)
{
    if (CharacterCacheEntry const* playerData = sCharacterCache->GetCharacterCacheByGuid(guid))
        return playerData->guildId;
    return 0;
}

void Player::SetInArenaTeam(uint32 ArenaTeamId, uint8 slot, uint8 type)
{
#ifdef LICH_KING
    TODO
#else
    SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (slot * 6), ArenaTeamId);
#endif
}

uint32 Player::GetArenaTeamIdFromCharacterInfo(ObjectGuid guid, uint8 type)
{
    CharacterCacheEntry const* characterInfo = sCharacterCache->GetCharacterCacheByGuid(guid);
    if (!characterInfo)
        return 0;

    return characterInfo->arenaTeamId[type];
}

void Player::SetInGuild(uint32 guildId)
{
    SetUInt32Value(PLAYER_GUILDID, guildId);
    sCharacterCache->UpdateCharacterGuildId(GetGUID(), guildId);
}

void Player::SetRank(uint32 rankId)
{
    SetUInt32Value(PLAYER_GUILDRANK, rankId);
}

void Player::SetGuildIdInvited(uint32 GuildId)
{
    _guildIdInvited = GuildId;
}

uint32 Player::GetGuildId() const
{
    return GetUInt32Value(PLAYER_GUILDID);
}

uint32 Player::GetRank() const
{
    return GetUInt32Value(PLAYER_GUILDRANK);
}

void Player::InitTaxiNodesForLevel() 
{ 
    m_taxi.InitTaxiNodesForLevel(GetRace(), GetLevel()); 
}

bool Player::CanNoReagentCast(SpellInfo const* spellInfo) const
{
    // don't take reagents for spells with SPELL_ATTR5_NO_REAGENT_WHILE_PREP
    if (spellInfo->HasAttribute(SPELL_ATTR5_NO_REAGENT_WHILE_PREP) &&
        HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION))
        return true;

#ifdef LICH_KING
    // Check no reagent use mask
    flag96 noReagentMask;
    noReagentMask[0] = GetUInt32Value(PLAYER_NO_REAGENT_COST_1);
    noReagentMask[1] = GetUInt32Value(PLAYER_NO_REAGENT_COST_1 + 1);
    noReagentMask[2] = GetUInt32Value(PLAYER_NO_REAGENT_COST_1 + 2);
    if (spellInfo->SpellFamilyFlags & noReagentMask)
        return true;
#endif

    return false;
}

#ifdef TESTS
PlayerbotTestingAI* Player::GetTestingPlayerbotAI() const
{
    return dynamic_cast<PlayerbotTestingAI*>(m_playerbotAI);
}
#endif

bool Player::IsTestingBot() const
{
#ifdef TESTS
    return GetTestingPlayerbotAI();
#else
    return false;
#endif
}

void Player::SaveSafePosition(Position pos)
{
    _lastSafePosition = pos;
}

bool Player::UndermapRecall()
{
    if (!_lastSafePosition.is_initialized() || IsBeingTeleported())
        return false;

    if (GetDistance2d(_lastSafePosition->GetPositionX(), _lastSafePosition->GetPositionY()) > 50.0f)
    {
        _lastSafePosition.reset();
        return false;
    }

    NearTeleportTo(*_lastSafePosition, TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET);
    _lastSafePosition.reset();
    return true;
}
