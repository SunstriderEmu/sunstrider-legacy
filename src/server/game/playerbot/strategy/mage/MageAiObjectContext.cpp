
#include "../../playerbot.h"
#include "../Strategy.h"
#include "MageActions.h"
#include "MageAiObjectContext.h"
#include "FrostMageStrategy.h"
#include "ArcaneMageStrategy.h"
#include "GenericMageNonCombatStrategy.h"
#include "FireMageStrategy.h"
#include "../generic/PullStrategy.h"
#include "MageTriggers.h"
#include "../NamedObjectContext.h"

using namespace ai;


namespace ai
{
    namespace mage
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &mage::StrategyFactoryInternal::nc;
                creators["pull"] = &mage::StrategyFactoryInternal::pull;
                creators["fire aoe"] = &mage::StrategyFactoryInternal::fire_aoe;
                creators["frost aoe"] = &mage::StrategyFactoryInternal::frost_aoe;
            }

        private:
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<GenericMageNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> pull(PlayerbotAI* ai) { return std::make_shared<PullStrategy>(ai, "shoot"); }
            static std::shared_ptr<Strategy> fire_aoe(PlayerbotAI* ai) { return std::make_shared<FireMageAoeStrategy>(ai); }
            static std::shared_ptr<Strategy> frost_aoe(PlayerbotAI* ai) { return std::make_shared<FrostMageAoeStrategy>(ai); }
        };

        class MageStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            MageStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["frost"] = &mage::MageStrategyFactoryInternal::frost;
                creators["fire"] = &mage::MageStrategyFactoryInternal::fire;
                creators["arcane"] = &mage::MageStrategyFactoryInternal::arcane;
            }

        private:
            static std::shared_ptr<Strategy> frost(PlayerbotAI* ai) { return std::make_shared<FrostMageStrategy>(ai); }
            static std::shared_ptr<Strategy> fire(PlayerbotAI* ai) { return std::make_shared<FireMageStrategy>(ai); }
            static std::shared_ptr<Strategy> arcane(PlayerbotAI* ai) { return std::make_shared<ArcaneMageStrategy>(ai); }
        };

        class MageBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            MageBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bmana"] = &mage::MageBuffStrategyFactoryInternal::bmana;
                creators["bdps"] = &mage::MageBuffStrategyFactoryInternal::bdps;
            }

        private:
            static std::shared_ptr<Strategy> bmana(PlayerbotAI* ai) { return std::make_shared<MageBuffManaStrategy>(ai); }
            static std::shared_ptr<Strategy> bdps(PlayerbotAI* ai) { return std::make_shared<MageBuffDpsStrategy>(ai); }
        };
    };
};


namespace ai
{
    namespace mage
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["fireball"] = &TriggerFactoryInternal::fireball;
                creators["pyroblast"] = &TriggerFactoryInternal::pyroblast;
                creators["combustion"] = &TriggerFactoryInternal::combustion;
                creators["icy veins"] = &TriggerFactoryInternal::icy_veins;
                creators["arcane intellect"] = &TriggerFactoryInternal::arcane_intellect;
                creators["arcane intellect on party"] = &TriggerFactoryInternal::arcane_intellect_on_party;
                creators["mage armor"] = &TriggerFactoryInternal::mage_armor;
                creators["remove curse"] = &TriggerFactoryInternal::remove_curse;
                creators["remove curse on party"] = &TriggerFactoryInternal::remove_curse_on_party;
                creators["counterspell"] = &TriggerFactoryInternal::counterspell;
                creators["polymorph"] = &TriggerFactoryInternal::polymorph;
                creators["spellsteal"] = &TriggerFactoryInternal::spellsteal;
                creators["hot streak"] = &TriggerFactoryInternal::hot_streak;
                creators["living bomb"] = &TriggerFactoryInternal::living_bomb;
                creators["missile barrage"] = &TriggerFactoryInternal::missile_barrage;
                creators["arcane blast"] = &TriggerFactoryInternal::arcane_blast;
                creators["counterspell on enemy healer"] = &TriggerFactoryInternal::counterspell_enemy_healer;

            }

        private:
            static std::shared_ptr<Trigger> hot_streak(PlayerbotAI* ai) { return std::make_shared<HotStreakTrigger>(ai); }
            static std::shared_ptr<Trigger> fireball(PlayerbotAI* ai) { return std::make_shared<FireballTrigger>(ai); }
            static std::shared_ptr<Trigger> pyroblast(PlayerbotAI* ai) { return std::make_shared<PyroblastTrigger>(ai); }
            static std::shared_ptr<Trigger> combustion(PlayerbotAI* ai) { return std::make_shared<CombustionTrigger>(ai); }
            static std::shared_ptr<Trigger> icy_veins(PlayerbotAI* ai) { return std::make_shared<IcyVeinsTrigger>(ai); }
            static std::shared_ptr<Trigger> arcane_intellect(PlayerbotAI* ai) { return std::make_shared<ArcaneIntellectTrigger>(ai); }
            static std::shared_ptr<Trigger> arcane_intellect_on_party(PlayerbotAI* ai) { return std::make_shared<ArcaneIntellectOnPartyTrigger>(ai); }
            static std::shared_ptr<Trigger> mage_armor(PlayerbotAI* ai) { return std::make_shared<MageArmorTrigger>(ai); }
            static std::shared_ptr<Trigger> remove_curse(PlayerbotAI* ai) { return std::make_shared<RemoveCurseTrigger>(ai); }
            static std::shared_ptr<Trigger> remove_curse_on_party(PlayerbotAI* ai) { return std::make_shared<PartyMemberRemoveCurseTrigger>(ai); }
            static std::shared_ptr<Trigger> counterspell(PlayerbotAI* ai) { return std::make_shared<CounterspellInterruptSpellTrigger>(ai); }
            static std::shared_ptr<Trigger> polymorph(PlayerbotAI* ai) { return std::make_shared<PolymorphTrigger>(ai); }
            static std::shared_ptr<Trigger> spellsteal(PlayerbotAI* ai) { return std::make_shared<SpellstealTrigger>(ai); }
            static std::shared_ptr<Trigger> living_bomb(PlayerbotAI* ai) { return std::make_shared<LivingBombTrigger>(ai); }
            static std::shared_ptr<Trigger> missile_barrage(PlayerbotAI* ai) { return std::make_shared<MissileBarrageTrigger>(ai); }
            static std::shared_ptr<Trigger> arcane_blast(PlayerbotAI* ai) { return std::make_shared<ArcaneBlastTrigger>(ai); }
            static std::shared_ptr<Trigger> counterspell_enemy_healer(PlayerbotAI* ai) { return std::make_shared<CounterspellEnemyHealerTrigger>(ai); }
        };
    };
};


namespace ai
{
    namespace mage
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["frostbolt"] = &AiObjectContextInternal::frostbolt;
                creators["blizzard"] = &AiObjectContextInternal::blizzard;
                creators["frost nova"] = &AiObjectContextInternal::frost_nova;
                creators["arcane intellect"] = &AiObjectContextInternal::arcane_intellect;
                creators["arcane intellect on party"] = &AiObjectContextInternal::arcane_intellect_on_party;
                creators["conjure water"] = &AiObjectContextInternal::conjure_water;
                creators["conjure food"] = &AiObjectContextInternal::conjure_food;
                creators["molten armor"] = &AiObjectContextInternal::molten_armor;
                creators["mage armor"] = &AiObjectContextInternal::mage_armor;
                creators["ice armor"] = &AiObjectContextInternal::ice_armor;
                creators["frost armor"] = &AiObjectContextInternal::frost_armor;
                creators["fireball"] = &AiObjectContextInternal::fireball;
                creators["pyroblast"] = &AiObjectContextInternal::pyroblast;
                creators["flamestrike"] = &AiObjectContextInternal::flamestrike;
                creators["fire blast"] = &AiObjectContextInternal::fire_blast;
                creators["scorch"] = &AiObjectContextInternal::scorch;
                creators["counterspell"] = &AiObjectContextInternal::counterspell;
                creators["remove curse"] = &AiObjectContextInternal::remove_curse;
                creators["remove curse on party"] = &AiObjectContextInternal::remove_curse_on_party;
                creators["icy veins"] = &AiObjectContextInternal::icy_veins;
                creators["combustion"] = &AiObjectContextInternal::combustion;
                creators["ice block"] = &AiObjectContextInternal::ice_block;
                creators["polymorph"] = &AiObjectContextInternal::polymorph;
                creators["spellsteal"] = &AiObjectContextInternal::spellsteal;
                creators["living bomb"] = &AiObjectContextInternal::living_bomb;
                creators["dragon's breath"] = &AiObjectContextInternal::dragons_breath;
                creators["blast wave"] = &AiObjectContextInternal::blast_wave;
                creators["invisibility"] = &AiObjectContextInternal::invisibility;
                creators["evocation"] = &AiObjectContextInternal::evocation;
                creators["arcane blast"] = &AiObjectContextInternal::arcane_blast;
                creators["arcane barrage"] = &AiObjectContextInternal::arcane_barrage;
                creators["arcane missiles"] = &AiObjectContextInternal::arcane_missiles;
                creators["counterspell on enemy healer"] = &AiObjectContextInternal::counterspell_on_enemy_healer;
            }

        private:
            static std::shared_ptr<Action> arcane_missiles(PlayerbotAI* ai) { return std::make_shared<CastArcaneMissilesAction>(ai); }
            static std::shared_ptr<Action> arcane_barrage(PlayerbotAI* ai) { return std::make_shared<CastArcaneBarrageAction>(ai); }
            static std::shared_ptr<Action> arcane_blast(PlayerbotAI* ai) { return std::make_shared<CastArcaneBlastAction>(ai); }
            static std::shared_ptr<Action> frostbolt(PlayerbotAI* ai) { return std::make_shared<CastFrostboltAction>(ai); }
            static std::shared_ptr<Action> blizzard(PlayerbotAI* ai) { return std::make_shared<CastBlizzardAction>(ai); }
            static std::shared_ptr<Action> frost_nova(PlayerbotAI* ai) { return std::make_shared<CastFrostNovaAction>(ai); }
            static std::shared_ptr<Action> arcane_intellect(PlayerbotAI* ai) { return std::make_shared<CastArcaneIntellectAction>(ai); }
            static std::shared_ptr<Action> arcane_intellect_on_party(PlayerbotAI* ai) { return std::make_shared<CastArcaneIntellectOnPartyAction>(ai); }
            static std::shared_ptr<Action> conjure_water(PlayerbotAI* ai) { return std::make_shared<CastConjureWaterAction>(ai); }
            static std::shared_ptr<Action> conjure_food(PlayerbotAI* ai) { return std::make_shared<CastConjureFoodAction>(ai); }
            static std::shared_ptr<Action> molten_armor(PlayerbotAI* ai) { return std::make_shared<CastMoltenArmorAction>(ai); }
            static std::shared_ptr<Action> mage_armor(PlayerbotAI* ai) { return std::make_shared<CastMageArmorAction>(ai); }
            static std::shared_ptr<Action> ice_armor(PlayerbotAI* ai) { return std::make_shared<CastIceArmorAction>(ai); }
            static std::shared_ptr<Action> frost_armor(PlayerbotAI* ai) { return std::make_shared<CastFrostArmorAction>(ai); }
            static std::shared_ptr<Action> fireball(PlayerbotAI* ai) { return std::make_shared<CastFireballAction>(ai); }
            static std::shared_ptr<Action> pyroblast(PlayerbotAI* ai) { return std::make_shared<CastPyroblastAction>(ai); }
            static std::shared_ptr<Action> flamestrike(PlayerbotAI* ai) { return std::make_shared<CastFlamestrikeAction>(ai); }
            static std::shared_ptr<Action> fire_blast(PlayerbotAI* ai) { return std::make_shared<CastFireBlastAction>(ai); }
            static std::shared_ptr<Action> scorch(PlayerbotAI* ai) { return std::make_shared<CastScorchAction>(ai); }
            static std::shared_ptr<Action> counterspell(PlayerbotAI* ai) { return std::make_shared<CastCounterspellAction>(ai); }
            static std::shared_ptr<Action> remove_curse(PlayerbotAI* ai) { return std::make_shared<CastRemoveCurseAction>(ai); }
            static std::shared_ptr<Action> remove_curse_on_party(PlayerbotAI* ai) { return std::make_shared<CastRemoveCurseOnPartyAction>(ai); }
            static std::shared_ptr<Action> icy_veins(PlayerbotAI* ai) { return std::make_shared<CastIcyVeinsAction>(ai); }
            static std::shared_ptr<Action> combustion(PlayerbotAI* ai) { return std::make_shared<CastCombustionAction>(ai); }
            static std::shared_ptr<Action> ice_block(PlayerbotAI* ai) { return std::make_shared<CastIceBlockAction>(ai); }
            static std::shared_ptr<Action> polymorph(PlayerbotAI* ai) { return std::make_shared<CastPolymorphAction>(ai); }
            static std::shared_ptr<Action> spellsteal(PlayerbotAI* ai) { return std::make_shared<CastSpellstealAction>(ai); }
            static std::shared_ptr<Action> living_bomb(PlayerbotAI* ai) { return std::make_shared<CastLivingBombAction>(ai); }
            static std::shared_ptr<Action> dragons_breath(PlayerbotAI* ai) { return std::make_shared<CastDragonsBreathAction>(ai); }
            static std::shared_ptr<Action> blast_wave(PlayerbotAI* ai) { return std::make_shared<CastBlastWaveAction>(ai); }
            static std::shared_ptr<Action> invisibility(PlayerbotAI* ai) { return std::make_shared<CastInvisibilityAction>(ai); }
            static std::shared_ptr<Action> evocation(PlayerbotAI* ai) { return std::make_shared<CastEvocationAction>(ai); }
            static std::shared_ptr<Action> counterspell_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<CastCounterspellOnEnemyHealerAction>(ai); }
        };
    };
};



MageAiObjectContext::MageAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::mage::StrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::MageStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::MageBuffStrategyFactoryInternal());
    actionContexts.Add(new ai::mage::AiObjectContextInternal());
    triggerContexts.Add(new ai::mage::TriggerFactoryInternal());
}
