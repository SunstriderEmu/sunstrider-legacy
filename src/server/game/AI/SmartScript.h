//Adapted and updated from TrinityCore : 2014-01-24

#ifndef TRINITY_SMARTSCRIPT_H
#define TRINITY_SMARTSCRIPT_H

#include "Common.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Unit.h"
#include "ConditionMgr.h"
#include "CreatureTextMgr.h"
#include "Spell.h"
#include "GridNotifiers.h"

#include "SmartScriptMgr.h"

class TC_GAME_API SmartScript
{
    public:
        SmartScript();
        ~SmartScript();

        void OnInitialize(WorldObject* obj, AreaTriggerEntry const* at = nullptr);
        void GetScript();
        void FillScript(SmartAIEventList e, WorldObject* obj, AreaTriggerEntry const* at);

        void ProcessEventsFor(SMART_EVENT e, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        void ProcessEvent(SmartScriptHolder& e, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        bool CheckTimer(SmartScriptHolder const& e) const;
        void RecalcTimer(SmartScriptHolder& e, uint32 min, uint32 max);
        void UpdateTimer(SmartScriptHolder& e, uint32 const diff);
        void InitTimer(SmartScriptHolder& e);
        void ProcessAction(SmartScriptHolder& e, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        void ProcessTimedAction(SmartScriptHolder& e, uint32 const& min, uint32 const& max, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        /* Strips the list depending on the given target flags and the target type
        To improve: 
            Make it so that target flags are directly filtering target during search instead of altering the resulting list. 
            In the current form, target as "get the closest creature" do not interact well with target flags because they return only one target
            which can then be filtered out by target flags. Instead, we'd prefer returning the closest creature that match target flags.
        */
        void FilterByTargetFlags(SMARTAI_TARGETS type, SMARTAI_TARGETS_FLAGS flags, ObjectVector& list, WorldObject const* caster);
        bool IsTargetAllowedByTargetFlags(WorldObject const* target, SMARTAI_TARGETS_FLAGS flags, WorldObject const* caster, SMARTAI_TARGETS type);
        //May return null, must be deleted after usage
        void GetTargets(ObjectVector& targets, SmartScriptHolder const& e, Unit* invoker = nullptr);
        //returns a NEW object list, the called must delete if after usage
        void GetWorldObjectsInDist(ObjectVector& targets, float dist) const;
        void InstallTemplate(SmartScriptHolder const& e);
        SmartScriptHolder CreateEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, SmartPhaseMask phaseMask = SmartPhaseMask(0), SmartPhaseMask templatePhaseMask = SmartPhaseMask(0));
        void AddEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, SmartPhaseMask phaseMask = SmartPhaseMask(0), SmartPhaseMask templatePhaseMask = SmartPhaseMask(0));
        void SetPathId(uint32 id) { mPathId = id; }
        uint32 GetPathId() const { return mPathId; }
        WorldObject* GetBaseObject() const
        {
            WorldObject* obj = nullptr;
            if (me)
                obj = me;
            else if (go)
                obj = go;
            return obj;
        }

        static bool IsUnit(WorldObject* obj)
        {
            return obj && (obj->GetTypeId() == TYPEID_UNIT || obj->GetTypeId() == TYPEID_PLAYER);
        }

        static bool IsPlayer(WorldObject* obj)
        {
            return obj && obj->GetTypeId() == TYPEID_PLAYER;
        }

        static bool IsCreature(WorldObject* obj)
        {
            return obj && obj->GetTypeId() == TYPEID_UNIT;
        }

        static bool IsCharmedCreature(WorldObject* obj)
        {
            if (!obj)
                return false;

            if (Creature* creatureObj = obj->ToCreature())
                return creatureObj->IsCharmed();

            return false;
        }

        static bool IsGameObject(WorldObject* obj)
        {
            return obj && obj->GetTypeId() == TYPEID_GAMEOBJECT;
        }

        void OnUpdate(const uint32 diff);
        void OnMoveInLineOfSight(Unit* who);

        Unit* DoSelectLowestHpFriendly(float range, uint32 MinHPDiff);
        void DoFindFriendlyCC(std::list<Creature*>& _list, float range);
        void DoFindFriendlyMissingBuff(std::list<Creature*>& list, float range, uint32 spellid);
        Unit* DoFindClosestOrFurthestFriendlyInRange(float range, bool playerOnly, bool nearest = true);

        void StoreTargetList(ObjectVector const& targets, uint32 id)
        {
            // insert or replace
            _storedTargets.erase(id);
            _storedTargets.emplace(id, ObjectGuidVector(targets));
        }

        bool IsSmart(Creature* c = nullptr)
        {
            bool smart = true;
            if (c && c->GetAIName() != SMARTAI_AI_NAME)
                smart = false;

            if (!me || me->GetAIName() != SMARTAI_AI_NAME)
                smart = false;

            if (!smart)
                TC_LOG_ERROR("sql.sql","SmartScript: Action target Creature (GUID: %u Entry: %u) is not using SmartAI, action skipped to prevent crash.", c ? c->GetSpawnId() : (me ? me->GetSpawnId() : 0), c ? c->GetEntry() : (me ? me->GetEntry() : 0));

            return smart;
        }

        bool IsSmartGO(GameObject* g = nullptr)
        {
            bool smart = true;
            if (g && g->GetAIName() != SMARTAI_GOBJECT_AI_NAME)
                smart = false;

            if (!go || go->GetAIName() != SMARTAI_GOBJECT_AI_NAME)
                smart = false;
            if (!smart)
                TC_LOG_ERROR("sql.sql","SmartScript: Action target GameObject (GUID: %u Entry: %u) is not using SmartGameObjectAI, action skipped to prevent crash.", g ? g->GetSpawnId() : (go ? go->GetSpawnId() : 0), g ? g->GetEntry() : (go ? go->GetEntry() : 0));

            return smart;
        }

        ObjectVector const* GetStoredTargetVector(uint32 id, WorldObject const& ref) const
        {
            auto itr = _storedTargets.find(id);
            if (itr != _storedTargets.end())
                return itr->second.GetObjectVector(ref);
            return nullptr;

        }

        void StoreCounter(uint32 id, uint32 value, uint32 reset)
        {
            CounterMap::iterator itr = mCounterList.find(id);
            if (itr != mCounterList.end())
            {
                if (reset == 0)
                    itr->second += value;
                else
                    itr->second = value;
            }
            else
                mCounterList.insert(std::make_pair(id, value));

            ProcessEventsFor(SMART_EVENT_COUNTER_SET, nullptr, id);
        }

        uint32 GetCounterValue(uint32 id) const
        {
            CounterMap::const_iterator itr = mCounterList.find(id);
            if (itr != mCounterList.end())
                return itr->second;
            return 0;
        }

        GameObject* FindGameObjectNear(WorldObject* searchObject, uint32 guid) const
        {
            auto bounds = searchObject->GetMap()->GetGameObjectBySpawnIdStore().equal_range(guid);
            if (bounds.first == bounds.second)
                return nullptr;

            return bounds.first->second;
        }

        Creature* FindCreatureNear(WorldObject* searchObject, uint32 guid) const
        {
            auto bounds = searchObject->GetMap()->GetCreatureBySpawnIdStore().equal_range(guid);
            if (bounds.first == bounds.second)
                return nullptr;

            auto creatureItr = std::find_if(bounds.first, bounds.second, [](Map::CreatureBySpawnIdContainer::value_type const& pair)
            {
                return pair.second->IsAlive();
            });

            return creatureItr != bounds.second ? creatureItr->second : bounds.first->second;
        }

        ObjectVectorMap _storedTargets;

        void OnReset();
        void ResetBaseObject()
        {
			WorldObject* lookupRoot = me;
			if (!lookupRoot)
				lookupRoot = go;

			if (lookupRoot)
			{
				if (meOrigGUID)
				{
					if (Creature* m = ObjectAccessor::GetCreature(*lookupRoot, meOrigGUID))
					{
						me = m;
						go = nullptr;
					}
				}
				if (goOrigGUID)
				{
					if (GameObject* o = ObjectAccessor::GetGameObject(*lookupRoot, goOrigGUID))
					{
						me = nullptr;
						go = o;
					}
				}
			}
            goOrigGUID = 0;
            meOrigGUID = 0;
        }

        uint32 GetPhase() { return mEventPhase; }

        uint32 GetLastProcessedActionId() { return mLastProcessedActionId; }

        //TIMED_ACTIONLIST (script type 9 aka script9)
        void SetScript9(SmartScriptHolder& e, uint32 entry);
        Unit* GetLastInvoker(Unit* invoker = nullptr);
        uint64 mLastInvoker;
        uint32 mLastProcessedActionId;
        typedef std::unordered_map<uint32, uint32> CounterMap;
        CounterMap mCounterList;

    private:
        void IncPhase(uint32 p);
        void DecPhase(uint32 p);
        bool IsInPhase(SmartPhaseMask phaseMask) const;
        bool IsInTemplatePhase(SmartPhaseMask phaseMask) const;
        void SetPhase(uint32 p = 0);
        void SetTemplatePhase(uint32 p = 0);

        SmartAIEventList mEvents;
        SmartAIEventList mInstallEvents;
        SmartAIEventList mTimedActionList;
        bool isProcessingTimedActionList;
        Creature* me;
        uint64 meOrigGUID;
        GameObject* go;
        uint64 goOrigGUID;
        AreaTriggerEntry const* trigger;
        SmartScriptType mScriptType;
        uint32 mEventPhase;
        uint32 mEventTemplatePhase;
        uint32 mStoredPhase; //for SMART_ACTION_STORE_PHASE & SMART_ACTION_RESTORE_PHASE

        std::unordered_map<int32, int32> mStoredDecimals;
        uint32 mPathId;
        SmartAIEventStoredList  mStoredEvents;
        std::vector<uint32>mRemIDs;

        uint32 mTextTimer;
        uint32 mLastTextID;
        uint32 mTalkerEntry;
        bool mUseTextTimer;

        SMARTAI_TEMPLATE mTemplate;
        void InstallEvents();

        void RemoveStoredEvent(uint32 id)
        {
            if (!mStoredEvents.empty())
            {
                for (auto i = mStoredEvents.begin(); i != mStoredEvents.end(); ++i)
                {
                    if (i->event_id == id)
                    {
                        mStoredEvents.erase(i);
                        return;
                    }
                }
            }
        }
        SmartScriptHolder FindLinkedEvent(uint32 link)
        {
            if (!mEvents.empty())
            {
                for (auto & mEvent : mEvents)
                {
                    if (mEvent.event_id == link)
                    {
                        return mEvent;
                    }
                }
            }
            SmartScriptHolder s;
            return s;
        }
};

#endif

