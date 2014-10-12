/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
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

#ifndef TRINITY_CREATUREAI_H
#define TRINITY_CREATUREAI_H

#include "Common.h"
#include "Define.h"
#include "Policies/Singleton.h"
#include "Dynamic/ObjectRegistry.h"
#include "Dynamic/FactoryHolder.h"
//#include "GameObjectAI.h"

class Unit;
class Creature;
class Player;
class Quest;
class GameObject;
class GameObjectAI;
struct SpellEntry;

#define TIME_INTERVAL_LOOK   5000
#define VISIBILITY_RANGE    10000

//Spell targets used by SelectSpell
enum SelectTarget
{
    SELECT_TARGET_DONTCARE = 0,                             //All target types allowed

    SELECT_TARGET_SELF,                                     //Only Self casting

    SELECT_TARGET_SINGLE_ENEMY,                             //Only Single Enemy
    SELECT_TARGET_AOE_ENEMY,                                //Only AoE Enemy
    SELECT_TARGET_ANY_ENEMY,                                //AoE or Single Enemy

    SELECT_TARGET_SINGLE_FRIEND,                            //Only Single Friend
    SELECT_TARGET_AOE_FRIEND,                               //Only AoE Friend
    SELECT_TARGET_ANY_FRIEND,                               //AoE or Single Friend
};

//Spell Effects used by SelectSpell
enum SelectEffect
{
    SELECT_EFFECT_DONTCARE = 0,                             //All spell effects allowed
    SELECT_EFFECT_DAMAGE,                                   //Spell does damage
    SELECT_EFFECT_HEALING,                                  //Spell does healing
    SELECT_EFFECT_AURA,                                     //Spell applies an aura
};

enum SCEquip
{
    EQUIP_NO_CHANGE = -1,
    EQUIP_UNEQUIP   = 0
};

//Selection method used by SelectTarget
enum SelectAggroTarget
{
    SELECT_TARGET_RANDOM = 0,                               //Just selects a random target
    SELECT_TARGET_TOPAGGRO,                                 //Selects targes from top aggro to bottom
    SELECT_TARGET_BOTTOMAGGRO,                              //Selects targets from bottom aggro to top
    SELECT_TARGET_NEAREST,
    SELECT_TARGET_FARTHEST,
};

class UnitAI
{
    protected:
        Unit *me;
        //combat movement part not yet implemented. Creatures with m_combatDistance and target distance > 5.0f wont show melee weapons.
        float m_combatDistance;         
        bool m_allowCombatMovement;
        bool m_restoreCombatMovementOnOOM;
    public:
        UnitAI(Unit *u) : me(u), m_combatDistance(0.5f), m_allowCombatMovement(true), m_restoreCombatMovementOnOOM(false) {}

        virtual void AttackStartIfCan(Unit *);
        virtual void AttackStart(Unit *);
        virtual void UpdateAI(const uint32 diff) = 0;

        float GetCombatDistance() { return m_combatDistance; };
        void SetCombatDistance(float dist);

        bool IsCombatMovementAllowed() { return m_allowCombatMovement; };
        void SetCombatMovementAllowed(bool allow);
        void SetRestoreCombatMovementOnOOM(bool set);
        bool GetRestoreCombatMovementOnOOM();

        virtual void InitializeAI() { Reset(); }

        virtual void Reset() {};

        // Called when unit is charmed
        virtual void OnCharmed(Unit* charmer, bool apply) = 0;
        virtual void OnPossess(Unit* charmer, bool apply) = 0;

        // Pass parameters between AI
        virtual void DoAction(const int32 param) {}
        virtual uint32 GetData(uint32 /*id = 0*/) { return 0; }
        virtual void SetData(uint32 /*id*/, uint32 /*value*/) {}
        
        virtual void AttackedBy(Unit* who) {}

        //Do melee swing of current victim if in rnage and ready and not casting
        void DoMeleeAttackIfReady();
        bool DoSpellAttackIfReady(uint32 spell);
        
        virtual void sGossipHello(Player* player) {}
        virtual void sGossipSelect(Player* player, uint32 sender, uint32 action) {}
        virtual void sGossipSelectCode(Player* player, uint32 sender, uint32 action, const char* code) {}
        virtual void sQuestAccept(Player* player, Quest const* quest) {}
        virtual void sQuestSelect(Player* player, Quest const* quest) {}
        virtual void sQuestComplete(Player* player, Quest const* quest) {}
        virtual void sQuestReward(Player* player, Quest const* quest, uint32 opt) {}
};

class PlayerAI : public UnitAI
{
    protected:
        Player *me;
    public:
        PlayerAI(Player *p) : UnitAI((Unit*)p), me(p) {}

        void OnCharmed(Unit* charmer, bool apply);
        void OnPossess(Unit* charmer, bool apply);
};

class SimpleCharmedAI : public PlayerAI
{
    public:
        void UpdateAI(const uint32 diff);
};

class CreatureAI : public UnitAI
{
    protected:
        Creature *me;

        bool UpdateVictim(bool evade = true);
    public:
        CreatureAI(Creature *c) : UnitAI((Unit*)c), me(c) {}

        virtual ~CreatureAI() {}

        void Talk(uint8 id, uint64 targetGuid = 0);

        // Called if IsVisible(Unit *who) is true at each *who move, AND if creature is aggressive
        virtual void MoveInLineOfSight(Unit *);
        
        //Same as MoveInLineOfSight but with is called with every react state (so not only if the creature is aggressive)
        virtual void MoveInLineOfSight2(Unit *) {}

        // Called at stopping attack by any attacker
        virtual void EnterEvadeMode();

        // Called at any Damage from any attacker (before damage apply)
        virtual void DamageTaken(Unit *done_by, uint32 & /*damage*/) {}
        
        // Called when the creature receives heal
        virtual void HealReceived(Unit* /*done_by*/, uint32& /*addhealth*/) {}

        // Called when the creature is killed
        virtual void JustDied(Unit *) {}

        // Called when the creature kills a unit
        virtual void KilledUnit(Unit *) {}

        // Called when the creature summon successfully other creature
        virtual void JustSummoned(Creature* ) {}
        virtual void IsSummonedBy(Unit* /*summoner*/) { }

        virtual void SummonedCreatureDespawn(Creature* /*unit*/) {}

        // Called when hit by a spell
        virtual void SpellHit(Unit*, const SpellEntry*) {}

        // Called when spell hits a target
        virtual void SpellHitTarget(Unit* target, const SpellEntry*) {}

        // Called when the creature is target of hostile action: swing, hostile spell landed, fear/etc)
        virtual void AttackedBy(Unit* /*attacker*/) { }
        virtual bool IsEscorted() { return false; }

        // Called when vitim entered water and creature can not enter water
        virtual bool canReachByRangeAttack(Unit*) { return false; }

        // Called when creature is spawned or respawned (for reseting variables)
        virtual void JustRespawned() {}

        // Called at waypoint reached or point movement finished
        virtual void MovementInform(uint32 /*MovementType*/, uint32 /*Data*/) {}
        
        // Called when creature finishes a spell cast
        virtual void OnSpellFinish(Unit *caster, uint32 spellId, Unit *target, bool ok) {}
        
        // Called when creature reaches its home position
        virtual void JustReachedHome() {}

        // Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
        virtual void EnterCombat(Unit* enemy) {}
        
        virtual void ReceiveEmote(Player* /*player*/, uint32 /*text_emote*/) {}
        
        virtual void DespawnDueToGameEventEnd(uint32 /*eventId*/) {}

        // called when the corpse of this creature gets removed
        virtual void CorpseRemoved(uint32& respawnDelay) {}

        void OnCharmed(Unit* charmer, bool apply);
        void OnPossess(Unit* charmer, bool apply);
        
        // Called when creature's master (pet case) killed a unit
        virtual void MasterKilledUnit(Unit* unit) {}
        
        virtual bool sOnDummyEffect(Unit* /*caster*/, uint32 /*spellId*/, uint32 /*effIndex*/) { return false; }

        virtual void OnRemove() {}
        
        /* Script interaction */
        virtual uint64 message(uint32 id, uint64 data) { return 0; }
};

struct SelectableAI : public FactoryHolder<CreatureAI>, public Permissible<Creature>
{

    SelectableAI(const char *id) : FactoryHolder<CreatureAI>(id) {}
};

template<class REAL_AI>
struct CreatureAIFactory : public SelectableAI
{
    CreatureAIFactory(const char *name) : SelectableAI(name) {}

    CreatureAI* Create(void *) const;

    int Permit(const Creature *c) const { return REAL_AI::Permissible(c); }
};

enum Permitions
{
    PERMIT_BASE_NO                 = -1,
    PERMIT_BASE_IDLE               = 1,
    PERMIT_BASE_REACTIVE           = 100,
    PERMIT_BASE_PROACTIVE          = 200,
    PERMIT_BASE_FACTION_SPECIFIC   = 400,
    PERMIT_BASE_SPECIAL            = 800
};

typedef FactoryHolder<CreatureAI> CreatureAICreator;
typedef FactoryHolder<CreatureAI>::FactoryHolderRegistry CreatureAIRegistry;
typedef FactoryHolder<CreatureAI>::FactoryHolderRepository CreatureAIRepository;

//GO
struct SelectableGameObjectAI : public FactoryHolder<GameObjectAI>, public Permissible<GameObject>
{
    SelectableGameObjectAI(const char *id) : FactoryHolder<GameObjectAI>(id) {}
};

template<class REAL_GO_AI>
struct GameObjectAIFactory : public SelectableGameObjectAI
{
    GameObjectAIFactory(const char *name) : SelectableGameObjectAI(name) {}

    GameObjectAI* Create(void *) const;

    int Permit(const GameObject *g) const { return REAL_GO_AI::Permissible(g); }
};

template<class REAL_GO_AI>
inline GameObjectAI*
GameObjectAIFactory<REAL_GO_AI>::Create(void *data) const
{
    GameObject* go = reinterpret_cast<GameObject *>(data);
    return (new REAL_GO_AI(go));
}

typedef FactoryHolder<GameObjectAI> GameObjectAICreator;
typedef FactoryHolder<GameObjectAI>::FactoryHolderRegistry GameObjectAIRegistry;
typedef FactoryHolder<GameObjectAI>::FactoryHolderRepository GameObjectAIRepository;
#endif

