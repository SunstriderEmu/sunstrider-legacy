#pragma once

namespace ai
{
    class ShadowPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
    {
    public:
        ShadowPriestStrategyActionNodeFactory()
        {
            creators["mind flay"] = &mind_flay;
            creators["mind blast"] = &mind_blast;
            creators["dispersion"] = &dispersion;
        }
    private:
        static std::shared_ptr<ActionNode> mind_flay(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("mind flay",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("shoot") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> mind_blast(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("mind blast",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("mind flay") }),
                /*C*/ ActionList());
        }
        static std::shared_ptr<ActionNode> dispersion(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("dispersion",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("mana potion") }),
                /*C*/ ActionList());
        }
    };
};
