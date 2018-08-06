
#ifndef TRINITY_POOLHANDLER_H
#define TRINITY_POOLHANDLER_H

#include "Define.h"
#include "Creature.h"
#include "GameObject.h"
#include "SpawnData.h"
#include "QuestDef.h"

struct PoolTemplateData
{
    uint32  MaxLimit;
    float   MaxLimitPercent; //sun custom
};

struct PoolObject
{
    ObjectGuid::LowType  guid;
    float   chance;
    PoolObject(ObjectGuid::LowType _guid, float _chance) : guid(_guid), chance(std::fabs(_chance)) { }
};

class Pool                                                  // for Pool of Pool case
{
};

typedef std::set<ObjectGuid::LowType> ActivePoolObjects;
typedef std::map<uint32, uint32> ActivePoolPools;

class TC_GAME_API ActivePoolData
{
    public:
        template<typename T>
        bool IsActiveObject(uint32 db_guid_or_pool_id) const;

        uint32 GetActiveObjectCount(uint32 pool_id) const;

        template<typename T>
        void ActivateObject(uint32 db_guid_or_pool_id, uint32 pool_id);

        template<typename T>
        void RemoveObject(uint32 db_guid_or_pool_id, uint32 pool_id);

        //ActivePoolObjects GetActiveQuests() const { return mActiveQuests; } // a copy of the set
    private:
        ActivePoolObjects mSpawnedCreatures;
        ActivePoolObjects mSpawnedGameobjects;
        //ActivePoolObjects mActiveQuests;
        ActivePoolPools   mSpawnedPools;
};

template <class T>
class TC_GAME_API PoolGroup
{
    typedef std::vector<PoolObject> PoolObjectList;
    public:
        explicit PoolGroup() : poolId(0) { }
        void SetPoolId(uint32 pool_id) { poolId = pool_id; }
        ~PoolGroup() { };
        bool isEmpty() const { return ExplicitlyChanced.empty() && EqualChanced.empty(); }
        void AddEntry(PoolObject& poolitem, uint32 maxentries);
        bool CheckPool() const;
        PoolObject* RollOne(ActivePoolData& spawns, uint32 triggerFrom);
        void DespawnObject(ActivePoolData& spawns, ObjectGuid::LowType guid=0);
        void Despawn1Object(ObjectGuid::LowType guid);
        void SpawnObject(ActivePoolData& spawns, uint32 limit, float limitPercent, uint32 triggerFrom);

        void Spawn1Object(PoolObject* obj);
        void ReSpawn1Object(PoolObject* obj);
        void RemoveOneRelation(uint32 child_pool_id);
        uint32 GetFirstEqualChancedObjectId()
        {
            if (EqualChanced.empty())
                return 0;
            return EqualChanced.front().guid;
        }
        uint32 GetPoolId() const { return poolId; }
    private:
        uint32 poolId;
        PoolObjectList ExplicitlyChanced;
        PoolObjectList EqualChanced;
};
/*
typedef std::multimap<uint32, uint32> PooledQuestRelation;
typedef std::pair<PooledQuestRelation::const_iterator, PooledQuestRelation::const_iterator> PooledQuestRelationBounds;
typedef std::pair<PooledQuestRelation::iterator, PooledQuestRelation::iterator> PooledQuestRelationBoundsNC;
*/
class PoolsMaxLimitTest;

class TC_GAME_API PoolMgr
{
    friend class PoolsMaxLimitTest;

    private:
        PoolMgr();
        ~PoolMgr() { };

    public:
        typedef std::unordered_map<uint32, PoolTemplateData>      PoolTemplateDataMap;
        typedef std::unordered_map<uint32, PoolGroup<Creature>>   PoolGroupCreatureMap;
        typedef std::unordered_map<uint32, PoolGroup<GameObject>> PoolGroupGameObjectMap;
        typedef std::unordered_map<uint32, PoolGroup<Pool>>       PoolGroupPoolMap;
        //typedef std::unordered_map<uint32, PoolGroup<Quest>>      PoolGroupQuestMap;

        static PoolMgr* instance();

        void LoadFromDB();
        /*
        void LoadQuestPools();
        void SaveQuestsToDB();
        */

        void Initialize();

        template<typename T>
        uint32 IsPartOfAPool(uint32 db_guid_or_pool_id) const;
        uint32 IsPartOfAPool(SpawnObjectType type, ObjectGuid::LowType spawnId) const;

        template<typename T>
        bool IsSpawnedObject(uint32 db_guid_or_pool_id) const { return mSpawnedData.IsActiveObject<T>(db_guid_or_pool_id); }

        bool CheckPool(uint32 pool_id) const;

        void SpawnPool(uint32 pool_id);
        void DespawnPool(uint32 pool_id);

        template<typename T>
        void UpdatePool(uint32 pool_id, uint32 db_guid_or_pool_id);

        template<typename T>
        void RemoveActiveObject(uint32 pool_id, uint32 db_guid_or_pool_id);

        /*
        void ChangeDailyQuests();
        void ChangeWeeklyQuests();

        PooledQuestRelation mQuestCreatureRelation;
        PooledQuestRelation mQuestGORelation;
        */

    private:
        template<typename T>
        void SpawnPool(uint32 pool_id, uint32 db_guid_or_pool_id);
        template<typename T, typename P>
        void SpawnPool(uint32 pool_id, uint32 db_guid_or_pool_id, P& poolGroup); //sun refactor to avoid code duplication

        typedef std::pair<uint32, uint32>           SearchPair;
        typedef std::map<uint32 /*guid*/, uint32 /*poolId*/>            SearchMap;

        PoolTemplateDataMap    mPoolTemplate;
        PoolGroupCreatureMap   mPoolCreatureGroups;
        PoolGroupGameObjectMap mPoolGameobjectGroups;
        PoolGroupPoolMap       mPoolPoolGroups;
        //PoolGroupQuestMap      mPoolQuestGroups;
        SearchMap mCreatureSearchMap;
        SearchMap mGameobjectSearchMap;
        SearchMap mPoolSearchMap;
        //SearchMap mQuestSearchMap;

        // LoadFromDB helpers
        template<typename T>
        void LoadEntry(uint32 poolid, ObjectGuid::LowType db_guid_or_pool_id, float chance);
        template<typename T, typename G, typename S>
        void LoadEntry(G& groupsMap, S& searchMap, uint32 poolid, ObjectGuid::LowType db_guid_or_pool_id, float chance);
        void LoadPoolTemplate(uint32 poolId, uint32 maxLimit, float maxLimitPercent);

        // For testing, probably inefficient
        template<typename T>
        uint32 GetFirstFreePoolId() const;

        // dynamic data
        ActivePoolData mSpawnedData;
};

#define sPoolMgr PoolMgr::instance()

// Method that tell if the creature is part of a pool and return the pool id if yes
template<>
inline uint32 PoolMgr::IsPartOfAPool<Creature>(uint32 db_guid) const
{
    SearchMap::const_iterator itr = mCreatureSearchMap.find(db_guid);
    if (itr != mCreatureSearchMap.end())
        return itr->second;

    return 0;
}

// Method that tell if the gameobject is part of a pool and return the pool id if yes
template<>
inline uint32 PoolMgr::IsPartOfAPool<GameObject>(uint32 db_guid) const
{
    SearchMap::const_iterator itr = mGameobjectSearchMap.find(db_guid);
    if (itr != mGameobjectSearchMap.end())
        return itr->second;

    return 0;
}

// Method that tell if the quest is part of another pool and return the pool id if yes
/*
template<>
inline uint32 PoolMgr::IsPartOfAPool<Quest>(uint32 pool_id) const
{
    SearchMap::const_iterator itr = mQuestSearchMap.find(pool_id);
    if (itr != mQuestSearchMap.end())
        return itr->second;

    return 0;
}
*/

// Method that tell if the pool is part of another pool and return the pool id if yes
template<>
inline uint32 PoolMgr::IsPartOfAPool<Pool>(uint32 pool_id) const
{
    SearchMap::const_iterator itr = mPoolSearchMap.find(pool_id);
    if (itr != mPoolSearchMap.end())
        return itr->second;

    return 0;
}

#endif
