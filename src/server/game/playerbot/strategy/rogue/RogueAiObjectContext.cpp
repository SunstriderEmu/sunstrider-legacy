
#include "../../playerbot.h"
#include "RogueActions.h"
#include "RogueTriggers.h"
#include "RogueAiObjectContext.h"
#include "DpsRogueStrategy.h"
#include "GenericRogueNonCombatStrategy.h"
#include "../generic/PullStrategy.h"
#include "../NamedObjectContext.h"

using namespace ai;


namespace ai
{
    namespace rogue
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["dps"] = &rogue::StrategyFactoryInternal::dps;
                creators["nc"] = &rogue::StrategyFactoryInternal::nc;
                creators["pull"] = &rogue::StrategyFactoryInternal::pull;
            }

        private:
            static std::shared_ptr<Strategy> dps(PlayerbotAI* ai) { return std::make_shared<DpsRogueStrategy>(ai); }
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<GenericRogueNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> pull(PlayerbotAI* ai) { return std::make_shared<PullStrategy>(ai, "shoot"); }
        };
    };
};

namespace ai
{
    namespace rogue
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["kick"] = &TriggerFactoryInternal::kick;
                creators["rupture"] = &TriggerFactoryInternal::rupture;
                creators["slice and dice"] = &TriggerFactoryInternal::slice_and_dice;
                creators["expose armor"] = &TriggerFactoryInternal::expose_armor;
                creators["kick on enemy healer"] = &TriggerFactoryInternal::kick_on_enemy_healer;

            }

        private:
            static std::shared_ptr<Trigger> kick(PlayerbotAI* ai) { return std::make_shared<KickInterruptSpellTrigger>(ai); }
            static std::shared_ptr<Trigger> rupture(PlayerbotAI* ai) { return std::make_shared<RuptureTrigger>(ai); }
            static std::shared_ptr<Trigger> slice_and_dice(PlayerbotAI* ai) { return std::make_shared<SliceAndDiceTrigger>(ai); }
            static std::shared_ptr<Trigger> expose_armor(PlayerbotAI* ai) { return std::make_shared<ExposeArmorTrigger>(ai); }
            static std::shared_ptr<Trigger> kick_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<KickInterruptEnemyHealerSpellTrigger>(ai); }
        };
    };
};


namespace ai
{
    namespace rogue
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["riposte"] = &AiObjectContextInternal::riposte;
                creators["mutilate"] = &AiObjectContextInternal::mutilate;
                creators["sinister strike"] = &AiObjectContextInternal::sinister_strike;
                creators["kidney shot"] = &AiObjectContextInternal::kidney_shot;
                creators["rupture"] = &AiObjectContextInternal::rupture;
                creators["slice and dice"] = &AiObjectContextInternal::slice_and_dice;
                creators["eviscerate"] = &AiObjectContextInternal::eviscerate;
                creators["vanish"] = &AiObjectContextInternal::vanish;
                creators["evasion"] = &AiObjectContextInternal::evasion;
                creators["kick"] = &AiObjectContextInternal::kick;
                creators["feint"] = &AiObjectContextInternal::feint;
                creators["backstab"] = &AiObjectContextInternal::backstab;
                creators["expose armor"] = &AiObjectContextInternal::expose_armor;
                creators["kick on enemy healer"] = &AiObjectContextInternal::kick_on_enemy_healer;
            }

        private:
            static std::shared_ptr<Action> riposte(PlayerbotAI* ai) { return std::make_shared<CastRiposteAction>(ai); }
            static std::shared_ptr<Action> mutilate(PlayerbotAI* ai) { return std::make_shared<CastMutilateAction>(ai); }
            static std::shared_ptr<Action> sinister_strike(PlayerbotAI* ai) { return std::make_shared<CastSinisterStrikeAction>(ai); }
            static std::shared_ptr<Action> kidney_shot(PlayerbotAI* ai) { return std::make_shared<CastKidneyShotAction>(ai); }
            static std::shared_ptr<Action> rupture(PlayerbotAI* ai) { return std::make_shared<CastRuptureAction>(ai); }
            static std::shared_ptr<Action> slice_and_dice(PlayerbotAI* ai) { return std::make_shared<CastSliceAndDiceAction>(ai); }
            static std::shared_ptr<Action> eviscerate(PlayerbotAI* ai) { return std::make_shared<CastEviscerateAction>(ai); }
            static std::shared_ptr<Action> vanish(PlayerbotAI* ai) { return std::make_shared<CastVanishAction>(ai); }
            static std::shared_ptr<Action> evasion(PlayerbotAI* ai) { return std::make_shared<CastEvasionAction>(ai); }
            static std::shared_ptr<Action> kick(PlayerbotAI* ai) { return std::make_shared<CastKickAction>(ai); }
            static std::shared_ptr<Action> feint(PlayerbotAI* ai) { return std::make_shared<CastFeintAction>(ai); }
            static std::shared_ptr<Action> backstab(PlayerbotAI* ai) { return std::make_shared<CastBackstabAction>(ai); }
            static std::shared_ptr<Action> expose_armor(PlayerbotAI* ai) { return std::make_shared<CastExposeArmorAction>(ai); }
            static std::shared_ptr<Action> kick_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<CastKickOnEnemyHealerAction>(ai); }
        };
    };
};

RogueAiObjectContext::RogueAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::rogue::StrategyFactoryInternal());
    actionContexts.Add(new ai::rogue::AiObjectContextInternal());
    triggerContexts.Add(new ai::rogue::TriggerFactoryInternal());
}
