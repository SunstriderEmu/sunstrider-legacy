#pragma once

#include "GenericDruidStrategy.h"
#include "DruidAiObjectContext.h"

namespace ai
{
    class ShapeshiftDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
    {
    public:
        ShapeshiftDruidStrategyActionNodeFactory()
        {
            creators["rejuvenation"] = &rejuvenation;
            creators["regrowth"] = &regrowth;
            creators["healing touch"] = &healing_touch;
            creators["rejuvenation on party"] = &rejuvenation_on_party;
            creators["regrowth on party"] = &regrowth_on_party;
            creators["healing touch on party"] = &healing_touch_on_party;
        }
    private:
        static std::shared_ptr<ActionNode> regrowth(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("regrowth",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("healing touch") }),
                /*C*/ NextAction::array({ std::make_shared<NextAction>("melee", 10.0f) }));
        }
        static std::shared_ptr<ActionNode> rejuvenation(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("rejuvenation",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> healing_touch(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("healing touch",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> regrowth_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("regrowth on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("healing touch on party") }),
                /*C*/ NextAction::array({ std::make_shared<NextAction>("melee", 10.0f) }));
        }
        static std::shared_ptr<ActionNode> rejuvenation_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("rejuvenation on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> healing_touch_on_party(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("healing touch on party",
                /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
                /*A*/ ActionList(),
                /*C*/ ActionList());
        }
    };

    class FeralDruidStrategy : public GenericDruidStrategy
    {
    protected:
        FeralDruidStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual int GetType() { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_MELEE; }
    };

}
