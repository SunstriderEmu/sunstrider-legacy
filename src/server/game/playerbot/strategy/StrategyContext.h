#pragma once

#include "generic/NonCombatStrategy.h"
#include "generic/RacialsStrategy.h"
#include "generic/ChatCommandHandlerStrategy.h"
#include "generic/WorldPacketHandlerStrategy.h"
#include "generic/DeadStrategy.h"
#include "generic/QuestStrategies.h"
#include "generic/LootNonCombatStrategy.h"
#include "generic/DuelStrategy.h"
#include "generic/KiteStrategy.h"
#include "generic/FleeStrategy.h"
#include "generic/FollowMasterStrategy.h"
#include "generic/RunawayStrategy.h"
#include "generic/StayStrategy.h"
#include "generic/UseFoodStrategy.h"
#include "generic/ConserveManaStrategy.h"
#include "generic/EmoteStrategy.h"
#include "generic/AttackRtiStrategy.h"
#include "generic/AttackWeakStrategy.h"
#include "generic/TankAoeStrategy.h"
#include "generic/TankAssistStrategy.h"
#include "generic/DpsAoeStrategy.h"
#include "generic/DpsAssistStrategy.h"
#include "generic/PassiveStrategy.h"
#include "generic/GrindingStrategy.h"
#include "generic/UsePotionsStrategy.h"
#include "generic/GuardStrategy.h"
#include "generic/CastTimeStrategy.h"
#include "generic/ThreatStrategy.h"
#include "generic/TellTargetStrategy.h"
#include "generic/AttackEnemyPlayersStrategy.h"
#include "generic/MoveRandomStrategy.h"

namespace ai
{
    class StrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        StrategyContext()
        {
            creators["racials"] = &StrategyContext::racials;
            creators["loot"] = &StrategyContext::loot;
            creators["gather"] = &StrategyContext::gather;
            creators["emote"] = &StrategyContext::emote;
            creators["passive"] = &StrategyContext::passive;
            creators["conserve mana"] = &StrategyContext::conserve_mana;
            creators["food"] = &StrategyContext::food;
            creators["chat"] = &StrategyContext::chat;
            creators["default"] = &StrategyContext::world_packet;
            creators["ready check"] = &StrategyContext::ready_check;
            creators["dead"] = &StrategyContext::dead;
            creators["flee"] = &StrategyContext::flee;
            creators["duel"] = &StrategyContext::duel;
            creators["kite"] = &StrategyContext::kite;
            creators["potions"] = &StrategyContext::potions;
            creators["cast time"] = &StrategyContext::cast_time;
            creators["threat"] = &StrategyContext::threat;
            creators["tell target"] = &StrategyContext::tell_target;
            creators["pvp"] = &StrategyContext::pvp;
            creators["move random"] = &StrategyContext::move_random;
            creators["lfg"] = &StrategyContext::lfg;
        }

    private:
        static std::shared_ptr<Strategy> tell_target(PlayerbotAI* ai) { return std::make_shared<TellTargetStrategy>(ai); }
        static std::shared_ptr<Strategy> threat(PlayerbotAI* ai) { return std::make_shared<ThreatStrategy>(ai); }
        static std::shared_ptr<Strategy> cast_time(PlayerbotAI* ai) { return std::make_shared<CastTimeStrategy>(ai); }
        static std::shared_ptr<Strategy> potions(PlayerbotAI* ai) { return std::make_shared<UsePotionsStrategy>(ai); }
        static std::shared_ptr<Strategy> kite(PlayerbotAI* ai) { return std::make_shared<KiteStrategy>(ai); }
        static std::shared_ptr<Strategy> duel(PlayerbotAI* ai) { return std::make_shared<DuelStrategy>(ai); }
        static std::shared_ptr<Strategy> flee(PlayerbotAI* ai) { return std::make_shared<FleeStrategy>(ai); }
        static std::shared_ptr<Strategy> dead(PlayerbotAI* ai) { return std::make_shared<DeadStrategy>(ai); }
        static std::shared_ptr<Strategy> racials(PlayerbotAI* ai) { return std::make_shared<RacialsStrategy>(ai); }
        static std::shared_ptr<Strategy> loot(PlayerbotAI* ai) { return std::make_shared<LootNonCombatStrategy>(ai); }
        static std::shared_ptr<Strategy> gather(PlayerbotAI* ai) { return std::make_shared<GatherStrategy>(ai); }
        static std::shared_ptr<Strategy> emote(PlayerbotAI* ai) { return std::make_shared<EmoteStrategy>(ai); }
        static std::shared_ptr<Strategy> passive(PlayerbotAI* ai) { return std::make_shared<PassiveStrategy>(ai); }
        static std::shared_ptr<Strategy> conserve_mana(PlayerbotAI* ai) { return std::make_shared<ConserveManaStrategy>(ai); }
        static std::shared_ptr<Strategy> food(PlayerbotAI* ai) { return std::make_shared<UseFoodStrategy>(ai); }
        static std::shared_ptr<Strategy> chat(PlayerbotAI* ai) { return std::make_shared<ChatCommandHandlerStrategy>(ai); }
        static std::shared_ptr<Strategy> world_packet(PlayerbotAI* ai) { return std::make_shared<WorldPacketHandlerStrategy>(ai); }
        static std::shared_ptr<Strategy> ready_check(PlayerbotAI* ai) { return std::make_shared<ReadyCheckStrategy>(ai); }
        static std::shared_ptr<Strategy> pvp(PlayerbotAI* ai) { return std::make_shared<AttackEnemyPlayersStrategy>(ai); }
        static std::shared_ptr<Strategy> move_random(PlayerbotAI* ai) { return std::make_shared<MoveRandomStrategy>(ai); }
        static std::shared_ptr<Strategy> lfg(PlayerbotAI* ai) { return std::make_shared<LfgStrategy>(ai); }
    };

    class MovementStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        MovementStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["follow"] = &MovementStrategyContext::follow_master;
            creators["stay"] = &MovementStrategyContext::stay;
            creators["runaway"] = &MovementStrategyContext::runaway;
            creators["flee from adds"] = &MovementStrategyContext::flee_from_adds;
            creators["guard"] = &MovementStrategyContext::guard;
        }

    private:
        static std::shared_ptr<Strategy> guard(PlayerbotAI* ai) { return std::make_shared<GuardStrategy>(ai); }
        static std::shared_ptr<Strategy> follow_master(PlayerbotAI* ai) { return std::make_shared<FollowMasterStrategy>(ai); }
        static std::shared_ptr<Strategy> stay(PlayerbotAI* ai) { return std::make_shared<StayStrategy>(ai); }
        static std::shared_ptr<Strategy> runaway(PlayerbotAI* ai) { return std::make_shared<RunawayStrategy>(ai); }
        static std::shared_ptr<Strategy> flee_from_adds(PlayerbotAI* ai) { return std::make_shared<FleeFromAddsStrategy>(ai); }
    };

    class AssistStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        AssistStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["dps assist"] = &AssistStrategyContext::dps_assist;
            creators["dps aoe"] = &AssistStrategyContext::dps_aoe;
            creators["tank assist"] = &AssistStrategyContext::tank_assist;
            creators["tank aoe"] = &AssistStrategyContext::tank_aoe;
            creators["attack weak"] = &AssistStrategyContext::attack_weak;
            creators["grind"] = &AssistStrategyContext::grind;
            creators["attack rti"] = &AssistStrategyContext::attack_rti;
        }

    private:
        static std::shared_ptr<Strategy> dps_assist(PlayerbotAI* ai) { return std::make_shared<DpsAssistStrategy>(ai); }
        static std::shared_ptr<Strategy> dps_aoe(PlayerbotAI* ai) { return std::make_shared<DpsAoeStrategy>(ai); }
        static std::shared_ptr<Strategy> tank_assist(PlayerbotAI* ai) { return std::make_shared<TankAssistStrategy>(ai); }
        static std::shared_ptr<Strategy> tank_aoe(PlayerbotAI* ai) { return std::make_shared<TankAoeStrategy>(ai); }
        static std::shared_ptr<Strategy> attack_weak(PlayerbotAI* ai) { return std::make_shared<AttackWeakStrategy>(ai); }
        static std::shared_ptr<Strategy> grind(PlayerbotAI* ai) { return std::make_shared<GrindingStrategy>(ai); }
        static std::shared_ptr<Strategy> attack_rti(PlayerbotAI* ai) { return std::make_shared<AttackRtiStrategy>(ai); }
    };

    class QuestStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        QuestStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["quest"] = &QuestStrategyContext::quest;
            creators["accept all quests"] = &QuestStrategyContext::accept_all_quests;
        }

    private:
        static std::shared_ptr<Strategy> quest(PlayerbotAI* ai) { return std::make_shared<DefaultQuestStrategy>(ai); }
        static std::shared_ptr<Strategy> accept_all_quests(PlayerbotAI* ai) { return std::make_shared<AcceptAllQuestsStrategy>(ai); }
    };
};
