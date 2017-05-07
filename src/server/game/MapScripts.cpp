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
void Map::ScriptsStart(ScriptMapMap const& scripts, uint32 id, Object* source, Object* target, bool start)
{
    ///- Find the script map
    auto s = scripts.find(id);
    if (s == scripts.end())
        return;

    // prepare static data
    uint64 sourceGUID = source ? source->GetGUID() : (uint64)0; //some script commands doesn't have source
    uint64 targetGUID = target ? target->GetGUID() : (uint64)0;
    uint64 ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : (uint64)0;

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
		m_scriptSchedule.insert(ScriptScheduleMap::value_type(time_t(GameTime::GetGameTime() + iter.first), sa));
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
    uint64 sourceGUID = source->GetGUID();
    uint64 targetGUID = target->GetGUID();
    uint64 ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : (uint64)0;

    ScriptAction sa;
    sa.sourceGUID = sourceGUID;
    sa.targetGUID = targetGUID;
    sa.ownerGUID  = ownerGUID;

    sa.script = &script;
    //TC_LOG_INFO("SCRIPTCMD: Inserting script with source guid " UI64FMTD " target guid " UI64FMTD " owner guid " UI64FMTD " script id %u", sourceGUID, targetGUID, ownerGUID, script.id);
	m_scriptSchedule.insert(ScriptScheduleMap::value_type(time_t(GameTime::GetGameTime() + delay), sa));

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
	while (!m_scriptSchedule.empty() && (iter->first <= GameTime::GetGameTime()))
    {
        ScriptAction const& step = iter->second;

        Object* source = nullptr;

        if(step.sourceGUID)
        {
            switch(GUID_HIPART(step.sourceGUID))
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
                    source = source = GetGameObject(step.sourceGUID);
                    break;
                case HighGuid::Corpse:
                    source = GetCorpse(step.sourceGUID);
                    break;
                case HighGuid::Mo_Transport:
					GetTransport(step.sourceGUID);
					break;
                default:
                    TC_LOG_ERROR("scripts","*_script source with unsupported high guid value %u",GUID_HIPART(step.sourceGUID));
                    break;
            }
        }

        //if(source && !source->IsInWorld()) source = NULL;

        Object* target = nullptr;

        if(step.targetGUID)
        {
            switch(GUID_HIPART(step.targetGUID))
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
                    TC_LOG_ERROR("scripts","*_script source with unsupported high guid value %u",GUID_HIPART(step.targetGUID));
                    break;
            }
        }
        
        if (!source && !target)
            TC_LOG_ERROR("scripts","World::ScriptProcess: no source neither target for this script, if this is the last line before a crash, then you'd better return here.");

        //if(target && !target->IsInWorld()) target = NULL;

        if (GUID_HIPART(step.sourceGUID) == 16256 || GUID_HIPART(step.targetGUID) == 16256) {
            TC_LOG_ERROR("scripts","Source high GUID seems to be corrupted, skipping this script. Source GUID: " UI64FMTD ", target GUID: " UI64FMTD ", owner GUID: " UI64FMTD, step.sourceGUID, step.targetGUID, step.ownerGUID);
            if (m_scriptSchedule.size() == 1) {
                m_scriptSchedule.clear();
                break;
            }
        }

        switch (step.script->command)
        {
            case SCRIPT_COMMAND_TALK:
            {
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TALK call for NULL creature.");
                    break;
                }

                if(source->GetTypeId()!=TYPEID_UNIT)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TALK call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }
                if(step.script->datalong > 3)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TALK invalid chat type (%u), skipping.",step.script->datalong);
                    break;
                }

                uint64 unit_target = target ? target->GetGUID() : 0;

                //datalong 0=normal say, 1=whisper, 2=yell, 3=emote text
                switch(step.script->datalong)
                {
                    case 0:                                 // Say
                        (source->ToCreature())->old_Say(step.script->dataint, LANG_UNIVERSAL, unit_target);
                        break;
                    case 1:                                 // Whisper
                        if(!unit_target)
                        {
                            TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TALK attempt to whisper (%u) NULL, skipping.",step.script->datalong);
                            break;
                        }
                        (source->ToCreature())->old_Whisper(step.script->dataint,unit_target);
                        break;
                    case 2:                                 // Yell
                        (source->ToCreature())->old_Yell(step.script->dataint, LANG_UNIVERSAL, unit_target);
                        break;
                    case 3:                                 // Emote text
                        (source->ToCreature())->old_TextEmote(step.script->dataint, unit_target);
                        break;
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

                (source->ToCreature())->HandleEmoteCommand(step.script->datalong);
                break;
            case SCRIPT_COMMAND_FIELD_SET:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FIELD_SET call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FIELD_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->SetUInt32Value(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_MOVE_TO:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_MOVE_TO call for NULL creature.");
                    break;
                }

                if(source->GetTypeId()!=TYPEID_UNIT)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_MOVE_TO call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }
                ((Unit *)source)->MonsterMoveWithSpeed(step.script->x, step.script->y, step.script->z, step.script->datalong2 );
                ((Unit *)source)->GetMap()->CreatureRelocation((source->ToCreature()), step.script->x, step.script->y, step.script->z, 0);
                break;
            case SCRIPT_COMMAND_FLAG_SET:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_SET call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->SetFlag(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_FLAG_REMOVE:
                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_REMOVE call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_FLAG_REMOVE call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->RemoveFlag(step.script->datalong, step.script->datalong2);
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

                pSource->TeleportTo(step.script->datalong, step.script->x, step.script->y, step.script->z, step.script->o);
                break;
            }

            case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            {
                if(!step.script->datalong)                  // creature not specified
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for NULL creature.");
                    break;
                }

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

                float x = step.script->x;
                float y = step.script->y;
                float z = step.script->z;
                float o = step.script->o;

                Creature* pCreature = summoner->SummonCreature(step.script->datalong, x, y, z, o,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,step.script->datalong2);
                if (!pCreature)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_TEMP_SUMMON failed for creature (entry: %u).",step.script->datalong);
                    break;
                }

                break;
            }

            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            {
                if(!step.script->datalong)                  // gameobject not specified
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
                int32 time_to_despawn = step.script->datalong2<5 ? 5 : (int32)step.script->datalong2;

                CellCoord p(Trinity::ComputeCellCoord(summoner->GetPositionX(), summoner->GetPositionY()));
                Cell cell(p);

                Trinity::GameObjectWithSpawnIdCheck go_check(*summoner,step.script->datalong);
                Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck> checker(go,go_check);

                TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck>, GridTypeMapContainer > object_checker(checker);
                cell.Visit(p, object_checker, *summoner->GetMap());

                if ( !go )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }

                if( go->GetGoType()==GAMEOBJECT_TYPE_FISHINGNODE ||
                    go->GetGoType()==GAMEOBJECT_TYPE_FISHINGNODE ||
                    go->GetGoType()==GAMEOBJECT_TYPE_DOOR        ||
                    go->GetGoType()==GAMEOBJECT_TYPE_BUTTON      ||
                    go->GetGoType()==GAMEOBJECT_TYPE_TRAP )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT can not be used with gameobject of type %u (guid: %u).", uint32(go->GetGoType()), step.script->datalong);
                    break;
                }

                if( go->isSpawned() )
                    break;                                  //gameobject already spawned

                go->SetLootState(GO_READY);
                go->SetRespawnTime(time_to_despawn);        //despawn object in ? seconds

                go->GetMap()->AddToMap(go);
                break;
            }
            case SCRIPT_COMMAND_OPEN_DOOR:
            {
                if(!step.script->datalong)                  // door not specified
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_OPEN_DOOR call for NULL door.");
                    break;
                }

                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_OPEN_DOOR call for NULL unit.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))          // must be any Unit (creature or player)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_OPEN_DOOR call for non-unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *door = nullptr;
                int32 time_to_close = step.script->datalong2 < 15 ? 15 : (int32)step.script->datalong2;

                CellCoord p(Trinity::ComputeCellCoord(caster->GetPositionX(), caster->GetPositionY()));
                Cell cell(p);

                Trinity::GameObjectWithSpawnIdCheck go_check(*caster,step.script->datalong);
                Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck> checker(door,go_check);

                TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck>, GridTypeMapContainer > object_checker(checker);
                cell.Visit(p, object_checker, *caster->GetMap());

                if ( !door )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_OPEN_DOOR failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }
                if ( door->GetGoType() != GAMEOBJECT_TYPE_DOOR )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_OPEN_DOOR failed for non-door(GoType: %u).", door->GetGoType());
                    break;
                }

                if( !door->GetGoState() )
                    break;                                  //door already  open

                door->UseDoorOrButton(time_to_close);

                if(target && target->isType(TYPEMASK_GAMEOBJECT) && ((GameObject*)target)->GetGoType()==GAMEOBJECT_TYPE_BUTTON)
                    ((GameObject*)target)->UseDoorOrButton(time_to_close);
                break;
            }
            case SCRIPT_COMMAND_CLOSE_DOOR:
            {
                if(!step.script->datalong)                  // guid for door not specified
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_CLOSE_DOOR call for NULL door.");
                    break;
                }

                if(!source)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_CLOSE_DOOR call for NULL unit.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))          // must be any Unit (creature or player)
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_CLOSE_DOOR call for non-unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *door = nullptr;
                int32 time_to_open = step.script->datalong2 < 15 ? 15 : (int32)step.script->datalong2;

                CellCoord p(Trinity::ComputeCellCoord(caster->GetPositionX(), caster->GetPositionY()));
                Cell cell(p);

                Trinity::GameObjectWithSpawnIdCheck go_check(*caster,step.script->datalong);
                Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck> checker(door,go_check);

                TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithSpawnIdCheck>, GridTypeMapContainer > object_checker(checker);
                cell.Visit(p, object_checker, *caster->GetMap());

                if ( !door )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_CLOSE_DOOR failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }
                if ( door->GetGoType() != GAMEOBJECT_TYPE_DOOR )
                {
                    TC_LOG_ERROR("scripts","SCRIPT_COMMAND_CLOSE_DOOR failed for non-door(GoType: %u).", door->GetGoType());
                    break;
                }

                if( door->GetGoState() )
                    break;                                  //door already closed

                door->UseDoorOrButton(time_to_open);

                if(target && target->isType(TYPEMASK_GAMEOBJECT) && ((GameObject*)target)->GetGoType()==GAMEOBJECT_TYPE_BUTTON)
                    ((GameObject*)target)->UseDoorOrButton(time_to_open);

                break;
            }
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
                    (step.script->datalong2==0 || worldObject->IsWithinDistInMap(player,float(step.script->datalong2))) )
                    player->AreaExploredOrEventHappens(step.script->datalong);
                else
                    player->FailQuest(step.script->datalong);

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
                Object* cmdTarget = step.script->datalong2 ? source : target;

                if(!cmdTarget)
                {
                    TC_LOG_ERROR("scripts", "SCRIPT_COMMAND_REMOVE_AURA call for NULL %s.", step.script->datalong2 ? "source" : "target");
                    break;
                }

                if(!cmdTarget->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("scripts", "SCRIPT_COMMAND_REMOVE_AURA %s isn't unit (TypeId: %u), skipping.",step.script->datalong2 ? "source" : "target",cmdTarget->GetTypeId());
                    break;
                }

                ((Unit*)cmdTarget)->RemoveAurasDueToSpell(step.script->datalong);
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
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CAST_SPELL source caster isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Object* cmdTarget = step.script->datalong2 ? source : target;

                if(!cmdTarget)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CAST_SPELL (ID: %u) call for NULL %s.",step.script->id, step.script->datalong2 ? "source" : "target");
                    break;
                }

                if(!cmdTarget->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CAST_SPELL %s isn't unit (TypeId: %u), skipping.",step.script->datalong2 ? "source" : "target",cmdTarget->GetTypeId());
                    break;
                }

                Unit* spellTarget = (Unit*)cmdTarget;

                //TODO: when GO cast implemented, code below must be updated accordingly to also allow GO spell cast
                ((Unit*)source)->CastSpell(spellTarget,step.script->datalong,false);

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

                if(!sWaypointMgr->GetPath(step.script->datalong))
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_START_MOVE source mover has an invalid path (%u), skipping.", step.script->datalong);
                    break;
                }

                dynamic_cast<Unit*>(source)->GetMotionMaster()->MovePath(step.script->datalong);
                break;
            }

            case SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT:
            {
                if(!step.script->datalong || !step.script->datalong2)
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_CALLSCRIPT calls invallid db_script_id or lowguid not present: skipping.");
                    break;
                }
                //our target
                Creature* creatureTarget = nullptr;
                auto creatureBounds = dynamic_cast<Unit*>(source)->GetMap()->GetCreatureBySpawnIdStore().equal_range(step.script->datalong);
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
                    TC_LOG_ERROR("scripts", "%s target was not found (entry: %u)", step.script->GetDebugInfo().c_str(), step.script->datalong);
                    break;
                }   

                //TC_LOG_DEBUG("scripts","target passed");
                //Lets choose our ScriptMap map
                ScriptMapMap *datamap = nullptr;
                switch (step.script->dataint)
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

                uint32 script_id = step.script->datalong2;
                //insert script into schedule but do not start it
                ScriptsStart(*datamap, script_id, creatureTarget, nullptr, false);
                break;
            }

            case SCRIPT_COMMAND_PLAY_SOUND:
            {
                if (!source)
                    break;
                //datalong sound_id, datalong2 onlyself
                Player* target = step.script->datalong2 ? source->ToPlayer() : nullptr;
                ((WorldObject*)source)->PlayDirectSound(step.script->datalong, target);
                break;
            }

            case SCRIPT_COMMAND_KILL:
            {
                if (!source || (source->ToCreature())->IsDead())
                    break;

                (source->ToCreature())->DealDamage((source->ToCreature()), (source->ToCreature())->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);

                switch (step.script->dataint)
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
                if (step.script->datalong2)
                    source->ToPlayer()->CastedCreatureOrGO(step.script->datalong, 0, step.script->datalong2);
                else
                    source->ToPlayer()->KilledMonsterCredit(step.script->datalong, 0);
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

                    smartAI->SetData(step.script->datalong, step.script->datalong2);
                }
                else if (GameObject* gob = setObject->ToGameObject())
                {
                    if (!gob->AI() || gob->GetAIName() != SMARTAI_GOBJECT_AI_NAME)
                        break;

                    SmartGameObjectAI* smartAI = dynamic_cast<SmartGameObjectAI*>(gob->AI()); //dynamic cast to be extra safe
                    if (!smartAI)
                        break;

                    smartAI->SetData(step.script->datalong, step.script->datalong2);
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
