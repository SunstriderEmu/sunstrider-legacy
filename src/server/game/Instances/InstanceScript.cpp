
#include "InstanceScript.h"
#include "Player.h"
#include "Map.h"
#include "MapReference.h"
#include "Chat.h"
#include "Language.h"
#include "ScriptMgr.h"

InstanceScript::InstanceScript(Map *map) : instance(map)
{
#ifdef TRINITY_API_USE_DYNAMIC_LINKING
    uint32 scriptId = sObjectMgr->GetInstanceTemplate(map->GetId())->ScriptId;
    auto const scriptname = sObjectMgr->GetScriptName(scriptId);
    ASSERT(!scriptname.empty());
    // Acquire a strong reference from the script module
    // to keep it loaded until this object is destroyed.
    module_reference = sScriptMgr->AcquireModuleReferenceOfScriptName(scriptname);
#endif // #ifndef TRINITY_API_USE_DYNAMIC_LINKING
}

void InstanceScript::CastOnAllPlayers(uint32 spellId)
{
    Map::PlayerList const& players = instance->GetPlayers();

    if (!players.isEmpty())
    {
        for(const auto & player : players)
        {
            if (Player* plr = player.GetSource())
                plr->CastSpell(plr, spellId, true);                
        }
    }
}

void InstanceScript::RemoveAuraOnAllPlayers(uint32 spellId)
{
    Map::PlayerList const& players = instance->GetPlayers();

    if (!players.isEmpty())
    {
        for(const auto & player : players)
        {
            if (Player* plr = player.GetSource())
                plr->RemoveAurasDueToSpell(spellId);
        }
    }
}

void InstanceScript::MonsterPulled(Creature* creature, Unit* puller)
{
    
}

void InstanceScript::PlayerDied(Player* player)
{
    
}

void InstanceScript::SendScriptInTestNoLootMessageToAll()
{
    Map::PlayerList const& players = instance->GetPlayers();

    if (!players.isEmpty()) {
        for (const auto & player : players) {
            if (Player* plr = player.GetSource())
                ChatHandler(plr).SendSysMessage(LANG_SCRIPT_IN_TEST_NO_LOOT);
        }
    }
}

bool InstanceScript::IsEncounterInProgress() const
{
    for (const auto & bosse : bosses)
        //if (itr->state == IN_PROGRESS)
        if (GetData(bosse.bossId) == IN_PROGRESS)
            return true;

    return false;
}

void InstanceScript::LoadDoorData(const DoorData* data)
{
    while (data->entry)
    {
        if (data->bossId < bosses.size())
            doors.insert(std::make_pair(data->entry, DoorInfo(&bosses[data->bossId], data->type, BoundaryType(data->boundary))));

        ++data;
    }
    ;//sLog->outDebug(LOG_FILTER_TSCR, "InstanceScript::LoadDoorData: " UI64FMTD " doors loaded.", uint64(doors.size()));
}

void InstanceScript::UpdateDoorState(GameObject* door)
{
    DoorInfoMapBounds range = doors.equal_range(door->GetEntry());
    if (range.first == range.second)
        return;

    // xinef: doors can be assigned to few bosses, if any of them demands doors closed - they should be closed (added & operator for assigment)
    bool open = true;
    for (; range.first != range.second && open; ++range.first)
    {
        DoorInfo const& info = range.first->second;

        //EncounterState state = info.bossInfo->state;
        EncounterState state = EncounterState(GetData(info.bossInfo->bossId));
        switch (info.type)
        {
        case DOOR_TYPE_ROOM:
            open &= (state != IN_PROGRESS) ? true : false;
            break;
        case DOOR_TYPE_PASSAGE:
            open &= (state == DONE) ? true : false;
            break;
        case DOOR_TYPE_SPAWN_HOLE:
            open &= (state == IN_PROGRESS) ? true : false;
            break;
        default:
            break;
        }
    }

    door->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
}

void InstanceScript::AddDoor(GameObject* door, bool add)
{
    DoorInfoMapBounds range = doors.equal_range(door->GetEntry());
    if (range.first == range.second)
        return;

    for (; range.first != range.second; ++range.first)
    {
        DoorInfo const& data = range.first->second;

        if (add)
        {
            data.bossInfo->door[data.type].insert(door);
            switch (data.boundary)
            {
            default:
            case BOUNDARY_NONE:
                break;
            case BOUNDARY_N:
            case BOUNDARY_S:
                data.bossInfo->boundary[data.boundary] = door->GetPositionX();
                break;
            case BOUNDARY_E:
            case BOUNDARY_W:
                data.bossInfo->boundary[data.boundary] = door->GetPositionY();
                break;
            case BOUNDARY_NW:
            case BOUNDARY_SE:
                data.bossInfo->boundary[data.boundary] = door->GetPositionX() + door->GetPositionY();
                break;
            case BOUNDARY_NE:
            case BOUNDARY_SW:
                data.bossInfo->boundary[data.boundary] = door->GetPositionX() - door->GetPositionY();
                break;
            }
        }
        else
            data.bossInfo->door[data.type].erase(door);
    }

    if (add)
        UpdateDoorState(door);
}


bool InstanceScript::SetBossState(uint32 id, EncounterState state)
{
    if (id < bosses.size())
    {
        BossInfo* bossInfo = &bosses[id];
        //if (bossInfo->state == TO_BE_DECIDED) // loading
        if (GetData(id) == TO_BE_DECIDED) // loading
        {
            //bossInfo->state = state;
            SetData(id, state);
            //sLog->outError("Inialize boss %u state as %u.", id, (uint32)state);
            return false;
        }
        else
        {
            if (GetData(id) == state)
            //if (bossInfo->state == state)
                return false;

            /* NYI
            if (state == DONE)
                for (MinionSet::iterator i = bossInfo->minion.begin(); i != bossInfo->minion.end(); ++i)
                    if ((*i)->isWorldBoss() && (*i)->IsAlive())
                        return false;
                        */
            //bossInfo->state = state;
            SetData(id, state);
            SaveToDB();
        }

        for (auto & type : bossInfo->door)
            for (auto i = type.begin(); i != type.end(); ++i)
                UpdateDoorState(*i);

        /* NYI
        for (MinionSet::iterator i = bossInfo->minion.begin(); i != bossInfo->minion.end(); ++i)
            UpdateMinionState(*i, state);
            */

        return true;
    }
    return false;
}


Player* InstanceScript::GetPlayer() const
{
    Map::PlayerList const& players = instance->GetPlayers();

    if (!players.isEmpty()) {
        for (const auto & player : players) {
            Player* plr = player.GetSource();
            if (plr)
                return plr;
        }
    }

    TC_LOG_ERROR("scripts","InstanceScript: GetPlayerInMap (map: %u) called, and no player found!", (instance ? instance->GetId() : 0));
    return nullptr;
}

/// Returns a pointer to a loaded Creature that was stored in m_mNpcEntryGuidStore. Can return NULL
Creature* InstanceScript::GetSingleCreatureFromStorage(uint32 uiEntry, bool bSkipDebugLog /*=false*/)
{
    auto find = m_mNpcEntryGuidStore.find(uiEntry);
    if (find != m_mNpcEntryGuidStore.end())
        return instance->GetCreature(find->second);

    // Output log, possible reason is not added GO to map, or not yet loaded;
    if (!bSkipDebugLog)
        error_log("OLDScript requested creature with entry %u, but no npc of this entry was created yet, or it was not stored by script for map %u.", uiEntry, instance->GetId());

    return nullptr;
}

/**
   Constructor for DialogueHelper

   @param   pDialogueArray The static const array of DialogueEntry holding the information about the dialogue. This array MUST be terminated by {0,0,0}
 */
DialogueHelper::DialogueHelper(DialogueEntry const* pDialogueArray) :
    m_pInstance(nullptr),
    m_pDialogueArray(pDialogueArray),
    m_pCurrentEntry(nullptr),
    m_pDialogueTwoSideArray(nullptr),
    m_pCurrentEntryTwoSide(nullptr),
    m_uiTimer(0),
    m_bIsFirstSide(true)
{}

/**
   Constructor for DialogueHelper (Two Sides)

   @param   pDialogueTwoSideArray The static const array of DialogueEntryTwoSide holding the information about the dialogue. This array MUST be terminated by {0,0,0,0,0}
 */
DialogueHelper::DialogueHelper(DialogueEntryTwoSide const* pDialogueTwoSideArray) :
    m_pInstance(nullptr),
    m_pDialogueArray(nullptr),
    m_pCurrentEntry(nullptr),
    m_pDialogueTwoSideArray(pDialogueTwoSideArray),
    m_pCurrentEntryTwoSide(nullptr),
    m_uiTimer(0),
    m_bIsFirstSide(true)
{}

/**
   Function to start a (part of a) dialogue

   @param   iTextEntry The TextEntry of the dialogue that will be started (must be always the entry of first side)
 */
void DialogueHelper::StartNextDialogueText(int32 iTextEntry)
{
    // Find iTextEntry
    bool bFound = false;

    if (m_pDialogueArray)                                   // One Side
    {
        for (DialogueEntry const* pEntry = m_pDialogueArray; pEntry->iTextEntry; ++pEntry)
        {
            if (pEntry->iTextEntry == iTextEntry)
            {
                m_pCurrentEntry = pEntry;
                bFound = true;
                break;
            }
        }
    }
    else                                                    // Two Sides
    {
        for (DialogueEntryTwoSide const* pEntry = m_pDialogueTwoSideArray; pEntry->iTextEntry; ++pEntry)
        {
            if (pEntry->iTextEntry == iTextEntry)
            {
                m_pCurrentEntryTwoSide = pEntry;
                bFound = true;
                break;
            }
        }
    }

    if (!bFound)
    {
        error_log("OLDScript call DialogueHelper::StartNextDialogueText, but textEntry %i is not in provided dialogue (on map id %u)", iTextEntry, m_pInstance ? m_pInstance->instance->GetId() : 0);
        return;
    }

    DoNextDialogueStep();
}

/// Internal helper function to do the actual say of a DialogueEntry
void DialogueHelper::DoNextDialogueStep()
{
    // Last Dialogue Entry done?
    if ((m_pCurrentEntry && !m_pCurrentEntry->iTextEntry) || (m_pCurrentEntryTwoSide && !m_pCurrentEntryTwoSide->iTextEntry))
    {
        m_uiTimer = 0;
        return;
    }

    // Get Text, SpeakerEntry and Timer
    int32 iTextEntry = 0;
    uint32 uiSpeakerEntry = 0;

    if (m_pDialogueArray)                               // One Side
    {
        uiSpeakerEntry = m_pCurrentEntry->uiSayerEntry;
        iTextEntry = m_pCurrentEntry->iTextEntry;

        m_uiTimer = m_pCurrentEntry->uiTimer;
    }
    else                                                // Two Sides
    {
        // Second Entries can be 0, if they are the entry from first side will be taken
        uiSpeakerEntry = !m_bIsFirstSide && m_pCurrentEntryTwoSide->uiSayerEntryAlt ? m_pCurrentEntryTwoSide->uiSayerEntryAlt : m_pCurrentEntryTwoSide->uiSayerEntry;
        iTextEntry = !m_bIsFirstSide && m_pCurrentEntryTwoSide->iTextEntryAlt ? m_pCurrentEntryTwoSide->iTextEntryAlt : m_pCurrentEntryTwoSide->iTextEntry;

        m_uiTimer = m_pCurrentEntryTwoSide->uiTimer;
    }

    // Simulate Case
    if (uiSpeakerEntry && iTextEntry < 0)
    {
        // Use Speaker if directly provided
        Creature* pSpeaker = GetSpeakerByEntry(uiSpeakerEntry);
        if (m_pInstance && !pSpeaker)                       // Get Speaker from instance
            pSpeaker = m_pInstance->GetSingleCreatureFromStorage(uiSpeakerEntry);

        if (pSpeaker)
            DoScriptText(iTextEntry, pSpeaker);
    }

    JustDidDialogueStep(m_pDialogueArray ?  m_pCurrentEntry->iTextEntry : m_pCurrentEntryTwoSide->iTextEntry);

    // Increment position
    if (m_pDialogueArray)
        ++m_pCurrentEntry;
    else
        ++m_pCurrentEntryTwoSide;
}

/// Call this function within any DialogueUpdate method. This is required for saying next steps in a dialogue
void DialogueHelper::DialogueUpdate(uint32 uiDiff)
{
    if (m_uiTimer)
    {
        if (m_uiTimer <= uiDiff)
            DoNextDialogueStep();
        else
            m_uiTimer -= uiDiff;
    }
}


void InstanceScript::SaveToDB()
{
	if (!Save()) 
		return;

	std::string data = Save();
	CharacterDatabase.EscapeString(data);
	CharacterDatabase.PExecute("UPDATE instance SET data = '%s' WHERE id = '%d'", data.c_str(), instance->GetInstanceId());
}

void InstanceScript::HandleGameObject(uint64 GUID, bool open, GameObject *go)
{
	if (!go)
		go = instance->GetGameObject(GUID);
	if (go)
		go->SetGoState(GOState(open ? 0 : 1));
	else
		TC_LOG_ERROR("scripts", "ZoneScript: HandleGameObject failed for gameobject with GUID %u", GUID_LOPART(GUID));
}

void InstanceScript::DoRespawnGameObject(uint64 uiGuid, uint32 uiTimeToDespawn)
{
	if (GameObject* pGo = instance->GetGameObject(uiGuid))
	{
		//not expect any of these should ever be handled
		if (pGo->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE || pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR ||
			pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON || pGo->GetGoType() == GAMEOBJECT_TYPE_TRAP)
			return;

		if (pGo->isSpawned())
			return;

		pGo->SetRespawnTime(uiTimeToDespawn);
	}
}

void InstanceScript::DoUseDoorOrButton(uint64 uiGuid, uint32 uiWithRestoreTime, bool bUseAlternativeState)
{
	if (!uiGuid)
		return;

	GameObject* pGo = instance->GetGameObject(uiGuid);

	if (pGo)
	{
		if (pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR || pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
		{
			if (pGo->getLootState() == GO_READY)
				pGo->UseDoorOrButton(uiWithRestoreTime);
			else if (pGo->getLootState() == GO_ACTIVATED)
				pGo->ResetDoorOrButton();
		}
		else
			error_log("OSCR: Script call DoUseDoorOrButton, but gameobject entry %u is type %u.", pGo->GetEntry(), pGo->GetGoType());
	}
}