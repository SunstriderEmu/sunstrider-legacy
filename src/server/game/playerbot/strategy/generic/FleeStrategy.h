#pragma once

namespace ai
{
    class FleeStrategy : public Strategy
    {
    public:
        FleeStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual std::string getName() { return "flee"; };
    };

    class FleeFromAddsStrategy : public Strategy
    {
    public:
        FleeFromAddsStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual std::string getName() { return "flee from adds"; };
    };

}
