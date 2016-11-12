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
SDName: Boss_Netherspite
SD%Complete: 80
SDComment: some workaround
SDCategory: Karazhan
EndScriptData */


#include "def_karazhan.h"

#define EMOTE_PHASE_PORTAL          -1532089
#define EMOTE_PHASE_BANISH          -1532090

#define SPELL_NETHERBURN_AURA       30522
#define SPELL_VOIDZONE              37063
#define SPELL_NETHER_INFUSION       38688
#define SPELL_NETHERBREATH          38523
#define SPELL_BANISH_VISUAL         39833
#define SPELL_BANISH_ROOT           42716
#define SPELL_EMPOWERMENT           38549
#define SPELL_NETHERSPITE_ROAR      38684

const float PortalCoord[3][3] =
{
    {-11195.353516, -1613.237183, 278.237258}, // Left side
    {-11137.846680, -1685.607422, 278.239258}, // Right side
    {-11094.493164, -1591.969238, 279.949188}  // Back side
};

enum Netherspite_Portal{
    RED_PORTAL = 0, // Perseverence
    GREEN_PORTAL = 1, // Serenity
    BLUE_PORTAL = 2 // Dominance
};

const uint32 PortalID[3] = {17369, 17367, 17368};
const uint32 PortalVisual[3] = {30487,30490,30491};
const uint32 PortalBeam[3] = {30465,30464,30463};
const uint32 PlayerBuff[3] = {30421,30422,30423};
const uint32 NetherBuff[3] = {30466,30467,30468};
const uint32 PlayerDebuff[3] = {38637,38638,38639};

struct  boss_netherspiteAI : public ScriptedAI
{
    boss_netherspiteAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());

        for(int i=0; i<3; ++i)
        { 
            PortalGUID[i] = 0;
            BeamTarget[i] = 0;
            BeamerGUID[i] = 0;
        }
    }

    InstanceScript* pInstance;

    bool PortalPhase;
    bool Berserk;
    uint32 PhaseTimer; // timer for phase switching
    uint32 VoidZoneTimer;
    uint32 NetherInfusionTimer; // berserking timer
    uint32 NetherbreathTimer;
    uint32 EmpowermentTimer;
    uint32 PortalTimer; // timer for beam checking
    uint64 PortalGUID[3]; // guid's of portals
    uint64 BeamerGUID[3]; // guid's of auxiliary beaming portals
    uint64 BeamTarget[3]; // guid's of portals' current targets
    uint32 BuffTimer[3]; // independant buff timer for each portal

    bool IsBetween(WorldObject* boss, WorldObject* target, WorldObject* portal) // the in-line checker
    {
        if(!boss || !portal || !target)
            return false;

        float xboss, yboss, xportal, yportal, xtarget, ytarget;
        xboss = boss->GetPositionX();
        yboss = boss->GetPositionY();
        xportal = portal->GetPositionX();
        yportal = portal->GetPositionY();
        xtarget = target->GetPositionX();
        ytarget = target->GetPositionY();
        
        // check if target is between (not checking distance from the beam yet)
        if(    dist(xboss,yboss,xtarget,ytarget)>=dist(xboss,yboss,xportal,yportal)  //boss is further from target than from portal
            || dist(xportal,yportal,xtarget,ytarget)>=dist(xboss,yboss,xportal,yportal)) //portal is further from target than from boss
            return false;
        // check  distance from the beam
        return (std::fabs((xboss-xportal)*ytarget+(yportal-yboss)*xtarget-xboss*yportal+xportal*yboss)/dist(xboss,yboss,xportal,yportal) < 2.0f);
    }

    float dist(float xa, float ya, float xb, float yb) // auxiliary method for distance
    {
        return sqrt((xa-xb)*(xa-xb) + (ya-yb)*(ya-yb));
    }

    void Reset()
    override {
        Berserk = false;
        NetherInfusionTimer = 540000;
        VoidZoneTimer = 15000;
        NetherbreathTimer = 3000;
        
        for (uint32 & i : BuffTimer)
            i = 1000;

        HandleDoors(true);
        Destroyportalortals();
        
        me->RemoveAurasDueToSpell(SPELL_NETHERBURN_AURA);
    }

    void SummonPortals()
    {
        uint8 r = rand()%4;
        uint8 pos[3];
        pos[RED_PORTAL] = (r%2 ? (r>1 ? 2: 1): 0);
        pos[GREEN_PORTAL] = (r%2 ? 0: (r>1 ? 2: 1));
        pos[BLUE_PORTAL] = (r>1 ? 1: 2); // Blue Portal not on the left side (0)

        for(int i=0; i<3; ++i)
            if(Creature *portal = me->SummonCreature(PortalID[i],PortalCoord[pos[i]][0],PortalCoord[pos[i]][1],PortalCoord[pos[i]][2],0,TEMPSUMMON_TIMED_DESPAWN,60000))
            {
                PortalGUID[i] = portal->GetGUID();
                portal->AddAura(PortalVisual[i], portal);
                portal->SetReactState(REACT_PASSIVE);
                if(portal->AI()) portal->AI()->SetCombatMovementAllowed(false);
            }
    }

    void Destroyportalortals()
    {
        for(int i=0; i<3; ++i)
        {
            if(Creature *portal = ObjectAccessor::GetCreature(*me, PortalGUID[i]))
            {
                portal->SetVisibility(VISIBILITY_OFF);
                portal->DealDamage(portal, portal->GetMaxHealth());
                portal->RemoveFromWorld();
            }
            if(Creature *portal = ObjectAccessor::GetCreature(*me, BeamerGUID[i]))
            {
                portal->SetVisibility(VISIBILITY_OFF);
                portal->DealDamage(portal, portal->GetMaxHealth());
                portal->RemoveFromWorld();
            }
            PortalGUID[i] = 0;
            BeamTarget[i] = 0;
        }
    }

    void UpdatePortals(uint32 const diff) // Here we handle the beams' behavior
    {
        for(int j=0; j<3; ++j) // j = color
            if(Creature *portal = ObjectAccessor::GetCreature(*me, PortalGUID[j]))
            {
                // the one who's been casted upon before
                Unit *current = ObjectAccessor::GetUnit(*portal, BeamTarget[j]);
                // temporary store for the best suitable beam reciever
                Unit *target = me;

                if(Map* map = me->GetMap())
                {
                    Map::PlayerList const& players = map->GetPlayers();
                
                    // get the best suitable target
                    for(const auto & player : players)
                    {
                        Player* p = player.GetSource();
                        if(p && p->IsAlive() // alive
                            && !p->IsGameMaster()
                            && (!target || (target->GetExactDistance2d(portal)) > (p->GetExactDistance2d(portal)) ) // closer than current best
                            && !p->HasAuraEffect(PlayerDebuff[j],0) // not extargetausted
                            && !p->HasAuraEffect(PlayerBuff[(j+1)%3],0) // not on another beam
                            && !p->HasAuraEffect(PlayerBuff[(j+2)%3],0)
                            && IsBetween(me, p, portal)) // on the beam
                            target = p;
                    }
                }
                // buff the target
                if(target->GetTypeId() == TYPEID_PLAYER) {
                    if (BuffTimer[j] <= diff) {
                        target->AddAura(PlayerBuff[j], target);
                        BuffTimer[j] = 1000;
                    }
                }
                else if (BuffTimer[j] <= diff) {
                    target->AddAura(NetherBuff[j], target);
                    BuffTimer[j] = 1000;
                }
                // cast visual beam on the chosen target if switched
                // simple target switching isn't working -> using BeamerGUID to cast (workaround)
                if(!current || target != current)
                {
                    //if (current && current->GetTyportaleId() == TyportalEID_PLAYER)
                    //    current->AddAura(PlayerDebuff[j], current);
                    BeamTarget[j] = target->GetGUID();
                    // remove currently beaming portal
                    if(Creature *beamer = ObjectAccessor::GetCreature(*portal, BeamerGUID[j]))
                    {
                        beamer->CastSpell(target, PortalBeam[j], false);
                        beamer->SetVisibility(VISIBILITY_OFF);
                        beamer->DealDamage(beamer, beamer->GetMaxHealth());
                        beamer->RemoveFromWorld();
                        BeamerGUID[j] = 0;
                    }
                    // create new one and start beaming on the target
                    if(Creature *beamer = portal->SummonCreature(PortalID[j],portal->GetPositionX(),portal->GetPositionY(),portal->GetPositionZ(),portal->GetOrientation(),TEMPSUMMON_TIMED_DESPAWN,60000))
                    {
                        beamer->CastSpell(target, PortalBeam[j], false);
                        BeamerGUID[j] = beamer->GetGUID();
                    }
                }
                // aggro target if Red Beam
                if(j==RED_PORTAL && me->GetVictim() != target && target->GetTypeId() == TYPEID_PLAYER)
                    me->getThreatManager().addThreat(target, 100000.0f+me->GetThreat(me->GetVictim()));
            }
    }

    void SwitchToPortalPhase()
    {
        me->RemoveAurasDueToSpell(SPELL_BANISH_ROOT);
        me->RemoveAurasDueToSpell(SPELL_BANISH_VISUAL);
        SummonPortals();
        PhaseTimer = 60000;
        PortalPhase = true;
        PortalTimer = 8000;
        EmpowermentTimer = 10000;
        DoScriptText(EMOTE_PHASE_PORTAL,me);
    }

    void SwitchToBanishPhase()
    {
        me->RemoveAurasDueToSpell(SPELL_EMPOWERMENT);
        me->RemoveAurasDueToSpell(SPELL_NETHERBURN_AURA);
        DoCast(me,SPELL_BANISH_VISUAL,true);
        DoCast(me,SPELL_BANISH_ROOT,true);
        Destroyportalortals();
        PhaseTimer = 30000;
        PortalPhase = false;
        DoScriptText(EMOTE_PHASE_BANISH,me);

        for(uint32 i : NetherBuff)
            me->RemoveAurasDueToSpell(i);
    }

    void HandleDoors(bool open) // Massive Door switcher
    {
        if(GameObject *Door = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GAMEOBJECT_MASSIVE_DOOR)))
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
    }

    void EnterCombat(Unit *who)
    override {
        me->AddAura(SPELL_NETHERBURN_AURA, me);
        
        HandleDoors(false);
        SwitchToPortalPhase();
    }

    void JustDied(Unit* killer)
    override {
        HandleDoors(true);
        Destroyportalortals();
    }

    void UpdateAI(const uint32 diff)
    override {
        if(!UpdateVictim())
            return;
            
        for (uint32 & i : BuffTimer) {
            if (i >= diff)
                i -= diff;
        }
        
        float x, y, z, o;
        me->GetHomePosition(x, y, z, o);
        if (me->GetDistance2d(x, y) > 65.0f)
            EnterEvadeMode();

        // Void Zone
        if(VoidZoneTimer < diff)
        {
            DoCast(SelectTarget(SELECT_TARGET_RANDOM,1,45,true),SPELL_VOIDZONE,true);
            VoidZoneTimer = 15000;
        }else VoidZoneTimer -= diff;

        // NetherInfusion Berserk
        if(!Berserk && NetherInfusionTimer < diff)
        {
            me->AddAura(SPELL_NETHER_INFUSION, me);
            DoCast(me, SPELL_NETHERSPITE_ROAR);
            Berserk = true;
        }else NetherInfusionTimer -= diff;

        if(PortalPhase) // PORTAL PHASE
        {
            // Distribute beams and buffs
            if(PortalTimer < diff)
            {
                UpdatePortals(diff);
                PortalTimer = 200;
            }else PortalTimer -= diff;

            // Empowerment & Nether Burn
            if(EmpowermentTimer < diff)
            {
                DoCast(me, SPELL_EMPOWERMENT);
                me->AddAura(SPELL_NETHERBURN_AURA, me);
                EmpowermentTimer = 90000;
            }else EmpowermentTimer -= diff;
            
            if(PhaseTimer < diff)
            {
                if(!me->IsNonMeleeSpellCast(false))
                {
                    SwitchToBanishPhase();
                    return;
                }
            }else PhaseTimer -= diff;
        }
        else // BANISH PHASE
        {
            // Netherbreath
            if(NetherbreathTimer < diff)
            {
                if(Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0,40,true))
                    DoCast(target,SPELL_NETHERBREATH);
                NetherbreathTimer = 5000+rand()%2000;
            }else NetherbreathTimer -= diff;

            if(PhaseTimer < diff)
            {
                if(!me->IsNonMeleeSpellCast(false))
                {
                    SwitchToPortalPhase();
                    return;
                }
            }else PhaseTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_netherspite(Creature *_Creature)
{
    return new boss_netherspiteAI (_Creature);
}

void AddSC_boss_netherspite()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name="boss_netherspite";
    newscript->GetAI = &GetAI_boss_netherspite;
    sScriptMgr->RegisterOLDScript(newscript);
}

