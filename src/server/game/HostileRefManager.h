#ifndef _HOSTILEREFMANAGER
#define _HOSTILEREFMANAGER

#include "Common.h"
#include "RefManager.h"

class Unit;
class ThreatManager;
class HostileReference;
class SpellInfo;

//=================================================

class TC_GAME_API HostileRefManager : public RefManager<Unit, ThreatManager>
{
    public:
        explicit HostileRefManager(Unit *pOwner) : iOwner(pOwner) { }
        ~HostileRefManager() override;

        Unit* GetOwner() const { return iOwner; }

        // send threat to all my haters for the pVictim
        // The pVictim is then hated by them as well
        // use for buffs and healing threat functionality
        void threatAssist(Unit *pVictim, float threat, SpellInfo const* threatSpell = nullptr, bool pSingleTarget = false, bool skipModifiers = false);

        void addThreatPercent(int32 pValue);

        // The references are not needed anymore
        // tell the source to remove them from the list and free the mem
        void deleteReferences();
        void deleteReferencesOutOfRange(float range);

        HostileReference* getFirst() { return ((HostileReference* ) RefManager<Unit, ThreatManager>::getFirst()); }

        void updateThreatTables();

        void setOnlineOfflineState(bool pIsOnline);

        // set state for one reference, defined by Unit
        void setOnlineOfflineState(Unit *pCreature,bool pIsOnline);

        // delete one reference, defined by Unit
        void deleteReference(Unit *pCreature);
    private:
        Unit *iOwner;
};
//=================================================
#endif

