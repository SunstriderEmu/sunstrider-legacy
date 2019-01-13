
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

    WorldObject* caster = m_caster;
    if (m_originalCaster)
        caster = m_originalCaster;

    int32 duration = m_spellInfo->GetDuration();
    if (Player* modOwner = caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);

    TempSummon* summon = nullptr;

    // determine how many units should be summoned
    uint32 numSummons = 1;

    // some spells need to summon many units, for those spells number of summons is stored in effect value
    // however so far noone found a generic check to find all of those (there's no related data in summonproperties.dbc
    // and in spell attributes, possibly we need to add a table for those)
    // so here's a list of MiscValueB values, which is currently most generic check
    switch (properties->Id)
    {
#ifdef LICH_KING
    // no spell on BC with both this id and damage > 1
    case 1101:
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
    case 66:
#endif
    case 64: //various spells
    case 61: //various spells
        numSummons = (damage > 0) ? damage : 1;
        break;
    default:
        numSummons = 1;
        break;
    }

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
            if (!_unitCaster)
                return;

            summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, _unitCaster, m_spellInfo->Id);
            break;
        case SUMMON_TYPE_LIGHTWELL:
#endif
        case SUMMON_TYPE_TOTEM:
        {
            if (!_unitCaster)
                return;

            summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, _unitCaster, m_spellInfo->Id);
            if (!summon || !summon->IsTotem())
                return;

            // Mana Tide Totem
            if (m_spellInfo->Id == 16190)
                damage = _unitCaster->CountPctFromMaxHealth(10);

            if (damage)                                            // if not spell info, DB values used
            {
                summon->SetMaxHealth(damage);
                summon->SetHealth(damage);
            }
            break;
        }
        case SUMMON_TYPE_MINIPET:
        {
            if (!_unitCaster)
                return;

            summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, _unitCaster, m_spellInfo->Id);
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
                    pos = caster->GetRandomPoint(*destTarget, radius);

                summon = caster->SummonCreature(entry, pos, summonType, duration);
                if (!summon)
                    continue;

                if (properties->Category == SUMMON_CATEGORY_ALLY)
                {
                    summon->SetOwnerGUID(caster->GetGUID());
                    summon->SetFaction(caster->GetFaction());
                    summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
                }

                ExecuteLogEffectSummonObject(effIndex, summon);

                //lolhack section
                if (_unitCaster)
                {
                    switch (m_spellInfo->Id)
                    {
                    case 45392: //Summon Demonic Vapor
                        if (summon->AI())
                            summon->AI()->AttackStart(_unitCaster);
                        break;
                    case 45891: //Sinister Reflection Summon
                        if (summon->AI())
                            summon->AI()->AttackStart(_unitCaster);
                        break;
                    case 45836: //Summon Blue Drake
                        //summon->SetSummoner(m_caster);
                        if (m_caster->GetTypeId() == TYPEID_PLAYER)
                            m_caster->CastSpell((Unit*)nullptr, 45839, true);
                        m_caster->CastSpell((Unit*)nullptr, 45838, true);
                        summon->CastSpell((Unit*)nullptr, 45838, true);
                        break;
                    }
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
                        m_caster->ToPlayer()->KilledMonsterCredit(15893, ObjectGuid::Empty);
                    break;
                case 26333:
                case 26334:
                case 26336:
                case 26337:
                case 26338:
                case 26335:
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        m_caster->ToPlayer()->KilledMonsterCredit(15893, ObjectGuid::Empty);
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
                        m_caster->ToPlayer()->KilledMonsterCredit(15894, ObjectGuid::Empty);
                    break;
                default:
                    break;
            }

            //lolhack section 3
            switch (entry)
            {
                case 23369: // Whirling Blade (no idea what this is for)
                {
                    if (_unitCaster && _unitCaster->GetVictim())
                        summon->AI()->AttackStart(_unitCaster->GetVictim());
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
        if (!_unitCaster)
            return;

        summon = m_caster->GetMap()->SummonCreature(entry, *destTarget, properties, duration, _unitCaster, m_spellInfo->Id);
        summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, m_spellInfo->Id);
        break;
#ifdef LICH_KING
    case SUMMON_CATEGORY_VEHICLE:
        if (!_unitCaster)
            return;

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

        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);

        // if we have small value, it indicates seat position
        if (basePoints > 0 && basePoints < MAX_VEHICLE_SEATS)
            args.AddSpellBP0(basePoints);

        _unitCaster->CastSpell(summon, spellId, args);

        uint32 faction = properties->Faction;
        if (!faction)
            faction = _unitCaster->GetFaction();

        summon->SetFaction(faction);
        break;
#endif
    }

    if (summon)
    {
        summon->SetCreatorGUID(caster->GetGUID());
        ExecuteLogEffectSummonObject(effIndex, summon);
    }
}

void Spell::SummonGuardian(uint32 i, uint32 entry, SummonPropertiesEntry const* properties, uint32 numGuardians)
{
    if (!_unitCaster)
        return;

    if (_unitCaster->IsTotem())
        _unitCaster = _unitCaster->ToTotem()->GetOwner();

    // in another case summon new
    uint8 level = _unitCaster->GetLevel();

    // level of pet summoned using engineering item based at engineering skill level
    if (m_CastItem && _unitCaster->GetTypeId() == TYPEID_PLAYER)
        if (ItemTemplate const* proto = m_CastItem->GetTemplate())
            if (proto->RequiredSkill == SKILL_ENGINEERING)
                if (uint16 skill202 = _unitCaster->ToPlayer()->GetSkillValue(SKILL_ENGINEERING))
                    level = skill202 / 5;

    float radius = 5.0f;
    int32 duration = m_spellInfo->GetDuration();

    if (Player* modOwner = _unitCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);

    //TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Map* map = _unitCaster->GetMap();

    for (uint32 count = 0; count < numGuardians; ++count)
    {
        Position pos;
        if (numGuardians == 1)
            pos = *destTarget;
        else
        {
            // sun: new angle calculation. Dest target is ignored (observed on spell 33966)
            // Example for two:
            //    X       (leader)
            // x     x 
            float x, y, z;
            float newAngle = Trinity::NormalizeOrientation(M_PI / 2.0f * (numGuardians / 2.0f) * (count+1) + M_PI / 4.0f);
            m_caster->GetClosePoint(x, y, z, m_caster->GetCombatReach(), radius, newAngle);
            pos.Relocate(x, y, z);
        }

        TempSummon* summon = map->SummonCreature(entry, pos, properties, duration, _unitCaster, m_spellInfo->Id);
        if (!summon)
            return;

        if (summon->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
            ((Guardian*)summon)->InitStatsForLevel(level);

        if (properties && properties->Category == SUMMON_CATEGORY_ALLY)
            summon->SetFaction(_unitCaster->GetFaction());

        if (summon->HasUnitTypeMask(UNIT_MASK_MINION) && m_targets.HasDst())
            ((Minion*)summon)->SetFollowAngle(_unitCaster->GetRelativeAngle(summon)); //sun: fixed angle

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
        

        //sunstrider: removed, this breaks any aura added in AI inits/reset scripting hooks. Instead, trigger move follow here.
        //summon->AI()->EnterEvadeMode();
        if (Unit* owner = summon->GetCharmerOrOwner())
        {
            summon->GetMotionMaster()->Clear();
            summon->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, summon->GetFollowAngle(), MOTION_SLOT_ACTIVE);
        }

        ExecuteLogEffectSummonObject(i, summon);
    }
}
