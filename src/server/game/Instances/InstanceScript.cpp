
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

bool InstanceHasScript(WorldObject const* obj, char const* scriptName)
{
    if (InstanceMap* instance = obj->GetMap()->ToInstanceMap())
        return instance->GetScriptName() == scriptName;

    return false;
}