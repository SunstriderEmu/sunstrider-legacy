#pragma once

namespace ai
{
    class CombatStrategy : public Strategy
    {
    public:
        CombatStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual int GetType() { return STRATEGY_TYPE_COMBAT; }
    };

}
