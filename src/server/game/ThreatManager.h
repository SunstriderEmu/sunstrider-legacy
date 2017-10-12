#ifndef _THREATMANAGER
#define _THREATMANAGER

#include "SharedDefines.h"
#include "Reference.h"
#include "UnitEvents.h"

#include <list>

//==============================================================

class Unit;
class Creature;
class ThreatManager;
class SpellInfo;

//==============================================================
// Class to calculate the real threat based

class TC_GAME_API ThreatCalcHelper
{
    public:
        static float calcThreat(Unit* pHatedUnit, Unit* pHatingUnit, float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const *threatSpell = nullptr);
        static bool isValidProcess(Unit* hatedUnit, Unit* hatingUnit, SpellInfo const* threatSpell = nullptr);
};

//==============================================================

class TC_GAME_API HostileReference : public Reference<Unit, ThreatManager>
{
    private:
        float iThreat;
        float iTempThreatModifier;                          // used for taunt and for SPELL_AURA_MOD_TOTAL_THREAT (ex: Fade spell)
        uint64 iUnitGuid;
        bool iOnline;
        bool iAccessible;
    private:
        // Inform the source, that the status of that reference was changed
        void fireStatusChanged(const ThreatRefStatusChangeEvent& pThreatRefStatusChangeEvent);

    public:
        Unit* getSourceUnit();
        HostileReference(Unit* pUnit, ThreatManager *pThreatManager, float pThreat);

        //=================================================
        void addThreat(float pMod);

        void setThreat(float pThreat) { addThreat(pThreat - iThreat); }

        void addThreatPercent(int32 percent);

        float getThreat() const { return iThreat + iTempThreatModifier; }

        bool isOnline() const { return iOnline; }

        // The Unit might be in water and the creature can not enter the water, but has range attack
        // in this case online = true, but accessible = false
        bool isAccessible() const { return iAccessible; }

        // used for temporary setting a threat and reducing it later again.
        // the threat modification is stored
        void setTempThreat(float threat) 
        { 
            addTempThreat(threat - iTempThreatModifier);
        }

        void addTempThreat(float threat)          
        {
            iTempThreatModifier += threat;
                
            ThreatRefStatusChangeEvent event(UEV_THREAT_REF_THREAT_CHANGE, this, threat);
            fireStatusChanged(event);
        }                 

        void resetTempThreat()
        {
            addTempThreat(-iTempThreatModifier);
            ASSERT(iTempThreatModifier == 0);
        }

        float getTempThreatModifier() { return iTempThreatModifier; }

        //=================================================
        // check, if source can reach target and set the status
        void updateOnlineStatus();

        void setOnlineOfflineState(bool pIsOnline);

        void setAccessibleState(bool pIsAccessible);
        //=================================================

        bool operator ==(const HostileReference& pHostileReference) const { return pHostileReference.getUnitGuid() == getUnitGuid(); }

        //=================================================

        uint64 getUnitGuid() const { return iUnitGuid; }

        //=================================================
        // reference is not needed anymore. realy delete it !

        void removeReference();

        //=================================================

        HostileReference* next() { return ((HostileReference* ) Reference<Unit, ThreatManager>::next()); }

        //=================================================

        // Tell our refTo (target) object that we have a link
        void targetObjectBuildLink() override;

        // Tell our refTo (taget) object, that the link is cut
        void targetObjectDestroyLink() override;

        // Tell our refFrom (source) object, that the link is cut (Target destroyed)
        void sourceObjectDestroyLink() override;
};

//==============================================================
class ThreatManager;

class TC_GAME_API ThreatContainer
{
    public:
        typedef std::list<HostileReference*> StorageType;
    private:
        StorageType iThreatList;
        bool iDirty;
    protected:
        friend class ThreatManager;

        void remove(HostileReference* pRef) { iThreatList.remove(pRef); }

        void addReference(HostileReference* pHostileReference) { iThreatList.push_back(pHostileReference); }
        void clearReferences();
        // Sort the list if necessary
        void update();
    public:

        ThreatContainer() { iDirty = false; }
        ~ThreatContainer() { clearReferences(); }

        HostileReference* addThreat(Unit* pVictim, float pThreat);
        
        //modify threat for victim. Do not remove from threat list in any case
        void modifyThreatPercent(Unit *pVictim, int32 percent);

        HostileReference* selectNextVictim(Creature* pAttacker, HostileReference* pCurrentVictim);

        void setDirty(bool pDirty) { iDirty = pDirty; }

        bool isDirty() const { return iDirty; }

        bool empty() const { return(iThreatList.empty()); }

        HostileReference* getMostHated() { return iThreatList.empty() ? nullptr : iThreatList.front(); }

        HostileReference* getReferenceByTarget(Unit const* pVictim) const;

        StorageType& getThreatList() { return iThreatList; }
        StorageType const& getThreatList() const { return iThreatList; }
};

//=================================================

class TC_GAME_API ThreatManager
{
    private:
        HostileReference* iCurrentVictim;
        Unit* iOwner;
        ThreatContainer iThreatContainer;
        ThreatContainer iThreatOfflineContainer;

        void _addThreat(Unit* target, float threat);

public:

    void AddThreat(Unit* victim, float amount, SpellInfo const* spell = nullptr, bool ignoreModifiers = false, bool ignoreRedirection = false);
    bool IsThreatenedBy(Unit const* who, bool includeOffline = false) const { return (FindReference(who, includeOffline) != nullptr); }
    void ClearAllThreat();

private:
    HostileReference* FindReference(Unit const* who, bool includeOffline) const { if (auto* ref = iThreatContainer.getReferenceByTarget(who)) return ref; if (includeOffline) if (auto* ref = iThreatOfflineContainer.getReferenceByTarget(who)) return ref; return nullptr; }

public:

    public:
        explicit ThreatManager(Unit *pOwner);

        ~ThreatManager() { clearReferences(); }

        void clearReferences();
        void clearCurrentVictim() { iCurrentVictim = nullptr; }

        void addThreat(Unit* pVictim, float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const *threatSpell = nullptr);

        void doAddThreat(Unit* victim, float threat);

        //modify threat for victim. Do not remove from threat list in any case
        void modifyThreatPercent(Unit *pVictim, int32 pPercent);

        float getThreat(Unit *pVictim, bool pAlsoSearchOfflineList = false) const;

        bool IsThreatListEmpty(bool includeOffline = false) const { return includeOffline ? areThreatListsEmpty() : isThreatListEmpty(); }
        bool isThreatListEmpty() const { return iThreatContainer.empty(); }
		bool areThreatListsEmpty() const { return iThreatContainer.empty() && iThreatOfflineContainer.empty(); }

        bool processThreatEvent(const UnitBaseEvent* pUnitBaseEvent);

        HostileReference* GetCurrentVictim() const { return iCurrentVictim; }

        Unit*  GetOwner() { return iOwner; }

        Unit* getHostilTarget();

        void tauntApply(Unit* pTaunter);
        void tauntFadeOut(Unit *pTaunter);
        
        void detauntApply(Unit* pDetaunter);
        void detauntFadeOut(Unit* pDetaunter);

        void setCurrentVictim(HostileReference* pHostileReference);

        void setDirty(bool pDirty) { iThreatContainer.setDirty(pDirty); }

        // methods to access the lists from the outside to do sume dirty manipulation (scriping and such)
        // I hope they are used as little as possible.
        inline std::list<HostileReference*>& getThreatList() { return iThreatContainer.getThreatList(); }
        inline std::list<HostileReference*> const& getThreatList() const { return iThreatContainer.getThreatList(); }
        inline std::list<HostileReference*>& getOfflineThreatList() { return iThreatOfflineContainer.getThreatList(); }
        inline ThreatContainer& getOnlineContainer() { return iThreatContainer; }
        inline ThreatContainer& getOfflineContainer() { return iThreatOfflineContainer; }
};

//=================================================
#endif

