#include "Chat.h"
#include "Language.h"
class learn_commandscript : public CommandScript
{
public:
    learn_commandscript() : CommandScript("learn_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> learnCommandTable =
        {
            { "all_gm",         SEC_GAMEMASTER2,  false,  &HandleLearnAllGMCommand,               "" },
            { "all_crafts",     SEC_GAMEMASTER2,  false,  &HandleLearnAllCraftsCommand,           "" },
            { "all_default",    SEC_GAMEMASTER1,  false,  &HandleLearnAllDefaultCommand,          "" },
            { "all_lang",       SEC_GAMEMASTER1,  false,  &HandleLearnAllLangCommand,             "" },
            { "all_myclass",    SEC_GAMEMASTER3,  false,  &HandleLearnAllMyClassCommand,          "" },
            { "all_myproficiencies",SEC_GAMEMASTER3,false,&HandleLearnAllMyProeficienciesCommand, "" },
            { "all_myspells",   SEC_GAMEMASTER3,  false,  &HandleLearnAllMySpellsCommand,         "" },
            { "all_mytalents",  SEC_GAMEMASTER3,  false,  &HandleLearnAllMyTalentsCommand,        "" },
            { "all_recipes",    SEC_GAMEMASTER2,  false,  &HandleLearnAllRecipesCommand,          "" },
            { "",               SEC_GAMEMASTER3,  false,  &HandleLearnCommand,                    "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "learn",          SEC_GAMEMASTER1,  false, nullptr,                                 "", learnCommandTable },
            { "unlearn",        SEC_GAMEMASTER3,  false, &HandleUnLearnCommand,                   "" },
        };
        return commandTable;
    }

    static bool HandleLearnAllGMCommand(ChatHandler* handler, char const* /*args*/)
    {
        static const char *gmSpellList[] =
        {
            "24347",                                            // Become A Fish, No Breath Bar
            "35132",                                            // Visual Boom
            "38488",                                            // Attack 4000-8000 AOE
            "38795",                                            // Attack 2000 AOE + Slow Down 90%
            "15712",                                            // Attack 200
            "1852",                                             // GM Spell Silence
            "31899",                                            // Kill
            "31924",                                            // Kill
            "29878",                                            // Kill My Self
            "26644",                                            // More Kill

            "28550",                                            //Invisible 24
            "23452",                                            //Invisible + Target
            "0"
        };

        uint16 gmSpellIter = 0;
        while( strcmp(gmSpellList[gmSpellIter], "0") )
        {
            uint32 spell = atol((char const*)gmSpellList[gmSpellIter++]);

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
            if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,handler->GetSession()->GetPlayer()))
            {
                handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
                continue;
            }

            handler->GetSession()->GetPlayer()->LearnSpell(spell, false);
        }

        handler->SendSysMessage(LANG_LEARNING_GM_SKILLS);
        return true;
    }

    static bool HandleLearnAllMyClassCommand(ChatHandler* handler, char const* /*args*/)
    {
        HandleLearnAllMySpellsCommand(handler, "");
        HandleLearnAllMyTalentsCommand(handler, "");
        return true;
    }

    static bool HandleLearnAllMyProeficienciesCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->GetSession()->GetPlayer()->LearnAllClassProficiencies();
        return true;
    }

    static bool HandleLearnAllMySpellsCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->GetSession()->GetPlayer()->LearnAllClassSpells();
        handler->SendSysMessage(LANG_COMMAND_LEARN_CLASS_SPELLS);
        return true;
    }

    static void learnAllHighRanks(Player* player, uint32 spellid)
    {
        SpellChainNode const* node;
        do
        {
            node = sSpellMgr->GetSpellChainNode(spellid);
            player->LearnSpell(spellid, false);
            if (!node || !node->next)
                break;
            spellid=node->next->Id;
        }
        while (node->next);
    }

    static bool HandleLearnAllMyTalentsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        uint32 classMask = player->GetClassMask();

        for (uint32 i = 0; i < sTalentStore.GetNumRows(); i++)
        {
            TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
            if(!talentInfo)
                continue;

            TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
            if(!talentTabInfo)
                continue;

            if( (classMask & talentTabInfo->ClassMask) == 0 )
                continue;

            // search highest talent rank
            uint32 spellid = 0;
            int rank = 4;
            for(; rank >= 0; --rank)
            {
                if(talentInfo->RankID[rank]!=0)
                {
                    spellid = talentInfo->RankID[rank];
                    break;
                }
            }

            if(!spellid)                                        // ??? none spells in talent
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,handler->GetSession()->GetPlayer(),false))
                continue;

            // learn highest rank of talent
            player->LearnSpell(spellid, false);

            // and learn all non-talent spell ranks (recursive by tree)
            learnAllHighRanks(player,spellid);
        }

        handler->SendSysMessage(LANG_COMMAND_LEARN_CLASS_TALENTS);
        return true;
    }

    static bool HandleLearnAllLangCommand(ChatHandler* handler, char const* /*args*/)
    {
        // skipping UNIVERSAL language (0)
        for(int i = 1; i < LANGUAGES_COUNT; ++i)
            handler->GetSession()->GetPlayer()->LearnSpell(lang_description[i].spell_id, false);

        handler->SendSysMessage(LANG_COMMAND_LEARN_ALL_LANG);
        return true;
    }

    static bool HandleLearnAllDefaultCommand(ChatHandler* handler, char const* args)
    {
        char* pName = strtok((char*)args, "");
        Player *player = nullptr;
        if (pName)
        {
            std::string name = pName;

            if(!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            player = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
        }
        else
            player = handler->GetSelectedPlayerOrSelf();

        if(!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->LearnDefaultSpells();
        player->learnQuestRewardedSpells();

        handler->PSendSysMessage(LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST,player->GetName().c_str());
        return true;
    }

    static bool HandleLearnCommand(ChatHandler* handler, char const* args)
    {
        Player* targetPlayer = handler->GetSelectedPlayerOrSelf();
        if(!targetPlayer)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spell =  handler->extractSpellIdFromLink((char*)args);
        if(!spell || !sSpellMgr->GetSpellInfo(spell))
            return false;

        if (targetPlayer->HasSpell(spell))
        {
            if(targetPlayer == handler->GetSession()->GetPlayer())
                handler->SendSysMessage(LANG_YOU_KNOWN_SPELL);
            else
                handler->PSendSysMessage(LANG_TARGET_KNOWN_SPELL,targetPlayer->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
        if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,handler->GetSession()->GetPlayer()))
        {
            handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
            handler->SetSentErrorMessage(true);
            return false;
        }

        targetPlayer->LearnSpell(spell, false);

        return true;
    }

    static bool HandleLearnAllRecipesCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        //  Learns all recipes of specified profession and sets skill to max
        //  Example: .learn all_recipes enchanting

        Player* target = handler->GetSelectedPlayerOrSelf();
        if( !target )
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return false;
        }

        std::wstring wnamepart;

        if(!Utf8toWStr(args,wnamepart))
            return false;

        // converting string that we try to find to lower case
        wstrToLower( wnamepart );

        uint32 classmask = handler->GetSession()->GetPlayer()->GetClassMask();

        for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
        {
            SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(i);
            if( !skillInfo )
                continue;

            if( skillInfo->categoryId != SKILL_CATEGORY_PROFESSION &&
                skillInfo->categoryId != SKILL_CATEGORY_SECONDARY )
                continue;

            int loc = handler->GetSessionDbcLocale();
            std::string name = skillInfo->name[loc];

            if(Utf8FitTo(name, wnamepart))
            {
                for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
                {
                    SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
                    if( !skillLine )
                        continue;

                    if( skillLine->skillId != i || skillLine->forward_spellid )
                        continue;

                    // skip racial skills
                    if( skillLine->racemask != 0 )
                        continue;

                    // skip wrong class skills
                    if( skillLine->classmask && (skillLine->classmask & classmask) == 0)
                        continue;

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->spellId);
                    if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,handler->GetSession()->GetPlayer(),false))
                        continue;

                    if( !target->HasSpell(spellInfo->Id) )
                        handler->GetSession()->GetPlayer()->LearnSpell(skillLine->spellId, false);
                }

                uint16 MaxLevel = target->GetPureMaxSkillValue(skillInfo->id);

                uint16 step = (MaxLevel-1) / 75;
                target->SetSkill(skillInfo->id, step, MaxLevel, MaxLevel);
                handler->PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, name.c_str());
                return true;
            }
        }

        return false;
    }

    static bool HandleLearnAllCraftsCommand(ChatHandler* handler, char const* /*args*/)
    {
        uint32 classmask = handler->GetSession()->GetPlayer()->GetClassMask();

        for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
        {
            SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(i);
            if( !skillInfo )
                continue;

            if( skillInfo->categoryId == SKILL_CATEGORY_PROFESSION || skillInfo->categoryId == SKILL_CATEGORY_SECONDARY )
            {
                for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
                {
                    SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
                    if( !skillLine )
                        continue;

                    // skip racial skills
                    if( skillLine->racemask != 0 )
                        continue;

                    // skip wrong class skills
                    if( skillLine->classmask && (skillLine->classmask & classmask) == 0)
                        continue;

                    if( skillLine->skillId != i || skillLine->forward_spellid )
                        continue;

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->spellId);
                    if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,handler->GetSession()->GetPlayer(),false))
                        continue;

                    handler->GetSession()->GetPlayer()->LearnSpell(skillLine->spellId, false);
                }
            }
        }

        handler->SendSysMessage(LANG_COMMAND_LEARN_ALL_CRAFT);
        return true;
    }

    static bool HandleUnLearnCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
        uint32 min_id =  handler->extractSpellIdFromLink((char*)args);
        if(!min_id)
            return false;

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
        char* tail = strtok(nullptr,"");

        uint32 max_id =  handler->extractSpellIdFromLink(tail);

        if (!max_id)
        {
            // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
            max_id =  min_id+1;
        }
        else
        {
            if (max_id < min_id)
                std::swap(min_id,max_id);

            max_id=max_id+1;
        }

        Player* target = handler->GetSelectedPlayerOrSelf();
        if(!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        for(uint32 spell=min_id;spell<max_id;spell++)
        {
            if (target->HasSpell(spell))
                target->RemoveSpell(spell);
            else
                handler->SendSysMessage(LANG_FORGET_SPELL);
        }

        return true;
    }

};

void AddSC_learn_commandscript()
{
    new learn_commandscript();
}