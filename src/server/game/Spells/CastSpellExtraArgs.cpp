#include "SpellDefines.h"
#include "SpellAuraEffects.h"

//sun: also inherit ForceHitResult in this case
CastSpellExtraArgs::CastSpellExtraArgs(AuraEffect const* eff) 
	: TriggerFlags(TRIGGERED_FULL_MASK), TriggeringAura(eff), ForceHitResult(eff->GetBase()->GetForceHitResult()) 
{ }
