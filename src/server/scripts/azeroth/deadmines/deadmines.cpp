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
SDName: Deadmines
SD%Complete: 0
SDComment: Placeholder
SDCategory: Deadmines
EndScriptData */


#include "def_deadmines.h"
#include "Spell.h"

#define SOUND_CANNONFIRE    1400
#define SOUND_DESTROYDOOR    3079
#define SAY_MR_SMITE_ALARM1 "You there, check out that noise!"
#define SOUND_MR_SMITE_ALARM1  5775
#define SAY_MR_SMITE_ALARM2 "We're under attack! A vast, ye swabs! Repel the invaders!"
#define SOUND_MR_SMITE_ALARM2  5777

#define GO_IRONCLAD_DOOR    16397
#define GO_DEFIAS_CANNON    16398
#define GO_DOOR_LEVER        101833

#define CANNON_BLAST_TIMER 3000
#define PIRATES_DELAY_TIMER 1000

class instance_deadmines : public InstanceMapScript
{
public:
    instance_deadmines() : InstanceMapScript("instance_deadmines", 36) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_deadmines_script(map);
    }

    struct instance_deadmines_script : public InstanceScript
    {
        instance_deadmines_script(Map *Map) : InstanceScript(Map) { Initialize(); };

        GameObject* IronCladDoor;
        GameObject* DefiasCannon;
        GameObject* DoorLever;
        Creature* DefiasPirate1;
        Creature* DefiasPirate2;
        Creature* DefiasCompanion;
        uint32 State;
        uint32 CannonBlast_Timer;
        uint32 PiratesDelay_Timer;

        void Initialize()
            override {
            IronCladDoor = nullptr;
            DefiasCannon = nullptr;
            DoorLever = nullptr;
            State = CANNON_NOT_USED;
        }

        void Update(uint32 diff)
            override {
            if (!IronCladDoor || !DefiasCannon || !DoorLever)
                return;

            switch (State)
            {
            case CANNON_GUNPOWDER_USED:
                CannonBlast_Timer = CANNON_BLAST_TIMER;
                // it's a hack - Mr. Smite should do that but his too far away
                if (Creature* yeller = IronCladDoor->SummonTrigger(POSITION_GET_X_Y_Z(IronCladDoor), 0.0f, 1))
                {
                    yeller->SetName("Mr. Smite");
                    yeller->Yell(SAY_MR_SMITE_ALARM1);
                }
                DoPlaySound(IronCladDoor, SOUND_MR_SMITE_ALARM1);
                State = CANNON_BLAST_INITIATED;
                break;
            case CANNON_BLAST_INITIATED:
                PiratesDelay_Timer = PIRATES_DELAY_TIMER;
                if (CannonBlast_Timer < diff)
                {
                    SummonCreatures();
                    ShootCannon();
                    BlastOutDoor();
                    LeverStucked();
                    if (Creature* yeller = IronCladDoor->SummonTrigger(POSITION_GET_X_Y_Z(IronCladDoor), 0.0f, 1))
                    {
                        yeller->SetName("Mr. Smite");
                        yeller->Yell(SAY_MR_SMITE_ALARM2);
                    }
                    DoPlaySound(IronCladDoor, SOUND_MR_SMITE_ALARM2);
                    State = PIRATES_ATTACK;
                }
                else
                    CannonBlast_Timer -= diff;
                break;
            case PIRATES_ATTACK:
                if (PiratesDelay_Timer < diff)
                {
                    MoveCreaturesInside();
                    State = EVENT_DONE;
                }
                else
                    PiratesDelay_Timer -= diff;
                break;
            }
        }

        void SummonCreatures()
        {
            DefiasPirate1 = IronCladDoor->SummonCreature(657, IronCladDoor->GetPositionX() - 2, IronCladDoor->GetPositionY() - 7, IronCladDoor->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
            DefiasPirate2 = IronCladDoor->SummonCreature(657, IronCladDoor->GetPositionX() + 3, IronCladDoor->GetPositionY() - 6, IronCladDoor->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
            DefiasCompanion = IronCladDoor->SummonCreature(3450, IronCladDoor->GetPositionX() + 2, IronCladDoor->GetPositionY() - 6, IronCladDoor->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
        }

        void MoveCreaturesInside()
        {
            if (!DefiasPirate1 || !DefiasPirate2 || !DefiasCompanion)
                return;

            MoveCreatureInside(DefiasPirate1);
            MoveCreatureInside(DefiasPirate2);
            MoveCreatureInside(DefiasCompanion);
        }

        void MoveCreatureInside(Creature *creature)
        {
            creature->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
            creature->GetMotionMaster()->MovePoint(0, -102.7, -655.9, creature->GetPositionZ());
        }

        void ShootCannon()
        {
            DefiasCannon->SetUInt32Value(GAMEOBJECT_STATE, 0);
            DoPlaySound(DefiasCannon, SOUND_CANNONFIRE);
        }

        void BlastOutDoor()
        {
            IronCladDoor->SetUInt32Value(GAMEOBJECT_STATE, 2);
            DoPlaySound(IronCladDoor, SOUND_DESTROYDOOR);
        }

        void LeverStucked()
        {
            DoorLever->SetUInt32Value(GAMEOBJECT_FLAGS, 4);
        }

        void OnGameObjectCreate(GameObject *go)
            override {
            switch (go->GetEntry())
            {
            case GO_IRONCLAD_DOOR:
                IronCladDoor = go;
                break;
            case GO_DEFIAS_CANNON:
                DefiasCannon = go;
                break;
            case GO_DOOR_LEVER:
                DoorLever = go;
                break;
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type == EVENT_STATE)
            {
                if (DefiasCannon && IronCladDoor)
                    State = data;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == EVENT_STATE)
                return State;
            return 0;
        }

        void DoPlaySound(GameObject* unit, uint32 sound)
        {
            WorldPacket data(4);
            data.SetOpcode(SMSG_PLAY_SOUND);
            data << uint32(sound);
            unit->SendMessageToSet(&data, false);
        }

        void DoPlaySoundCreature(Unit* unit, uint32 sound)
        {
            WorldPacket data(4);
            data.SetOpcode(SMSG_PLAY_SOUND);
            data << uint32(sound);
            unit->SendMessageToSet(&data, false);
        }
    };
};

class ItemDefiasGunpowder : ItemScript
{
public:
    ItemDefiasGunpowder() : ItemScript("item_defias_gunpowder") {}

    bool OnUse(Player* player, Item* _Item, SpellCastTargets const& targets) override
    {
        InstanceScript *pInstance = (player->GetInstanceScript()) ? ((InstanceScript*)player->GetInstanceScript()) : nullptr;

        if (!pInstance)
        {
            player->GetSession()->SendNotification("Instance script not initialized");
            return true;
        }
        if (pInstance->GetData(EVENT_STATE) != CANNON_NOT_USED)
            return false;
        if (targets.GetGOTarget() && targets.GetGOTarget()->GetTypeId() == TYPEID_GAMEOBJECT &&
            targets.GetGOTarget()->GetEntry() == GO_DEFIAS_CANNON)
        {
            pInstance->SetData(EVENT_STATE, CANNON_GUNPOWDER_USED);
        }

        player->DestroyItemCount(_Item->GetEntry(), 1, true);
        return true;
    }
};

void AddSC_instance_deadmines()
{
    new instance_deadmines();
    new ItemDefiasGunpowder();
}


