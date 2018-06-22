
#include "../../playerbot.h"
#include "MageActions.h"

using namespace ai;

std::shared_ptr<Value<Unit*>> CastPolymorphAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", getName());
}
