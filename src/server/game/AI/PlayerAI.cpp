
#include "PlayerAI.h"
#include "CommonHelpers.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"

PlayerAI::PlayerAI(Player* player) : UnitAI(player), me(player),
    _selfSpec(Trinity::Helpers::Entity::GetPlayerSpecialization(player)),
    _isSelfHealer(Trinity::Helpers::Entity::IsPlayerHealer(player)),
    _isSelfRangedAttacker(Trinity::Helpers::Entity::IsPlayerRangedAttacker(player))
{
}

uint8 PlayerAI::GetSpec(Player const * who) const
{
    return (!who || who == me) ? _selfSpec : Trinity::Helpers::Entity::GetPlayerSpecialization(who);
}

bool PlayerAI::IsHealer(Player const * who) const
{
    return (!who || who == me) ? _isSelfHealer : Trinity::Helpers::Entity::IsPlayerHealer(who);
}

bool PlayerAI::IsRangedAttacker(Player const * who) const
{
    return (!who || who == me) ? _isSelfRangedAttacker : Trinity::Helpers::Entity::IsPlayerRangedAttacker(who);
}

PlayerAI::TargetedSpell PlayerAI::VerifySpellCast(uint32 spellId, Unit* target)
{
    // Find highest spell rank that we know
    uint32 knownRank, nextRank;
    if (me->HasSpell(spellId))
    {
        // this will save us some lookups if the player has the highest rank (expected case)
        knownRank = spellId;
        nextRank = sSpellMgr->GetNextSpellInChain(knownRank);
    }
    else
    {
        knownRank = 0;
        nextRank = sSpellMgr->GetFirstSpellInChain(spellId);
    }

    while (nextRank && me->HasSpell(nextRank))
    {
        knownRank = nextRank;
        nextRank = sSpellMgr->GetNextSpellInChain(knownRank);
    }

    if (!knownRank)
        return {};

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(knownRank);
    if (!spellInfo)
        return {};

    if (me->GetSpellHistory()->HasGlobalCooldown(spellInfo))
        return {};

    auto spell = new Spell(me, spellInfo, TRIGGERED_NONE);
    if (spell->CanAutoCast(target))
        return{ spell, target };

    delete spell;
    return {};
}

PlayerAI::TargetedSpell PlayerAI::VerifySpellCast(uint32 spellId, SpellTarget target)
{
    Unit* pTarget = nullptr;
    switch (target)
    {
        case TARGET_NONE:
            break;
        case TARGET_VICTIM:
            pTarget = me->GetVictim();
            if (!pTarget)
                return {};
            break;
        case TARGET_CHARMER:
            pTarget = me->GetCharmer();
            if (!pTarget)
                return {};
            break;
        case TARGET_SELF:
            pTarget = me;
            break;
    }

    return VerifySpellCast(spellId, pTarget);
}

PlayerAI::TargetedSpell PlayerAI::SelectSpellCast(PossibleSpellVector& spells)
{
    uint32 totalWeights = 0;
    for (PossibleSpell const& wSpell : spells)
        totalWeights += wSpell.second;

    TargetedSpell selected;
    uint32 randNum = urand(0, totalWeights - 1);
    for (PossibleSpell const& wSpell : spells)
    {
        if (selected)
        {
            delete wSpell.first.first;
            continue;
        }

        if (randNum < wSpell.second)
            selected = wSpell.first;
        else
        {
            randNum -= wSpell.second;
            delete wSpell.first.first;
        }
    }

    spells.clear();
    return selected;
}

void PlayerAI::DoCastAtTarget(TargetedSpell spell)
{
    SpellCastTargets targets;
    targets.SetUnitTarget(spell.second);
    spell.first->prepare(targets);
}

void PlayerAI::DoRangedAttackIfReady()
{
    if (me->HasUnitState(UNIT_STATE_CASTING))
        return;

    if (!me->IsAttackReady(RANGED_ATTACK))
        return;

    Unit* victim = me->GetVictim();
    if (!victim)
        return;

    uint32 rangedAttackSpell = 0;

    Item const* rangedItem = me->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (ItemTemplate const* rangedTemplate = rangedItem ? rangedItem->GetTemplate() : nullptr)
    {
        switch (rangedTemplate->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_GUN:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                rangedAttackSpell = SPELL_SHOOT;
                break;
            case ITEM_SUBCLASS_WEAPON_THROWN:
                rangedAttackSpell = SPELL_THROW;
                break;
            case ITEM_SUBCLASS_WEAPON_WAND:
                rangedAttackSpell = SPELL_SHOOT_WAND;
                break;
        }
    }

    if (!rangedAttackSpell)
        return;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(rangedAttackSpell);
    if (!spellInfo)
        return;
 
    Spell* spell = new Spell(me, spellInfo, TRIGGERED_CAST_DIRECTLY);
    if (spell->CheckPetCast(victim) != SPELL_CAST_OK)
    {
        delete spell;
        return;
    }
 
    SpellCastTargets targets;
    targets.SetUnitTarget(victim);
    spell->prepare(targets);

    me->CastSpell(victim, rangedAttackSpell, TRIGGERED_CAST_DIRECTLY);
    me->ResetAttackTimer(RANGED_ATTACK);
}

void PlayerAI::DoAutoAttackIfReady()
{
    if (IsRangedAttacker())
        DoRangedAttackIfReady();
    else
        DoMeleeAttackIfReady();
}

void PlayerAI::CancelAllShapeshifts()
{
#ifdef LICH_KING
    std::list<AuraEffect*> const& shapeshiftAuras = me->GetAuraEffectsByType(SPELL_AURA_MOD_SHAPESHIFT);
    std::set<Aura*> removableShapeshifts;
    for (AuraEffect* auraEff : shapeshiftAuras)
    {
        Aura* aura = auraEff->GetBase();
        if (!aura)
            continue;
        SpellInfo const* auraInfo = aura->GetSpellInfo();
        if (!auraInfo)
            continue;
        if (auraInfo->HasAttribute(SPELL_ATTR0_CANT_CANCEL))
            continue;
        if (!auraInfo->IsPositive() || auraInfo->IsPassive())
            continue;
        removableShapeshifts.insert(aura);
    }

    for (Aura* aura : removableShapeshifts)
        me->RemoveOwnedAura(aura, AURA_REMOVE_BY_CANCEL);
#else
    //quick hack fix for bc :
    me->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
#endif
}

struct ValidTargetSelectPredicate
{
    ValidTargetSelectPredicate(UnitAI const* ai) : _ai(ai) { }
    UnitAI const* const _ai;

    bool operator()(Unit const* target) const
    {
        return _ai->CanAIAttack(target);
    }
};

bool SimpleCharmedPlayerAI::CanAIAttack(Unit const* who) const
{
    if (!me->IsValidAttackTarget(who) || who->HasBreakableByDamageCrowdControlAura())
        return false;
    if (Unit* charmer = me->GetCharmer())
        if (!charmer->IsValidAttackTarget(who))
            return false;
    return UnitAI::CanAIAttack(who);
}

Unit* SimpleCharmedPlayerAI::SelectAttackTarget() const
{
    if (Unit* charmer = me->GetCharmer())
    {
        if (UnitAI* charmerAI = charmer->GetAI())
            return charmerAI->SelectTarget(SELECT_TARGET_RANDOM, 0, ValidTargetSelectPredicate(this));
        return charmer->GetVictim();
    }
    return nullptr;
}

PlayerAI::TargetedSpell SimpleCharmedPlayerAI::SelectAppropriateCastForSpec()
{
    PossibleSpellVector spells;

    switch (me->GetClass())
    {
        case CLASS_WARRIOR:
            if (!me->IsWithinMeleeRange(me->GetVictim()))
            {
                VerifyAndPushSpellCast(spells, SPELL_CHARGE, TARGET_VICTIM, 15);
                VerifyAndPushSpellCast(spells, SPELL_INTERCEPT, TARGET_VICTIM, 10);
            }
            VerifyAndPushSpellCast(spells, SPELL_ENRAGED_REGEN, TARGET_NONE, 3);
            VerifyAndPushSpellCast(spells, SPELL_INTIMIDATING_SHOUT, TARGET_VICTIM, 4);
            if (me->GetVictim() && me->GetVictim()->HasUnitState(UNIT_STATE_CASTING))
            {
                VerifyAndPushSpellCast(spells, SPELL_PUMMEL, TARGET_VICTIM, 15);
                VerifyAndPushSpellCast(spells, SPELL_SHIELD_BASH, TARGET_VICTIM, 15);
            }
            VerifyAndPushSpellCast(spells, SPELL_BLOODRAGE, TARGET_NONE, 5);
            switch (GetSpec())
            {
                case SPEC_WARRIOR_PROTECTION:
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_SHOCKWAVE, TARGET_VICTIM, 3);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_CONCUSSION_BLOW, TARGET_VICTIM, 5);
                    VerifyAndPushSpellCast(spells, SPELL_DISARM, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_LAST_STAND, TARGET_NONE, 5);
                    VerifyAndPushSpellCast(spells, SPELL_SHIELD_BLOCK, TARGET_NONE, 1);
                    VerifyAndPushSpellCast(spells, SPELL_SHIELD_SLAM, TARGET_VICTIM, 4);
                    VerifyAndPushSpellCast(spells, SPELL_SHIELD_WALL, TARGET_NONE, 5);
                    VerifyAndPushSpellCast(spells, SPELL_SPELL_REFLECTION, TARGET_NONE, 3);
                    VerifyAndPushSpellCast(spells, SPELL_DEVASTATE, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_REND, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_THUNDER_CLAP, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_DEMO_SHOUT, TARGET_VICTIM, 1);
                    break;
                case SPEC_WARRIOR_ARMS:
                    VerifyAndPushSpellCast(spells, SPELL_SWEEPING_STRIKES, TARGET_NONE, 2);
                    VerifyAndPushSpellCast(spells, SPELL_MORTAL_STRIKE, TARGET_VICTIM, 5);
                    VerifyAndPushSpellCast(spells, SPELL_BLADESTORM, TARGET_NONE, 10);
                    VerifyAndPushSpellCast(spells, SPELL_REND, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_RETALIATION, TARGET_NONE, 3);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_SHATTERING_THROW, TARGET_VICTIM, 3);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_SWEEPING_STRIKES, TARGET_NONE, 5);
                    VerifyAndPushSpellCast(spells, SPELL_THUNDER_CLAP, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_EXECUTE, TARGET_VICTIM, 15);
                    break;
                case SPEC_WARRIOR_FURY:
                    VerifyAndPushSpellCast(spells, SPELL_DEATH_WISH, TARGET_NONE, 10);
                    VerifyAndPushSpellCast(spells, SPELL_BLOODTHIRST, TARGET_VICTIM, 4);
                    VerifyAndPushSpellCast(spells, SPELL_DEMO_SHOUT, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_EXECUTE, TARGET_VICTIM, 15);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_HEROIC_FURY, TARGET_NONE, 5);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_RECKLESSNESS, TARGET_NONE, 8);
                    VerifyAndPushSpellCast(spells, SPELL_PIERCING_HOWL, TARGET_VICTIM, 2);
                    break;
            }
            break;
        case CLASS_PALADIN:
            VerifyAndPushSpellCast(spells, SPELL_AURA_MASTERY, TARGET_NONE, 3);
            VerifyAndPushSpellCast(spells, SPELL_LAY_ON_HANDS, TARGET_CHARMER, 8);
            VerifyAndPushSpellCast(spells, SPELL_BLESSING_OF_MIGHT, TARGET_CHARMER, 8);
            VerifyAndPushSpellCast(spells, SPELL_AVENGING_WRATH, TARGET_NONE, 5);
            VerifyAndPushSpellCast(spells, SPELL_DIVINE_PROTECTION, TARGET_NONE, 4);
            VerifyAndPushSpellCast(spells, SPELL_DIVINE_SHIELD, TARGET_NONE, 2);
            VerifyAndPushSpellCast(spells, SPELL_HAMMER_OF_JUSTICE, TARGET_VICTIM, 6);
            VerifyAndPushSpellCast(spells, SPELL_HAND_OF_FREEDOM, TARGET_SELF, 3);
            VerifyAndPushSpellCast(spells, SPELL_HAND_OF_PROTECTION, TARGET_SELF, 1);
            if (Creature* creatureCharmer = ObjectAccessor::GetCreature(*me, me->GetCharmerGUID()))
            {
                if (creatureCharmer->IsDungeonBoss() || creatureCharmer->IsWorldBoss())
                    VerifyAndPushSpellCast(spells, SPELL_HAND_OF_SACRIFICE, creatureCharmer, 10);
                else
                    VerifyAndPushSpellCast(spells, SPELL_HAND_OF_PROTECTION, creatureCharmer, 3);
            }

            switch (GetSpec())
            {
                case SPEC_PALADIN_PROTECTION:
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_HAMMER_OF_RIGHTEOUS, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_DIVINE_SACRIFICE, TARGET_NONE, 2);
                    VerifyAndPushSpellCast(spells, SPELL_SHIELD_OF_RIGHTEOUS, TARGET_VICTIM, 4);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_JUDGEMENT_, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_CONSECRATION, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_HOLY_SHIELD, TARGET_NONE, 1);
                    break;
                case SPEC_PALADIN_HOLY:
                    VerifyAndPushSpellCast(spells, SPELL_HOLY_SHOCK, TARGET_CHARMER, 3);
                    VerifyAndPushSpellCast(spells, SPELL_HOLY_SHOCK, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_FLASH_OF_LIGHT, TARGET_CHARMER, 4);
                    VerifyAndPushSpellCast(spells, SPELL_HOLY_LIGHT, TARGET_CHARMER, 3);
                    VerifyAndPushSpellCast(spells, SPELL_DIVINE_FAVOR, TARGET_NONE, 5);
                    VerifyAndPushSpellCast(spells, SPELL_DIVINE_ILLUMINATION, TARGET_NONE, 3);
                    break;
                case SPEC_PALADIN_RETRIBUTION:
                    VerifyAndPushSpellCast(spells, SPELL_CRUSADER_STRIKE, TARGET_VICTIM, 4);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_DIVINE_STORM, TARGET_VICTIM, 5);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_JUDGEMENT_, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_HAMMER_OF_WRATH, TARGET_VICTIM, 5);
                    VerifyAndPushSpellCast(spells, SPELL_RIGHTEOUS_FURY, TARGET_NONE, 2);
                    break;
            }
            break;
        case CLASS_HUNTER:
            VerifyAndPushSpellCast(spells, SPELL_DETERRENCE, TARGET_NONE, 3);
            VerifyAndPushSpellCast(spells, SPELL_EXPLOSIVE_TRAP, TARGET_NONE, 1);
#ifdef LICH_KING
            VerifyAndPushSpellCast(spells, SPELL_FREEZING_ARROW, TARGET_VICTIM, 2);
#endif
            VerifyAndPushSpellCast(spells, SPELL_RAPID_FIRE, TARGET_NONE, 10);
#ifdef LICH_KING
            VerifyAndPushSpellCast(spells, SPELL_KILL_SHOT, TARGET_VICTIM, 10);
#endif
            if (me->GetVictim() && me->GetVictim()->GetPowerType() == POWER_MANA && !me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_VIPER_STING, me->GetGUID()))
                VerifyAndPushSpellCast(spells, SPELL_VIPER_STING, TARGET_VICTIM, 5);

            switch (GetSpec())
            {
                case SPEC_HUNTER_BEAST_MASTERY:
                    VerifyAndPushSpellCast(spells, SPELL_AIMED_SHOT, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_ARCANE_SHOT, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_STEADY_SHOT, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_MULTI_SHOT, TARGET_VICTIM, 2);
                    break;
                case SPEC_HUNTER_MARKSMANSHIP:
                    VerifyAndPushSpellCast(spells, SPELL_AIMED_SHOT, TARGET_VICTIM, 2);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_CHIMERA_SHOT, TARGET_VICTIM, 5);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_ARCANE_SHOT, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_STEADY_SHOT, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_READINESS, TARGET_NONE, 10);
                    VerifyAndPushSpellCast(spells, SPELL_SILENCING_SHOT, TARGET_VICTIM, 5);
                    break;
                case SPEC_HUNTER_SURVIVAL:
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_EXPLOSIVE_SHOT, TARGET_VICTIM, 8);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_BLACK_ARROW, TARGET_VICTIM, 5);
                    VerifyAndPushSpellCast(spells, SPELL_MULTI_SHOT, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_STEADY_SHOT, TARGET_VICTIM, 1);
                    break;
            }
            break;
        case CLASS_ROGUE:
        {
#ifdef LICH_KING
            VerifyAndPushSpellCast(spells, SPELL_DISMANTLE, TARGET_VICTIM, 8);
#endif
            VerifyAndPushSpellCast(spells, SPELL_EVASION, TARGET_NONE, 8);
            VerifyAndPushSpellCast(spells, SPELL_VANISH, TARGET_NONE, 4);
            VerifyAndPushSpellCast(spells, SPELL_BLIND, TARGET_VICTIM, 2);
            VerifyAndPushSpellCast(spells, SPELL_CLOAK_OF_SHADOWS, TARGET_NONE, 2);

            uint32 builder = 0, finisher = 0;
            switch (GetSpec())
            {
                case SPEC_ROGUE_ASSASSINATION:
                    builder = SPELL_MUTILATE, finisher = SPELL_ENVENOM;
                    VerifyAndPushSpellCast(spells, SPELL_COLD_BLOOD, TARGET_NONE, 20);
                    break;
                case SPEC_ROGUE_COMBAT:
                    builder = SPELL_SINISTER_STRIKE, finisher = SPELL_EVISCERATE;
                    VerifyAndPushSpellCast(spells, SPELL_ADRENALINE_RUSH, TARGET_NONE, 6);
                    VerifyAndPushSpellCast(spells, SPELL_BLADE_FLURRY, TARGET_NONE, 5);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_KILLING_SPREE, TARGET_NONE, 25);
#endif
                    break;
                case SPEC_ROGUE_SUBLETY:
                    builder = SPELL_HEMORRHAGE, finisher = SPELL_EVISCERATE;
                    VerifyAndPushSpellCast(spells, SPELL_PREPARATION, TARGET_NONE, 10);
                    if (!me->IsWithinMeleeRange(me->GetVictim()))
                        VerifyAndPushSpellCast(spells, SPELL_SHADOWSTEP, TARGET_VICTIM, 25);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_SHADOW_DANCE, TARGET_NONE, 10);
#endif
                    break;
            }

            if (Unit* victim = me->GetVictim())
            {
                if (victim->HasUnitState(UNIT_STATE_CASTING))
                    VerifyAndPushSpellCast(spells, SPELL_KICK, TARGET_VICTIM, 25);

                uint8 const cp = (me->GetComboTarget() == victim->GetGUID()) ? me->GetComboPoints() : 0;
                if (cp >= 4)
                    VerifyAndPushSpellCast(spells, finisher, TARGET_VICTIM, 10);
                if (cp <= 4)
                    VerifyAndPushSpellCast(spells, builder, TARGET_VICTIM, 5);
            }
            break;
        }
        case CLASS_PRIEST:
            VerifyAndPushSpellCast(spells, SPELL_FEAR_WARD, TARGET_SELF, 2);
            VerifyAndPushSpellCast(spells, SPELL_POWER_WORD_FORT, TARGET_CHARMER, 1);
            VerifyAndPushSpellCast(spells, SPELL_DIVINE_SPIRIT, TARGET_CHARMER, 1);
            VerifyAndPushSpellCast(spells, SPELL_SHADOW_PROTECTION, TARGET_CHARMER, 2);
#ifdef LICH_KING
            VerifyAndPushSpellCast(spells, SPELL_DIVINE_HYMN, TARGET_NONE, 5);
            VerifyAndPushSpellCast(spells, SPELL_HYMN_OF_HOPE, TARGET_NONE, 5);
#endif
            VerifyAndPushSpellCast(spells, SPELL_SHADOW_WORD_DEATH, TARGET_VICTIM, 1);
            VerifyAndPushSpellCast(spells, SPELL_PSYCHIC_SCREAM, TARGET_VICTIM, 3);
            switch (GetSpec())
            {
                case SPEC_PRIEST_DISCIPLINE:
                    VerifyAndPushSpellCast(spells, SPELL_POWER_WORD_SHIELD, TARGET_CHARMER, 3);
                    VerifyAndPushSpellCast(spells, SPELL_INNER_FOCUS, TARGET_NONE, 3);
                    VerifyAndPushSpellCast(spells, SPELL_PAIN_SUPPRESSION, TARGET_CHARMER, 15);
                    VerifyAndPushSpellCast(spells, SPELL_POWER_INFUSION, TARGET_CHARMER, 10);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_PENANCE, TARGET_CHARMER, 3);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_FLASH_HEAL, TARGET_CHARMER, 1);
                    break;
                case SPEC_PRIEST_HOLY:
                    VerifyAndPushSpellCast(spells, SPELL_DESPERATE_PRAYER, TARGET_NONE, 3);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_GUARDIAN_SPIRIT, TARGET_CHARMER, 5);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_FLASH_HEAL, TARGET_CHARMER, 1);
                    VerifyAndPushSpellCast(spells, SPELL_RENEW, TARGET_CHARMER, 3);
                    break;
                case SPEC_PRIEST_SHADOW:
                    if (!me->HasAura(SPELL_SHADOWFORM))
                    {
                        VerifyAndPushSpellCast(spells, SPELL_SHADOWFORM, TARGET_NONE, 100);
                        break;
                    }
                    if (Unit* victim = me->GetVictim())
                    {
                        if (!victim->GetAuraApplicationOfRankedSpell(SPELL_VAMPIRIC_TOUCH, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_VAMPIRIC_TOUCH, TARGET_VICTIM, 4);
                        if (!victim->GetAuraApplicationOfRankedSpell(SPELL_SHADOW_WORD_PAIN, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_SHADOW_WORD_PAIN, TARGET_VICTIM, 3);
#ifdef LICH_KING
                        if (!victim->GetAuraApplicationOfRankedSpell(SPELL_DEVOURING_PLAGUE, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_DEVOURING_PLAGUE, TARGET_VICTIM, 4);
#endif
                    }
                    VerifyAndPushSpellCast(spells, SPELL_MIND_BLAST, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_MIND_FLAY, TARGET_VICTIM, 2);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_DISPERSION, TARGET_NONE, 10);
#endif
                    break;
            }
            break;
#ifdef LICH_KING
        case CLASS_DEATH_KNIGHT:
        {
            if (!me->IsWithinMeleeRange(me->GetVictim()))
                VerifyAndPushSpellCast(spells, SPELL_DEATH_GRIP, TARGET_VICTIM, 25);
            VerifyAndPushSpellCast(spells, SPELL_STRANGULATE, TARGET_VICTIM, 15);
            VerifyAndPushSpellCast(spells, SPELL_EMPOWER_RUNE_WEAP, TARGET_NONE, 5);
            VerifyAndPushSpellCast(spells, SPELL_ICEBORN_FORTITUDE, TARGET_NONE, 15);
            VerifyAndPushSpellCast(spells, SPELL_ANTI_MAGIC_SHELL, TARGET_NONE, 10);

            bool hasFF = false, hasBP = false;
            if (Unit* victim = me->GetVictim())
            {
                if (victim->HasUnitState(UNIT_STATE_CASTING))
                    VerifyAndPushSpellCast(spells, SPELL_MIND_FREEZE, TARGET_VICTIM, 25);

                hasFF = !!victim->GetAuraApplicationOfRankedSpell(AURA_FROST_FEVER, me->GetGUID()), hasBP = !!victim->GetAuraApplicationOfRankedSpell(AURA_BLOOD_PLAGUE, me->GetGUID());
                if (hasFF && hasBP)
                    VerifyAndPushSpellCast(spells, SPELL_PESTILENCE, TARGET_VICTIM, 3);
                if (!hasFF)
                    VerifyAndPushSpellCast(spells, SPELL_ICY_TOUCH, TARGET_VICTIM, 4);
                if (!hasBP)
                    VerifyAndPushSpellCast(spells, SPELL_PLAGUE_STRIKE, TARGET_VICTIM, 4);
            }
            switch (GetSpec())
            {
                case SPEC_DEATH_KNIGHT_BLOOD:
                    VerifyAndPushSpellCast(spells, SPELL_RUNE_TAP, TARGET_NONE, 2);
                    VerifyAndPushSpellCast(spells, SPELL_HYSTERIA, TARGET_SELF, 5);
                    if (Creature* creatureCharmer = ObjectAccessor::GetCreature(*me, me->GetCharmerGUID()))
                        if (!creatureCharmer->IsDungeonBoss() && !creatureCharmer->IsWorldBoss())
                            VerifyAndPushSpellCast(spells, SPELL_HYSTERIA, creatureCharmer, 15);
                    VerifyAndPushSpellCast(spells, SPELL_HEART_STRIKE, TARGET_VICTIM, 2);
                    if (hasFF && hasBP)
                        VerifyAndPushSpellCast(spells, SPELL_DEATH_STRIKE, TARGET_VICTIM, 8);
                    VerifyAndPushSpellCast(spells, SPELL_DEATH_COIL_DK, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_MARK_OF_BLOOD, TARGET_VICTIM, 20);
                    VerifyAndPushSpellCast(spells, SPELL_VAMPIRIC_BLOOD, TARGET_NONE, 10);
                    break;
                case SPEC_DEATH_KNIGHT_FROST:
                    if (hasFF && hasBP)
                        VerifyAndPushSpellCast(spells, SPELL_OBLITERATE, TARGET_VICTIM, 5);
                    VerifyAndPushSpellCast(spells, SPELL_HOWLING_BLAST, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_UNBREAKABLE_ARMOR, TARGET_NONE, 10);
                    VerifyAndPushSpellCast(spells, SPELL_DEATHCHILL, TARGET_NONE, 10);
                    VerifyAndPushSpellCast(spells, SPELL_FROST_STRIKE, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_BLOOD_STRIKE, TARGET_VICTIM, 1);
                    break;
                case SPEC_DEATH_KNIGHT_UNHOLY:
                    if (hasFF && hasBP)
                        VerifyAndPushSpellCast(spells, SPELL_SCOURGE_STRIKE, TARGET_VICTIM, 5);
                    VerifyAndPushSpellCast(spells, SPELL_DEATH_AND_DECAY, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_ANTI_MAGIC_ZONE, TARGET_NONE, 8);
                    VerifyAndPushSpellCast(spells, SPELL_SUMMON_GARGOYLE, TARGET_VICTIM, 7);
                    VerifyAndPushSpellCast(spells, SPELL_BLOOD_STRIKE, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_DEATH_COIL_DK, TARGET_VICTIM, 3);
                    break;
            }
            break;
        }
#endif
        case CLASS_SHAMAN:
            VerifyAndPushSpellCast(spells, SPELL_HEROISM, TARGET_NONE, 25);
            VerifyAndPushSpellCast(spells, SPELL_BLOODLUST, TARGET_NONE, 25);
            VerifyAndPushSpellCast(spells, SPELL_GROUNDING_TOTEM, TARGET_NONE, 2);
            switch (GetSpec())
            {
                case SPEC_SHAMAN_RESTORATION:
                    if (Unit* charmer = me->GetCharmer())
                        if (!charmer->GetAuraApplicationOfRankedSpell(SPELL_EARTH_SHIELD, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_EARTH_SHIELD, charmer, 2);
                    if (me->HasAura(SPELL_SHA_NATURE_SWIFT))
                        VerifyAndPushSpellCast(spells, SPELL_HEALING_WAVE, TARGET_CHARMER, 20);
                    else
                        VerifyAndPushSpellCast(spells, SPELL_LESSER_HEAL_WAVE, TARGET_CHARMER, 1);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_TIDAL_FORCE, TARGET_NONE, 4);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_SHA_NATURE_SWIFT, TARGET_NONE, 4);
                    VerifyAndPushSpellCast(spells, SPELL_MANA_TIDE_TOTEM, TARGET_NONE, 3);
                    break;
                case SPEC_SHAMAN_ELEMENTAL:
                    if (Unit* victim = me->GetVictim())
                    {
#ifdef LICH_KING
                        if (victim->GetAuraOfRankedSpell(SPELL_FLAME_SHOCK, GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_LAVA_BURST, TARGET_VICTIM, 5);
                        else
#endif
                            VerifyAndPushSpellCast(spells, SPELL_FLAME_SHOCK, TARGET_VICTIM, 3);
                    }
                    VerifyAndPushSpellCast(spells, SPELL_CHAIN_LIGHTNING, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_LIGHTNING_BOLT, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_ELEMENTAL_MASTERY, TARGET_VICTIM, 5);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_THUNDERSTORM, TARGET_NONE, 3);
#endif
                    break;
                case SPEC_SHAMAN_ENHANCEMENT:
#ifdef LICH_KING
                    if (Aura const* maelstrom = me->GetAura(AURA_MAELSTROM_WEAPON))
                        if (maelstrom->GetStackAmount() == 5)
                            VerifyAndPushSpellCast(spells, SPELL_LIGHTNING_BOLT, TARGET_VICTIM, 5);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_STORMSTRIKE, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_EARTH_SHOCK, TARGET_VICTIM, 2);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_LAVA_LASH, TARGET_VICTIM, 1);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_SHAMANISTIC_RAGE, TARGET_NONE, 10);
                    break;
            }
            break;
        case CLASS_MAGE:
            if (me->GetVictim() && me->GetVictim()->HasUnitState(UNIT_STATE_CASTING))
                VerifyAndPushSpellCast(spells, SPELL_COUNTERSPELL, TARGET_VICTIM, 25);
            VerifyAndPushSpellCast(spells, SPELL_DAMPEN_MAGIC, TARGET_CHARMER, 2);
            VerifyAndPushSpellCast(spells, SPELL_EVOCATION, TARGET_NONE, 3);
            VerifyAndPushSpellCast(spells, SPELL_MANA_SHIELD, TARGET_NONE, 1);
#ifdef LICH_KING
            VerifyAndPushSpellCast(spells, SPELL_MIRROR_IMAGE, TARGET_NONE, 3);
#endif
            VerifyAndPushSpellCast(spells, SPELL_SPELLSTEAL, TARGET_VICTIM, 2);
            VerifyAndPushSpellCast(spells, SPELL_ICE_BLOCK, TARGET_NONE, 1);
            VerifyAndPushSpellCast(spells, SPELL_ICY_VEINS, TARGET_NONE, 3);
            switch (GetSpec())
            {
                case SPEC_MAGE_ARCANE:
                    if (Aura* abAura = me->GetAura(AURA_ARCANE_BLAST))
                        if (abAura->GetStackAmount() >= 3)
                            VerifyAndPushSpellCast(spells, SPELL_ARCANE_MISSILES, TARGET_VICTIM, 7);
                    VerifyAndPushSpellCast(spells, SPELL_ARCANE_BLAST, TARGET_VICTIM, 5);
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_ARCANE_BARRAGE, TARGET_VICTIM, 1);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_ARCANE_POWER, TARGET_NONE, 8);
                    VerifyAndPushSpellCast(spells, SPELL_PRESENCE_OF_MIND, TARGET_NONE, 7);
                    break;
                case SPEC_MAGE_FIRE:
#ifdef LICH_KING
                    if (me->GetVictim() && !me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_LIVING_BOMB))
                        VerifyAndPushSpellCast(spells, SPELL_LIVING_BOMB, TARGET_VICTIM, 3);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_COMBUSTION, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_FIREBALL, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_FIRE_BLAST, TARGET_VICTIM, 1);
                    VerifyAndPushSpellCast(spells, SPELL_DRAGONS_BREATH, TARGET_VICTIM, 2);
                    VerifyAndPushSpellCast(spells, SPELL_BLAST_WAVE, TARGET_VICTIM, 1);
                    break;
                case SPEC_MAGE_FROST:
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_DEEP_FREEZE, TARGET_VICTIM, 10);
                    VerifyAndPushSpellCast(spells, SPELL_FROST_NOVA, TARGET_VICTIM, 3);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_FROSTBOLT, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_COLD_SNAP, TARGET_VICTIM, 5);
                    if (me->GetVictim() && me->GetVictim()->HasAuraState(AURA_STATE_FROZEN, nullptr, me))
                        VerifyAndPushSpellCast(spells, SPELL_ICE_LANCE, TARGET_VICTIM, 5);
                    break;
            }
            break;
        case CLASS_WARLOCK:
            VerifyAndPushSpellCast(spells, SPELL_DEATH_COIL_W, TARGET_VICTIM, 2);
            VerifyAndPushSpellCast(spells, SPELL_FEAR, TARGET_VICTIM, 2);
            VerifyAndPushSpellCast(spells, SPELL_SEED_OF_CORRUPTION, TARGET_VICTIM, 4);
            VerifyAndPushSpellCast(spells, SPELL_HOWL_OF_TERROR, TARGET_NONE, 2);
            if (me->GetVictim() && !me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_CORRUPTION, me->GetGUID()))
                VerifyAndPushSpellCast(spells, SPELL_CORRUPTION, TARGET_VICTIM, 10);
            switch (GetSpec())
            {
                case SPEC_WARLOCK_AFFLICTION:
                    if (Unit* victim = me->GetVictim())
                    {
                        VerifyAndPushSpellCast(spells, SPELL_SHADOW_BOLT, TARGET_VICTIM, 7);
                        if (!victim->GetAuraApplicationOfRankedSpell(SPELL_UNSTABLE_AFFLICTION, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_UNSTABLE_AFFLICTION, TARGET_VICTIM, 8);
#ifdef LICH_KING
                        if (!victim->GetAuraApplicationOfRankedSpell(SPELL_HAUNT, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_HAUNT, TARGET_VICTIM, 8);
#endif
                        if (!victim->GetAuraApplicationOfRankedSpell(SPELL_CURSE_OF_AGONY, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_CURSE_OF_AGONY, TARGET_VICTIM, 4);
                        if (victim->HealthBelowPct(25))
                            VerifyAndPushSpellCast(spells, SPELL_DRAIN_SOUL, TARGET_VICTIM, 100);
                    }
                    break;
                case SPEC_WARLOCK_DEMONOLOGY:
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_METAMORPHOSIS, TARGET_NONE, 15);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_SHADOW_BOLT, TARGET_VICTIM, 7);
#ifdef LICH_KING
                    if (me->HasAura(AURA_DECIMATION))
                        VerifyAndPushSpellCast(spells, SPELL_SOUL_FIRE, TARGET_VICTIM, 100);
      
                    if (me->HasAura(SPELL_METAMORPHOSIS))
                    {
                        VerifyAndPushSpellCast(spells, SPELL_IMMOLATION_AURA, TARGET_NONE, 30);
                        if (!me->IsWithinMeleeRange(me->GetVictim()))
                            VerifyAndPushSpellCast(spells, SPELL_DEMON_CHARGE, TARGET_VICTIM, 20);
                    }
#endif

                    if (me->GetVictim() && !me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_IMMOLATE, me->GetGUID()))
                        VerifyAndPushSpellCast(spells, SPELL_IMMOLATE, TARGET_VICTIM, 5);
#ifdef LICH_KING
                    if (me->HasAura(AURA_MOLTEN_CORE))
                        VerifyAndPushSpellCast(spells, SPELL_INCINERATE, TARGET_VICTIM, 10);
#endif
                    break;
                case SPEC_WARLOCK_DESTRUCTION:
                    if (me->GetVictim() && !me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_IMMOLATE, me->GetGUID()))
                        VerifyAndPushSpellCast(spells, SPELL_IMMOLATE, TARGET_VICTIM, 8);
                    if (me->GetVictim() && me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_IMMOLATE, me->GetGUID()))
                        VerifyAndPushSpellCast(spells, SPELL_CONFLAGRATE, TARGET_VICTIM, 8);
                    VerifyAndPushSpellCast(spells, SPELL_SHADOWFURY, TARGET_VICTIM, 5);
                    VerifyAndPushSpellCast(spells, SPELL_CHAOS_BOLT, TARGET_VICTIM, 10);
                    VerifyAndPushSpellCast(spells, SPELL_SHADOWBURN, TARGET_VICTIM, 3);
                    VerifyAndPushSpellCast(spells, SPELL_INCINERATE, TARGET_VICTIM, 7);
                    break;
            }
            break;
        case CLASS_DRUID:
            VerifyAndPushSpellCast(spells, SPELL_INNERVATE, TARGET_CHARMER, 5);
            VerifyAndPushSpellCast(spells, SPELL_BARKSKIN, TARGET_NONE, 5);
            switch (GetSpec())
            {
                case SPEC_DRUID_RESTORATION:
                    if (!me->HasAura(SPELL_TREE_OF_LIFE))
                    {
                        CancelAllShapeshifts();
                        VerifyAndPushSpellCast(spells, SPELL_TREE_OF_LIFE, TARGET_NONE, 100);
                        break;
                    }
                    VerifyAndPushSpellCast(spells, SPELL_TRANQUILITY, TARGET_NONE, 10);
                    VerifyAndPushSpellCast(spells, SPELL_NATURE_SWIFTNESS, TARGET_NONE, 7);
                    if (Creature* creatureCharmer = ObjectAccessor::GetCreature(*me, me->GetCharmerGUID()))
                    {
#ifdef LICH_KING
                        VerifyAndPushSpellCast(spells, SPELL_NOURISH, creatureCharmer, 5);
                        VerifyAndPushSpellCast(spells, SPELL_WILD_GROWTH, creatureCharmer, 5);
#endif
                        if (!creatureCharmer->GetAuraApplicationOfRankedSpell(SPELL_REJUVENATION, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_REJUVENATION, creatureCharmer, 8);
                        if (!creatureCharmer->GetAuraApplicationOfRankedSpell(SPELL_REGROWTH, me->GetGUID()))
                            VerifyAndPushSpellCast(spells, SPELL_REGROWTH, creatureCharmer, 8);
                        uint8 lifebloomStacks = 0;
                        if (Aura const* lifebloom = creatureCharmer->GetAura(SPELL_LIFEBLOOM, me->GetGUID()))
                            lifebloomStacks = lifebloom->GetStackAmount();
                        if (lifebloomStacks < 3)
                            VerifyAndPushSpellCast(spells, SPELL_LIFEBLOOM, creatureCharmer, 5);
                        if (creatureCharmer->GetAuraApplicationOfRankedSpell(SPELL_REJUVENATION) ||
                            creatureCharmer->GetAuraApplicationOfRankedSpell(SPELL_REGROWTH))
                            VerifyAndPushSpellCast(spells, SPELL_SWIFTMEND, creatureCharmer, 10);
                        if (me->HasAura(SPELL_NATURE_SWIFTNESS))
                            VerifyAndPushSpellCast(spells, SPELL_HEALING_TOUCH, creatureCharmer, 100);
                    }
                    break;
                case SPEC_DRUID_BALANCE:
                {
                    if (!me->HasAura(SPELL_MOONKIN_FORM))
                    {
                        CancelAllShapeshifts();
                        VerifyAndPushSpellCast(spells, SPELL_MOONKIN_FORM, TARGET_NONE, 100);
                        break;
                    }
#ifdef LICH_KING
                    uint32 const mainAttackSpell = me->HasAura(AURA_ECLIPSE_LUNAR) ? SPELL_STARFIRE : SPELL_WRATH;
                    VerifyAndPushSpellCast(spells, SPELL_STARFALL, TARGET_NONE, 20);
                    VerifyAndPushSpellCast(spells, mainAttackSpell, TARGET_VICTIM, 10);
#else
                    VerifyAndPushSpellCast(spells, SPELL_WRATH, TARGET_VICTIM, 10);
#endif
                    if (me->GetVictim() && !me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_INSECT_SWARM, me->GetGUID()))
                        VerifyAndPushSpellCast(spells, SPELL_INSECT_SWARM, TARGET_VICTIM, 7);
                    if (me->GetVictim() && !me->GetVictim()->GetAuraApplicationOfRankedSpell(SPELL_MOONFIRE, me->GetGUID()))
                        VerifyAndPushSpellCast(spells, SPELL_MOONFIRE, TARGET_VICTIM, 5);
#ifdef LICH_KING
                    if (me->GetVictim() && me->GetVictim()->HasUnitState(UNIT_STATE_CASTING))
                        VerifyAndPushSpellCast(spells, SPELL_TYPHOON, TARGET_NONE, 15);
#endif
                    break;
                }
                case SPEC_DRUID_FERAL:
                    if (!me->HasAura(SPELL_CAT_FORM))
                    {
                        CancelAllShapeshifts();
                        VerifyAndPushSpellCast(spells, SPELL_CAT_FORM, TARGET_NONE, 100);
                        break;
                    }
#ifdef LICH_KING
                    VerifyAndPushSpellCast(spells, SPELL_BERSERK, TARGET_NONE, 20);
                    VerifyAndPushSpellCast(spells, SPELL_SURVIVAL_INSTINCTS, TARGET_NONE, 15);
#endif
                    VerifyAndPushSpellCast(spells, SPELL_TIGER_FURY, TARGET_NONE, 15);
                    VerifyAndPushSpellCast(spells, SPELL_DASH, TARGET_NONE, 5);
                    if (Unit* victim = me->GetVictim())
                    {
                        uint8 const cp = (me->GetComboTarget() == victim->GetGUID()) ? me->GetComboPoints() : 0;
                        if (victim->HasUnitState(UNIT_STATE_CASTING) && cp >= 1)
                            VerifyAndPushSpellCast(spells, SPELL_MAIM, TARGET_VICTIM, 25);
#ifdef LICH_KING
                        if (!me->IsWithinMeleeRange(victim))
                            VerifyAndPushSpellCast(spells, SPELL_FERAL_CHARGE_CAT, TARGET_VICTIM, 25);
#endif
                        if (cp >= 4)
                            VerifyAndPushSpellCast(spells, SPELL_RIP, TARGET_VICTIM, 50);
                        if (cp <= 4)
                        {
                            VerifyAndPushSpellCast(spells, SPELL_MANGLE_CAT, TARGET_VICTIM, 10);
                            VerifyAndPushSpellCast(spells, SPELL_CLAW, TARGET_VICTIM, 5);
                            if (!victim->GetAuraApplicationOfRankedSpell(SPELL_RAKE, me->GetGUID()))
                                VerifyAndPushSpellCast(spells, SPELL_RAKE, TARGET_VICTIM, 8);
#ifdef LICH_KING
                            if (!me->HasAura(SPELL_SAVAGE_ROAR))
                                VerifyAndPushSpellCast(spells, SPELL_SAVAGE_ROAR, TARGET_NONE, 15);
#endif
                        }
                    }
                    break;
            }
            break;
    }

    return SelectSpellCast(spells);
}

static const float CASTER_CHASE_DISTANCE = 28.0f;
void SimpleCharmedPlayerAI::UpdateAI(const uint32 diff)
{
    Creature* charmer = me->GetCharmer() ? me->GetCharmer()->ToCreature() : nullptr;
    if (!charmer)
        return;

    //kill self if charm aura has infinite duration
    if (charmer->IsInEvadeMode())
    {
        Player::AuraEffectList const& auras = me->GetAuraEffectsByType(SPELL_AURA_MOD_CHARM);
        for (auto aura : auras)
            if (aura->GetCasterGUID() == charmer->GetGUID() && aura->GetBase()->IsPermanent())
            {
                me->KillSelf();
                return;
            }
    }

    if (charmer->IsInCombat())
    {
        Unit* target = me->GetVictim();
        if (!target || !CanAIAttack(target))
        {
            target = SelectAttackTarget();
            if (!target || !CanAIAttack(target))
            {
                if (!_isFollowing)
                {
                    _isFollowing = true;
                    me->AttackStop();
                    me->CastStop();
                    me->StopMoving();
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                }
                return;
            }
            _isFollowing = false;

            if (IsRangedAttacker())
            {
                _chaseCloser = !me->IsWithinLOSInMap(target, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2);
                if (_chaseCloser)
                    AttackStart(target);
                else
                    AttackStartCaster(target, CASTER_CHASE_DISTANCE);
            }
            else
                AttackStart(target);
            _forceFacing = true;
        }

        if (me->IsStopped() && !me->HasUnitState(UNIT_STATE_CANNOT_TURN))
        {
            float targetAngle = me->GetAbsoluteAngle(target);
            if (_forceFacing || fabs(me->GetOrientation() - targetAngle) > 0.4f)
            {
                me->SetFacingTo(targetAngle);
                _forceFacing = false;
            }
        }

        if (_castCheckTimer <= diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                _castCheckTimer = 0;
            else
            {
                if (IsRangedAttacker())
                { // chase to zero if the target isn't in line of sight
                    bool inLOS = me->IsWithinLOSInMap(target, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2);
                    if (_chaseCloser != !inLOS)
                    {
                        _chaseCloser = !inLOS;
                        if (_chaseCloser)
                            AttackStart(target);
                        else
                            AttackStartCaster(target, CASTER_CHASE_DISTANCE);
                    }
                }
                if (TargetedSpell shouldCast = SelectAppropriateCastForSpec())
                    DoCastAtTarget(shouldCast);
                _castCheckTimer = 500;
            }
        }
        else
            _castCheckTimer -= diff;

        DoAutoAttackIfReady();
    }
    else if (!_isFollowing)
    {
        _isFollowing = true;
        me->AttackStop();
        me->CastStop();
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            me->StopMoving();
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, me->GetFollowAngle());
        }
    }
}

void SimpleCharmedPlayerAI::OnCharmed(bool isNew)
{
    if (me->IsCharmed())
    {
        me->CastStop();
        me->AttackStop();
        me->StopMoving();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(0, me->GetPosition(), false); // force re-sync of current position for all clients
    }
    else
    {
        me->CastStop();
        me->AttackStop();
        // @todo only voluntary movement (don't cancel stuff like death grip or charge mid-animation)
        me->GetMotionMaster()->Clear();
        me->StopMoving();
    }
    PlayerAI::OnCharmed(isNew);
}
