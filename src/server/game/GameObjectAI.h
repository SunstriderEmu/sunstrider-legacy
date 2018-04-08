
#ifndef TRINITY_GAMEOBJECTAI_H
#define TRINITY_GAMEOBJECTAI_H

#include "Define.h"
#include <list>
#include "CreatureAI.h"
#include "Object.h"
#include "GameObject.h"
#include "Optional.h"

enum GOState: uint32;
enum LootState: uint32;

class TC_GAME_API GameObjectAI
{
    protected:
        GameObject* const me;
    public:
        explicit GameObjectAI(GameObject *g) : me(g) {}
        virtual ~GameObjectAI() = default;

        virtual void UpdateAI(const uint32 diff) {}

        virtual void InitializeAI() { Reset(); }

        virtual void Reset() {};
        
        static int Permissible(const GameObject* go);
        
        // Called when a player opens a gossip dialog with the gameobject.
        virtual bool GossipHello(Player* /*player*/) { return false; }
        // Called when a Player clicks a GameObject, before GossipHello
        virtual bool OnReportUse(Player* /*player*/) { return false; } // (LK) prevents achievement tracking if returning true

        // Called when a player selects a gossip item in the gameobject's gossip menu.
        virtual bool GossipSelect(Player* player, uint32 menuId, uint32 gossipListId) { return false; }

        // Called when a player selects a gossip with a code in the gameobject's gossip menu.
        virtual bool GossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) { return false; }

        virtual void QuestAccept(Player* player, Quest const* quest) {  }

        virtual void QuestReward(Player* player, Quest const* quest, uint32 opt) { }

        // Called when the dialog status between a player and the gameobject is requested.
        virtual Optional<QuestGiverStatus> GetDialogStatus(Player* /*player*/) { return boost::none; }

        virtual void Destroyed(Player* player, uint32 eventId) { }
        virtual void SetData(uint32 id, uint32 value, Unit* setter = nullptr) { }
        virtual void GetData(uint32 id) const { }
        virtual void EventInform(uint32 /*eventId*/) { }

        virtual void OnGameEvent(bool /*start*/, uint16 /*eventId*/) { }
        virtual void OnStateChanged(GOState /*state*/, Unit* /*unit*/) { }
        virtual void OnLootStateChanged(LootState /*state*/, Unit* /*unit*/) { }

        // Called when hit by a spell
        virtual void SpellHit(Unit* /*caster*/, SpellInfo const* /*spellInfo*/) { }
        virtual void SpellHit(GameObject* /*caster*/, SpellInfo const* /*spellInfo*/) { }

        // Called when spell hits a target
        virtual void SpellHitTarget(Unit* /*target*/, SpellInfo const* /*spellInfo*/) { }
        virtual void SpellHitTarget(GameObject* /*target*/, SpellInfo const* /*spellInfo*/) { }
};

class TC_GAME_API NullGameObjectAI : public GameObjectAI
{
    public:
        explicit NullGameObjectAI(GameObject *g);

        void UpdateAI(const uint32) override {}

        static int Permissible(const GameObject* go) { return PERMIT_BASE_IDLE; }
};
#endif

