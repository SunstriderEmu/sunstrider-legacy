////
#include "../../playerbot.h"
#include "DruidMultipliers.h"
#include "CatDpsDruidStrategy.h"

using namespace ai;

class CatDpsDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CatDpsDruidStrategyActionNodeFactory()
    {
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["melee"] = &melee;
        creators["feral charge - cat"] = &feral_charge_cat;
        creators["cat form"] = &cat_form;
        creators["claw"] = &claw;
        creators["mangle (cat)"] = &mangle_cat;
        creators["rake"] = &rake;
        creators["ferocious bite"] = &ferocious_bite;
        creators["rip"] = &rip;
    }
private:
    static std::shared_ptr<ActionNode> faerie_fire_feral(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("faerie fire (feral)",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> melee(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("melee",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("feral charge - cat") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> feral_charge_cat(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("feral charge - cat",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("reach melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> cat_form(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("cat form",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> claw(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("claw",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mangle_cat(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mangle (cat)",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("claw") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> rake(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("rake",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> ferocious_bite(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("ferocious bite",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("rip") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> rip(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("rip",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
};

CatDpsDruidStrategy::CatDpsDruidStrategy(PlayerbotAI* ai) : FeralDruidStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<CatDpsDruidStrategyActionNodeFactory>());
}

ActionList CatDpsDruidStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("mangle (cat)", ACTION_NORMAL + 1) });
}

void CatDpsDruidStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "cat form",
        NextAction::array({ std::make_shared<NextAction>("cat form", ACTION_MOVE + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "rake",
        NextAction::array({ std::make_shared<NextAction>("rake", ACTION_NORMAL + 5) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "combo points available",
        NextAction::array({ std::make_shared<NextAction>("ferocious bite", ACTION_NORMAL + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium threat",
        NextAction::array({ std::make_shared<NextAction>("cower", ACTION_EMERGENCY + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "faerie fire (feral)",
        NextAction::array({ std::make_shared<NextAction>("faerie fire (feral)", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "tiger's fury",
        NextAction::array({ std::make_shared<NextAction>("tiger's fury", ACTION_EMERGENCY + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "entangling roots",
        NextAction::array({ std::make_shared<NextAction>("entangling roots on cc", ACTION_HIGH + 1) })));

}

void CatAoeDruidStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("swipe (cat)", ACTION_HIGH + 2) })));
}

