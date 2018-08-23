#include "Chat.h"
#include "Language.h"

inline Pet* GetSelectedPlayerPetOrOwn(ChatHandler* handler)
{
    if (Unit* target = handler->GetSelectedUnit())
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
            return target->ToPlayer()->GetPet();
        if (target->IsPet())
            return target->ToPet();
        return nullptr;
    }
    Player* player = handler->GetSession()->GetPlayer();
    return player ? player->GetPet() : nullptr;
}

class pet_commandscript : public CommandScript
{
public:
    pet_commandscript() : CommandScript("pet_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> petCommandTable =
        {
            { "create",         SEC_GAMEMASTER2,     false, &HandleCreatePetCommand,           "" },
            { "learn",          SEC_GAMEMASTER2,     false, &HandlePetLearnCommand,            "" },
            { "unlearn",        SEC_GAMEMASTER2,     false, &HandlePetUnlearnCommand,          "" },
            { "tp",             SEC_GAMEMASTER2,     false, &HandlePetTpCommand,               "" },
            { "rename",         SEC_GAMEMASTER2,     false, &HandlePetRenameCommand,           "" },
            { "happy",          SEC_GAMEMASTER1,     false, &HandlePetHappyCommand,           "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "pet",            SEC_GAMEMASTER2,  false, nullptr,                       "", petCommandTable },
        };
        return commandTable;
    }

    static bool HandleCreatePetCommand(ChatHandler* handler, char const* args)
    {
        Player *player = handler->GetSession()->GetPlayer();
        Creature *creatureTarget = handler->GetSelectedCreature();

        if (!creatureTarget || creatureTarget->IsPet() || creatureTarget->GetTypeId() == TYPEID_PLAYER)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creatureTarget->GetEntry());
        // Creatures with family 0 crashes the server
        if (cInfo->family == CREATURE_FAMILY_NONE)
        {
            handler->PSendSysMessage("This creature cannot be tamed. (family id: 0).");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetPetGUID())
        {
            handler->PSendSysMessage("You already have a pet.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Everything looks OK, create new pet
        Pet* pet = new Pet(player, HUNTER_PET);
        if (!pet->CreateBaseAtCreature(creatureTarget))
        {
            delete pet;
            handler->PSendSysMessage("Error 1");
            return false;
        }

        creatureTarget->DespawnOrUnsummon();
        creatureTarget->SetHealth(0); // just for nice GM-mode view

        pet->SetGuidValue(UNIT_FIELD_CREATEDBY, player->GetGUID());
        pet->SetCreatorGUID(player->GetGUID());
        pet->SetFaction(player->GetFaction());

        if (!pet->InitStatsForLevel(creatureTarget->GetLevel()))
        {
            TC_LOG_ERROR("command", "ERROR: InitStatsForLevel() in EffectTameCreature failed! Pet deleted.");
            handler->PSendSysMessage("Error 2");
            delete pet;
            return false;
        }

        // prepare visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, creatureTarget->GetLevel() - 1);

        pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
        // this enables pet details window (Shift+P)
        pet->AIM_Initialize();
        pet->InitPetCreateSpells();
        pet->SetHealth(pet->GetMaxHealth());

        player->GetMap()->AddToMap(pet->ToCreature(), true);

        // visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, creatureTarget->GetLevel());

        pet->SetLoyaltyLevel(BEST_FRIEND);
        pet->SetPower(POWER_HAPPINESS, 1050000); //maxed

        player->SetMinion(pet, true);
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        player->PetSpellInitialize();

        return true;
    }

    static bool HandlePetLearnCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Pet* pet = GetSelectedPlayerPetOrOwn(handler);
        if (!pet)
        {
            handler->PSendSysMessage("You have no pet");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (!spellId || !sSpellMgr->GetSpellInfo(spellId))
            return false;

        // Check if pet already has it
        if (pet->HasSpell(spellId))
        {
            handler->PSendSysMessage("Pet already has spell: %u.", spellId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Check if spell is valid
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo))
        {
            handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spellId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        pet->LearnSpell(spellId);

        handler->PSendSysMessage("Pet has learned spell %u.", spellId);
        return true;
    }

    static bool HandlePetUnlearnCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Pet* pet = GetSelectedPlayerPetOrOwn(handler);
        if (!pet)
        {
            //SendSysMessage(LANG_SELECT_PLAYER_OR_PET);
            handler->PSendSysMessage("You have no pet.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 spellId = handler->extractSpellIdFromLink((char*)args);

        if (pet->HasSpell(spellId))
            pet->RemoveSpell(spellId, false);
        else
            handler->PSendSysMessage("Pet doesn't have that spell.");

        return true;
    }

    static bool HandlePetTpCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Pet* pet = GetSelectedPlayerPetOrOwn(handler);
        if (!pet)
        {
            handler->PSendSysMessage("You have no pet.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 tp = atol(args);

        pet->SetTP(tp);

        handler->PSendSysMessage("Pet's tp changed to %u.", tp);
        return true;
    }

    static bool HandlePetHappyCommand(ChatHandler* handler, char const* args)
    {
        Pet* pet = GetSelectedPlayerPetOrOwn(handler);
        if (!pet)
        {
            //SendSysMessage(LANG_SELECT_PLAYER_OR_PET);
            handler->PSendSysMessage("You have no pet.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        pet->SetLoyaltyLevel(BEST_FRIEND);
        pet->SetPower(POWER_HAPPINESS, 1050000); //maxed
        return true;
    }

    static bool HandlePetRenameCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Pet* targetPet = GetSelectedPlayerPetOrOwn(handler);
        if (!targetPet)
        {
            //SendSysMessage(LANG_SELECT_PLAYER_OR_PET);
            handler->PSendSysMessage("Select a pet or pet owner.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (targetPet->getPetType() != HUNTER_PET)
        {
            handler->SendSysMessage("Must select a hunter pet");
            handler->SetSentErrorMessage(true);
            return false;
        }

        Unit *owner = targetPet->GetOwner();
        if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
            (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);

        targetPet->SetName(args);
        targetPet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(nullptr));

        return true;
    }
};

void AddSC_pet_commandscript()
{
    new pet_commandscript();
}
