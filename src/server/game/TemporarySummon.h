#ifndef TRINITYCORE_TEMPSUMMON_H
#define TRINITYCORE_TEMPSUMMON_H

#include "Creature.h"
#include "ObjectAccessor.h"

class TC_GAME_API TempSummon : public Creature
{
public:
    explicit TempSummon(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject);
	~TempSummon() {};
    void Update(uint32 time) override;
	virtual void InitStats(uint32 lifetime);
	virtual void InitSummon();
    //void Summon(TempSummonType type, uint32 lifetime, Map* map);
	virtual void UnSummon(uint32 msTime = 0);
	void RemoveFromWorld() override;
    void SetTempSummonType(TempSummonType type);
    void SaveToDB(uint32 mapid, uint8 spawnMask) override { }

    Unit* GetSummoner() const;
	Creature* GetSummonerCreatureBase() const;
    ObjectGuid GetSummonerGUID() const { return m_summonerGUID; }
    TempSummonType GetSummonType() { return m_type; }
	uint32 GetTimer() const { return m_timer; }

	const SummonPropertiesEntry* const m_Properties;
private:
    TempSummonType m_type;
    uint32 m_timer;
    uint32 m_lifetime;
	ObjectGuid m_summonerGUID;
};

class TC_GAME_API Minion : public TempSummon
{
public:
	Minion(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject);
	void InitStats(uint32 duration) override;
	void RemoveFromWorld() override;
	Unit* GetOwner() const { return m_owner; }
	float GetFollowAngle() const override { return m_followAngle; }
	void SetFollowAngle(float angle) { m_followAngle = angle; }
	bool IsPetGhoul() const { return GetEntry() == 26125; } // Ghoul may be guardian or pet
	bool IsSpiritWolf() const { return GetEntry() == 29264; } // Spirit wolf from feral spirits
	bool IsGuardianPet() const;
	bool IsRisenAlly() const { return GetEntry() == 30230; }
protected:
	Unit* const m_owner;
	float m_followAngle;
};

class TC_GAME_API Guardian : public Minion
{
public:
	Guardian(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject);
	void InitStats(uint32 duration) override;
	bool InitStatsForLevel(uint8 level);
	void InitSummon() override;

	bool UpdateStats(Stats stat) override;
	bool UpdateAllStats() override;
	void UpdateResistances(uint32 school) override;
	void UpdateArmor() override;
	void UpdateMaxHealth() override;
	void UpdateMaxPower(Powers power) override;
	void UpdateAttackPowerAndDamage(bool ranged = false) override;
	void UpdateDamagePhysical(WeaponAttackType attType) override;

	int32 GetBonusDamage() const { return m_bonusSpellDamage; }
	float GetBonusStatFromOwner(Stats stat) const { return m_statFromOwner[stat]; }
	void SetBonusDamage(int32 damage);
protected:
	int32   m_bonusSpellDamage;
	float   m_statFromOwner[MAX_STATS];
};

class TC_GAME_API Puppet : public Minion
{
public:
	Puppet(SummonPropertiesEntry const* properties, Unit* owner);
	void InitStats(uint32 duration) override;
	void InitSummon() override;
	void Update(uint32 time) override;
	void RemoveFromWorld() override;
};

class TC_GAME_API ForcedUnsummonDelayEvent : public BasicEvent
{
public:
	ForcedUnsummonDelayEvent(TempSummon& owner) : BasicEvent(), m_owner(owner) { }
	bool Execute(uint64 e_time, uint32 p_time) override;

private:
	TempSummon& m_owner;
};

#endif

