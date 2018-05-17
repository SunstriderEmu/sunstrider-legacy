//
#include "../../playerbot.h"
#include "SpellAuras.h"
#include "CheckMountStateAction.h"
#include "Unit.h"
#include "SpellAuraEffects.h"

using namespace ai;

ObjectGuid extractGuid(WorldPacket& packet);

bool CheckMountStateAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!bot->GetGroup() || !master)
        return false;

    if (bot->IsFlying())
        return false;

    if (master->IsMounted() && !bot->IsMounted())
    {
        return Mount();
    }
    else if (!master->IsMounted() && bot->IsMounted())
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        return true;
    }
    return false;
}


bool CheckMountStateAction::Mount()
{
    Player* master = GetMaster();
    ai->RemoveShapeshift();

    Unit::AuraEffectList const& auras = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
    if (auras.empty()) 
        return false;

    const SpellInfo* masterSpell = auras.front()->GetBase()->GetSpellInfo();
    int32 masterSpeed = max(masterSpell->Effects[1].BasePoints, masterSpell->Effects[2].BasePoints);

    map<uint32, map<int32, vector<uint32> > > allSpells;
#ifdef LICH_KING
    for(PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;
        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);

        if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        if(itr->second->state == PLAYERSPELL_REMOVED || itr->second->disabled || spellInfo->IsPassive())
            continue;

        int32 effect = max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints);
        if (effect < masterSpeed)
            continue;

        uint32 index = (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;
        allSpells[index][effect].push_back(spellId);
    }
#else
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag* bag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!bag)
            continue;

        for (uint32 j = 0; j < bag->GetBagSize(); j++)
        {
            Item* pItem = bag->GetItemByPos(j);
            if (!pItem)
                continue;

            uint32 spellId = pItem->GetTemplate()->Spells[0].SpellId;
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

            if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED || spellInfo->IsPassive())
                continue;

            int32 effect = max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints);
            if (effect < masterSpeed)
                continue;

            uint32 index = (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;
            allSpells[index][effect].push_back(spellId);
        }
    }
#endif
    int masterMountType = (masterSpell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
        masterSpell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;

    map<int32, vector<uint32> >& spells = allSpells[masterMountType];
    for (map<int32, vector<uint32> >::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        vector<uint32>& ids = i->second;
        int index = urand(0, ids.size() - 1);
        if (index >= ids.size())
            continue;

        ai->CastSpell(ids[index], bot);
        return true;
    }

    return false;
}
