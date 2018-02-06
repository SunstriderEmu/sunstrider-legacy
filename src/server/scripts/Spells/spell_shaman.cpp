#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "Player.h"
enum ShamanSpells
{
    SPELL_SHAMAN_BIND_SIGHT                     = 6277,
    SPELL_SHAMAN_EARTH_SHIELD_HEAL              = 379,
    SPELL_SHAMAN_ELEMENTAL_MASTERY              = 16166,
    SPELL_SHAMAN_FIRE_NOVA_R1                   = 1535,
    SPELL_SHAMAN_FIRE_NOVA_TRIGGERED_R1         = 8349,
    SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD          = 23552,
    SPELL_SHAMAN_ITEM_LIGHTNING_SHIELD_DAMAGE   = 27635,
    SPELL_SHAMAN_ITEM_MANA_SURGE                = 23571,
    SPELL_SHAMAN_LIGHTNING_SHIELD_R1            = 26364,
    SPELL_SHAMAN_MANA_TIDE_TOTEM                = 39609,
    SPELL_SHAMAN_NATURE_GUARDIAN                = 31616,
    SPELL_SHAMAN_NATURE_GUARDIAN_THREAT         = 39301, // Serverside
    SPELL_SHAMAN_TOTEM_EARTHBIND_TOTEM          = 6474,
    SPELL_SHAMAN_TOTEMIC_MASTERY                = 38437,
    SPELL_SHAMAN_TOTEMIC_POWER_MP5              = 28824,
    SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER      = 28825,
    SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER     = 28826,
    SPELL_SHAMAN_TOTEMIC_POWER_ARMOR            = 28827,
    SPELL_SHAMAN_WINDFURY_WEAPON_R1             = 8232,
    SPELL_SHAMAN_WINDFURY_ATTACK_MH             = 25504,
    SPELL_SHAMAN_WINDFURY_ATTACK_OH             = 33750,
    SPELL_SHAMAN_ENERGY_SURGE                   = 40465,
    SPELL_SHAMAN_POWER_SURGE                    = 40466,
    SPELL_SHAMAN_FLAMETONGUE_ATTACK             = 10444,
    SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_R1     = 45284,
    SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_R1    = 45297,
    SPELL_SHAMAN_LIGHTNING_SHIELD_DAMAGE_R1     = 26364,
    SPELL_SHAMAN_SHAMANISTIC_RAGE_PROC          = 30824,
};

//TODO: should this be moved to DB?
class spell_sham_imprvd_weapon_totems : public SpellScriptLoader
{
public:
    spell_sham_imprvd_weapon_totems() : SpellScriptLoader("spell_sham_imprvd_weapon_totems") { }

    class spell_sham_imprvd_weapon_totems_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sham_imprvd_weapon_totems_AuraScript);

        void HandleEffectCalcSpellMod(AuraEffect const* aurEff, SpellModifier*& spellMod)
        {
            if (!spellMod)
            {
                // + effect value for Aspect of the Viper
                spellMod = new SpellModifier(aurEff->GetBase());
                spellMod->op = SPELLMOD_EFFECT1;
                spellMod->type = SPELLMOD_PCT;
                spellMod->spellId = GetId();
                switch (aurEff->GetEffIndex())
                {
                case 0:
                    spellMod->mask = 0x00200000000LL;    // Windfury Totem
                    break;
                case 1:
                    spellMod->mask = 0x00400000000LL;    // Flametongue Totem
                    break;
                }
            }
            spellMod->value = aurEff->GetAmount();
        }

        void Register()
        {
            DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_sham_imprvd_weapon_totems_AuraScript::HandleEffectCalcSpellMod, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sham_imprvd_weapon_totems_AuraScript();
    }
};

// 33757 - Windfury Weapon (Passive)
class spell_sha_windfury_weapon : public SpellScriptLoader
{
public:
    spell_sha_windfury_weapon() : SpellScriptLoader("spell_sha_windfury_weapon") { }

    class spell_sha_windfury_weapon_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_windfury_weapon_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_WINDFURY_WEAPON_R1,
                    SPELL_SHAMAN_WINDFURY_ATTACK_MH,
                    SPELL_SHAMAN_WINDFURY_ATTACK_OH
                });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Player* player = eventInfo.GetActor()->ToPlayer();
            if (!player)
                return false;

            Item* item = player->GetItemByGuid(GetAura()->GetCastItemGUID());
            if (!item || !item->IsEquipped())
                return false;

            WeaponAttackType attType = static_cast<WeaponAttackType>(player->GetAttackBySlot(item->GetSlot()));
            if (attType != BASE_ATTACK && attType != OFF_ATTACK)
                return false;

            if (((attType == BASE_ATTACK) && !(eventInfo.GetTypeMask() & PROC_FLAG_DONE_MAINHAND_ATTACK)) ||
                ((attType == OFF_ATTACK) && !(eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)))
                return false;

            return true;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Player* player = eventInfo.GetActor()->ToPlayer();

            uint32 spellId = 0;
            WeaponAttackType attType = BASE_ATTACK;
            if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_MAINHAND_ATTACK)
                spellId = SPELL_SHAMAN_WINDFURY_ATTACK_MH;

            if (eventInfo.GetTypeMask() & PROC_FLAG_DONE_OFFHAND_ATTACK)
            {
                spellId = SPELL_SHAMAN_WINDFURY_ATTACK_OH;
                attType = OFF_ATTACK;
            }

            Item* item = ASSERT_NOTNULL(player->GetWeaponForAttack(attType));

            int32 enchantId = static_cast<int32>(item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT));
            uint32 apSpell = 0;
            switch (enchantId)
            {
                case 283:  apSpell = 33757; break;   //1 Rank
                case 284:  apSpell = 33756; break;   //2 Rank
                case 525:  apSpell = 33755; break;   //3 Rank
                case 1669: apSpell = 33754; break;   //4 Rank
                case 2636: apSpell = 33727; break;   //5 Rank
            }
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(apSpell);
            if (!spellInfo)
                return;

            int32 extraAttackPower = spellInfo->Effects[EFFECT_0].BasePoints;
            if (!extraAttackPower)
                return;

            // Value gained from additional AP
            int32 amount = static_cast<int32>(extraAttackPower / 14.f * player->GetAttackTime(attType) / 1000.f);

            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(amount);
            // Attack twice
            for (uint8 i = 0; i < 2; ++i)
                player->CastSpell(eventInfo.GetProcTarget(), spellId, args);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_sha_windfury_weapon_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_sha_windfury_weapon_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_windfury_weapon_AuraScript();
    }
};

// 40463 - Shaman Tier 6 Trinket
class spell_sha_item_t6_trinket : public SpellScriptLoader
{
public:
    spell_sha_item_t6_trinket() : SpellScriptLoader("spell_sha_item_t6_trinket") { }

    class spell_sha_item_t6_trinket_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_item_t6_trinket_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_ENERGY_SURGE,
                    SPELL_SHAMAN_POWER_SURGE
                });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
            if (!spellInfo)
                return;

            uint32 spellId;
            int32 chance;

            // Lesser Healing Wave
            if (spellInfo->SpellFamilyFlags & 0x00000080)
            {
                spellId = SPELL_SHAMAN_ENERGY_SURGE;
                chance = 10;
            }
            // Lightning Bolt
            else if (spellInfo->SpellFamilyFlags & 0x00000001)
            {
                spellId = SPELL_SHAMAN_ENERGY_SURGE;
                chance = 15;
            }
            // Stormstrike
            else if (spellInfo->SpellFamilyFlags & 0x0000001000000000LL)
            {
                spellId = SPELL_SHAMAN_POWER_SURGE;
                chance = 50;
            }
            else
                return;

            if (roll_chance_i(chance))
                eventInfo.GetActor()->CastSpell(nullptr, spellId, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_sha_item_t6_trinket_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_item_t6_trinket_AuraScript();
    }
};

// -30675 - Lightning Overload
class spell_sha_lightning_overload : public SpellScriptLoader
{
public:
    spell_sha_lightning_overload() : SpellScriptLoader("spell_sha_lightning_overload") { }

    class spell_sha_lightning_overload_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_lightning_overload_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_R1,
                    SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_R1
                });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
            if (!spellInfo)
                return;

            uint32 spellId;

            // Lightning Bolt
            if (spellInfo->SpellFamilyFlags & 0x00000001)
                spellId = sSpellMgr->GetSpellWithRank(SPELL_SHAMAN_LIGHTNING_BOLT_OVERLOAD_R1, spellInfo->GetRank());
            // Chain Lightning
            else
            {
                // Chain lightning has [LightOverload_Proc_Chance] / [Max_Number_of_Targets] chance to proc of each individual target hit.
                // A maxed LO would have a 33% / 3 = 11% chance to proc of each target.
                // LO chance was already "accounted" at the proc chance roll, now need to divide the chance by [Max_Number_of_Targets]
                float chance = 100.0f / spellInfo->Effects[EFFECT_0].ChainTarget;
                if (!roll_chance_f(chance))
                    return;

                spellId = sSpellMgr->GetSpellWithRank(SPELL_SHAMAN_CHAIN_LIGHTNING_OVERLOAD_R1, spellInfo->GetRank());
            }

            
            
            
            
            

            eventInfo.GetActor()->CastSpell(eventInfo.GetProcTarget(), spellId, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_sha_lightning_overload_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_lightning_overload_AuraScript();
    }
};

// 28823 - Totemic Power
class spell_sha_t3_6p_bonus : public SpellScriptLoader
{
public:
    spell_sha_t3_6p_bonus() : SpellScriptLoader("spell_sha_t3_6p_bonus") { }

    class spell_sha_t3_6p_bonus_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_t3_6p_bonus_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_TOTEMIC_POWER_ARMOR,
                    SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER,
                    SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER,
                    SPELL_SHAMAN_TOTEMIC_POWER_MP5
                });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            uint32 spellId;
            Unit* caster = eventInfo.GetActor();
            Unit* target = eventInfo.GetProcTarget();

            switch (target->GetClass())
            {
            case CLASS_PALADIN:
            case CLASS_PRIEST:
            case CLASS_SHAMAN:
            case CLASS_DRUID:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_MP5;
                break;
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_SPELL_POWER;
                break;
            case CLASS_HUNTER:
            case CLASS_ROGUE:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_ATTACK_POWER;
                break;
            case CLASS_WARRIOR:
                spellId = SPELL_SHAMAN_TOTEMIC_POWER_ARMOR;
                break;
            default:
                return;
            }

            caster->CastSpell(target, spellId, aurEff);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_sha_t3_6p_bonus_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_t3_6p_bonus_AuraScript();
    }
};

enum TotemOfFlowingWater
{
    SPELL_LESSER_HEALING_WAVE_MANA = 28850
};

// Item - 23005: Totem of Flowing Water
// 28849 - Lesser Healing Wave
class spell_item_totem_of_flowing_water : public AuraScript
{
    PrepareAuraScript(spell_item_totem_of_flowing_water);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_LESSER_HEALING_WAVE_MANA });
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActor()->CastSpell(nullptr, SPELL_LESSER_HEALING_WAVE_MANA, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_totem_of_flowing_water::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// -974 - Earth Shield
// WoWWiki: "The cooldown before another charge will trigger is 3-4 seconds." -> cd defined in spell_proc table
class spell_sha_earth_shield : public SpellScriptLoader
{
public:
    spell_sha_earth_shield() : SpellScriptLoader("spell_sha_earth_shield") { }

    class spell_sha_earth_shield_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_earth_shield_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_SHAMAN_EARTH_SHIELD_HEAL });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();

            CastSpellExtraArgs args(aurEff);
            args.OriginalCaster = GetCasterGUID();
            args.AddSpellBP0(aurEff->GetAmount());
            GetTarget()->CastSpell(GetTarget(), SPELL_SHAMAN_EARTH_SHIELD_HEAL, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_sha_earth_shield_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_earth_shield_AuraScript();
    }
};

// -30881 - Nature's Guardian
class spell_sha_nature_guardian : public SpellScriptLoader
{
public:
    spell_sha_nature_guardian() : SpellScriptLoader("spell_sha_nature_guardian") { }

    class spell_sha_nature_guardian_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_nature_guardian_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    SPELL_SHAMAN_NATURE_GUARDIAN,
                    SPELL_SHAMAN_NATURE_GUARDIAN_THREAT
                });
        }

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            DamageInfo* damageInfo = eventInfo.GetDamageInfo();
            if (!damageInfo || !damageInfo->GetDamage())
                return false;

            if (!roll_chance_i(10 * GetAura()->GetSpellInfo()->GetRank()))
                return false;

            int32 healthpct = 30; //GetSpellInfo()->Effects[EFFECT_1].CalcValue();
            if (Unit* target = eventInfo.GetActionTarget())
                if (target->HealthBelowPctDamaged(healthpct, damageInfo->GetDamage()))
                    return true;

            return false;
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Unit* target = eventInfo.GetActionTarget();
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(CalculatePct(target->GetMaxHealth(), aurEff->GetAmount()));
            target->CastSpell(target, SPELL_SHAMAN_NATURE_GUARDIAN, args);
            if (Unit* attacker = eventInfo.GetActor())
                target->CastSpell(attacker, SPELL_SHAMAN_NATURE_GUARDIAN_THREAT, true);
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_sha_nature_guardian_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_sha_nature_guardian_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_nature_guardian_AuraScript();
    }
};

// 30823 - Shamanistic Rage
class spell_sha_shamanistic_rage : public SpellScriptLoader
{
public:
    spell_sha_shamanistic_rage() : SpellScriptLoader("spell_sha_shamanistic_rage") { }

    class spell_sha_shamanistic_rage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sha_shamanistic_rage_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ SPELL_SHAMAN_SHAMANISTIC_RAGE_PROC });
        }

        void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
        {
            PreventDefaultAction();

            Unit* target = GetTarget();
            int32 amount = CalculatePct(static_cast<int32>(target->GetTotalAttackPowerValue(BASE_ATTACK)), aurEff->GetAmount());
            CastSpellExtraArgs args(aurEff);
            args.AddSpellBP0(amount);
            target->CastSpell(target, SPELL_SHAMAN_SHAMANISTIC_RAGE_PROC, args);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_sha_shamanistic_rage_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_sha_shamanistic_rage_AuraScript();
    }
};

void AddSC_shaman_spell_scripts()
{
    new spell_sham_imprvd_weapon_totems();
    new spell_sha_windfury_weapon();
    new spell_sha_item_t6_trinket();
    new spell_sha_lightning_overload();
    new spell_sha_t3_6p_bonus();
    RegisterAuraScript(spell_item_totem_of_flowing_water);
    new spell_sha_earth_shield();
    new spell_sha_nature_guardian();
    new spell_sha_shamanistic_rage();
}
