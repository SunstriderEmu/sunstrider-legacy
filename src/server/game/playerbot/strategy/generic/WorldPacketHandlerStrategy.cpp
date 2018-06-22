
#include "../../playerbot.h"
#include "WorldPacketHandlerStrategy.h"

using namespace ai;

void WorldPacketHandlerStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "group invite",
        NextAction::array({ std::make_shared<NextAction>("accept invitation", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "group set leader",
        NextAction::array({ std::make_shared<NextAction>("leader", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "not enough money",
        NextAction::array({ std::make_shared<NextAction>("tell not enough money", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "not enough reputation",
        NextAction::array({ std::make_shared<NextAction>("tell not enough reputation", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cannot equip",
        NextAction::array({ std::make_shared<NextAction>("tell cannot equip", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "use game object",
        NextAction::array( {
            std::make_shared<NextAction>("add loot", relevance),
            std::make_shared<NextAction>("use meeting stone", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "gossip hello",
        NextAction::array({
            std::make_shared<NextAction>("trainer", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "activate taxi",
        NextAction::array({ std::make_shared<NextAction>("remember taxi", relevance), std::make_shared<NextAction>("taxi", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "taxi done",
        NextAction::array({ std::make_shared<NextAction>("taxi", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "trade status",
        NextAction::array({ std::make_shared<NextAction>("accept trade", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "area trigger",
        NextAction::array({ std::make_shared<NextAction>("reach area trigger", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "within area trigger",
        NextAction::array({ std::make_shared<NextAction>("area trigger", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "loot response",
        NextAction::array({ std::make_shared<NextAction>("store loot", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "item push result",
        NextAction::array({ std::make_shared<NextAction>("query item usage", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "ready check finished",
        NextAction::array({ std::make_shared<NextAction>("finish ready check", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "often",
        NextAction::array({ std::make_shared<NextAction>("security check", relevance), std::make_shared<NextAction>("check mail", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "guild invite",
        NextAction::array({ std::make_shared<NextAction>("guild accept", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "seldom",
        NextAction::array({ std::make_shared<NextAction>("lfg leave", relevance) })));

}

WorldPacketHandlerStrategy::WorldPacketHandlerStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
    supported.push_back("loot roll");
    supported.push_back("check mount state");
    supported.push_back("quest objective completed");
    supported.push_back("party command");
    supported.push_back("ready check");
    supported.push_back("uninvite");
    supported.push_back("lfg role check");
    supported.push_back("lfg teleport");
}


void ReadyCheckStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "timer",
        NextAction::array({ std::make_shared<NextAction>("ready check", relevance) })));
}
