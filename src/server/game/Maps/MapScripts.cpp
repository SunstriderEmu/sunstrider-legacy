#include "CellImpl.h"
#include "GameTime.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "GossipDef.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "Item.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SmartAI.h"
#include "SmartScriptMgr.h"
#include "Transport.h"
#include "WaypointManager.h"
#include "World.h"

/// Put scripts in the execution queue
void Map::ScriptsStart(std::map<uint32, std::multimap<uint32, ScriptInfo>> const& scripts, uint32 id, Object* source, Object* target, bool start)
{
    ///- Find the script map
    auto s = scripts.find(id);
    if (s == scripts.end())
        return;

    // prepare static data
    ObjectGuid sourceGUID = source ? source->GetGUID() : ObjectGuid::Empty; //some script commands doesn't have source
    ObjectGuid targetGUID = target ? target->GetGUID() : ObjectGuid::Empty;
    ObjectGuid ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : ObjectGuid::Empty;

    ///- Schedule script execution for all scripts in the script map
    ScriptMap const *s2 = &(s->second);
    bool immedScript = false;
    for (const auto & iter : *s2)
    {
        ScriptAction sa;
        sa.sourceGUID = sourceGUID;
        sa.targetGUID = targetGUID;
        sa.ownerGUID  = ownerGUID;

        sa.script = &iter.second;
        //TC_LOG_INFO("SCRIPT: Inserting script with source guid " UI64FMTD " target guid " UI64FMTD " owner guid " UI64FMTD " script id %u", sourceGUID, targetGUID, ownerGUID, id);
        m_scriptSchedule.insert(ScriptScheduleMap::value_type(time_t(GetGameTime() + iter.first), sa));
        if (iter.first == 0)
            immedScript = true;

        sMapMgr->IncreaseScheduledScriptsCount();
    }
    ///- If one of the effects should be immediate, launch the script execution
    if (start && immedScript && !i_scriptLock)
    {
        i_scriptLock = true;
        ScriptsProcess();
        i_scriptLock = false;
    }
}

void Map::ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target)
{
    ASSERT(source);
    ASSERT(target);
    // NOTE: script record _must_ exist until command executed

    // prepare static data
    ObjectGuid sourceGUID = source->GetGUID();
    ObjectGuid targetGUID = target->GetGUID();
    ObjectGuid ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : ObjectGuid::Empty;

    ScriptAction sa;
    sa.sourceGUID = sourceGUID;
    sa.targetGUID = targetGUID;
    sa.ownerGUID  = ownerGUID;

    sa.script = &script;
    //TC_LOG_INFO("SCRIPTCMD: Inserting script with source guid " UI64FMTD " target guid " UI64FMTD " owner guid " UI64FMTD " script id %u", sourceGUID, targetGUID, ownerGUID, script.id);
    m_scriptSchedule.insert(ScriptScheduleMap::value_type(time_t(GetGameTime() + delay), sa));

    sMapMgr->IncreaseScheduledScriptsCount();

    ///- If effects should be immediate, launch the script execution
    if (delay == 0 && !i_scriptLock)
    {
        i_scriptLock = true;
        ScriptsProcess();
        i_scriptLock = false;
    }
}

/// Process queued scripts
void Map::ScriptsProcess()
{
    if (m_scriptSchedule.empty())
        return;

    ///- Process overdue queued scripts
    auto iter = m_scriptSchedule.begin();
                                                            // ok as multimap is a *sorted* associative container
    while (!m_scriptSchedule.empty() && (iter->first <= GetGameTime()))
    {
        ScriptAction const& step = iter->second;

        Object* source = nullptr;

        if(step.sourceGUID)
        {
            switch(HighGuid(step.sourceGUID.GetHigh()))
            {
                case HighGuid::Item:
                    if (Player* player = GetPlayer(step.ownerGUID))
                source = player->GetItemByGuid(step.sourceGUID);
                    break;
                case HighGuid::Unit:
                    source = GetCreature(step.sourceGUID);
                    break;
                case HighGuid::Pet:
                    source = GetPet(step.sourceGUID);
                    break;
                case HighGuid::Player:
                    source = GetPlayer(step.sourceGUID);
                    break;
            case HighGuid::Transport:
                case HighGuid::GameObject:
                    source = GetGameObject(step.sourceGUID);
                    break;
                case HighGuid::Corpse:
                    source = GetCorpse(step.sourceGUID);
                    break;
                case HighGuid::Mo_Transport:
                    GetTransport(step.sourceGUID);
            break;
                default:
                    TC_LOG_ERROR("scripts","*_script source with unsupported high guid value %u", uint32(step.sourceGUID.GetHigh()));
                    break;
            }
        }

        //if(source && !source->IsInWorld()) source = NULL;

        WorldObject* target = nullptr;

        if(step.targetGUID)
        {
            switch(step.targetGUID.GetHigh())
            {
                case HighGuid::Unit:
                    target = GetCreature(step.targetGUID);
                    break;
                case HighGuid::Pet:
                    target = GetPet(step.targetGUID);
                    break;
                case HighGuid::Player:                       // empty GUID case also
                    target = GetPlayer(step.targetGUID);
                    break;
                case HighGuid::GameObject:
                    target = GetGameObject(step.targetGUID);
                    break;
                case HighGuid::Corpse:
                    target = GetCorpse(step.targetGUID);
                    break;
                case HighGuid::Mo_Transport:
                    target = GetTransport(step.targetGUID);
                    break;
                default:
                    TC_LOG_ERROR("scripts","*_script source with unsupported high guid value %u", uint32(step.targetGUID.GetHigh()));
                    break;
            }
        }
        
        if (!source && !target)
            TC_LOG_ERROR("scripts","World::ScriptProcess: no source neither target for this script, if this is the last line before a crash, then you'd better return here.");

        switch (step.script->command)
        {
            case SCRIPT_COMMAND_TALK:
            {
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TALK call for NULL creature.");
                    break;
                }

                if (step.script->Talk.ChatType > CHAT_TYPE_BOSS_WHISPER && step.script->Talk.ChatType != CHAT_MSG_RAID_BOSS_WHISPER)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TALK invalid chat type (%u), skipping.", step.script->Talk.ChatType);
                    break;
                }

                if (step.script->Talk.Flags & SF_TALK_USE_PLAYER)
                    source = _GetScriptPlayerSourceOrTarget(source, target, step.script);
                else
                    source = _GetScriptCreatureSourceOrTarget(source, target, step.script);

                Unit* sourceUnit = source->ToUnit();
                //datalong 0=normal say, 1=whisper, 2=yell, 3=emote text
                switch (step.script->Talk.ChatType)
                {
                case CHAT_TYPE_SAY:
                    sourceUnit->Say(step.script->Talk.TextID, target);
                    break;
                case CHAT_TYPE_YELL:
                    sourceUnit->Yell(step.script->Talk.TextID, target);
                    break;
                case CHAT_TYPE_TEXT_EMOTE:
                case CHAT_TYPE_BOSS_EMOTE:
                    sourceUnit->TextEmote(step.script->Talk.TextID, target, step.script->Talk.ChatType == CHAT_TYPE_BOSS_EMOTE);
                    break;
                case CHAT_TYPE_WHISPER:
                case CHAT_TYPE_BOSS_WHISPER:
                {
                    Player* receiver = target ? target->ToPlayer() : nullptr;
                    if (!receiver)
                        TC_LOG_ERROR("scripts", "%s attempt to whisper to non-player unit, skipping.", step.script->GetDebugInfo().c_str());
                    else
                        sourceUnit->Whisper(step.script->Talk.TextID, receiver, step.script->Talk.ChatType == CHAT_TYPE_BOSS_WHISPER);
                    break;
                }
                case CHAT_MSG_RAID_BOSS_WHISPER:  //Not yet handled 
                default:
                    break;                              // must be already checked at load
                }
                break;
            }

            case SCRIPT_COMMAND_EMOTE:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_EMOTE call for NULL creature.");
                    break;
                }

                if(source->GetTypeId()!=TYPEID_UNIT)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_EMOTE call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                (source->ToCreature())->HandleEmoteCommand(step.script->Emote.EmoteID);
                break;
            case SCRIPT_COMMAND_FIELD_SET:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FIELD_SET call for NULL object.");
                    break;
                }
                if(step.script->FieldSet.FieldID <= OBJECT_FIELD_ENTRY || step.script->FieldSet.FieldID >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FIELD_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->FieldSet.FieldID, source->GetValuesCount(), source->GetTypeId());
                    break;
                }

                source->SetUInt32Value(step.script->FieldSet.FieldID, step.script->FieldSet.FieldValue);
                break;
            case SCRIPT_COMMAND_MOVE_TO:
            {
                if (!source)
                {
                    TC_LOG_ERROR("scripts", "SCRIPT_COMMAND_MOVE_TO call for NULL creature.");
                    break;
                }

                if (source->GetTypeId() != TYPEID_UNIT)
                {
                    TC_LOG_ERROR("scripts", "SCRIPT_COMMAND_MOVE_TO call for non-creature (TypeId: %u), skipping.", source->GetTypeId());
                    break;
                }
                Unit* unit = (Unit*)source;
                if (step.script->MoveTo.CalculateSpeed != 0)
                {
                    unit->SetWalk(true);
                    unit->GetMotionMaster()->MovePoint(0, step.script->MoveTo.DestX, step.script->MoveTo.DestY, step.script->MoveTo.DestZ);
                } else if (step.script->MoveTo.TravelTime != 0)
                {
                    float speed = unit->GetDistance(step.script->MoveTo.DestX, step.script->MoveTo.DestY, step.script->MoveTo.DestZ) / ((float)step.script->MoveTo.TravelTime * 0.001f);
                    unit->MonsterMoveWithSpeed(step.script->MoveTo.DestX, step.script->MoveTo.DestY, step.script->MoveTo.DestZ, speed);
                }
                else {
                    unit->NearTeleportTo(step.script->MoveTo.DestX, step.script->MoveTo.DestY, step.script->MoveTo.DestZ, unit->GetOrientation());
                }
                break;
            }
            case SCRIPT_COMMAND_FLAG_SET:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_SET call for NULL object.");
                    break;
                }
                if (step.script->FlagToggle.FieldID <= OBJECT_FIELD_ENTRY || step.script->FlagToggle.FieldID >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->FlagToggle.FieldID, source->GetValuesCount(), source->GetTypeId());
                    break;
                }

                source->SetFlag(step.script->FlagToggle.FieldID, step.script->FlagToggle.FieldValue);
                break;
            case SCRIPT_COMMAND_FLAG_REMOVE:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_REMOVE call for NULL object.");
                    break;
                }
                if(step.script->FlagToggle.FieldID <= OBJECT_FIELD_ENTRY || step.script->FlagToggle.FieldID >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_REMOVE call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->FlagToggle.FieldID, source->GetValuesCount(), source->GetTypeId());
                    break;
                }

                source->RemoveFlag(step.script->FlagToggle.FieldID, step.script->FlagToggle.FieldValue);
                break;

            case SCRIPT_COMMAND_TELEPORT_TO:
            {
                // accept player in any one from target/source arg
                if (!target && !source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TELEPORT_TO call for NULL object.");
                    break;
                }

                                                            // must be only Player
                if((!target || target->GetTypeId() != TYPEID_PLAYER) && (!source || source->GetTypeId() != TYPEID_PLAYER))
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TELEPORT_TO call for non-player (TypeIdSource: %u)(TypeIdTarget: %u), skipping.", source ? source->GetTypeId() : 0, target ? target->GetTypeId() : 0);
                    break;
                }

                Player* pSource = target && target->GetTypeId() == TYPEID_PLAYER ? target->ToPlayer() : source->ToPlayer();

                pSource->TeleportTo(step.script->TeleportTo.MapID, step.script->TeleportTo.DestX, step.script->TeleportTo.DestY, step.script->TeleportTo.DestZ, step.script->TeleportTo.Orientation);
                break;
            }

            case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            {
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for NULL world object.");
                    break;
                }

                WorldObject* summoner = dynamic_cast<WorldObject*>(source);
                if(!summoner)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for non-WorldObject (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                float x = step.script->TempSummonCreature.PosX;
                float y = step.script->TempSummonCreature.PosY;
                float z = step.script->TempSummonCreature.PosZ;
                float o = step.script->TempSummonCreature.Orientation;

                Creature* pCreature = summoner->SummonCreature(step.script->TempSummonCreature.CreatureEntry, x, y, z, o,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, step.script->TempSummonCreature.DespawnDelay);
                if (!pCreature)
                {
                    TC_LOG_ERROR("scripts","%s failed for creature (entry: %u).", step.script->GetDebugInfo().c_str(), step.script->TempSummonCreature.CreatureEntry);
                    break;
                }

                break;
            }

            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            {
                if(!step.script->RespawnGameobject.GOGuid)                  // gameobject not specified
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for NULL gameobject.");
                    break;
                }

                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for NULL world object.");
                    break;
                }

                WorldObject* summoner = dynamic_cast<WorldObject*>(source);

                if(!summoner)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for non-WorldObject (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                GameObject *go = nullptr;

                CellCoord p(Trinity::ComputeCellCoord(summoner->GetPositionX(), summoner->GetPositionY()));
                Cell cell(p);

                Trinity::GameObjectWithSpawnIdCheck go_check(*summoner,step.script->RespawnGameobject.GOGuid);
                Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck> checker(summoner, go, go_check);

                TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck>, GridTypeMapContainer > object_checker(checker);
                cell.Visit(p, object_checker, *summoner->GetMap(), *summoner, summoner->GetGridActivationRange());

                if ( !go )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT failed for gameobject (guid: %u).", step.script->RespawnGameobject.GOGuid);
                    break;
                }

                if( go->GetGoType()==GAMEOBJECT_TYPE_FISHINGNODE ||
                    go->GetGoType()==GAMEOBJECT_TYPE_DOOR        ||
                    go->GetGoType()==GAMEOBJECT_TYPE_BUTTON      ||
                    go->GetGoType()==GAMEOBJECT_TYPE_TRAP )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT can not be used with gameobject of type %u (guid: %u).", uint32(go->GetGoType()), step.script->RespawnGameobject.GOGuid);
                    break;
                }

                // Check that GO is not spawned
                if (!go->isSpawned())
                {
                    int32 nTimeToDespawn = std::max(5, int32(step.script->RespawnGameobject.DespawnDelay));
                    go->SetLootState(GO_READY);
                    go->SetRespawnTime(nTimeToDespawn);        //despawn object in ? seconds

                    if (!go->IsInWorld()) //sunstrider: A Gobjet may be already in world (but invisible) even when waiting for respawn
                        go->GetMap()->AddToMap(go);
                }
                break;
            }
            case SCRIPT_COMMAND_OPEN_DOOR:
            case SCRIPT_COMMAND_CLOSE_DOOR:
                _ScriptProcessDoor(source, target, step.script);
                break;
            case SCRIPT_COMMAND_QUEST_EXPLORED:
            {
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_QUEST_EXPLORED call for NULL source.");
                    break;
                }

                if(!target)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_QUEST_EXPLORED call for NULL target.");
                    break;
                }

                // when script called for item spell casting then target == (unit or GO) and source is player
                WorldObject* worldObject;
                Player* player;

                if(target->GetTypeId()==TYPEID_PLAYER)
                {
                    if(source->GetTypeId()!=TYPEID_UNIT && source->GetTypeId()!=TYPEID_GAMEOBJECT)
                    {
                        TC_LOG_ERROR("scripts","SCRIPT_COMMAND_QUEST_EXPLORED call for non-creature and non-gameobject (TypeId: %u), skipping.",source->GetTypeId());
                        break;
                    }

                    worldObject = (WorldObject*)source;
                    player = target->ToPlayer();
                }
                else
                {
                    if(target->GetTypeId()!=TYPEID_UNIT && target->GetTypeId()!=TYPEID_GAMEOBJECT)
                    {
                        TC_LOG_ERROR("scripts","SCRIPT_COMMAND_QUEST_EXPLORED call for non-creature and non-gameobject (TypeId: %u), skipping.",target->GetTypeId());
                        break;
                    }

                    if(source->GetTypeId()!=TYPEID_PLAYER)
                    {
                        TC_LOG_ERROR("scripts","SCRIPT_COMMAND_QUEST_EXPLORED call for non-player(TypeId: %u), skipping.",source->GetTypeId());
                        break;
                    }

                    worldObject = (WorldObject*)target;
                    player = source->ToPlayer();
                }

                // quest id and flags checked at script loading
                if( (worldObject->GetTypeId()!=TYPEID_UNIT || ((Unit*)worldObject)->IsAlive()) &&
                    (step.script->QuestExplored.Distance == 0 || worldObject->IsWithinDistInMap(player,float(step.script->QuestExplored.Distance))) )
                    player->AreaExploredOrEventHappens(step.script->QuestExplored.QuestID);
                else
                    player->FailQuest(step.script->QuestExplored.QuestID);

                break;
            }

            case SCRIPT_COMMAND_ACTIVATE_OBJECT:
            {
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_ACTIVATE_OBJECT must have source caster.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_ACTIVATE_OBJECT source caster isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                if(!target)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_ACTIVATE_OBJECT call for NULL gameobject.");
                    break;
                }

                if(target->GetTypeId()!=TYPEID_GAMEOBJECT)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_ACTIVATE_OBJECT call for non-gameobject (TypeId: %u), skipping.",target->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *go = (GameObject*)target;

                go->Use(caster);
                break;
            }

            case SCRIPT_COMMAND_REMOVE_AURA:
            {
                Object* cmdTarget = step.script->RemoveAura.Flags ? source : target;

                if(!cmdTarget)
                {
                    TC_LOG_ERROR("scripts", "SCRIPT_COMMAND_REMOVE_AURA call for NULL %s.", step.script->RemoveAura.Flags ? "source" : "target");
                    break;
                }

                if(!cmdTarget->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("scripts", "SCRIPT_COMMAND_REMOVE_AURA %s isn't unit (TypeId: %u), skipping.",step.script->RemoveAura.Flags ? "source" : "target",cmdTarget->GetTypeId());
                    break;
                }

                ((Unit*)cmdTarget)->RemoveAurasDueToSpell(step.script->RemoveAura.SpellID);
                break;
            }

            case SCRIPT_COMMAND_CAST_SPELL:
            {
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_CAST_SPELL must have source caster.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_CAST_SPELL source caster isn't unit (TypeId: %u), skipping.", source->GetTypeId());
                    break;
                }
                
                Unit* uSource = nullptr;
                Unit* uTarget = nullptr;
                // source/target cast spell at target/source (script->datalong2: 0: s->t 1: s->s 2: t->t 3: t->s
                switch (step.script->CastSpell.Flags)
                {
                case SF_CASTSPELL_SOURCE_TO_TARGET: // source -> target
                    uSource = source ? source->ToUnit() : nullptr;
                    uTarget = target ? target->ToUnit() : nullptr;
                    break;
                case SF_CASTSPELL_SOURCE_TO_SOURCE: // source -> source
                    uSource = source ? source->ToUnit() : nullptr;
                    uTarget = uSource;
                    break;
                case SF_CASTSPELL_TARGET_TO_TARGET: // target -> target
                    uSource = target ? target->ToUnit() : nullptr;
                    uTarget = uSource;
                    break;
                case SF_CASTSPELL_TARGET_TO_SOURCE: // target -> source
                    uSource = target ? target->ToUnit() : nullptr;
                    uTarget = source ? source->ToUnit() : nullptr;
                    break;
                case SF_CASTSPELL_SEARCH_CREATURE: // source -> creature with entry
                    uSource = source ? source->ToUnit() : nullptr;
                    uTarget = uSource ? uSource->FindNearestCreature(abs(step.script->CastSpell.CreatureEntry), step.script->CastSpell.SearchRadius) : nullptr;
                    break;
                }

                if (!uSource || !uSource->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("scripts", "%s no source unit found for spell %u", step.script->GetDebugInfo().c_str(), step.script->CastSpell.SpellID);
                    break;
                }

                if (!uTarget || !uTarget->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("scripts", "%s no target unit found for spell %u", step.script->GetDebugInfo().c_str(), step.script->CastSpell.SpellID);
                    break;
                }

                //TODO: when GO cast implemented, code below must be updated accordingly to also allow GO spell cast
                bool triggered = (step.script->CastSpell.Flags != SF_CASTSPELL_SEARCH_CREATURE) ?
                    step.script->CastSpell.CreatureEntry & SF_CASTSPELL_TRIGGERED :
                    step.script->CastSpell.CreatureEntry < 0;
                uSource->CastSpell(uTarget, step.script->CastSpell.Flags, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);

                break;
            }

            case SCRIPT_COMMAND_LOAD_PATH:
            {
                if(!source)
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_START_MOVE is tried to apply to NON-existing unit.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_START_MOVE source mover isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                if(!sWaypointMgr->GetPath(step.script->LoadPath.PathID))
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_START_MOVE source mover has an invalid path (%u), skipping.", step.script->LoadPath.PathID);
                    break;
                }

                dynamic_cast<Unit*>(source)->GetMotionMaster()->MovePath(step.script->LoadPath.PathID, step.script->LoadPath.IsRepeatable != 0);
                break;
            }

            case SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT:
            {
                if(!step.script->CallScript.CreatureEntry || !step.script->CallScript.ScriptID)
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_CALLSCRIPT calls invallid db_script_id or lowguid not present: skipping.");
                    break;
                }
                //our target
                Creature* creatureTarget = nullptr;
                auto creatureBounds = dynamic_cast<Unit*>(source)->GetMap()->GetCreatureBySpawnIdStore().equal_range(step.script->CallScript.CreatureEntry);
                if (creatureBounds.first != creatureBounds.second)
                {
                    // Prefer alive (last respawned) creature
                    auto creatureItr = std::find_if(creatureBounds.first, creatureBounds.second, [](Map::CreatureBySpawnIdContainer::value_type const& pair)
                    {
                        return pair.second->IsAlive();
                    });
                    creatureTarget = creatureItr != creatureBounds.second ? creatureItr->second : creatureBounds.first->second;
                }

                //TC_LOG_DEBUG("scripts","attempting to pass target...");
                if (!creatureTarget)
                {
                    TC_LOG_ERROR("scripts", "%s target was not found (entry: %u)", step.script->GetDebugInfo().c_str(), step.script->CallScript.CreatureEntry);
                    break;
                }   

                //TC_LOG_DEBUG("scripts","target passed");
                //Lets choose our ScriptMap map
                ScriptMapMap *datamap = nullptr;
                switch (step.script->CallScript.ScriptType)
                {
                case 1://QUEST END SCRIPTMAP
                    datamap = &sQuestEndScripts;
                    break;
                case 2://QUEST START SCRIPTMAP
                    datamap = &sQuestStartScripts;
                    break;
                case 3://SPELLS SCRIPTMAP
                    datamap = &sSpellScripts;
                    break;
                case 4://GAMEOBJECTS SCRIPTMAP
                    datamap = &sGameObjectScripts;
                    break;
                case 5://EVENTS SCRIPTMAP
                    datamap = &sEventScripts;
                    break;
                case 6://WAYPOINTS SCRIPTMAP
                    datamap = &sWaypointScripts;
                    break;
                default:
                    TC_LOG_ERROR("scripts", "SCRIPT_COMMAND_CALLSCRIPT ERROR: no scriptmap present... ignoring");
                    break;
                }
                //if no scriptmap present...
                if (!datamap)
                    break;

                uint32 script_id = step.script->CallScript.ScriptID;
                //insert script into schedule but do not start it
                ScriptsStart(*datamap, script_id, creatureTarget, nullptr, false);
                break;
            }

            case SCRIPT_COMMAND_PLAY_SOUND:
            {
                // Source must be WorldObject.
                if (WorldObject* object = _GetScriptWorldObject(source, true, step.script))
                {
                    // PlaySound.Flags bitmask: 0/1=anyone/target
                    Player* player = nullptr;
                    if (step.script->PlaySound.Flags & SF_PLAYSOUND_TARGET_PLAYER)
                    {
                        // Target must be Player.
                        player = _GetScriptPlayer(target, false, step.script);
                        if (!target)
                            break;
                    }

                    // PlaySound.Flags bitmask: 0/2=without/with distance dependent
                    if (step.script->PlaySound.Flags & SF_PLAYSOUND_DISTANCE_SOUND)
                        object->PlayDistanceSound(step.script->PlaySound.SoundID, player);
                    else
                        object->PlayDirectSound(step.script->PlaySound.SoundID, player);
                }
            }

            case SCRIPT_COMMAND_KILL:
            {
                if (!source || (source->ToCreature())->IsDead())
                    break;

                source->ToCreature()->KillSelf();

                switch (step.script->Kill.RemoveCorpse)
                {
                case 0: break; //return false not remove corpse
                case 1: (source->ToCreature())->RemoveCorpse(); break;
                }
                break;
            }

            case SCRIPT_COMMAND_KILL_CREDIT:
            {
                if (!source || ((Unit*)source)->GetTypeId() != TYPEID_PLAYER)
                    break;
                if (step.script->KillCredit.SpellID)
                    source->ToPlayer()->CastedCreatureOrGO(step.script->KillCredit.CreatureEntry, ObjectGuid::Empty, step.script->KillCredit.SpellID);
                else
                    source->ToPlayer()->KilledMonsterCredit(step.script->KillCredit.CreatureEntry, ObjectGuid::Empty);
                break;
            }

            case SCRIPT_COMMAND_SMART_SET_DATA:
            {
                if (!target && !source)
                    break;

                //use target, or source if no target
                Object* setObject = target ? target : source;

                if (Creature* c = setObject->ToCreature())
                {
                    if (!c->AI() || c->GetAIName() != SMARTAI_AI_NAME)
                        break;

                    SmartAI* smartAI = dynamic_cast<SmartAI*>(c->AI()); //dynamic cast to be extra safe
                    if (!smartAI)
                        break;

                    smartAI->SetData(step.script->SmartSetData.DataID, step.script->SmartSetData.Value);
                }
                else if (GameObject* gob = setObject->ToGameObject())
                {
                    if (!gob->AI() || gob->GetAIName() != SMARTAI_GOBJECT_AI_NAME)
                        break;

                    SmartGameObjectAI* smartAI = dynamic_cast<SmartGameObjectAI*>(gob->AI()); //dynamic cast to be extra safe
                    if (!smartAI)
                        break;

                    smartAI->SetData(step.script->SmartSetData.DataID, step.script->SmartSetData.Value);
                }
            }
            break;

            default:
                TC_LOG_ERROR("scripts","Unknown script command %u called.",step.script->command);
                break;
        }

        m_scriptSchedule.erase(iter);

        iter = m_scriptSchedule.begin();
        sMapMgr->DecreaseScheduledScriptCount();
    }
    return;
}


// Helpers for ScriptProcess method.
inline Player* Map::_GetScriptPlayerSourceOrTarget(Object* source, Object* target, ScriptInfo const* scriptInfo) const
{
    Player* player = nullptr;
    if (!source && !target)
        TC_LOG_ERROR("scripts", "%s source and target objects are NULL.", scriptInfo->GetDebugInfo().c_str());
    else
    {
        // Check target first, then source.
        if (target)
            player = target->ToPlayer();
        if (!player && source)
            player = source->ToPlayer();

        if (!player)
            TC_LOG_ERROR("scripts", "%s neither source nor target object is player (source: TypeId: %u, Entry: %u, GUID: %u; target: TypeId: %u, Entry: %u, GUID: %u), skipping.",
                scriptInfo->GetDebugInfo().c_str(),
                source ? source->GetTypeId() : 0, source ? source->GetEntry() : 0, source ? source->GetGUID().GetCounter() : 0,
                target ? target->GetTypeId() : 0, target ? target->GetEntry() : 0, target ? target->GetGUID().GetCounter() : 0);
    }
    return player;
}

inline Creature* Map::_GetScriptCreatureSourceOrTarget(Object* source, Object* target, ScriptInfo const* scriptInfo, bool bReverse) const
{
    Creature* creature = nullptr;
    if (!source && !target)
        TC_LOG_ERROR("scripts", "%s source and target objects are NULL.", scriptInfo->GetDebugInfo().c_str());
    else
    {
        if (bReverse)
        {
            // Check target first, then source.
            if (target)
                creature = target->ToCreature();
            if (!creature && source)
                creature = source->ToCreature();
        }
        else
        {
            // Check source first, then target.
            if (source)
                creature = source->ToCreature();
            if (!creature && target)
                creature = target->ToCreature();
        }

        if (!creature)
            TC_LOG_ERROR("scripts", "%s neither source nor target are creatures (source: TypeId: %u, Entry: %u, GUID: %u; target: TypeId: %u, Entry: %u, GUID: %u), skipping.",
                scriptInfo->GetDebugInfo().c_str(),
                source ? source->GetTypeId() : 0, source ? source->GetEntry() : 0, source ? source->GetGUID().GetCounter() : 0,
                target ? target->GetTypeId() : 0, target ? target->GetEntry() : 0, target ? target->GetGUID().GetCounter() : 0);
    }
    return creature;
}

inline Unit* Map::_GetScriptUnit(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const
{
    Unit* unit = nullptr;
    if (!obj)
        TC_LOG_ERROR("scripts", "%s %s object is NULL.", scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target");
    else if (!obj->isType(TYPEMASK_UNIT))
        TC_LOG_ERROR("scripts", "%s %s object is not unit (TypeId: %u, Entry: %u, GUID: %u), skipping.",
            scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target", obj->GetTypeId(), obj->GetEntry(), obj->GetGUID().GetCounter());
    else
    {
        unit = obj->ToUnit();
        if (!unit)
            TC_LOG_ERROR("scripts", "%s %s object could not be cast to unit.",
                scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target");
    }
    return unit;
}

inline Player* Map::_GetScriptPlayer(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const
{
    Player* player = nullptr;
    if (!obj)
        TC_LOG_ERROR("scripts", "%s %s object is NULL.", scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target");
    else
    {
        player = obj->ToPlayer();
        if (!player)
            TC_LOG_ERROR("scripts", "%s %s object is not a player (TypeId: %u, Entry: %u, GUID: %u).",
                scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target", obj->GetTypeId(), obj->GetEntry(), obj->GetGUID().GetCounter());
    }
    return player;
}

inline Creature* Map::_GetScriptCreature(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const
{
    Creature* creature = nullptr;
    if (!obj)
        TC_LOG_ERROR("scripts", "%s %s object is NULL.", scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target");
    else
    {
        creature = obj->ToCreature();
        if (!creature)
            TC_LOG_ERROR("scripts", "%s %s object is not a creature (TypeId: %u, Entry: %u, GUID: %u).", scriptInfo->GetDebugInfo().c_str(),
                isSource ? "source" : "target", obj->GetTypeId(), obj->GetEntry(), obj->GetGUID().GetCounter());
    }
    return creature;
}

inline WorldObject* Map::_GetScriptWorldObject(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const
{
    WorldObject* pWorldObject = nullptr;
    if (!obj)
        TC_LOG_ERROR("scripts", "%s %s object is NULL.",
            scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target");
    else
    {
        pWorldObject = dynamic_cast<WorldObject*>(obj);
        if (!pWorldObject)
            TC_LOG_ERROR("scripts", "%s %s object is not a world object (TypeId: %u, Entry: %u, GUID: %u).",
                scriptInfo->GetDebugInfo().c_str(), isSource ? "source" : "target", obj->GetTypeId(), obj->GetEntry(), obj->GetGUID().GetCounter());
    }
    return pWorldObject;
}

inline void Map::_ScriptProcessDoor(Object* source, Object* target, ScriptInfo const* scriptInfo) const
{
    bool bOpen = false;
    ObjectGuid::LowType guid = scriptInfo->ToggleDoor.GOGuid;
    int32 nTimeToToggle = std::max(15, int32(scriptInfo->ToggleDoor.ResetDelay));
    switch (scriptInfo->command)
    {
    case SCRIPT_COMMAND_OPEN_DOOR: bOpen = true; break;
    case SCRIPT_COMMAND_CLOSE_DOOR: break;
    default:
        TC_LOG_ERROR("scripts", "%s unknown command for _ScriptProcessDoor.", scriptInfo->GetDebugInfo().c_str());
        return;
    }
    if (!guid)
        TC_LOG_ERROR("scripts", "%s door guid is not specified.", scriptInfo->GetDebugInfo().c_str());
    else if (!source)
        TC_LOG_ERROR("scripts", "%s source object is NULL.", scriptInfo->GetDebugInfo().c_str());
    else if (!source->isType(TYPEMASK_UNIT))
        TC_LOG_ERROR("scripts", "%s source object is not unit (TypeId: %u, Entry: %u, GUID: %u), skipping.", scriptInfo->GetDebugInfo().c_str(),
            source->GetTypeId(), source->GetEntry(), source->GetGUID().GetCounter());
    else
    {
        WorldObject* wSource = dynamic_cast <WorldObject*> (source);
        if (!wSource)
            TC_LOG_ERROR("scripts", "%s source object could not be cast to world object (TypeId: %u, Entry: %u, GUID: %u), skipping.",
                scriptInfo->GetDebugInfo().c_str(), source->GetTypeId(), source->GetEntry(), source->GetGUID().GetCounter());
        else
        {
            GameObject* pDoor = _FindGameObject(wSource, guid);
            if (!pDoor)
                TC_LOG_ERROR("scripts", "%s gameobject was not found (guid: %u).", scriptInfo->GetDebugInfo().c_str(), guid);
            else if (pDoor->GetGoType() != GAMEOBJECT_TYPE_DOOR)
                TC_LOG_ERROR("scripts", "%s gameobject is not a door (GoType: %u, Entry: %u, GUID: %u).",
                    scriptInfo->GetDebugInfo().c_str(), pDoor->GetGoType(), pDoor->GetEntry(), pDoor->GetGUID().GetCounter());
            else if (bOpen == (pDoor->GetGoState() == GO_STATE_READY))
            {
                pDoor->UseDoorOrButton(nTimeToToggle);

                if (target && target->isType(TYPEMASK_GAMEOBJECT))
                {
                    GameObject* goTarget = target->ToGameObject();
                    if (goTarget && goTarget->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
                        goTarget->UseDoorOrButton(nTimeToToggle);
                }
            }
        }
    }
}

inline GameObject* Map::_FindGameObject(WorldObject* searchObject, ObjectGuid::LowType guid) const
{
    auto bounds = searchObject->GetMap()->GetGameObjectBySpawnIdStore().equal_range(guid);
    if (bounds.first == bounds.second)
        return nullptr;

    return bounds.first->second;
}