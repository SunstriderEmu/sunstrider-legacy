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
SDName: AV Marshals
SD%Complete:
SDComment: Returns AV marshals to inside the building
SDCategory: Bosses
EndScriptData */



#define SPELL_CHARGE               22911
#define SPELL_CLEAVE               40504
#define SPELL_DEMORALIZING_SHOUT   23511
#define SPELL_ENRAGE               8599
#define SPELL_WHIRLWIND1           15589
#define SPELL_WHIRLWIND2           13736

struct  AV_MarshalsAI : public ScriptedAI
{
     AV_MarshalsAI(Creature *c) : ScriptedAI(c) {Reset();}
     
     uint32 ChargeTimer;
     uint32 CleaveTimer;
     uint32 DemoralizingShoutTimer;
     uint32 Whirlwind1Timer;
     uint32 Whirlwind2Timer;
     uint32 EnrageTimer;
     uint32 ResetTimer;
     
     void Reset()
     override {
        ChargeTimer             = (2+rand()%10)*IN_MILLISECONDS;
        CleaveTimer                = (1+rand()%10)*IN_MILLISECONDS;
        DemoralizingShoutTimer  = (2+rand()%18)*IN_MILLISECONDS;
        Whirlwind1Timer            = (1+rand()%12)*IN_MILLISECONDS;
        Whirlwind2Timer            = (5+rand()%15)*IN_MILLISECONDS;
        EnrageTimer                = (5+rand()%20)*IN_MILLISECONDS;
        ResetTimer                = 5000;
     }
     
     void JustRespawned()
     override {
         Reset();
     }
     
     void KilledUnit(Unit* victim)override {}
     
     void JustDied(Unit* Killer)override {}
     
     void UpdateAI(const uint32 diff)
     override {
        if (!UpdateVictim())
            return;

        if (ChargeTimer <diff)
        {
            if(DoCast(me->GetVictim(), SPELL_CHARGE) == SPELL_CAST_OK)
                ChargeTimer = (10+rand()%15)*IN_MILLISECONDS;
        }else ChargeTimer -= diff;            
        
        if (CleaveTimer < diff)
        {
            if(DoCast(me->GetVictim(), SPELL_CLEAVE) == SPELL_CAST_OK)
                CleaveTimer =  (10+rand()%6)*IN_MILLISECONDS;
        }else CleaveTimer -= diff;

        if (DemoralizingShoutTimer < diff)
        {
            if(DoCast(me->GetVictim(), SPELL_DEMORALIZING_SHOUT) == SPELL_CAST_OK)
                DemoralizingShoutTimer = (10+rand()%5)*IN_MILLISECONDS;
        }else DemoralizingShoutTimer -= diff;

        if (Whirlwind1Timer < diff)
        {
            if(DoCast(me->GetVictim(), SPELL_WHIRLWIND1) == SPELL_CAST_OK)
                Whirlwind1Timer = (6+rand()%14)*IN_MILLISECONDS;
        }else Whirlwind1Timer -= diff;

        if (Whirlwind2Timer < diff)
        {
            if(DoCast(me->GetVictim(), SPELL_WHIRLWIND2) == SPELL_CAST_OK)
                Whirlwind2Timer = (10+rand()%15)*IN_MILLISECONDS;
        }else Whirlwind2Timer -= diff;

        if (EnrageTimer < diff)
        {
            if(DoCast(me->GetVictim(), SPELL_ENRAGE) == SPELL_CAST_OK)
                EnrageTimer = (10+rand()%20)*IN_MILLISECONDS;
        }else EnrageTimer -= diff;    
            

        // check if creature is not outside of building
        /*if(ResetTimer < diff)
        {
            float x, y, z;
            me->GetPosition(x, y, z);
            if(x < 700)
            {
                EnterEvadeMode();
            }
            ResetTimer = 2000;
        }else ResetTimer -= diff;*/

        DoMeleeAttackIfReady();
     }
};

CreatureAI* GetAI_AV_Marshals(Creature *_Creature)
{
    return new AV_MarshalsAI (_Creature);
}

void AddSC_AV_Marshals()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name = "AV_Marshals";
    newscript->GetAI = &GetAI_AV_Marshals;
    sScriptMgr->RegisterOLDScript(newscript);
}
