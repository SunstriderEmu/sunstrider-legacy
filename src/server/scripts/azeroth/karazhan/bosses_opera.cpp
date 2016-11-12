/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Bosses_Opera
SD%Complete: 90
SDComment: Oz, Hood, and RAJ event implemented. RAJ event requires more testing.
SDCategory: Karazhan
EndScriptData */


#include "def_karazhan.h"

/***********************************/
/*** OPERA WIZARD OF OZ EVENT *****/
/*********************************/

#define SAY_DOROTHEE_DEATH          -1532025
#define SAY_DOROTHEE_SUMMON         -1532026
#define SAY_DOROTHEE_TITO_DEATH     -1532027
#define SAY_DOROTHEE_AGGRO          -1532028

#define SAY_ROAR_AGGRO              -1532029
#define SAY_ROAR_DEATH              -1532030
#define SAY_ROAR_SLAY               -1532031

#define SAY_STRAWMAN_AGGRO          -1532032
#define SAY_STRAWMAN_DEATH          -1532033
#define SAY_STRAWMAN_SLAY           -1532034

#define SAY_TINHEAD_AGGRO           -1532035
#define SAY_TINHEAD_DEATH           -1532036
#define SAY_TINHEAD_SLAY            -1532037
#define EMOTE_RUST                  -1532038

#define SAY_CRONE_AGGRO             -1532039
#define SAY_CRONE_AGGRO2            -1532040
#define SAY_CRONE_DEATH             -1532041
#define SAY_CRONE_SLAY              -1532042

/**** Spells ****/
// Dorothee
#define SPELL_WATERBOLT         31012
#define SPELL_SCREAM            31013
#define SPELL_SUMMONTITO        31014

// Tito
#define SPELL_YIPPING           31015

// Strawman
#define SPELL_BRAIN_BASH        31046
#define SPELL_BRAIN_WIPE        31069
#define SPELL_BURNING_STRAW     31075

// Tinhead
#define SPELL_CLEAVE            31043
#define SPELL_RUST              31086

// Roar
#define SPELL_MANGLE            31041
#define SPELL_SHRED             31042

// Crone
#define SPELL_CHAIN_LIGHTNING   32337

// Cyclone
#define SPELL_KNOCKBACK         32334
#define SPELL_CYCLONE_VISUAL    32332

/** Creature Entries **/
#define CREATURE_TITO           17548
#define CREATURE_CYCLONE        18412
#define CREATURE_CRONE          18168

void SummonCroneIfReady(InstanceScript* pInstance, Creature *_Creature)
{
    pInstance->SetData(DATA_OPERA_OZ_DEATHCOUNT, 0);        // Increment DeathCount
    if(pInstance->GetData(DATA_OPERA_OZ_DEATHCOUNT) == 4)
    {
        Creature* Crone = _Creature->SummonCreature(CREATURE_CRONE,  -10891.96, -1755.95, _Creature->GetPositionZ(), 4.64, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
        if(Crone)
        {
            if(_Creature->GetVictim())
                Crone->AI()->AttackStart(_Creature->GetVictim());
        }
    }
};

struct boss_dorotheeAI : public ScriptedAI
{
    boss_dorotheeAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript* pInstance;

    uint32 AggroTimer;

    uint32 WaterBoltTimer;
    uint32 FearTimer;
    uint32 SummonTitoTimer;

    bool SummonedTito;
    bool TitoDied;

    void Reset()
    override {
        AggroTimer = 500;

        WaterBoltTimer = 5000;
        FearTimer = 15000;
        SummonTitoTimer = 47500;

        SummonedTito = false;
        TitoDied = false;
    }

    void EnterCombat(Unit* who)
    override {
        DoScriptText(SAY_DOROTHEE_AGGRO, me);
    }

    void SummonTito();                                      // See below

    void JustDied(Unit* killer)
    override {
        DoScriptText(SAY_DOROTHEE_DEATH, me);

        if(pInstance)
            SummonCroneIfReady(pInstance, me);
    }

    void AttackStart(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    override {
        if(AggroTimer)
        {
            if(AggroTimer <= diff)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                AggroTimer = 0;
            }else AggroTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        if(WaterBoltTimer < diff)
        {
            DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_WATERBOLT);
            WaterBoltTimer = TitoDied ? 1500 : 5000;
        }else WaterBoltTimer -= diff;

        if(FearTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_SCREAM);
            FearTimer = 30000;
        }else FearTimer -= diff;

        if(!SummonedTito)
        {
            if(SummonTitoTimer < diff)
                SummonTito();
            else SummonTitoTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

struct mob_titoAI : public ScriptedAI
{
    mob_titoAI(Creature* c) : ScriptedAI(c) {}

    uint64 DorotheeGUID;

    uint32 YipTimer;

    void Reset()
    override {
        DorotheeGUID = 0;

        YipTimer = 10000;
    }

    void EnterCombat(Unit* who) override {}

    void JustDied(Unit* killer)
    override {
        if(DorotheeGUID)
        {
            Creature* Dorothee = (ObjectAccessor::GetCreature((*me), DorotheeGUID));
            if(Dorothee && Dorothee->IsAlive())
            {
                ((boss_dorotheeAI*)Dorothee->AI())->TitoDied = true;
                DoScriptText(SAY_DOROTHEE_TITO_DEATH, Dorothee);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim())
            return;

        if(YipTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_YIPPING);
            YipTimer = 10000;
        }else YipTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

void boss_dorotheeAI::SummonTito()
{
    Creature* Tito = DoSpawnCreature(CREATURE_TITO, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 45000);
    if(Tito)
    {
        DoScriptText(SAY_DOROTHEE_SUMMON, me);
        ((mob_titoAI*)Tito->AI())->DorotheeGUID = me->GetGUID();
        Tito->AI()->AttackStart(me->GetVictim());
        SummonedTito = true;
        TitoDied = false;
    }
}

struct boss_strawmanAI : public ScriptedAI
{
    boss_strawmanAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript* pInstance;

    uint32 AggroTimer;
    uint32 BrainBashTimer;
    uint32 BrainWipeTimer;

    void Reset()
    override {
        AggroTimer = 13000;
        BrainBashTimer = 5000;
        BrainWipeTimer = 7000;
    }

    void AttackStart(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterCombat(Unit* who)
    override {
        DoScriptText(SAY_STRAWMAN_AGGRO, me);
    }

    void SpellHit(Unit* caster, const SpellInfo *Spell)
    override {
        if((Spell->SchoolMask == SPELL_SCHOOL_MASK_FIRE) && (!(rand()%10)))
            DoCast(me, SPELL_BURNING_STRAW, true);
    }

    void JustDied(Unit* killer)
    override {
        DoScriptText(SAY_STRAWMAN_DEATH, me);

        if(pInstance)
            SummonCroneIfReady(pInstance, me);
    }

    void KilledUnit(Unit* victim)
    override {
        DoScriptText(SAY_STRAWMAN_SLAY, me);
    }

    void UpdateAI(const uint32 diff)
    override {
        if(AggroTimer)
        {
            if(AggroTimer <= diff)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                AggroTimer = 0;
            }else AggroTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        if(BrainBashTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_BRAIN_BASH);
            BrainBashTimer = 15000;
        }else BrainBashTimer -= diff;

        if(BrainWipeTimer < diff)
        {
            DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_BRAIN_WIPE);
            BrainWipeTimer = 20000;
        }else BrainWipeTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct boss_tinheadAI : public ScriptedAI
{
    boss_tinheadAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript* pInstance;

    uint32 AggroTimer;
    uint32 CleaveTimer;
    uint32 RustTimer;

    uint8 RustCount;

    void Reset()
    override {
        AggroTimer = 15000;
        CleaveTimer = 5000;
        RustTimer   = 30000;

        RustCount   = 0;
    }

    void EnterCombat(Unit* who)
    override {
        DoScriptText(SAY_TINHEAD_AGGRO, me);
    }

    void AttackStart(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* killer)
    override {
        DoScriptText(SAY_TINHEAD_DEATH, me);

        if(pInstance)
            SummonCroneIfReady(pInstance, me);
    }

    void KilledUnit(Unit* victim)
    override {
        DoScriptText(SAY_TINHEAD_SLAY, me);
    }

    void UpdateAI(const uint32 diff)
    override {
        if(AggroTimer)
        {
            if(AggroTimer < diff)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                AggroTimer = 0;
            }else AggroTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE);
            CleaveTimer = 5000;
        }else CleaveTimer -= diff;

        if(RustCount < 8)
        {
            if(RustTimer < diff)
            {
                RustCount++;
                 DoScriptText(EMOTE_RUST, me);
                DoCast(me, SPELL_RUST);
                RustTimer = 6000;
            }else RustTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

struct boss_roarAI : public ScriptedAI
{
    boss_roarAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript* pInstance;

    uint32 AggroTimer;
    uint32 MangleTimer;
    uint32 ShredTimer;

    void Reset()
    override {
        AggroTimer = 20000;
        MangleTimer = 5000;
        ShredTimer  = 10000;
    }

    void MoveInLineOfSight(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void AttackStart(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void EnterCombat(Unit* who)
    override {
        DoScriptText(SAY_ROAR_AGGRO, me);
    }

    void JustDied(Unit* killer)
    override {
        DoScriptText(SAY_ROAR_DEATH, me);

        if(pInstance)
            SummonCroneIfReady(pInstance, me);
    }

    void KilledUnit(Unit* victim)
    override {
        DoScriptText(SAY_ROAR_SLAY, me);
    }

    void UpdateAI(const uint32 diff)
    override {
        if(AggroTimer)
        {
            if(AggroTimer <= diff)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                AggroTimer = 0;
            }else AggroTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        if(MangleTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_MANGLE);
            MangleTimer = 5000 + rand()%3000;
        }else MangleTimer -= diff;

        if(ShredTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_SHRED);
            ShredTimer = 10000 + rand()%5000;
        }else ShredTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct boss_croneAI : public ScriptedAI
{
    boss_croneAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript* pInstance;

    uint32 CycloneTimer;
    uint32 ChainLightningTimer;

    void Reset()
    override {
        CycloneTimer = 30000;
        ChainLightningTimer = 10000;
    }

    void EnterCombat(Unit* who)
    override {
        switch(rand()%2)
        {
        case 0: DoScriptText(SAY_CRONE_AGGRO, me); break;
        case 1: DoScriptText(SAY_CRONE_AGGRO2, me); break;
        }
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
    }

    void JustDied(Unit* killer)
    override {
        DoScriptText(SAY_CRONE_DEATH, me);

        if(pInstance)
        {
            pInstance->SetData(DATA_OPERA_EVENT, DONE);
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim())
            return;

        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        if(CycloneTimer < diff)
        {
            Creature* Cyclone = DoSpawnCreature(CREATURE_CYCLONE, rand()%10, rand()%10, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
            if(Cyclone)
                Cyclone->CastSpell(Cyclone, SPELL_CYCLONE_VISUAL, true);
            CycloneTimer = 30000;
        }else CycloneTimer -= diff;

        if(ChainLightningTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_CHAIN_LIGHTNING);
            ChainLightningTimer = 15000;
        }else ChainLightningTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct mob_cycloneAI : public ScriptedAI
{
    mob_cycloneAI(Creature* c) : ScriptedAI(c) {}

    uint32 MoveTimer;

    void Reset()
    override {
        MoveTimer = 1000;
    }

    void EnterCombat(Unit* who) override {}

    void MoveInLineOfSight(Unit* who)
    override {
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!me->HasAuraEffect(SPELL_KNOCKBACK, 0))
            DoCast(me, SPELL_KNOCKBACK, true);

        if(MoveTimer < diff)
        {
            float PosX, PosY, PosZ;
            me->GetRandomPoint(me,10, PosX, PosY, PosZ);
            me->GetMotionMaster()->MovePoint(0, PosX, PosY, PosZ);
            MoveTimer = 5000 + rand()%3000;
        }else MoveTimer -= diff;
    }
};

CreatureAI* GetAI_boss_dorothee(Creature* _Creature)
{
    return new boss_dorotheeAI(_Creature);
}

CreatureAI* GetAI_boss_strawman(Creature* _Creature)
{
    return new boss_strawmanAI(_Creature);
}

CreatureAI* GetAI_boss_tinhead(Creature* _Creature)
{
    return new boss_tinheadAI(_Creature);
}

CreatureAI* GetAI_boss_roar(Creature* _Creature)
{
    return new boss_roarAI(_Creature);
}

CreatureAI* GetAI_boss_crone(Creature* _Creature)
{
    return new boss_croneAI(_Creature);
}

CreatureAI* GetAI_mob_tito(Creature* _Creature)
{
    return new mob_titoAI(_Creature);
}

CreatureAI* GetAI_mob_cyclone(Creature* _Creature)
{
    return new mob_cycloneAI(_Creature);
}

/**************************************/
/**** Opera Red Riding Hood Event ****/
/************************************/

/**** Yells for the Wolf ****/
#define SAY_WOLF_AGGRO                  -1532043
#define SAY_WOLF_SLAY                   -1532044
#define SAY_WOLF_HOOD                   -1532045
#define SOUND_WOLF_DEATH                9275                //Only sound on death, no text.

/**** Spells For The Wolf ****/
#define SPELL_LITTLE_RED_RIDING_HOOD    30768
#define SPELL_TERRIFYING_HOWL           30752
#define SPELL_WIDE_SWIPE                30761

#define GOSSIP_GRANDMA          "What phat lewtz you have grandmother?"

/**** The Wolf's Entry ****/
#define CREATURE_BIG_BAD_WOLF           17521

bool GossipHello_npc_grandmother(Player* player, Creature* _Creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_GRANDMA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    player->SEND_GOSSIP_MENU_TEXTID(8990, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_grandmother(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    if(action == GOSSIP_ACTION_INFO_DEF)
    {
        _Creature->SetVisibility(VISIBILITY_OFF);
        float x,y,z;
        _Creature->GetPosition(x,y,z);
        Creature* BigBadWolf = _Creature->SummonCreature(CREATURE_BIG_BAD_WOLF, x, y, z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
        if(BigBadWolf)
        {
            BigBadWolf->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            BigBadWolf->AI()->AttackStart(player);
        }

        _Creature->SetDeathState(JUST_DIED);
    }

    return true;
}

struct boss_bigbadwolfAI : public ScriptedAI
{
    boss_bigbadwolfAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript* pInstance;

    uint32 ChaseTimer;
    uint32 FearTimer;
    uint32 SwipeTimer;

    uint64 HoodGUID;
    float TempThreat;

    bool IsChasing;

    void Reset()
    override {
        ChaseTimer = 30000;
        FearTimer = 25000 + rand()%10000;
        SwipeTimer = 5000;

        HoodGUID = 0;
        TempThreat = 0;

        IsChasing = false;
        
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
    }

    void EnterCombat(Unit* who)
    override {
        DoScriptText(SAY_WOLF_AGGRO, me);
    }

    void JustDied(Unit* killer)
    override {
        DoPlaySoundToSet(me, SOUND_WOLF_DEATH);

        if(pInstance)
        {
            pInstance->SetData(DATA_OPERA_EVENT, DONE);
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();

        if(ChaseTimer < diff)
        {
            if(!IsChasing)
            {
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0);
                if(target && target->GetTypeId() == TYPEID_PLAYER)
                {
                    DoScriptText(SAY_WOLF_HOOD, me);

                    DoCast(target, SPELL_LITTLE_RED_RIDING_HOOD, true);
                    TempThreat = me->GetThreat(target);
                    if(TempThreat)
                        DoModifyThreatPercent(target, -100);
                    HoodGUID = target->GetGUID();
                    me->AddThreat(target, 1000000.0f);
                    ChaseTimer = 20000;
                    IsChasing = true;
                    me->SetFullTauntImmunity(true);
                }
            }
            else
            {
                IsChasing = false;
                me->SetFullTauntImmunity(false);
                Unit* target = ObjectAccessor::GetUnit((*me), HoodGUID);
                if(target)
                {
                    HoodGUID = 0;
                    if(me->GetThreat(target))
                        DoModifyThreatPercent(target, -100);
                    me->AddThreat(target, TempThreat);
                    TempThreat = 0;
                }

                ChaseTimer = 40000;
            }
        }else ChaseTimer -= diff;

        if(IsChasing)
            return;

        if(FearTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_TERRIFYING_HOWL);
            FearTimer = 25000 + rand()%10000;
        }else FearTimer -= diff;

        if(SwipeTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_WIDE_SWIPE);
            SwipeTimer = 25000 + rand()%5000;
        }else SwipeTimer -= diff;

    }
};

CreatureAI* GetAI_boss_bigbadwolf(Creature* _Creature)
{
    return new boss_bigbadwolfAI(_Creature);
}

/**********************************************/
/******** Opera Romeo and Juliet Event *******/
/********************************************/

/**** Speech *****/
#define SAY_JULIANNE_AGGRO              -1532046
#define SAY_JULIANNE_ENTER              -1532047
#define SAY_JULIANNE_DEATH01            -1532048
#define SAY_JULIANNE_DEATH02            -1532049
#define SAY_JULIANNE_RESURRECT          -1532050
#define SAY_JULIANNE_SLAY               -1532051

#define SAY_ROMULO_AGGRO                -1532052
#define SAY_ROMULO_DEATH                -1532053
#define SAY_ROMULO_ENTER                -1532054
#define SAY_ROMULO_RESURRECT            -1532055
#define SAY_ROMULO_SLAY                 -1532056

/***** Spells For Julianne *****/
#define SPELL_BLINDING_PASSION          30890
#define SPELL_DEVOTION                  30887
#define SPELL_ETERNAL_AFFECTION         30878
#define SPELL_POWERFUL_ATTRACTION       30889
#define SPELL_DRINK_POISON              30907

/***** Spells For Romulo ****/
#define SPELL_BACKWARD_LUNGE            30815
#define SPELL_DARING                    30841
#define SPELL_DEADLY_SWATHE             30817
#define SPELL_POISON_THRUST             30822

/**** Other Misc. Spells ****/
#define SPELL_UNDYING_LOVE              30951
#define SPELL_RES_VISUAL                24171

/*** Misc. Information ****/
#define CREATURE_ROMULO             17533
#define ROMULO_X                    -10900
#define ROMULO_Y                    -1758

enum RAJPhase
{
    PHASE_JULIANNE      = 0,
    PHASE_ROMULO        = 1,
    PHASE_BOTH          = 2,
};

void PretendToDie(Creature* _Creature)
{
    _Creature->InterruptNonMeleeSpells(false);
    _Creature->SetHealth(0);
    _Creature->ClearComboPointHolders();
    _Creature->RemoveAllAurasOnDeath();
    _Creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
    _Creature->ModifyAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, false);
    _Creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    _Creature->ClearAllReactives();
    _Creature->SetUInt64Value(UNIT_FIELD_TARGET,0);
    _Creature->GetMotionMaster()->Clear();
    _Creature->GetMotionMaster()->MoveIdle();
    _Creature->SetUInt32Value(UNIT_FIELD_BYTES_1,PLAYER_STATE_DEAD);
};

void Resurrect(Creature* target)
{
    target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    target->SetHealth(target->GetMaxHealth());
    target->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
    target->CastSpell(target, SPELL_RES_VISUAL, true);
    if(target->GetVictim())
    {
        target->SetUInt64Value(UNIT_FIELD_TARGET, target->GetVictim()->GetGUID());
        target->GetMotionMaster()->MoveChase(target->GetVictim());
        target->AI()->AttackStart(target->GetVictim());
    }
};

struct boss_julianneAI : public ScriptedAI
{
    boss_julianneAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
        EntryYellTimer = 1000;
        AggroYellTimer = 10000;
    }

    InstanceScript* pInstance;

    uint64 RomuloGUID;

    uint32 Phase;

    uint32 EntryYellTimer;
    uint32 AggroYellTimer;
    uint32 BlindingPassionTimer;
    uint32 DevotionTimer;
    uint32 EternalAffectionTimer;
    uint32 PowerfulAttractionTimer;
    uint32 SummonRomuloTimer;
    uint32 ResurrectTimer;

    bool IsFakingDeath;
    bool SummonedRomulo;
    bool RomuloDead;

    void Reset()
    override {
        if(RomuloGUID)
        {
            if(Unit* Romulo = ObjectAccessor::GetUnit(*me, RomuloGUID))
            {
                Romulo->SetVisibility(VISIBILITY_OFF);
                Romulo->DealDamage(Romulo, Romulo->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            }

            RomuloGUID = 0;
        }

        Phase = PHASE_JULIANNE;

        BlindingPassionTimer = 30000;
        DevotionTimer = 15000;
        EternalAffectionTimer = 25000;
        PowerfulAttractionTimer = 5000;

        if(IsFakingDeath)
            Resurrect(me);

        IsFakingDeath = false;
        SummonedRomulo = false;
        RomuloDead = false;
    }

    void EnterCombat(Unit* who) override {}

    void AttackStart(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void DamageTaken(Unit* done_by, uint32 &damage) override;

    void JustDied(Unit* killer)
    override {
        DoScriptText(SAY_JULIANNE_DEATH02, me);

        if(pInstance)
        {
            pInstance->SetData(DATA_OPERA_EVENT, DONE);
        }
    }

    void KilledUnit(Unit* victim)
    override {
       DoScriptText(SAY_JULIANNE_SLAY, me);
    }

    void UpdateAI(const uint32 diff) override;
};

struct boss_romuloAI : public ScriptedAI
{
    boss_romuloAI(Creature* c) : ScriptedAI(c)
    {
        EntryYellTimer = 8000;
        AggroYellTimer = 15000;
    }

    uint64 JulianneGUID;

    uint32 Phase;

    uint32 EntryYellTimer;
    uint32 AggroYellTimer;
    uint32 BackwardLungeTimer;
    uint32 DaringTimer;
    uint32 DeadlySwatheTimer;
    uint32 PoisonThrustTimer;
    uint32 ResurrectTimer;

    bool JulianneDead;
    bool IsFakingDeath;

    void Reset()
    override {
        JulianneGUID = 0;

        Phase = PHASE_ROMULO;

        BackwardLungeTimer = 15000;
        DaringTimer = 20000;
        DeadlySwatheTimer = 25000;
        PoisonThrustTimer = 10000;

        if(IsFakingDeath)
            Resurrect(me);

        IsFakingDeath = false;
        JulianneDead = false;
    }

    void DamageTaken(Unit* done_by, uint32 &damage) override;

    void EnterCombat(Unit* who)
    override {
        DoScriptText(SAY_ROMULO_AGGRO, me);
        if(JulianneGUID)
        {
            Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID));
            if(Julianne && Julianne->GetVictim())
            {
                me->AddThreat(Julianne->GetVictim(), 1.0f);
                AttackStart(Julianne->GetVictim());
            }
        }
    }

    void MoveInLineOfSight(Unit* who)
    override {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* killer)
    override {
        DoScriptText(SAY_ROMULO_DEATH, me);
    }

    void KilledUnit(Unit* victim)
    override {
        DoScriptText(SAY_ROMULO_SLAY, me);
    }

    void UpdateAI(const uint32 diff) override;
};

void KillLovers(Creature* creature, Creature* creatureLover, Unit* killer){
    
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    killer->Kill(creature, false);
    if(creatureLover)
    {
        creatureLover->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        killer->Kill(creatureLover, false);
    }
}

void boss_julianneAI::DamageTaken(Unit* done_by, uint32 &damage)
{
    if(damage < me->GetHealth() || done_by == me || done_by->GetGUID() == RomuloGUID)
        return;

    if(Phase == PHASE_JULIANNE)
    {
        DoScriptText(SAY_JULIANNE_DEATH01, me);
        me->InterruptNonMeleeSpells(true);
        DoCast(me, SPELL_DRINK_POISON);
        PretendToDie(me);
        Phase = PHASE_ROMULO;
        damage = 0;
        IsFakingDeath = true;
        SummonRomuloTimer = 10000;
        return;
    }

    if(!IsFakingDeath)
    {
        Creature* Romulo = (ObjectAccessor::GetCreature((*me), RomuloGUID));
        if(Romulo && Romulo->IsAlive() && !((boss_romuloAI*)Romulo->AI())->IsFakingDeath)
        {
            ((boss_romuloAI*)Romulo->AI())->ResurrectTimer = 10000;
            ((boss_romuloAI*)Romulo->AI())->JulianneDead = true;
        }
        else
        {
            if (Romulo)
                KillLovers(me, Romulo, done_by);
            return;
        }

        IsFakingDeath = true;
        PretendToDie(me);
        damage = 0;
    }
    else
        damage = 0;
}

void boss_romuloAI::DamageTaken(Unit* done_by, uint32 &damage)
{
    if(damage < me->GetHealth() || done_by == me || done_by->GetGUID() == JulianneGUID)
        return;

    if(!IsFakingDeath)
    {
        IsFakingDeath = true;
        PretendToDie(me);

        if(Phase == PHASE_BOTH)
        {
            Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID));
            if(Julianne && Julianne->IsAlive() && !((boss_julianneAI*)Julianne->AI())->IsFakingDeath)
            {
                ((boss_julianneAI*)Julianne->AI())->ResurrectTimer = 10000;
                ((boss_julianneAI*)Julianne->AI())->RomuloDead = true;
            }
            else
            {
                if (Julianne)
                    KillLovers(me, Julianne, done_by);
                return;
            }
        }
        else
        {
            Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID));
            if(Julianne)
            {
                Resurrect(Julianne);
                me->SetHealth(me->GetMaxHealth());
                ((boss_julianneAI*)Julianne->AI())->ResurrectTimer = 4000;
                ((boss_julianneAI*)Julianne->AI())->RomuloDead = true;
                ((boss_julianneAI*)Julianne->AI())->Phase = PHASE_BOTH;
                ((boss_julianneAI*)Julianne->AI())->IsFakingDeath = false;
            }
            Phase = PHASE_BOTH;
        }

        damage = 0;
    }

    if(IsFakingDeath) damage = 0;
}

void boss_julianneAI::UpdateAI(const uint32 diff)
{
    if(EntryYellTimer)
    {
        if(EntryYellTimer < diff)
        {
            DoScriptText(SAY_JULIANNE_ENTER, me);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            EntryYellTimer = 0;
        }else EntryYellTimer -= diff;
    }

    if(AggroYellTimer)
    {
        if(AggroYellTimer < diff)
        {
            DoScriptText(SAY_JULIANNE_AGGRO, me);
            AggroYellTimer = 0;
        }else AggroYellTimer -= diff;
    }

    if(Phase == PHASE_ROMULO && !SummonedRomulo)
    {
        if(SummonRomuloTimer < diff)
        {
            Creature* Romulo = me->SummonCreature(CREATURE_ROMULO, ROMULO_X, ROMULO_Y, me->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 45000);
            if(Romulo)
            {
                RomuloGUID = Romulo->GetGUID();
                ((boss_romuloAI*)Romulo->AI())->JulianneGUID = me->GetGUID();
                ((boss_romuloAI*)Romulo->AI())->Phase = PHASE_ROMULO;
                if(me->GetVictim())
                {
                    Romulo->AI()->AttackStart(me->GetVictim());
                    Romulo->AddThreat(me->GetVictim(), 50.0f);
                }
                DoZoneInCombat(Romulo);
            }
            SummonedRomulo = true;
        }else SummonRomuloTimer -= diff;
    }

    if(!UpdateVictim() ||IsFakingDeath)
        return;

    if(RomuloDead)
    {
        if(ResurrectTimer < diff)
        {
            Creature* Romulo = (ObjectAccessor::GetCreature((*me), RomuloGUID));
            if(Romulo && ((boss_romuloAI*)Romulo->AI())->IsFakingDeath)
            {
                DoScriptText(SAY_JULIANNE_RESURRECT, me);
                Resurrect(Romulo);
                ((boss_romuloAI*)Romulo->AI())->IsFakingDeath = false;
                ResurrectTimer = 10000;
            }
            RomuloDead = false;
        }else ResurrectTimer -= diff;
    }

    if(BlindingPassionTimer < diff)
    {
        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_BLINDING_PASSION);
        BlindingPassionTimer = 30000 + rand()%15000;
    }else BlindingPassionTimer -= diff;

    if(DevotionTimer < diff)
    {
        DoCast(me, SPELL_DEVOTION);
        DevotionTimer = 15000 + rand()%30000;
    }else DevotionTimer -= diff;

    if(PowerfulAttractionTimer < diff)
    {
        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_POWERFUL_ATTRACTION);
        PowerfulAttractionTimer = 5000 + rand()%25000;
    }else PowerfulAttractionTimer -= diff;

    if(EternalAffectionTimer < diff)
    {
        if(rand()%2 == 1 && SummonedRomulo)
        {
            Creature* Romulo = (ObjectAccessor::GetCreature((*me), RomuloGUID));
            if(Romulo && Romulo->IsAlive() && !((boss_romuloAI*)Romulo->AI())->IsFakingDeath)
                DoCast(Romulo, SPELL_ETERNAL_AFFECTION);
            else
                return;
        }else DoCast(me, SPELL_ETERNAL_AFFECTION);

        EternalAffectionTimer = 45000 + rand()%15000;
    }else EternalAffectionTimer -= diff;

    DoMeleeAttackIfReady();
}

void boss_romuloAI::UpdateAI(const uint32 diff)
{
    if(!UpdateVictim() || IsFakingDeath)
        return;

    if(JulianneDead)
    {
        if(ResurrectTimer < diff)
        {
            Creature* Julianne = (ObjectAccessor::GetCreature((*me), JulianneGUID));
            if(Julianne && ((boss_julianneAI*)Julianne->AI())->IsFakingDeath)
            {
                DoScriptText(SAY_ROMULO_RESURRECT, me);
                Resurrect(Julianne);
                ((boss_julianneAI*)Julianne->AI())->IsFakingDeath = false;
                ResurrectTimer = 10000;
            }
            JulianneDead = false;
        }else ResurrectTimer -= diff;
    }

    if(BackwardLungeTimer < diff)
    {
        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0, true, true);
        if(target && !me->HasInArc(M_PI, target))
        {
            DoCast(target, SPELL_BACKWARD_LUNGE);
            BackwardLungeTimer = 15000 + rand()%15000;
        }
    }else BackwardLungeTimer -= diff;

    if(DaringTimer < diff)
    {
        DoCast(me, SPELL_DARING);
        DaringTimer = 20000 + rand()%20000;
    }else DaringTimer -= diff;

    if(DeadlySwatheTimer < diff)
    {
        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_DEADLY_SWATHE);
        DeadlySwatheTimer = 15000 + rand()%10000;
    }else DeadlySwatheTimer -= diff;

    if(PoisonThrustTimer < diff)
    {
        DoCast(me->GetVictim(), SPELL_POISON_THRUST);
        PoisonThrustTimer = 10000 + rand()%10000;
    }else PoisonThrustTimer -= diff;

    DoMeleeAttackIfReady();
}

CreatureAI* GetAI_boss_julianne(Creature* _Creature)
{
    return new boss_julianneAI(_Creature);
}

CreatureAI* GetAI_boss_romulo(Creature* _Creature)
{
    return new boss_romuloAI(_Creature);
}

void AddSC_bosses_opera()
{
    OLDScript* newscript;

    // Oz
    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_dorothee;
    newscript->Name = "boss_dorothee";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_strawman;
    newscript->Name = "boss_strawman";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_tinhead;
    newscript->Name = "boss_tinhead";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_roar;
    newscript->Name = "boss_roar";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_crone;
    newscript->Name = "boss_crone";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_mob_tito;
    newscript->Name = "mob_tito";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_mob_cyclone;
    newscript->Name = "mob_cyclone";
    sScriptMgr->RegisterOLDScript(newscript);

    // Hood
    newscript = new OLDScript;
    newscript->OnGossipHello = &GossipHello_npc_grandmother;
    newscript->OnGossipSelect = &GossipSelect_npc_grandmother;
    newscript->Name = "npc_grandmother";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_bigbadwolf;
    newscript->Name = "boss_bigbadwolf";
    sScriptMgr->RegisterOLDScript(newscript);

    // Romeo And Juliet
    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_julianne;
    newscript->Name = "boss_julianne";
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_boss_romulo;
    newscript->Name = "boss_romulo";
    sScriptMgr->RegisterOLDScript(newscript);
}

