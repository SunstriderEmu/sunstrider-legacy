#pragma once

namespace ai
{
    class TellTargetStrategy : public Strategy
    {
    public:
        TellTargetStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual std::string getName() { return "TellTarget"; }
    };


}
