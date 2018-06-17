
#include "../../playerbot.h"
#include "PriestActions.h"

using namespace ai;


ActionList CastAbolishDiseaseAction::getAlternatives()
{
    return NextAction::merge(NextAction::array({ new NextAction("cure disease") }), CastSpellAction::getAlternatives());
}

ActionList CastAbolishDiseaseOnPartyAction::getAlternatives()
{
    return NextAction::merge(NextAction::array({ new NextAction("cure disease on party") }), CastSpellAction::getAlternatives());
}

