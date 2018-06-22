
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "HealShamanStrategy.h"

using namespace ai;

class GenericShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericShamanStrategyActionNodeFactory()
    {
        creators["flametongue weapon"] = &flametongue_weapon;
        creators["frostbrand weapon"] = &frostbrand_weapon;
        creators["windfury weapon"] = &windfury_weapon;
        creators["lesser healing wave"] = &lesser_healing_wave;
        creators["lesser healing wave on party"] = &lesser_healing_wave_on_party;
        creators["chain heal"] = &chain_heal;
        creators["riptide"] = &riptide;
        creators["chain heal on party"] = &chain_heal_on_party;
        creators["riptide on party"] = &riptide_on_party;
        creators["earth shock"] = &earth_shock;
    }
private:
    static std::shared_ptr<ActionNode> earth_shock(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("earth shock",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("flame shock") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> flametongue_weapon(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("flametongue weapon",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("frostbrand weapon") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> frostbrand_weapon(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("frostbrand weapon",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("rockbiter weapon") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> windfury_weapon(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("windfury weapon",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("rockbiter weapon") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> lesser_healing_wave(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("lesser healing wave",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("healing wave") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> lesser_healing_wave_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("lesser healing wave on party",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("healing wave on party") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> chain_heal(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("chain heal",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("lesser healing wave") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> riptide(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("riptide",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("healing wave") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> chain_heal_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("chain heal on party",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("lesser healing wave on party") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> riptide_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("riptide on party",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("healing wave on party") }),
            /*C*/ ActionList());
    }
};

GenericShamanStrategy::GenericShamanStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericShamanStrategyActionNodeFactory>());
}

void GenericShamanStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "wind shear",
        NextAction::array({ std::make_shared<NextAction>("wind shear", 23.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "wind shear on enemy healer",
        NextAction::array({ std::make_shared<NextAction>("wind shear on enemy healer", 23.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "purge",
        NextAction::array({ std::make_shared<NextAction>("purge", 10.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member medium health",
        NextAction::array({ std::make_shared<NextAction>("lesser healing wave on party", 25.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member low health",
        NextAction::array({ std::make_shared<NextAction>("riptide on party", 25.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe heal",
        NextAction::array({ std::make_shared<NextAction>("chain heal", 27.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("lesser healing wave", 26.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("riptide", 26.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "heroism",
        NextAction::array({ std::make_shared<NextAction>("heroism", 31.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "bloodlust",
        NextAction::array({ std::make_shared<NextAction>("bloodlust", 30.0f) })));
}

void ShamanBuffDpsStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "lightning shield",
        NextAction::array({ std::make_shared<NextAction>("lightning shield", 22.0f) })));
}

void ShamanBuffManaStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "water shield",
        NextAction::array({ std::make_shared<NextAction>("water shield", 22.0f) })));
}
