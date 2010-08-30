/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: boss_mechanolord_capacitus
SD%Complete: 90
SDComment:
SDCategory: Tempest Keep, The Mechanar
EndScriptData */

#include "precompiled.h"
#include "def_mechanar.h"

enum eCapacitusSays {
    SAY_AGGRO               = -1554028,
    SAY_PLAYERKILL_1        = -1554029,
    SAY_PLAYERKILL_2        = -1554030,
    SAY_DEATH               = -1554031,
    SAY_FIRST_SHIELD        = -1554032,
    SAY_SECOND_SHIELD       = -1554033
};

enum eCapacitusSpells {
    SPELL_HEADCRACK         = 35161,
    SPELL_FIRST_SHIELD      = 35159,
    SPELL_SECOND_SHIELD     = 35158,
    SPELL_POSITIVE_CHARGE   = 39088,
    SPELL_POSCHARGE_INCDMG  = 39089,    // Increase damage done by 100%
    SPELL_POSCHARGE_DAMAGE  = 39090,    // Deals 2k damage
    SPELL_NEGATIVE_CHARGE   = 39091,
    SPELL_NEGCHARGE_INCDMG  = 39092,    // Increase damage done by 100%
    SPELL_NEGCHARGE_DAMAGE  = 39093,    // Deals 2k damage
    SPELL_ENRAGE            = 27680     // Taken from Void Reaver
};

#define NPC_NETHER_CHARGE   20405

struct boss_mechanolord_capacitusAI : public ScriptedAI
{
    boss_mechanolord_capacitusAI(Creature *c) : ScriptedAI(c), summons(m_creature)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }
    
    uint32 HeadCrackTimer;
    uint32 FirstShieldTimer;
    uint32 SecondShieldTimer;
    uint32 PolarityTimer;
    uint32 NetherChargeTimer;
    uint32 HeroicEnrageTimer;
    uint32 CheckChargesTimer;       // Check near players' charges, and apply +% mod dmg or 2k nature damage depending on aura
    
    ScriptedInstance* pInstance;
    
    SummonList summons;
    
    bool HeroicMode;
    
    void Reset()
    {                               // Values are taken from existing (old) ACID script
        HeadCrackTimer = 10000;     // 10 sec, and then every 13 sec
        FirstShieldTimer = 15000;   // 15 sec, and then every 40 sec
        SecondShieldTimer = 30000;  // 30 sec, and then every 40 sec
        PolarityTimer = 5000;       // 5 sec, and then every 45-60 sec -> GUESSED
        NetherChargeTimer = 0;      // every 10 sec, starting when aggro
        HeroicEnrageTimer = 180000; // Enrage after 3 minutes (heroic only)
        CheckChargesTimer = 10000;
        
        if (pInstance && pInstance->GetData(DATA_MECHLORD_CAPACITUS) != DONE)
            pInstance->SetData(DATA_MECHLORD_CAPACITUS, NOT_STARTED);
            
        // Remove all charges related auras
        RemoveAurasDueToCharges();
        
        summons.DespawnAll();
    }
    
    void JustSummoned(Creature* pSummon)
    {
        summons.Summon(pSummon);
    }
    
    void SummonedCreatureDespawn(Creature* pSummon)
    {
        summons.Despawn(pSummon);
        
        if (summons.IsEmpty() && pInstance->GetData(DATA_MECHLORD_CAPACITUS) == DONE) {
            // Put all players on map out of combat (maybe dangerous in a few cases)
            Map *map = m_creature->GetMap();
            if (!map->IsDungeon())
                return;
                
            Map::PlayerList const& players = map->GetPlayers();        // Get a list with all the players
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr) {
                if (Player* plr = itr->getSource())
                    plr->CombatStop();
            }
        }
    }
    
    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        pInstance->SetData(DATA_MECHLORD_CAPACITUS, IN_PROGRESS);
    }
    
    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(RAND(SAY_PLAYERKILL_1, SAY_PLAYERKILL_2), m_creature);
    }
    
    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);
        
        if (pInstance)
            pInstance->SetData(DATA_MECHLORD_CAPACITUS, DONE);
            
        // Remove all charges related auras
        RemoveAurasDueToCharges();
    }
    
    void SpawnNetherCharge()
    {
        float posX, posY, posZ;
        m_creature->GetRandomPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 30, posX, posY, posZ);
        
        m_creature->SummonCreature(NPC_NETHER_CHARGE, posX, posY, posZ, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
    }
    
    void SetPolarityOnPlayers()
    {
        Unit* target = NULL;
        std::list<HostilReference *> t_list = m_creature->getThreatManager().getThreatList();
        for(std::list<HostilReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
        {
            target = Unit::GetUnit(*m_creature, (*itr)->getUnitGuid());
            if (target && target->GetTypeId() == TYPEID_PLAYER && target->isAlive())
            {
                // Remove charges related auras first
                if (target->HasAura(SPELL_POSITIVE_CHARGE))
                    target->RemoveAurasDueToSpell(SPELL_POSITIVE_CHARGE);
                else if (target->HasAura(SPELL_NEGATIVE_CHARGE))
                    target->RemoveAurasDueToSpell(SPELL_NEGATIVE_CHARGE);
                    
                if (target->HasAura(SPELL_POSCHARGE_INCDMG))
                    target->RemoveAurasDueToSpell(SPELL_POSCHARGE_INCDMG);
                else if (target->HasAura(SPELL_NEGCHARGE_INCDMG))
                    target->RemoveAurasDueToSpell(SPELL_NEGCHARGE_INCDMG);
                    
                target->CastSpell(target, RAND(SPELL_POSITIVE_CHARGE, SPELL_NEGATIVE_CHARGE), true);     // Random : positive or negative
            }
        }
    }
    
    void RemoveAurasDueToCharges()
    {
        Map *map = m_creature->GetMap();
        if (!map->IsDungeon())
            return;
            
        Map::PlayerList const& players = map->GetPlayers();        // Get a list with all the players
        
        for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            if (Player* target = itr->getSource()) {
                if (target->HasAura(SPELL_POSITIVE_CHARGE))
                    target->RemoveAurasDueToSpell(SPELL_POSITIVE_CHARGE);
                else if (target->HasAura(SPELL_NEGATIVE_CHARGE))
                    target->RemoveAurasDueToSpell(SPELL_NEGATIVE_CHARGE);
                    
                if (target->HasAura(SPELL_POSCHARGE_INCDMG))
                    target->RemoveAurasDueToSpell(SPELL_POSCHARGE_INCDMG);
                else if (target->HasAura(SPELL_NEGCHARGE_INCDMG))
                    target->RemoveAurasDueToSpell(SPELL_NEGCHARGE_INCDMG);
            }
        }
    }
    
    void DoChargeEffects()      // Deal 2000 damage for _EACH_ group mate <10 meters that has opposite charge, or add a+100% dmg aura _ONCE_ if there's a group mate with same charge <10 meters
    {
        Map *map = m_creature->GetMap();
        if (!map->IsDungeon())
            return;
        
        Map::PlayerList const& players = map->GetPlayers();        // Get a list with all the players
        
        /** How we will proceed:
          * First, for the damage : for each group member, check each group mate, if less than 10 meters and has opposite charge, deal 2k damage
          * Second, for the +100% dmg : for each group member, check each group mate, if less than 10 meters and has same charge AND DOES NOT HAVE +100% DMG AURA, add +100% dmg aura
          */
        bool deserveModDmg = false;     // Used to remove the +100% dmg if no group mate with same aura found within 10 meters
        if (!players.isEmpty()) {
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr) {
                if (Player* plr = itr->getSource()) {
                    if (Group* pGroup = plr->GetGroup()) {
                        for(GroupReference* groupItr = pGroup->GetFirstMember(); groupItr != NULL; groupItr = groupItr->next()) {
                            Player* pGroupGuy = groupItr->getSource();
                            
                            if (pGroupGuy && pGroupGuy->GetGUID() != plr->GetGUID() && pGroupGuy->GetDistance(plr) <= 10) {   // Check distance
                                if (plr->HasAura(SPELL_POSITIVE_CHARGE) && pGroupGuy->HasAura(SPELL_POSITIVE_CHARGE)) {         // Same charge, positive
                                    deserveModDmg = true;
                                    if (!pGroupGuy->HasAura(SPELL_POSCHARGE_INCDMG))        // Only apply if players hasn't it already (don't need to have a player with +500% damage lol)
                                        plr->AddAura(SPELL_POSCHARGE_INCDMG, pGroupGuy);
                                }
                                else if (plr->HasAura(SPELL_NEGATIVE_CHARGE) && pGroupGuy->HasAura(SPELL_NEGATIVE_CHARGE)) {    // Same charge, negative
                                    deserveModDmg = true;
                                    if (!pGroupGuy->HasAura(SPELL_NEGCHARGE_INCDMG))        // Only apply if players hasn't it already
                                        plr->AddAura(SPELL_NEGCHARGE_INCDMG, pGroupGuy);
                                }
                                
                                if (plr->HasAura(SPELL_POSITIVE_CHARGE) && pGroupGuy->HasAura(SPELL_NEGATIVE_CHARGE)) {         // Different charges, plr has +
                                    plr->CastSpell(plr, SPELL_POSCHARGE_DAMAGE, true);
                                }
                                else if (plr->HasAura(SPELL_NEGATIVE_CHARGE) && pGroupGuy->HasAura(SPELL_POSITIVE_CHARGE)) {    // Different charges, plr has -
                                    plr->CastSpell(plr, SPELL_NEGCHARGE_DAMAGE, true);
                                }
                            }
                            
                            if (pGroupGuy && !deserveModDmg) {      // You don't deserve +100% damage aura !
                                if (pGroupGuy->HasAura(SPELL_NEGCHARGE_INCDMG))
                                    pGroupGuy->RemoveAurasDueToSpell(SPELL_NEGCHARGE_INCDMG);
                                else if (pGroupGuy->HasAura(SPELL_POSCHARGE_INCDMG))
                                    pGroupGuy->RemoveAurasDueToSpell(SPELL_POSCHARGE_INCDMG);
                            }
                        }
                    }
                }
            }
        }
        
        return;
    }
    
    void UpdateAI(const uint32 diff)
    {
        // Return since we have no target
        if (!UpdateVictim())
            return;
            
        if (HeadCrackTimer <= diff) {
            if (m_creature->getVictim())
                DoCast(m_creature->getVictim(), SPELL_HEADCRACK);
                
            HeadCrackTimer = 13000;
        } else HeadCrackTimer -= diff;
        
        if (NetherChargeTimer <= diff) {
            SpawnNetherCharge();
            
            NetherChargeTimer = 10000;
        } else NetherChargeTimer -= diff;
        
        if (HeroicMode)     // Heroic: no shields, but polarity shift and enrage
        {
            if (HeroicEnrageTimer <= diff) {
                DoCast(m_creature, SPELL_ENRAGE);
            } else HeroicEnrageTimer -= diff;
            
            if (PolarityTimer <= diff) {
                SetPolarityOnPlayers();
                
                PolarityTimer = 44000 + rand()%16000;   // 45-60 sec
            } else PolarityTimer -= diff;
            
            if (CheckChargesTimer <= diff) {
                DoChargeEffects();
                
                CheckChargesTimer = 5000;
            } else CheckChargesTimer -= diff;
        } else {            // Normal: shields, but no polarity shift
            if (FirstShieldTimer <= diff) {
                if (m_creature->getVictim())
                    DoCast(m_creature->getVictim(), SPELL_FIRST_SHIELD);
                
                DoScriptText(SAY_FIRST_SHIELD, m_creature);
                FirstShieldTimer = 40000;
            } else FirstShieldTimer -= diff;
            
            if (SecondShieldTimer <= diff) {
                if (m_creature->getVictim())
                    DoCast(m_creature->getVictim(), SPELL_SECOND_SHIELD);
                    
                DoScriptText(SAY_SECOND_SHIELD, m_creature);
                SecondShieldTimer = 40000;
            } else SecondShieldTimer -= diff;
        }
        
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_mechanolord_capacitus(Creature* pCreature)
{
    return new boss_mechanolord_capacitusAI(pCreature);
}

struct npc_netherchargeAI : public Scripted_NoMovementAI
{
    npc_netherchargeAI(Creature *c) : Scripted_NoMovementAI(c) {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }
    
    uint32 pulseTimer;
    uint32 suicideTimer;
    
    ScriptedInstance *pInstance;
    
    void Reset() {
        DoCast(m_creature, 37670);
        pulseTimer = 14000;
        suicideTimer = 18000;
    }
    
    void JustDied(Unit *pKiller) {
        if (pInstance) {
            if (Creature *pMechanolord = pKiller->FindCreatureInGrid(19219, 40.0, false))
                pInstance->SetData(DATA_NETHERCHARGE, DONE);
        }
    }
    
    void Aggro(Unit *pWho) {}
    
    void UpdateAI(uint32 const diff) {
        if (pulseTimer <= diff) {
            DoCast(m_creature, 35151);
            pulseTimer = 1000;
        } else pulseTimer -= diff;
        
        if (suicideTimer <= diff)
            m_creature->Kill(m_creature);
        else
            suicideTimer -= diff;
        
    }
};

CreatureAI* GetAI_npc_nethercharge(Creature *pCreature)
{
    return new npc_netherchargeAI(pCreature);
}

void AddSC_boss_mechanolord_capacitus()
{
    Script* newscript;
    
    newscript = new Script;
    newscript->Name="boss_mechanolord_capacitus";
    newscript->GetAI = &GetAI_boss_mechanolord_capacitus;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_nethercharge";
    newscript->GetAI = &GetAI_npc_nethercharge;
    newscript->RegisterSelf();
}
