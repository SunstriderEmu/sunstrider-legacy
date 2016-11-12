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
SDName: Boss_Galvangar
SD%Complete: 
SDComment: timers should be adjusted
EndScriptData */

#define YELL_AGGRO                 -2100021
#define YELL_EVADE                 -2100022

#define SPELL_CLEAVE               15284
#define SPELL_FRIGHTENING_SHOUT    19134
#define SPELL_WHIRLWIND1           15589
#define SPELL_WHIRLWIND2           13736
#define SPELL_MORTAL_STRIKE        16856


struct  boss_galvangarAI : public ScriptedAI
{
    boss_galvangarAI(Creature *c) : ScriptedAI(c) {}


    uint32 CleaveTimer;
    uint32 FrighteningShoutTimer;
    uint32 Whirlwind1Timer;
    uint32 Whirlwind2Timer;
    uint32 MortalStrikeTimer;
    uint32 ResetTimer;


    void Reset()
    override {
        CleaveTimer                = (1+rand()%10)*IN_MILLISECONDS;
        FrighteningShoutTimer    = (2+rand()%18)*IN_MILLISECONDS;
        Whirlwind1Timer            = (1+rand()%12)*IN_MILLISECONDS;
        Whirlwind2Timer            = (5+rand()%15)*IN_MILLISECONDS;
        MortalStrikeTimer        = (5+rand()%20)*IN_MILLISECONDS;
        ResetTimer                = 5000;
        
        me->ApplySpellImmune(0, IMMUNITY_ID, 5760, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 5761, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 8692, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 8693, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 8694, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 11398, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 11399, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 11400, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 1714, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 11719, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 31589, true);
    }

    void EnterCombat(Unit *who)
    override {
        DoScriptText(YELL_AGGRO, me);
    }

    void JustRespawned()
    override {
        Reset();
    }

    void KilledUnit(Unit* victim)override {}

    void JustDied(Unit* Killer)override {}

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim())
            return;

        if (CleaveTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE);
            CleaveTimer =  (10+rand()%6)*IN_MILLISECONDS;
        }else CleaveTimer -= diff;

        if (FrighteningShoutTimer < diff)
        {
            TC_LOG_DEBUG("misc","Alterac Valley: Galvangar: Casting SPELL_FRIGHTENING_SHOUT");
            DoCast(me, SPELL_FRIGHTENING_SHOUT);
            FrighteningShoutTimer = (10+rand()%5)*IN_MILLISECONDS;
        }else FrighteningShoutTimer -= diff;

        if (Whirlwind1Timer < diff)
        {
            DoCast(me->GetVictim(), SPELL_WHIRLWIND1);
            Whirlwind1Timer = (6+rand()%14)*IN_MILLISECONDS;
        }else Whirlwind1Timer -= diff;

        if (Whirlwind2Timer < diff)
        {
            DoCast(me->GetVictim(), SPELL_WHIRLWIND2);
            Whirlwind2Timer = (10+rand()%15)*IN_MILLISECONDS;
        }else Whirlwind2Timer -= diff;

        if (MortalStrikeTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_MORTAL_STRIKE);
            MortalStrikeTimer = (10+rand()%20)*IN_MILLISECONDS;
        }else MortalStrikeTimer -= diff;

        // check if creature is not outside of building
        if(ResetTimer < diff)
        {
            float x, y, z;
            me->GetPosition(x, y, z);
            if(x > -504)
        {
                DoScriptText(YELL_EVADE, me);
                    EnterEvadeMode();
        }
            ResetTimer = 2000;
        }else ResetTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_galvangar(Creature *_Creature)
{
    return new boss_galvangarAI (_Creature);
}

void AddSC_boss_galvangar()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name = "boss_galvangar";
    newscript->GetAI = &GetAI_boss_galvangar;
    sScriptMgr->RegisterOLDScript(newscript);
}
