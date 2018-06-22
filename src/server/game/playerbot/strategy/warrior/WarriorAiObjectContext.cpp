
#include "../../playerbot.h"
#include "WarriorActions.h"
#include "WarriorAiObjectContext.h"
#include "GenericWarriorNonCombatStrategy.h"
#include "TankWarriorStrategy.h"
#include "DpsWarriorStrategy.h"
#include "../generic/PullStrategy.h"
#include "WarriorTriggers.h"
#include "../NamedObjectContext.h"

using namespace ai;


namespace ai
{
    namespace warrior
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &warrior::StrategyFactoryInternal::nc;
                creators["pull"] = &warrior::StrategyFactoryInternal::pull;
                creators["aoe"] = &warrior::StrategyFactoryInternal::aoe;
            }

        private:
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<GenericWarriorNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> aoe(PlayerbotAI* ai) { return std::make_shared<DpsWarrirorAoeStrategy>(ai); }
            static std::shared_ptr<Strategy> pull(PlayerbotAI* ai) { return std::make_shared<PullStrategy>(ai, "shoot"); }
        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["tank"] = &warrior::CombatStrategyFactoryInternal::tank;
                creators["dps"] = &warrior::CombatStrategyFactoryInternal::dps;
            }

        private:
            static std::shared_ptr<Strategy> tank(PlayerbotAI* ai) { return std::make_shared<TankWarriorStrategy>(ai); }
            static std::shared_ptr<Strategy> dps(PlayerbotAI* ai) { return std::make_shared<DpsWarriorStrategy>(ai); }
        };
    };
};

namespace ai
{
    namespace warrior
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["hamstring"] = &TriggerFactoryInternal::hamstring;
                creators["victory rush"] = &TriggerFactoryInternal::victory_rush;
                creators["death wish"] = &TriggerFactoryInternal::death_wish;
                creators["battle shout"] = &TriggerFactoryInternal::battle_shout;
                creators["rend"] = &TriggerFactoryInternal::rend;
                creators["rend on attacker"] = &TriggerFactoryInternal::rend_on_attacker;
                creators["bloodrage"] = &TriggerFactoryInternal::bloodrage;
                creators["shield bash"] = &TriggerFactoryInternal::shield_bash;
                creators["disarm"] = &TriggerFactoryInternal::disarm;
                creators["concussion blow"] = &TriggerFactoryInternal::concussion_blow;
                creators["sword and board"] = &TriggerFactoryInternal::SwordAndBoard;
                creators["shield bash on enemy healer"] = &TriggerFactoryInternal::shield_bash_on_enemy_healer;

            }

        private:
            static std::shared_ptr<Trigger> hamstring(PlayerbotAI* ai) { return std::make_shared<HamstringTrigger>(ai); }
            static std::shared_ptr<Trigger> victory_rush(PlayerbotAI* ai) { return std::make_shared<VictoryRushTrigger>(ai); }
            static std::shared_ptr<Trigger> death_wish(PlayerbotAI* ai) { return std::make_shared<DeathWishTrigger>(ai); }
            static std::shared_ptr<Trigger> battle_shout(PlayerbotAI* ai) { return std::make_shared<BattleShoutTrigger>(ai); }
            static std::shared_ptr<Trigger> rend(PlayerbotAI* ai) { return std::make_shared<RendDebuffTrigger>(ai); }
            static std::shared_ptr<Trigger> rend_on_attacker(PlayerbotAI* ai) { return std::make_shared<RendDebuffOnAttackerTrigger>(ai); }
            static std::shared_ptr<Trigger> bloodrage(PlayerbotAI* ai) { return std::make_shared<BloodrageDebuffTrigger>(ai); }
            static std::shared_ptr<Trigger> shield_bash(PlayerbotAI* ai) { return std::make_shared<ShieldBashInterruptSpellTrigger>(ai); }
            static std::shared_ptr<Trigger> disarm(PlayerbotAI* ai) { return std::make_shared<DisarmDebuffTrigger>(ai); }
            static std::shared_ptr<Trigger> concussion_blow(PlayerbotAI* ai) { return std::make_shared<ConcussionBlowTrigger>(ai); }
            static std::shared_ptr<Trigger> SwordAndBoard(PlayerbotAI* ai) { return std::make_shared<SwordAndBoardTrigger>(ai); }
            static std::shared_ptr<Trigger> shield_bash_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<ShieldBashInterruptEnemyHealerSpellTrigger>(ai); }
        };
    };
};


namespace ai
{
    namespace warrior
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["devastate"] = &AiObjectContextInternal::devastate;
                creators["overpower"] = &AiObjectContextInternal::overpower;
                creators["charge"] = &AiObjectContextInternal::charge;
                creators["bloodthirst"] = &AiObjectContextInternal::bloodthirst;
                creators["rend"] = &AiObjectContextInternal::rend;
                creators["rend on attacker"] = &AiObjectContextInternal::rend_on_attacker;
                creators["mocking blow"] = &AiObjectContextInternal::mocking_blow;
                creators["death wish"] = &AiObjectContextInternal::death_wish;
                creators["berserker rage"] = &AiObjectContextInternal::berserker_rage;
                creators["victory rush"] = &AiObjectContextInternal::victory_rush;
                creators["execute"] = &AiObjectContextInternal::execute;
                creators["defensive stance"] = &AiObjectContextInternal::defensive_stance;
                creators["hamstring"] = &AiObjectContextInternal::hamstring;
                creators["shield bash"] = &AiObjectContextInternal::shield_bash;
                creators["shield block"] = &AiObjectContextInternal::shield_block;
                creators["bloodrage"] = &AiObjectContextInternal::bloodrage;
                creators["battle stance"] = &AiObjectContextInternal::battle_stance;
                creators["heroic strike"] = &AiObjectContextInternal::heroic_strike;
                creators["intimidating shout"] = &AiObjectContextInternal::intimidating_shout;
                creators["demoralizing shout"] = &AiObjectContextInternal::demoralizing_shout;
                creators["challenging shout"] = &AiObjectContextInternal::challenging_shout;
                creators["shield wall"] = &AiObjectContextInternal::shield_wall;
                creators["battle shout"] = &AiObjectContextInternal::battle_shout;
                creators["thunder clap"] = &AiObjectContextInternal::thunder_clap;
                creators["taunt"] = &AiObjectContextInternal::taunt;
                creators["revenge"] = &AiObjectContextInternal::revenge;
                creators["slam"] = &AiObjectContextInternal::slam;
                creators["shield slam"] = &AiObjectContextInternal::shield_slam;
                creators["disarm"] = &AiObjectContextInternal::disarm;
                creators["sunder armor"] = &AiObjectContextInternal::sunder_armor;
                creators["last stand"] = &AiObjectContextInternal::last_stand;
                creators["shockwave"] = &AiObjectContextInternal::shockwave;
                creators["cleave"] = &AiObjectContextInternal::cleave;
                creators["concussion blow"] = &AiObjectContextInternal::concussion_blow;
                creators["shield bash on enemy healer"] = &AiObjectContextInternal::shield_bash_on_enemy_healer;
            }

        private:
            static std::shared_ptr<Action> devastate(PlayerbotAI* ai) { return std::make_shared<CastDevastateAction>(ai); }
            static std::shared_ptr<Action> last_stand(PlayerbotAI* ai) { return std::make_shared<CastLastStandAction>(ai); }
            static std::shared_ptr<Action> shockwave(PlayerbotAI* ai) { return std::make_shared<CastShockwaveAction>(ai); }
            static std::shared_ptr<Action> cleave(PlayerbotAI* ai) { return std::make_shared<CastCleaveAction>(ai); }
            static std::shared_ptr<Action> concussion_blow(PlayerbotAI* ai) { return std::make_shared<CastConcussionBlowAction>(ai); }
            static std::shared_ptr<Action> taunt(PlayerbotAI* ai) { return std::make_shared<CastTauntAction>(ai); }
            static std::shared_ptr<Action> revenge(PlayerbotAI* ai) { return std::make_shared<CastRevengeAction>(ai); }
            static std::shared_ptr<Action> slam(PlayerbotAI* ai) { return std::make_shared<CastSlamAction>(ai); }
            static std::shared_ptr<Action> shield_slam(PlayerbotAI* ai) { return std::make_shared<CastShieldSlamAction>(ai); }
            static std::shared_ptr<Action> disarm(PlayerbotAI* ai) { return std::make_shared<CastDisarmAction>(ai); }
            static std::shared_ptr<Action> sunder_armor(PlayerbotAI* ai) { return std::make_shared<CastSunderArmorAction>(ai); }
            static std::shared_ptr<Action> overpower(PlayerbotAI* ai) { return std::make_shared<CastOverpowerAction>(ai); }
            static std::shared_ptr<Action> charge(PlayerbotAI* ai) { return std::make_shared<CastChargeAction>(ai); }
            static std::shared_ptr<Action> bloodthirst(PlayerbotAI* ai) { return std::make_shared<CastBloodthirstAction>(ai); }
            static std::shared_ptr<Action> rend(PlayerbotAI* ai) { return std::make_shared<CastRendAction>(ai); }
            static std::shared_ptr<Action> rend_on_attacker(PlayerbotAI* ai) { return std::make_shared<CastRendOnAttackerAction>(ai); }
            static std::shared_ptr<Action> mocking_blow(PlayerbotAI* ai) { return std::make_shared<CastMockingBlowAction>(ai); }
            static std::shared_ptr<Action> death_wish(PlayerbotAI* ai) { return std::make_shared<CastDeathWishAction>(ai); }
            static std::shared_ptr<Action> berserker_rage(PlayerbotAI* ai) { return std::make_shared<CastBerserkerRageAction>(ai); }
            static std::shared_ptr<Action> victory_rush(PlayerbotAI* ai) { return std::make_shared<CastVictoryRushAction>(ai); }
            static std::shared_ptr<Action> execute(PlayerbotAI* ai) { return std::make_shared<CastExecuteAction>(ai); }
            static std::shared_ptr<Action> defensive_stance(PlayerbotAI* ai) { return std::make_shared<CastDefensiveStanceAction>(ai); }
            static std::shared_ptr<Action> hamstring(PlayerbotAI* ai) { return std::make_shared<CastHamstringAction>(ai); }
            static std::shared_ptr<Action> shield_bash(PlayerbotAI* ai) { return std::make_shared<CastShieldBashAction>(ai); }
            static std::shared_ptr<Action> shield_block(PlayerbotAI* ai) { return std::make_shared<CastShieldBlockAction>(ai); }
            static std::shared_ptr<Action> bloodrage(PlayerbotAI* ai) { return std::make_shared<CastBloodrageAction>(ai); }
            static std::shared_ptr<Action> battle_stance(PlayerbotAI* ai) { return std::make_shared<CastBattleStanceAction>(ai); }
            static std::shared_ptr<Action> heroic_strike(PlayerbotAI* ai) { return std::make_shared<CastHeroicStrikeAction>(ai); }
            static std::shared_ptr<Action> intimidating_shout(PlayerbotAI* ai) { return std::make_shared<CastIntimidatingShoutAction>(ai); }
            static std::shared_ptr<Action> demoralizing_shout(PlayerbotAI* ai) { return std::make_shared<CastDemoralizingShoutAction>(ai); }
            static std::shared_ptr<Action> challenging_shout(PlayerbotAI* ai) { return std::make_shared<CastChallengingShoutAction>(ai); }
            static std::shared_ptr<Action> shield_wall(PlayerbotAI* ai) { return std::make_shared<CastShieldWallAction>(ai); }
            static std::shared_ptr<Action> battle_shout(PlayerbotAI* ai) { return std::make_shared<CastBattleShoutAction>(ai); }
            static std::shared_ptr<Action> thunder_clap(PlayerbotAI* ai) { return std::make_shared<CastThunderClapAction>(ai); }
            static std::shared_ptr<Action> shield_bash_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<CastShieldBashOnEnemyHealerAction>(ai); }

        };
    };
};

WarriorAiObjectContext::WarriorAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::warrior::StrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::CombatStrategyFactoryInternal());
    actionContexts.Add(new ai::warrior::AiObjectContextInternal());
    triggerContexts.Add(new ai::warrior::TriggerFactoryInternal());
}
