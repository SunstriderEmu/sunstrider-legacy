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
    static ActionNode* faerie_fire_feral(PlayerbotAI* ai)
    {
        return new ActionNode ("faerie fire (feral)",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* melee(PlayerbotAI* ai)
    {
        return new ActionNode ("melee",
            /*P*/ NextAction::array({ new NextAction("feral charge - cat") }),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* feral_charge_cat(PlayerbotAI* ai)
    {
        return new ActionNode ("feral charge - cat",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("reach melee") }),
            /*C*/ NULL);
    }
    static ActionNode* cat_form(PlayerbotAI* ai)
    {
        return new ActionNode ("cat form",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* claw(PlayerbotAI* ai)
    {
        return new ActionNode ("claw",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ NULL);
    }
    static ActionNode* mangle_cat(PlayerbotAI* ai)
    {
        return new ActionNode ("mangle (cat)",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("claw") }),
            /*C*/ NULL);
    }
    static ActionNode* rake(PlayerbotAI* ai)
    {
        return new ActionNode ("rake",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* ferocious_bite(PlayerbotAI* ai)
    {
        return new ActionNode ("ferocious bite",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("rip") }),
            /*C*/ NULL);
    }
    static ActionNode* rip(PlayerbotAI* ai)
    {
        return new ActionNode ("rip",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
};

CatDpsDruidStrategy::CatDpsDruidStrategy(PlayerbotAI* ai) : FeralDruidStrategy(ai)
{
    actionNodeFactories.Add(new CatDpsDruidStrategyActionNodeFactory());
}

NextAction** CatDpsDruidStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("mangle (cat)", ACTION_NORMAL + 1) });
}

void CatDpsDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "cat form",
        NextAction::array({ new NextAction("cat form", ACTION_MOVE + 2) })));

    triggers.push_back(new TriggerNode(
        "rake",
        NextAction::array({ new NextAction("rake", ACTION_NORMAL + 5) })));

    triggers.push_back(new TriggerNode(
        "combo points available",
        NextAction::array({ new NextAction("ferocious bite", ACTION_NORMAL + 9) })));

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array({ new NextAction("cower", ACTION_EMERGENCY + 1) })));

    triggers.push_back(new TriggerNode(
        "faerie fire (feral)",
        NextAction::array({ new NextAction("faerie fire (feral)", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "tiger's fury",
        NextAction::array({ new NextAction("tiger's fury", ACTION_EMERGENCY + 1) })));

    triggers.push_back(new TriggerNode(
        "entangling roots",
        NextAction::array({ new NextAction("entangling roots on cc", ACTION_HIGH + 1) })));

}

void CatAoeDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("swipe (cat)", ACTION_HIGH + 2) })));
}

