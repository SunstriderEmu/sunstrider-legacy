
#include "../../playerbot.h"
#include "PriestActions.h"

using namespace ai;


NextAction** CastAbolishDiseaseAction::getAlternatives()
{
    return NextAction::merge(NextAction::array({ new NextAction("cure disease") }), CastSpellAction::getAlternatives());
}

NextAction** CastAbolishDiseaseOnPartyAction::getAlternatives()
{
    return NextAction::merge(NextAction::array({ new NextAction("cure disease on party") }), CastSpellAction::getAlternatives());
}

