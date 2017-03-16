#include "Chat.h"
#include "Language.h"

bool ChatHandler::HandleLearnAllGMCommand(const char* /*args*/)
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
        if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
        {
            PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
            continue;
        }

        m_session->GetPlayer()->LearnSpell(spell, false);
    }

    SendSysMessage(LANG_LEARNING_GM_SKILLS);
    return true;
}

bool ChatHandler::HandleLearnAllMyClassCommand(const char* /*args*/)
{
    HandleLearnAllMySpellsCommand("");
    HandleLearnAllMyTalentsCommand("");
    return true;
}

bool ChatHandler::HandleLearnAllMyProeficienciesCommand(const char* /* args */)
{
    m_session->GetPlayer()->LearnAllClassProficiencies();
    return true;
}

bool ChatHandler::HandleLearnAllMySpellsCommand(const char* /*args*/)
{
    m_session->GetPlayer()->LearnAllClassSpells();
    SendSysMessage(LANG_COMMAND_LEARN_CLASS_SPELLS);
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

bool ChatHandler::HandleLearnAllMyTalentsCommand(const char* /*args*/)
{
    Player* player = m_session->GetPlayer();
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
        if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
            continue;

        // learn highest rank of talent
        player->LearnSpell(spellid, false);

        // and learn all non-talent spell ranks (recursive by tree)
        learnAllHighRanks(player,spellid);
    }

    SendSysMessage(LANG_COMMAND_LEARN_CLASS_TALENTS);
    return true;
}

bool ChatHandler::HandleLearnAllLangCommand(const char* /*args*/)
{
    // skipping UNIVERSAL language (0)
    for(int i = 1; i < LANGUAGES_COUNT; ++i)
        m_session->GetPlayer()->LearnSpell(lang_description[i].spell_id, false);

    SendSysMessage(LANG_COMMAND_LEARN_ALL_LANG);
    return true;
}

bool ChatHandler::HandleLearnAllDefaultCommand(const char* args)
{
    char* pName = strtok((char*)args, "");
    Player *player = nullptr;
    if (pName)
    {
        std::string name = pName;

        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
    }
    else
        player = GetSelectedPlayerOrSelf();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    player->LearnDefaultSpells();
    player->learnQuestRewardedSpells();

    PSendSysMessage(LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST,player->GetName().c_str());
    return true;
}

bool ChatHandler::HandleLearnCommand(const char* args)
{
    Player* targetPlayer = GetSelectedPlayerOrSelf();
    if(!targetPlayer)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if(!spell || !sSpellMgr->GetSpellInfo(spell))
        return false;

    if (targetPlayer->HasSpell(spell))
    {
        if(targetPlayer == m_session->GetPlayer())
            SendSysMessage(LANG_YOU_KNOWN_SPELL);
        else
            PSendSysMessage(LANG_TARGET_KNOWN_SPELL,targetPlayer->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    targetPlayer->LearnSpell(spell, false);

    return true;
}

bool ChatHandler::HandleLearnAllRecipesCommand(const char* args)
{
    ARGS_CHECK

    //  Learns all recipes of specified profession and sets skill to max
    //  Example: .learn all_recipes enchanting

    Player* target = GetSelectedPlayerOrSelf();
    if( !target )
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        return false;
    }

    std::wstring wnamepart;

    if(!Utf8toWStr(args,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    uint32 classmask = m_session->GetPlayer()->GetClassMask();

    for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
    {
        SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(i);
        if( !skillInfo )
            continue;

        if( skillInfo->categoryId != SKILL_CATEGORY_PROFESSION &&
            skillInfo->categoryId != SKILL_CATEGORY_SECONDARY )
            continue;

        int loc = GetSessionDbcLocale();
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
                if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
                    continue;

                if( !target->HasSpell(spellInfo->Id) )
                    m_session->GetPlayer()->LearnSpell(skillLine->spellId, false);
            }

            uint16 MaxLevel = target->GetPureMaxSkillValue(skillInfo->id);

            uint16 step = (MaxLevel-1) / 75;
            target->SetSkill(skillInfo->id, step, MaxLevel, MaxLevel);
            PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, name.c_str());
            return true;
        }
    }

    return false;
}

bool ChatHandler::HandleLearnAllCraftsCommand(const char* /*args*/)
{
    uint32 classmask = m_session->GetPlayer()->GetClassMask();

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
                if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
                    continue;

                m_session->GetPlayer()->LearnSpell(skillLine->spellId, false);
            }
        }
    }

    SendSysMessage(LANG_COMMAND_LEARN_ALL_CRAFT);
    return true;
}

bool ChatHandler::HandleUnLearnCommand(const char* args)
{
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    uint32 min_id = extractSpellIdFromLink((char*)args);
    if(!min_id)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    char* tail = strtok(nullptr,"");

    uint32 max_id = extractSpellIdFromLink(tail);

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

    Player* target = GetSelectedPlayerOrSelf();
    if(!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    for(uint32 spell=min_id;spell<max_id;spell++)
    {
        if (target->HasSpell(spell))
            target->RemoveSpell(spell);
        else
            SendSysMessage(LANG_FORGET_SPELL);
    }

    return true;
}
