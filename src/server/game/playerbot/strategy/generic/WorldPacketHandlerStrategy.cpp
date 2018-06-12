
#include "../../playerbot.h"
#include "WorldPacketHandlerStrategy.h"

using namespace ai;

void WorldPacketHandlerStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "group invite",
        NextAction::array({ new NextAction("accept invitation", relevance) })));

    triggers.push_back(new TriggerNode(
        "group set leader",
        NextAction::array({ new NextAction("leader", relevance) })));

    triggers.push_back(new TriggerNode(
        "not enough money",
        NextAction::array({ new NextAction("tell not enough money", relevance) })));

    triggers.push_back(new TriggerNode(
        "not enough reputation",
        NextAction::array({ new NextAction("tell not enough reputation", relevance) })));

    triggers.push_back(new TriggerNode(
        "cannot equip",
        NextAction::array({ new NextAction("tell cannot equip", relevance) })));

    triggers.push_back(new TriggerNode(
        "use game object",
        NextAction::array( {
            new NextAction("add loot", relevance),
            new NextAction("use meeting stone", relevance) })));

    triggers.push_back(new TriggerNode(
        "gossip hello",
        NextAction::array({
            new NextAction("trainer", relevance) })));

    triggers.push_back(new TriggerNode(
        "activate taxi",
        NextAction::array({ new NextAction("remember taxi", relevance), new NextAction("taxi", relevance) })));

    triggers.push_back(new TriggerNode(
        "taxi done",
        NextAction::array({ new NextAction("taxi", relevance) })));

    triggers.push_back(new TriggerNode(
        "trade status",
        NextAction::array({ new NextAction("accept trade", relevance) })));

    triggers.push_back(new TriggerNode(
        "area trigger",
        NextAction::array({ new NextAction("reach area trigger", relevance) })));

    triggers.push_back(new TriggerNode(
        "within area trigger",
        NextAction::array({ new NextAction("area trigger", relevance) })));

    triggers.push_back(new TriggerNode(
        "loot response",
        NextAction::array({ new NextAction("store loot", relevance) })));

    triggers.push_back(new TriggerNode(
        "item push result",
        NextAction::array({ new NextAction("query item usage", relevance) })));

    triggers.push_back(new TriggerNode(
        "ready check finished",
        NextAction::array({ new NextAction("finish ready check", relevance) })));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array({ new NextAction("security check", relevance), new NextAction("check mail", relevance) })));

    triggers.push_back(new TriggerNode(
        "guild invite",
        NextAction::array({ new NextAction("guild accept", relevance) })));

    triggers.push_back(new TriggerNode(
        "seldom",
        NextAction::array({ new NextAction("lfg leave", relevance) })));

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


void ReadyCheckStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "timer",
        NextAction::array({ new NextAction("ready check", relevance) })));
}
