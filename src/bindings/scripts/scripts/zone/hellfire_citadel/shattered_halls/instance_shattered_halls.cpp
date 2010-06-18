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
SDName: Instance_Shattered_Halls
SD%Complete: 90
SDComment: Loots of the Executioner should depend on the amount of prisoners killed
SDCategory: Hellfire Citadel, Shattered Halls
EndScriptData */

#include "precompiled.h"
#include "def_shattered_halls.h"

#define ENCOUNTERS  6

uint32 HordePrisoners[3] = { 17296, 17295, 17297 };
uint32 AlliancePrisoners[3] = { 17290, 17292, 17289 };
uint32 PrisonersCoord[3][4] = 
{
    { 147.752808, -79.643730, 1.917701, 5.537074 },
    { 142.168777, -84.358223, 1.908038, 6.264657 },
    { 145.993637, -89.312386, 1.915693, 0.714077 }
};

struct TRINITY_DLL_DECL instance_shattered_halls : public ScriptedInstance
{
    instance_shattered_halls(Map* pMap) : ScriptedInstance(pMap) { Initialize(); };

    uint32 Encounters[ENCOUNTERS];
    uint32 SaveIntervalTimer;
    uint32 TimerLeft;
    
    uint64 FirstDoorGUID;
    uint64 SecondDoorGUID;
    uint64 NethekurseGUID;
    uint64 ExecutionerGUID;
    
    Creature* Executioner;
    Creature* FirstPrisoner;
    Creature* SecondPrisoner;
    Creature* ThirdPrisoner;
    
    bool HeroicMode;
    bool hasCasted80min;
    bool hasCasted25min;
    bool hasCasted15min;

    void Initialize()
    {
        SaveIntervalTimer = 30000;                  // Saving every 30 sec looks correct
        
        FirstDoorGUID = 0;
        SecondDoorGUID = 0;
        NethekurseGUID = 0;
        ExecutionerGUID = 0;
        
        Executioner = NULL;
        FirstPrisoner = NULL;
        SecondPrisoner = NULL;
        ThirdPrisoner = NULL;

        for(uint8 i = 0; i < ENCOUNTERS-1; i++)     // Do not override saved Timer value !
            Encounters[i] = NOT_STARTED;
            
        TimerLeft = 4800000;                        // 80 mins, in ms
            
        HeroicMode = this->instance->IsHeroic();
    }

    void OnObjectCreate(GameObject* pGo)
    {
        switch (pGo->GetEntry())
        {
        case ENTRY_FIRST_DOOR:
            FirstDoorGUID = pGo->GetGUID();
            if (Encounters[DATA_NETHEKURSE_EVENT] == DONE)
                HandleGameObject(NULL, true, pGo);
            break;
        case ENTRY_SECOND_DOOR:
            SecondDoorGUID = pGo->GetGUID();
            if (Encounters[DATA_NETHEKURSE_EVENT] == DONE)
                HandleGameObject(NULL, true, pGo);
            break;
        }
    }

    void OnCreatureCreate(Creature* pCreature, uint32 creature_entry)
    {
        uint8 pTeam = 0;
        if (Player* tempPlayer = GetPlayerInMap())
            pTeam = tempPlayer->GetTeam();
            
        switch (creature_entry) {
        case 16807:     // Nethekurse
            NethekurseGUID = pCreature->GetGUID();
            break;
        case 17301:     // Executioner
            ExecutionerGUID = pCreature->GetGUID();
            Executioner = pCreature;
            if (GetData(DATA_NETHEKURSE_EVENT) == NOT_STARTED)
                pCreature->SetVisibility(VISIBILITY_OFF);
            if (GetData(DATA_BLADEFIST_EVENT) == NOT_STARTED)
                pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
            break;
        // Horde prisoners
        /*case 17296:
            if (pTeam == HORDE) FirstPrisoner = pCreature;
            else pCreature->SetVisibility(VISIBILITY_OFF);
            break;
        case 17295:
            if (pTeam == HORDE) SecondPrisoner = pCreature;
            else pCreature->SetVisibility(VISIBILITY_OFF);
            break;
        case 17297:
            if (pTeam == HORDE) ThirdPrisoner = pCreature;
            else pCreature->SetVisibility(VISIBILITY_OFF);
            break;
        // Alliance prisoners
        case 17290:
            if (pTeam == ALLIANCE)  FirstPrisoner = pCreature;
            else pCreature->SetVisibility(VISIBILITY_OFF);
            break;
        case 17292:
            if (pTeam == ALLIANCE)  SecondPrisoner = pCreature;
            else pCreature->SetVisibility(VISIBILITY_OFF);
            break;
        case 17289:
            if (pTeam == ALLIANCE)  ThirdPrisoner = pCreature;
            else pCreature->SetVisibility(VISIBILITY_OFF);
            break;*/
        }
    }
    
    Player* GetPlayerInMap()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* plr = itr->getSource())
                    return plr;
            }
        }

        debug_log("TSCR: Instance Shattered Halls: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }
    
    // This is a hack. Actually, LootMgr should be able to dynamically change loot table depending on the amount of prisoners killed.
    void RewardAllPlayersInMapForQuest()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* plr = itr->getSource()) {
                    if (plr->GetQuestStatus(10884) == QUEST_STATUS_INCOMPLETE) {
                        plr->CompleteQuest(10884);      // Directly complete quest, so we are sure it is
                        // Now, we try to give the quest item, that looks more blizzlike that way
                        ItemPosCountVec dest;
                        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 31716, 1);
                        if (msg == EQUIP_ERR_OK) {
                            Item* item = plr->StoreNewItem(dest, 31716, true);
                            if (item)
                                plr->SendNewItem(item, 1, false, true);
                        } else
                            plr->SendEquipError(msg,NULL,NULL);
                    }
                }
            }
        }
    }
    
    void CastSpellOnAllPlayersInMap(uint32 spellid)
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* plr = itr->getSource())
                    plr->CastSpell(plr, spellid, true);
            }
        }
    }
    
    bool IsEncounterInProgress()
    {
        for (uint8 i = 0; i < ENCOUNTERS; i++)
            if (Encounters[i] == IN_PROGRESS) return true;
            
        return false;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch (type)
        {
        case DATA_NETHEKURSE_EVENT:
            Encounters[DATA_NETHEKURSE_EVENT] = data;
            if (data == DONE) {
                HandleGameObject(FirstDoorGUID, true, NULL);
                HandleGameObject(SecondDoorGUID, true, NULL);
            }
            break;
        case DATA_PORUNG_EVENT:
            Encounters[DATA_PORUNG_EVENT] = data;
            break;
        case DATA_OMROGG_EVENT:
            Encounters[DATA_OMROGG_EVENT] = data;
            break;
        case DATA_BLADEFIST_EVENT:
            Encounters[DATA_BLADEFIST_EVENT] = data;
            break;
        case DATA_EXECUTIONER_EVENT:
            Encounters[DATA_EXECUTIONER_EVENT] = data;
            break;
        case DATA_TIMER_LEFT:
            Encounters[5] = data;
        }
        
        if (data == DONE)
            SaveToDB();
    }

    uint32 GetData(uint32 type)
    {
        switch (type)
        {
        case DATA_NETHEKURSE_EVENT:
            return Encounters[DATA_NETHEKURSE_EVENT];
        case DATA_PORUNG_EVENT:
            return Encounters[DATA_PORUNG_EVENT];
        case DATA_OMROGG_EVENT:
            return Encounters[DATA_OMROGG_EVENT];
        case DATA_BLADEFIST_EVENT:
            return Encounters[DATA_BLADEFIST_EVENT];
        case DATA_EXECUTIONER_EVENT:
            return Encounters[DATA_EXECUTIONER_EVENT];
        case DATA_TIMER_LEFT:
            return Encounters[5];
        }
        
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch(data)
        {
        case ENTRY_FIRST_DOOR:
            return FirstDoorGUID;
        case ENTRY_SECOND_DOOR:
            return SecondDoorGUID;
        case DATA_NETHEKURSE_GUID:
            return NethekurseGUID;
        case DATA_EXECUTIONER_GUID:
            return ExecutionerGUID;
        }
        
        return 0;
    }
    
    const char* Save()
    {
        OUT_SAVE_INST_DATA;
        std::ostringstream stream;
        stream << Encounters[0] << " " << Encounters[1] << " " << Encounters[2] << " " << Encounters[3] << " " << Encounters[4] << " " << Encounters[5];
        char* out = new char[stream.str().length() + 1];
        strcpy(out, stream.str().c_str());
        if(out)
        {
            OUT_SAVE_INST_DATA_COMPLETE;
            return out;
        }

        return NULL;
    }
    
    void Load(const char* in)
    {
        if(!in)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(in);
        std::istringstream stream(in);
        stream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3] >> Encounters[4] >> Encounters[5];
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS && i != 4)                // Do not load an encounter as "In Progress" - reset it instead, except for Executioner.
                Encounters[i] = NOT_STARTED;
                
        TimerLeft = Encounters[5];
                
        // Check timer and respawn prisoners if needed
        /*if (GetData(DATA_EXECUTIONER_EVENT) == IN_PROGRESS) {
            if (Player* plr = GetPlayerInMap()) {
                // Less than 25 mins, one is dead
                if (TimerLeft < 1500000 && FirstPrisoner)
                    FirstPrisoner->DealDamage(FirstPrisoner, FirstPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                // Less than 15 mins, two are deads
                if (TimerLeft < 900000 && SecondPrisoner)
                    SecondPrisoner->DealDamage(SecondPrisoner, SecondPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }
        }
        else if (GetData(DATA_EXECUTIONER_EVENT) == DONE) {         // All prisoners dead
            if (FirstPrisoner)
                FirstPrisoner->DealDamage(FirstPrisoner, FirstPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            if (SecondPrisoner)
                SecondPrisoner->DealDamage(SecondPrisoner, SecondPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            if (ThirdPrisoner)
                ThirdPrisoner->DealDamage(ThirdPrisoner, ThirdPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }*/
                
        OUT_LOAD_INST_DATA_COMPLETE;
    }
    
    /*void SpawnPrisoners(uint32 team, uint8 amount)      // Amount depends on time left, team depends on player's team
    {
        if (!Executioner) {
            sLog.outError("Instance Shattered Halls: SpawnPrisoners: Executioner not found ! Instance script will fail.");
            return;
        }
        
        switch (team) {
        case HORDE:
            FirstPrisoner = Executioner->SummonCreature(HordePrisoners[0], PrisonersCoord[0][0], PrisonersCoord[0][1], PrisonersCoord[0][2], PrisonersCoord[0][3], TEMPSUMMON_DEAD_DESPAWN, 0);
            if (!FirstPrisoner) sLog.outError("Instance Shattered Halls: SpawnPrisoners: First prisoner not correctly spawned !");
            if (amount == 1) break;
            SecondPrisoner = Executioner->SummonCreature(HordePrisoners[1], PrisonersCoord[1][0], PrisonersCoord[1][1], PrisonersCoord[1][2], PrisonersCoord[1][3], TEMPSUMMON_DEAD_DESPAWN, 0);
            if (!SecondPrisoner) sLog.outError("Instance Shattered Halls: SpawnPrisoners: Second prisoner not correctly spawned !");
            if (amount == 2) break;
            ThirdPrisoner = Executioner->SummonCreature(HordePrisoners[2], PrisonersCoord[2][0], PrisonersCoord[2][1], PrisonersCoord[2][2], PrisonersCoord[2][3], TEMPSUMMON_DEAD_DESPAWN, 0);
            if (!ThirdPrisoner) sLog.outError("Instance Shattered Halls: SpawnPrisoners: Third prisoner not correctly spawned !");
            break;
        case ALLIANCE:
            FirstPrisoner = Executioner->SummonCreature(AlliancePrisoners[0], PrisonersCoord[0][0], PrisonersCoord[0][1], PrisonersCoord[0][2], PrisonersCoord[0][3], TEMPSUMMON_DEAD_DESPAWN, 0);
            if (!FirstPrisoner) sLog.outError("Instance Shattered Halls: SpawnPrisoners: First prisoner not correctly spawned !");
            if (amount == 1) break;
            SecondPrisoner = Executioner->SummonCreature(AlliancePrisoners[1], PrisonersCoord[1][0], PrisonersCoord[1][1], PrisonersCoord[1][2], PrisonersCoord[1][3], TEMPSUMMON_DEAD_DESPAWN, 0);
            if (!SecondPrisoner) sLog.outError("Instance Shattered Halls: SpawnPrisoners: Second prisoner not correctly spawned !");
            if (amount == 2) break;
            ThirdPrisoner = Executioner->SummonCreature(AlliancePrisoners[2], PrisonersCoord[2][0], PrisonersCoord[2][1], PrisonersCoord[2][2], PrisonersCoord[2][3], TEMPSUMMON_DEAD_DESPAWN, 0);
            if (!ThirdPrisoner) sLog.outError("Instance Shattered Halls: SpawnPrisoners: Third prisoner not correctly spawned !");
            break;
        }
    }*/
    
    // Update is only needed in Heroic, for the timer
    void Update(uint32 const diff)
    {   
        if (!HeroicMode)
            return;
            
        // If Executioner is dead and some prisoners are still alive, reward players for quest
        /*if (Executioner && Executioner->isDead() && GetData(DATA_EXECUTIONER_EVENT) != DONE) {
            if ( (FirstPrisoner && FirstPrisoner->isAlive()) || (SecondPrisoner && SecondPrisoner->isAlive()) || (ThirdPrisoner && ThirdPrisoner->isAlive()) ) {
                RewardAllPlayersInMapForQuest();
                SetData(DATA_EXECUTIONER_EVENT, DONE);
            }
        }*/
            
        if (GetData(DATA_EXECUTIONER_EVENT) != IN_PROGRESS)
            return;
            
        // Decrease timer at each update
        TimerLeft -= diff;
        
        if (!hasCasted80min) {      // Event just started
            // TODO: Correct this [PH] yell
            if (Executioner)
                Executioner->Yell("[PH] J'ai 3 otages ! Dans 55 minutes, je tuerai le premier d'entre eux !", LANG_UNIVERSAL, 0);
            CastSpellOnAllPlayersInMap(EXEC_TIMER_55);
            Player* plr = GetPlayerInMap();
            if (!plr) {
                sLog.outError("Instance Shattered Halls: Update: No player found in map when event started !");
                return;
            }
            //SpawnPrisoners(plr->GetTeam(), 3);
            hasCasted80min = true;
        }
        
        if (TimerLeft < 1500000 && !hasCasted25min) {               // 25 min left and debuff not casted yet
            // TODO: Correct this [PH] yell
            if (Executioner)
                Executioner->Yell("[PH] Le premier est mort ! Dans 10 minutes, un autre connaitra le meme sort !", LANG_UNIVERSAL, 0);
            CastSpellOnAllPlayersInMap(GetData(EXEC_TIMER_10));      // 2nd timer of 10 mins
            /*if (FirstPrisoner && Executioner)
                Executioner->DealDamage(FirstPrisoner, FirstPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);*/
            hasCasted25min = true;
        }
        
        if (TimerLeft < 900000 && !hasCasted15min) {                // 15 min left and debuff not casted yet
            // TODO: Correct this [PH] yell
            if (Executioner)
                Executioner->Yell("[PH] Et de deux ! Dans 15 minutes, je tuerai le dernier, et il sera trop tard !", LANG_UNIVERSAL, 0);
            CastSpellOnAllPlayersInMap(GetData(EXEC_TIMER_15));      // 3rd (and last) timer of 15 mins
            /*if (SecondPrisoner && Executioner)
                Executioner->DealDamage(SecondPrisoner, SecondPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);*/
            hasCasted15min = true;
        }
        
        if (TimerLeft < diff) {     // TIME UP ! Kill the third prisoner and stop the timer, killing executioner won't give the quest item
            // TODO: Correct this [PH] yell
            if (Executioner) {
                Executioner->Yell("[PH] Trop tard ! Vous avez perdu, ils sont tous morts de ma main.", LANG_UNIVERSAL, 0);
                Executioner->loot.RemoveItem(31716);
            }
            /*if (ThirdPrisoner && Executioner)
                Executioner->DealDamage(ThirdPrisoner, ThirdPrisoner->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);*/
            SetData(DATA_EXECUTIONER_EVENT, FAIL);
        }
        
        // Save time left in DB every 30 sec
        if (SaveIntervalTimer <= diff) {
            SetData(DATA_TIMER_LEFT, TimerLeft);
            SaveToDB();
            SaveIntervalTimer = 30000;
        }else SaveIntervalTimer -= diff;
    }
};

InstanceData* GetInstanceData_instance_shattered_halls(Map* pMap)
{
    return new instance_shattered_halls(pMap);
}

// AreaTrigger that starts the timed event
bool AreaTrigger_at_shattered_halls(Player* pPlayer, AreaTriggerEntry* at) {
    if (ScriptedInstance* pInstance = ((ScriptedInstance*)pPlayer->GetInstanceData())) {
        if (pInstance->GetData(DATA_EXECUTIONER_EVENT) != IN_PROGRESS) {
            pInstance->SetData(DATA_EXECUTIONER_EVENT, IN_PROGRESS);
            if (Creature* Executioner = Creature::GetCreature(*pPlayer, pInstance->GetData64(DATA_EXECUTIONER_GUID))) {
                Executioner->SetVisibility(VISIBILITY_ON);
                Executioner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
                // TODO: Yell something ?
            }
        }
    }
    
    return true;
}

void AddSC_instance_shattered_halls()
{
    Script* newscript;
    
    newscript = new Script;
    newscript->Name = "instance_shattered_halls";
    newscript->GetInstanceData = &GetInstanceData_instance_shattered_halls;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "at_shattered_halls";
    newscript->pAreaTrigger = &AreaTrigger_at_shattered_halls;
    newscript->RegisterSelf();
}

