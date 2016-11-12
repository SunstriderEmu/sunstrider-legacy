/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Balinda
SD%Complete: 
SDComment: Timers should be adjusted
EndScriptData */

#define YELL_AGGRO                  -2100019
#define YELL_EVADE                  -2100020

#define SPELL_ARCANE_EXPLOSION      46608
#define SPELL_CONE_OF_COLD          38384
#define SPELL_FIREBALL              46988
#define SPELL_FROSTBOLT             46987
#define WATER_ELEMENTAL             25040
#define SPELL_WATERBOLT             46983

struct  mob_water_elementalAI : public ScriptedAI
{
    mob_water_elementalAI(Creature *c) : ScriptedAI(c) 
    {
        SetRestoreCombatMovementOnOOM(true);
    }

    uint32 WaterBoltTimer;
    uint64 balindaGUID;
    uint32 ResetTimer;

    void Reset() override {
        WaterBoltTimer  = 3000;
        ResetTimer      = 5000;
        SetCombatMovementAllowed(false);
    }
    
    void JustDied(Unit* killer) override;
    void SummonedCreatureDespawn(Creature*) override;
    
    void UpdateAI(const uint32 diff) override {
        if (!UpdateVictim())
            return;

        if (WaterBoltTimer <= diff) {
            DoCast(me->GetVictim(), SPELL_WATERBOLT);
            WaterBoltTimer = 5000;
        } else WaterBoltTimer -= diff;

        // check if creature is not outside of building
        if (ResetTimer <= diff) {
            float x, y, z;
            me->GetPosition(x, y, z);
            if (x > -6)
                EnterEvadeMode();
            ResetTimer = 5000;
        } else ResetTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct  boss_balindaAI : public ScriptedAI
{
    boss_balindaAI(Creature *c) : ScriptedAI(c), summons(me) 
    {
        SetRestoreCombatMovementOnOOM(true);
    }


    uint32 ArcaneExplosionTimer;
    uint32 ConeofcoldTimer;
    uint32 FireboltTimer;
    uint32 FrostboltTimer;
    uint32 ResetTimer;
    uint32 WaterElementalTimer;
       
    SummonList summons;
       
    void Reset() override
    {
        ArcaneExplosionTimer    = (10+rand()%5)*IN_MILLISECONDS;
        ConeofcoldTimer         = 8000;
        FireboltTimer           = 1000;
        FrostboltTimer          = 4000;
        ResetTimer              = 5000;
        WaterElementalTimer     = 0;

        summons.DespawnAll();
        
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
        SetCombatMovementAllowed(false);
    }

    void EnterCombat(Unit *who) override
    {
        DoScriptText(YELL_AGGRO, me);
    }

    void JustDied(Unit* Killer) override
    {
        summons.DespawnAll();
    }
    
    void JustSummoned(Creature* pSummon) override
    {
        summons.Summon(pSummon);
    }
    
    void SummonedCreatureDespawn(Creature* pSummon) override { summons.Despawn(pSummon); }

    void UpdateAI(const uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (WaterElementalTimer <= diff) {
            if (Creature* elemental = DoSpawnCreature(WATER_ELEMENTAL, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 45000)) {
                ((mob_water_elementalAI*)elemental->AI())->balindaGUID = me->GetGUID();
                elemental->SetLevel(70);
                elemental->AI()->AttackStart(SelectTarget(SELECT_TARGET_RANDOM, 0));
                elemental->SetFaction(me->GetFaction());         
            }
            WaterElementalTimer = 50000;
        } else WaterElementalTimer -= diff;

        if (ArcaneExplosionTimer <= diff) {
            if(DoCast(me->GetVictim(), SPELL_ARCANE_EXPLOSION) == SPELL_CAST_OK)
                ArcaneExplosionTimer = (10+rand()%5)*IN_MILLISECONDS;
        } else ArcaneExplosionTimer -= diff;

        if (ConeofcoldTimer <= diff) {
            if(DoCast(me->GetVictim(), SPELL_CONE_OF_COLD) == SPELL_CAST_OK)
                ConeofcoldTimer = (10+rand()%10)*IN_MILLISECONDS;
        } else ConeofcoldTimer -= diff;

        if (FireboltTimer <= diff) {
            if(DoCast(me->GetVictim(), SPELL_FIREBALL) == SPELL_CAST_OK)
                FireboltTimer = (5+rand()%4)*IN_MILLISECONDS;
        } else FireboltTimer -= diff;

        if (FrostboltTimer <= diff) {
            if(DoCast(me->GetVictim(), SPELL_FROSTBOLT) == SPELL_CAST_OK)
                FrostboltTimer = (4+rand()%8)*IN_MILLISECONDS;
        } else FrostboltTimer -= diff;

        // check if creature is not outside of building
        if(ResetTimer <= diff) {
            float x, y, z;
            me->GetPosition(x, y, z);
            if (x > -6) {
                DoScriptText(YELL_EVADE, me);
                EnterEvadeMode();
            }
            ResetTimer = 5000;
        } else ResetTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

void mob_water_elementalAI::JustDied(Unit *killer)
{
       Creature* balinda = (ObjectAccessor::GetCreature((*me), balindaGUID));
       ((boss_balindaAI*)balinda->AI())->WaterElementalTimer = 8000;
}

void mob_water_elementalAI::SummonedCreatureDespawn(Creature*)
{
       Creature* balinda = (ObjectAccessor::GetCreature((*me), balindaGUID));
       ((boss_balindaAI*)balinda->AI())->WaterElementalTimer = 8000;
}

CreatureAI* GetAI_boss_balinda(Creature *_Creature)
{
    return new boss_balindaAI (_Creature);
}

CreatureAI* GetAI_mob_water_elemental(Creature *_Creature)
{
    return new mob_water_elementalAI (_Creature);
}

void AddSC_boss_balinda()
{
    OLDScript *newscript;
    
    newscript = new OLDScript;
    newscript->Name = "boss_balinda";
    newscript->GetAI = &GetAI_boss_balinda;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name = "mob_water_elemental";
    newscript->GetAI = &GetAI_mob_water_elemental;
    sScriptMgr->RegisterOLDScript(newscript);
};
