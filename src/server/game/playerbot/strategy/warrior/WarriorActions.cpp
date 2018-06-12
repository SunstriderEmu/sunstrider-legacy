
#include "../../playerbot.h"
#include "WarriorActions.h"

using namespace ai;

NextAction** CastRendAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ new NextAction("reach melee") }), CastDebuffSpellAction::getPrerequisites());
}

NextAction** CastRendOnAttackerAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ new NextAction("reach melee") }), CastDebuffSpellOnAttackerAction::getPrerequisites());
}

NextAction** CastDisarmAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ new NextAction("reach melee"), new NextAction("defensive stance") }), CastDebuffSpellAction::getPrerequisites());
}

NextAction** CastSunderArmorAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ new NextAction("reach melee") }), CastDebuffSpellAction::getPrerequisites());
}

NextAction** CastRevengeAction::getPrerequisites()
{
    return NextAction::merge( NextAction::array({ new NextAction("defensive stance") }), CastMeleeSpellAction::getPrerequisites());
}
