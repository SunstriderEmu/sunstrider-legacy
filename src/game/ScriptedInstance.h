/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_INSTANCE_H
#define SC_INSTANCE_H

#include "InstanceData.h"
#include "Map.h"

#define OUT_SAVE_INST_DATA             
#define OUT_SAVE_INST_DATA_COMPLETE    
#define OUT_LOAD_INST_DATA(a)          
#define OUT_LOAD_INST_DATA_COMPLETE    
#define OUT_LOAD_INST_DATA_FAIL        TC_LOG_ERROR("FIXME","Unable to load Instance Data for Instance %s (Map %d, Instance Id: %d).",instance->GetMapName(), instance->GetId(), instance->GetInstanceId())

class ScriptedInstance : public InstanceData
{
    public:

        ScriptedInstance(Map *map) : InstanceData(map) {}
        ~ScriptedInstance() {}

        // Default accessor functions
        Creature* GetSingleCreatureFromStorage(uint32 uiEntry, bool bSkipDebugLog = false);

        //All-purpose data storage 64 bit
        virtual uint64 GetData64(uint32 Data) { return 0; }
        virtual void SetData64(uint32 Data, uint64 Value) { }

        // Called every instance update
        virtual void Update(uint32) {}

        // Save and Load instance data to the database
        const char* Save() { return NULL; }
        void Load(const char* in) { }
        
        // Misc
        void CastOnAllPlayers(uint32 spellId);  // TODO: Add Unit* caster as parameter?
        void RemoveAuraOnAllPlayers(uint32 spellId);
        virtual void MonsterPulled(Creature* creature, Unit* puller); // puller can be a pet, thus use a Unit ptr
        virtual void PlayerDied(Player* player);
        
        void SendScriptInTestNoLootMessageToAll();
        
        Player* GetPlayerInMap();

    protected:
        // Storage for GO-Guids and NPC-Guids
        typedef std::map<uint32, uint64> EntryGuidMap;
        EntryGuidMap m_mNpcEntryGuidStore;                  ///< Store unique NPC-Guids by entry
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntry
{
    int32 iTextEntry;                                       ///< To be said text entry
    uint32 uiSayerEntry;                                    ///< Entry of the mob who should say
    uint32 uiTimer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntryTwoSide
{
    int32 iTextEntry;                                       ///< To be said text entry (first side)
    uint32 uiSayerEntry;                                    ///< Entry of the mob who should say (first side)
    int32 iTextEntryAlt;                                    ///< To be said text entry (second side)
    uint32 uiSayerEntryAlt;                                 ///< Entry of the mob who should say (second side)
    uint32 uiTimer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// Helper class handling a dialogue given as static const array of DialogueEntry or DialogueEntryTwoSide
class DialogueHelper
{
    public:
        // The array MUST be terminated by {0,0,0}
        DialogueHelper(DialogueEntry const* pDialogueArray);
        // The array MUST be terminated by {0,0,0,0,0}
        DialogueHelper(DialogueEntryTwoSide const* aDialogueTwoSide);

        /// Function to initialize the dialogue helper for instances. If not used with instances, GetSpeakerByEntry MUST be overwritten to obtain the speakers
        void InitializeDialogueHelper(ScriptedInstance* pInstance) { m_pInstance = pInstance; }
        /// Set if take first entries or second entries
        void SetDialogueSide(bool bIsFirstSide) { m_bIsFirstSide = bIsFirstSide; }

        void StartNextDialogueText(int32 iTextEntry);

        void DialogueUpdate(uint32 uiDiff);

        void SetNewArray(DialogueEntry const* pDialogueArray) { m_pDialogueArray = pDialogueArray; }
        void SetNewArray(DialogueEntryTwoSide const* aDialogueTwoSide) { m_pDialogueTwoSideArray = aDialogueTwoSide; }

    protected:
        /// Will be called when a dialogue step was done
        virtual void JustDidDialogueStep(int32 /*iEntry*/) {}
        /// Will be called to get a speaker, MUST be implemented if not used in instances
        virtual Creature* GetSpeakerByEntry(uint32 /*uiEntry*/) { return NULL; }

    private:
        void DoNextDialogueStep();

        ScriptedInstance* m_pInstance;

        DialogueEntry const* m_pDialogueArray;
        DialogueEntry const* m_pCurrentEntry;
        DialogueEntryTwoSide const* m_pDialogueTwoSideArray;
        DialogueEntryTwoSide const* m_pCurrentEntryTwoSide;

        uint32 m_uiTimer;
        bool m_bIsFirstSide;
};

#endif

