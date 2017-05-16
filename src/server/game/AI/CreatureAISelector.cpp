
#include "Creature.h"
#include "CreatureAIImpl.h"
#include "CreatureAISelector.h"
#include "NullCreatureAI.h"
#include "MovementGenerator.h"
#include "ScriptCalls.h"
#include "Pet.h"
#include "ScriptMgr.h"

namespace FactorySelector
{
    CreatureAI* selectAI(Creature *creature)
    {
        const CreatureAICreator *ai_factory = nullptr;

        // Always PetAI for hunter pets
        if (creature->IsPet() && creature->ToPet()->getPetType() == HUNTER_PET)
            ai_factory = sCreatureAIRegistry->GetRegistryItem("PetAI");

        //scriptname in db
        if ((!ai_factory || !((Pet*)creature)->isControlled()) && !creature->IsCharmed())
            if (CreatureAI* scriptedAI = sScriptMgr->GetCreatureAI(creature))
                return scriptedAI;

        assert( creature->GetCreatureTemplate() != nullptr );
        CreatureTemplate const *cinfo=creature->GetCreatureTemplate();

        if(!ai_factory) // else try to select AI by NPC flags
        {
            std::string ainame = cinfo->AIName;
            if (!ainame.empty())  // select by script name
                ai_factory = sCreatureAIRegistry->GetRegistryItem(ainame.c_str());
#ifdef LICH_KING
            else if (creature->IsVehicle())
                ai_factory = ai_registry.GetRegistryItem("VehicleAI");
#endif
			else if (creature->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN) && ((Guardian*)creature)->GetOwner()->GetTypeId() == TYPEID_PLAYER)
				ai_factory = sCreatureAIRegistry->GetRegistryItem("PetAI");
            else if( creature->IsGuard() )
                ai_factory = sCreatureAIRegistry->GetRegistryItem("GuardAI");
            else if(creature->IsTotem())
                ai_factory = sCreatureAIRegistry->GetRegistryItem("TotemAI");
            else if (creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK))
                ai_factory = sCreatureAIRegistry->GetRegistryItem("NullCreatureAI");
            else if(creature->IsTrigger())
                ai_factory = sCreatureAIRegistry->GetRegistryItem("NullCreatureAI");
			else if (creature->IsCritter() && !creature->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
				ai_factory = sCreatureAIRegistry->GetRegistryItem("CritterAI");
        }

        // Else select by permit check
        if(!ai_factory)
        {
            int best_val = -1;
            typedef CreatureAIRegistry::RegistryMapType RMT;
            RMT const &l = sCreatureAIRegistry->GetRegisteredItems();
            for(const auto & iter : l)
            {
                const CreatureAICreator *factory = iter.second;
                const SelectableAI *p = dynamic_cast<const SelectableAI *>(factory);
                assert( p != nullptr );
                int val = p->Permit(creature);
                if( val > best_val )
                {
                    best_val = val;
                    ai_factory = p;
                }
            }
        }

        return ( ai_factory == nullptr ? new NullCreatureAI(creature) : ai_factory->Create(creature) );
    }

    MovementGenerator* selectMovementGenerator(Creature *creature)
    {
        MovementGeneratorRegistry& mv_registry(*MovementGeneratorRegistry::instance());
        assert( creature->GetCreatureTemplate() != nullptr );
        MovementGeneratorType defaultMovementType = creature->GetDefaultMovementType();
        //override default WAYPOINT_MOTION_TYPE type if no waypoint path id found for creature (or WAYPOINT_MOTION_TYPE will fail anyway)
        if (defaultMovementType == WAYPOINT_MOTION_TYPE && creature->GetWaypointPathId() == 0)
            defaultMovementType = IDLE_MOTION_TYPE;
        const MovementGeneratorCreator *mv_factory = mv_registry.GetRegistryItem( creature->GetDefaultMovementType());

        /* if( mv_factory == NULL  )
        {
            int best_val = -1;
            std::vector<std::string> l;
            mv_registry.GetRegisteredItems(l);
            for( std::vector<std::string>::iterator iter = l.begin(); iter != l.end(); ++iter)
            {
            const MovementGeneratorCreator *factory = mv_registry.GetRegistryItem((*iter).c_str());
            const SelectableMovement *p = dynamic_cast<const SelectableMovement *>(factory);
            assert( p != NULL );
            int val = p->Permit(creature);
            if( val > best_val )
            {
                best_val = val;
                mv_factory = p;
            }
            }
        }*/

        return ( mv_factory == nullptr ? nullptr : mv_factory->Create(creature) );

    }
    
    GameObjectAI* SelectGameObjectAI(GameObject *go)
    {
        const GameObjectAICreator *ai_factory = nullptr;
        
        if (GameObjectAI* scriptedAI = sScriptMgr->GetGameObjectAI(go))
            return scriptedAI;

        ai_factory = sGameObjectAIRegistry->GetRegistryItem(go->GetAIName());

        //future goAI types go here

        std::string ainame = (ai_factory == nullptr) ? "NullGameObjectAI" : ai_factory->key();

        //sLog->outStaticDebug("GameObject %u used AI is %s.", go->GetGUIDLow(), ainame.c_str());

        return (ai_factory == nullptr ? new NullGameObjectAI(go) : ai_factory->Create(go));
    }
} // namespace FactorySelector

