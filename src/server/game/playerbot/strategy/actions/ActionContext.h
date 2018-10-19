#pragma once

#include "GenericActions.h"
#include "NonCombatActions.h"
#include "EmoteAction.h"
#include "AddLootAction.h"
#include "LootAction.h"
#include "AddLootAction.h"
#include "StayActions.h"
#include "FollowActions.h"
#include "ChangeStrategyAction.h"
#include "ChooseTargetActions.h"
#include "SuggestWhatToDoAction.h"
#include "PositionAction.h"
#include "AttackAction.h"
#include "CheckMailAction.h"

namespace ai
{
    class ActionContext : public NamedObjectContext<Action>
    {
    public:
        ActionContext()
        {
            creators["attack"] = &ActionContext::melee;
            creators["melee"] = &ActionContext::melee;
            creators["reach spell"] = &ActionContext::ReachSpell;
            creators["reach melee"] = &ActionContext::ReachMelee;
            creators["flee"] = &ActionContext::flee;
            creators["gift of the naaru"] = &ActionContext::gift_of_the_naaru;
            creators["shoot"] = &ActionContext::shoot;
            creators["lifeblood"] = &ActionContext::lifeblood;
            creators["arcane torrent"] = &ActionContext::arcane_torrent;
            creators["end pull"] = &ActionContext::end_pull;
            creators["healthstone"] = &ActionContext::healthstone;
            creators["healing potion"] = &ActionContext::healing_potion;
            creators["mana potion"] = &ActionContext::mana_potion;
            creators["food"] = &ActionContext::food;
            creators["drink"] = &ActionContext::drink;
            creators["tank assist"] = &ActionContext::tank_assist;
            creators["dps assist"] = &ActionContext::dps_assist;
            creators["attack rti target"] = &ActionContext::attack_rti_target;
            creators["loot"] = &ActionContext::loot;
            creators["add loot"] = &ActionContext::add_loot;
            creators["add gathering loot"] = &ActionContext::add_gathering_loot;
            creators["add all loot"] = &ActionContext::add_all_loot;
            creators["shoot"] = &ActionContext::shoot;
            creators["follow"] = &ActionContext::follow;
            creators["runaway"] = &ActionContext::runaway;
            creators["stay"] = &ActionContext::stay;
            creators["attack anything"] = &ActionContext::attack_anything;
            creators["attack least hp target"] = &ActionContext::attack_least_hp_target;
            creators["attack enemy player"] = &ActionContext::enemy_player_target;
            creators["emote"] = &ActionContext::emote;
            creators["suggest what to do"] = &ActionContext::suggest_what_to_do;
            creators["move random"] = &ActionContext::move_random;
            creators["move to loot"] = &ActionContext::move_to_loot;
            creators["open loot"] = &ActionContext::open_loot;
            creators["guard"] = &ActionContext::guard;
            creators["move out of enemy contact"] = &ActionContext::move_out_of_enemy_contact;
            creators["set facing"] = &ActionContext::set_facing;
            creators["attack duel opponent"] = &ActionContext::attack_duel_opponent;
            creators["drop target"] = &ActionContext::drop_target;
            creators["check mail"] = &ActionContext::check_mail;
        }

    private:
        static std::shared_ptr<Action> check_mail(PlayerbotAI* ai) { return std::make_shared<CheckMailAction>(ai); }
        static std::shared_ptr<Action> drop_target(PlayerbotAI* ai) { return std::make_shared<DropTargetAction>(ai); }
        static std::shared_ptr<Action> attack_duel_opponent(PlayerbotAI* ai) { return std::make_shared<AttackDuelOpponentAction>(ai); }
        static std::shared_ptr<Action> guard(PlayerbotAI* ai) { return std::make_shared<GuardAction>(ai); }
        static std::shared_ptr<Action> open_loot(PlayerbotAI* ai) { return std::make_shared<OpenLootAction>(ai); }
        static std::shared_ptr<Action> move_to_loot(PlayerbotAI* ai) { return std::make_shared<MoveToLootAction>(ai); }
        static std::shared_ptr<Action> move_random(PlayerbotAI* ai) { return std::make_shared<MoveRandomAction>(ai); }
        static std::shared_ptr<Action> shoot(PlayerbotAI* ai) { return std::make_shared<CastShootAction>(ai); }
        static std::shared_ptr<Action> melee(PlayerbotAI* ai) { return std::make_shared<MeleeAction>(ai); }
        static std::shared_ptr<Action> ReachSpell(PlayerbotAI* ai) { return std::make_shared<ReachSpellAction>(ai); }
        static std::shared_ptr<Action> ReachMelee(PlayerbotAI* ai) { return std::make_shared<ReachMeleeAction>(ai); }
        static std::shared_ptr<Action> flee(PlayerbotAI* ai) { return std::make_shared<FleeAction>(ai); }
        static std::shared_ptr<Action> gift_of_the_naaru(PlayerbotAI* ai) { return std::make_shared<CastGiftOfTheNaaruAction>(ai); }
        static std::shared_ptr<Action> lifeblood(PlayerbotAI* ai) { return std::make_shared<CastLifeBloodAction>(ai); }
        static std::shared_ptr<Action> arcane_torrent(PlayerbotAI* ai) { return std::make_shared<CastArcaneTorrentAction>(ai); }
        static std::shared_ptr<Action> end_pull(PlayerbotAI* ai) { return std::make_shared<ChangeCombatStrategyAction>(ai, "-pull"); }

        static std::shared_ptr<Action> emote(PlayerbotAI* ai) { return std::make_shared<EmoteAction>(ai); }
        static std::shared_ptr<Action> suggest_what_to_do(PlayerbotAI* ai) { return std::make_shared<SuggestWhatToDoAction>(ai); }
        static std::shared_ptr<Action> attack_anything(PlayerbotAI* ai) { return std::make_shared<AttackAnythingAction>(ai); }
        static std::shared_ptr<Action> attack_least_hp_target(PlayerbotAI* ai) { return std::make_shared<AttackLeastHpTargetAction>(ai); }
        static std::shared_ptr<Action> enemy_player_target(PlayerbotAI* ai) { return std::make_shared<AttackEnemyPlayerAction>(ai); }
        static std::shared_ptr<Action> stay(PlayerbotAI* ai) { return std::make_shared<StayAction>(ai); }
        static std::shared_ptr<Action> runaway(PlayerbotAI* ai) { return std::make_shared<RunAwayAction>(ai); }
        static std::shared_ptr<Action> follow(PlayerbotAI* ai) { return std::make_shared<FollowAction>(ai); }
        static std::shared_ptr<Action> add_gathering_loot(PlayerbotAI* ai) { return std::make_shared<AddGatheringLootAction>(ai); }
        static std::shared_ptr<Action> add_loot(PlayerbotAI* ai) { return std::make_shared<AddLootAction>(ai); }
        static std::shared_ptr<Action> add_all_loot(PlayerbotAI* ai) { return std::make_shared<AddAllLootAction>(ai); }
        static std::shared_ptr<Action> loot(PlayerbotAI* ai) { return std::make_shared<LootAction>(ai); }
        static std::shared_ptr<Action> dps_assist(PlayerbotAI* ai) { return std::make_shared<DpsAssistAction>(ai); }
        static std::shared_ptr<Action> attack_rti_target(PlayerbotAI* ai) { return std::make_shared<AttackRtiTargetAction>(ai); }
        static std::shared_ptr<Action> tank_assist(PlayerbotAI* ai) { return std::make_shared<TankAssistAction>(ai); }
        static std::shared_ptr<Action> drink(PlayerbotAI* ai) { return std::make_shared<DrinkAction>(ai); }
        static std::shared_ptr<Action> food(PlayerbotAI* ai) { return std::make_shared<EatAction>(ai); }
        static std::shared_ptr<Action> mana_potion(PlayerbotAI* ai) { return std::make_shared<UseManaPotion>(ai); }
        static std::shared_ptr<Action> healing_potion(PlayerbotAI* ai) { return std::make_shared<UseHealingPotion>(ai); }
        static std::shared_ptr<Action> healthstone(PlayerbotAI* ai) { return std::make_shared<UseItemAction>(ai, "healthstone"); }
        static std::shared_ptr<Action> move_out_of_enemy_contact(PlayerbotAI* ai) { return std::make_shared<MoveOutOfEnemyContactAction>(ai); }
        static std::shared_ptr<Action> set_facing(PlayerbotAI* ai) { return std::make_shared<SetFacingTargetAction>(ai); }
    };

};
