
#include "../../playerbot.h"
#include "QuestStrategies.h"

using namespace ai;

QuestStrategy::QuestStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
    supported.push_back("accept quest");
}

void QuestStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "quest share",
        NextAction::array({ std::make_shared<NextAction>("accept quest share", relevance) })));
}


void DefaultQuestStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "use game object",
        NextAction::array({
            std::make_shared<NextAction>("talk to quest giver", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "gossip hello",
        NextAction::array( {
            std::make_shared<NextAction>("talk to quest giver", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "complete quest",
        NextAction::array({ std::make_shared<NextAction>("talk to quest giver", relevance) })));
}

DefaultQuestStrategy::DefaultQuestStrategy(PlayerbotAI* ai) : QuestStrategy(ai)
{
}



void AcceptAllQuestsStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    QuestStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "use game object",
        NextAction::array({
            std::make_shared<NextAction>("talk to quest giver", relevance), std::make_shared<NextAction>("accept all quests", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "gossip hello",
        NextAction::array({
            std::make_shared<NextAction>("talk to quest giver", relevance), std::make_shared<NextAction>("accept all quests", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "complete quest",
        NextAction::array({
            std::make_shared<NextAction>("talk to quest giver", relevance), std::make_shared<NextAction>("accept all quests", relevance) })));
}

AcceptAllQuestsStrategy::AcceptAllQuestsStrategy(PlayerbotAI* ai) : QuestStrategy(ai)
{
}
