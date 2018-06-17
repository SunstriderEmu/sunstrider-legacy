//
#include "../../playerbot.h"
#include "RogueMultipliers.h"
#include "DpsRogueStrategy.h"

using namespace ai;

class DpsRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsRogueStrategyActionNodeFactory()
    {
        creators["riposte"] = &riposte;
        creators["mutilate"] = &mutilate;
        creators["sinister strike"] = &sinister_strike;
        creators["kick"] = &kick;
        creators["kidney shot"] = &kidney_shot;
        creators["rupture"] = &rupture;
        creators["backstab"] = &backstab;
    }
private:
    static ActionNode* riposte(PlayerbotAI* ai)
    {
        return new ActionNode ("riposte",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("mutilate") }),
            /*C*/ {});
    }
    static ActionNode* mutilate(PlayerbotAI* ai)
    {
        return new ActionNode ("mutilate",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("sinister strike") }),
            /*C*/ {});
    }
    static ActionNode* sinister_strike(PlayerbotAI* ai)
    {
        return new ActionNode ("sinister strike",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ {});
    }
    static ActionNode* kick(PlayerbotAI* ai)
    {
        return new ActionNode ("kick",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("kidney shot") }),
            /*C*/ {});
    }
    static ActionNode* kidney_shot(PlayerbotAI* ai)
    {
        return new ActionNode ("kidney shot",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* rupture(PlayerbotAI* ai)
    {
        return new ActionNode ("rupture",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("eviscerate") }),
            /*C*/ {});
    }
    static ActionNode* backstab(PlayerbotAI* ai)
    {
        return new ActionNode ("backstab",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("mutilate") }),
            /*C*/ {});
    }
};

DpsRogueStrategy::DpsRogueStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(new DpsRogueStrategyActionNodeFactory());
}

ActionList DpsRogueStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("riposte", ACTION_NORMAL) });
}

void DpsRogueStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "combo points available",
        NextAction::array({ new NextAction("rupture", ACTION_HIGH + 2) })));

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array({ new NextAction("vanish", ACTION_HIGH) })));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array({ new NextAction("evasion", ACTION_EMERGENCY), new NextAction("feint", ACTION_EMERGENCY) })));

    triggers.push_back(new TriggerNode(
        "kick",
        NextAction::array({ new NextAction("kick", ACTION_INTERRUPT + 2) })));

    triggers.push_back(new TriggerNode(
        "kick on enemy healer",
        NextAction::array({ new NextAction("kick on enemy healer", ACTION_INTERRUPT + 1) })));

    triggers.push_back(new TriggerNode(
        "behind target",
        NextAction::array({ new NextAction("backstab", ACTION_NORMAL) })));
}
