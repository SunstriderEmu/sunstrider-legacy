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
        static ActionNode* mind_flay(PlayerbotAI* ai)
        {
            return new ActionNode ("mind flay",
                /*P*/ {},
                /*A*/ NextAction::array({ new NextAction("shoot") }),
                /*C*/ {});
        }
        static ActionNode* mind_blast(PlayerbotAI* ai)
        {
            return new ActionNode ("mind blast",
                /*P*/ {},
                /*A*/ NextAction::array({ new NextAction("mind flay") }),
                /*C*/ {});
        }
        static ActionNode* dispersion(PlayerbotAI* ai)
        {
            return new ActionNode ("dispersion",
                /*P*/ {},
                /*A*/ NextAction::array({ new NextAction("mana potion") }),
                /*C*/ {});
        }
    };
};
