
#ifndef TRINITYCORE_PET_H
#define TRINITYCORE_PET_H

#include "PetDefines.h"
#include "TemporarySummon.h"

#define HAPPINESS_LEVEL_SIZE        333000

extern const uint32 LevelUpLoyalty[6];
extern const uint32 LevelStartLoyalty[6];

typedef std::unordered_map<uint32, PetSpell> PetSpellMap;
typedef std::map<uint32, uint32> TeachSpellMap;
typedef std::vector<uint32> AutoSpellList;

class Player;


class TC_GAME_API Pet : public Guardian
{
    public:
        explicit Pet(Player* owner, PetType type = MAX_PET_TYPE);
        ~Pet() override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        void SetDisplayId(uint32 modelId) override;

        PetType getPetType() const { return m_petType; }
        void setPetType(PetType type) { m_petType = type; }
        bool isControlled() const { return getPetType()==SUMMON_PET || getPetType()==HUNTER_PET; }
        bool isTempSummoned() const { return m_duration > 0; }

		bool IsPermanentPetFor(Player* owner) const;        // pet have tab in character windows and set UNIT_FIELD_PETNUMBER

        bool Create (ObjectGuid::LowType guidlow, Map *map, uint32 phaseMask, uint32 Entry, uint32 pet_number);
        bool CreateBaseAtCreature(Creature* creature);
        bool CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner);
        bool CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask);
        bool LoadPetFromDB(Player* owner,uint32 petentry = 0,uint32 petnumber = 0, bool current = false );
        bool IsLoading() const override { return m_loading; }
        void SavePetToDB(PetSaveMode mode);
        void Remove(PetSaveMode mode, bool returnreagent = false);
        static void DeleteFromDB(ObjectGuid::LowType guidlow);

        void SetDeathState(DeathState s) override;                   // overwrite virtual Creature::setDeathState and Unit::setDeathState
        void Update(uint32 diff) override;                           // overwrite virtual Creature::Update and Unit::Update

        uint8 GetPetAutoSpellSize() const override { return m_autospells.size(); }
        uint32 GetPetAutoSpellOnPos(uint8 pos) const override
        {
            if (pos >= m_autospells.size())
                return 0;
            else
                return m_autospells[pos];
        }

        void LooseHappiness();
		HappinessState GetHappinessState();
        void TickLoyaltyChange();
        void ModifyLoyalty(int32 addvalue);
        uint32 GetMaxLoyaltyPoints(uint32 level);
        uint32 GetStartLoyaltyPoints(uint32 level);
        void KillLoyaltyBonus(uint32 level);
        uint32 GetLoyaltyLevel() { return GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_PET_LOYALTY); }
        void SetLoyaltyLevel(LoyaltyLevel level);
        void GivePetXP(uint32 xp);
        void GivePetLevel(uint32 level);
        void SynchronizeLevelWithOwner();
        //bool InitStatsForLevel(uint32 level);
        bool HaveInDiet(ItemTemplate const* item) const;
        uint32 GetCurrentFoodBenefitLevel(uint32 itemlevel);
        void SetDuration(int32 dur) { m_duration = dur; }
        int32 GetDuration() const { return m_duration; }

        int32 GetBonusDamage() { return m_bonusdamage; }
        void SetBonusDamage(int32 damage) { m_bonusdamage = damage; }

        bool   CanTakeMoreActiveSpells(uint32 SpellIconID);
        void   ToggleAutocast(SpellInfo const* spellInfo, bool apply);
        bool   HasTPForSpell(uint32 spellid);
        int32  GetTPForSpell(uint32 spellid);

        bool HasSpell(uint32 spell) const override;
        void AddTeachSpell(uint32 learned_id, uint32 source_id) { m_teachspells[learned_id] = source_id; }

        void LearnPetPassives();
        void CastPetAuras(bool current);
        void CastPetAura(PetAura const* aura);

        void _LoadSpellCooldowns();
        void _LoadAuras(uint32 timediff);
        void _SaveAuras(SQLTransaction& trans);
        void _LoadSpells();
        void _SaveSpells(SQLTransaction& trans);

        bool AddSpell(uint32 spell_id, ActiveStates active = ACT_DECIDE, PetSpellState state = PETSPELL_NEW, PetSpellType type = PETSPELL_NORMAL);
        bool LearnSpell(uint32 spell_id);
        bool RemoveSpell(uint32 spell_id, bool learn_prev, bool clear_ab = true);
        bool UnlearnSpell(uint32 spell_id, bool learn_prev, bool clear_ab = true);
        void CleanupActionBar();
        std::string GenerateActionBarData() const;

        PetSpellMap     m_spells;
        TeachSpellMap   m_teachspells;
        AutoSpellList   m_autospells;

        void InitPetCreateSpells();
        void CheckLearning(uint32 spellid);
        uint32 ResetTalentsCost() const;

        bool CanWalk() const override { return true; }
        bool CanSwim() const override { return true; }

        void  SetTP(int32 TP);
        int32 GetDispTP();

        int32   m_TrainingPoints;
        uint32  m_resetTalentsCost;
        time_t  m_resetTalentsTime;

        uint64 GetAuraUpdateMask() { return m_auraUpdateMask; }
        void SetAuraUpdateMask(uint8 slot) { m_auraUpdateMask |= (uint64(1) << slot); }
        void UnsetAuraUpdateMask(uint8 slot) { m_auraUpdateMask &= ~(uint64(1) << slot); }
        void ResetAuraUpdateMask() { m_auraUpdateMask = 0; }

        DeclinedName const* GetDeclinedNames() const { return m_declinedname; }

        bool    m_removed;                                  // prevent overwrite pet state in DB at next Pet::Update if pet already removed(saved)

        Player* GetOwner() const;

    protected:
        uint32  m_regenTimer;
        uint32  m_happinessTimer;
        uint32  m_loyaltyTimer;
        PetType m_petType;
        int32   m_duration;                                 // time until unsummon (used mostly for summoned guardians and not used for controlled pets)
        int32   m_loyaltyPoints;
        int32   m_bonusdamage;
        uint64  m_auraUpdateMask;
        bool    m_loading;

        DeclinedName *m_declinedname;

    private:
        void SaveToDB(uint32, uint8) override                        // overwrited of Creature::SaveToDB     - don't must be called
        {
            ABORT();
        }
        void DeleteFromDB() override                                 // overwrited of Creature::DeleteFromDB - don't must be called
        {
            ABORT();
        }
};
#endif

