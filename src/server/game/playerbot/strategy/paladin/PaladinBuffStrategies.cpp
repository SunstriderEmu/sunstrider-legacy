
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "PaladinBuffStrategies.h"

using namespace ai;

void PaladinBuffManaStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array({ new NextAction("seal of wisdom", 90.0f) })));
}

void PaladinBuffHealthStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array({ new NextAction("seal of light", 90.0f) })));
}

void PaladinBuffSpeedStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "crusader aura",
        NextAction::array({ new NextAction("crusader aura", 40.0f) })));
}

void PaladinBuffDpsStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array({ new NextAction("seal of vengeance", 89.0f) })));

    triggers.push_back(new TriggerNode(
        "retribution aura",
        NextAction::array({ new NextAction("retribution aura", 90.0f) })));
}

void PaladinShadowResistanceStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "shadow resistance aura",
        NextAction::array({ new NextAction("shadow resistance aura", 90.0f) })));
}

void PaladinFrostResistanceStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "frost resistance aura",
        NextAction::array({ new NextAction("frost resistance aura", 90.0f) })));
}

void PaladinFireResistanceStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "fire resistance aura",
        NextAction::array({ new NextAction("fire resistance aura", 90.0f) })));
}


void PaladinBuffArmorStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "seal",
        NextAction::array({ new NextAction("seal of light", 89.0f) })));

    triggers.push_back(new TriggerNode(
        "devotion aura",
        NextAction::array({ new NextAction("devotion aura", 90.0f) })));
}

