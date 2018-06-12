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
    static ActionNode* tank_attack_chat_shortcut(PlayerbotAI* ai)
    {
        return new ActionNode ("tank attack chat shortcut",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NextAction::array({ new NextAction("attack my target", 100.0f) }));
    }
};

void ChatCommandHandlerStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "rep",
        NextAction::array({ new NextAction("reputation", relevance) })));

    triggers.push_back(new TriggerNode(
        "q",
        NextAction::array( {
            new NextAction("query quest", relevance),
            new NextAction("query item usage", relevance) })));

    triggers.push_back(new TriggerNode(
        "add all loot",
        NextAction::array({ new NextAction("add all loot", relevance), new NextAction("loot", relevance) })));

    triggers.push_back(new TriggerNode(
        "u",
        NextAction::array({ new NextAction("use", relevance) })));

    triggers.push_back(new TriggerNode(
        "c",
        NextAction::array({ new NextAction("item count", relevance) })));

    triggers.push_back(new TriggerNode(
        "e",
        NextAction::array({ new NextAction("equip", relevance) })));

    triggers.push_back(new TriggerNode(
        "ue",
        NextAction::array({ new NextAction("unequip", relevance) })));

    triggers.push_back(new TriggerNode(
        "t",
        NextAction::array({ new NextAction("trade", relevance) })));

    triggers.push_back(new TriggerNode(
        "nt",
        NextAction::array({ new NextAction("trade", relevance) })));

    triggers.push_back(new TriggerNode(
        "s",
        NextAction::array({ new NextAction("sell", relevance) })));

    triggers.push_back(new TriggerNode(
        "b",
        NextAction::array({ new NextAction("buy", relevance) })));

    triggers.push_back(new TriggerNode(
        "r",
        NextAction::array({ new NextAction("reward", relevance) })));

    triggers.push_back(new TriggerNode(
        "attack",
        NextAction::array({ new NextAction("attack my target", relevance) })));

    triggers.push_back(new TriggerNode(
        "accept",
        NextAction::array({ new NextAction("accept quest", relevance) })));

    triggers.push_back(new TriggerNode(
        "follow",
        NextAction::array({ new NextAction("follow chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "stay",
        NextAction::array({ new NextAction("stay chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "flee",
        NextAction::array({ new NextAction("flee chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "tank attack",
        NextAction::array({ new NextAction("tank attack chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "grind",
        NextAction::array({ new NextAction("grind chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "talk",
        NextAction::array({ new NextAction("gossip hello", relevance) })));

    triggers.push_back(new TriggerNode(
        "cast",
        NextAction::array({ new NextAction("cast custom spell", relevance) })));

    triggers.push_back(new TriggerNode(
        "revive",
        NextAction::array({ new NextAction("spirit healer", relevance) })));

    triggers.push_back(new TriggerNode(
        "runaway",
        NextAction::array({ new NextAction("runaway chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "warning",
        NextAction::array({ new NextAction("runaway chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "max dps",
        NextAction::array({ new NextAction("max dps chat shortcut", relevance) })));

    triggers.push_back(new TriggerNode(
        "attackers",
        NextAction::array({ new NextAction("tell attackers", relevance) })));
}



ChatCommandHandlerStrategy::ChatCommandHandlerStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
    actionNodeFactories.Add(new ChatCommandActionNodeFactoryInternal());

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
