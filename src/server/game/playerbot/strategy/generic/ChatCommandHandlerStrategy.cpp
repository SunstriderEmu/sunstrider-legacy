//
#include "../../playerbot.h"
#include "ChatCommandHandlerStrategy.h"

using namespace ai;

class ChatCommandActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
public:
    ChatCommandActionNodeFactoryInternal()
    {
        creators["tank attack chat shortcut"] = &tank_attack_chat_shortcut;
    }

private:
    static std::shared_ptr<ActionNode> tank_attack_chat_shortcut(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("tank attack chat shortcut",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("attack my target", 100.0f) }));
    }
};

void ChatCommandHandlerStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "rep",
        NextAction::array({ std::make_shared<NextAction>("reputation", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "q",
        NextAction::array( {
            std::make_shared<NextAction>("query quest", relevance),
            std::make_shared<NextAction>("query item usage", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "add all loot",
        NextAction::array({ std::make_shared<NextAction>("add all loot", relevance), std::make_shared<NextAction>("loot", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "u",
        NextAction::array({ std::make_shared<NextAction>("use", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "c",
        NextAction::array({ std::make_shared<NextAction>("item count", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "e",
        NextAction::array({ std::make_shared<NextAction>("equip", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "ue",
        NextAction::array({ std::make_shared<NextAction>("unequip", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "t",
        NextAction::array({ std::make_shared<NextAction>("trade", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "nt",
        NextAction::array({ std::make_shared<NextAction>("trade", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "s",
        NextAction::array({ std::make_shared<NextAction>("sell", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "b",
        NextAction::array({ std::make_shared<NextAction>("buy", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "r",
        NextAction::array({ std::make_shared<NextAction>("reward", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "attack",
        NextAction::array({ std::make_shared<NextAction>("attack my target", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "accept",
        NextAction::array({ std::make_shared<NextAction>("accept quest", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "follow",
        NextAction::array({ std::make_shared<NextAction>("follow chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "stay",
        NextAction::array({ std::make_shared<NextAction>("stay chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "flee",
        NextAction::array({ std::make_shared<NextAction>("flee chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "tank attack",
        NextAction::array({ std::make_shared<NextAction>("tank attack chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "grind",
        NextAction::array({ std::make_shared<NextAction>("grind chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "talk",
        NextAction::array({ std::make_shared<NextAction>("gossip hello", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cast",
        NextAction::array({ std::make_shared<NextAction>("cast custom spell", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "revive",
        NextAction::array({ std::make_shared<NextAction>("spirit healer", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "runaway",
        NextAction::array({ std::make_shared<NextAction>("runaway chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "warning",
        NextAction::array({ std::make_shared<NextAction>("runaway chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "max dps",
        NextAction::array({ std::make_shared<NextAction>("max dps chat shortcut", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "attackers",
        NextAction::array({ std::make_shared<NextAction>("tell attackers", relevance) })));
}



ChatCommandHandlerStrategy::ChatCommandHandlerStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<ChatCommandActionNodeFactoryInternal>());

    supported.push_back("quests");
    supported.push_back("stats");
    supported.push_back("leave");
    supported.push_back("reputation");
    supported.push_back("log");
    supported.push_back("los");
    supported.push_back("drop");
    supported.push_back("ll");
    supported.push_back("release");
    supported.push_back("teleport");
    supported.push_back("taxi");
    supported.push_back("repair");
    supported.push_back("talents");
    supported.push_back("spells");
    supported.push_back("co");
    supported.push_back("nc");
    supported.push_back("dead");
    supported.push_back("trainer");
    supported.push_back("chat");
    supported.push_back("home");
    supported.push_back("destroy");
    supported.push_back("reset ai");
    supported.push_back("emote");
    supported.push_back("buff");
    supported.push_back("help");
    supported.push_back("gb");
    supported.push_back("bank");
    supported.push_back("invite");
    supported.push_back("spell");
    supported.push_back("rti");
    supported.push_back("position");
    supported.push_back("summon");
    supported.push_back("who");
    supported.push_back("save mana");
    supported.push_back("formation");
}
