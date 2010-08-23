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
SDName: Boss_Teron_Gorefiend
SD%Complete: 90
SDComment: don't know, maybe the blossoms can move up as on offi
SDCategory: Black Temple
EndScriptData */
/*

            
DELETE FROM `creature_template` WHERE (`entry`=23109);
INSERT INTO `creature_template` (`entry`, `heroic_entry`, `modelid_A`, `modelid_A2`, `modelid_H`, `modelid_H2`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `minhealth`, `maxhealth`, `minmana`, `maxmana`, `armor`, `faction_A`, `faction_H`, `npcflag`, `speed`, `scale`, `rank`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `baseattacktime`, `rangeattacktime`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `class`, `race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `type`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `PetSpellDataId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `RacialLeader`, `RegenHealth`, `equipment_id`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`) VALUES (23109, 0, 21300, 0, 21300, 0, 'Vengeful Spirit', '', '', 70, 70, 20000000, 20000000, 0, 0, 0, 35, 35, 0, 1, 1, 1, 0, 0, 0, 0, 1000000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40314, 40175, 40325, 40157, 0, 0, 0, '', 0, 1, 0, 1, 0, 8388624, 0, 'vengeful_spirit');

DELETE FROM spell_script_target WHERE ENTRY = 40268;
INSERT INTO spell_script_target VALUES (40268, 1, 23109);
           

UPDATE `creature_template` SET 
    `mindmg` = 2500, 
    `maxdmg` = 3000, 
    `attackpower` = ROUND((`mindmg` + `maxdmg`) / 4 * 7), 
    `mindmg` = ROUND(`mindmg` - `attackpower` / 7), 
    `maxdmg` = ROUND(`maxdmg` - `attackpower` / 7) 
  WHERE `entry` = 23111;

UPDATE `creature_template` SET `baseattacktime` = 2000 WHERE `entry` = 23111;
UPDATE `creature_template` SET `faction_A` = 1813, `faction_H` = 1813 WHERE `entry` = 23111;

UPDATE `creature_template` SET `speed` = 0.8 WHERE `entry` = 23111;
  */
#include "precompiled.h"
#include "def_black_temple.h"
#include "SpellMgr.h"
#include "Spell.h"
#include "WorldPacket.h"

 //Speech'n'sound
#define SAY_INTRO                       -1564037
#define SAY_AGGRO                       -1564038
#define SAY_SLAY1                       -1564039
#define SAY_SLAY2                       -1564040
#define SAY_SPELL1                      -1564041
#define SAY_SPELL2                      -1564042
#define SAY_SPECIAL1                    -1564043
#define SAY_SPECIAL2                    -1564044
#define SAY_ENRAGE                      -1564045
#define SAY_DEATH                       -1564046

//Spells
#define SPELL_INCINERATE                40239
#define SPELL_CRUSHING_SHADOWS          40243
#define SPELL_SHADOWBOLT                40185
#define SPELL_PASSIVE_SHADOWFORM        40326
#define SPELL_SHADOW_OF_DEATH           40251
#define SPELL_BERSERK                   45078
#define SPELL_SUMMON_SPIRIT             40266

#define SPELL_ATROPHY                   40327               // Shadowy Constructs use this when they get within melee range of a player

#define CREATURE_DOOM_BLOSSOM           23123
#define CREATURE_SHADOWY_CONSTRUCT      23111
#define CREATURE_GHOST                  23109

struct TRINITY_DLL_DECL mob_doom_blossomAI : public ScriptedAI
{
    mob_doom_blossomAI(Creature *c) : ScriptedAI(c) {}

    uint32 MoveTimer;
    uint32 ShadowBoltTimer;

    void Reset()
    {
        MoveTimer = 1000;
        ShadowBoltTimer = 1000 + rand()%500;

        m_creature->GetMotionMaster()->Clear(false);
        m_creature->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ() + 7);
    }

    void Aggro(Unit *who) { }
    void AttackStart(Unit* who) { }
    void MoveInLineOfSight(Unit* who) { }

    void UpdateAI(const uint32 diff)
    {
        if (ShadowBoltTimer <= diff)
        {
            DoCast(SelectUnit(0, 100, true, false, true, 0, 0), SPELL_SHADOWBOLT, true);
            ShadowBoltTimer = 1000 + rand()%500;
        }else ShadowBoltTimer -= diff;
        
        if (MoveTimer <= diff) {
            if (Creature *pTeron = m_creature->FindCreatureInGrid(22871, 80.0f, true)) {
                m_creature->GetMotionMaster()->MovePoint(0, pTeron->GetPositionX(), pTeron->GetPositionY(), m_creature->GetPositionZ());
                MoveTimer = 4000;
            }
        } else MoveTimer -= diff;
    }
    
    void DamageDeal(Unit* target, uint32 &damage)
    {
        if (target->GetDisplayId() == 21300)
            damage = 0;
    }

};

struct TRINITY_DLL_DECL boss_teron_gorefiendAI : public ScriptedAI
{
    boss_teron_gorefiendAI(Creature *c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    uint32 IncinerateTimer;
    uint32 SummonDoomBlossomTimer;
    uint32 EnrageTimer;
    uint32 CrushingShadowsTimer;
    uint32 ShadowOfDeathTimer;
    uint32 RandomYellTimer;
    uint32 AggroTimer;
    

    uint64 AggroTargetGUID;

    SummonList Summons;

    bool Intro;

    void Reset()
    {
        if(pInstance){
            if (pInstance->GetData(DATA_TERONGOREFIENDEVENT) != DONE)
                pInstance->SetData(DATA_TERONGOREFIENDEVENT, NOT_STARTED);
        }

        IncinerateTimer = 20000 + rand()%10000;
        SummonDoomBlossomTimer = 24000;
        EnrageTimer = 600000;
        CrushingShadowsTimer = 22000;
        ShadowOfDeathTimer = 10000;
        RandomYellTimer = 50000;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        // Start off unattackable so that the intro is done properly
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    

        m_creature->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        m_creature->ApplySpellImmune(0, IMMUNITY_EFFECT,SPELL_EFFECT_ATTACK_ME, true);

        
        m_creature->ApplySpellImmune(0, IMMUNITY_ID, 40314, true);
        m_creature->ApplySpellImmune(0, IMMUNITY_ID, 40175, true);
        m_creature->ApplySpellImmune(0, IMMUNITY_ID, 40157, true);

        AggroTimer = 20000;

        Intro = false;
        DespawnConstructs();
        Summons.DespawnAll();
    }

    void Aggro(Unit *who) {}

    void MoveInLineOfSight(Unit *who)
    {
        if(!who || (!who->isAlive())) return;

        if(who->isTargetableForAttack() && who->isInAccessiblePlaceFor(m_creature) && m_creature->IsHostileTo(who))
        {
            float attackRadius = m_creature->GetAttackDistance(who);

            if (m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE && m_creature->IsWithinLOSInMap(who))
            {
                m_creature->AddThreat(who, 1.0f);
                DoZoneInCombat();
            }

            if(!InCombat && !Intro && m_creature->IsWithinDistInMap(who, 60.0f) && (who->GetTypeId() == TYPEID_PLAYER))
            {
                if(pInstance)
                    pInstance->SetData(DATA_TERONGOREFIENDEVENT, IN_PROGRESS);

                m_creature->GetMotionMaster()->Clear(false);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DoScriptText(SAY_INTRO, m_creature);
                m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                AggroTargetGUID = who->GetGUID();
                Intro = true;
            }
        }
    }
    
    void JustSummoned(Creature* summon)
    {
        Summons.Summon(summon);
    }
    
    Unit* GetConstructTarget() {
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true))
            return target;
        else
            return NULL;
    }
    
    bool HasPlayerTarget()
    {
        Map::PlayerList const &PlayerList = m_creature->GetMap()->GetPlayers();
        if (PlayerList.isEmpty())
            return false;
        for(std::list<HostilReference*>::iterator itr = m_creature->getThreatManager().getThreatList().begin(); itr != m_creature->getThreatManager().getThreatList().end(); ++itr)
        {
            for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* i_pl = i->getSource())
                    if(i_pl->GetGUID() == (*itr)->getUnitGuid())
                        return true;
            }
        }
        return false;
    }

    void KilledUnit(Unit *victim)
    {
        switch(rand()%2)
        {
            case 0: DoScriptText(SAY_SLAY1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY2, m_creature); break;
        }
    }
    
    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (done_by->GetDisplayId() == 21300) {
            DoModifyThreatPercent(done_by,-100);
            damage = 0;
            if (done_by->GetTypeId() == TYPEID_PLAYER) {
                WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
                data << uint32(0);                                      // spellId
                data << uint8(SPELL_FAILED_IMMUNE);                     // reason
                done_by->ToPlayer()->GetSession()->SendPacket(&data);               // send message: Invalid target
            }
        }
    }
    
    void DamageDeal(Unit* target, uint32 &damage)
    {
        if (target->GetDisplayId() == 21300)
            damage = 0;
    }

    void JustDied(Unit *victim)
    {
        if(pInstance)
            pInstance->SetData(DATA_TERONGOREFIENDEVENT, DONE);

        DoScriptText(SAY_DEATH, m_creature);
        DespawnConstructs();
        Summons.DespawnAll();
    }

    void SetThreatList(Creature* unit)
    {
        if(!unit) return;

        std::list<HostilReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
        std::list<HostilReference*>::iterator i = m_threatlist.begin();
        for(i = m_threatlist.begin(); i != m_threatlist.end(); i++)
        {
            Unit* pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid());
            if(pUnit && pUnit->isAlive())
            {
                float threat = DoGetThreat(pUnit);
                unit->AddThreat(pUnit, threat + 5000000.0f);
            }
        }
    }
    
    float CalculateRandomLocation(float Loc, uint32 radius)
    {
        float coord = Loc;
        switch(rand()%2)
        {
            case 0:
                coord += rand()%radius;
                break;
            case 1:
                coord -= rand()%radius;
                break;
        }
        return coord;
    }

    void UpdateAI(const uint32 diff)
    {
        if(Intro)
        {
            if(AggroTimer < diff)
            {
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                DoScriptText(SAY_AGGRO, m_creature);
                m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
                Intro = false;
                if(AggroTargetGUID)
                {
                    Unit* pUnit = Unit::GetUnit((*m_creature), AggroTargetGUID);
                    if(pUnit)
                        AttackStart(pUnit);

                    DoZoneInCombat();
                }
                else
                {
                    EnterEvadeMode();
                    return;
                }
            }else AggroTimer -= diff;
        }

        if(!UpdateVictim() || Intro)
            return;

        // Ignore NPCs
        if(!HasPlayerTarget())
        {
            EnterEvadeMode();
            return;
        }

        if(ShadowOfDeathTimer < diff)
        {
            if (Unit* pShadowVictim = SelectUnit(1, 100, true, true, true, SPELL_SHADOW_OF_DEATH, 1)){
                m_creature->InterruptNonMeleeSpells(false);
                DoCast(pShadowVictim, SPELL_SHADOW_OF_DEATH, true);
            }
            ShadowOfDeathTimer = 30000;
        }else ShadowOfDeathTimer -= diff;

        if(SummonDoomBlossomTimer < diff)
        {
            Creature* DoomBlossom = m_creature->SummonCreature(CREATURE_DOOM_BLOSSOM, CalculateRandomLocation(m_creature->GetPositionX(), 8), CalculateRandomLocation(m_creature->GetPositionY(), 8), m_creature->GetPositionZ() + 7, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            if(DoomBlossom)
            {
                DoomBlossom->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DoomBlossom->setFaction(m_creature->getFaction());
                SetThreatList(DoomBlossom);
                SummonDoomBlossomTimer = 30000;
            }
        }else SummonDoomBlossomTimer -= diff;

        if(IncinerateTimer < diff)
        {
            Unit* target = SelectUnit(1, 100, true, false, true, 0, 0);
            if(!target)
                target = m_creature->getVictim();

            if(target)
            {
                switch(rand()%2)
                {
                case 0: DoScriptText(SAY_SPECIAL1, m_creature); break;
                case 1: DoScriptText(SAY_SPECIAL2, m_creature); break;
                }
                DoCast(target, SPELL_INCINERATE);
                IncinerateTimer = 15000 + rand()%10000;
            }
        }else IncinerateTimer -= diff;

        if(CrushingShadowsTimer < diff)
        {
            DoCast(m_creature,SPELL_CRUSHING_SHADOWS, true);
            CrushingShadowsTimer = 10000 + rand()%16 * 1000;
        }else CrushingShadowsTimer -= diff;


        if(RandomYellTimer < diff)
        {
            switch(rand()%2)
            {
            case 0: DoScriptText(SAY_SPELL1, m_creature); break;
            case 1: DoScriptText(SAY_SPELL2, m_creature); break;
            }
            RandomYellTimer = 50000 + rand()%51 * 1000;
        }else RandomYellTimer -= diff;

        if(!m_creature->HasAura(SPELL_BERSERK, 0))
        {
            if(EnrageTimer < diff)
            {
                DoCast(m_creature, SPELL_BERSERK);
                DoScriptText(SAY_ENRAGE, m_creature);
            }else EnrageTimer -= diff;
        }
        DoMeleeAttackIfReady();
    }

    void DespawnConstructs()
    {
        CellPair pair(Trinity::ComputeCellPair(m_creature->GetPositionX(), m_creature->GetPositionY()));
        Cell cell(pair);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();
        std::list<Creature*> ConstructList;

        Trinity::AllCreaturesOfEntryInRange check(m_creature, CREATURE_SHADOWY_CONSTRUCT, 100);
        Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(ConstructList, check);
        TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

        CellLock<GridReadGuard> cell_lock(cell, pair);
        cell_lock->Visit(cell_lock, visitor, *(m_creature->GetMap()));

        if(!ConstructList.empty())
        {
            for(std::list<Creature*>::iterator itr = ConstructList.begin(); itr != ConstructList.end(); ++itr)
            {
                (*itr)->setDeathState(JUST_DIED);
                (*itr)->RemoveCorpse();
            }
        }
    }
};

struct TRINITY_DLL_DECL mob_shadowy_constructAI : public ScriptedAI
{
    mob_shadowy_constructAI(Creature* c) : ScriptedAI(c) 
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }
    
    ScriptedInstance* pInstance;

    uint32 AtrophyTimer;
    uint32 AttackTimer;
    uint64 TeronGUID;

    bool SetAggro;

    void Reset()
    {
        m_creature->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        m_creature->ApplySpellImmune(0, IMMUNITY_EFFECT,SPELL_EFFECT_ATTACK_ME, true);

        // immune to frost trap aura
        m_creature->ApplySpellImmune(0, IMMUNITY_ID, 3600, true);

        AtrophyTimer = 2000;
        SetAggro = false;
        AttackTimer = 3000;
        if (pInstance)
            TeronGUID = pInstance->GetData64(DATA_TERON);
            
        if (Creature *pTeron = m_creature->FindCreatureInGrid(22871, 80.0f, true)) {
            if (Unit *pTarget = ((boss_teron_gorefiendAI*)pTeron->AI())->GetConstructTarget())
                m_creature->AI()->AttackStart(pTarget);
        }
        
        m_creature->setActive(true);
    }

    void Aggro(Unit* who) {}

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (done_by->GetDisplayId() != 21300) {
            damage = 0;
            if (done_by->GetTypeId() == TYPEID_PLAYER) {
                WorldPacket data(SMSG_CAST_FAILED, (4+2));              // prepare packet error message
                data << uint32(0);                                      // spellId
                data << uint8(SPELL_FAILED_IMMUNE);                     // reason
                done_by->ToPlayer()->GetSession()->SendPacket(&data);               // send message: Invalid target
            }
        }
        else
            DoModifyThreatPercent(done_by,-100);
    }

    void SpellHit(Unit *caster, SpellEntry const* spellInfo)
    {
        if (caster->GetDisplayId() != 21300)
            m_creature->RemoveAurasByCasterSpell(spellInfo->Id,caster->GetGUID());
    }

    void UpdateAI(const uint32 diff)
    {
        float x,y,z;
        m_creature->GetPosition(x,y,z);
        z = m_creature->GetMap()->GetVmapHeight(x, y, z, true);
        m_creature->Relocate(x,y,z,0);
        if(AttackTimer < diff)
        {
            SetAggro = true;
            AttackTimer = 99999999;

        }else AttackTimer -= diff;

        if (SetAggro){
            if (Creature* Teron = (Unit::GetCreature((*m_creature), TeronGUID)))
                SetThreatList(Teron);
                
            m_creature->AI()->AttackStart(SelectUnit(0, 100, true, true, true, SPELL_SHADOW_OF_DEATH, 1));
            DoCast(m_creature, SPELL_PASSIVE_SHADOWFORM, true);

            SetAggro = false;
        }

        if(AtrophyTimer < diff)
        {
            if (Unit *playerInMelee = m_creature->SelectNearestTarget(5)){
                if (!playerInMelee->isPossessed() && !playerInMelee->isPossessing())
                    DoCast(playerInMelee, SPELL_ATROPHY);
            }
            AtrophyTimer = 2000;

        }else AtrophyTimer -= diff;

        DoMeleeAttackIfReady();
    }

    void DoMeleeAttackIfReady()
    {
        if(m_creature->isAttackReady())
        {
            if(m_creature->IsWithinMeleeRange(m_creature->getVictim()))
            {
                if (m_creature->canMelee() && (!m_creature->getVictim()->isPossessed() && !m_creature->getVictim()->isPossessing()))
                {
                    m_creature->AttackerStateUpdate(m_creature->getVictim());
                    m_creature->resetAttackTimer();
                } else{
                    m_creature->resetAttackTimer();
                }
            }
        }
    }
    
    void SetThreatList(Creature* unit)
    {
        if(!unit) return;

        std::list<HostilReference*>& m_threatlist = unit->getThreatManager().getThreatList();
        std::list<HostilReference*>::iterator i = m_threatlist.begin();
        for(i = m_threatlist.begin(); i != m_threatlist.end(); i++)
        {
            Unit* pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid());
            if(pUnit && pUnit->isAlive())
            {
                if ( pUnit->GetDisplayId() != 21300 && !pUnit->HasAura(40282,0)){
                    float threat = unit->getThreatManager().getThreat(pUnit);
                    m_creature->AddThreat(pUnit, threat + 5000000.0f);
                }
            }
        }
    }
    
    void DamageDeal(Unit* target, uint32 &damage)
    {
        if (target->GetDisplayId() == 21300)
            damage = 0;
    }
 };

CreatureAI* GetAI_mob_doom_blossom(Creature *_Creature)
{
    return new mob_doom_blossomAI(_Creature);
}

CreatureAI* GetAI_mob_shadowy_construct(Creature *_Creature)
{
    return new mob_shadowy_constructAI(_Creature);
}

CreatureAI* GetAI_boss_teron_gorefiend(Creature *_Creature)
{
    return new boss_teron_gorefiendAI (_Creature);
}

void AddSC_boss_teron_gorefiend()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "mob_doom_blossom";
    newscript->GetAI = &GetAI_mob_doom_blossom;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowy_construct";
    newscript->GetAI = &GetAI_mob_shadowy_construct;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_teron_gorefiend";
    newscript->GetAI = &GetAI_boss_teron_gorefiend;
    newscript->RegisterSelf();
}
