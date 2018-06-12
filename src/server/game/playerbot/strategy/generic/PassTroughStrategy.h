#pragma once

namespace ai
{
    class PassTroughStrategy : public Strategy
    {
    public:
        PassTroughStrategy(PlayerbotAI* ai, float relevance = 100.0f) : Strategy(ai), relevance(relevance) {}

        virtual void InitTriggers(std::list<TriggerNode*> &triggers)
        {
            for (list<std::string>::iterator i = supported.begin(); i != supported.end(); i++)
            {
                std::string s = i->c_str();

                triggers.push_back(new TriggerNode(
                    s, 
                    NextAction::array({ new NextAction(s, relevance) })));
            }
        }

    protected:
        list<std::string> supported;
        float relevance;
    };
}
