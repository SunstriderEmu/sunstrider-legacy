
#include "../../playerbot.h"
#include "WarriorActions.h"

using namespace ai;

ActionList CastRendAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ std::make_shared<NextAction>("reach melee") }), CastDebuffSpellAction::getPrerequisites());
}

ActionList CastRendOnAttackerAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ std::make_shared<NextAction>("reach melee") }), CastDebuffSpellOnAttackerAction::getPrerequisites());
}

ActionList CastDisarmAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ std::make_shared<NextAction>("reach melee"), std::make_shared<NextAction>("defensive stance") }), CastDebuffSpellAction::getPrerequisites());
}

ActionList CastSunderArmorAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ std::make_shared<NextAction>("reach melee") }), CastDebuffSpellAction::getPrerequisites());
}

ActionList CastRevengeAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ std::make_shared<NextAction>("defensive stance") }), CastMeleeSpellAction::getPrerequisites());
}
