#pragma once

namespace ai
{
    class PassiveStrategy : public Strategy
    {
    public:
        PassiveStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitMultipliers(std::list<std::shared_ptr<Multiplier>> &multipliers);
        virtual std::string getName() { return "passive"; }
    };


}
