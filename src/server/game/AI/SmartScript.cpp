
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "ObjectDefines.h"
#include "GridDefines.h"
#include "GridNotifiers.h"
#include "SpellMgr.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "ChatTextBuilder.h"
#include "InstanceScript.h"
#include "GossipDef.h"
#include "ScriptedCreature.h"
#include "SmartScript.h"
#include "SmartAI.h"
#include "GameEventMgr.h"
#include "ScriptedGossip.h"
#include "Transport.h"
#include "ScriptedCreature.h"
#include "CreatureTextMgr.h"
#include "Language.h"

/*
class TrinityStringTextBuilder 
{
    public:
        TrinityStringTextBuilder(WorldObject* obj, ChatMsg msgtype, int32 id, uint32 language, WorldObject* target)
            : _source(obj), _msgType(msgtype), _textId(id), _language(language), _target(target)
        {
        }

        size_t operator()(WorldPacket* data, LocaleConstant locale) const
        {
            std::string text = sObjectMgr->GetTrinityString(_textId, locale);
            return 0; //ChatHandler::BuildChatPacket(*data, _msgType, Language(_language), _source, _target, text, 0, "", locale);
        }

        WorldObject* _source;
        ChatMsg _msgType;
        int32 _textId;
        uint32 _language;
        WorldObject* _target;
};
*/
SmartScript::SmartScript() :
    go(nullptr),
    me(nullptr),
    trigger(nullptr),
    mEventPhase(0),
    mStoredPhase(0),
    mEventTemplatePhase(0),
    mPathId(0),
    mTextTimer(0),
    mLastTextID(0),
    mUseTextTimer(false),
    mTalkerEntry(0),
    mTemplate(SMARTAI_TEMPLATE_BASIC),
    meOrigGUID(0),
    goOrigGUID(0),
    mLastInvoker(0),
    mScriptType(SMART_SCRIPT_TYPE_CREATURE),
    isProcessingTimedActionList(false),
    mLastProcessedActionId(0)
{
}

SmartScript::~SmartScript()
{
    mCounterList.clear();
}

void SmartScript::OnReset()
{
    SetPhase(0);
    ResetBaseObject();
    for (auto & mEvent : mEvents)
    {
        if (!(mEvent.event.event_flags & SMART_EVENT_FLAG_DONT_RESET))
        {
            InitTimer(mEvent);
            mEvent.runOnce = false;
        }
    }
    ProcessEventsFor(SMART_EVENT_RESET);
    mLastInvoker = 0;
    mCounterList.clear();
}

void SmartScript::ProcessEventsFor(SMART_EVENT e, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob)
{
    for (auto& mEvent : mEvents)
    {
        SMART_EVENT eventType = SMART_EVENT(mEvent.GetEventType());
        if (eventType == SMART_EVENT_LINK)//special handling
            continue;

        if (eventType == e/* && (!i->event.event_phase_mask || IsInPhase(i->event.event_phase_mask)) && !(i->event.event_flags & SMART_EVENT_FLAG_NOT_REPEATABLE && i->runOnce)*/)
        {
            if (sConditionMgr->IsObjectMeetingSmartEventConditions(mEvent.entryOrGuid, mEvent.event_id, mEvent.source_type, unit, GetBaseObject()))
                ProcessEvent(mEvent, unit, var0, var1, bvar, spell, gob);
        }
    }
}

void SmartScript::ProcessAction(SmartScriptHolder& e, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob)
{
    //calc random
    if (e.GetEventType() != SMART_EVENT_LINK && e.event.event_chance < 100 && e.event.event_chance)
    {
        uint32 rnd = urand(1, 100);
        if (e.event.event_chance <= rnd)
            return;
    }
    e.runOnce = true;//used for repeat check

    if (unit)
        mLastInvoker = unit->GetGUID();

    if (Unit* tempInvoker = GetLastInvoker())
        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: Invoker: %s (guidlow: %u)", tempInvoker->GetName().c_str(), tempInvoker->GetGUIDLow());
    
    mLastProcessedActionId = e.event_id;

    ObjectVector targets;
    GetTargets(targets, e, unit);

    switch (e.GetActionType())
    {
        case SMART_ACTION_TALK:
        {
            Creature* talker = e.target.type == 0 ? me : nullptr;
            Unit* talkTarget = nullptr;

            for (WorldObject* target : targets)
            {
                if (IsCreature(target) && !target->ToCreature()->IsPet()) // Prevented sending text to pets.
                {
                    if (e.action.talk.useTalkTarget)
                    {
                        talker = me;
                        talkTarget = target->ToCreature();
                    } else
                        talker = target->ToCreature();
                    break;
                }
                else if (IsPlayer(target))
                {
                    talker = me;
                    talkTarget = target->ToPlayer();
                    break;
                }
            }

            /* sunstrider: why this check? SMART_TARGET_NONE should be a valid target for most talk types
            if (!talkTarget)
                return;
            */

            if (!talker)
                break;

            mTalkerEntry = talker->GetEntry();
            mLastTextID = e.action.talk.textGroupID;

            mUseTextTimer = true;
            mTextTimer = sCreatureTextMgr->SendChat(talker, uint8(e.action.talk.textGroupID), talkTarget);
            //if action specified a duration, erase the default duration
            if(e.action.talk.duration)
                mTextTimer = e.action.talk.duration;
            
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_TALK: talker: %s (GuidLow: %u), textGuid: %u",
                talker->GetName().c_str(), talker->GetGUIDLow(), talkTarget ? talkTarget->GetGUIDLow() : 0);
            break;
        }
        case SMART_ACTION_SIMPLE_TALK:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    sCreatureTextMgr->SendChat((target)->ToCreature(), uint8(e.action.talk.textGroupID), IsPlayer(GetLastInvoker()) ? GetLastInvoker() : nullptr);
                else if (IsPlayer(target) && me)
                {
                    Unit* templastInvoker = GetLastInvoker();
                    sCreatureTextMgr->SendChat(me, uint8(e.action.talk.textGroupID), IsPlayer(templastInvoker) ? templastInvoker : nullptr, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, (target)->ToPlayer());
                }
                TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_SIMPLE_TALK: talker: %s (GuidLow: %u), textGroupId: %u",
                    target->GetName().c_str(), (target)->GetGUIDLow(), uint8(e.action.talk.textGroupID));
            }

            break;
        }
        case SMART_ACTION_PLAY_EMOTE:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    target->ToUnit()->HandleEmoteCommand(e.action.emote.emote);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_PLAY_EMOTE: target: %s (GuidLow: %u), emote: %u",
                        target->GetName().c_str(), target->GetGUIDLow(), e.action.emote.emote);
                }
            }

            break;
        }
        case SMART_ACTION_SOUND:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    if (e.action.sound.distance == 1)
                        target->PlayDistanceSound(e.action.sound.sound, e.action.sound.onlySelf ? target->ToPlayer() : nullptr);
                    else
                        target->PlayDirectSound(e.action.sound.sound, e.action.sound.onlySelf ? target->ToPlayer() : nullptr);

                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_SOUND: target: %s (GuidLow: %u), sound: %u, onlyself: %u",
                        target->GetName().c_str(), target->GetGUIDLow(), e.action.sound.sound, e.action.sound.onlySelf);
                }
            }

            break;
        }
        case SMART_ACTION_SET_FACTION:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                {
                    if (e.action.faction.factionID)
                    {
                        target->ToCreature()->SetFaction(e.action.faction.factionID);
                        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_SET_FACTION: Creature entry %u, GuidLow %u set faction to %u",
                            target->GetEntry(), target->GetGUIDLow(), e.action.faction.factionID);
                    }
                    else
                    {
                        if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(me->GetEntry()))
                        {
                            if (target->ToCreature()->GetFaction() != ci->faction)
                            {
                                target->ToCreature()->SetFaction(ci->faction);
                                TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_SET_FACTION: Creature entry %u, GuidLow %u set faction to %u",
                                    target->GetEntry(), target->GetGUIDLow(), ci->faction);
                            }
                        }
                    }
                }
            }
            break;
        }
        case SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL:
        {
            for (WorldObject* target : targets)
            {
                if (!IsCreature(target))
                    continue;

                if (e.action.morphOrMount.creature || e.action.morphOrMount.model)
                {
                    //set model based on entry from creature_template
                    if (e.action.morphOrMount.creature)
                    {
                        if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(e.action.morphOrMount.creature))
                        {
                            uint32 displayId = ObjectMgr::ChooseDisplayId(ci);
                            target->ToCreature()->SetDisplayId(displayId);
                            TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL: Creature entry %u, GuidLow %u set displayid to %u",
                                target->GetEntry(), target->GetGUIDLow(), displayId);
                        }
                    }
                    //if no param1, then use value from param2 (modelId)
                    else
                    {
                        target->ToCreature()->SetDisplayId(e.action.morphOrMount.model);
                        TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL: Creature entry %u, GuidLow %u set displayid to %u",
                            target->GetEntry(), target->GetGUIDLow(), e.action.morphOrMount.model);
                    }
                }
                else
                {
                    target->ToCreature()->DeMorph();
                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_MORPH_TO_ENTRY_OR_MODEL: Creature entry %u, GuidLow %u demorphs.",
                        target->GetEntry(), target->GetGUIDLow());
                }
            }
        }
        case SMART_ACTION_FAIL_QUEST:
        {
            for (WorldObject* target : targets)
            {
                if (IsPlayer(target))
                {
                    target->ToPlayer()->FailQuest(e.action.quest.quest);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_FAIL_QUEST: Player guidLow %u fails quest %u",
                        target->GetGUIDLow(), e.action.quest.quest);
                }
            }
            break;
        }
        case SMART_ACTION_ADD_QUEST:
        {
            for (WorldObject* target : targets)
            {
                if (IsPlayer(target))
                {
                    if (Quest const* q = sObjectMgr->GetQuestTemplate(e.action.quest.quest))
                    {
                        target->ToPlayer()->AddQuestAndCheckCompletion(q, nullptr);
                        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_ADD_QUEST: Player guidLow %u add quest %u",
                            target->GetGUIDLow(), e.action.quest.quest);
                    }
                }
            }

            break;
        }
        case SMART_ACTION_SET_REACT_STATE:
        {
            for (WorldObject* target : targets)
            {
                if (!IsCreature(target))
                    continue;

                target->ToCreature()->SetReactState(ReactStates(e.action.react.state));
            }

            break;
        }
        case SMART_ACTION_RANDOM_EMOTE:
        {
            uint32 emotes[SMART_ACTION_PARAM_COUNT];
            emotes[0] = e.action.randomEmote.emote1;
            emotes[1] = e.action.randomEmote.emote2;
            emotes[2] = e.action.randomEmote.emote3;
            emotes[3] = e.action.randomEmote.emote4;
            emotes[4] = e.action.randomEmote.emote5;
            emotes[5] = e.action.randomEmote.emote6;
            uint32 temp[SMART_ACTION_PARAM_COUNT];
            uint32 count = 0;
            for (uint32 emote : emotes)
            {
                if (emote)
                {
                    temp[count] = emote;
                    ++count;
                }
            }

            if (count == 0)
            {
                break;
            }

            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    uint32 emote = temp[urand(0, count - 1)];
                    target->ToUnit()->HandleEmoteCommand(emote);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_RANDOM_EMOTE: Creature guidLow %u handle random emote %u",
                        target->GetGUIDLow(), emote);
                }
            }

            break;
        }
        case SMART_ACTION_THREAT_ALL_PCT:
        {
            if (!me) 
                return;

            std::list<HostileReference*>& threatList = me->GetThreatManager().getThreatList();
            for (auto & i : threatList)
                if (Unit* target = ObjectAccessor::GetUnit(*me,i->getUnitGuid()))
                {
                    me->GetThreatManager().modifyThreatPercent(target, e.action.threatPCT.threatINC ? (int32)e.action.threatPCT.threatINC : -(int32)e.action.threatPCT.threatDEC);
                     TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_THREAT_ALL_PCT: Creature guidLow %u modify threat for unit %u, value %i",
                        me->GetGUIDLow(), target->GetGUIDLow(), e.action.threatPCT.threatINC ? (int32)e.action.threatPCT.threatINC : -(int32)e.action.threatPCT.threatDEC);
                }
            break;
        }
        case SMART_ACTION_THREAT_SINGLE_PCT:
        {
            if (!me)
                break;

            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    me->GetThreatManager().modifyThreatPercent(target->ToUnit(), e.action.threatPCT.threatINC ? (int32)e.action.threatPCT.threatINC : -(int32)e.action.threatPCT.threatDEC);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_THREAT_SINGLE_PCT: Creature guidLow %u modify threat for unit %u, value %i",
                        me->GetGUIDLow(), target->GetGUIDLow(), e.action.threatPCT.threatINC ? (int32)e.action.threatPCT.threatINC : -(int32)e.action.threatPCT.threatDEC);
                }
            }

            break;
        }
        case SMART_ACTION_CALL_AREAEXPLOREDOREVENTHAPPENS:
        {
            for (WorldObject* target : targets)
            {
                if (IsPlayer(target))
                {
                    target->ToPlayer()->GroupEventHappens(e.action.quest.quest, me);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_CALL_AREAEXPLOREDOREVENTHAPPENS: Player guidLow %u credited quest %u",
                        target->GetGUIDLow(), e.action.quest.quest);
                }
            }

            break;
        }
        case SMART_ACTION_CAST:
        {
            if (e.action.cast.targetsLimit > 0 && targets.size() > e.action.cast.targetsLimit)
                Trinity::Containers::RandomResize(targets, e.action.cast.targetsLimit);

            for (WorldObject* target : targets)
            {
                if (go)
                {
                    // may be nullptr
                    go->CastSpell(target->ToUnit(), e.action.cast.spell);
                }

                if (!IsUnit(target))
                    continue;

                    if (!(e.action.cast.castFlags & SMARTCAST_AURA_NOT_PRESENT) || !target->ToUnit()->HasAuraEffect(e.action.cast.spell))
                    {
                        TriggerCastFlags triggerFlag = TRIGGERED_NONE;
                        if (e.action.cast.castFlags & SMARTCAST_TRIGGERED)
                        {
                            if (e.action.cast.triggerFlags)
                                triggerFlag = TriggerCastFlags(e.action.cast.triggerFlags);
                            else
                                triggerFlag = TRIGGERED_FULL_MASK;
                        }

                        if (me) //creature case
                        {
                            if (e.action.cast.castFlags & SMARTCAST_INTERRUPT_PREVIOUS)
                                me->InterruptNonMeleeSpells(false);

                            uint32 result = me->CastSpell(target->ToUnit(), e.action.cast.spell, triggerFlag);
                            if (e.action.cast.castFlags & SMARTCAST_COMBAT_MOVE)
                            {
                                // If cast flag SMARTCAST_COMBAT_MOVE is set combat movement will not be allowed
                                // unless target is outside spell range, out of mana, or LOS.

                                bool _allowMove = false;
                                if(result != SPELL_CAST_OK)
                                    _allowMove = true;

                                CAST_AI(SmartAI, me->AI())->SetCombatMove(_allowMove);
                            }

                            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_CAST:: Creature %u casts spell %u on target %u with castflags %u with result %u",
                                me->GetGUIDLow(), e.action.cast.spell, (target)->GetGUIDLow(), e.action.cast.castFlags, result);
                        }
                        else if (go) 
                        { //gameobject case
                            uint32 result = go->CastSpell(target->ToUnit(), e.action.cast.spell, triggerFlag);
                            TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_CAST:: Gameobject %u casts spell %u on target %u with result %u",
                                go->GetGUIDLow(), e.action.cast.spell, target->GetGUIDLow(), result);
                        }
                    }
                    else
                        TC_LOG_DEBUG("scripts.ai","Spell %u not cast because it has flag SMARTCAST_AURA_NOT_PRESENT and the target (Guid: " UI64FMTD " Entry: %u Type: %u) already has the aura", e.action.cast.spell, target->GetGUID(), target->GetEntry(), uint32(target->GetTypeId()));
            }

            break;
        }
        case SMART_ACTION_INVOKER_CAST:
        {
            Unit* tempLastInvoker = GetLastInvoker(unit);
            if (!tempLastInvoker)
                break;

            if (e.action.cast.targetsLimit > 0 && targets.size() > e.action.cast.targetsLimit)
                Trinity::Containers::RandomResize(targets, e.action.cast.targetsLimit);

            for (WorldObject* target : targets)
            {
                if (!IsUnit(target))
                    continue;

                if (!(e.action.cast.castFlags & SMARTCAST_AURA_NOT_PRESENT) || !target->ToUnit()->HasAuraEffect(e.action.cast.spell))
                {
                    if (e.action.cast.castFlags & SMARTCAST_INTERRUPT_PREVIOUS)
                        tempLastInvoker->InterruptNonMeleeSpells(false);

                    TriggerCastFlags triggerFlag = TRIGGERED_NONE;
                    if (e.action.cast.castFlags & SMARTCAST_TRIGGERED)
                    {
                        if (e.action.cast.triggerFlags)
                            triggerFlag = TriggerCastFlags(e.action.cast.triggerFlags);
                        else
                            triggerFlag = TRIGGERED_FULL_MASK;
                    }

                    tempLastInvoker->CastSpell((target)->ToUnit(), e.action.cast.spell, triggerFlag);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_INVOKER_CAST: Invoker %u casts spell %u on target %u with castflags %u",
                        tempLastInvoker->GetGUIDLow(), e.action.cast.spell, target->GetGUIDLow(), e.action.cast.castFlags);
                }
                else
                    TC_LOG_DEBUG("scripts.ai","Spell %u not cast because it has flag SMARTCAST_AURA_NOT_PRESENT and the target (Guid: " UI64FMTD " Entry: %u Type: %u) already has the aura", e.action.cast.spell, target->GetGUID(), target->GetEntry(), uint32(target->GetTypeId()));
            }

            break;
        }
        case SMART_ACTION_ADD_AURA:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    target->ToUnit()->AddAura(e.action.cast.spell, target->ToUnit());
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_ADD_AURA: Adding aura %u to unit %u",
                        e.action.cast.spell, target->GetGUIDLow());
                }
            }

            break;
        }
        case SMART_ACTION_ACTIVATE_GOBJECT:
        {
            for (WorldObject* target : targets)
            {
                if (IsGameObject(target))
                {
                    // Activate
                    target->ToGameObject()->SetLootState(GO_READY);
                    target->ToGameObject()->UseDoorOrButton(0, false, unit);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_ACTIVATE_GOBJECT. Gameobject %u (entry: %u) activated",
                        target->GetGUIDLow(), target->GetEntry());
                }
            }

            break;
        }
        case SMART_ACTION_RESET_GOBJECT:
        {
            for (WorldObject* target : targets)
            {
                if (IsGameObject(target))
                {
                    target->ToGameObject()->ResetDoorOrButton();
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_RESET_GOBJECT. Gameobject %u (entry: %u) reset",
                        target->GetGUIDLow(), target->GetEntry());
                }
            }

            break;
        }
        case SMART_ACTION_SET_EMOTE_STATE:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    target->ToUnit()->SetEmoteState(e.action.emote.emote);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_SET_EMOTE_STATE. Unit %u set emotestate to %u",
                        target->GetGUIDLow(), e.action.emote.emote);
                }
            }


            break;
        }
        case SMART_ACTION_SET_UNIT_FLAG:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    if (!e.action.unitFlag.type)
                    {
                        target->ToUnit()->SetFlag(UNIT_FIELD_FLAGS, e.action.unitFlag.flag);
                        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_SET_UNIT_FLAG. Unit %u added flag %u to UNIT_FIELD_FLAGS",
                        target->GetGUIDLow(), e.action.unitFlag.flag);
                    }
                    else
                    {
                        target->ToUnit()->SetFlag(UNIT_FIELD_FLAGS_2, e.action.unitFlag.flag);
                        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_SET_UNIT_FLAG. Unit %u added flag %u to UNIT_FIELD_FLAGS_2",
                        target->GetGUIDLow(), e.action.unitFlag.flag);
                    }
                }
            }


            break;
        }
        case SMART_ACTION_REMOVE_UNIT_FLAG:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    if (!e.action.unitFlag.type)
                    {
                        target->ToUnit()->RemoveFlag(UNIT_FIELD_FLAGS, e.action.unitFlag.flag);
                        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_REMOVE_UNIT_FLAG. Unit %u removed flag %u to UNIT_FIELD_FLAGS",
                        target->GetGUIDLow(), e.action.unitFlag.flag);
                    }
                    else
                    {
                        target->ToUnit()->RemoveFlag(UNIT_FIELD_FLAGS_2, e.action.unitFlag.flag);
                        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_REMOVE_UNIT_FLAG. Unit %u removed flag %u to UNIT_FIELD_FLAGS_2",
                        target->GetGUIDLow(), e.action.unitFlag.flag);
                    }
                }
            }


            break;
        }
        case SMART_ACTION_AUTO_ATTACK:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetAutoAttack(e.action.autoAttack.attack);
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_AUTO_ATTACK: Creature: %u bool on = %u",
                me->GetGUIDLow(), e.action.autoAttack.attack);
            break;
        }
        case SMART_ACTION_ALLOW_COMBAT_MOVEMENT:
        {
            if (!IsSmart())
                break;

            bool move = e.action.combatMove.move;
            CAST_AI(SmartAI, me->AI())->SetCombatMove(move);
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_ALLOW_COMBAT_MOVEMENT: Creature %u bool on = %u",
                me->GetGUIDLow(), e.action.combatMove.move);
            break;
        }
        case SMART_ACTION_SET_EVENT_PHASE:
        {
            for (WorldObject* target : targets)
            {
                if(!IsUnit(target))
                    break;

                Creature* c = target->ToCreature();
                if(!IsSmart(c))
                    break;

                ENSURE_AI(SmartAI, c->AI())->GetScript()->SetPhase(e.action.setEventPhase.phase);
                TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_SET_EVENT_PHASE: Creature %u set event phase %u",
                    target->GetGUIDLow(), e.action.setEventPhase.phase);
            }

            break;
        }
        case SMART_ACTION_INC_EVENT_PHASE:
        {
            for (WorldObject* target : targets)
            {
                if(!IsUnit(target))
                    break;

                Creature* c = target->ToCreature();
                if(!c || !IsSmart(c))
                    break;
                    
                ENSURE_AI(SmartAI, c->AI())->GetScript()->IncPhase(e.action.incEventPhase.inc);
                ENSURE_AI(SmartAI, c->AI())->GetScript()->DecPhase(e.action.incEventPhase.dec);
                    
                TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_INC_EVENT_PHASE: Creature %u inc event phase by %u, "
                    "decrease by %u", GetBaseObject()->GetGUIDLow(), e.action.incEventPhase.inc, e.action.incEventPhase.dec);
            }

            break;
        }
        case SMART_ACTION_EVADE:
        {
            if (!me)
                break;

            me->AI()->EnterEvadeMode();
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_EVADE: Creature %u EnterEvadeMode", me->GetGUIDLow());
            break;
        }
        case SMART_ACTION_FLEE_FOR_ASSIST: //scripts.ai : Emote
        {
            if (!me)
                break;

            me->DoFleeToGetAssistance();

            if (e.action.fleeAssist.withEmote)
            {
                Trinity::BroadcastTextBuilder builder(me, CHAT_MSG_MONSTER_EMOTE, BROADCAST_TEXT_FLEE_FOR_ASSIST);
                sCreatureTextMgr->SendChatPacket(me, builder, CHAT_MSG_MONSTER_EMOTE);
            }
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_FLEE_FOR_ASSIST: Creature %u DoFleeToGetAssistance", me->GetGUIDLow());
            break;
        }
        case SMART_ACTION_CALL_GROUPEVENTHAPPENS:
        {
            //sunstrider: apply to both invoker unit and targets, why should we ignore targets?
            if (unit)
                targets.push_back(unit);

            for (WorldObject* target : targets)
            {
                if (!IsUnit(target))
                    continue;
                // If invoker was pet or charm
                Player* player = target->ToUnit()->GetCharmerOrOwnerPlayerOrPlayerItself();
                if (!player)
                    continue;

                if (player && GetBaseObject())
                {
                    player->GroupEventHappens(e.action.quest.quest, GetBaseObject());
                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction: SMART_ACTION_CALL_GROUPEVENTHAPPENS: Player %u, group credit for quest %u",
                        unit->GetGUIDLow(), e.action.quest.quest);
                }

#ifdef LICH_KING
                // Special handling for vehicles
                if (Vehicle* vehicle = target->GetVehicleKit())
                    for (SeatMap::iterator it = vehicle->Seats.begin(); it != vehicle->Seats.end(); ++it)
                        if (Player* player = ObjectAccessor::GetPlayer(*(target), it->second.Passenger.Guid))
                            player->GroupEventHappens(e.action.quest.quest, GetBaseObject());
#endif
            }
            break;
        }

        case SMART_ACTION_COMBAT_STOP:
        {
            if (!me)
                break;

            me->CombatStop(true);
            TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_COMBAT_STOP: %u CombatStop", me->GetGUIDLow());
            break;
        } 
        case SMART_ACTION_REMOVEAURASFROMSPELL:
        {
            for (WorldObject* target : targets)
            {
                if (!IsUnit(target))
                    continue;

                if (e.action.removeAura.spell)
                {
                    /* TODO SPELLS 
                    if (e.action.removeAura.charges)
                    {
                        if (Aura* aur = target->ToUnit()->GetAura(e.action.removeAura.spell))
                            aur->ModCharges(-e.action.removeAura.charges, AURA_REMOVE_BY_EXPIRE);
                    } else */
                        target->ToUnit()->RemoveAurasDueToSpell(e.action.removeAura.spell);
                }
                else
                    target->ToUnit()->RemoveAllAuras();

                TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_REMOVEAURASFROMSPELL: Unit %u, spell %u",
                    target->GetGUIDLow(), e.action.removeAura.spell);
            }


            break;
        }
        case SMART_ACTION_FOLLOW:
        {
            if (!IsSmart())
                break;

            if (targets.empty())
            {
                ENSURE_AI(SmartAI, me->AI())->StopFollow(false);
                break;
            }

            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    ENSURE_AI(SmartAI, me->AI())->SetFollow(target->ToUnit(), float(e.action.follow.dist) + 0.1f, e.action.follow.angle, e.action.follow.credit, e.action.follow.entry, e.action.follow.creditType);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_FOLLOW: Creature %u following target %u",
                        me->GetGUIDLow(), target->GetGUIDLow());
                    break;
                }
            }


            break;
        }
        case SMART_ACTION_RANDOM_PHASE:
        {
            if (!GetBaseObject())
                break;

            uint32 phases[SMART_ACTION_PARAM_COUNT];
            phases[0] = e.action.randomPhase.phase1;
            phases[1] = e.action.randomPhase.phase2;
            phases[2] = e.action.randomPhase.phase3;
            phases[3] = e.action.randomPhase.phase4;
            phases[4] = e.action.randomPhase.phase5;
            phases[5] = e.action.randomPhase.phase6;
            uint32 temp[SMART_ACTION_PARAM_COUNT];
            uint32 count = 0;
            for (uint32 phase : phases)
            {
                if (phase > 0)
                {
                    temp[count] = phase;
                    ++count;
                }
            }

            if (count == 0)
                break;

            uint32 phase = temp[urand(0, count - 1)];
            SetPhase(phase);
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_RANDOM_PHASE: Creature %u sets event phase to %u",
                GetBaseObject()->GetGUIDLow(), phase);
            break;
        }
        case SMART_ACTION_RANDOM_PHASE_RANGE:
        {
            if (!GetBaseObject())
                break;

            uint32 phase = urand(e.action.randomPhaseRange.phaseMin, e.action.randomPhaseRange.phaseMax);
            SetPhase(phase);
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_RANDOM_PHASE_RANGE: Creature %u sets event phase to %u",
                GetBaseObject()->GetGUIDLow(), phase);
            break;
        }
        case SMART_ACTION_CALL_KILLEDMONSTER:
        {
            if (e.target.type == SMART_TARGET_NONE || e.target.type == SMART_TARGET_SELF) // Loot recipient and his group members
            {
                if (!me)
                    break;

                if (Player* player = me->GetLootRecipient())
                {
                    player->RewardPlayerAndGroupAtEvent(e.action.killedMonster.creature, player);
                    TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_CALL_KILLEDMONSTER: Player %u, Killcredit: %u",
                        player->GetGUIDLow(), e.action.killedMonster.creature);
                }
            }
            else // Specific target type
            {
                for (WorldObject* target : targets)
                {
                    if (IsPlayer(target))
                    {
                        target->ToPlayer()->RewardPlayerAndGroupAtEvent(e.action.killedMonster.creature, target->ToPlayer());
                        TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_CALL_KILLEDMONSTER: Player %u, Killcredit: %u",
                            target->GetGUIDLow(), e.action.killedMonster.creature);
                    }
                }

    
            }
            break;
        }
        case SMART_ACTION_SET_INST_DATA:
        {
            WorldObject* obj = GetBaseObject();
            if (!obj)
                obj = unit;

            if (!obj)
                break;

            InstanceScript* instance = ((InstanceScript*)obj->GetInstanceScript());
            if (!instance)
            {
                TC_LOG_ERROR("scripts.ai","SmartScript: Event %u attempt to set instance data without instance script. EntryOrGuid %d", e.GetEventType(), e.entryOrGuid);
                break;
            }

            switch (e.action.setInstanceData.type)
            {
                case 0:
                    instance->SetData(e.action.setInstanceData.field, e.action.setInstanceData.data);
                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction: SMART_ACTION_SET_INST_DATA: SetData Field: %u, data: %u",
                        e.action.setInstanceData.field, e.action.setInstanceData.data);
                    break;
                case 1:
                    instance->SetBossState(e.action.setInstanceData.field, static_cast<EncounterState>(e.action.setInstanceData.data));
                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction: SMART_ACTION_SET_INST_DATA: SetBossState BossId: %u, State: %u (%s)",
                        e.action.setInstanceData.field, e.action.setInstanceData.data, /* InstanceScript::GetBossStateName(e.action.setInstanceData.data).c_str()*/ "(boss name NYI)");
                    break;
                default: // Static analysis
                    break;
            }
            break;
        }
        case SMART_ACTION_SET_INST_DATA64:
        {
            WorldObject* obj = GetBaseObject();
            if (!obj)
                obj = unit;

            if (!obj)
                break;

            InstanceScript* instance = ((InstanceScript*)obj->GetInstanceScript());
            if (!instance)
            {
                TC_LOG_ERROR("scripts.ai","SmartScript: Event %u attempt to set instance data without instance script. EntryOrGuid %d", e.GetEventType(), e.entryOrGuid);
                break;
            }

            if (targets.empty())
                break;

            instance->SetData64(e.action.setInstanceData64.field, targets.front()->GetGUID());
            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_SET_INST_DATA64: Field: %u, data: " UI64FMTD,
                e.action.setInstanceData64.field, targets.front()->GetGUID());


            break;
        }
        case SMART_ACTION_UPDATE_TEMPLATE:
        {
            if (targets.empty())
                break;

            for (WorldObject* target : targets)
                if (IsCreature(target))
                    target->ToCreature()->UpdateEntry(e.action.updateTemplate.creature);


            break;
        }
        case SMART_ACTION_DIE:
        {
            if (me && !me->IsDead())
            {
                me->Kill(me);
                TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_DIE: Creature %u", me->GetGUIDLow());
            }
            break;
        }
        case SMART_ACTION_SET_IN_COMBAT_WITH_ZONE:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                {
                    me->AI()->DoZoneInCombat();
                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction: SMART_ACTION_SET_IN_COMBAT_WITH_ZONE: Creature %u", me->GetGUIDLow());
                }
            }
            
            break;
        }
        case SMART_ACTION_CALL_FOR_HELP:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                {
                    target->ToCreature()->CallForHelp((float)e.action.callHelp.range);
                    if (e.action.callHelp.withEmote)
                    {
                        Trinity::BroadcastTextBuilder builder(me, CHAT_MSG_MONSTER_EMOTE, BROADCAST_TEXT_CALL_FOR_HELP);
                        sCreatureTextMgr->SendChatPacket(me, builder, CHAT_MSG_MONSTER_EMOTE);
                    }
                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction: SMART_ACTION_CALL_FOR_HELP: Creature %u, target: %u", me->GetGUIDLow(), target->GetGUIDLow());
                }
            }

            break;
        }
        case SMART_ACTION_SET_SHEATH:
        {
            if (me)
            {
                me->SetSheath(SheathState(e.action.setSheath.sheath));
                TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction: SMART_ACTION_SET_SHEATH: Creature %u, State: %u",
                    me->GetGUIDLow(), e.action.setSheath.sheath);
            }
            break;
        }
        case SMART_ACTION_FORCE_DESPAWN:
        {
            if (targets.empty())
                break;

            // there should be at least a world update tick before despawn, to avoid breaking linked actions
            int32 const respawnDelay = std::max<int32>(e.action.forceDespawn.delay, 1);

            for (WorldObject* target : targets)
            {
                if (Creature* creatureTarget = target->ToCreature())
                {
                    if (SmartAI* smartAI = CAST_AI(SmartAI, creatureTarget->AI()))
                    {
                        smartAI->SetDespawnTime(respawnDelay);
                        smartAI->StartDespawn();
                    }
                    else
                        creatureTarget->DespawnOrUnsummon(respawnDelay);
                }
				else if (GameObject* goTarget = target->ToGameObject())
					goTarget->SetDespawnTimer(respawnDelay);
            }

            break;
        }
        case SMART_ACTION_SET_INGAME_PHASE_MASK:
        {
            if (targets.empty())
                break;

            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                    target->ToUnit()->SetPhaseMask(e.action.ingamePhaseMask.mask, true);
                else if (IsGameObject(target))
                    target->ToGameObject()->SetPhaseMask(e.action.ingamePhaseMask.mask, true);
            }


            break;
        }
        case SMART_ACTION_MOUNT_TO_ENTRY_OR_MODEL:
        {
            for (WorldObject* target : targets)
            {
                if (!IsUnit(target))
                    continue;

                if (e.action.morphOrMount.creature || e.action.morphOrMount.model)
                {
                    if (e.action.morphOrMount.creature > 0)
                    {
                        if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(e.action.morphOrMount.creature))
                        {
                            uint32 display_id = sObjectMgr->ChooseDisplayId(cInfo);
                            me->Mount(display_id);
                        }
                    }
                    else
                        target->ToUnit()->Mount(e.action.morphOrMount.model);
                }
                else
                    me->Dismount();
            }


            break;
        }
        case SMART_ACTION_SET_INVINCIBILITY_HP_LEVEL:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                {
                    SmartAI* ai = CAST_AI(SmartAI, target->ToCreature()->AI());
                    if (!ai)
                        continue;

                    if (e.action.invincHP.percent)
                        ai->SetInvincibilityHpLevel(target->ToCreature()->CountPctFromMaxHealth(int32(e.action.invincHP.percent)));
                    else
                        ai->SetInvincibilityHpLevel(e.action.invincHP.minHP);
                }
            }


            break;
        }
        case SMART_ACTION_SET_DATA:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToCreature()->AI()->SetData(e.action.setData.field, e.action.setData.data, me);
                else if (IsGameObject(target))
                    target->ToGameObject()->AI()->SetData(e.action.setData.field, e.action.setData.data, me);
            }


            break;
        }
        case SMART_ACTION_MOVE_FORWARD:
        {
            if (!me)
                break;

            float x, y, z;
            me->GetClosePoint(x, y, z, me->GetCombatReach() / 3, (float)e.action.moveRandom.distance);
            me->GetMotionMaster()->MovePoint(SMART_RANDOM_POINT, x, y, z);
            break;
        }
        case SMART_ACTION_SET_VISIBILITY:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                    target->ToUnit()->SetVisible(e.action.visibility.state ? true : false);
            }

            break;
        }
        case SMART_ACTION_MOVE_OFFSET:
        {
            for (WorldObject* target : targets)
            {
                if (!IsCreature(target))
                    continue;

                if (!(e.event.event_flags & SMART_EVENT_FLAG_WHILE_CHARMED) && IsCharmedCreature(target))
                    continue;

                Position pos = target->GetPosition();

                // Use forward/backward/left/right cartesian plane movement
                float x, y, z, o;
                o = pos.GetOrientation();
                x = pos.GetPositionX() + (std::cos(o - (M_PI / 2))*e.target.x) + (std::cos(o)*e.target.y);
                y = pos.GetPositionY() + (std::sin(o - (M_PI / 2))*e.target.x) + (std::sin(o)*e.target.y);
                z = pos.GetPositionZ() + e.target.z;
                target->ToCreature()->GetMotionMaster()->MovePoint(SMART_RANDOM_POINT, x, y, z);
            }

            break;
        }
        case SMART_ACTION_RANDOM_SOUND:
        {
            std::vector<uint32> sounds;
            std::copy_if(e.action.randomSound.sounds.begin(), e.action.randomSound.sounds.end(),
                std::back_inserter(sounds), [](uint32 sound) { return sound != 0; });

            bool onlySelf = e.action.randomSound.onlySelf != 0;

            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    uint32 sound = Trinity::Containers::SelectRandomContainerElement(sounds);
                    if (e.action.randomSound.distance == 1)
                        target->PlayDistanceSound(sound, onlySelf ? target->ToPlayer() : nullptr);
                    else
                        target->PlayDirectSound(sound, onlySelf ? target->ToPlayer() : nullptr);

                    TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_RANDOM_SOUND: target: %s (" UI64FMTD " ), sound: %u, onlyself: %s",
                        target->GetName().c_str(), target->GetGUID(), sound, onlySelf ? "true" : "false");
                }
            }
        }
        case SMART_ACTION_SET_CORPSE_DELAY:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToCreature()->SetCorpseDelay(e.action.corpseDelay.timer);
            }


            break;
        }
        case SMART_ACTION_SPAWN_SPAWNGROUP:
        {
            if (e.action.groupSpawn.minDelay == 0 && e.action.groupSpawn.maxDelay == 0)
            {
                bool const ignoreRespawn = ((e.action.groupSpawn.spawnflags & SMARTAI_SPAWN_FLAGS::SMARTAI_SPAWN_FLAG_IGNORE_RESPAWN) != 0);
                bool const force = ((e.action.groupSpawn.spawnflags & SMARTAI_SPAWN_FLAGS::SMARTAI_SPAWN_FLAG_FORCE_SPAWN) != 0);

                // Instant spawn
                GetBaseObject()->GetMap()->SpawnGroupSpawn(e.action.groupSpawn.groupId, ignoreRespawn, force);
            }
            else
            {
                // Delayed spawn (use values from parameter to schedule event to call us back
                SmartEvent ne = SmartEvent();
                ne.type = (SMART_EVENT)SMART_EVENT_UPDATE;
                ne.event_chance = 100;

                ne.minMaxRepeat.min = e.action.groupSpawn.minDelay;
                ne.minMaxRepeat.max = e.action.groupSpawn.maxDelay;
                ne.minMaxRepeat.repeatMin = 0;
                ne.minMaxRepeat.repeatMax = 0;

                ne.event_flags = 0;
                ne.event_flags |= SMART_EVENT_FLAG_NOT_REPEATABLE;

                SmartAction ac = SmartAction();
                ac.type = (SMART_ACTION)SMART_ACTION_SPAWN_SPAWNGROUP;
                ac.groupSpawn.groupId = e.action.groupSpawn.groupId;
                ac.groupSpawn.minDelay = 0;
                ac.groupSpawn.maxDelay = 0;
                ac.groupSpawn.spawnflags = e.action.groupSpawn.spawnflags;
                ac.timeEvent.id = e.action.timeEvent.id;

                SmartScriptHolder ev = SmartScriptHolder();
                ev.event = ne;
                ev.event_id = e.event_id;
                ev.target = e.target;
                ev.action = ac;
                InitTimer(ev);
                mStoredEvents.push_back(ev);
            }
            break;
        }
        case SMART_ACTION_DESPAWN_SPAWNGROUP:
        {
            if (e.action.groupSpawn.minDelay == 0 && e.action.groupSpawn.maxDelay == 0)
            {
                bool const deleteRespawnTimes = ((e.action.groupSpawn.spawnflags & SMARTAI_SPAWN_FLAGS::SMARTAI_SPAWN_FLAG_NOSAVE_RESPAWN) != 0);

                // Instant spawn
                GetBaseObject()->GetMap()->SpawnGroupDespawn(e.action.groupSpawn.groupId, deleteRespawnTimes);
            }
            else
            {
                // Delayed spawn (use values from parameter to schedule event to call us back
                SmartEvent ne = SmartEvent();
                ne.type = (SMART_EVENT)SMART_EVENT_UPDATE;
                ne.event_chance = 100;

                ne.minMaxRepeat.min = e.action.groupSpawn.minDelay;
                ne.minMaxRepeat.max = e.action.groupSpawn.maxDelay;
                ne.minMaxRepeat.repeatMin = 0;
                ne.minMaxRepeat.repeatMax = 0;

                ne.event_flags = 0;
                ne.event_flags |= SMART_EVENT_FLAG_NOT_REPEATABLE;

                SmartAction ac = SmartAction();
                ac.type = (SMART_ACTION)SMART_ACTION_DESPAWN_SPAWNGROUP;
                ac.groupSpawn.groupId = e.action.groupSpawn.groupId;
                ac.groupSpawn.minDelay = 0;
                ac.groupSpawn.maxDelay = 0;
                ac.groupSpawn.spawnflags = e.action.groupSpawn.spawnflags;
                ac.timeEvent.id = e.action.timeEvent.id;

                SmartScriptHolder ev = SmartScriptHolder();
                ev.event = ne;
                ev.event_id = e.event_id;
                ev.target = e.target;
                ev.action = ac;
                InitTimer(ev);
                mStoredEvents.push_back(ev);
            }
            break;
        }
        case SMART_ACTION_DISABLE_EVADE:
        {
            if (!IsSmart())
                break;

            ENSURE_AI(SmartAI, me->AI())->SetEvadeDisabled(e.action.disableEvade.disable != 0);
            break;
        }
        case SMART_ACTION_REMOVE_AURAS_BY_TYPE: // can be used to exit vehicle for example
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                    target->ToUnit()->RemoveAurasByType((AuraType)e.action.auraType.type);
            }

            break;
        }
        case SMART_ACTION_SET_SIGHT_DIST:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToCreature()->m_SightDistance = e.action.sightDistance.dist;
            }

            break;
        }
        case SMART_ACTION_FLEE:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToCreature()->GetMotionMaster()->MoveFleeing(me, e.action.flee.fleeTime);
            }

            break;
        }
        case SMART_ACTION_ADD_THREAT:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                    me->GetThreatManager().AddThreat(target->ToUnit(), (float)e.action.threatPCT.threatINC - (float)e.action.threatPCT.threatDEC);
            }

            break;
        }
        case SMART_ACTION_LOAD_EQUIPMENT:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToCreature()->LoadEquipment(e.action.loadEquipment.id, e.action.loadEquipment.force != 0);
            }

            break;
        }
        case SMART_ACTION_TRIGGER_RANDOM_TIMED_EVENT:
        {
            uint32 eventId = urand(e.action.randomTimedEvent.minId, e.action.randomTimedEvent.maxId);
            ProcessEventsFor((SMART_EVENT)SMART_EVENT_TIMED_EVENT_TRIGGERED, NULL, eventId);
            break;
        }

        case SMART_ACTION_REMOVE_ALL_GAMEOBJECTS:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                    target->ToUnit()->RemoveAllGameObjects();
            }

            break;
        }
        case SMART_ACTION_STOP_MOTION:
        {
            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    if (e.action.stopMotion.stopMovement)
                        target->ToUnit()->StopMoving();
                    if (e.action.stopMotion.movementExpired)
                        target->ToUnit()->GetMotionMaster()->MovementExpired();
                }
            }

            break;
        }
        case SMART_ACTION_GO_SET_GO_STATE:
        {
            for (WorldObject* target : targets)
            {
                if (IsGameObject(target))
                    target->ToGameObject()->SetGoState((GOState)e.action.goState.state);
            }

            break;
        }
        case SMART_ACTION_SET_ACTIVE:
        {
            for (WorldObject* target : targets)
            {
                target->SetKeepActive(e.action.active.state);
            }

            break;
        }
        case SMART_ACTION_ATTACK_START:
        {
            if (!me)
                break;

            if (targets.empty())
                break;

            // attack random target
            if (Unit* target = Trinity::Containers::SelectRandomContainerElement(targets)->ToUnit())
                me->AI()->AttackStart(target);

            break;
        }
        case SMART_ACTION_ASSIST:
        {
            if (!me)
                break;

            for (WorldObject* target : targets)
            {
                if (IsUnit(target))
                {
                    Unit* targetUnit = target->ToUnit();
                    if(Unit* victim = targetUnit->GetVictim())
                    {
                        if (me->CanCreatureAttack(victim))
                        {
                            me->AI()->AttackStart(victim);
                            break; //found a valid target, no need to continue
                        }
                    }
                }
            }


            break;
        }
        case SMART_ACTION_SUMMON_CREATURE:
        {

            float x, y, z, o;
            for (WorldObject* target : targets)
            {
                target->GetPosition(x, y, z, o);
                x += e.target.x;
                y += e.target.y;
                z += e.target.z;
                o += e.target.o;
                if (Creature* summon = GetBaseObject()->SummonCreature(e.action.summonCreature.creature, x, y, z, o, (TempSummonType)e.action.summonCreature.type, e.action.summonCreature.duration))
                {
                    if (e.action.summonCreature.attackInvoker)
                        summon->EngageWithTarget(target->ToUnit());
                    if (e.action.summonCreature.attackVictim)
                        if(Unit* victim = target->ToUnit()->GetVictim())
                            summon->EngageWithTarget(victim);
                }
            }

            if (e.GetTargetType() != SMART_TARGET_POSITION)
                break;

            if (Creature* summon = GetBaseObject()->SummonCreature(e.action.summonCreature.creature, e.target.x, e.target.y, e.target.z, e.target.o, (TempSummonType)e.action.summonCreature.type, e.action.summonCreature.duration))
            {
                if (me && e.action.summonCreature.attackInvoker)
                    summon->EngageWithTarget(me);
                if (e.action.summonCreature.attackVictim)
                    if(Unit* victim = me->ToUnit()->GetVictim())
                        summon->EngageWithTarget(victim);
            }
            break;
        }
        case SMART_ACTION_SUMMON_GO:
        {
            if (!GetBaseObject())
                break;

            for (WorldObject* target : targets)
            {
                Position pos = target->GetPositionWithOffset(Position(e.target.x, e.target.y, e.target.z, e.target.o));
                G3D::Quat rot = G3D::Matrix3::fromEulerAnglesZYX(pos.GetOrientation(), 0.f, 0.f);
                GetBaseObject()->SummonGameObject(e.action.summonGO.entry, pos, rot, e.action.summonGO.despawnTime);
            }

            if (e.GetTargetType() != SMART_TARGET_POSITION)
                break;

            GetBaseObject()->SummonGameObject(e.action.summonGO.entry, Position(e.target.x, e.target.y, e.target.z, e.target.o), G3D::Quat(), e.action.summonGO.despawnTime);
            break;
        }
        case SMART_ACTION_KILL_UNIT:
        {
            for (WorldObject* target : targets)
            {
                if (!IsUnit(target))
                    continue;

                target->ToUnit()->Kill(target->ToUnit());
            }


            break;
        }
        case SMART_ACTION_INSTALL_AI_TEMPLATE:
        {
            InstallTemplate(e);
            break;
        }
        case SMART_ACTION_ADD_ITEM:
            {
            for (WorldObject* target : targets)
            {
                if(!IsPlayer((target))) 
                    continue;

                target->ToPlayer()->AddItem(e.action.item.entry, e.action.item.count);
            }
            break;
            }
        case SMART_ACTION_REMOVE_ITEM:
        {
            for (WorldObject* target : targets)
            {
                if (!IsPlayer(target))
                    continue;

                target->ToPlayer()->DestroyItemCount(e.action.item.entry, e.action.item.count, true);
            }


            break;
        }
        case SMART_ACTION_STORE_TARGET_LIST:
        {
            StoreTargetList(targets, e.action.storeTargets.id);
            break;
        }
        case SMART_ACTION_TELEPORT:
        {
            for (WorldObject* target : targets)
            {
                if (IsPlayer(target))
                {
                    uint32 targetMap = e.action.teleport.ignoreMap ? target->GetMapId() : e.action.teleport.mapID;
                    target->ToPlayer()->TeleportTo(targetMap, e.target.x, e.target.y, e.target.z, e.target.o);
                } else if (IsCreature(target))
                    target->ToCreature()->NearTeleportTo(e.target.x, e.target.y, e.target.z, e.target.o);

                if(e.action.teleport.useVisual)
                    if(Unit* u = target->ToUnit())
                        me->CastSpell(u, 46614, TRIGGERED_FULL_MASK); //teleport visual
            }


            break;
        }
        case SMART_ACTION_TELEPORT_ON_ME:
        {
            if (targets.empty())
                break;

            float x,y,z,o;
            me->GetPosition(x,y,z,o);

            for (WorldObject* target : targets)
            {
                if(Unit* u = target->ToUnit())
                {
                    u->NearTeleportTo(x, y, z, o);
                    if(e.action.teleportOnMe.useVisual)
                        me->CastSpell(u, 46614, TRIGGERED_FULL_MASK); //teleport visual
                }

            }


            break;
        }
        case SMART_ACTION_SELF_TELEPORT_ON_TARGET:
        {
            float x,y,z;
            bool found = false;

            for (WorldObject* target : targets)
            {
                if(Unit* u = target->ToUnit())
                {
                    u->GetPosition(x,y,z);
                    found = true;
                    break;
                }

            }

            if (!found)
                break;

            me->NearTeleportTo(x, y, z, me->GetOrientation());
            if(e.action.teleportSelfOnTarget.useVisual)
                me->CastSpell(me, 46614, TRIGGERED_FULL_MASK); //teleport visual


            break;
        }
        case SMART_ACTION_SET_DISABLE_GRAVITY:
        {
            if (!IsSmart())
                break;

            ENSURE_AI(SmartAI, me->AI())->SetDisableGravity(e.action.setDisableGravity.disable != 0);
            break;
        }
        case SMART_ACTION_SET_CAN_FLY:
        {
            if (!IsSmart())
                break;

            ENSURE_AI(SmartAI, me->AI())->SetCanFly(e.action.setFly.fly);
            break;
        }
        case SMART_ACTION_SET_RUN:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetRun(e.action.setRun.run);
            break;
        }
        case SMART_ACTION_SET_SWIM:
        {
            if (!IsSmart())
                break;

            CAST_AI(SmartAI, me->AI())->SetSwim(e.action.setSwim.swim);
            break;
        }
        case SMART_ACTION_SET_COUNTER:
        {
            if (!targets.empty())
            {
                for (WorldObject* target : targets)
                {
                    if (IsCreature(target))
                    {
                        if (SmartAI* ai = CAST_AI(SmartAI, target->ToCreature()->AI()))
                            ai->GetScript()->StoreCounter(e.action.setCounter.counterId, e.action.setCounter.value, e.action.setCounter.reset);
                        else
                            TC_LOG_ERROR("sql.sql", "SmartScript: Action target for SMART_ACTION_SET_COUNTER is not using SmartAI, skipping");
                    }
                    else if (IsGameObject(target))
                    {
                        if (SmartGameObjectAI* ai = CAST_AI(SmartGameObjectAI, target->ToGameObject()->AI()))
                            ai->GetScript()->StoreCounter(e.action.setCounter.counterId, e.action.setCounter.value, e.action.setCounter.reset);
                        else
                            TC_LOG_ERROR("sql.sql", "SmartScript: Action target for SMART_ACTION_SET_COUNTER is not using SmartGameObjectAI, skipping");
                    }
                }
            }
            else
                StoreCounter(e.action.setCounter.counterId, e.action.setCounter.value, e.action.setCounter.reset);

            break;
        }
        case SMART_ACTION_WP_START:
        {
            if (!IsSmart())
                break;

            bool run = e.action.wpStart.run;
            uint32 entry = e.action.wpStart.pathID;
            bool repeat = e.action.wpStart.repeat != 0;

            for (WorldObject* target : targets)
            {
                if (IsPlayer(target))
                {
                    if (IsPlayer(target))
                    {
                        StoreTargetList(targets, SMART_ESCORT_TARGETS);
                        break;
                    }
                }
            }

            me->SetReactState((ReactStates)e.action.wpStart.reactState);
            ENSURE_AI(SmartAI, me->AI())->StartPath(run, entry, repeat, unit);

            uint32 quest = e.action.wpStart.quest;
            uint32 DespawnTime = e.action.wpStart.despawnTime;
            ENSURE_AI(SmartAI, me->AI())->SetEscortQuest(quest);
            ENSURE_AI(SmartAI, me->AI())->SetDespawnTime(DespawnTime);
            break;
        }
        case SMART_ACTION_WP_PAUSE:
        {
            if (!IsSmart())
                break;

            uint32 delay = e.action.wpPause.delay;
            ENSURE_AI(SmartAI, me->AI())->PausePath(delay, e.GetEventType() == SMART_EVENT_WAYPOINT_REACHED ? false : true);
            break;
        }
        case SMART_ACTION_WP_STOP:
        {
            if (!IsSmart())
                break;

            uint32 DespawnTime = e.action.wpStop.despawnTime;
            uint32 quest = e.action.wpStop.quest;
            bool fail = e.action.wpStop.fail;
            CAST_AI(SmartAI, me->AI())->StopPath(DespawnTime, quest, fail);
            break;
        }
        case SMART_ACTION_WP_RESUME:
        {
            if (!IsSmart())
                break;

            ENSURE_AI(SmartAI, me->AI())->SetWPPauseTimer(0);
            break;
        }
        case SMART_ACTION_SET_ORIENTATION:
            {
                if (!me)
                    break;
                if (e.GetTargetType() == SMART_TARGET_SELF)
                    me->SetFacingTo((me->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && me->GetTransGUID() ?
                        me->GetTransportHomePosition() : me->GetHomePosition()).GetOrientation());
                else if (e.GetTargetType() == SMART_TARGET_POSITION)
                    me->SetFacingTo(e.target.o);
                else if (!targets.empty())
                {
                    me->SetFacingToObject(targets[0]);
                }
                break;
            }
            /*
        case SMART_ACTION_PLAYMOVIE: //LK
        {
            for (WorldObject* target : targets)
            {
                if (!IsPlayer(target))
                    continue;

                target->ToPlayer()->SendMovieStart(e.action.movie.entry);
            }

            break;
        }
        */
        case SMART_ACTION_MOVE_TO_POS:
        {
            if (!IsSmart())
                break;

            WorldObject* target = nullptr;

            /*if (e.GetTargetType() == SMART_TARGET_CREATURE_RANGE || e.GetTargetType() == SMART_TARGET_CREATURE_GUID ||
                e.GetTargetType() == SMART_TARGET_CREATURE_DISTANCE || e.GetTargetType() == SMART_TARGET_GAMEOBJECT_RANGE ||
                e.GetTargetType() == SMART_TARGET_GAMEOBJECT_GUID || e.GetTargetType() == SMART_TARGET_GAMEOBJECT_DISTANCE ||
                e.GetTargetType() == SMART_TARGET_CLOSEST_CREATURE || e.GetTargetType() == SMART_TARGET_CLOSEST_GAMEOBJECT ||
                e.GetTargetType() == SMART_TARGET_OWNER_OR_SUMMONER || e.GetTargetType() == SMART_TARGET_ACTION_INVOKER ||
                e.GetTargetType() == SMART_TARGET_CLOSEST_ENEMY || e.GetTargetType() == SMART_TARGET_CLOSEST_FRIENDLY)*/
            {
                // we want to move to random element
                if (!targets.empty())
                    target = Trinity::Containers::SelectRandomContainerElement(targets);
            }

            if (!target)
            {                
                G3D::Vector3 dest(e.target.x, e.target.y, e.target.z);
                if (e.action.MoveToPos.transport)
                    if (TransportBase* trans = me->GetTransport())
                        trans->CalculatePassengerPosition(dest.x, dest.y, dest.z);

                //MovePoint will ignore e.target.o if it is 0.0f
                me->GetMotionMaster()->MovePoint(e.action.MoveToPos.pointId, dest.x, dest.y, dest.z, e.target.o, e.action.MoveToPos.disablePathfinding == 0);
            }
            else {
                float x, y, z;
                target->GetPosition(x, y, z);
                if (e.action.MoveToPos.ContactDistance > 0)
                    target->GetContactPoint(me, x, y, z, e.action.MoveToPos.ContactDistance);
                me->GetMotionMaster()->MovePoint(e.action.MoveToPos.pointId, x + e.target.x, y + e.target.y, z + e.target.z, e.action.MoveToPos.disablePathfinding == 0);
            }
            break;
        }
        case SMART_ACTION_RESPAWN_TARGET:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToCreature()->Respawn();
                else if (IsGameObject(target))
                {
                    // do not modify respawndelay of already spawned gameobjects
                    if (target->ToGameObject()->isSpawnedByDefault())
                        target->ToGameObject()->Respawn();
                    else
                        target->ToGameObject()->SetRespawnTime(e.action.RespawnTarget.goRespawnTime);
                }
            }


            break;
        }
        case SMART_ACTION_CLOSE_GOSSIP:
        {
            for (WorldObject* target : targets)
            {
                if (IsPlayer(target))
                    target->ToPlayer()->PlayerTalkClass->SendCloseGossip();
            }

            break;
        }
        case SMART_ACTION_EQUIP:
        {
            for (WorldObject* target : targets)
            {
                if (Creature* npc = target->ToCreature())
                {
#ifdef LICH_KING
                    std::array<uint32, MAX_EQUIPMENT_ITEMS> slot;
#endif
                    if (int8 equipId = static_cast<int8>(e.action.equip.entry))
                    {
                        EquipmentInfo const* eInfo = sObjectMgr->GetEquipmentInfo(npc->GetEntry(), equipId);
                        if (!eInfo)
                        {
                            TC_LOG_ERROR("sql.sql", "SmartScript: SMART_ACTION_EQUIP uses non-existent equipment info id %u for creature %u", equipId, npc->GetEntry());
                            break;
                        }

#ifdef LICH_KING
                        npc->SetCurrentEquipmentId(equipId);
                        std::copy(std::begin(eInfo->ItemEntry), std::end(eInfo->ItemEntry), std::begin(slot));
#else
                        //to improve: mask not taken into account...
                        npc->LoadEquipment(equipId, true);
#endif
                    }
                    else
                    {
#ifdef LICH_KING
                        slot[0] = e.action.equip.slot1;
                        slot[1] = e.action.equip.slot2;
                        slot[2] = e.action.equip.slot3;
#else
                        TC_LOG_ERROR("sql.sql", "SmartScript: SMART_ACTION_EQUIP cannot be used with weapon IDs on 2.4.3, use creature_equip_template ids instead (npc = %u)", npc->GetEntry());
#endif
                    }

#ifdef LICH_KING
                    for (uint32 i = 0; i < MAX_EQUIPMENT_ITEMS; ++i)
                        if (!e.action.equip.mask || (e.action.equip.mask & (1 << i)))
                            npc->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + i, slot[i]);
#endif
                }
            }

            for (WorldObject* target : targets)
            {
                if (Creature* npc = target->ToCreature())
                {
                    
                    uint32 slot[3];
                   /* int8 equipId = (int8)e.action.equip.entry;
                    if (equipId)
                    {
                        EquipmentInfo const* einfo = sObjectMgr->GetEquipmentInfo(npc->GetEntry(), equipId);
                        if (!einfo)
                        {
                            TC_LOG_ERROR("scripts.ai","SmartScript: SMART_ACTION_EQUIP uses non-existent equipment info id %u for creature %u", equipId, npc->GetEntry());
                            break;
                        }

                        npc->SetCurrentEquipmentId(equipId);
                        slot[0] = einfo->ItemEntry[0];
                        slot[1] = einfo->ItemEntry[1];
                        slot[2] = einfo->ItemEntry[2];
                    }
                    else
                    {*/
                        slot[0] = e.action.equip.slot1;
                        slot[1] = e.action.equip.slot2;
                        slot[2] = e.action.equip.slot3;
                    //}
#ifdef LICH_KING
                    if (!e.action.equip.mask || (e.action.equip.mask & 1))
                        npc->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, slot[0]);
                    if (!e.action.equip.mask || (e.action.equip.mask & 2))
                        npc->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, slot[1]);
                    if (!e.action.equip.mask || (e.action.equip.mask & 4))
                        npc->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, slot[2]);
#endif
                }
            }


            break;
        }
        case SMART_ACTION_CREATE_TIMED_EVENT:
        {
            SmartEvent ne = SmartEvent();
            ne.type = (SMART_EVENT)SMART_EVENT_UPDATE;
            ne.event_chance = e.action.timeEvent.chance;
            if (!ne.event_chance) ne.event_chance = 100;

            ne.minMaxRepeat.min = e.action.timeEvent.min;
            ne.minMaxRepeat.max = e.action.timeEvent.max;
            ne.minMaxRepeat.repeatMin = e.action.timeEvent.repeatMin;
            ne.minMaxRepeat.repeatMax = e.action.timeEvent.repeatMax;

            ne.event_flags = 0;
            if (!ne.minMaxRepeat.repeatMin && !ne.minMaxRepeat.repeatMax)
                ne.event_flags |= SMART_EVENT_FLAG_NOT_REPEATABLE;

            SmartAction ac = SmartAction();
            ac.type = (SMART_ACTION)SMART_ACTION_TRIGGER_TIMED_EVENT;
            ac.timeEvent.id = e.action.timeEvent.id;

            SmartScriptHolder ev = SmartScriptHolder();
            ev.event = ne;
            ev.event_id = e.action.timeEvent.id;
            ev.target = e.target;
            ev.action = ac;
            InitTimer(ev);
            mStoredEvents.push_back(ev);
            break;
        }
        case SMART_ACTION_TRIGGER_TIMED_EVENT:
            ProcessEventsFor((SMART_EVENT)SMART_EVENT_TIMED_EVENT_TRIGGERED, nullptr, e.action.timeEvent.id);
            // remove this event if not repeatable
            if (e.event.event_flags & SMART_EVENT_FLAG_NOT_REPEATABLE)
                mRemIDs.push_back(e.action.timeEvent.id);
            break;
        case SMART_ACTION_REMOVE_TIMED_EVENT:
            mRemIDs.push_back(e.action.timeEvent.id);
            break;
        case SMART_ACTION_OVERRIDE_SCRIPT_BASE_OBJECT:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                {
                    if (!meOrigGUID)
                        meOrigGUID = me ? me->GetGUID() : 0;
                    if (!goOrigGUID)
                        goOrigGUID = go ? go->GetGUID() : 0;
                    go = nullptr;
                    me = target->ToCreature();
                    break;
                }
                else if (IsGameObject(target))
                {
                    if (!meOrigGUID)
                        meOrigGUID = me ? me->GetGUID() : 0;
                    if (!goOrigGUID)
                        goOrigGUID = go ? go->GetGUID() : 0;
                    go = target->ToGameObject();
                    me = nullptr;
                    break;
                }
            }


            break;
        }
        case SMART_ACTION_RESET_SCRIPT_BASE_OBJECT:
            ResetBaseObject();
            break;
        case SMART_ACTION_CALL_SCRIPT_RESET:
            OnReset();
            break;
        case SMART_ACTION_SET_RANGED_MOVEMENT:
        {
            if (!IsSmart())
                break;

            float attackDistance = float(e.action.setRangedMovement.distance);
            float attackAngle = float(e.action.setRangedMovement.angle) / 180.0f * M_PI;

            for (WorldObject* target : targets)
            {
                if (Creature* targetCreature = target->ToCreature())
                    if (IsSmart(targetCreature) && targetCreature->GetVictim())
                        if (CAST_AI(SmartAI, targetCreature->AI())->CanCombatMove())
                            targetCreature->GetMotionMaster()->MoveChase(targetCreature->GetVictim(), attackDistance, attackAngle);
            }
            break;
        }
        case SMART_ACTION_CALL_TIMED_ACTIONLIST:
        {
            if (e.GetTargetType() == SMART_TARGET_NONE)
            {
                SMARTAI_DB_ERROR(e.entryOrGuid, "SmartScript: Entry %d SourceType %u Event %u Action %u is using TARGET_NONE(0) for Script9 target. Please correct target_type in database.", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType());
                break;
            }

            for (WorldObject* target : targets)
            {
                if (Creature* unitTarget = target->ToCreature())
                {
                    if (IsSmart(unitTarget))
                        CAST_AI(SmartAI, unitTarget->AI())->SetScript9(e, e.action.timedActionList.id, GetLastInvoker());
                }
                else if (GameObject* goTarget = target->ToGameObject())
                {
                    if (IsSmartGO(goTarget))
                        CAST_AI(SmartGameObjectAI, goTarget->AI())->SetScript9(e, e.action.timedActionList.id, GetLastInvoker());
                }
            }

            break;
        }
        case SMART_ACTION_SET_NPC_FLAG:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToUnit()->SetUInt32Value(UNIT_NPC_FLAGS, e.action.unitFlag.flag);
            }

            break;
        }
        case SMART_ACTION_ADD_NPC_FLAG:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToUnit()->SetFlag(UNIT_NPC_FLAGS, e.action.unitFlag.flag);
            }

            break;
        }
        case SMART_ACTION_REMOVE_NPC_FLAG:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                    target->ToUnit()->RemoveFlag(UNIT_NPC_FLAGS, e.action.unitFlag.flag);
            }

            break;
        }
        case SMART_ACTION_CROSS_CAST:
        {
            if (targets.empty())
                break;

            ObjectVector casters;
            GetTargets(casters, CreateEvent(SMART_EVENT_UPDATE_IC, 0, 0, 0, 0, 0, SMART_ACTION_NONE, 0, 0, 0, 0, 0, 0, (SMARTAI_TARGETS)e.action.crossCast.targetType, e.action.crossCast.targetParam1, e.action.crossCast.targetParam2, e.action.crossCast.targetParam3, SmartPhaseMask(0)), unit);

            for (WorldObject* caster : casters)
            {
                if (!IsUnit(caster))
                    continue;

                Unit* casterUnit = caster->ToUnit();

                bool interruptedSpell = false;

                for (WorldObject* target : targets)
                {
                    if (!IsUnit(target))
                        continue;

                    if (!(e.action.cast.castFlags & SMARTCAST_AURA_NOT_PRESENT) || !target->ToUnit()->HasAuraEffect(e.action.cast.spell))
                    {
                        if (!interruptedSpell && e.action.cast.castFlags & SMARTCAST_INTERRUPT_PREVIOUS)
                        {
                            casterUnit->InterruptNonMeleeSpells(false);
                            interruptedSpell = true;
                        }

                        TriggerCastFlags triggerFlag = TRIGGERED_NONE;
                        if (e.action.cast.castFlags & SMARTCAST_TRIGGERED)
                        {
                            if (e.action.cast.triggerFlags)
                                triggerFlag = TriggerCastFlags(e.action.cast.triggerFlags);
                            else
                                triggerFlag = TRIGGERED_FULL_MASK;
                        }

                        casterUnit->CastSpell(target->ToUnit(), e.action.cast.spell, triggerFlag);
                    }
                    else
                        TC_LOG_DEBUG("scripts.ai","Spell %u not cast because it has flag SMARTCAST_AURA_NOT_PRESENT and the target (Guid: " UI64FMTD " Entry: %u Type: %u) already has the aura", e.action.cast.spell, target->GetGUID(), target->GetEntry(), uint32(target->GetTypeId()));
                }
            }

            break;
        }
        case SMART_ACTION_CALL_RANDOM_TIMED_ACTIONLIST:
        {
            uint32 actions[SMART_ACTION_PARAM_COUNT];
            actions[0] = e.action.randTimedActionList.entry1;
            actions[1] = e.action.randTimedActionList.entry2;
            actions[2] = e.action.randTimedActionList.entry3;
            actions[3] = e.action.randTimedActionList.entry4;
            actions[4] = e.action.randTimedActionList.entry5;
            actions[5] = e.action.randTimedActionList.entry6;
            uint32 temp[SMART_ACTION_PARAM_COUNT];
            uint32 count = 0;
            for (uint32 action : actions)
            {
                if (action > 0)
                {
                    temp[count] = action;
                    ++count;
                }
            }

            if (count == 0)
                break;

            uint32 id = temp[urand(0, count - 1)];
            if (e.GetTargetType() == SMART_TARGET_NONE)
            {
                SMARTAI_DB_ERROR(e.entryOrGuid, "SmartScript: Entry %d SourceType %u Event %u Action %u is using TARGET_NONE(0) for Script9 target. Please correct target_type in database.", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType());
                break;
            }

            for (WorldObject* target : targets)
            {
                if (Creature* creTarget = target->ToCreature())
                {
                    if (IsSmart(creTarget))
                        CAST_AI(SmartAI, creTarget->AI())->SetScript9(e, id, GetLastInvoker());
                }
                else if (GameObject* goTarget = target->ToGameObject())
                {
                    if (IsSmartGO(goTarget))
                        CAST_AI(SmartGameObjectAI, goTarget->AI())->SetScript9(e, id, GetLastInvoker());
                }
            }
            break;
        }
        case SMART_ACTION_CALL_RANDOM_RANGE_TIMED_ACTIONLIST:
        {
            uint32 id = urand(e.action.randTimedActionList.entry1, e.action.randTimedActionList.entry2);
            if (e.GetTargetType() == SMART_TARGET_NONE)
            {
                SMARTAI_DB_ERROR(e.entryOrGuid, "SmartScript: Entry %d SourceType %u Event %u Action %u is using TARGET_NONE(0) for Script9 target. Please correct target_type in database.", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType());
                break;
            }

            for (WorldObject* target : targets)
            {
                if (Creature* creTarget = target->ToCreature())
                {
                    if (IsSmart(creTarget))
                        CAST_AI(SmartAI, creTarget->AI())->SetScript9(e, id, GetLastInvoker());
                }
                else if (GameObject* goTarget = target->ToGameObject())
                {
                    if (IsSmartGO(goTarget))
                        CAST_AI(SmartGameObjectAI, goTarget->AI())->SetScript9(e, id, GetLastInvoker());
                }
            }
            break;
        }
        case SMART_ACTION_ACTIVATE_TAXI:
        {
            for (WorldObject* target : targets)
                if (IsPlayer(target))
                    target->ToPlayer()->ActivateTaxiPathTo(e.action.taxi.id);


            break;
        }
        case SMART_ACTION_RANDOM_MOVE:
        {
            bool foundTarget = false;

            for (WorldObject* target : targets)
            {
                if (IsCreature((target)))
                {
                    foundTarget = true;

                    if (e.action.moveRandom.distance)
                        target->ToCreature()->GetMotionMaster()->MoveRandom((float)e.action.moveRandom.distance);
                    else
                        target->ToCreature()->GetMotionMaster()->MoveIdle();
                }
            }

            if (!foundTarget && me && IsCreature(me))
            {
                if (e.action.moveRandom.distance)
                    me->GetMotionMaster()->MoveRandom((float)e.action.moveRandom.distance);
                else
                    me->GetMotionMaster()->MoveIdle();
            }


            break;
        }
        case SMART_ACTION_SET_UNIT_FIELD_BYTES_1:
        case SMART_ACTION_SET_UNIT_FIELD_BYTES_2:
        {
            uint32 field = (e.GetActionType() == SMART_ACTION_SET_UNIT_FIELD_BYTES_1 ? UNIT_FIELD_BYTES_1 : UNIT_FIELD_BYTES_2);
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->SetByteFlag(field, e.action.setunitByte.type, e.action.setunitByte.byte1);


            break;
        }
        case SMART_ACTION_REMOVE_UNIT_FIELD_BYTES_1:
        case SMART_ACTION_REMOVE_UNIT_FIELD_BYTES_2:
        {
            uint32 field = (e.GetActionType() == SMART_ACTION_REMOVE_UNIT_FIELD_BYTES_1 ? UNIT_FIELD_BYTES_1 : UNIT_FIELD_BYTES_2);
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->RemoveByteFlag(field, e.action.delunitByte.type, e.action.delunitByte.byte1);


            break;
        }
        case SMART_ACTION_INTERRUPT_SPELL:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->InterruptNonMeleeSpells(e.action.interruptSpellCasting.withDelayed, e.action.interruptSpellCasting.spell_id, e.action.interruptSpellCasting.withInstant);


            break;
        }
        case SMART_ACTION_SEND_GO_CUSTOM_ANIM:
        {
            for (WorldObject* target : targets)
                if (IsGameObject(target))
                    target->ToGameObject()->SendCustomAnim(e.action.sendGoCustomAnim.anim);

            break;
        }
        case SMART_ACTION_SET_DYNAMIC_FLAG:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->SetUInt32Value(UNIT_DYNAMIC_FLAGS, e.action.unitFlag.flag);

            break;
        }
        case SMART_ACTION_ADD_DYNAMIC_FLAG:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->SetFlag(UNIT_DYNAMIC_FLAGS, e.action.unitFlag.flag);


            break;
        }
        case SMART_ACTION_REMOVE_DYNAMIC_FLAG:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->RemoveFlag(UNIT_DYNAMIC_FLAGS, e.action.unitFlag.flag);


            break;
        }
        case SMART_ACTION_JUMP_TO_POS: //LK
        {
            /*
            for (WorldObject* target : targets)
                if (Creature* creature = target->ToCreature())
                    creature->GetMotionMaster()->MoveJump(e.target.x, e.target.y, e.target.z, 0.0f, float(e.action.jump.speedxy), float(e.action.jump.speedz)); // @todo add optional jump orientation support?
            */
            break;
        }
        case SMART_ACTION_GO_SET_LOOT_STATE:
        {
            for (WorldObject* target : targets)
                if (IsGameObject(target))
                    target->ToGameObject()->SetLootState((LootState)e.action.setGoLootState.state);


            break;
        }
        case SMART_ACTION_SEND_TARGET_TO_TARGET:
        {
            WorldObject* ref = GetBaseObject();
            if (!ref)
                ref = unit;

            if (!ref)
                break;

            ObjectVector const* storedTargets = GetStoredTargetVector(e.action.sendTargetToTarget.id, *ref);
            if (!storedTargets)
                break;

            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                {
                    if (SmartAI* ai = CAST_AI(SmartAI, target->ToCreature()->AI()))
                        ai->GetScript()->StoreTargetList(ObjectVector(*storedTargets), e.action.sendTargetToTarget.id);   // store a copy of target list
                    else
                        TC_LOG_ERROR("server.loading","SmartScript: Action target for SMART_ACTION_SEND_TARGET_TO_TARGET is not using SmartAI, skipping");
                }
                else if (IsGameObject(target))
                {
                    if (SmartGameObjectAI* ai = CAST_AI(SmartGameObjectAI, target->ToGameObject()->AI()))
                        ai->GetScript()->StoreTargetList(ObjectVector(*storedTargets), e.action.sendTargetToTarget.id);   // store a copy of target list
                    else
                        TC_LOG_ERROR("server.loading","SmartScript: Action target for SMART_ACTION_SEND_TARGET_TO_TARGET is not using SmartGameObjectAI, skipping");
                }
            }


            break;
        }
        case SMART_ACTION_SEND_GOSSIP_MENU: 
        {
            if (!GetBaseObject() || !IsSmart())
                break;

            TC_LOG_DEBUG("scripts.ai","SmartScript::ProcessAction:: SMART_ACTION_SEND_GOSSIP_MENU: gossipMenuId %d, gossipNpcTextId %d",
                e.action.sendGossipMenu.gossipMenuId, e.action.sendGossipMenu.gossipNpcTextId);

            // override default gossip
            if (me)
                ENSURE_AI(SmartAI, me->AI())->SetGossipReturn(true);
            else if (go)
                ENSURE_AI(SmartGameObjectAI, go->AI())->SetGossipReturn(true);

            for (WorldObject* target : targets)
            {
                if (Player* player = target->ToPlayer())
                {
                    if (e.action.sendGossipMenu.gossipMenuId)
                        player->PrepareGossipMenu(GetBaseObject(), e.action.sendGossipMenu.gossipMenuId, true);
                    else
                        ClearGossipMenuFor(player);

                    //default to default menu text if no text id given in action
                    uint32 textId = e.action.sendGossipMenu.gossipNpcTextId;
                    if (!textId)
                        textId = player->GetDefaultGossipMenuForSource(GetBaseObject());

                    player->SEND_GOSSIP_MENU_TEXTID(textId, GetBaseObject()->GetGUID());
                }
            }


            break;
        }
        case SMART_ACTION_SET_HOME_POS:
        {
            for (WorldObject* target : targets)
            {
                if (IsCreature(target))
                {
                    if (e.GetTargetType() == SMART_TARGET_SELF)
                        target->ToCreature()->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    else if (e.GetTargetType() == SMART_TARGET_POSITION)
                        target->ToCreature()->SetHomePosition(e.target.x, e.target.y, e.target.z, e.target.o);
                    else if (e.GetTargetType() == SMART_TARGET_CREATURE_RANGE || e.GetTargetType() == SMART_TARGET_CREATURE_GUID ||
                             e.GetTargetType() == SMART_TARGET_CREATURE_DISTANCE || e.GetTargetType() == SMART_TARGET_GAMEOBJECT_RANGE ||
                             e.GetTargetType() == SMART_TARGET_GAMEOBJECT_GUID || e.GetTargetType() == SMART_TARGET_GAMEOBJECT_DISTANCE ||
                             e.GetTargetType() == SMART_TARGET_CLOSEST_CREATURE || e.GetTargetType() == SMART_TARGET_CLOSEST_GAMEOBJECT ||
                             e.GetTargetType() == SMART_TARGET_OWNER_OR_SUMMONER || e.GetTargetType() == SMART_TARGET_ACTION_INVOKER ||
                             e.GetTargetType() == SMART_TARGET_CLOSEST_ENEMY || e.GetTargetType() == SMART_TARGET_CLOSEST_FRIENDLY)
                    {
                        target->ToCreature()->SetHomePosition(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation());
                    }
                    else
                        TC_LOG_ERROR("server.loading","SmartScript: Action target for SMART_ACTION_SET_HOME_POS is invalid, skipping");
                }
            }


            break;
        }
        case SMART_ACTION_SET_HEALTH_REGEN:
        {
            for (WorldObject* target : targets)
                if (IsCreature(target))
                    target->ToCreature()->setRegeneratingHealth(e.action.setHealthRegen.regenHealth);


            break;
        }
        case SMART_ACTION_SET_ROOT:
        {
            for (WorldObject* target : targets)
                if (IsCreature(target))
                    target->ToCreature()->SetControlled(e.action.setRoot.root, UNIT_STATE_ROOT);


            break;
        }
        case SMART_ACTION_SET_GO_FLAG:
        {
            for (WorldObject* target : targets)
                if (IsGameObject(target))
                    target->ToGameObject()->SetUInt32Value(GAMEOBJECT_FLAGS, e.action.goFlag.flag);


            break;
        }
        case SMART_ACTION_ADD_GO_FLAG:
        {
            for (WorldObject* target : targets)
                if (IsGameObject(target))
                    target->ToGameObject()->SetFlag(GAMEOBJECT_FLAGS, e.action.goFlag.flag);


            break;
        }
        case SMART_ACTION_REMOVE_GO_FLAG:
        {
            for (WorldObject* target : targets)
                if (IsGameObject(target))
                    target->ToGameObject()->RemoveFlag(GAMEOBJECT_FLAGS, e.action.goFlag.flag);


            break;
        }
        case SMART_ACTION_SUMMON_CREATURE_GROUP:
        {
            /*
            std::list<TempSummon*> summonList;
            GetBaseObject()->SummonCreatureGroup(e.action.creatureGroup.group, &summonList);

            for (std::list<TempSummon*>::const_iterator itr = summonList.begin(); itr != summonList.end(); ++itr)
                if (unit && e.action.creatureGroup.attackInvoker)
                    target->AI()->AttackStart(unit);

            */
            break;
        }
        case SMART_ACTION_SET_POWER:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->SetPower(Powers(e.action.power.powerType), e.action.power.newPower);

            break;
        }
        case SMART_ACTION_ADD_POWER:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->SetPower(Powers(e.action.power.powerType), target->ToUnit()->GetPower(Powers(e.action.power.powerType)) + e.action.power.newPower);
            break;
        }
        case SMART_ACTION_REMOVE_POWER:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->SetPower(Powers(e.action.power.powerType), target->ToUnit()->GetPower(Powers(e.action.power.powerType)) - e.action.power.newPower);
            break;
        }
        case SMART_ACTION_GAME_EVENT_STOP:
        {
            uint32 eventId = e.action.gameEventStop.id;
            if (!sGameEventMgr->IsActiveEvent(eventId))
            {
                TC_LOG_ERROR("scripts.ai","SmartScript::ProcessAction: At case SMART_ACTION_GAME_EVENT_STOP, inactive event (id: %u)", eventId);
                break;
            }
            sGameEventMgr->StopEvent(eventId, true);
            break;
        }
        case SMART_ACTION_GAME_EVENT_START:
        {
            uint32 eventId = e.action.gameEventStart.id;
            if (sGameEventMgr->IsActiveEvent(eventId))
            {
                TC_LOG_ERROR("scripts.ai","SmartScript::ProcessAction: At case SMART_ACTION_GAME_EVENT_START, already activated event (id: %u)", eventId);
                break;
            }
            sGameEventMgr->StartEvent(eventId, true);
            break;
        }
        case SMART_ACTION_START_CLOSEST_WAYPOINT:
        {
            uint32 waypoints[SMART_ACTION_PARAM_COUNT];
            waypoints[0] = e.action.closestWaypointFromList.wp1;
            waypoints[1] = e.action.closestWaypointFromList.wp2;
            waypoints[2] = e.action.closestWaypointFromList.wp3;
            waypoints[3] = e.action.closestWaypointFromList.wp4;
            waypoints[4] = e.action.closestWaypointFromList.wp5;
            waypoints[5] = e.action.closestWaypointFromList.wp6;
            float distanceToClosest = std::numeric_limits<float>::max();
            std::pair<uint32, uint32> closest = { 0, 0 };

            for (WorldObject* _target : targets)
            {
                if (Creature* creature = _target->ToCreature())
                {
                    if (IsSmart(creature))
                    {
                        for (uint32 pathId : waypoints)
                        {
                            if (!pathId)
                                continue;

                            auto path = sSmartWaypointMgr->GetPath(pathId);
                            if (!path || path->nodes.empty())
                                continue;

                            for (auto itr = path->nodes.begin(); itr != path->nodes.end(); ++itr)
                            {
                                WaypointNode const waypoint = *itr;
                                float distamceToThisNode = creature->GetDistance(waypoint.x, waypoint.y, waypoint.z);
                                if (distamceToThisNode < distanceToClosest)
                                {
                                    distanceToClosest = distamceToThisNode;
                                    closest.first = pathId;
                                    closest.second = waypoint.id;
                                }
                            }
                        }

                        if (closest.first != 0)
                            ENSURE_AI(SmartAI, creature->AI())->StartPath(false, closest.first, true, nullptr, closest.second);
                    }
                }
            }

            break;
        }
        case SMART_ACTION_LOAD_PATH:
        {
            for (WorldObject* _target : targets)
            {
                 if (Creature* target = _target->ToCreature())
                 {
                     //path Id can be 0, if then remove path and set motion type to 0
                     target->LoadPath(e.action.setPath.pathId);
                     if(e.action.setPath.pathId)
                         target->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                     else
                         target->SetDefaultMovementType(IDLE_MOTION_TYPE);

                     target->GetMotionMaster()->Initialize();
                 }
            }

            break;
        }
        case SMART_ACTION_PREVENT_MOVE_HOME:
        {
            for (auto target : targets)
                if (Creature* creatureTarget = target->ToCreature())
                    creatureTarget->SetHomeless(e.action.preventMoveHome.prevent);
                    //CAST_AI(SmartAI, target->AI())->SetPreventMoveHome(e.action.preventMoveHome.prevent);

 
            break;
        }
        case SMART_ACTION_ADD_TO_FORMATION:
        {
            if(!me->GetFormation())
                sCreatureGroupMgr->AddCreatureToGroup(me->GetGUIDLow(), me);

            for (auto target : targets)
                if (Creature* creatureTarget = target->ToCreature())
                    sCreatureGroupMgr->AddCreatureToGroup(me->GetGUIDLow(), creatureTarget);

            break;
        }
        case SMART_ACTION_REMOVE_FROM_FORMATION:
        {
            for (auto target : targets)
                if (Creature* creatureTarget = target->ToCreature())
                    sCreatureGroupMgr->RemoveCreatureFromGroup(me->GetGUIDLow(), creatureTarget);

            break;
        }
        case SMART_ACTION_BREAK_FORMATION:
            sCreatureGroupMgr->BreakFormation(me);
            break;
        case SMART_ACTION_SET_MECHANIC_IMMUNITY:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->ApplySpellImmune(0, IMMUNITY_MECHANIC, e.action.mechanicImmunity.type, e.action.mechanicImmunity.apply);

            break;
        }
        case SMART_ACTION_SET_SPELL_IMMUNITY:
        {
            for (WorldObject* target : targets)
                if (IsUnit(target))
                    target->ToUnit()->ApplySpellImmune(e.action.spellImmunity.id, IMMUNITY_ID, 0, e.action.spellImmunity.apply);

            break;
        }
        case SMART_ACTION_SET_EVENT_TEMPLATE_PHASE:
        {
            for (WorldObject* target : targets)
            {
                if (!IsUnit(target))
                    break;

                Creature* c = target->ToCreature();
                if (!IsSmart(c))
                    break;

                ENSURE_AI(SmartAI, c->AI())->GetScript()->SetTemplatePhase(e.action.setEventPhase.phase);
                TC_LOG_DEBUG("scripts.ai", "SmartScript::ProcessAction:: SMART_ACTION_SET_EVENT_PHASE: Creature %u set event template phase %u",
                    target->GetGUIDLow(), e.action.setEventPhase.phase);
            }
            break;
        }
        case SMART_ACTION_STORE_PHASE:
            mStoredPhase = GetPhase();
            break;
        case SMART_ACTION_RESTORE_PHASE:
            SetPhase(mStoredPhase);
            break;
        default:
            TC_LOG_ERROR("sql.sql","SmartScript::ProcessAction: Entry %d SourceType %u, Event %u, Unhandled Action type %u", e.entryOrGuid, e.GetScriptType(), e.event_id, e.GetActionType());
            break;
    }

    if (e.link && e.link != e.event_id)
    {
        SmartScriptHolder linked = FindLinkedEvent(e.link);
        if (linked.GetActionType() && linked.GetEventType() == SMART_EVENT_LINK)
            ProcessEvent(linked, unit, var0, var1, bvar, spell, gob);
        else
            TC_LOG_ERROR("sql.sql","SmartScript::ProcessAction: Entry %d SourceType %u, Event %u, Link Event %u not found or invalid, skipped.", e.entryOrGuid, e.GetScriptType(), e.event_id, e.link);
    }
}

void SmartScript::ProcessTimedAction(SmartScriptHolder& e, uint32 const& min, uint32 const& max, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob)
{
    // We may want to execute action rarely and because of this if condition is not fulfilled the action will be rechecked in a long time
    if (sConditionMgr->IsObjectMeetingSmartEventConditions(e.entryOrGuid, e.event_id, e.source_type, unit, GetBaseObject()))
    {
        RecalcTimer(e, min, max);
        ProcessAction(e, unit, var0, var1, bvar, spell, gob);
    }
    else
        RecalcTimer(e, std::min<uint32>(min, 5000), std::min<uint32>(min, 5000));
}

void SmartScript::InstallTemplate(SmartScriptHolder const& e)
{
    if (!GetBaseObject())
        return;
    if (mTemplate)
    {
        TC_LOG_ERROR("sql.sql","SmartScript::InstallTemplate: Entry %d SourceType %u AI Template can not be set more then once, skipped.", e.entryOrGuid, e.GetScriptType());
        return;
    }
    mTemplate = (SMARTAI_TEMPLATE)e.action.installTtemplate.id;
    switch ((SMARTAI_TEMPLATE)e.action.installTtemplate.id)
    {
        case SMARTAI_TEMPLATE_CASTER:
            {
                uint32 spellID = e.action.installTtemplate.param1;
                uint32 repeatMin = e.action.installTtemplate.param2;
                uint32 repeatMax = e.action.installTtemplate.param3;
                if (repeatMin > repeatMax)
                    repeatMax = repeatMin;

                uint32 range = e.action.installTtemplate.param4;
                uint32 manaPercent = e.action.installTtemplate.param5;
                // (valid values: 10-99)
                manaPercent = std::max(std::min(manaPercent, uint32(99)), uint32(10));

                //only cast spell in phase 1
                AddEvent(SMART_EVENT_UPDATE_IC, 0, 0, 0, repeatMin, repeatMax, SMART_ACTION_CAST, spellID, e.target.raw.param1, 0, 0, 0, 0, SMART_TARGET_VICTIM, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                if (range)
                {
                    //enable movement (switch to phase 1) when out of given range
                    AddEvent(SMART_EVENT_RANGE, 0, range, 300, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 1, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                    //disable movement when in given range
                    AddEvent(SMART_EVENT_RANGE, 0, 0, range, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 0, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                }
                // start casting (go to phase 1) when at least at manaPercent
                AddEvent(SMART_EVENT_MANA_PCT, 0, manaPercent, 100, 1000, 1000, SMART_ACTION_SET_EVENT_TEMPLATE_PHASE, 1, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0));
                // stop casting (go back to phase 0) when under manaPercent
                AddEvent(SMART_EVENT_MANA_PCT, 0, 0, manaPercent, 1000, 1000, SMART_ACTION_SET_EVENT_TEMPLATE_PHASE, 0, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0));
                // enable movement when under given mana
                AddEvent(SMART_EVENT_MANA_PCT, 0, 0, manaPercent, 1000, 1000, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 1, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0));
                break;
            }
        case SMARTAI_TEMPLATE_CASTER_SUN:
            {
                uint32 spellID = e.action.installTtemplate.param1;
                uint32 repeatMin = e.action.installTtemplate.param2;
                uint32 repeatMax = e.action.installTtemplate.param3;
                if (repeatMin > repeatMax)
                    repeatMax = repeatMin;

                uint32 castFlags = e.action.installTtemplate.param4;

                auto spellInfo = sSpellMgr->GetSpellInfo(spellID);
                if (spellInfo == nullptr)
                {
                    SMARTAI_DB_ERROR(e.entryOrGuid, "SmartScript: Entry %d SourceType %u Event %u Action %u is using invalid spell %u", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType(), spellID);
                    break;
                }

                uint32 range = spellInfo->GetMaxRange();
                uint32 manaPercent = uint32((float(spellInfo->ManaCost) / float(me->GetMaxPower(POWER_MANA))) * 100.0f) + 1;

                //only cast spell in phase 1
                AddEvent(SMART_EVENT_UPDATE_IC, 0, 0, 0, repeatMin, repeatMax, SMART_ACTION_CAST, spellID, castFlags, 0, 0, 0, 0, SMART_TARGET_VICTIM, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                //enable movement (switch to phase 1) when out of given range
                AddEvent(SMART_EVENT_RANGE, 0, range, 300, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 1, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                //disable movement when in given range
                AddEvent(SMART_EVENT_RANGE, 0, 0, range, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 0, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                // start casting (go to phase 1) when at least at manaPercent
                AddEvent(SMART_EVENT_MANA_PCT, 0, manaPercent, 100, 1000, 1000, SMART_ACTION_SET_EVENT_TEMPLATE_PHASE, 1, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0));
                // stop casting (go back to phase 0) when under manaPercent
                AddEvent(SMART_EVENT_MANA_PCT, 0, 0, manaPercent, 1000, 1000, SMART_ACTION_SET_EVENT_TEMPLATE_PHASE, 0, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0));
                // enable movement when under given mana
                AddEvent(SMART_EVENT_MANA_PCT, 0, 0, manaPercent, 1000, 1000, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 1, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, 0, 0, SmartPhaseMask(0));
                break;
            }
        case SMARTAI_TEMPLATE_TURRET:
            {
                AddEvent(SMART_EVENT_UPDATE_IC, 0, 0, 0, e.action.installTtemplate.param2, e.action.installTtemplate.param3, SMART_ACTION_CAST, e.action.installTtemplate.param1, e.target.raw.param1, 0, 0, 0, 0, SMART_TARGET_VICTIM, 0, 0, 0, SmartPhaseMask(0));
                AddEvent(SMART_EVENT_JUST_CREATED, 0, 0, 0, 0, 0, SMART_ACTION_ALLOW_COMBAT_MOVEMENT, 0, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, SmartPhaseMask(0));
                break;
            }
        case SMARTAI_TEMPLATE_CAGED_NPC_PART:
            {
                if (!me)
                    return;
                //store cage as id1
                AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_STORE_TARGET_LIST, 1, 0, 0, 0, 0, 0, SMART_TARGET_CLOSEST_GAMEOBJECT, e.action.installTtemplate.param1, 10, 0, SmartPhaseMask(0));

                 //reset(close) cage on hostage(me) respawn
                AddEvent(SMART_EVENT_UPDATE, SMART_EVENT_FLAG_NOT_REPEATABLE, 0, 0, 0, 0, SMART_ACTION_RESET_GOBJECT, 0, 0, 0, 0, 0, 0, SMART_TARGET_GAMEOBJECT_DISTANCE, e.action.installTtemplate.param1, 5, 0, SmartPhaseMask(0));

                AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_SET_RUN, e.action.installTtemplate.param3, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, SmartPhaseMask(0));
                AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_SET_EVENT_TEMPLATE_PHASE, 1, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, SmartPhaseMask(0));

                AddEvent(SMART_EVENT_UPDATE, SMART_EVENT_FLAG_NOT_REPEATABLE, 1000, 1000, 0, 0, SMART_ACTION_MOVE_OFFSET, 0, 0, 0, 0, 0, 0, SMART_TARGET_SELF, 0, e.action.installTtemplate.param4, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                //phase 1: give quest credit on movepoint reached
                AddEvent(SMART_EVENT_MOVEMENTINFORM, 0, POINT_MOTION_TYPE, SMART_RANDOM_POINT, 0, 0, SMART_ACTION_SET_DATA, 0, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 1, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                //phase 1: despawn after time on movepoint reached
                AddEvent(SMART_EVENT_MOVEMENTINFORM, 0, POINT_MOTION_TYPE, SMART_RANDOM_POINT, 0, 0, SMART_ACTION_FORCE_DESPAWN, e.action.installTtemplate.param2, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));

                if (sCreatureTextMgr->TextExist(me->GetEntry(), (uint8)e.action.installTtemplate.param5))
                    AddEvent(SMART_EVENT_MOVEMENTINFORM, 0, POINT_MOTION_TYPE, SMART_RANDOM_POINT, 0, 0, SMART_ACTION_TALK, e.action.installTtemplate.param5, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, SmartPhaseMask(0), SmartPhaseMask(1));
                break;
            }
        case SMARTAI_TEMPLATE_CAGED_GO_PART:
            {
                if (!go)
                    return;
                //store hostage as id1
                AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_STORE_TARGET_LIST, 1, 0, 0, 0, 0, 0, SMART_TARGET_CLOSEST_CREATURE, e.action.installTtemplate.param1, 10, 0, SmartPhaseMask(0));
                //store invoker as id2
                AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_STORE_TARGET_LIST, 2, 0, 0, 0, 0, 0, SMART_TARGET_NONE, 0, 0, 0, SmartPhaseMask(0));
                //signal hostage
                AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_SET_DATA, 0, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 1, 0, 0, SmartPhaseMask(0));
                //when hostage raeched end point, give credit to invoker
                if (e.action.installTtemplate.param2)
                    AddEvent(SMART_EVENT_DATA_SET, 0, 0, 0, 0, 0, SMART_ACTION_CALL_KILLEDMONSTER, e.action.installTtemplate.param1, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 2, 0, 0, SmartPhaseMask(0));
                else
                    AddEvent(SMART_EVENT_GO_STATE_CHANGED, 0, 2, 0, 0, 0, SMART_ACTION_CALL_KILLEDMONSTER, e.action.installTtemplate.param1, 0, 0, 0, 0, 0, SMART_TARGET_STORED, 2, 0, 0, SmartPhaseMask(0));
                break;
            }
        case SMARTAI_TEMPLATE_BASIC:
        default:
            return;
    }
}

void SmartScript::AddEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, SmartPhaseMask phaseMask, SmartPhaseMask templatePhaseMask)
{
    mInstallEvents.push_back(CreateEvent(e, event_flags, event_param1, event_param2, event_param3, event_param4, action, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, t, target_param1, target_param2, target_param3, phaseMask, templatePhaseMask));
}

SmartScriptHolder SmartScript::CreateEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, SmartPhaseMask phaseMask, SmartPhaseMask templatePhaseMask)
{
    SmartScriptHolder script;
    script.event.type = e;
    script.event.raw.param1 = event_param1;
    script.event.raw.param2 = event_param2;
    script.event.raw.param3 = event_param3;
    script.event.raw.param4 = event_param4;
    script.event.event_phase_mask = phaseMask;
    script.event.event_template_phase_mask = templatePhaseMask;
    script.event.event_flags = event_flags;
    script.event.event_chance = 100;

    script.action.type = action;
    script.action.raw.param1 = action_param1;
    script.action.raw.param2 = action_param2;
    script.action.raw.param3 = action_param3;
    script.action.raw.param4 = action_param4;
    script.action.raw.param5 = action_param5;
    script.action.raw.param6 = action_param6;

    script.target.type = t;
    script.target.raw.param1 = target_param1;
    script.target.raw.param2 = target_param2;
    script.target.raw.param3 = target_param3;

    script.source_type = SMART_SCRIPT_TYPE_CREATURE;
    InitTimer(script);
    return script;
}

bool SmartScript::IsTargetAllowedByTargetFlags(WorldObject const* target, SMARTAI_TARGETS_FLAGS flags, WorldObject const* caster, SMARTAI_TARGETS type)
{
    if (Creature const* c = target->ToCreature())
    {
        if (((flags & SMART_TARGET_FLAG_IN_COMBAT_ONLY)
            && (!c->IsInCombat()))
            || ((flags & SMART_TARGET_FLAG_OUT_OF_COMBAT_ONLY)
                && (c->IsInCombat()))
            || ((flags & SMART_TARGET_FLAG_CAN_TARGET_DEAD) == 0
                && (c->IsDead())
                && type != SMART_TARGET_SELF //still allow self cast
                )
            //this next one should be moved outside of this condition if/when WorldObject get a GetFaction function
            || ((flags & SMART_TARGET_FLAG_SAME_FACTION)
                && (c->GetFaction() != me->GetFaction()))
            )
        {
            return false;
        }
    }
    return true;
}

void SmartScript::FilterByTargetFlags(SMARTAI_TARGETS type, SMARTAI_TARGETS_FLAGS flags, ObjectVector& list, WorldObject const* caster)
{
    if (flags & SMART_TARGET_FLAG_UNIQUE_TARGET)
        if (!list.empty())
        {
            list.resize(1);
            return;
        }

    for (auto itr = list.begin(); itr != list.end(); )
    {
        if (!IsTargetAllowedByTargetFlags(*itr, flags, caster, type))
        {
            itr = list.erase(itr);
            continue;
        }
        itr++;
    }
}

void SmartScript::GetTargets(ObjectVector& targets, SmartScriptHolder const& e, Unit* invoker /*= NULL*/)
{
    Unit* scriptTrigger = nullptr;
    if (invoker)
        scriptTrigger = invoker;
    else if (Unit* tempLastInvoker = GetLastInvoker())
        scriptTrigger = tempLastInvoker;

    WorldObject* baseObject = GetBaseObject();
    switch (e.GetTargetType())
    {
        case SMART_TARGET_SELF:
            if (baseObject)
                targets.push_back(baseObject);
            break;
        case SMART_TARGET_VICTIM:
            if (me)
                if (Unit* victim = me->GetVictim())
                    targets.push_back(victim);
            break;
        case SMART_TARGET_HOSTILE_SECOND_AGGRO:
            if (me)
            {
                if (e.target.hostilRandom.powerType)
                {
                    if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 1, PowerUsersSelector(me, Powers(e.target.hostilRandom.powerType - 1), (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0)))
                        targets.push_back(u);
                }
                else if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 1, (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0))
                    targets.push_back(u);
            }
            break;
        case SMART_TARGET_HOSTILE_LAST_AGGRO:
            if (me)
            {
                if (e.target.hostilRandom.powerType)
                {
                    if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_BOTTOMAGGRO, 0, PowerUsersSelector(me, Powers(e.target.hostilRandom.powerType - 1), (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0)))
                        targets.push_back(u);
                }
                else if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_BOTTOMAGGRO, 0, (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0))
                    targets.push_back(u);
            }
            break;
        case SMART_TARGET_HOSTILE_RANDOM:
            if (me)
            {
                if (e.target.hostilRandom.powerType)
                {
                    if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, PowerUsersSelector(me, Powers(e.target.hostilRandom.powerType - 1), (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0)))
                        targets.push_back(u);
                }
                else if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0))
                    targets.push_back(u);
            }
            break;
        case SMART_TARGET_HOSTILE_RANDOM_NOT_TOP:
            if (me)
            {
                if (e.target.hostilRandom.powerType)
                {
                    if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1, PowerUsersSelector(me, Powers(e.target.hostilRandom.powerType - 1), (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0)))
                        targets.push_back(u);
                }
                else if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1, (float)e.target.hostilRandom.maxDist, e.target.hostilRandom.playerOnly != 0))
                    targets.push_back(u);
            }
            break;
        case SMART_TARGET_FARTHEST:
            if (me)
            {
                if (Unit* u = me->AI()->SelectTarget(SELECT_TARGET_FARTHEST, 0, FarthestTargetSelector(me, (float)e.target.farthest.maxDist, e.target.farthest.playerOnly != 0, e.target.farthest.isInLos != 0)))
                    targets.push_back(u);
            }
            break;
        case SMART_TARGET_ACTION_INVOKER:
            if (scriptTrigger)
                targets.push_back(scriptTrigger);
            break;
#ifdef LICH_KING
        case SMART_TARGET_ACTION_INVOKER_VEHICLE:
            if (scriptTrigger && scriptTrigger->GetVehicle() && scriptTrigger->GetVehicle()->GetBase())
                targets.push_back(scriptTrigger->GetVehicle()->GetBase());
            break;
#endif
        case SMART_TARGET_INVOKER_PARTY:
            if (scriptTrigger)
            {
                if (Player* player = scriptTrigger->ToPlayer())
                {
                    if (Group* group = player->GetGroup())
                    {
                        for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                            if (Player* member = groupRef->GetSource())
                                if (member->IsInMap(player))
                                    targets.push_back(member);
                    }
                    // We still add the player to the list if there is no group. If we do
                    // this even if there is a group (thus the else-check), it will add the
                    // same player to the list twice. We don't want that to happen.
                    else
                        targets.push_back(scriptTrigger);
                }
            }
            break;
        case SMART_TARGET_CREATURE_RANGE:
        {
            ObjectVector units;
            GetWorldObjectsInDist(units, (float)e.target.unitRange.maxDist);
            for (WorldObject* unit : units)
            {
                if (!IsCreature(unit))
                    continue;

                if (me && me->GetGUID() == unit->GetGUID())
                    continue;

                if (e.target.unitRange.creature && unit->ToCreature()->GetEntry() != e.target.unitRange.creature)
                    continue;

                if(!baseObject->IsInRange(unit, (float)e.target.unitRange.minDist, (float)e.target.unitRange.maxDist))
                    continue;

                targets.push_back(unit);
            }

            break;
        }
        case SMART_TARGET_CREATURE_DISTANCE:
        {
            ObjectVector units;
            GetWorldObjectsInDist(units, static_cast<float>(e.target.unitDistance.dist));

            for (WorldObject* unit : units)
            {
                if (!IsCreature(unit))
                    continue;

                if (me && me->GetGUID() == unit->GetGUID())
                    continue;

                // check alive state - 1 alive, 2 dead, 0 both
                if (uint32 state = e.target.unitDistance.livingState)
                {
                    if (unit->ToCreature()->IsAlive() && state == 2)
                        continue;
                    if (!unit->ToCreature()->IsAlive() && state == 1)
                        continue;
                }

                if ((e.target.unitDistance.creature && unit->ToCreature()->GetEntry() == e.target.unitDistance.creature) || !e.target.unitDistance.creature)
                    targets.push_back(unit);
            }

            break;
        }
        case SMART_TARGET_GAMEOBJECT_DISTANCE:
        {
            ObjectVector gobjects;
            GetWorldObjectsInDist(gobjects, static_cast<float>(e.target.goDistance.dist));

            for (WorldObject* gob : gobjects)
            {
                if (!IsGameObject(gob))
                    continue;

                if (go && go->GetGUID() == gob->GetGUID())
                    continue;

                if ((e.target.goDistance.entry && gob->ToGameObject()->GetEntry() == e.target.goDistance.entry) || !e.target.goDistance.entry)
                    targets.push_back(gob);
            }

            break;
        }
        case SMART_TARGET_GAMEOBJECT_RANGE:
        {
            ObjectVector gobjects;
            GetWorldObjectsInDist(gobjects, static_cast<float>(e.target.goDistance.dist));

            for (WorldObject* gob : gobjects)
            {
                if (!IsGameObject(gob))
                    continue;

                if (go && go->GetGUID() == gob->GetGUID())
                    continue;

                if (((e.target.goRange.entry && IsGameObject(gob) && gob->ToGameObject()->GetEntry() == e.target.goRange.entry) || !e.target.goRange.entry) && baseObject->IsInRange(gob, (float)e.target.goRange.minDist, (float)e.target.goRange.maxDist))
                    targets.push_back(gob);
            }

            break;
        }
        case SMART_TARGET_CREATURE_GUID:
        {
            Creature* target = nullptr;
            if (!scriptTrigger && !baseObject)
            {
                TC_LOG_ERROR("server.loading","SMART_TARGET_CREATURE_GUID can not be used without invoker");
                break;
            }

            target = FindCreatureNear(scriptTrigger ? scriptTrigger : baseObject, e.target.unitGUID.spawnId);

            if (target && (!e.target.unitGUID.entry || target->GetEntry() == e.target.unitGUID.entry))
                targets.push_back(target);
            break;
        }
        case SMART_TARGET_GAMEOBJECT_GUID:
        {
            GameObject* target = nullptr;
            if (!scriptTrigger && !baseObject)
            {
                TC_LOG_ERROR("server.loading","SMART_TARGET_GAMEOBJECT_GUID can not be used without invoker");
                break;
            }

            target = FindGameObjectNear(scriptTrigger ? scriptTrigger : baseObject, e.target.goGUID.spawnId);

            if (target && (!e.target.goGUID.entry || target->GetEntry() == e.target.goGUID.entry))
                targets.push_back(target);
            break;
        }
        case SMART_TARGET_PLAYER_RANGE:
        {
            ObjectVector units;
            GetWorldObjectsInDist(units, static_cast<float>(e.target.playerRange.maxDist));

            if (!units.empty() && baseObject)
                for (WorldObject* unit : units)
                    if (IsPlayer(unit) && baseObject->IsInRange(unit, float(e.target.playerRange.minDist), float(e.target.playerRange.maxDist)))
                        targets.push_back(unit);
            break;
        }
        case SMART_TARGET_PLAYER_DISTANCE:
        {
            ObjectVector units;
            GetWorldObjectsInDist(units, static_cast<float>(e.target.playerDistance.dist));

            for (WorldObject* unit : units)
                if (IsPlayer(unit))
                    targets.push_back(unit);
            break;
        }
        case SMART_TARGET_PLAYER_CASTING_DISTANCE:
        {
            ObjectVector units;
            GetWorldObjectsInDist(units, static_cast<float>(e.target.playerDistance.dist));

            for (WorldObject* unit : units)
                if (IsPlayer(unit) && unit->ToPlayer()->HasUnitState(UNIT_STATE_CASTING))
                    targets.push_back(unit);

            break;
        }
        case SMART_TARGET_FRIENDLY_HEALTH_PCT:
        {
            ObjectVector units;
            GetWorldObjectsInDist(units, static_cast<float>(e.target.playerDistance.dist));

            for (WorldObject* unit : units)
                if (unit->isType(TYPEMASK_UNIT)
                    && baseObject->IsWithinDist(unit, (float)e.target.friendlyHealthPct.maxDist)
                    && (unit->ToUnit()->GetHealthPct() < e.target.friendlyHealthPct.percentBelow)
                    && (!e.target.friendlyHealthPct.entry || (unit->ToCreature() && unit->ToCreature()->GetEntry() == e.target.friendlyHealthPct.entry))
                   )
                    targets.push_back(unit);
            break;
        }
        case SMART_TARGET_STORED:
        {
             WorldObject* ref = GetBaseObject();
            if (!ref)
                ref = scriptTrigger;

            if (ref)
                if (ObjectVector const* stored = GetStoredTargetVector(e.target.stored.id, *ref))
                    targets.assign(stored->begin(), stored->end());
            break;
        }
        case SMART_TARGET_CLOSEST_CREATURE:
        {
            if (Creature* target = GetClosestCreatureWithEntry(baseObject, e.target.closest.entry, float(e.target.closest.dist ? e.target.closest.dist : 100), !e.target.closest.dead))
                targets.push_back(target);
            break;
        }
        case SMART_TARGET_CLOSEST_GAMEOBJECT:
        {
            if (GameObject* target = GetClosestGameObjectWithEntry(baseObject, e.target.closest.entry, float(e.target.closest.dist ? e.target.closest.dist : 100)))
                targets.push_back(target);
            break;
        }
        case SMART_TARGET_CLOSEST_PLAYER:
        {
            if (WorldObject* obj = GetBaseObject())
                if (Player* target = obj->SelectNearestPlayer(float(e.target.playerDistance.dist)))
                    targets.push_back(target);
            break;
        }
        case SMART_TARGET_OWNER_OR_SUMMONER:
        {
            if (me)
            {
                uint64 charmerOrOwnerGuid = me->GetCharmerOrOwnerGUID();

                /* TC
                if (!charmerOrOwnerGuid)
                    if (TempSummon* tempSummon = me->ToTempSummon())
                        if (Unit* summoner = tempSummon->GetSummoner())
                            charmerOrOwnerGuid = summoner->GetGUID();
                            */
                if (!charmerOrOwnerGuid)
                    charmerOrOwnerGuid = me->GetCreatorGUID();

                if (Unit* owner = ObjectAccessor::GetUnit(*me, charmerOrOwnerGuid))
                    targets.push_back(owner);
            }
            else if (go)
            {
                if (Unit* owner = ObjectAccessor::GetUnit(*go, go->GetOwnerGUID()))
                    targets.push_back(owner);
            }

            // Get owner of owner
            if (e.target.owner.useCharmerOrOwner && !targets.empty())
            {
                Unit* owner = targets.front()->ToUnit();
                targets.clear();

                if (Unit* base = ObjectAccessor::GetUnit(*owner, owner->GetCharmerOrOwnerGUID()))
                    targets.push_back(base);
            }
            break;
        }
        case SMART_TARGET_THREAT_LIST:
        {
            if (me)
            {
                auto threatList = me->GetThreatManager().getThreatList();
                for (auto itr : threatList)
                    if (Unit* temp = ObjectAccessor::GetUnit(*me, itr->getUnitGuid()))
                        if (e.target.hostilRandom.maxDist == 0 || me->IsWithinCombatRange(temp, (float)e.target.hostilRandom.maxDist))
                            targets.push_back(temp);
            }
            break;
        }
        case SMART_TARGET_CLOSEST_ENEMY:
        {
            if (me)
                if (Unit* target = me->SelectNearestTarget(e.target.closestAttackable.maxDist, e.target.closestAttackable.playerOnly, e.target.closestAttackable.farthest))
                    targets.push_back(target);

            break;
        }
        case SMART_TARGET_CLOSEST_FRIENDLY:
        {
            if (me)
                if (Unit* target = DoFindClosestOrFurthestFriendlyInRange(e.target.closestFriendly.maxDist, e.target.closestFriendly.playerOnly, !e.target.closestFriendly.farthest))
                    targets.push_back(target);

            break;
        }
        case SMART_TARGET_LOOT_RECIPIENTS:
        {
            if (me)
            {
                if (Group* lootGroup = me->GetLootRecipientGroup())
                {
                    for (GroupReference* it = lootGroup->GetFirstMember(); it != nullptr; it = it->next())
                        if (Player* recipient = it->GetSource())
                            if (recipient->IsInMap(me))
                                targets.push_back(recipient);
                }
                else
                {
                    if (Player* recipient = me->GetLootRecipient())
                        targets.push_back(recipient);
                }
            }
        }
        case SMART_TARGET_POSITION:
        case SMART_TARGET_NONE:
        default:
            break;
    }

    FilterByTargetFlags((SMARTAI_TARGETS)e.GetTargetType(), e.GetTargetFlags(), targets, me);
}

void SmartScript::GetWorldObjectsInDist(ObjectVector& targets, float dist) const
{
    WorldObject* obj = GetBaseObject();
    if (!obj)
        return;

    Trinity::AllWorldObjectsInRange u_check(obj, dist);
    Trinity::WorldObjectListSearcher<Trinity::AllWorldObjectsInRange> searcher(obj, targets, u_check);
    Cell::VisitAllObjects(obj, searcher, dist);
}

void SmartScript::ProcessEvent(SmartScriptHolder& e, Unit* unit, uint32 var0, uint32 var1, bool bvar, const SpellInfo* spell, GameObject* gob)
{
    if (!e.active && e.GetEventType() != SMART_EVENT_LINK)
        return;

    if ((e.event.event_phase_mask && !IsInPhase(e.event.event_phase_mask)) || ((e.event.event_flags & SMART_EVENT_FLAG_NOT_REPEATABLE) && e.runOnce))
        return;

    if ((e.event.event_template_phase_mask && !IsInTemplatePhase(e.event.event_template_phase_mask)) || ((e.event.event_flags & SMART_EVENT_FLAG_NOT_REPEATABLE) && e.runOnce))
        return;

    if (!(e.event.event_flags & SMART_EVENT_FLAG_WHILE_CHARMED) && IsCreature(me) && IsCharmedCreature(me))
        return;

    switch (e.GetEventType())
    {
        case SMART_EVENT_LINK://special handling
            ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            break;
        //called from Update tick
        case SMART_EVENT_UPDATE:
            ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            break;
        case SMART_EVENT_UPDATE_OOC:
            if (me && me->IsInCombat())
                return;
            ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            break;
        case SMART_EVENT_UPDATE_IC:
            if (!me || !me->IsInCombat())
                return;
            ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            break;
        case SMART_EVENT_HEALT_PCT:
        {
            if (!me || !me->IsInCombat() || !me->GetMaxHealth())
                return;
            uint32 perc = (uint32)me->GetHealthPct();
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            break;
        }
        case SMART_EVENT_TARGET_HEALTH_PCT:
        {
            if (!me || !me->IsInCombat() || !me->GetVictim() || !me->GetVictim()->GetMaxHealth())
                return;
            uint32 perc = (uint32)me->GetVictim()->GetHealthPct();
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax, me->GetVictim());
            break;
        }
        case SMART_EVENT_MANA_PCT:
        {
            if (!me || !me->IsInCombat() || !me->GetMaxPower(POWER_MANA))
                return;
            uint32 perc = uint32(100.0f * me->GetPower(POWER_MANA) / me->GetMaxPower(POWER_MANA));
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            break;
        }
        case SMART_EVENT_TARGET_MANA_PCT:
        {
            if (!me || !me->IsInCombat() || !me->GetVictim() || !me->GetVictim()->GetMaxPower(POWER_MANA))
                return;
            uint32 perc = uint32(100.0f * me->GetVictim()->GetPower(POWER_MANA) / me->GetVictim()->GetMaxPower(POWER_MANA));
            if (perc > e.event.minMaxRepeat.max || perc < e.event.minMaxRepeat.min)
                return;
            ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax, me->GetVictim());
            break;
        }
        case SMART_EVENT_RANGE:
        {
            if (!me || !me->IsInCombat() || !me->GetVictim())
                return;

            if (me->IsInRange(me->GetVictim(), (float)e.event.minMaxRepeat.min, (float)e.event.minMaxRepeat.max))
                ProcessTimedAction(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax, me->GetVictim());
            else // make it predictable
                RecalcTimer(e, 500, 500);
            break;
        }
        case SMART_EVENT_VICTIM_CASTING:
        {
            if (!me || !me->IsInCombat())
                return;

            Unit* victim = me->GetVictim();

            if (!victim || !victim->IsNonMeleeSpellCast(false, false, true))
                return;

            if (e.event.targetCasting.spellId > 0)
                if (Spell* currSpell = victim->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    if (currSpell->m_spellInfo->Id != e.event.targetCasting.spellId)
                        return;

            ProcessTimedAction(e, e.event.targetCasting.repeatMin, e.event.targetCasting.repeatMax, me->GetVictim());
            break;
        }
        case SMART_EVENT_FRIENDLY_HEALTH:
        {
            if (!me || !me->IsInCombat())
                return;

            Unit* target = DoSelectLowestHpFriendly((float)e.event.friendlyHealth.radius, e.event.friendlyHealth.hpDeficit);
            if (!target || !target->IsInCombat())
            {
                // if there are at least two same npcs, they will perform the same action immediately even if this is useless...
                RecalcTimer(e, 1000, 3000);
                return;
            }
            ProcessTimedAction(e, e.event.friendlyHealth.repeatMin, e.event.friendlyHealth.repeatMax, target);
            break;
        }
        case SMART_EVENT_FRIENDLY_IS_CC:
        {
            if (!me || !me->IsInCombat())
                return;

            std::list<Creature*> pList;
            DoFindFriendlyCC(pList, (float)e.event.friendlyCC.radius);
            if (pList.empty())
            {
                // if there are at least two same npcs, they will perform the same action immediately even if this is useless...
                RecalcTimer(e, 1000, 3000);
                return;
            }
            ProcessTimedAction(e, e.event.friendlyCC.repeatMin, e.event.friendlyCC.repeatMax, Trinity::Containers::SelectRandomContainerElement(pList));
            break;
        }
        case SMART_EVENT_FRIENDLY_MISSING_BUFF:
        {
            std::list<Creature*> pList;
            DoFindFriendlyMissingBuff(pList, (float)e.event.missingBuff.radius, e.event.missingBuff.spell);

            if (pList.empty())
                return;

            ProcessTimedAction(e, e.event.missingBuff.repeatMin, e.event.missingBuff.repeatMax, Trinity::Containers::SelectRandomContainerElement(pList));
            break;
        }
        case SMART_EVENT_HAS_AURA:
        {
            if (!me)
                return;
            uint32 count = me->GetAuraCount(e.event.aura.spell);
            if ((e.event.aura.count == 0 && count == 0)  //no count specified, assume at least one
                || (e.event.aura.count && count >= e.event.aura.count)) //else check if at least count specified
                ProcessTimedAction(e, e.event.aura.repeatMin, e.event.aura.repeatMax);
            break;
        }
        case SMART_EVENT_TARGET_BUFFED:
        {
            if (!me || !me->GetVictim())
                return;
            uint32 count = me->GetVictim()->GetAuraCount(e.event.aura.spell);
            if (count < e.event.aura.count)
                return;
            ProcessTimedAction(e, e.event.aura.repeatMin, e.event.aura.repeatMax);
            break;
        }
        case SMART_EVENT_CHARMED:
            if ((e.GetActionType() == SMART_ACTION_KILL_UNIT && unit == me)
                || e.GetActionType() == SMART_ACTION_DIE)
            {
                TC_LOG_ERROR("sql.sql", "SmartAI: Tried to kill self from event SMART_EVENT_CHARMED. This is not allowed as this would cause a crash");
                break;
            }

            if (bvar == (e.event.charm.onRemove != 1))
                ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            break;
        //no params
        case SMART_EVENT_AGGRO:
        case SMART_EVENT_DEATH:
        case SMART_EVENT_EVADE:
        case SMART_EVENT_REACHED_HOME:
        case SMART_EVENT_CHARMED_TARGET:
        case SMART_EVENT_CORPSE_REMOVED:
        case SMART_EVENT_AI_INIT:
        case SMART_EVENT_TRANSPORT_ADDPLAYER:
        case SMART_EVENT_TRANSPORT_REMOVE_PLAYER:
       // case SMART_EVENT_QUEST_ACCEPTED:
        case SMART_EVENT_QUEST_OBJ_COPLETETION:
        case SMART_EVENT_QUEST_COMPLETION:
       // case SMART_EVENT_QUEST_REWARDED:
        case SMART_EVENT_QUEST_FAIL:
        case SMART_EVENT_JUST_SUMMONED:
        case SMART_EVENT_RESET:
        case SMART_EVENT_JUST_CREATED:
        case SMART_EVENT_FOLLOW_COMPLETED:
        case SMART_EVENT_ON_SPELLCLICK:
        case SMART_EVENT_VICTIM_DIED:
            ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            break;
        case SMART_EVENT_GOSSIP_HELLO:
            if (e.event.gossipHello.noReportUse && var0)
                return;
            ProcessAction(e, unit, var0, var1, bvar, spell, gob);
            break;
        case SMART_EVENT_ENTER_PHASE:
            if(var0 != e.event.phaseChanged.changedTo)
                return;

            ProcessAction(e, unit);

            break;
        case SMART_EVENT_IS_BEHIND_TARGET:
            {
                if (!me)
                    return;

                if (Unit* victim = me->GetVictim())
                {
                    if (!victim->HasInArc(static_cast<float>(M_PI), me))
                        ProcessTimedAction(e, e.event.behindTarget.cooldownMin, e.event.behindTarget.cooldownMax, victim);
                }
                break;
            }
        case SMART_EVENT_RECEIVE_EMOTE:
            if (e.event.emote.emote == var0)
            {
                RecalcTimer(e, e.event.emote.cooldownMin, e.event.emote.cooldownMax);
                ProcessAction(e, unit);
            }
            break;
        case SMART_EVENT_KILL:
        {
            if (!me || !unit)
                return;
            if (e.event.kill.playerOnly && unit->GetTypeId() != TYPEID_PLAYER)
                return;
            if (e.event.kill.creature && unit->GetEntry() != e.event.kill.creature)
                return;
            RecalcTimer(e, e.event.kill.cooldownMin, e.event.kill.cooldownMax);
            ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_SPELLHIT_TARGET:
        case SMART_EVENT_SPELLHIT:
        {
            if (!spell)
                return;
            if ((!e.event.spellHit.spell || spell->Id == e.event.spellHit.spell) &&
                (!e.event.spellHit.school || (spell->SchoolMask & e.event.spellHit.school)))
                {
                    RecalcTimer(e, e.event.spellHit.cooldownMin, e.event.spellHit.cooldownMax);
                    ProcessAction(e, unit, 0, 0, bvar, spell);
                }
            break;
        }
        case SMART_EVENT_OOC_LOS:
        {
            if (!me || me->IsInCombat())
                return;
            //can trigger if closer than fMaxAllowedRange
            float range = (float)e.event.los.maxDist;

            //if range is ok and we are actually in LOS
            if (me->IsWithinDistInMap(unit, range) && me->IsWithinLOSInMap(unit, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
            {
                //if friendly event&&who is not hostile OR hostile event&&who is hostile
                if ((e.event.los.noHostile && !me->IsHostileTo(unit)) ||
                    (!e.event.los.noHostile && me->IsHostileTo(unit)))
                {
                    RecalcTimer(e, e.event.los.cooldownMin, e.event.los.cooldownMax);
                    ProcessAction(e, unit);
                }
            }
            break;
        }
        case SMART_EVENT_IC_LOS:
        {
            if (!me || !me->IsInCombat())
                return;
            //can trigger if closer than fMaxAllowedRange
            float range = (float)e.event.los.maxDist;

            //if range is ok and we are actually in LOS
            if (me->IsWithinDistInMap(unit, range) && me->IsWithinLOSInMap(unit, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
            {
                //if friendly event&&who is not hostile OR hostile event&&who is hostile
                if ((e.event.los.noHostile && !me->IsHostileTo(unit)) ||
                    (!e.event.los.noHostile && me->IsHostileTo(unit)))
                {
                    RecalcTimer(e, e.event.los.cooldownMin, e.event.los.cooldownMax);
                    ProcessAction(e, unit);
                }
            }
            break;
        }
        case SMART_EVENT_RESPAWN:
        {
            if (!GetBaseObject())
                return;
            if (e.event.respawn.type == SMART_SCRIPT_RESPAWN_CONDITION_MAP && GetBaseObject()->GetMapId() != e.event.respawn.map)
                return;
            if (e.event.respawn.type == SMART_SCRIPT_RESPAWN_CONDITION_AREA && GetBaseObject()->GetZoneId() != e.event.respawn.area)
                return;
            ProcessAction(e);
            break;
        }
        case SMART_EVENT_SUMMONED_UNIT:
        {
            if (!IsCreature(unit))
                return;
            if (e.event.summoned.creature && unit->GetEntry() != e.event.summoned.creature)
                return;
            RecalcTimer(e, e.event.summoned.cooldownMin, e.event.summoned.cooldownMax);
            ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_RECEIVE_HEAL:
        case SMART_EVENT_DAMAGED:
        case SMART_EVENT_DAMAGED_TARGET:
        {
            if (var0 > e.event.minMaxRepeat.max || var0 < e.event.minMaxRepeat.min)
                return;
            RecalcTimer(e, e.event.minMaxRepeat.repeatMin, e.event.minMaxRepeat.repeatMax);
            ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_MOVEMENTINFORM:
        {
            if ((e.event.movementInform.type && var0 != e.event.movementInform.type) || (e.event.movementInform.id && var1 != e.event.movementInform.id))
                return;
            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_TRANSPORT_RELOCATE:
        case SMART_EVENT_WAYPOINT_START:
        {
            if (e.event.waypoint.pathID && var0 != e.event.waypoint.pathID)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_WAYPOINT_REACHED:
        case SMART_EVENT_WAYPOINT_RESUMED:
        case SMART_EVENT_WAYPOINT_PAUSED:
        case SMART_EVENT_WAYPOINT_STOPPED:
        case SMART_EVENT_WAYPOINT_ENDED:
        {
            if (!me || (e.event.waypoint.pointID && var0 != e.event.waypoint.pointID) || (e.event.waypoint.pathID && GetPathId() != e.event.waypoint.pathID))
                return;
            ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_SUMMON_DESPAWNED:
            if (e.event.summoned.creature && e.event.summoned.creature != var0)
                return;
            RecalcTimer(e, e.event.summoned.cooldownMin, e.event.summoned.cooldownMax);
            ProcessAction(e, unit, var0);
            break;
        case SMART_EVENT_INSTANCE_PLAYER_ENTER:
        {
            if (e.event.instancePlayerEnter.team && var0 != e.event.instancePlayerEnter.team)
                return;
            RecalcTimer(e, e.event.instancePlayerEnter.cooldownMin, e.event.instancePlayerEnter.cooldownMax);
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_ACCEPTED_QUEST:
        case SMART_EVENT_REWARD_QUEST:
        {
            if (e.event.quest.quest && var0 != e.event.quest.quest)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_TRANSPORT_ADDCREATURE:
        {
            if (e.event.transportAddCreature.creature && var0 != e.event.transportAddCreature.creature)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_AREATRIGGER_ONTRIGGER:
        {
            if (e.event.areatrigger.id && var0 != e.event.areatrigger.id)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_TEXT_OVER:
        {
            if (var0 != e.event.textOver.textGroupID || (e.event.textOver.creatureEntry && e.event.textOver.creatureEntry != var1))
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_DATA_SET:
        {
            if (e.event.dataSet.id != var0 || e.event.dataSet.value != var1)
                return;
            RecalcTimer(e, e.event.dataSet.cooldownMin, e.event.dataSet.cooldownMax);
            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_PASSENGER_REMOVED:
        case SMART_EVENT_PASSENGER_BOARDED:
        {
            if (!unit)
                return;
            RecalcTimer(e, e.event.minMax.repeatMin, e.event.minMax.repeatMax);
            ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_TIMED_EVENT_TRIGGERED:
        {
            if (e.event.timedEvent.id == var0)
                ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_GOSSIP_SELECT:
        {
            TC_LOG_DEBUG("sql.sql","SmartScript: Gossip Select:  menu %u action %u", var0, var1);//little help for scripters
            if (!e.event.gossip.any && (e.event.gossip.sender != var0 || e.event.gossip.action != var1))
                return;
            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_EVENT_PHASE_CHANGE:
        {
            if (!IsInPhase(SmartPhaseMask(e.event.eventPhaseChange.phasemask)))
                return;

            ProcessAction(e, GetLastInvoker());
            break;
        }
        case SMART_EVENT_GAME_EVENT_START:
        case SMART_EVENT_GAME_EVENT_END:
        {
            if (e.event.gameEvent.gameEventId != var0)
                return;
            ProcessAction(e, nullptr, var0);
            break;
        }
        case SMART_EVENT_GO_STATE_CHANGED:
        {
            if (e.event.goStateChanged.state != var0)
                return;

            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_GO_LOOT_STATE_CHANGED:
        {
            if (((1 << var0) & e.event.goLootStateChanged.stateMask) == 0)
                return;

            ProcessAction(e, unit, var0, var1);
            break;
        }
        case SMART_EVENT_GO_EVENT_INFORM:
        {
            if (e.event.eventInform.eventId != var0)
                return;
            ProcessAction(e, nullptr, var0);
            break;
        }
        case SMART_EVENT_ACTION_DONE:
        {
            if (e.event.doAction.eventId != var0)
                return;
            ProcessAction(e, unit, var0);
            break;
        }
        case SMART_EVENT_FRIENDLY_HEALTH_PCT:
        {
            //This event is shitty, do not use, targets will get overriden in action anyway. It's only kept for compat with Trinity.
            if (!me || !me->IsInCombat())
                return;

            ObjectVector _targets;
            GetTargets(_targets, e);
            if (_targets.empty())
                return;

            Unit* chosenTarget = nullptr;
            for (WorldObject* target : _targets)
            {
                if (IsUnit(target) && me->IsFriendlyTo((target)->ToUnit()) && (target)->ToUnit()->IsAlive() && (target)->ToUnit()->IsInCombat())
                {
                    uint32 healthPct = uint32((target)->ToUnit()->GetHealthPct());

                    if (healthPct > e.event.friendlyHealthPct.maxHpPct || healthPct < e.event.friendlyHealthPct.minHpPct)
                        continue;

                    chosenTarget = (target)->ToUnit();
                    break;
                }
            }

            if (!chosenTarget)
                return;

            ProcessTimedAction(e, e.event.friendlyHealthPct.repeatMin, e.event.friendlyHealthPct.repeatMax, chosenTarget);
            break;
        }
        case SMART_EVENT_DISTANCE_CREATURE:
        {
            if (!me)
                return;

            Creature* creature = nullptr;

            if (e.event.distance.guid != 0)
            {
                creature = FindCreatureNear(me, e.event.distance.guid);

                if (!creature)
                    return;

                if (!me->IsInRange(creature, 0, (float)e.event.distance.dist))
                    return;
            }
            else if (e.event.distance.entry != 0)
            {
                std::list<Creature*> list;
                me->GetCreatureListWithEntryInGrid(list, e.event.distance.entry, (float)e.event.distance.dist);

                if (!list.empty())
                    creature = list.front();
            }

            if (creature)
                ProcessTimedAction(e, e.event.distance.repeat, e.event.distance.repeat, creature);

            break;
        }
        case SMART_EVENT_DISTANCE_GAMEOBJECT:
        {
            if (!me)
                return;

            GameObject* gameobject = nullptr;

            if (e.event.distance.guid != 0)
            {
                gameobject = FindGameObjectNear(me, e.event.distance.guid);

                if (!gameobject)
                    return;

                if (!me->IsInRange(gameobject, 0, (float)e.event.distance.dist))
                    return;
            }
            else if (e.event.distance.entry != 0)
            {
                std::list<GameObject*> list;
                me->GetGameObjectListWithEntryInGrid(list, e.event.distance.entry, (float)e.event.distance.dist);

                if (!list.empty())
                    gameobject = list.front();
            }

            if (gameobject)
                ProcessTimedAction(e, e.event.distance.repeat, e.event.distance.repeat, nullptr, 0, 0, false, nullptr, gameobject);

            break;
        }
        case SMART_EVENT_COUNTER_SET:
            if (e.event.counter.id != var0 || GetCounterValue(e.event.counter.id) != e.event.counter.value)
                return;
            
            ProcessTimedAction(e, e.event.counter.cooldownMin, e.event.counter.cooldownMax);
            break;
        case SMART_EVENT_FRIENDLY_KILLED:
        {
            if(!unit || !me)
                return;

            if(unit->GetDistance(me) > e.event.friendlyDeath.range)
                return;

            if(e.event.friendlyDeath.entry && unit->GetEntry() != e.event.friendlyDeath.range)
                return;

            if(e.event.friendlyDeath.guid && unit->GetGUIDLow() != e.event.friendlyDeath.guid)
                return;

            ProcessAction(e, unit);
            break;
        }
        case SMART_EVENT_VICTIM_NOT_IN_LOS:
        {
            if (!me)
                return;
            
            Unit const* victim = me->GetVictim();
            if(!victim)
                return;

            bool withinLos = me->IsWithinLOSInMap(victim, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2);
            if(!e.event.victimNotInLoS.invert) //normal case, triggers if cannot see
            {
                if(withinLos)
                    return;
            } else { //triger if can see
                if(!withinLos)
                    return;
            }

            ProcessTimedAction(e, e.event.victimNotInLoS.repeat, e.event.victimNotInLoS.repeat);
        }
        case SMART_EVENT_AFFECTED_BY_MECHANIC:
        {
            auto auraList = me->GetAuras();
            for (auto itr : auraList)
            {
                if (itr.second->GetSpellInfo()->GetAllEffectsMechanicMask() & e.event.affectedByMechanic.mechanicMask)
                {
                    ProcessTimedAction(e, e.event.affectedByMechanic.repeat, e.event.affectedByMechanic.repeat);
                    break;
                }
            }
            //nothing found
            break;
        }
        default:
            TC_LOG_ERROR("sql.sql","SmartScript::ProcessEvent: Unhandled Event type %u", e.GetEventType());
            break;
    }
}

void SmartScript::InitTimer(SmartScriptHolder& e)
{
    switch (e.GetEventType())
    {
        //set only events which have initial timers
        case SMART_EVENT_UPDATE:
        case SMART_EVENT_UPDATE_IC:
        case SMART_EVENT_UPDATE_OOC:
            RecalcTimer(e, e.event.minMaxRepeat.min, e.event.minMaxRepeat.max);
            break;
        case SMART_EVENT_IC_LOS:
        case SMART_EVENT_OOC_LOS:
            RecalcTimer(e, e.event.los.cooldownMin, e.event.los.cooldownMax);
            break;
        case SMART_EVENT_DISTANCE_CREATURE:
        case SMART_EVENT_DISTANCE_GAMEOBJECT:
            RecalcTimer(e, e.event.distance.repeat, e.event.distance.repeat);
            break;
        case SMART_EVENT_VICTIM_NOT_IN_LOS:
            RecalcTimer(e, e.event.victimNotInLoS.repeat, e.event.victimNotInLoS.repeat);
            break;
        case SMART_EVENT_FRIENDLY_HEALTH_PCT:
            RecalcTimer(e, e.event.friendlyHealthPct.repeatMin, e.event.friendlyHealthPct.repeatMax);
            break;
        case SMART_EVENT_AFFECTED_BY_MECHANIC:
            RecalcTimer(e, e.event.affectedByMechanic.repeat, e.event.affectedByMechanic.repeat);
            break;
        default:
            e.active = true;
            break;
    }
}
void SmartScript::RecalcTimer(SmartScriptHolder& e, uint32 min, uint32 max)
{
    // min/max was checked at loading!
    e.timer = urand(uint32(min), uint32(max));
    e.active = e.timer ? false : true;
}

void SmartScript::UpdateTimer(SmartScriptHolder& e, uint32 const diff)
{
    if (e.GetEventType() == SMART_EVENT_LINK)
        return;

    if (e.event.event_phase_mask && !IsInPhase(e.event.event_phase_mask))
        return;

    if (e.GetEventType() == SMART_EVENT_UPDATE_IC && (!me || !me->IsInCombat()))
        return;

    if (e.GetEventType() == SMART_EVENT_UPDATE_OOC && (me && me->IsInCombat())) //can be used with me=NULL (go script)
        return;

    if (e.timer < diff)
    {
        // delay spell cast event if another spell is being cast
        if (e.GetActionType() == SMART_ACTION_CAST)
        {
            if (!(e.action.cast.castFlags & SMARTCAST_INTERRUPT_PREVIOUS))
            {
                if (me && me->HasUnitState(UNIT_STATE_CASTING))
                {
                    e.timer = 1;
                    return;
                }
            }
        }

        // Delay flee for assist event if stunned or rooted
        if (e.GetActionType() == SMART_ACTION_FLEE_FOR_ASSIST)
        {
            if (me && me->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_LOST_CONTROL))
            {
                e.timer = 1;
                return;
            }
        }

        e.active = true;//activate events with cooldown
        switch (e.GetEventType())//process ONLY timed events
        {
            case SMART_EVENT_UPDATE:
            case SMART_EVENT_UPDATE_OOC:
            case SMART_EVENT_UPDATE_IC:
            case SMART_EVENT_HEALT_PCT:
            case SMART_EVENT_TARGET_HEALTH_PCT:
            case SMART_EVENT_MANA_PCT:
            case SMART_EVENT_TARGET_MANA_PCT:
            case SMART_EVENT_RANGE:
            case SMART_EVENT_VICTIM_CASTING:
            case SMART_EVENT_FRIENDLY_HEALTH:
            case SMART_EVENT_FRIENDLY_IS_CC:
            case SMART_EVENT_FRIENDLY_MISSING_BUFF:
            case SMART_EVENT_HAS_AURA:
            case SMART_EVENT_TARGET_BUFFED:
            case SMART_EVENT_IS_BEHIND_TARGET:
            case SMART_EVENT_FRIENDLY_HEALTH_PCT:
            case SMART_EVENT_DISTANCE_CREATURE:
            case SMART_EVENT_DISTANCE_GAMEOBJECT:
            case SMART_EVENT_VICTIM_NOT_IN_LOS:
            case SMART_EVENT_AFFECTED_BY_MECHANIC:
            {
                ProcessEvent(e);
                if (e.GetScriptType() == SMART_SCRIPT_TYPE_TIMED_ACTIONLIST)
                {
                    e.enableTimed = false;//disable event if it is in an ActionList and was processed once
                    for (auto & i : mTimedActionList)
                    {
                        //find the first event which is not the current one and enable it
                        if (i.event_id > e.event_id)
                        {
                            i.enableTimed = true;
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    else
        e.timer -= diff;
}

bool SmartScript::CheckTimer(SmartScriptHolder const& e) const
{
    return e.active;
}

void SmartScript::InstallEvents()
{
    if (!mInstallEvents.empty())
    {
        for (auto & mInstallEvent : mInstallEvents)
            mEvents.push_back(mInstallEvent);//must be before UpdateTimers

        mInstallEvents.clear();
    }
}

void SmartScript::IncPhase(uint32 p)
{
    // protect phase from overflowing
    SetPhase(std::min<uint32>(SMART_EVENT_PHASE_COUNT, mEventPhase + p));
}

void SmartScript::DecPhase(uint32 p)
{
    if (p >= mEventPhase)
        SetPhase(0);
    else
        SetPhase(mEventPhase - p);
}

bool SmartScript::IsInPhase(SmartPhaseMask phaseMask) const
{
    if (mEventPhase == 0)
        return false;

    return ((1 << (mEventPhase - 1)) & phaseMask) != 0;
}

bool SmartScript::IsInTemplatePhase(SmartPhaseMask phaseMask) const
{
    if (mEventTemplatePhase == 0)
        return false;

    return ((1 << (mEventTemplatePhase - 1)) & phaseMask) != 0;
}

void SmartScript::SetPhase(uint32 p)
{

    uint32 previous = mEventPhase;
    mEventPhase = p;
    if (mEventPhase != previous) 
    {
        ProcessEventsFor(SMART_EVENT_EVENT_PHASE_CHANGE); //TC event
        ProcessEventsFor(SMART_EVENT_ENTER_PHASE, nullptr, mEventPhase); //Sunstrider event
    }
}

void SmartScript::SetTemplatePhase(uint32 p)
{
    mEventTemplatePhase = p;
}

void SmartScript::OnUpdate(uint32 const diff)
{
    if ((mScriptType == SMART_SCRIPT_TYPE_CREATURE || mScriptType == SMART_SCRIPT_TYPE_GAMEOBJECT) && !GetBaseObject())
        return;

    InstallEvents();//before UpdateTimers

    for (auto & mEvent : mEvents)
        UpdateTimer(mEvent, diff);

    if (!mStoredEvents.empty())
    {
        SmartAIEventStoredList::iterator i, icurr;
        for (i = mStoredEvents.begin(); i != mStoredEvents.end();)
        {
            icurr = i++;
            UpdateTimer(*icurr, diff);
        }
    }

    bool needCleanup = true;
    if (!mTimedActionList.empty())
    {
        isProcessingTimedActionList = true;
        for (auto & i : mTimedActionList)
        {
            if (i.enableTimed)
            {
                UpdateTimer(i, diff);
                needCleanup = false;
            }
        }
        isProcessingTimedActionList = false;
    }
    if (needCleanup)
        mTimedActionList.clear();

    if (!mRemIDs.empty())
    {
        for (auto i : mRemIDs)
            RemoveStoredEvent(i);

        mRemIDs.clear();
    }

    if (mUseTextTimer && me)
    {
        if (mTextTimer < diff)
        {
            uint32 textID = mLastTextID;
            mLastTextID = 0;
            uint32 entry = mTalkerEntry;
            mTalkerEntry = 0;
            mTextTimer = 0;
            mUseTextTimer = false;
            ProcessEventsFor(SMART_EVENT_TEXT_OVER, nullptr, textID, entry);
        } else mTextTimer -= diff;
    }
}

void SmartScript::FillScript(SmartAIEventList e, WorldObject* obj, AreaTriggerEntry const* at)
{
    if (e.empty())
    {
        if (obj)
            TC_LOG_DEBUG("sql.sql","SmartScript: EventMap for Entry %u is empty but is using SmartScript.", obj->GetEntry());
        if (at)
            TC_LOG_DEBUG("sql.sql","SmartScript: EventMap for AreaTrigger %u is empty but is using SmartScript.", at->id);
        return;
    }
    for (auto & i : e)
    {
        #ifndef TRINITY_DEBUG
            if (i.event.event_flags & SMART_EVENT_FLAG_DEBUG_ONLY)
                continue;
        #endif

        if (i.event.event_flags & SMART_EVENT_FLAG_DIFFICULTY_ALL)//if has instance flag add only if in it
        {
            if(obj && obj->GetMap()->IsDungeon())
            {
                if ((1 << (obj->GetMap()->GetSpawnMode()+1)) & i.event.event_flags)
                    mEvents.push_back(i);
            } else {
                //if out of instance, still play "normal" difficulty events
                if(i.event.event_flags & SMART_EVENT_FLAG_DIFFICULTY_0)
                    mEvents.push_back(i);
            }
            continue;
        }
        mEvents.push_back(i);//NOTE: 'world(0)' events still get processed in ANY instance mode
    }
}

void SmartScript::GetScript()
{
    SmartAIEventList e;
    if (me)
    {
        e = sSmartScriptMgr->GetScript(-((int32)me->GetSpawnId()), mScriptType);
        if (e.empty())
            e = sSmartScriptMgr->GetScript((int32)me->GetEntry(), mScriptType);
        FillScript(e, me, nullptr);
    }
    else if (go)
    {
        e = sSmartScriptMgr->GetScript(-((int32)go->GetSpawnId()), mScriptType);
        if (e.empty())
            e = sSmartScriptMgr->GetScript((int32)go->GetEntry(), mScriptType);
        FillScript(e, go, nullptr);
    }
    else if (trigger)
    {
        e = sSmartScriptMgr->GetScript((int32)trigger->id, mScriptType);
        FillScript(e, nullptr, trigger);
    }
}

void SmartScript::OnInitialize(WorldObject* obj, AreaTriggerEntry const* at)
{
    if (obj)//handle object based scripts
    {
        switch (obj->GetTypeId())
        {
            case TYPEID_UNIT:
                mScriptType = SMART_SCRIPT_TYPE_CREATURE;
                me = obj->ToCreature();
                TC_LOG_DEBUG("scripts.ai","SmartScript::OnInitialize: source is Creature %u", me->GetEntry());
                break;
            case TYPEID_GAMEOBJECT:
                mScriptType = SMART_SCRIPT_TYPE_GAMEOBJECT;
                go = obj->ToGameObject();
                TC_LOG_DEBUG("scripts.ai","SmartScript::OnInitialize: source is GameObject %u", go->GetEntry());
                break;
            default:
                TC_LOG_ERROR("misc","SmartScript::OnInitialize: Unhandled TypeID !WARNING!");
                return;
        }
    } else if (at)
    {
        mScriptType = SMART_SCRIPT_TYPE_AREATRIGGER;
        trigger = at;
        TC_LOG_DEBUG("scripts.ai","SmartScript::OnInitialize: source is AreaTrigger %u", trigger->id);
    }
    else
    {
        TC_LOG_ERROR("misc","SmartScript::OnInitialize: !WARNING! Initialized objects are NULL.");
        return;
    }

    GetScript();//load copy of script

    for (auto & mEvent : mEvents)
        InitTimer(mEvent);//calculate timers for first time use

    ProcessEventsFor(SMART_EVENT_AI_INIT);
    InstallEvents();
    ProcessEventsFor(SMART_EVENT_JUST_CREATED);
}

void SmartScript::OnMoveInLineOfSight(Unit* who)
{
    ProcessEventsFor(SMART_EVENT_OOC_LOS, who);

    if (!me)
        return;

    if (me->GetVictim())
        return;

    ProcessEventsFor(SMART_EVENT_IC_LOS, who);

}

Unit* SmartScript::DoSelectLowestHpFriendly(float range, uint32 MinHPDiff)
{
    if (!me)
        return nullptr;

    Unit* unit = nullptr;

    Trinity::MostHPMissingInRange u_check(me, range, MinHPDiff);
    Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(me, unit, u_check);
    Cell::VisitGridObjects(me, searcher, range);
    return unit;
}

void SmartScript::DoFindFriendlyCC(std::list<Creature*>& _list, float range)
{
    if (!me) 
        return;
    
    Trinity::FriendlyCCedInRange u_check(me, range);
    Trinity::CreatureListSearcher<Trinity::FriendlyCCedInRange> searcher(me, _list, u_check);
    Cell::VisitGridObjects(me, searcher, range);
}

void SmartScript::DoFindFriendlyMissingBuff(std::list<Creature*>& list, float range, uint32 spellid)
{
    if (!me)
        return;

    Trinity::FriendlyMissingBuffInRange u_check(me, range, spellid);
    Trinity::CreatureListSearcher<Trinity::FriendlyMissingBuffInRange> searcher(me, list, u_check);
    Cell::VisitGridObjects(me, searcher, range);
}

Unit* SmartScript::DoFindClosestOrFurthestFriendlyInRange(float range, bool playerOnly, bool nearest)
{
    if (!me)
        return nullptr;

    Unit *target = nullptr;
    Trinity::NearestFriendlyUnitInObjectRangeCheck u_check(me, me, range,playerOnly,nearest);
    Trinity::UnitLastSearcher<Trinity::NearestFriendlyUnitInObjectRangeCheck> searcher(me, target, u_check);
    Cell::VisitGridObjects(me, searcher, range);

    return target;
}

void SmartScript::SetScript9(SmartScriptHolder& e, uint32 entry)
{
    //do NOT clear mTimedActionList if it's being iterated because it will invalidate the iterator and delete
    // any SmartScriptHolder contained like the "e" parameter passed to this function
    if (isProcessingTimedActionList)
    {
        TC_LOG_ERROR("scripts.ai","SAI : Entry %d SourceType %u Event %u Action %u is trying to overwrite timed action list from a timed action, this is not allowed!.", e.entryOrGuid, e.GetScriptType(), e.GetEventType(), e.GetActionType());
        return;
    }
    mTimedActionList.clear();
    mTimedActionList = sSmartScriptMgr->GetScript(entry, SMART_SCRIPT_TYPE_TIMED_ACTIONLIST);
    if (mTimedActionList.empty())
        return;
    for (auto i = mTimedActionList.begin(); i != mTimedActionList.end(); ++i)
    {
        i->enableTimed = i == mTimedActionList.begin();//enable processing only for the first action

        if (e.action.timedActionList.timerType == 0)
            i->event.type = SMART_EVENT_UPDATE_OOC;
        else if (e.action.timedActionList.timerType == 1)
            i->event.type = SMART_EVENT_UPDATE_IC;
        else if (e.action.timedActionList.timerType > 1)
            i->event.type = SMART_EVENT_UPDATE;

        InitTimer((*i));
    }
}

Unit* SmartScript::GetLastInvoker(Unit* invoker)
{
    // Look for invoker only on map of base object... Prevents multithreaded crashes
    if (WorldObject* baseObject = GetBaseObject())
        return ObjectAccessor::GetUnit(*baseObject, mLastInvoker);
    // used for area triggers invoker cast
    else if (invoker)
        return ObjectAccessor::GetUnit(*invoker, mLastInvoker);

    return nullptr;
}
