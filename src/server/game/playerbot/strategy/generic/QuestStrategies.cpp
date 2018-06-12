
#include "../../playerbot.h"
#include "QuestStrategies.h"

using namespace ai;

QuestStrategy::QuestStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
    supported.push_back("accept quest");
}

void QuestStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "quest share",
        NextAction::array({ new NextAction("accept quest share", relevance) })));
}


void DefaultQuestStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "use game object",
        NextAction::array({
            new NextAction("talk to quest giver", relevance) })));

    triggers.push_back(new TriggerNode(
        "gossip hello",
        NextAction::array( {
            new NextAction("talk to quest giver", relevance) })));

    triggers.push_back(new TriggerNode(
        "complete quest",
        NextAction::array({ new NextAction("talk to quest giver", relevance) })));
}

DefaultQuestStrategy::DefaultQuestStrategy(PlayerbotAI* ai) : QuestStrategy(ai)
{
}



void AcceptAllQuestsStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "use game object",
        NextAction::array({
            new NextAction("talk to quest giver", relevance), new NextAction("accept all quests", relevance) })));

    triggers.push_back(new TriggerNode(
        "gossip hello",
        NextAction::array({
            new NextAction("talk to quest giver", relevance), new NextAction("accept all quests", relevance) })));

    triggers.push_back(new TriggerNode(
        "complete quest",
        NextAction::array({
            new NextAction("talk to quest giver", relevance), new NextAction("accept all quests", relevance) })));
}

AcceptAllQuestsStrategy::AcceptAllQuestsStrategy(PlayerbotAI* ai) : QuestStrategy(ai)
{
}
