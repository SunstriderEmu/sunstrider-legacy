
#include "Common.h"
#include "SharedDefines.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "SkillExtraItems.h"
#include "Unit.h"
#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "Spell.h"
#include "DynamicObject.h"
#include "SpellAuras.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Pet.h"
#include "GameObject.h"
#include "GossipDef.h"
#include "Creature.h"
#include "Totem.h"
#include "CreatureAI.h"
#include "BattleGround.h"
#include "BattleGroundEY.h"
#include "BattleGroundWS.h"
#include "OutdoorPvPMgr.h"
#include "Management/VMapFactory.h"
#include "Language.h"
#include "SocialMgr.h"
#include "Util.h"
#include "TemporarySummon.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"
#include "InstanceScript.h"
#include "LogsDatabaseAccessor.h"

void Spell::EffectSummonType(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 entry = m_spellInfo->Effects[effIndex].MiscValue;
    if (!entry)
        return;

    SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(m_spellInfo->Effects[effIndex].MiscValueB);
    if (!properties)
    {
        TC_LOG_ERROR("spells", "EffectSummonType: Unhandled summon type %u.", m_spellInfo->Effects[effIndex].MiscValueB);
        return;
    }

    if (!m_originalCaster)
        return;

    int32 duration = m_spellInfo->GetDuration();
    if (Player* modOwner = m_originalCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);

    TempSummon* summon = nullptr;

    // determine how many units should be summoned
    uint32 numSummons = 1;

#ifdef LICH_KING
    // some spells need to summon many units, for those spells number of summons is stored in effect value
    // however so far noone found a generic check to find all of those (there's no related data in summonproperties.dbc
    // and in spell attributes, possibly we need to add a table for those)
    // so here's a list of MiscValueB values, which is currently most generic check
    switch (properties->Id)
    {
    case 64:
    case 61:
    case 1101:
    case 66:
    case 648:
    case 2301:
    case 1061:
    case 1261:
    case 629:
    case 181:
    case 715:
    case 1562:
    case 833:
    case 1161:
    case 713:
        numSummons = (damage > 0) ? damage : 1;
        break;
    default:
        numSummons = 1;
        break;
    }
#endif

    switch (properties->Category)
    {
    case SUMMON_CATEGORY_WILD:
    case SUMMON_CATEGORY_ALLY:
#ifdef LICH_KING
    case SUMMON_CATEGORY_UNK:
        if (properties->Flags & 512) //no such flags on BC
        {
            SummonGuardian(effIndex, entry, properties, numSummons);
            break;
        }
#endif
        //Only ones used on BC: 0,1,2,4,5
        switch (properties->Type)
        {
        case SUMMON_TYPE_PET:
        case SUMMON_TYPE_GUARDIAN:
        case SUMMON_TYPE_GUARDIAN2:
        case SUMMON_TYPE_MINION:
            SummonGuardian(effIndex, entry, properties, numSummons);
            break;
#ifdef LICH_KING
            // Summons a vehicle, but doesn't force anyone to enter it (see SUMMON_CATEGORY_VEHICLE)
        case SUMMON_TYPE_VEHICLE:
        case SUMMON_TYPE_VEHICLE2:
            summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster, m_spellInfo->Id);
            break;
        case SUMMON_TYPE_LIGHTWELL:
#endif
        case SUMMON_TYPE_TOTEM:
        {
            summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster, m_spellInfo->Id);
            if (!summon || !summon->IsTotem())
                return;

            // Mana Tide Totem
            if (m_spellInfo->Id == 16190)
                damage = m_caster->CountPctFromMaxHealth(10);

            if (damage)                                            // if not spell info, DB values used
            {
                summon->SetMaxHealth(damage);
                summon->SetHealth(damage);
            }
            break;
        }
        case SUMMON_TYPE_MINIPET:
        {
            summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster, m_spellInfo->Id);
            if (!summon || !summon->HasUnitTypeMask(UNIT_MASK_MINION))
                return;

            summon->SelectLevel();       // some summoned creaters have different from 1 DB data for level/hp
            summon->SetUInt32Value(UNIT_NPC_FLAGS, summon->GetCreatureTemplate()->npcflag);

            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

            summon->AI()->EnterEvadeMode();
            break;
        }
        default:
        {
            float radius = m_spellInfo->Effects[effIndex].CalcRadius();

            TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;

            for (uint32 count = 0; count < numSummons; ++count)
            {
                Position pos;
                if (count == 0)
                    pos = *destTarget;
                else
                    // randomize position for multiple summons
                    pos = m_caster->GetRandomPoint(*destTarget, radius);

                summon = m_originalCaster->SummonCreature(entry, pos, summonType, duration);
                if (!summon)
                    continue;

                if (properties->Category == SUMMON_CATEGORY_ALLY)
                {
                    summon->SetOwnerGUID(m_originalCaster->GetGUID());
                    summon->SetFaction(m_originalCaster->GetFaction());
                    summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
                }

                //lolhack section
                switch (m_spellInfo->Id)
                {
                    case 45392:
                        summon->SetSummoner(m_originalCaster);
                        if (summon->AI())
                            summon->AI()->AttackStart(m_caster);
                        break;
                    case 45891:
                        if (summon->AI())
                            summon->AI()->AttackStart(m_caster);
                        break;
                    case 45410:
                        summon->SetSummoner(m_originalCaster);
                        break;
                    case 45836:
                        summon->SetSummoner(m_caster);
                        if (m_caster->GetTypeId() == TYPEID_PLAYER)
                            m_caster->CastSpell((Unit*)nullptr, 45839, TRIGGERED_FULL_MASK);
                        m_caster->CastSpell((Unit*)nullptr, 45838, TRIGGERED_FULL_MASK);
                        summon->CastSpell((Unit*)nullptr, 45838, TRIGGERED_FULL_MASK);
                        break;
                }
                //TC ExecuteLogEffectSummonObject(effIndex, summon);
            }
            //hacks 2 - handle special cases
            switch (m_spellInfo->Id)
            {
                //quest 9711
                case 31333:
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        (m_caster->ToPlayer())->AreaExploredOrEventHappens(9711);
                    break;
                }
                case 26286:
                case 26291:
                case 26292:
                case 26294:
                case 26295:
                case 26293:
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        m_caster->ToPlayer()->KilledMonsterCredit(15893, 0);
                    break;
                case 26333:
                case 26334:
                case 26336:
                case 26337:
                case 26338:
                case 26335:
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        m_caster->ToPlayer()->KilledMonsterCredit(15893, 0);
                    break;
                case 26516:
                case 26517:
                case 26518:
                case 26519:
                case 26488:
                case 26490:
                case 26325:
                case 26304:
                case 26329:
                case 26328:
                case 26327:
                case 26326:
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        m_caster->ToPlayer()->KilledMonsterCredit(15894, 0);
                    break;
                default:
                    break;
            }

            //lolhack section 3
            switch (entry)
            {
                case 89: // Inferno summon
                {
                    // Enslave demon effect, without mana cost and cooldown
                    m_caster->CastSpell(summon, 20882, TRIGGERED_FULL_MASK);          // FIXME: enslave does not scale with level, level 62+ minions cannot be enslaved
                    summon->CastSpell(summon, 22703, TRIGGERED_FULL_MASK, nullptr); // Inferno effect
                } break;
                case 23369: // Whirling Blade (no idea what this is for)
                {
                    if (m_caster->GetVictim())
                        summon->AI()->AttackStart(m_caster->GetVictim());
                } break;
            }

            return;
        }
        }//switch
        break;
    case SUMMON_CATEGORY_PET:
        SummonGuardian(effIndex, entry, properties, numSummons);
        break;
    case SUMMON_CATEGORY_PUPPET:
        summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_originalCaster, m_spellInfo->Id);
        summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
        break;
#ifdef LICH_KING
    case SUMMON_CATEGORY_VEHICLE:
        // Summoning spells (usually triggered by npc_spellclick) that spawn a vehicle and that cause the clicker
        // to cast a ride vehicle spell on the summoned unit.
        summon = m_originalCaster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, m_caster, m_spellInfo->Id);
        if (!summon || !summon->IsVehicle())
            return;

        // The spell that this effect will trigger. It has SPELL_AURA_CONTROL_VEHICLE
        uint32 spellId = VEHICLE_SPELL_RIDE_HARDCODED;
        int32 basePoints = m_spellInfo->Effects[effIndex].CalcValue();
        if (basePoints > MAX_VEHICLE_SEATS)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(basePoints);
            if (spellInfo && spellInfo->HasAura(SPELL_AURA_CONTROL_VEHICLE))
                spellId = spellInfo->Id;
        }

        // if we have small value, it indicates seat position
        if (basePoints > 0 && basePoints < MAX_VEHICLE_SEATS)
            m_originalCaster->CastCustomSpell(spellId, SPELLVALUE_BASE_POINT0, basePoints, summon, TRIGGERED_FULL_MASK);
        else
            m_originalCaster->CastSpell(summon, spellId, TRIGGERED_FULL_MASK);

        uint32 faction = properties->Faction;
        if (!faction)
            faction = m_originalCaster->GetFaction();

        summon->SetFaction(faction);
        break;
#endif
    }

    if (summon)
    {
        summon->SetCreatorGUID(m_originalCaster->GetGUID());
        //TC ExecuteLogEffectSummonObject(effIndex, summon);

    }
}


void Spell::EffectSummonTotem(uint32 effIndex)
{
    EffectSummonType(effIndex);
}

//if caster is a creature, this handles like a normal summon (instead of 'toggling' minipet like for players)
void Spell::EffectSummonCritter(uint32 effIndex)
{
    EffectSummonType(effIndex);
    
}

void Spell::EffectSummonWild(uint32 effIndex)
{
    EffectSummonType(effIndex);
    
}

void Spell::SummonGuardian(uint32 i, uint32 entry, SummonPropertiesEntry const* properties, uint32 numGuardians)
{
    Unit* caster = m_originalCaster;
    if (!caster)
        return;

    if (caster->IsTotem())
        caster = caster->ToTotem()->GetOwner();

    // in another case summon new
    uint8 level = caster->GetLevel();

    // level of pet summoned using engineering item based at engineering skill level
    if (m_CastItem && caster->GetTypeId() == TYPEID_PLAYER)
        if (ItemTemplate const* proto = m_CastItem->GetTemplate())
            if (proto->RequiredSkill == SKILL_ENGINEERING)
                if (uint16 skill202 = caster->ToPlayer()->GetSkillValue(SKILL_ENGINEERING))
                    level = skill202 / 5;

    float radius = 5.0f;
    int32 duration = m_spellInfo->GetDuration();

    if (Player* modOwner = m_originalCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);

    //TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Map* map = caster->GetMap();

    for (uint32 count = 0; count < numGuardians; ++count)
    {
        Position pos;
        if (count == 0)
            pos = *destTarget;
        else
            // randomize position for multiple summons
            pos = m_caster->GetRandomPoint(*destTarget, radius);

        TempSummon* summon = map->SummonCreature(entry, pos, properties, duration, caster, m_spellInfo->Id);
        if (!summon)
            return;

        if (properties && properties->Category == SUMMON_CATEGORY_ALLY)
            summon->SetFaction(caster->GetFaction());

        if (summon->HasUnitTypeMask(UNIT_MASK_MINION) && m_targets.HasDst())
            ((Minion*)summon)->SetFollowAngle(m_caster->GetAngle(summon));

        switch (summon->GetEntry())
        {
#ifdef LICH_KING
            case 27893: //Rune Weapon
            {
                if (uint32 weapon = m_caster->GetUInt32Value(PLAYER_VISIBLE_ITEM_16_ENTRYID))
                {
                    summon->SetDisplayId(11686); // modelid2
                    summon->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, weapon);
                }
                else
                    summon->SetDisplayId(1126); // modelid1
            }
#endif
            case 2673: //target dummy (engineering item)
            {
                summon->AddAura(42176, summon);
                summon->AddUnitState(UNIT_STATE_ROOT);
            }
        }
        

        //sunstrider: removed for now, this breaks any aura added in AI inits/reset scripting hooks
        //summon->AI()->EnterEvadeMode();

        //TC ExecuteLogEffectSummonObject(i, summon);
    }
}

void Spell::EffectSummonGuardian(uint32 effIndex)
{
    EffectSummonType(effIndex);
    
}

void Spell::EffectSummonPossessed(uint32 effIndex)
{
    EffectSummonType(effIndex);
}

void Spell::EffectSummonDemon(uint32 i)
{
    EffectSummonType(i);
}
