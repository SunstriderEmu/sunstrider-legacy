#pragma once

#include "NearestGameObjects.h"
#include "LogLevelValue.h"
#include "NearestNpcsValue.h"
#include "PossibleTargetsValue.h"
#include "NearestAdsValue.h"
#include "NearestCorpsesValue.h"
#include "PartyMemberWithoutAuraValue.h"
#include "PartyMemberToHeal.h"
#include "PartyMemberToResurrect.h"
#include "CurrentTargetValue.h"
#include "SelfTargetValue.h"
#include "MasterTargetValue.h"
#include "LineTargetValue.h"
#include "TankTargetValue.h"
#include "DpsTargetValue.h"
#include "CcTargetValue.h"
#include "CurrentCcTargetValue.h"
#include "PetTargetValue.h"
#include "GrindTargetValue.h"
#include "RtiTargetValue.h"
#include "PartyMemberToDispel.h"
#include "StatsValues.h"
#include "AttackerCountValues.h"
#include "AttackersValue.h"
#include "AvailableLootValue.h"
#include "AlwaysLootListValue.h"
#include "LootStrategyValue.h"
#include "HasAvailableLootValue.h"
#include "LastMovementValue.h"
#include "DistanceValue.h"
#include "IsMovingValue.h"
#include "IsBehindValue.h"
#include "IsFacingValue.h"
#include "ItemCountValue.h"
#include "SpellIdValue.h"
#include "ItemForSpellValue.h"
#include "SpellCastUsefulValue.h"
#include "LastSpellCastValue.h"
#include "ChatValue.h"
#include "HasTotemValue.h"
#include "LeastHpTargetValue.h"
#include "AoeHealValues.h"
#include "RtiValue.h"
#include "PositionValue.h"
#include "ThreatValues.h"
#include "DuelTargetValue.h"
#include "InvalidTargetValue.h"
#include "EnemyPlayerValue.h"
#include "AttackerWithoutAuraTargetValue.h"
#include "LastSpellCastTimeValue.h"
#include "ManaSaveLevelValue.h"
#include "LfgValues.h"
#include "EnemyHealerTargetValue.h"
#include "Formations.h"
#include "ItemUsageValue.h"

namespace ai
{
    class ValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        ValueContext()
        {
            creators["nearest game objects"] = &ValueContext::nearest_game_objects;
            creators["nearest npcs"] = &ValueContext::nearest_npcs;
            creators["possible targets"] = &ValueContext::possible_targets;
            creators["nearest adds"] = &ValueContext::nearest_adds;
            creators["nearest corpses"] = &ValueContext::nearest_corpses;
            creators["log level"] = &ValueContext::log_level;
            creators["party member without aura"] = &ValueContext::party_member_without_aura;
            creators["attacker without aura"] = &ValueContext::attacker_without_aura;
            creators["party member to heal"] = &ValueContext::party_member_to_heal;
            creators["party member to resurrect"] = &ValueContext::party_member_to_resurrect;
            creators["current target"] = &ValueContext::current_target;
            creators["self target"] = &ValueContext::self_target;
            creators["master target"] = &ValueContext::master;
            creators["line target"] = &ValueContext::line_target;
            creators["tank target"] = &ValueContext::tank_target;
            creators["dps target"] = &ValueContext::dps_target;
            creators["least hp target"] = &ValueContext::least_hp_target;
            creators["enemy player target"] = &ValueContext::enemy_player_target;
            creators["cc target"] = &ValueContext::cc_target;
            creators["current cc target"] = &ValueContext::current_cc_target;
            creators["pet target"] = &ValueContext::pet_target;
            creators["old target"] = &ValueContext::old_target;
            creators["grind target"] = &ValueContext::grind_target;
            creators["rti target"] = &ValueContext::rti_target;
            creators["duel target"] = &ValueContext::duel_target;
            creators["party member to dispel"] = &ValueContext::party_member_to_dispel;
            creators["health"] = &ValueContext::health;
            creators["rage"] = &ValueContext::rage;
            creators["energy"] = &ValueContext::energy;
            creators["mana"] = &ValueContext::mana;
            creators["combo"] = &ValueContext::combo;
            creators["dead"] = &ValueContext::dead;
            creators["has mana"] = &ValueContext::has_mana;
            creators["attacker count"] = &ValueContext::attacker_count;
            creators["my attacker count"] = &ValueContext::my_attacker_count;
            creators["has aggro"] = &ValueContext::has_aggro;
            creators["mounted"] = &ValueContext::mounted;

            creators["can loot"] = &ValueContext::can_loot;
            creators["loot target"] = &ValueContext::loot_target;
            creators["available loot"] = &ValueContext::available_loot;
            creators["has available loot"] = &ValueContext::has_available_loot;
            creators["always loot list"] = &ValueContext::always_loot_list;
            creators["loot strategy"] = &ValueContext::loot_strategy;
            creators["last movement"] = &ValueContext::last_movement;
            creators["distance"] = &ValueContext::distance;
            creators["moving"] = &ValueContext::moving;
            creators["swimming"] = &ValueContext::swimming;
            creators["behind"] = &ValueContext::behind;
            creators["facing"] = &ValueContext::facing;

            creators["item count"] = &ValueContext::item_count;
            creators["inventory items"] = &ValueContext::inventory_item;

            creators["spell id"] = &ValueContext::spell_id;
            creators["item for spell"] = &ValueContext::item_for_spell;
            creators["spell cast useful"] = &ValueContext::spell_cast_useful;
            creators["last spell cast"] = &ValueContext::last_spell_cast;
            creators["last spell cast time"] = &ValueContext::last_spell_cast_time;
            creators["chat"] = &ValueContext::chat;
            creators["has totem"] = &ValueContext::has_totem;

            creators["aoe heal"] = &ValueContext::aoe_heal;

            creators["rti"] = &ValueContext::rti;
            creators["position"] = &ValueContext::position;
            creators["threat"] = &ValueContext::threat;

            creators["balance"] = &ValueContext::balance;
            creators["attackers"] = &ValueContext::attackers;
            creators["invalid target"] = &ValueContext::invalid_target;
            creators["mana save level"] = &ValueContext::mana_save_level;
            creators["combat"] = &ValueContext::combat;
            creators["lfg proposal"] = &ValueContext::lfg_proposal;
            creators["bag space"] = &ValueContext::bag_space;
            creators["enemy healer target"] = &ValueContext::enemy_healer_target;
            creators["formation"] = &ValueContext::formation;
            creators["item usage"] = &ValueContext::item_usage;
        }

    private:
        static std::shared_ptr<UntypedValue> item_usage(PlayerbotAI* ai) { return std::make_shared<ItemUsageValue>(ai); }
        static std::shared_ptr<UntypedValue> formation(PlayerbotAI* ai) { return std::make_shared<FormationValue>(ai); }
        static std::shared_ptr<UntypedValue> mana_save_level(PlayerbotAI* ai) { return std::make_shared<ManaSaveLevelValue>(ai); }
        static std::shared_ptr<UntypedValue> invalid_target(PlayerbotAI* ai) { return std::make_shared<InvalidTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> balance(PlayerbotAI* ai) { return std::make_shared<BalancePercentValue>(ai); }
        static std::shared_ptr<UntypedValue> attackers(PlayerbotAI* ai) { return std::make_shared<AttackersValue>(ai); }

        static std::shared_ptr<UntypedValue> position(PlayerbotAI* ai) { return std::make_shared<PositionValue>(ai); }
        static std::shared_ptr<UntypedValue> rti(PlayerbotAI* ai) { return std::make_shared<RtiValue>(ai); }

        static std::shared_ptr<UntypedValue> aoe_heal(PlayerbotAI* ai) { return std::make_shared<AoeHealValue>(ai); }

        static std::shared_ptr<UntypedValue> chat(PlayerbotAI* ai) { return std::make_shared<ChatValue>(ai); }
        static std::shared_ptr<UntypedValue> last_spell_cast(PlayerbotAI* ai) { return std::make_shared<LastSpellCastValue>(ai); }
        static std::shared_ptr<UntypedValue> last_spell_cast_time(PlayerbotAI* ai) { return std::make_shared<LastSpellCastTimeValue>(ai); }
        static std::shared_ptr<UntypedValue> spell_cast_useful(PlayerbotAI* ai) { return std::make_shared<SpellCastUsefulValue>(ai); }
        static std::shared_ptr<UntypedValue> item_for_spell(PlayerbotAI* ai) { return std::make_shared<ItemForSpellValue>(ai); }
        static std::shared_ptr<UntypedValue> spell_id(PlayerbotAI* ai) { return std::make_shared<SpellIdValue>(ai); }
        static std::shared_ptr<UntypedValue> inventory_item(PlayerbotAI* ai) { return std::make_shared<InventoryItemValue>(ai); }
        static std::shared_ptr<UntypedValue> item_count(PlayerbotAI* ai) { return std::make_shared<ItemCountValue>(ai); }
        static std::shared_ptr<UntypedValue> behind(PlayerbotAI* ai) { return std::make_shared<IsBehindValue>(ai); }
        static std::shared_ptr<UntypedValue> facing(PlayerbotAI* ai) { return std::make_shared<IsFacingValue>(ai); }
        static std::shared_ptr<UntypedValue> moving(PlayerbotAI* ai) { return std::make_shared<IsMovingValue>(ai); }
        static std::shared_ptr<UntypedValue> swimming(PlayerbotAI* ai) { return std::make_shared<IsSwimmingValue>(ai); }
        static std::shared_ptr<UntypedValue> distance(PlayerbotAI* ai) { return std::make_shared<DistanceValue>(ai); }
        static std::shared_ptr<UntypedValue> last_movement(PlayerbotAI* ai) { return std::make_shared<LastMovementValue>(ai); }

        static std::shared_ptr<UntypedValue> can_loot(PlayerbotAI* ai) { return std::make_shared<CanLootValue>(ai); }
        static std::shared_ptr<UntypedValue> available_loot(PlayerbotAI* ai) { return std::make_shared<AvailableLootValue>(ai); }
        static std::shared_ptr<UntypedValue> loot_target(PlayerbotAI* ai) { return std::make_shared<LootTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> has_available_loot(PlayerbotAI* ai) { return std::make_shared<HasAvailableLootValue>(ai); }
        static std::shared_ptr<UntypedValue> always_loot_list(PlayerbotAI* ai) { return std::make_shared<AlwaysLootListValue>(ai); }
        static std::shared_ptr<UntypedValue> loot_strategy(PlayerbotAI* ai) { return std::make_shared<LootStrategyValue>(ai); }

        static std::shared_ptr<UntypedValue> attacker_count(PlayerbotAI* ai) { return std::make_shared<AttackerCountValue>(ai); }
        static std::shared_ptr<UntypedValue> my_attacker_count(PlayerbotAI* ai) { return std::make_shared<MyAttackerCountValue>(ai); }
        static std::shared_ptr<UntypedValue> has_aggro(PlayerbotAI* ai) { return std::make_shared<HasAggroValue>(ai); }
        static std::shared_ptr<UntypedValue> mounted(PlayerbotAI* ai) { return std::make_shared<IsMountedValue>(ai); }
        static std::shared_ptr<UntypedValue> health(PlayerbotAI* ai) { return std::make_shared<HealthValue>(ai); }
        static std::shared_ptr<UntypedValue> rage(PlayerbotAI* ai) { return std::make_shared<RageValue>(ai); }
        static std::shared_ptr<UntypedValue> energy(PlayerbotAI* ai) { return std::make_shared<EnergyValue>(ai); }
        static std::shared_ptr<UntypedValue> mana(PlayerbotAI* ai) { return std::make_shared<ManaValue>(ai); }
        static std::shared_ptr<UntypedValue> combo(PlayerbotAI* ai) { return std::make_shared<ComboPointsValue>(ai); }
        static std::shared_ptr<UntypedValue> dead(PlayerbotAI* ai) { return std::make_shared<IsDeadValue>(ai); }
        static std::shared_ptr<UntypedValue> has_mana(PlayerbotAI* ai) { return std::make_shared<HasManaValue>(ai); }
        static std::shared_ptr<UntypedValue> nearest_game_objects(PlayerbotAI* ai) { return std::make_shared<NearestGameObjects>(ai); }
        static std::shared_ptr<UntypedValue> log_level(PlayerbotAI* ai) { return std::make_shared<LogLevelValue>(ai); }
        static std::shared_ptr<UntypedValue> nearest_npcs(PlayerbotAI* ai) { return std::make_shared<NearestNpcsValue>(ai); }
        static std::shared_ptr<UntypedValue> nearest_corpses(PlayerbotAI* ai) { return std::make_shared<NearestCorpsesValue>(ai); }
        static std::shared_ptr<UntypedValue> possible_targets(PlayerbotAI* ai) { return std::make_shared<PossibleTargetsValue>(ai); }
        static std::shared_ptr<UntypedValue> nearest_adds(PlayerbotAI* ai) { return std::make_shared<NearestAdsValue>(ai); }
        static std::shared_ptr<UntypedValue> party_member_without_aura(PlayerbotAI* ai) { return std::make_shared<PartyMemberWithoutAuraValue>(ai); }
        static std::shared_ptr<UntypedValue> attacker_without_aura(PlayerbotAI* ai) { return std::make_shared<AttackerWithoutAuraTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> party_member_to_heal(PlayerbotAI* ai) { return std::make_shared<PartyMemberToHeal>(ai); }
        static std::shared_ptr<UntypedValue> party_member_to_resurrect(PlayerbotAI* ai) { return std::make_shared<PartyMemberToResurrect>(ai); }
        static std::shared_ptr<UntypedValue> party_member_to_dispel(PlayerbotAI* ai) { return std::make_shared<PartyMemberToDispel>(ai); }
        static std::shared_ptr<UntypedValue> current_target(PlayerbotAI* ai) { return std::make_shared<CurrentTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> old_target(PlayerbotAI* ai) { return std::make_shared<CurrentTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> self_target(PlayerbotAI* ai) { return std::make_shared<SelfTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> master(PlayerbotAI* ai) { return std::make_shared<MasterTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> line_target(PlayerbotAI* ai) { return std::make_shared<LineTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> tank_target(PlayerbotAI* ai) { return std::make_shared<TankTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> dps_target(PlayerbotAI* ai) { return std::make_shared<DpsTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> least_hp_target(PlayerbotAI* ai) { return std::make_shared<LeastHpTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> enemy_player_target(PlayerbotAI* ai) { return std::make_shared<EnemyPlayerValue>(ai); }
        static std::shared_ptr<UntypedValue> cc_target(PlayerbotAI* ai) { return std::make_shared<CcTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> current_cc_target(PlayerbotAI* ai) { return std::make_shared<CurrentCcTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> pet_target(PlayerbotAI* ai) { return std::make_shared<PetTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> grind_target(PlayerbotAI* ai) { return std::make_shared<GrindTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> rti_target(PlayerbotAI* ai) { return std::make_shared<RtiTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> duel_target(PlayerbotAI* ai) { return std::make_shared<DuelTargetValue>(ai); }
        static std::shared_ptr<UntypedValue> has_totem(PlayerbotAI* ai) { return std::make_shared<HasTotemValue>(ai); }
        static std::shared_ptr<UntypedValue> threat(PlayerbotAI* ai) { return std::make_shared<ThreatValue>(ai); }
        static std::shared_ptr<UntypedValue> combat(PlayerbotAI* ai) { return std::make_shared<IsInCombatValue>(ai); }
        static std::shared_ptr<UntypedValue> lfg_proposal(PlayerbotAI* ai) { return std::make_shared<LfgProposalValue>(ai); }
        static std::shared_ptr<UntypedValue> bag_space(PlayerbotAI* ai) { return std::make_shared<BagSpaceValue>(ai); }
        static std::shared_ptr<UntypedValue> enemy_healer_target(PlayerbotAI* ai) { return std::make_shared<EnemyHealerTargetValue>(ai); }
    };
};
