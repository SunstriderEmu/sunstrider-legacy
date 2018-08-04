#pragma once

#include "HealthTriggers.h"
#include "GenericTriggers.h"
#include "LootTriggers.h"
#include "../triggers/GenericTriggers.h"
#include "LfgTriggers.h"

namespace ai
{

    class TriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TriggerContext()
        {
            creators["timer"] = &TriggerContext::Timer;
            creators["random"] = &TriggerContext::Random;
            creators["seldom"] = &TriggerContext::seldom;
            creators["often"] = &TriggerContext::often;

            creators["target critical health"] = &TriggerContext::TargetCriticalHealth;

            creators["critical health"] = &TriggerContext::CriticalHealth;
            creators["low health"] = &TriggerContext::LowHealth;
            creators["medium health"] = &TriggerContext::MediumHealth;
            creators["almost full health"] = &TriggerContext::AlmostFullHealth;

            creators["low mana"] = &TriggerContext::LowMana;
            creators["medium mana"] = &TriggerContext::MediumMana;

            creators["party member critical health"] = &TriggerContext::PartyMemberCriticalHealth;
            creators["party member low health"] = &TriggerContext::PartyMemberLowHealth;
            creators["party member medium health"] = &TriggerContext::PartyMemberMediumHealth;
            creators["party member almost full health"] = &TriggerContext::PartyMemberAlmostFullHealth;

            creators["light rage available"] = &TriggerContext::LightRageAvailable;
            creators["medium rage available"] = &TriggerContext::MediumRageAvailable;
            creators["high rage available"] = &TriggerContext::HighRageAvailable;

            creators["light energy available"] = &TriggerContext::LightEnergyAvailable;
            creators["medium energy available"] = &TriggerContext::MediumEnergyAvailable;
            creators["high energy available"] = &TriggerContext::HighEnergyAvailable;

            creators["loot available"] = &TriggerContext::LootAvailable;
            creators["no attackers"] = &TriggerContext::NoAttackers;
            creators["no target"] = &TriggerContext::NoTarget;
            creators["target in sight"] = &TriggerContext::TargetInSight;
            creators["not dps target active"] = &TriggerContext::not_dps_target_active;
            creators["has nearest adds"] = &TriggerContext::has_nearest_adds;
            creators["enemy player is attacking"] = &TriggerContext::enemy_player_is_attacking;

            creators["tank aoe"] = &TriggerContext::TankAoe;
            creators["lose aggro"] = &TriggerContext::LoseAggro;
            creators["has aggro"] = &TriggerContext::HasAggro;

            creators["light aoe"] = &TriggerContext::LightAoe;
            creators["medium aoe"] = &TriggerContext::MediumAoe;
            creators["high aoe"] = &TriggerContext::HighAoe;

            creators["enemy out of melee"] = &TriggerContext::EnemyOutOfMelee;
            creators["enemy out of spell"] = &TriggerContext::EnemyOutOfSpell;
            creators["enemy too close for spell"] = &TriggerContext::enemy_too_close_for_spell;
            creators["enemy too close for melee"] = &TriggerContext::enemy_too_close_for_melee;

            creators["combo points available"] = &TriggerContext::ComboPointsAvailable;

            creators["medium threat"] = &TriggerContext::MediumThreat;

            creators["dead"] = &TriggerContext::Dead;
            creators["party member dead"] = &TriggerContext::PartyMemberDead;
            creators["no pet"] = &TriggerContext::no_pet;
            creators["has attackers"] = &TriggerContext::has_attackers;
            creators["no possible targets"] = &TriggerContext::no_possible_targets;

            creators["no drink"] = &TriggerContext::no_drink;
            creators["no food"] = &TriggerContext::no_food;

            creators["panic"] = &TriggerContext::panic;
            creators["behind target"] = &TriggerContext::behind_target;
            creators["not facing target"] = &TriggerContext::not_facing_target;
            creators["far from master"] = &TriggerContext::far_from_master;
            creators["far from loot target"] = &TriggerContext::far_from_loot_target;
            creators["can loot"] = &TriggerContext::can_loot;
            creators["swimming"] = &TriggerContext::swimming;
            creators["target changed"] = &TriggerContext::target_changed;

            creators["critical aoe heal"] = &TriggerContext::critical_aoe_heal;
            creators["low aoe heal"] = &TriggerContext::low_aoe_heal;
            creators["medium aoe heal"] = &TriggerContext::medium_aoe_heal;
            creators["invalid target"] = &TriggerContext::invalid_target;
            creators["lfg proposal active"] = &TriggerContext::lfg_proposal_active;
        }

    private:
        static std::shared_ptr<Trigger> lfg_proposal_active(PlayerbotAI* ai) { return std::make_shared<LfgProposalActiveTrigger>(ai); }
        static std::shared_ptr<Trigger> invalid_target(PlayerbotAI* ai) { return std::make_shared<InvalidTargetTrigger>(ai); }
        static std::shared_ptr<Trigger> critical_aoe_heal(PlayerbotAI* ai) { return std::make_shared<AoeHealTrigger>(ai, "critical aoe heal", "critical", 2); }
        static std::shared_ptr<Trigger> low_aoe_heal(PlayerbotAI* ai) { return std::make_shared<AoeHealTrigger>(ai, "low aoe heal", "low", 2); }
        static std::shared_ptr<Trigger> medium_aoe_heal(PlayerbotAI* ai) { return std::make_shared<AoeHealTrigger>(ai, "medium aoe heal", "medium", 2); }
        static std::shared_ptr<Trigger> target_changed(PlayerbotAI* ai) { return std::make_shared<TargetChangedTrigger>(ai); }
        static std::shared_ptr<Trigger> swimming(PlayerbotAI* ai) { return std::make_shared<IsSwimmingTrigger>(ai); }
        static std::shared_ptr<Trigger> no_possible_targets(PlayerbotAI* ai) { return std::make_shared<NoPossibleTargetsTrigger>(ai); }
        static std::shared_ptr<Trigger> can_loot(PlayerbotAI* ai) { return std::make_shared<CanLootTrigger>(ai); }
        static std::shared_ptr<Trigger> far_from_loot_target(PlayerbotAI* ai) { return std::make_shared<FarFromCurrentLootTrigger>(ai); }
        static std::shared_ptr<Trigger> far_from_master(PlayerbotAI* ai) { return std::make_shared<FarFromMasterTrigger>(ai); }
        static std::shared_ptr<Trigger> behind_target(PlayerbotAI* ai) { return std::make_shared<IsBehindTargetTrigger>(ai); }
        static std::shared_ptr<Trigger> not_facing_target(PlayerbotAI* ai) { return std::make_shared<IsNotFacingTargetTrigger>(ai); }
        static std::shared_ptr<Trigger> panic(PlayerbotAI* ai) { return std::make_shared<PanicTrigger>(ai); }
        static std::shared_ptr<Trigger> no_drink(PlayerbotAI* ai) { return std::make_shared<NoDrinkTrigger>(ai); }
        static std::shared_ptr<Trigger> no_food(PlayerbotAI* ai) { return std::make_shared<NoFoodTrigger>(ai); }
        static std::shared_ptr<Trigger> LightAoe(PlayerbotAI* ai) { return std::make_shared<LightAoeTrigger>(ai); }
        static std::shared_ptr<Trigger> MediumAoe(PlayerbotAI* ai) { return std::make_shared<MediumAoeTrigger>(ai); }
        static std::shared_ptr<Trigger> HighAoe(PlayerbotAI* ai) { return std::make_shared<HighAoeTrigger>(ai); }
        static std::shared_ptr<Trigger> LoseAggro(PlayerbotAI* ai) { return std::make_shared<LoseAggroTrigger>(ai); }
        static std::shared_ptr<Trigger> HasAggro(PlayerbotAI* ai) { return std::make_shared<HasAggroTrigger>(ai); }
        static std::shared_ptr<Trigger> LowHealth(PlayerbotAI* ai) { return std::make_shared<LowHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> MediumHealth(PlayerbotAI* ai) { return std::make_shared<MediumHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> AlmostFullHealth(PlayerbotAI* ai) { return std::make_shared<AlmostFullHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> CriticalHealth(PlayerbotAI* ai) { return std::make_shared<CriticalHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> TargetCriticalHealth(PlayerbotAI* ai) { return std::make_shared<TargetCriticalHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> LowMana(PlayerbotAI* ai) { return std::make_shared<LowManaTrigger>(ai); }
        static std::shared_ptr<Trigger> MediumMana(PlayerbotAI* ai) { return std::make_shared<MediumManaTrigger>(ai); }
        static std::shared_ptr<Trigger> LightRageAvailable(PlayerbotAI* ai) { return std::make_shared<LightRageAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> MediumRageAvailable(PlayerbotAI* ai) { return std::make_shared<MediumRageAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> HighRageAvailable(PlayerbotAI* ai) { return std::make_shared<HighRageAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> LightEnergyAvailable(PlayerbotAI* ai) { return std::make_shared<LightEnergyAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> MediumEnergyAvailable(PlayerbotAI* ai) { return std::make_shared<MediumEnergyAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> HighEnergyAvailable(PlayerbotAI* ai) { return std::make_shared<HighEnergyAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> LootAvailable(PlayerbotAI* ai) { return std::make_shared<LootAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> NoAttackers(PlayerbotAI* ai) { return std::make_shared<NoAttackersTrigger>(ai); }
        static std::shared_ptr<Trigger> TankAoe(PlayerbotAI* ai) { return std::make_shared<TankAoeTrigger>(ai); }
        static std::shared_ptr<Trigger> Timer(PlayerbotAI* ai) { return std::make_shared<TimerTrigger>(ai); }
        static std::shared_ptr<Trigger> NoTarget(PlayerbotAI* ai) { return std::make_shared<NoTargetTrigger>(ai); }
        static std::shared_ptr<Trigger> TargetInSight(PlayerbotAI* ai) { return std::make_shared<TargetInSightTrigger>(ai); }
        static std::shared_ptr<Trigger> not_dps_target_active(PlayerbotAI* ai) { return std::make_shared<NotDpsTargetActiveTrigger>(ai); }
        static std::shared_ptr<Trigger> has_nearest_adds(PlayerbotAI* ai) { return std::make_shared<HasNearestAddsTrigger>(ai); }
        static std::shared_ptr<Trigger> enemy_player_is_attacking(PlayerbotAI* ai) { return std::make_shared<EnemyPlayerIsAttacking>(ai); }
        static std::shared_ptr<Trigger> Random(PlayerbotAI* ai) { return std::make_shared<RandomTrigger>(ai); }
        static std::shared_ptr<Trigger> seldom(PlayerbotAI* ai) { return std::make_shared<SeldomTrigger>(ai); }
        static std::shared_ptr<Trigger> often(PlayerbotAI* ai) { return std::make_shared<OftenTrigger>(ai); }
        static std::shared_ptr<Trigger> EnemyOutOfMelee(PlayerbotAI* ai) { return std::make_shared<EnemyOutOfMeleeTrigger>(ai); }
        static std::shared_ptr<Trigger> EnemyOutOfSpell(PlayerbotAI* ai) { return std::make_shared<EnemyOutOfSpellRangeTrigger>(ai); }
        static std::shared_ptr<Trigger> enemy_too_close_for_spell(PlayerbotAI* ai) { return std::make_shared<EnemyTooCloseForSpellTrigger>(ai); }
        static std::shared_ptr<Trigger> enemy_too_close_for_melee(PlayerbotAI* ai) { return std::make_shared<EnemyTooCloseForMeleeTrigger>(ai); }
        static std::shared_ptr<Trigger> ComboPointsAvailable(PlayerbotAI* ai) { return std::make_shared<ComboPointsAvailableTrigger>(ai); }
        static std::shared_ptr<Trigger> MediumThreat(PlayerbotAI* ai) { return std::make_shared<MediumThreatTrigger>(ai); }
        static std::shared_ptr<Trigger> Dead(PlayerbotAI* ai) { return std::make_shared<DeadTrigger>(ai); }
        static std::shared_ptr<Trigger> PartyMemberDead(PlayerbotAI* ai) { return std::make_shared<PartyMemberDeadTrigger>(ai); }
        static std::shared_ptr<Trigger> PartyMemberLowHealth(PlayerbotAI* ai) { return std::make_shared<PartyMemberLowHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> PartyMemberMediumHealth(PlayerbotAI* ai) { return std::make_shared<PartyMemberMediumHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> PartyMemberAlmostFullHealth(PlayerbotAI* ai) { return std::make_shared<PartyMemberAlmostFullHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> PartyMemberCriticalHealth(PlayerbotAI* ai) { return std::make_shared<PartyMemberCriticalHealthTrigger>(ai); }
        static std::shared_ptr<Trigger> no_pet(PlayerbotAI* ai) { return std::make_shared<NoPetTrigger>(ai); }
        static std::shared_ptr<Trigger> has_attackers(PlayerbotAI* ai) { return std::make_shared<HasAttackersTrigger>(ai); }

    };
};
