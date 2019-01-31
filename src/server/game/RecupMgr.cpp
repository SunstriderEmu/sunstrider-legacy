#include "RecupMgr.h"
#include "Chat.h"
#include "Language.h"

bool RecupMgr::RecupProfession(Player* player, RecupProfessionType profession, uint32 maxSkill /*= 375*/)
{
    if (maxSkill > 375)
        return false;

    switch (profession)
    {
    case RECUP_PROFESSION_FIRST_AID:
        player->SetSkill(129, 4, maxSkill, maxSkill); //first aid
        player->LearnSpell(27028, false); //first aid spell
        player->LearnSpell(27033, false); //bandage
        if(maxSkill >= 300)
            player->StoreNewItemInBestSlots(21990, 20); // netherweave bandages
        break;
    case RECUP_PROFESSION_COOKING:
        player->SetSkill(185, 4, maxSkill, maxSkill);
        player->LearnSpell(33359, false);
        break;
    case RECUP_PROFESSION_FISHING:
        player->SetSkill(356, 4, maxSkill, maxSkill);
        player->LearnSpell(33095, false);
        player->StoreNewItemInBestSlots(6256, 1); // Fishing pole
        break;
    case RECUP_PROFESSION_ENCHANTMENT:
        player->SetSkill(333, 4, maxSkill, maxSkill);
        player->LearnSpell(28029, false);
        if (maxSkill >= 300)
            player->StoreNewItemInBestSlots(16207, 1); // Runed arcanite rod
        break;
    case RECUP_PROFESSION_MINING:
        player->SetSkill(186, 4, maxSkill, maxSkill);
        player->LearnSpell(29354, false);
        player->StoreNewItemInBestSlots(2901, 1); //mining pick
        break;
    case RECUP_PROFESSION_FORGE:
        player->SetSkill(164, 4, maxSkill, maxSkill);
        player->LearnSpell(29844, false);
        break;
    case RECUP_PROFESSION_ENGINEERING:
        player->SetSkill(202, 4, maxSkill, maxSkill);
        player->LearnSpell(30350, false);
        break;
    case RECUP_PROFESSION_LEATHERWORKING:
        player->SetSkill(165, 4, maxSkill, maxSkill);
        player->LearnSpell(32549, false);
        break;
    case RECUP_PROFESSION_SKINNING:
        player->SetSkill(393, 4, maxSkill, maxSkill);
        player->LearnSpell(32678, false);
        break;
    case RECUP_PROFESSION_ALCHEMY:
        player->SetSkill(171, 4, maxSkill, maxSkill);
        player->LearnSpell(28596, false);
        break;
    case RECUP_PROFESSION_TAILORING:
        player->SetSkill(197, 4, maxSkill, maxSkill);
        player->LearnSpell(26790, false);
        break;
    case RECUP_PROFESSION_JEWELCRAFTING:
        player->SetSkill(755, 4, maxSkill, maxSkill);
        player->LearnSpell(28897, false);
        break;
    case RECUP_PROFESSION_HERBALISM:
        player->SetSkill(182, 4, maxSkill, maxSkill);
        player->LearnSpell(28695, false);
        break;
    case RECUP_PROFESSION_LOCKPICKING:
        player->SetSkill(633, 2, 225, 225);
        player->LearnSpell(6463, false);
        break;
    default:
        return false;
    }
    return true;
}