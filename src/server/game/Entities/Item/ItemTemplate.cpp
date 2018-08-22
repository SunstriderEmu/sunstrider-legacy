/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include "Packets/QueryPackets.h"

bool ItemTemplate::CanChangeEquipStateInCombat() const
{
    switch (InventoryType)
    {
        case INVTYPE_RELIC:
        case INVTYPE_SHIELD:
        case INVTYPE_HOLDABLE:
            return true;
    }

    switch (Class)
    {
        case ITEM_CLASS_WEAPON:
        case ITEM_CLASS_PROJECTILE:
            return true;
    }

    return false;
}

uint32 ItemTemplate::GetSkill() const
{
    const static uint32 item_weapon_skills[MAX_ITEM_SUBCLASS_WEAPON] =
    {
        SKILL_AXES,     SKILL_2H_AXES,  SKILL_BOWS,          SKILL_GUNS,         SKILL_MACES,
        SKILL_2H_MACES, SKILL_POLEARMS, SKILL_SWORDS,        SKILL_2H_SWORDS,    0,
        SKILL_STAVES,   0,              0,                   SKILL_FIST_WEAPONS, 0,
        SKILL_DAGGERS,  SKILL_THROWN,   SKILL_ASSASSINATION, SKILL_CROSSBOWS,    SKILL_WANDS,
        SKILL_FISHING
    };

    const static uint32 item_armor_skills[MAX_ITEM_SUBCLASS_ARMOR] =
    {
        0, SKILL_CLOTH, SKILL_LEATHER, SKILL_MAIL, SKILL_PLATE_MAIL, 0, SKILL_SHIELD, 0, 0, 0
    };

    switch (Class)
    {
    case ITEM_CLASS_WEAPON:
        if (SubClass >= MAX_ITEM_SUBCLASS_WEAPON)
            return 0;
        else
            return item_weapon_skills[SubClass];

    case ITEM_CLASS_ARMOR:
        if (SubClass >= MAX_ITEM_SUBCLASS_ARMOR)
            return 0;
        else
            return item_armor_skills[SubClass];

    default:
        return 0;
    }
}

void ItemTemplate::InitializeQueryData()
{
    for (uint8 loc = LOCALE_enUS; loc < TOTAL_LOCALES; ++loc)
        QueryData[loc] = BuildQueryData(static_cast<LocaleConstant>(loc));
}

WorldPacket ItemTemplate::BuildQueryData(LocaleConstant loc) const
{
    WorldPackets::Query::QueryItemSingleResponse response;

    std::string locName = Name1;
    std::string locDescription = Description;

    if (ItemLocale const* il = sObjectMgr->GetItemLocale(ItemId))
    {
        ObjectMgr::GetLocaleString(il->Name, loc, locName);
        ObjectMgr::GetLocaleString(il->Description, loc, locDescription);
    }

    response.ItemID = ItemId;
    response.Allow = true;

    response.Stats.Class = Class;
    response.Stats.SubClass = SubClass;
    response.Stats.SoundOverrideSubclass = SoundOverrideSubclass;
    response.Stats.Name = locName;
    response.Stats.DisplayInfoID = DisplayInfoID;
    response.Stats.Quality = Quality;
    response.Stats.Flags = Flags;
#ifdef LICH_KING
    response.Stats.Flags2 = Flags2;
#endif
    response.Stats.BuyPrice = BuyPrice;
    response.Stats.SellPrice = SellPrice;
    response.Stats.InventoryType = InventoryType;
    response.Stats.AllowableClass = AllowableClass;
    response.Stats.AllowableRace = AllowableRace;
    response.Stats.ItemLevel = ItemLevel;
    response.Stats.RequiredLevel = RequiredLevel;
    response.Stats.RequiredSkill = RequiredSkill;
    response.Stats.RequiredSkillRank = RequiredSkillRank;
    response.Stats.RequiredSpell = RequiredSpell;
    response.Stats.RequiredHonorRank = RequiredHonorRank;
    response.Stats.RequiredCityRank = RequiredCityRank;
    response.Stats.RequiredReputationFaction = RequiredReputationFaction;
    response.Stats.RequiredReputationRank = RequiredReputationRank;
    response.Stats.MaxCount = MaxCount;
    response.Stats.Stackable = Stackable;
    response.Stats.ContainerSlots = ContainerSlots;
#ifdef LICH_KING
    response.Stats.StatsCount = StatsCount;
#else
    response.Stats.StatsCount = MAX_ITEM_PROTO_STATS;
#endif
    for (uint32 i = 0; i < response.Stats.StatsCount; ++i)
    {
        response.Stats.ItemStat[i].ItemStatType = ItemStat[i].ItemStatType;
        response.Stats.ItemStat[i].ItemStatValue = ItemStat[i].ItemStatValue;
    }

#ifdef LICH_KING
    response.Stats.ScalingStatDistribution = ScalingStatDistribution;
    response.Stats.ScalingStatValue = ScalingStatValue;
#endif

    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
    {
        response.Stats.Damage[i].DamageMin = Damage[i].DamageMin;
        response.Stats.Damage[i].DamageMax = Damage[i].DamageMax;
        response.Stats.Damage[i].DamageType = Damage[i].DamageType;
    }

    response.Stats.Resistance[SPELL_SCHOOL_NORMAL] = Armor;
    response.Stats.Resistance[SPELL_SCHOOL_HOLY] = HolyRes;
    response.Stats.Resistance[SPELL_SCHOOL_FIRE] = FireRes;
    response.Stats.Resistance[SPELL_SCHOOL_NATURE] = NatureRes;
    response.Stats.Resistance[SPELL_SCHOOL_FROST] = FrostRes;
    response.Stats.Resistance[SPELL_SCHOOL_SHADOW] = ShadowRes;
    response.Stats.Resistance[SPELL_SCHOOL_ARCANE] = ArcaneRes;

    response.Stats.Delay = Delay;
    response.Stats.AmmoType = AmmoType;
    response.Stats.RangedModRange = RangedModRange;

    for (uint8 s = 0; s < MAX_ITEM_PROTO_SPELLS; ++s)
    {
        response.Stats.Spells[s].SpellId = Spells[s].SpellId;
        response.Stats.Spells[s].SpellTrigger = Spells[s].SpellTrigger;
        response.Stats.Spells[s].SpellCharges = Spells[s].SpellCharges;
        response.Stats.Spells[s].SpellCooldown = Spells[s].SpellCooldown;
        response.Stats.Spells[s].SpellCategory = Spells[s].SpellCategory;
        response.Stats.Spells[s].SpellCategoryCooldown = Spells[s].SpellCategoryCooldown;
    }

    response.Stats.Bonding = Bonding;
    response.Stats.Description = locDescription;
    response.Stats.PageText = PageText;
    response.Stats.LanguageID = LanguageID;
    response.Stats.PageMaterial = PageMaterial;
    response.Stats.StartQuest = StartQuest;
    response.Stats.LockID = LockID;
    response.Stats.Material = Material;
    response.Stats.Sheath = Sheath;
    response.Stats.RandomProperty = RandomProperty;
    response.Stats.RandomSuffix = RandomSuffix;
    response.Stats.Block = Block;
    response.Stats.ItemSet = ItemSet;
    response.Stats.MaxDurability = MaxDurability;
    response.Stats.Area = Area;
    response.Stats.Map = Map;
    response.Stats.BagFamily = BagFamily;
    response.Stats.TotemCategory = TotemCategory;

    for (uint8 s = 0; s < MAX_ITEM_PROTO_SOCKETS; ++s)
    {
        response.Stats.Socket[s].Color = Socket[s].Color;
        response.Stats.Socket[s].Content = Socket[s].Content;
    }

    response.Stats.SocketBonus = socketBonus;
    response.Stats.GemProperties = GemProperties;
    response.Stats.RequiredDisenchantSkill = RequiredDisenchantSkill;
    response.Stats.ArmorDamageModifier = ArmorDamageModifier;
    response.Stats.Duration = Duration;
#ifdef LICH_KING
    response.Stats.ItemLimitCategory = ItemLimitCategory;
    response.Stats.HolidayId = HolidayId;
#endif

    response.Write();
    return response.Move();
}
