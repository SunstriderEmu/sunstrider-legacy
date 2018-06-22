
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "PaladinBuffStrategies.h"

using namespace ai;

void PaladinBuffManaStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "seal",
        NextAction::array({ std::make_shared<NextAction>("seal of wisdom", 90.0f) })));
}

void PaladinBuffHealthStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "seal",
        NextAction::array({ std::make_shared<NextAction>("seal of light", 90.0f) })));
}

void PaladinBuffSpeedStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "crusader aura",
        NextAction::array({ std::make_shared<NextAction>("crusader aura", 40.0f) })));
}

void PaladinBuffDpsStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "seal",
        NextAction::array({ std::make_shared<NextAction>("seal of vengeance", 89.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "retribution aura",
        NextAction::array({ std::make_shared<NextAction>("retribution aura", 90.0f) })));
}

void PaladinShadowResistanceStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "shadow resistance aura",
        NextAction::array({ std::make_shared<NextAction>("shadow resistance aura", 90.0f) })));
}

void PaladinFrostResistanceStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "frost resistance aura",
        NextAction::array({ std::make_shared<NextAction>("frost resistance aura", 90.0f) })));
}

void PaladinFireResistanceStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "fire resistance aura",
        NextAction::array({ std::make_shared<NextAction>("fire resistance aura", 90.0f) })));
}


void PaladinBuffArmorStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "seal",
        NextAction::array({ std::make_shared<NextAction>("seal of light", 89.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "devotion aura",
        NextAction::array({ std::make_shared<NextAction>("devotion aura", 90.0f) })));
}

