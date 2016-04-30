#pragma once

namespace ai
{
    class EmoteStrategy : public Strategy
    {
    public:
        EmoteStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::list<TriggerNode*> &triggers);
        virtual std::string getName() { return "emote"; }
    };


}
