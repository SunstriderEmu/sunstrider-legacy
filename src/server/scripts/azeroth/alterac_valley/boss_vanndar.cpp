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
SDName: Boss_Vanndar
SD%Complete: 
SDComment: Some spells listed on wowwiki but doesn't exist on wowhead
EndScriptData */

#define YELL_AGGRO              -2100008

#define YELL_EVADE              -2100009
#define YELL_RESPAWN1            -2100010
#define YELL_RESPAWN2           -2100011

#define YELL_RANDOM1            -2100012
#define YELL_RANDOM2            -2100013
#define YELL_RANDOM3            -2100014
#define YELL_RANDOM4            -2100015
#define YELL_RANDOM5            -2100016
#define YELL_RANDOM6            -2100017
#define YELL_RANDOM7            -2100018


#define SPELL_AVATAR            19135
#define SPELL_THUNDERCLAP        15588
#define SPELL_STORMBOLT         20685 // not sure

#define MAX_HOME_DISTANCE       40.0f

struct  boss_vanndarAI : public ScriptedAI
{
    boss_vanndarAI(Creature *c) : ScriptedAI(c) {}

    uint32 AvatarTimer;
    uint32 ThunderclapTimer;
    uint32 StormboltTimer;
    uint32 DistanceCheckTimer;
    uint32 YellTimer;
    uint32 YellEvadeCooldown;

    void Reset() override {
        AvatarTimer            = 3000;
        ThunderclapTimer    = 4000;
        StormboltTimer      = 6000;
        DistanceCheckTimer  = 5000;
        YellTimer           = (20+rand()%10)*IN_MILLISECONDS; //20 to 30 seconds
        YellEvadeCooldown   = 0;
        
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
    }

    void EnterCombat(Unit *who) override
    {
        DoScriptText(YELL_AGGRO, me);
    }

    void JustRespawned() override
    {
        Reset();
        switch (rand()%1) 
        {
            case 0: DoScriptText(YELL_RESPAWN1, me); break;
            case 1: DoScriptText(YELL_RESPAWN2, me); break;
        }
    }

    void KilledUnit(Unit* victim) override {}

    void JustDied(Unit* Killer) override {}

    void UpdateAI(const uint32 diff) override {
        if(!UpdateVictim())
            return;
            
        if (AvatarTimer <= diff) {
            DoCast(me->GetVictim(), SPELL_AVATAR);
            AvatarTimer =  (15+rand()%5)*IN_MILLISECONDS;
        } else AvatarTimer -= diff;

        if (ThunderclapTimer <= diff) {
            DoCast(me->GetVictim(), SPELL_THUNDERCLAP);
            ThunderclapTimer = (5+rand()%10)*IN_MILLISECONDS;
        } else ThunderclapTimer -= diff;

        if (StormboltTimer <= diff)
        {
            DoCast(me->GetVictim(), SPELL_STORMBOLT);
            StormboltTimer = (10+rand()%15)*IN_MILLISECONDS;
        } else StormboltTimer -= diff;

        if (YellTimer <= diff) {
            switch (rand()%6) {
            case 0: DoScriptText(YELL_RANDOM1, me); break;
            case 1: DoScriptText(YELL_RANDOM2, me); break;
            case 2: DoScriptText(YELL_RANDOM3, me); break;
            case 3: DoScriptText(YELL_RANDOM4, me); break;
            case 4: DoScriptText(YELL_RANDOM5, me); break;
            case 5: DoScriptText(YELL_RANDOM6, me); break;
            case 6: DoScriptText(YELL_RANDOM7, me); break;
            }
            YellTimer = (20+rand()%10)*IN_MILLISECONDS; //20 to 30 seconds
        } else YellTimer -= diff;

        // check if creature is not outside of building
        if(DistanceCheckTimer < diff)
        {
            if(me->GetDistanceFromHome() > MAX_HOME_DISTANCE)
            {
                //evade all creatures from pool
                EnterEvadeMode();
                if(!YellEvadeCooldown)
                {
                    DoScriptText(YELL_EVADE, me);
                    YellEvadeCooldown = 5000;
                }
                std::list<Creature*> poolCreatures = me->GetMap()->GetAllCreaturesFromPool(me->GetCreaturePoolId());
                for(auto itr : poolCreatures)
                    if(itr->AI()) itr->AI()->EnterEvadeMode();
            }
            DistanceCheckTimer = 2000;
        }else DistanceCheckTimer -= diff;

        if(YellEvadeCooldown && YellEvadeCooldown >= diff)
            YellEvadeCooldown -= diff;

        DoMeleeAttackIfReady();
    }
};

/*
Les 3 premiers point formes une ligne partant de la droite vers la gauche (quand l'on regarde le fortin) et délimite la limite a franchir pour reset.

1/ x: 679.48 y: -29.64 z: 50.62
2/ x: 678.96 y: -23.44 z: 50.62
3/ x: 677.22 y: -17.72 z: 50.62

Les 3 points suivant sont les points respectif pour 1/ 2/ et 3/ quand l'on avance de 1m à partir de chacun des points (donc quand le boss doit déjà avoir reset)

1/ x: 675 y: -30 z:50.62
2/ x: 673.04 y: -25 z: 50.62
3/ x: 672.9 y: -18.8 z: 50.62
*/

CreatureAI* GetAI_boss_vanndar(Creature *pCreature)
{
    return new boss_vanndarAI (pCreature);
}

void AddSC_boss_vanndar()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name = "boss_vanndar";
    newscript->GetAI = &GetAI_boss_vanndar;
    sScriptMgr->RegisterOLDScript(newscript);
}
