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
SDName: AV Warmasters
SD%Complete:
SDComment: Returns AV Warmasters to building
SDCategory: Bosses
EndScriptData */



#define SPELL_CHARGE               22911
#define SPELL_CLEAVE               40504
#define SPELL_DEMORALIZING_SHOUT   23511
#define SPELL_ENRAGE               8599
#define SPELL_WHIRLWIND1           15589
#define SPELL_WHIRLWIND2           13736

#define MAX_HOME_DISTANCE          40.0f

struct  AV_WarmastersAI : public ScriptedAI
{
     AV_WarmastersAI(Creature *c) : ScriptedAI(c) {Reset();}
     
     uint32 ChargeTimer;
     uint32 CleaveTimer;
     uint32 DemoralizingShoutTimer;
     uint32 Whirlwind1Timer;
     uint32 Whirlwind2Timer;
     uint32 EnrageTimer;
     uint32 DistanceCheckTimer;
     
     void Reset()
     override {
        ChargeTimer             = (2+rand()%10)*IN_MILLISECONDS;
        CleaveTimer                = (1+rand()%10)*IN_MILLISECONDS;
        DemoralizingShoutTimer  = (2+rand()%18)*IN_MILLISECONDS;
        Whirlwind1Timer            = (1+rand()%12)*IN_MILLISECONDS;
        Whirlwind2Timer            = (5+rand()%15)*IN_MILLISECONDS;
        EnrageTimer                = (5+rand()%20)*IN_MILLISECONDS;
        DistanceCheckTimer      = 5000;
     }

     void JustRespawned() override
     {
         Reset();
     }
     
     void KilledUnit(Unit* victim) override {}
     
     void JustDied(Unit* Killer) override {}
     
     void UpdateAI(const uint32 diff) override
     {
        if (!UpdateVictim())
            return;

        if (ChargeTimer <diff)
        {
            DoCast(me->GetVictim(), SPELL_CHARGE);
            ChargeTimer = (10+rand()%15)*IN_MILLISECONDS;
        } else ChargeTimer -= diff;            
        
        if (CleaveTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE);
            CleaveTimer =  (10+rand()%6)*IN_MILLISECONDS;
        } else CleaveTimer -= diff;

        if (DemoralizingShoutTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_DEMORALIZING_SHOUT);
            DemoralizingShoutTimer = (10+rand()%5)*IN_MILLISECONDS;
        } else DemoralizingShoutTimer -= diff;

        if (Whirlwind1Timer < diff)
        {
            DoCast(me->GetVictim(), SPELL_WHIRLWIND1);
            Whirlwind1Timer = (6+rand()%14)*IN_MILLISECONDS;
        } else Whirlwind1Timer -= diff;

        if (Whirlwind2Timer < diff)
        {
            DoCast(me->GetVictim(), SPELL_WHIRLWIND2);
            Whirlwind2Timer = (10+rand()%15)*IN_MILLISECONDS;
        } else Whirlwind2Timer -= diff;

        if (EnrageTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_ENRAGE);
            EnrageTimer = (10+rand()%20)*IN_MILLISECONDS;
        } else EnrageTimer -= diff;    

        // check if creature is not outside of building
        if(DistanceCheckTimer < diff)
        {
            if(me->GetDistanceFromHome() > MAX_HOME_DISTANCE)
            {
                //evade all creatures from pool
                EnterEvadeMode();
                std::list<Creature*> poolCreatures = me->GetMap()->GetAllCreaturesFromPool(me->GetCreaturePoolId());
                for(auto itr : poolCreatures)
                    if(itr->AI()) itr->AI()->EnterEvadeMode();
            }
            DistanceCheckTimer = 2000;
        } else DistanceCheckTimer -= diff;

        DoMeleeAttackIfReady();
     }
};

CreatureAI* GetAI_AV_Warmasters(Creature *_Creature)
{
    return new AV_WarmastersAI (_Creature);
}

void AddSC_AV_Warmasters()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name = "AV_Warmasters";
    newscript->GetAI = &GetAI_AV_Warmasters;
    sScriptMgr->RegisterOLDScript(newscript);
}
