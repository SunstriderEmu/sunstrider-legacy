
#include "AIException.h"
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "CreatureAISelector.h"
#include "CreatureAIFactory.h"
#include "NullCreatureAI.h"
#include "MovementGenerator.h"
#include "Pet.h"
#include "ScriptMgr.h"

namespace FactorySelector
{
    template <class T, class Value>
    inline int32 GetPermitFor(T const* obj, Value const& value)
    {
        Permissible<T> const* const p = ASSERT_NOTNULL(dynamic_cast<Permissible<T> const*>(value.second.get()));
        return p->Permit(obj);
    }

    template <class T>
    struct PermissibleOrderPred
    {
        public:
            PermissibleOrderPred(T const* obj) : _obj(obj) { }

            template <class Value>
            bool operator()(Value const& left, Value const& right) const
            {
                return GetPermitFor(_obj, left) < GetPermitFor(_obj, right);
            }

        private:
            T const* const _obj;
    };

    template <class AI, class T>
    inline FactoryHolder<AI, T> const* SelectFactory(T* obj)
    {
        static_assert(std::is_same<AI, CreatureAI>::value || std::is_same<AI, GameObjectAI>::value, "Invalid template parameter");
        static_assert(std::is_same<AI, CreatureAI>::value == std::is_same<T, Creature>::value, "Incompatible AI for type");
        static_assert(std::is_same<AI, GameObjectAI>::value == std::is_same<T, GameObject>::value, "Incompatible AI for type");

        using AIRegistry = typename FactoryHolder<AI, T>::FactoryHolderRegistry;

        // AIName in db
        std::string const& aiName = obj->GetAIName();
        if (!aiName.empty())
            return AIRegistry::instance()->GetRegistryItem(aiName);

        // select by permit check
        typename AIRegistry::RegistryMapType const& items = AIRegistry::instance()->GetRegisteredItems();
        auto itr = std::max_element(items.begin(), items.end(), PermissibleOrderPred<T>(obj));
        if (itr != items.end() && GetPermitFor(obj, *itr) >= 0)
            return itr->second.get();

        // should _never_ happen, Null AI types defined as PERMIT_BASE_IDLE, it must've been found
        ABORT();
        return nullptr;
    }

    CreatureAI* SelectAI(Creature *creature)
    {
        // Always PetAI for hunter pets
        if (creature->IsPet())
            return ASSERT_NOTNULL(sCreatureAIRegistry->GetRegistryItem("PetAI"))->Create(creature);

        // scriptname in db
        try
        {
            if (CreatureAI* scriptedAI = sScriptMgr->GetCreatureAI(creature))
                return scriptedAI;
        }
        catch (InvalidAIException const& e)
        {
            TC_LOG_ERROR("entities.unit", "Exception trying to assign script '%s' to Creature (Entry: %u), this Creature will have a default AI. Exception message: %s",
                creature->GetScriptName().c_str(), creature->GetEntry(), e.what());
        }

        return SelectFactory<CreatureAI>(creature)->Create(creature);
    }

    MovementGenerator* SelectMovementGenerator(Unit* unit)
    {
        MovementGeneratorType type = unit->GetDefaultMovementType();
        if (Creature* creature = unit->ToCreature())
        {
            if (!creature->GetPlayerMovingMe())
                type = creature->GetDefaultMovementType();
            //sunstrider: override default WAYPOINT_MOTION_TYPE type if no waypoint path id found for creature (or WAYPOINT_MOTION_TYPE will fail anyway)
            if (type == WAYPOINT_MOTION_TYPE && creature->GetWaypointPath() == 0)
                type = IDLE_MOTION_TYPE;
        }

        MovementGeneratorCreator const* mv_factory = sMovementGeneratorRegistry->GetRegistryItem(type);
        return ASSERT_NOTNULL(mv_factory)->Create(unit);
    }
    
    GameObjectAI* SelectGameObjectAI(GameObject *go)
    {
        // scriptname in db
        if (GameObjectAI* scriptedAI = sScriptMgr->GetGameObjectAI(go))
            return scriptedAI;

        return SelectFactory<GameObjectAI>(go)->Create(go);
    }
} // namespace FactorySelector

