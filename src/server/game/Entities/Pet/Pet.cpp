#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Pet.h"
#include "MapManager.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "CreatureAI.h"
#include "Unit.h"
#include "Util.h"
#include "Creature.h"
#include "PetDefines.h"
#include "SpellHistory.h"

#define SPELL_WATER_ELEMENTAL_WATERBOLT 31707
#define SPELL_PET_RECENTLY_DISMISSED 47531

char const* petTypeSuffix[MAX_PET_TYPE] =
{
    "'s Minion",                                            // SUMMON_PET
    "'s Pet",                                               // HUNTER_PET
    "'s Guardian",                                          // GUARDIAN_PET
    "'s Companion"                                          // MINI_PET
};

//numbers represent minutes * 100 while happy (you get 100 loyalty points per min while happy)
uint32 const LevelUpLoyalty[6] =
{
    5500,
    11500,
    17000,
    23500,
    31000,
    39500,
};

uint32 const LevelStartLoyalty[6] =
{
    2000,
    4500,
    7000,
    10000,
    13500,
    17500,
};

Pet::Pet(Player* owner, PetType type) 
    : Guardian(NULL, owner, true),
    m_removed(false),
    m_regenFocusTimer(4000),
    m_loading(false)
{
    m_unitTypeMask |= UNIT_MASK_PET;
    if (type == HUNTER_PET)
        m_unitTypeMask |= UNIT_MASK_HUNTER_PET;

    m_name = "Pet";
    m_petType = type;

    m_happinessTimer = 7500;
    m_loyaltyTimer = 12000;
    m_duration = 0;
    m_bonusdamage = 0;

    m_loyaltyPoints = 0;
    m_TrainingPoints = 0;
    m_resetTalentsCost = 0;
    m_resetTalentsTime = 0;

    m_auraUpdateMask = 0;

    // pets always have a charminfo, even if they are not actually charmed
    if (!(m_unitTypeMask & UNIT_MASK_CONTROLABLE_GUARDIAN))
    {
        m_unitTypeMask |= UNIT_MASK_CONTROLABLE_GUARDIAN;
        InitCharmInfo();
    }

    if(type == MINI_PET || type == POSSESSED_PET)                                    // always passive
        SetReactState(REACT_PASSIVE);
    else if(type == GUARDIAN_PET)                           // always aggressive
        SetReactState(REACT_AGGRESSIVE);

    m_spells.clear();
    m_autospells.clear();
    m_declinedname = nullptr;
    //m_isActive = true;
}

Pet::~Pet()
{
    delete m_declinedname;
}

void Pet::AddToWorld()
{
    ///- Register the pet for guid lookup
    if(!IsInWorld())
    {   
        GetMap()->GetObjectsStore().Insert<Pet>(GetGUID(), this);
        Unit::AddToWorld();
        AIM_Initialize();
    }

    // Prevent stuck pets when zoning. Pets default to "follow" when added to world
    // so we'll reset flags and let the AI handle things
    if (GetCharmInfo() && GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        GetCharmInfo()->SetIsCommandAttack(false);
        GetCharmInfo()->SetIsCommandFollow(false);
        GetCharmInfo()->SetIsAtStay(false);
        GetCharmInfo()->SetIsFollowing(false);
        GetCharmInfo()->SetIsReturning(false);
    }
}

void Pet::RemoveFromWorld()
{
    ///- Remove the pet from the accessor
    if(IsInWorld())
    {
        GetMap()->GetObjectsStore().Remove<Pet>(GetGUID());
        ///- Don't call the function for Creature, normal mobs + totems go in a different storage
        Unit::RemoveFromWorld();
    }
}

bool Pet::LoadPetFromDB(Player* owner, uint32 petentry, uint32 petnumber, bool current )
{
    m_loading = true;

    uint32 ownerid = owner->GetGUID().GetCounter();
    Unit* target = nullptr;

    QueryResult result;

    if(petnumber)
        // known petnumber entry                  0   1      2      3        4      5    6           7              8        9           10    11    12       13         14       15            16      17              18        19                 20                 21              22
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, loyaltypoints, loyalty, trainpoint, slot, name, renamed, curhealth, curmana, curhappiness, abdata, TeachSpelldata, savetime, resettalents_cost, resettalents_time, CreatedBySpell, PetType FROM character_pet WHERE owner = '%u' AND id = '%u'",ownerid, petnumber);
    else if(current)
        // current pet (slot 0)                   0   1      2      3        4      5    6           7              8        9           10    11    12       13         14       15            16      17              18        19                 20                 21              22
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, loyaltypoints, loyalty, trainpoint, slot, name, renamed, curhealth, curmana, curhappiness, abdata, TeachSpelldata, savetime, resettalents_cost, resettalents_time, CreatedBySpell, PetType FROM character_pet WHERE owner = '%u' AND slot = '%u'", ownerid, PET_SAVE_AS_CURRENT);
    else if(petentry)
        // known petentry entry (unique for summoned pet, but non unique for hunter pet (only from current or not stabled pets)
        //                                        0   1      2      3        4      5    6           7              8        9           10    11    12       13         14       15            16      17              18        19                 20                 21              22
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, loyaltypoints, loyalty, trainpoint, slot, name, renamed, curhealth, curmana, curhappiness, abdata, TeachSpelldata, savetime, resettalents_cost, resettalents_time, CreatedBySpell, PetType FROM character_pet WHERE owner = '%u' AND entry = '%u' AND (slot = '%u' OR slot > '%u') ", ownerid, petentry, PET_SAVE_AS_CURRENT, PET_SAVE_LAST_STABLE_SLOT );
    else
        // any current or other non-stabled pet (for hunter "call pet")
        //                                        0   1      2      3        4      5    6           7              8        9           10    11    12       13         14       15            16      17              18        19                 20                 21              22
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, loyaltypoints, loyalty, trainpoint, slot, name, renamed, curhealth, curmana, curhappiness, abdata, TeachSpelldata, savetime, resettalents_cost, resettalents_time, CreatedBySpell, PetType FROM character_pet WHERE owner = '%u' AND (slot = '%u' OR slot > '%u') ",ownerid, PET_SAVE_AS_CURRENT, PET_SAVE_LAST_STABLE_SLOT);

    if (!result)
    {
        m_loading = false;
        return false;
    }

    Field *fields = result->Fetch();

    // update for case of current pet "slot = 0"
    petentry = fields[1].GetUInt32();
    if(!petentry)
    {
        m_loading = false;
        return false;
    }

    uint32 summon_spell_id = fields[21].GetUInt32();
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(summon_spell_id);

    bool isTemporarySummon = spellInfo && spellInfo->GetDuration() > 0;

    // check temporary summoned pets like mage water elemental
    if(current && isTemporarySummon)
    {
        m_loading = false;
        return false;
    }

    Map *map = owner->GetMap();
    ObjectGuid::LowType guid = map->GenerateLowGuid<HighGuid::Pet>();
    uint32 pet_number = fields[0].GetUInt32();
    if(!Create(guid, map, owner->GetPhaseMask(), petentry, pet_number))
    {
        m_loading = false;
        return false;
    }

    if (current && owner->IsPetNeedBeTemporaryUnsummoned())
    {
        m_loading = false;
        owner->SetTemporaryUnsummonedPetNumber(pet_number);
        return false;
    }

    //Shadowfiend hack, try to spawn it close to target
    float px, py, pz;
    if (petentry == 19668 && owner->ToPlayer() && owner->ToPlayer()->GetTarget()) 
    {
        target = owner->ToPlayer()->GetVictim();
        if (target && _CanCreatureAttack(target) == CAN_ATTACK_RESULT_OK)
        {
            target->GetClosePoint(px, py, pz, GetCombatReach(), PET_FOLLOW_DIST, this->GetFollowAngle());
            UpdateAllowedPositionZ(px, py, pz); //prevent it spawning on flying targets
        }
        else {
            //spawn at owner instead
            owner->GetClosePoint(px, py, pz, GetCombatReach(), PET_FOLLOW_DIST, this->GetFollowAngle());
            target = nullptr;
        }
    }
    else
        owner->GetClosePoint(px, py, pz, GetCombatReach(), PET_FOLLOW_DIST, this->GetFollowAngle());

    Relocate(px, py, pz, owner->GetOrientation());

    if(!IsPositionValid())
    {
        TC_LOG_ERROR("entities.pet","ERROR: Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUID().GetCounter(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    setPetType(PetType(fields[22].GetUInt8()));
    SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,owner->GetFaction());
    SetUInt32Value(UNIT_CREATED_BY_SPELL, summon_spell_id);

    if (IsCritter())
    {
        AIM_Initialize();
        float x, y, z;
        owner->GetClosePoint(x, y, z, GetCombatReach(), PET_FOLLOW_DIST, GetFollowAngle());
        Relocate(x, y, z, owner->GetOrientation());

        if (!IsPositionValid())
        {
            TC_LOG_ERROR("entities.pet", "Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
                ObjectGuid(GetGUID()).GetCounter(), GetEntry(), GetPositionX(), GetPositionY());
            m_loading = false;
            return false;
        }

        map->AddToMap(this->ToCreature());
        m_loading = false;
        return true;
    }

    m_charmInfo->SetPetNumber(pet_number, IsPermanentPetFor(owner));

    SetDisplayId(fields[3].GetUInt32());
    SetNativeDisplayId(fields[3].GetUInt32());
    uint32 petlevel = fields[4].GetUInt8();
    SetUInt32Value(UNIT_NPC_FLAGS , UNIT_NPC_FLAG_NONE);
    SetName(fields[11].GetString());

    switch(getPetType())
    {
        case SUMMON_PET:
            petlevel=owner->GetLevel();

            SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, uint8(CLASS_MAGE));
            break;
        case HUNTER_PET:
            SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, CLASS_WARRIOR);
            SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, GENDER_NONE);
            SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_PET_LOYALTY, fields[8].GetUInt32());
            SetSheath(SHEATH_STATE_MELEE);

            SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, fields[12].GetBool() ? UNIT_RENAME_NOT_ALLOWED : UNIT_RENAME_ALLOWED);

            SetTP(fields[9].GetInt32());
            SetMaxPower(POWER_HAPPINESS,GetCreatePowers(POWER_HAPPINESS));
            SetPower(   POWER_HAPPINESS,fields[15].GetUInt32());
            SetPowerType(POWER_FOCUS);
            break;
        default:
            TC_LOG_ERROR("entities.pet","Pet have incorrect type (%u) for pet loading.",getPetType());
    }

    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED); // this enables popup window (pet abandon, cancel) 
#ifndef LICH_KING
    SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_BUFF_LIMIT, UNIT_BYTE2_PLAYER_CONTROLLED_BUFF_LIMIT);
#endif
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(WorldGameTime::GetGameTime()));
    SetCreatorGUID(owner->GetGUID());

    InitStatsForLevel(petlevel);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, fields[5].GetUInt32());

    SynchronizeLevelWithOwner();
#ifdef LICH_KING

    // Set pet's position after setting level, its size depends on it
    float px, py, pz;
    owner->GetClosePoint(px, py, pz, GetCombatReach(), PET_FOLLOW_DIST, GetFollowAngle());
    Relocate(px, py, pz, owner->GetOrientation());
    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.pet", "Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUID().GetCounter(), GetEntry(), GetPositionX(), GetPositionY());
        m_loading = false;
        return false;
    }
#endif

    SetReactState( ReactStates( fields[6].GetUInt8() ));
    m_loyaltyPoints = fields[7].GetInt32();

    // set current pet as current
    if(fields[10].GetUInt8() != 0)
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        trans->PAppend("UPDATE character_pet SET slot = '%u' WHERE owner = '%u' AND slot = '0' AND id <> '%u'", PET_SAVE_NOT_IN_SLOT, ownerid, m_charmInfo->GetPetNumber());
        trans->PAppend("UPDATE character_pet SET slot = '%u' WHERE owner = '%u' AND id = '%u'", PET_SAVE_AS_CURRENT, ownerid, m_charmInfo->GetPetNumber());
        CharacterDatabase.CommitTransaction(trans);
    }

    //load spells/cooldowns/auras
    SetCanModifyStats(true);

    AIM_Initialize();

    uint32 savedhealth = fields[13].GetUInt32();
    uint32 savedmana = fields[14].GetUInt32();

    if(getPetType() == SUMMON_PET && !current)              //all (?) summon pets come with full health when called, but not when they are current
    {
        SetFullHealth();
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    }
    else
    {
        if(!savedhealth && getPetType() == HUNTER_PET)
            SetDeathState(JUST_DIED);
        else
        {
            SetHealth(savedhealth > GetMaxHealth() ? GetMaxHealth() : (!savedhealth ? 1 : savedhealth) );
            SetPower(POWER_MANA, savedmana > GetMaxPower(POWER_MANA) ? GetMaxPower(POWER_MANA) : savedmana);
        }
    }

    owner->SetMinion(this, true);
    map->AddToMap(this->ToCreature(), true);

    // Spells should be loaded after pet is added to map, because in CanCast is check on it
    _LoadSpells();
    _LoadSpellCooldowns();

    // since last save (in seconds)
    uint32 timediff = (map->GetGameTime() - fields[18].GetUInt32());
    _LoadAuras(timediff); //sunstrider: special pet handling in there, we don't load aura saved too long ago since last dismiss

    if (!isTemporarySummon)
    {
        m_charmInfo->LoadPetActionBar(fields[16].GetString());

        _LoadSpells();
#ifdef LICH_KING
        InitTalentForLevel();                               // re-init to check talent count
#else
        //init teach spells
        Tokens tokens = StrSplit(fields[17].GetString(), " ");
        Tokens::iterator iter;
        int index;
        for (iter = tokens.begin(), index = 0; index < 4; ++iter, ++index)
        {
            uint32 tmp = atol((*iter).c_str());

            ++iter;

            if (tmp)
                AddTeachSpell(tmp, atol((*iter).c_str()));
            else
                break;
        }
#endif
        LearnPetPassives(); //must be after _LoadAuras since _LoadAuras removes all auras
        if (map->IsBattleArena())
            RemoveArenaAuras(false);
        CastPetAuras(current);
    }
    else {
        // sunstrider: TC has dropped this table but we don't have the necessary data in creature_template to change this yet
        // Temporary summoned pets always have initial spell list at load
        InitPetCreateSpells();
    }

    CleanupActionBar();                                     // remove unknown spells from action bar after load

    owner->PetSpellInitialize();
    if(owner->GetGroup())
        owner->SetGroupUpdateFlag(GROUP_UPDATE_PET);

#ifdef LICH_KING
    owner->SendTalentsInfoData(true);
#endif

    //Declined names
    if(owner->GetTypeId() == TYPEID_PLAYER && getPetType() == HUNTER_PET)
    {
        result = CharacterDatabase.PQuery("SELECT genitive, dative, accusative, instrumental, prepositional FROM character_pet_declinedname WHERE owner = '%u' AND id = '%u'", owner->GetGUID().GetCounter(), GetCharmInfo()->GetPetNumber());

        if(result)
        {
            if(m_declinedname)
                delete m_declinedname;

            m_declinedname = new DeclinedName;
            Field* _fields = result->Fetch();
            for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            {
                m_declinedname->name[i] = _fields[i].GetString();
            }
        }
    }
    
    if (target)
        AI()->AttackStart(target);

    /*TC
    //set last used pet number (for use in BG's)
    if (owner->GetTypeId() == TYPEID_PLAYER && isControlled() && !isTemporarySummoned() && (getPetType() == SUMMON_PET || getPetType() == HUNTER_PET))
        owner->ToPlayer()->SetLastPetNumber(petId);
    */

    // must be after SetMinion (owner guid check)
    LoadTemplateImmunities();
    m_loading = false;
    return true;
}

bool Pet::IsPermanentPetFor(Player* owner) const
{
    switch (getPetType())
    {
    case SUMMON_PET:
        switch (owner->GetClass())
        {
        case CLASS_WARLOCK:
            return GetCreatureTemplate()->type == CREATURE_TYPE_DEMON;
#ifdef LICH_KING
        case CLASS_DEATH_KNIGHT:
            return GetCreatureTemplate()->type == CREATURE_TYPE_UNDEAD;
#endif
        default:
            return false;
        }
    case HUNTER_PET:
        return true;
    default:
        return false;
    }
}

void Pet::SavePetToDB(PetSaveMode mode)
{
    if(!GetEntry())
        return;

    // save only fully controlled creature
    if(!isControlled())
        return;

    //do not save pets for testing bots
    if (Player* p = GetOwner())
        if (p->IsTestingBot())
            return;

    // dont save not player pets
    if (!GetOwnerGUID().IsPlayer())
        return;

    uint32 curhealth = GetHealth();
    uint32 curmana = GetPower(POWER_MANA);

    //only alive and active pets get auras saved
    if((mode != PET_SAVE_AS_CURRENT && mode != PET_SAVE_NOT_IN_SLOT) || !IsAlive())
        RemoveAllAuras();

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    _SaveSpells(trans);
    GetSpellHistory()->SaveToDB<Pet>(trans);
    if(getPetType() == HUNTER_PET)
        _SaveAuras(trans);

    if (mode >= PET_SAVE_AS_CURRENT) //every mode but PET_SAVE_AS_DELETED
    {
        ObjectGuid::LowType owner = GetOwnerGUID().GetCounter();
        std::string name = m_name;
        CharacterDatabase.EscapeString(name);
        // remove current data
        trans->PAppend("DELETE FROM character_pet WHERE owner = '%u' AND id = '%u'", owner,m_charmInfo->GetPetNumber() );

        // prevent duplicate using slot (except PET_SAVE_NOT_IN_SLOT)
        if(mode!=PET_SAVE_NOT_IN_SLOT)
            trans->PAppend("UPDATE character_pet SET slot = 3 WHERE owner = '%u' AND slot = '%u'", owner, uint32(mode) );

        // prevent existence another hunter pet in PET_SAVE_AS_CURRENT and PET_SAVE_NOT_IN_SLOT
        if(getPetType()==HUNTER_PET && (mode==PET_SAVE_AS_CURRENT||mode==PET_SAVE_NOT_IN_SLOT))
            trans->PAppend("DELETE FROM character_pet WHERE owner = '%u' AND (slot = '%u' OR slot > '%u')", owner, PET_SAVE_AS_CURRENT, PET_SAVE_LAST_STABLE_SLOT );
        // save pet
        std::ostringstream ss;
        ss << "INSERT INTO character_pet ( id, entry,  owner, modelid, level, exp, Reactstate, loyaltypoints, loyalty, trainpoint, slot, name, renamed, curhealth, curmana, curhappiness, abdata, TeachSpelldata, savetime, resettalents_cost, resettalents_time, CreatedBySpell, PetType) "
            << "VALUES ("
            << m_charmInfo->GetPetNumber() << ", "
            << GetEntry() << ", "
            << owner << ", "
            << GetNativeDisplayId() << ", "
            << GetLevel() << ", "
            << GetUInt32Value(UNIT_FIELD_PETEXPERIENCE) << ", "
            << uint32(GetReactState()) << ", "
            << m_loyaltyPoints << ", "
            << GetLoyaltyLevel() << ", "
            << m_TrainingPoints << ", "
            << uint32(mode) << ", '"
            << name.c_str() << "', "
            << uint32((GetByteValue(UNIT_FIELD_BYTES_2, 2) == UNIT_RENAME_ALLOWED) ? 0 : 1) << ", "
            << curhealth << ", "
            << curmana << ", "
            << GetPower(POWER_HAPPINESS) << ", "
            << "'" << GenerateActionBarData() << "', '";
        
        //save spells the pet can teach to it's Master
        {
            int i = 0;
            for(auto itr = m_teachspells.begin(); i < 4 && itr != m_teachspells.end(); ++i, ++itr)
                ss << itr->first << " " << itr->second << " ";
            for(; i < 4; ++i)
                ss << uint32(0) << " " << uint32(0) << " ";
        }

        ss  << "', "
            << GetMap()->GetGameTime() << ", "
            << uint32(m_resetTalentsCost) << ", "
            << uint64(m_resetTalentsTime) << ", "
            << GetUInt32Value(UNIT_CREATED_BY_SPELL) << ", "
            << uint32(getPetType()) << ")";

        trans->Append( ss.str().c_str() );

        CharacterDatabase.CommitTransaction(trans);
    } else { // PET_SAVE_AS_DELETED
        RemoveAllAuras();
        DeleteFromDB(m_charmInfo->GetPetNumber());
    }
}

void Pet::DeleteFromDB(ObjectGuid::LowType guidlow)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt;

    trans->PAppend("DELETE FROM character_pet WHERE id = '%u'", guidlow);
    trans->PAppend("DELETE FROM character_pet_declinedname WHERE id = '%u'", guidlow);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURAS);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    trans->PAppend("DELETE FROM pet_spell WHERE guid = '%u'", guidlow);
    trans->PAppend("DELETE FROM pet_spell_cooldown WHERE guid = '%u'", guidlow);
    CharacterDatabase.CommitTransaction(trans);
}

void Pet::SetDeathState(DeathState s)                       // overwrite virtual Creature::setDeathState and Unit::setDeathState
{
    Creature::SetDeathState(s);
    if(GetDeathState()==CORPSE)
    {
        //remove summoned pet (no corpse)
        if(getPetType()==SUMMON_PET)
            Remove(PET_SAVE_NOT_IN_SLOT);
        // other will despawn at corpse desppawning (Pet::Update code)
        else
        {
            // pet corpse non lootable and non skinnable
            SetUInt32Value( UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE );
            RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

             //lose happiness when died and not in BG/Arena
            if (!GetMap()->IsBattlegroundOrArena())
                ModifyPower(POWER_HAPPINESS, -HAPPINESS_LEVEL_SIZE);

            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        }
    }
    else if(GetDeathState()==ALIVE)
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        CastPetAuras(true);
    }
}

void Pet::Update(uint32 diff)
{
    if(m_removed)                                           // pet already removed, just wait in remove queue, no updates
        return;

    if (m_loading)
        return;

    switch( m_deathState )
    {
        case CORPSE:
        {
            if (m_corpseRemoveTime <= GetMap()->GetGameTime())
            {
                assert(getPetType()!=SUMMON_PET && "Must be already removed.");
                if (m_petType != HUNTER_PET)
                    Remove(PET_SAVE_NOT_IN_SLOT);               //hunters' pets never get removed because of death, NEVER!
                return;
            }
            break;
        }
        case ALIVE:
        {
            // unsummon pet that lost owner
            Unit* owner = GetOwner();
            if(!owner || (!IsWithinDistInMap(owner, OWNER_MAX_DISTANCE) && !IsPossessed()) || (isControlled() && !owner->GetPetGUID()))
            {
                Remove(PET_SAVE_NOT_IN_SLOT, true);
                return;
            }

            if(isControlled())
            {
                if( owner->GetPetGUID() != GetGUID() )
                {
                    Remove(getPetType()==HUNTER_PET?PET_SAVE_AS_DELETED:PET_SAVE_NOT_IN_SLOT);
                    return;
                }
            }

            if(m_duration > 0)
            {
                if(m_duration > diff)
                    m_duration -= diff;
                else
                {
                    Remove(getPetType() != SUMMON_PET ? PET_SAVE_AS_DELETED:PET_SAVE_NOT_IN_SLOT);
                    return;
                }
            }

            if(getPetType() != HUNTER_PET)
                break;

            //regenerate Focus
            if(m_regenFocusTimer <= diff)
            {
                Regenerate(POWER_FOCUS);
                m_regenFocusTimer = PET_FOCUS_REGEN_INTERVAL;
            }
            else
                m_regenFocusTimer -= diff;

            //don't loose happiness for arena server
            if(!sWorld->getConfig(CONFIG_ARENASERVER_ENABLED))
            {
                if(m_happinessTimer <= diff)
                {
                    LooseHappiness();
                    m_happinessTimer = 7500;
                }
                else
                    m_happinessTimer -= diff;
            }

            if(m_loyaltyTimer <= diff)
            {
                TickLoyaltyChange();
                m_loyaltyTimer = 12000;
            }
            else
                m_loyaltyTimer -= diff;

            break;
        }
        default:
            break;
    }
    Creature::Update(diff);
}

void Pet::LooseHappiness()
{
    uint32 curValue = GetPower(POWER_HAPPINESS);
    if (curValue <= 0)
        return;
    int32 addvalue = (140 >> GetLoyaltyLevel()) * 125;      //value is 70/35/17/8/4 (per min) * 1000 / 8 (timer 7.5 secs)
    if(IsInCombat())                                        //we know in combat happiness fades faster, multiplier guess
        addvalue = int32(addvalue * 1.5);
    ModifyPower(POWER_HAPPINESS, -addvalue);
}

void Pet::ModifyLoyalty(int32 addvalue)
{
    uint32 loyaltylevel = GetLoyaltyLevel();

    if(addvalue > 0)                                        //only gain influenced, not loss
        addvalue = int32((float)addvalue * sWorld->GetRate(RATE_LOYALTY));

    if(loyaltylevel >= BEST_FRIEND && (addvalue + m_loyaltyPoints) > int32(GetMaxLoyaltyPoints(loyaltylevel)))
        return;

    m_loyaltyPoints += addvalue;

    if(m_loyaltyPoints < 0)
    {
        if(loyaltylevel > REBELLIOUS)
        {
            //level down
            --loyaltylevel;
            SetLoyaltyLevel(LoyaltyLevel(loyaltylevel));
            m_loyaltyPoints = GetStartLoyaltyPoints(loyaltylevel);
            SetTP(m_TrainingPoints - int32(GetLevel()));
        }
        else
        {
            m_loyaltyPoints = 0;
            Unit* owner = GetOwner();
            if(owner && owner->GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_PET_BROKEN, 0);
                (owner->ToPlayer())->SendDirectMessage(&data);

                //run away
                (owner->ToPlayer())->RemovePet(this,PET_SAVE_AS_DELETED);
            }
        }
    }
    //level up
    else if(m_loyaltyPoints > int32(GetMaxLoyaltyPoints(loyaltylevel)))
    {
        ++loyaltylevel;
        SetLoyaltyLevel(LoyaltyLevel(loyaltylevel));
        m_loyaltyPoints = GetStartLoyaltyPoints(loyaltylevel);
        SetTP(m_TrainingPoints + GetLevel());
    }
}

void Pet::TickLoyaltyChange()
{
    int32 addvalue;

    switch(GetHappinessState())
    {
        case HAPPY:   addvalue =  20; break;
        case CONTENT: addvalue =  10; break;
        case UNHAPPY: addvalue = -20; break;
        default:
            return;
    }
    ModifyLoyalty(addvalue);
}

void Pet::KillLoyaltyBonus(uint32 level)
{
    if(level > 100)
        return;

                                                            //at lower levels gain is faster | the lower loyalty the more loyalty is gained
    uint32 bonus = uint32(((100 - level) / 10) + (6 - GetLoyaltyLevel()));
    ModifyLoyalty(bonus);
}

HappinessState Pet::GetHappinessState()
{
    if(GetPower(POWER_HAPPINESS) < HAPPINESS_LEVEL_SIZE)
        return UNHAPPY;
    else if(GetPower(POWER_HAPPINESS) >= HAPPINESS_LEVEL_SIZE * 2)
        return HAPPY;
    else
        return CONTENT;
}

void Pet::SetLoyaltyLevel(LoyaltyLevel level)
{
    SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_PET_LOYALTY, level);
}

bool Pet::CanTakeMoreActiveSpells(uint32 spellid)
{
    uint8  activecount = 1;
    uint32 chainstartstore[ACTIVE_SPELLS_MAX];

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if(spellInfo && spellInfo->IsPassive())
        return true;

    chainstartstore[0] = sSpellMgr->GetFirstSpellInChain(spellid);

    for (auto & m_spell : m_spells)
    {
        spellInfo = sSpellMgr->GetSpellInfo(m_spell.first);
        if(spellInfo && spellInfo->IsPassive())
            continue;

        uint32 chainstart = sSpellMgr->GetFirstSpellInChain(m_spell.first);

        uint8 x;

        for(x = 0; x < activecount; x++)
        {
            if(chainstart == chainstartstore[x])
                break;
        }

        if(x == activecount)                                //spellchain not yet saved -> add active count
        {
            ++activecount;
            if(activecount > ACTIVE_SPELLS_MAX)
                return false;
            chainstartstore[x] = chainstart;
        }
    }
    return true;
}

bool Pet::HasTPForSpell(uint32 spellid)
{
    int32 neededtrainp = GetTPForSpell(spellid);
    if((m_TrainingPoints - neededtrainp < 0 || neededtrainp < 0) && neededtrainp != 0)
        return false;
    return true;
}

int32 Pet::GetTPForSpell(uint32 spellid)
{
    uint32 basetrainp = 0;

    SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellid);

    auto firstSpell = *(skill_bounds.first);
    if (!firstSpell.second->reqtrainpoints)
        return 0;

    basetrainp = firstSpell.second->reqtrainpoints;

    uint32 spenttrainp = 0;
    uint32 chainstart = sSpellMgr->GetFirstSpellInChain(spellid);

    for (auto & m_spell : m_spells)
    {
        if(m_spell.second.state == PETSPELL_REMOVED)
            continue;

        if(sSpellMgr->GetFirstSpellInChain(m_spell.first) == chainstart)
        {
            SkillLineAbilityMapBounds _skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(m_spell.first);
            for(auto _spell_idx2 = _skill_bounds.first; _spell_idx2 != _skill_bounds.second; ++_spell_idx2)
            {
                if(_spell_idx2->second->reqtrainpoints > spenttrainp)
                {
                    spenttrainp = _spell_idx2->second->reqtrainpoints;
                    break;
                }
            }
        }
    }

    return int32(basetrainp) - int32(spenttrainp);
}

uint32 Pet::GetMaxLoyaltyPoints(uint32 level)
{
    if (!level) {
        TC_LOG_ERROR("entities.pet","CRASH ALERT: Called Pet::GetMaxLoyaltyPoints with level 0 for creature entry %u, owner %s (GUID %u). Incrementing it to prevent crash.", GetEntry(), GetOwner() ? GetOwner()->GetName().c_str() : "unknown", GetOwner() ? GetOwner()->GetGUID().GetCounter() : 0);
        ++level;
    }

    return LevelUpLoyalty[level - 1];
}

uint32 Pet::GetStartLoyaltyPoints(uint32 level)
{
    return LevelStartLoyalty[level - 1];
}

void Pet::SetTP(int32 TP)
{
    m_TrainingPoints = TP;
    SetUInt32Value(UNIT_TRAINING_POINTS, (uint32)GetDispTP());
}

int32 Pet::GetDispTP()
{
    if(getPetType()!= HUNTER_PET)
        return(0);
    if(m_TrainingPoints < 0)
        return -m_TrainingPoints;
    else
        return -(m_TrainingPoints + 1);
}

void Pet::Remove(PetSaveMode mode, bool returnreagent)
{
    GetOwner()->RemovePet(this, mode, returnreagent);
    /*
    Unit* owner = GetOwner();

    if(owner)
    {
        if(owner->GetTypeId()==TYPEID_PLAYER)
        {
            (owner->ToPlayer())->RemovePet(this,mode,returnreagent);
            return;
        }

        // only if current pet in slot
        if(owner->GetMinionGUID()==GetGUID())
            owner->SetPet(nullptr);
    }

    AddObjectToRemoveList();
    m_removed = true;
    */
}

void Pet::GivePetXP(uint32 xp)
{
    if(getPetType() != HUNTER_PET)
        return;

    if ( xp < 1 )
        return;

    if(!IsAlive())
        return;

    uint32 level = GetLevel();

    // XP to money conversion processed in Player::RewardQuest
    if(level >= sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL))
        return;

    uint32 curXP = GetUInt32Value(UNIT_FIELD_PETEXPERIENCE);
    uint32 nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    uint32 newXP = curXP + xp;

    if(newXP >= nextLvlXP && level+1 > GetOwner()->GetLevel())
    {
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, nextLvlXP-1);
        return;
    }

    while( newXP >= nextLvlXP && level < sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL) )
    {
        newXP -= nextLvlXP;
        SetLevel( level + 1 );
        level = GetLevel();
        GivePetLevel(level);
        nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    }

    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, newXP);

    if(getPetType() == HUNTER_PET)
        KillLoyaltyBonus(level);
}

void Pet::GivePetLevel(uint32 level)
{
    if(!level)
        return;

    if (getPetType() == HUNTER_PET)
    {
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(level)*PET_XP_FACTOR));
    }

    InitStatsForLevel( level);
    /*TC 
    InitLevelupSpellsForLevel();
    InitTalentForLevel();
    */

    SetTP(m_TrainingPoints + (GetLoyaltyLevel() - 1));
}

bool Pet::CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask)
{
    TC_LOG_DEBUG("entities.pet", "Pet::CreateBaseForTamed");
    ObjectGuid::LowType guid = map->GenerateLowGuid<HighGuid::Pet>();
    uint32 petId = sObjectMgr->GeneratePetNumber();
    if (!Create(guid, map, phaseMask, cinfo->Entry, petId))
        return false;

    //SetNativeDisplayId(creature->GetNativeDisplayId());
    SetMaxPower(POWER_HAPPINESS, GetCreatePowers(POWER_HAPPINESS));
    SetPower(POWER_HAPPINESS, 166500);
    SetPowerType(POWER_FOCUS);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(GetLevel() + 1)*PET_XP_FACTOR));
    SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
#ifndef LICH_KING
    SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_BUFF_LIMIT, UNIT_BYTE2_PLAYER_CONTROLLED_BUFF_LIMIT);
#endif

    m_loyaltyPoints = 1000;
    if (cinfo->type == CREATURE_TYPE_BEAST)
    {
        SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, CLASS_WARRIOR);
        SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, GENDER_NONE);
        SetSheath(SHEATH_STATE_MELEE);
#ifdef LICH_KING
        SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);
        SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_1_UNK, UNIT_BYTE2_FLAG_UNK3);
#else
        SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_RENAME_ALLOWED);
#endif
        //SetUInt32Value(UNIT_MOD_CAST_SPEED, creature->GetUInt32Value(UNIT_MOD_CAST_SPEED));
        SetLoyaltyLevel(REBELLIOUS);
    }

    return true;
}

bool Pet::CreateBaseAtCreature(Creature* creature)
{
    if(!creature)
    {
        TC_LOG_ERROR("entities.pet","CRITICAL ERROR: NULL pointer parsed into CreateBaseAtCreature()");
        return false;
    }

    if (!CreateBaseAtTamed(creature->GetCreatureTemplate(), creature->GetMap(), creature->GetPhaseMask()))
        return false;

    Relocate(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());

    if(!IsPositionValid())
    {
        TC_LOG_ERROR("entities.pet","ERROR: Pet (guidlow %d, entry %d) not created base at creature. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUID().GetCounter(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    CreatureTemplate const *cinfo = GetCreatureTemplate();
    if(!cinfo)
    {
        TC_LOG_ERROR("entities.pet","ERROR: CreateBaseAtCreature() failed, creatureInfo is missing!");
        return false;
    }

    SetDisplayId(creature->GetDisplayId());

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name[sWorld->GetDefaultDbcLocale()]);

    if (GetName().empty())
        SetName(cinfo->Name);

    return true;
}

bool Pet::CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner)
{
    if (!CreateBaseAtTamed(cinfo, owner->GetMap(), owner->GetPhaseMask()))
        return false;

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name[sWorld->GetDefaultDbcLocale()]);

    Relocate(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());

    return true;
}

bool Guardian::InitStatsForLevel(uint8 petlevel)
{
    CreatureTemplate const *cinfo = GetCreatureTemplate();
    assert(cinfo);

    Unit* owner = GetOwner();
    if(!owner)
    {
        TC_LOG_ERROR("entities.pet","ERROR: attempt to summon pet (Entry %u) without owner! Attempt terminated.", cinfo->Entry);
        return false;
    }

    SetLevel(petlevel);

    //Determine pet type
    PetType petType = MAX_PET_TYPE;
    if (IsPet() && GetOwner()->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetOwner()->GetClass() == CLASS_WARLOCK
            || GetOwner()->GetClass() == CLASS_SHAMAN        // Fire Elemental
            || GetOwner()->GetClass() == CLASS_DEATH_KNIGHT) // Risen Ghoul
        {
            petType = SUMMON_PET;
        }
        else if (GetOwner()->GetClass() == CLASS_HUNTER)
        {
            petType = HUNTER_PET;
            m_unitTypeMask |= UNIT_MASK_HUNTER_PET;
        }
        else
        {
            TC_LOG_ERROR("entities.pet", "Unknown type pet %u is summoned by player class %u",
                GetEntry(), GetOwner()->GetClass());
        }
    }

    uint32 creature_ID = (petType == HUNTER_PET) ? 1 : cinfo->Entry;

    SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));

    SetStatFlatModifier(UNIT_MOD_ARMOR, BASE_VALUE, float(petlevel*50));

    SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);
    SetAttackTime(OFF_ATTACK, BASE_ATTACK_TIME);
    SetAttackTime(RANGED_ATTACK, BASE_ATTACK_TIME);

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0);

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family);
    if(cFamily && cFamily->minScale > 0.0f && petType == HUNTER_PET)
    {
        float scale;
        if (GetLevel() >= cFamily->maxScaleLevel)
            scale = cFamily->maxScale;
        else if (GetLevel() <= cFamily->minScaleLevel)
            scale = cFamily->minScale;
        else
          scale = cFamily->minScale + (float)(GetLevel() - cFamily->minScaleLevel) / (float)cFamily->maxScaleLevel * (cFamily->maxScale - cFamily->minScale);

        SetObjectScale(scale);
    }


    if (!IsHunterPet())  // Hunters pet should not inherit resistances from creature_template, they have separate auras for that
        for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            SetStatFlatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_VALUE, float(cinfo->resistance[i-1]));

    // Health, Mana or Power, Armor
    PetLevelInfo const* pInfo = sObjectMgr->GetPetLevelInfo(creature_ID, petlevel);
    if (pInfo)                                      // exist in DB
    {
        SetCreateHealth(pInfo->health);
        SetCreateMana(pInfo->mana);

        if (pInfo->armor > 0)
            SetStatFlatModifier(UNIT_MOD_ARMOR, BASE_VALUE, float(pInfo->armor));

        for (uint8 stat = 0; stat < MAX_STATS; ++stat)
            SetCreateStat(Stats(stat), float(pInfo->stats[stat]));
    }
    else                                            // not exist in DB, use some default fake data
    {
        // remove elite bonuses included in DB values
        CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(petlevel, cinfo->unit_class);
        SetCreateHealth(stats->BaseHealth[cinfo->expansion]);
        SetCreateMana(stats->BaseMana);

        SetCreateStat(STAT_STRENGTH, 22);
        SetCreateStat(STAT_AGILITY, 22);
        SetCreateStat(STAT_STAMINA, 25);
        SetCreateStat(STAT_INTELLECT, 28);
        SetCreateStat(STAT_SPIRIT, 27);
    }
     // Power
    if (petType == HUNTER_PET) // Hunter pets have focus
        SetPowerType(POWER_FOCUS);
#ifdef LICH_KING
    else if (IsPetGhoul() || IsRisenAlly()) // DK pets have energy
        SetPowerType(POWER_ENERGY);
#endif
    else
        SetPowerType(POWER_MANA);

    // Damage
    SetBonusDamage(0);
    switch(petType)
    {
        case SUMMON_PET:
        {
            //the damage bonus used for pets is either fire or shadow damage, whatever is higher
            int32 fire = owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
            int32 shadow = owner->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            int32 val = (fire > shadow) ? fire : shadow;
            if (val < 0)
                val = 0;

            SetBonusDamage(int32(val * 0.15f));

            //default dmg values
            float minDmg = float(petlevel - (petlevel / 4));
            float maxDmg = float(petlevel + (petlevel / 4));
            if (pInfo) //db value if any
            {
                minDmg = pInfo->minDamage;
                maxDmg = pInfo->maxDamage;
            } 

            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, minDmg);
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, maxDmg);

            //SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, float(cinfo->attackpower));

            break;
        }
        case HUNTER_PET:
        {
            SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(GetLevel() + 1)*PET_XP_FACTOR));

            //these formula may not be correct; however, it is designed to be close to what it should be
            //this makes dps 0.5 of pets level
            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)) );
            //damage range is then petlevel / 2
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)) );
            //damage is increased afterwards as strength and pet scaling modify attack power
            break;
        }
        default:
        {
            switch (GetEntry())
            {
                case ENTRY_WATER_ELEMENTAL: // mage Water Elemental
                {
                    //40% damage bonus of mage's frost damage
                    SetBonusDamage(int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FROST) * 0.4f)); //TC has 0.33f
                    break;
                }
                case ENTRY_TREANT: //force of nature
                    if (!pInfo)
                        SetCreateHealth(30 + 30 * petlevel);
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 2.5f - (petlevel / 2)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 2.5f + (petlevel / 2)));
                    break;
                case ENTRY_EARTH_ELEMENTAL: //earth elemental 36213
                    if (!pInfo)
                        SetCreateHealth(100 + 120 * petlevel);
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));
                    break;
                case ENTRY_FIRE_ELEMENTAL: //fire elemental
                    if (!pInfo)
                    {
                        SetCreateHealth(40 * petlevel);
                        SetCreateMana(28 + 10 * petlevel);
                    }
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel * 4 - petlevel));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel * 4 + petlevel));
                    break;
                case ENTRY_SHADOWFIEND: // Shadowfiend
                {
                    if (!pInfo) 
                    {
                        SetCreateMana(28 + 10 * petlevel);
                        SetCreateHealth(28 + 30 * petlevel);
                    }
                    int32 bonus_dmg = (int32(owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW)* 0.0657f));  // shadowfiend 65.7% per 10 hits so 6.57 per hit
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petlevel) + bonus_dmg)); //correct value unknown
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petlevel * 3.0f / 2.0f) + bonus_dmg)); //correct value unknown
                    SetAttackTime(BASE_ATTACK, 1350); // The Shadowfiend grants up to 11 hits per use, making its attack speed between 1.26 - 1.36 seconds per hit.

                    // 2 parts T4 DPS Bonus: should be handled in future table spell_dbc
                    if (owner->HasAura(37570))
                        SetStat(STAT_STAMINA, GetStat(STAT_STAMINA) + 75);
                    break;
                }
                case ENTRY_SNAKE_TRAP_VENOMOUS: //Snake Trap - Venomous Snake
                    if (!pInfo)
                    {
                        SetCreateHealth(uint32(107 * (petlevel - 40) * 0.025f));
                        SetCreateMana(0);
                    }
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petlevel / 2) - 25));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petlevel / 2) - 18));
                    break;
                case ENTRY_SNAKE_TRAP_VIPER: //Snake Trap - Viper
                    if (!pInfo)
                    {
                        SetCreateHealth(uint32(107 * (petlevel - 40) * 0.025f));
                        SetCreateMana(0);
                    }
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel / 2 - 10));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel / 2));
                    break;
                case ENTRY_STEAM_TONK: // Steam Tonk
                    if (!pInfo)
                    {
                        SetCreateHealth(30 + 10 * petlevel);
                        SetCreateMana(30 + 10 * petlevel);
                    }
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 1.0f);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 2.0f);
                    break;
#ifdef LICH_KING
                case 29264: // Feral Spirit
                {
                    if (!pInfo)
                        SetCreateHealth(30 * petlevel);

                    // wolf attack speed is 1.5s
                    SetAttackTime(BASE_ATTACK, cinfo->BaseAttackTime);

                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petlevel * 4 - petlevel)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petlevel * 4 + petlevel)));

                    SetStatFlatModifier(UNIT_MOD_ARMOR, BASE_VALUE, float(GetOwner()->GetArmor()) * 0.35f);  // Bonus Armor (35% of player armor)
                    SetStatFlatModifier(UNIT_MOD_STAT_STAMINA, BASE_VALUE, float(GetOwner()->GetStat(STAT_STAMINA)) * 0.3f);  // Bonus Stamina (30% of player stamina)
                    if (!HasAura(58877))//prevent apply twice for the 2 wolves
                        AddAura(58877, this);//Spirit Hunt, passive, Spirit Wolves' attacks heal them and their master for 150% of damage done.
                    break;
                }
                case 31216: // Mirror Image
                {
                    SetBonusDamage(int32(GetOwner()->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FROST) * 0.33f));
                    SetDisplayId(GetOwner()->GetDisplayId());
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 30 * petlevel);
                        SetCreateHealth(28 + 10 * petlevel);
                    }
                    break;
                }
                case 27829: // Ebon Gargoyle
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10 * petlevel);
                        SetCreateHealth(28 + 30 * petlevel);
                    }
                    SetBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));
                    break;
                }
                case 28017: // Bloodworms
                {
                    SetCreateHealth(4 * petlevel);
                    SetBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.006f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - 30 - (petlevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel - 30 + (petlevel / 4)));
                }
#endif
            }
        }
    }

    UpdateAllStats();

    SetFullHealth();
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));

    return true;
}

bool Pet::HaveInDiet(ItemTemplate const* item) const
{
    if (!item->FoodType)
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if(!cInfo)
        return false;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if(!cFamily)
        return false;

    uint32 diet = cFamily->petFoodMask;
    uint32 FoodMask = 1 << (item->FoodType-1);
    return diet & FoodMask;
}

uint32 Pet::GetCurrentFoodBenefitLevel(uint32 itemlevel)
{
    // -5 or greater food level
    if(GetLevel() <= itemlevel + 5)                         //possible to feed level 60 pet with level 55 level food for full effect
        return 35000;
    // -10..-6
    else if(GetLevel() <= itemlevel + 10)                   //pure guess, but sounds good
        return 17000;
    // -14..-11
    else if(GetLevel() <= itemlevel + 14)                   //level 55 food gets green on 70, makes sense to me
        return 8000;
    // -15 or less
    else
        return 0;                                           //food too low level
}

void Pet::_LoadSpellCooldowns()
{
    if (GetEntry() == 510) // Don't load cooldowns for mage water elem
        return;
    
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_SPELL_COOLDOWN);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult cooldownsResult = CharacterDatabase.Query(stmt);

    GetSpellHistory()->LoadFromDB<Pet>(cooldownsResult);
}

void Pet::_LoadSpells()
{
    QueryResult result = CharacterDatabase.PQuery("SELECT spell, slot, active FROM pet_spell WHERE guid = '%u'",m_charmInfo->GetPetNumber());

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            AddSpell(fields[0].GetUInt32(), ActiveStates(fields[2].GetUInt16()), PETSPELL_UNCHANGED);
        }
        while( result->NextRow() );
    }
}

void Pet::_SaveSpells(SQLTransaction& trans)
{
    for (PetSpellMap::iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        ++next;

        // prevent saving family passives to DB
        if (itr->second.type == PETSPELL_FAMILY)
            continue;

        switch (itr->second.state)
        {
        case PETSPELL_REMOVED:
            trans->PAppend("DELETE FROM pet_spell WHERE guid = '%u' and spell = '%u'", m_charmInfo->GetPetNumber(), itr->first);

            m_spells.erase(itr);
            continue; //not break
        case PETSPELL_CHANGED:
            trans->PAppend("DELETE FROM pet_spell WHERE guid = '%u' and spell = '%u'", m_charmInfo->GetPetNumber(), itr->first);
            trans->PAppend("INSERT INTO pet_spell (guid,spell,slot,active) VALUES ('%u', '%u', '%u','%u')", m_charmInfo->GetPetNumber(), itr->first, itr->second.slotId, itr->second.active);
            break;
        case PETSPELL_NEW:
            trans->PAppend("INSERT INTO pet_spell (guid,spell,slot,active) VALUES ('%u', '%u', '%u','%u')", m_charmInfo->GetPetNumber(), itr->first, itr->second.slotId, itr->second.active);
            break;
        case PETSPELL_UNCHANGED:
            continue;
        }
        itr->second.state = PETSPELL_UNCHANGED;
    }
}

void Pet::_LoadAuras(uint32 timediff)
{
    for (auto & m_modAura : m_modAuras)
        m_modAura.clear();

    // all aura related fields
    for(int i = UNIT_FIELD_AURA; i <= UNIT_FIELD_AURASTATE; ++i)
        SetUInt32Value(i, 0);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_AURA);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        do
        {
            int32 damage[3];
            int32 baseDamage[3];
            Field* fields = result->Fetch();
            ObjectGuid caster_guid(fields[0].GetUInt64());
            // NULL guid stored - pet is the caster of the spell - see Pet::_SaveAuras
            if (!caster_guid)
                caster_guid = GetGUID();
            uint32 spellid = fields[1].GetUInt32();
            uint8 effmask = fields[2].GetUInt8();
            uint8 recalculatemask = fields[3].GetUInt8();
            uint8 stackcount = fields[4].GetUInt8();
            damage[0] = fields[5].GetInt32();
            damage[1] = fields[6].GetInt32();
            damage[2] = fields[7].GetInt32();
            baseDamage[0] = fields[8].GetInt32();
            baseDamage[1] = fields[9].GetInt32();
            baseDamage[2] = fields[10].GetInt32();
            int32 maxduration = fields[11].GetInt32();
            int32 remaintime = fields[12].GetInt32();
            uint8 remaincharges = fields[13].GetUInt8();
            float critChance = fields[14].GetFloat();
            bool applyResilience = fields[15].GetBool();

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if (!spellInfo)
            {
                TC_LOG_ERROR("entities.pet", "Unknown aura (spellid %u), ignore.", spellid);
                continue;
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && (!spellInfo->IsPositive() || spellInfo->HasAttribute(SPELL_ATTR4_EXPIRE_OFFLINE)))
            {
                if (remaintime / IN_MILLISECONDS <= int32(timediff))
                    continue;

                remaintime -= timediff * IN_MILLISECONDS;
            }

            // prevent wrong values of remaincharges
            if (spellInfo->ProcCharges)
            {
                if (remaincharges <= 0 || remaincharges > spellInfo->ProcCharges)
                    remaincharges = spellInfo->ProcCharges;
            }
            else
                remaincharges = 0;

            AuraCreateInfo createInfo(spellInfo, effmask, this);
            createInfo
                .SetCasterGUID(caster_guid)
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
                TC_LOG_DEBUG("entities.pet", "Added aura spellid %u, effectmask %u", spellInfo->Id, effmask);
            }
        } while (result->NextRow());
    }
}

void Pet::_SaveAuras(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURAS);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    trans->Append(stmt);

    for (AuraMap::const_iterator itr = m_ownedAuras.begin(); itr != m_ownedAuras.end(); ++itr)
    {
        // windrunner: skip all auras from spell that apply at cast SPELL_AURA_MOD_SHAPESHIFT or pet area auras.
        for (const auto & Effect : itr->second->GetSpellInfo()->Effects)
        {
            if (Effect.ApplyAuraName == SPELL_AURA_MOD_STEALTH ||
                Effect.Effect == SPELL_EFFECT_APPLY_AREA_AURA_OWNER ||
                Effect.Effect == SPELL_EFFECT_APPLY_AREA_AURA_PET)
                continue;
        }

        // check if the aura has to be saved
        if (!itr->second->CanBeSaved() /* TC || IsPetAura(itr->second)*/)
            continue;

        Aura* aura = itr->second;

        int32 damage[MAX_SPELL_EFFECTS];
        int32 baseDamage[MAX_SPELL_EFFECTS];
        uint8 effMask = 0;
        uint8 recalculateMask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetEffect(i))
            {
                baseDamage[i] = aura->GetEffect(i)->GetBaseAmount();
                damage[i] = aura->GetEffect(i)->GetAmount();
                effMask |= (1 << i);
                if (aura->GetEffect(i)->CanBeRecalculated())
                    recalculateMask |= (1 << i);
            }
            else
            {
                baseDamage[i] = 0;
                damage[i] = 0;
            }
        }

        // don't save guid of caster in case we are caster of the spell - guid for pet is generated every pet load, so it won't match saved guid anyways
        ObjectGuid casterGUID = (itr->second->GetCasterGUID() == GetGUID()) ? ObjectGuid::Empty : itr->second->GetCasterGUID();

        uint8 index = 0;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_AURA);
        stmt->setUInt32(index++, m_charmInfo->GetPetNumber());
        stmt->setUInt64(index++, casterGUID.GetRawValue());
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

bool Pet::AddSpell(uint32 spellId, ActiveStates active /*= ACT_DECIDE*/, PetSpellState state /*= PETSPELL_NEW*/, PetSpellType type /*= PETSPELL_NORMAL*/)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
    {
        // do pet spell book cleanup
        if(state == PETSPELL_UNCHANGED)                     // spell load case
        {
            TC_LOG_ERROR("entities.pet","Pet::addSpell: Non-existed in SpellStore spell #%u request, deleting for all pets in `pet_spell`.", spellId);
            CharacterDatabase.PExecute("DELETE FROM pet_spell WHERE spell = '%u'", spellId);
        }
        else
            TC_LOG_ERROR("entities.pet","Pet::addSpell: Non-existed in SpellStore spell #%u request.", spellId);

        return false;
    }

    PetSpellMap::iterator itr = m_spells.find(spellId);
    if (itr != m_spells.end())
    {
        if (itr->second.state == PETSPELL_REMOVED)
            state = PETSPELL_CHANGED;
        else
        {
            if (state == PETSPELL_UNCHANGED && itr->second.state != PETSPELL_UNCHANGED)
            {
                // can be in case spell loading but learned at some previous spell loading
                itr->second.state = PETSPELL_UNCHANGED;

                if (active == ACT_ENABLED)
                    ToggleAutocast(spellInfo, true);
                else if (active == ACT_DISABLED)
                    ToggleAutocast(spellInfo, false);
            }

            return false;
        }
    }

    PetSpell newspell;
    newspell.state = state;
    newspell.type = type;

    if (active == ACT_DECIDE)                               // active was not used before, so we save it's autocast/passive state here
    {
        if (spellInfo->IsAutocastable())
            newspell.active = ACT_DISABLED;
        else
            newspell.active = ACT_PASSIVE;
    }
    else
        newspell.active = active;

    // talent: unlearn all other talent ranks (high and low)
    if (TalentSpellPos const* talentPos = GetTalentSpellPos(spellId))
    {
        if (TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id))
        {
            for (uint8 i = 0; i < MAX_TALENT_RANK; ++i)
            {
                // skip learning spell and no rank spell case
                uint32 rankSpellId = talentInfo->RankID[i];
                if (!rankSpellId || rankSpellId == spellId)
                    continue;

                // skip unknown ranks
                if (!HasSpell(rankSpellId))
                    continue;
                RemoveSpell(rankSpellId, false, false);
            }
        }
    }
    else if (spellInfo->IsRanked())
    {
        for (PetSpellMap::const_iterator itr2 = m_spells.begin(); itr2 != m_spells.end(); ++itr2)
        {
            if (itr2->second.state == PETSPELL_REMOVED)
                continue;

            SpellInfo const* oldRankSpellInfo = sSpellMgr->GetSpellInfo(itr2->first);

            if (!oldRankSpellInfo)
                continue;

            if (spellInfo->IsDifferentRankOf(oldRankSpellInfo))
            {
                // replace by new high rank
                if (spellInfo->IsHighRankOf(oldRankSpellInfo))
                {
                    newspell.active = itr2->second.active;

                    if (newspell.active == ACT_ENABLED)
                        ToggleAutocast(oldRankSpellInfo, false);

                    UnlearnSpell(itr2->first, false, false);
                    break;
                }
                // ignore new lesser rank
                else
                    return false;
            }
        }
    }

    m_spells[spellId] = newspell;

    if (spellInfo->IsPassive() && (!spellInfo->CasterAuraState || HasAuraState(AuraStateType(spellInfo->CasterAuraState))))
        CastSpell(this, spellId, true);
    else
        m_charmInfo->AddSpellToActionBar(spellInfo);

    if (newspell.active == ACT_ENABLED)
        ToggleAutocast(spellInfo, true);

#ifdef LICH_KING
    uint32 talentCost = GetTalentSpellCost(spellId);
    if (talentCost)
    {
        int32 free_points = GetMaxTalentPointsForLevel(getLevel());
        m_usedTalentCount += talentCost;
        // update free talent points
        free_points -= m_usedTalentCount;
        SetFreeTalentPoints(free_points > 0 ? free_points : 0);
    }
#endif
    return true;
}

bool Pet::LearnSpell(uint32 spell_id)
{
    // prevent duplicated entires in spell book
    if (!AddSpell(spell_id))
        return false;

#ifdef LICH_KING
    if (!m_loading)
    {
        WorldPacket data(SMSG_PET_LEARNED_SPELL, 4);
        data << uint32(spell_id);
        GetOwner()->SendDirectMessage(&data);
        GetOwner()->PetSpellInitialize();
    }
#endif
    return true;
}

bool Pet::UnlearnSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    if (RemoveSpell(spell_id, learn_prev, clear_ab))
    {
#ifdef LICH_KING
        if (!m_loading)
        {
            WorldPacket data(SMSG_PET_REMOVED_SPELL, 4);
            data << uint32(spell_id);
            GetOwner()->SendDirectMessage(&data);
        }
#endif
        return true;
    }
    return false;
}

bool Pet::RemoveSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    PetSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return false;

    if (itr->second.state == PETSPELL_REMOVED)
        return false;

    if (itr->second.state == PETSPELL_NEW)
        m_spells.erase(itr);
    else
        itr->second.state = PETSPELL_REMOVED;

    RemoveAurasDueToSpell(spell_id);

#ifdef LICH_KING
    uint32 talentCost = GetTalentSpellCost(spell_id);
    if (talentCost > 0)
    {
        if (m_usedTalentCount > talentCost)
            m_usedTalentCount -= talentCost;
        else
            m_usedTalentCount = 0;
        // update free talent points
        int32 free_points = GetMaxTalentPointsForLevel(getLevel()) - m_usedTalentCount;
        SetFreeTalentPoints(free_points > 0 ? free_points : 0);
    }

#endif
    if (learn_prev)
    {
        if (uint32 prev_id = sSpellMgr->GetPrevSpellInChain(spell_id))
            LearnSpell(prev_id);
        else
            learn_prev = false;
    }

    // if remove last rank or non-ranked then update action bar at server and client if need
    if (clear_ab && !learn_prev && m_charmInfo->RemoveSpellFromActionBar(spell_id))
    {
        if (!m_loading)
            GetOwner()->PetSpellInitialize(); // need update action bar for last removed rank
    }
    return true;
}

void Pet::CleanupActionBar()
{
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        if (UnitActionBarEntry const* ab = m_charmInfo->GetActionBarEntry(i))
            if (ab->GetAction() && ab->IsActionBarForSpell())
            {
                if (!HasSpell(ab->GetAction()))
                    m_charmInfo->SetActionBar(i, 0, ACT_PASSIVE);
                else if (ab->GetType() == ACT_ENABLED)
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ab->GetAction()))
                        ToggleAutocast(spellInfo, true);
                }
            }
}

void Pet::InitPetCreateSpells()
{
    m_charmInfo->InitPetActionBar();
    m_spells.clear();

    int32 usedtrainpoints = 0, petspellid;
    PetCreateSpellEntry const* CreateSpells = sObjectMgr->GetPetCreateSpellEntry(GetEntry());
    if(CreateSpells)
    {
        for(uint32 i : CreateSpells->spellid)
        {
            if(!i)
                break;

            SpellInfo const *learn_spellproto = sSpellMgr->GetSpellInfo(i);
            if(!learn_spellproto)
                continue;

            if(learn_spellproto->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL || learn_spellproto->Effects[0].Effect == SPELL_EFFECT_LEARN_PET_SPELL)
            {
                petspellid = learn_spellproto->Effects[0].TriggerSpell;
                Unit* owner = GetOwner();
                if(owner && owner->GetTypeId() == TYPEID_PLAYER && !(owner->ToPlayer())->HasSpell(learn_spellproto->Id))
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(petspellid);
                    if (!spellInfo) 
                    {
                        TC_LOG_ERROR("entities.pet", "InitPetCreateSpells() could not find triggered spell %u, learned from spell %u", petspellid, i);
                        continue;
                    }
                    if(spellInfo->IsPassive())          //learn passive skills when tamed, not sure if thats right
                        (owner->ToPlayer())->LearnSpell(learn_spellproto->Id, false);
                    else
                        AddTeachSpell(learn_spellproto->Effects[0].TriggerSpell, learn_spellproto->Id);
                }
            }
            else
                petspellid = learn_spellproto->Id;

            if (petspellid == SPELL_WATER_ELEMENTAL_WATERBOLT)
                AddSpell(petspellid,ACT_ENABLED);
            else
                AddSpell(petspellid);

            SkillLineAbilityMapBounds skill_bounds = sSpellMgr->GetSkillLineAbilityMapBounds(learn_spellproto->Effects[0].TriggerSpell);
            for(auto _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
            {
                usedtrainpoints += _spell_idx->second->reqtrainpoints;
            }
        }
    }

    LearnPetPassives();

    CastPetAuras(false);

    SetTP(-usedtrainpoints);
}

void Pet::CheckLearning(uint32 spellid)
{
                                                            //charmed case -> prevent crash
    if(GetTypeId() == TYPEID_PLAYER || getPetType() != HUNTER_PET)
        return;

    Unit* owner = GetOwner();

    if(m_teachspells.empty() || !owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    auto itr = m_teachspells.find(spellid);
    if(itr == m_teachspells.end())
        return;

    if(urand(0, 100) < 10)
    {
        (owner->ToPlayer())->LearnSpell(itr->second, false);
        m_teachspells.erase(itr);
    }
}

uint32 Pet::ResetTalentsCost() const
{
    uint32 days = (GetMap()->GetGameTime() - m_resetTalentsTime)/DAY;

    // The first time reset costs 10 silver; after 1 day cost is reset to 10 silver
    if(m_resetTalentsCost < 10*SILVER || days > 0)
        return 10*SILVER;
    // then 50 silver
    else if(m_resetTalentsCost < 50*SILVER)
        return 50*SILVER;
    // then 1 gold
    else if(m_resetTalentsCost < 1*GOLD)
        return 1*GOLD;
    // then increasing at a rate of 1 gold; cap 10 gold
    else
        return (m_resetTalentsCost + 1*GOLD > 10*GOLD ? 10*GOLD : m_resetTalentsCost + 1*GOLD);
}

void Pet::ToggleAutocast(SpellInfo const* spellInfo, bool apply)
{
    if(spellInfo->IsPassive())
        return;

    uint32 spellid = spellInfo->Id;

    PetSpellMap::iterator itr = m_spells.find(spellid);
    if (itr == m_spells.end())
        return;

    uint32 i;

    if (apply)
    {
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i)
            ;                                               // just search

        if (i == m_autospells.size())
        {
            m_autospells.push_back(spellid);

            if (itr->second.active != ACT_ENABLED)
            {
                itr->second.active = ACT_ENABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
    else
    {
        AutoSpellList::iterator itr2 = m_autospells.begin();
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i, ++itr2)
            ;                                               // just search

        if (i < m_autospells.size())
        {
            m_autospells.erase(itr2);
            if (itr->second.active != ACT_DISABLED)
            {
                itr->second.active = ACT_DISABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
}

bool Pet::Create(ObjectGuid::LowType guidlow, Map *map, uint32 phaseMask, uint32 Entry, uint32 pet_number)
{
    ASSERT(map);
    SetMap(map);

    SetPhaseMask(phaseMask, false);
    Object::_Create(guidlow, pet_number, HighGuid::Pet);

    m_spawnId = guidlow;
    m_originalEntry = Entry;

    if(!InitEntry(Entry))
        return false;

    SetSheath(SHEATH_STATE_MELEE);
#ifdef LICH_KING
    // Force regen flag for player pets, just like we do for players themselves
    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);
#endif

    if(getPetType() == MINI_PET) // always non-attackable
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

    GetThreatManager().Initialize();

    return true;
}

bool Pet::HasSpell(uint32 spell) const
{
    return (m_spells.find(spell) != m_spells.end());
}

// Get all passive spells in our skill line
void Pet::LearnPetPassives()
{
    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if(!cInfo)
        return;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if(!cFamily)
        return;

    PetFamilySpellsStore::const_iterator petStore = sPetFamilySpellsStore.find(cFamily->ID);
    if(petStore != sPetFamilySpellsStore.end())
    {
        // For general hunter pets skill 270
        // Passive 01~10, Passive 00 (20782, not used), Ferocious Inspiration (34457)
        // Scale 01~03 (34902~34904, bonus from owner, not used)
        for(uint32 petSet : petStore->second)
            AddSpell(petSet, ACT_DECIDE, PETSPELL_NEW, PETSPELL_FAMILY);
    }
}

void Pet::CastPetAuras(bool current)
{
    Unit* owner = GetOwner();
    if(!owner)
        return;

    if(getPetType() != HUNTER_PET && (getPetType() != SUMMON_PET || owner->GetClass() != CLASS_WARLOCK))
        return;

    for(auto itr = owner->m_petAuras.begin(); itr != owner->m_petAuras.end();)
    {
        PetAura const* pa = *itr;
        ++itr;

        if(!current && pa->IsRemovedOnChangePet())
            owner->RemovePetAura(pa);
        else
            CastPetAura(pa);
    }
}

void Pet::CastPetAura(PetAura const* aura)
{
    uint16 auraId = aura->GetAura(GetEntry());
    if(!auraId)
        return;

    CastSpellExtraArgs args;
    args.TriggerFlags = TRIGGERED_FULL_MASK;
    if(auraId == 35696)                                       // Demonic Knowledge
        args.AddSpellBP0(CalculatePct(aura->GetDamage(), GetStat(STAT_STAMINA) + GetStat(STAT_INTELLECT)));
    
    CastSpell(this, auraId, args);
}

void Pet::SynchronizeLevelWithOwner()
{
    Player* owner = GetOwner();

    switch (getPetType())
    {
    // always same level
    case SUMMON_PET:
        GivePetLevel(owner->GetLevel());
        break;
    // can't be greater owner level
    case HUNTER_PET:
        if (GetLevel() > owner->GetLevel())
            GivePetLevel(owner->GetLevel());
#ifdef LICH_KING
        else if (GetLevel() + 5 < owner->GetLevel())
            GivePetLevel(owner->GetLevel() - 5);
#endif
        break;
    default:
        break;
    }
}
Player* Pet::GetOwner() const
{
    return Minion::GetOwner()->ToPlayer();
}

void Pet::SetDisplayId(uint32 modelId)
{
    Guardian::SetDisplayId(modelId);

    if (!isControlled())
        return;

    if (GetOwner()->GetGroup())
        GetOwner()->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MODEL_ID);
}

std::string Pet::GenerateActionBarData() const
{
    std::ostringstream oss;

    for (uint32 i = ACTION_BAR_INDEX_START; i < ACTION_BAR_INDEX_END; ++i)
    {
        oss << uint32(m_charmInfo->GetActionBarEntry(i)->GetType()) << ' '
            << uint32(m_charmInfo->GetActionBarEntry(i)->GetAction()) << ' ';
    }

    return oss.str();
}
