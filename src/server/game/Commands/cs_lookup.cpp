#include "Chat.h"
#include "Language.h"
#include "GameEventMgr.h"
#include "AccountMgr.h"

uint32 ChatHandler::ReputationRankStrIndex[MAX_REPUTATION_RANK] =
{
    LANG_REP_HATED,    LANG_REP_HOSTILE, LANG_REP_UNFRIENDLY, LANG_REP_NEUTRAL,
    LANG_REP_FRIENDLY, LANG_REP_HONORED, LANG_REP_REVERED,    LANG_REP_EXALTED
};

bool ChatHandler::HandleLookupAreaCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // converting string that we try to find to lower case
    wstrToLower (wnamepart);
    
    // Search in AreaTable.dbc
    for (uint32 areaID = 0; areaID < sAreaTableStore.GetNumRows (); ++areaID)
    {
        AreaTableEntry const *areaEntry = sAreaTableStore.LookupEntry (areaID);
        if (areaEntry)
        {
            int loc = GetSessionDbcLocale();
            std::string name = areaEntry->area_name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo (name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    if (loc==GetSessionDbcLocale ())
                        continue;

                    name = areaEntry->area_name[loc];
                    if (name.empty ())
                        continue;

                    if (Utf8FitTo (name, wnamepart))
                        break;
                }
            }

            if (loc < TOTAL_LOCALES)
            {
                // send area in "id - [name]" format
                std::ostringstream ss;
                if (m_session)
                    ss << areaEntry->ID << " - |cffffffff|Harea:" << areaEntry->ID << "|h[" << name << " " << localeNames[loc]<< "]|h|r";
                else
                    ss << areaEntry->ID << " - " << name << " " << localeNames[loc];

                SendSysMessage (ss.str ().c_str());

                ++counter;
            }
        }
    }
    if (counter == 0)                                      // if counter == 0 then we found nth
        SendSysMessage (LANG_COMMAND_NOAREAFOUND);
    return true;
}

//Find tele in game_tele order by name
bool ChatHandler::HandleLookupTeleCommand(const char * args)
{
    if(!*args)
    {
        SendSysMessage(LANG_COMMAND_TELE_PARAMETER);
        SetSentErrorMessage(true);
        return false;
    }

    char const* str = strtok((char*)args, " ");
    if(!str)
        return false;

    std::string namepart = str;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    std::ostringstream reply;

    GameTeleContainer const & teleMap = sObjectMgr->GetGameTeleMap();
    for(const auto & itr : teleMap)
    {
        GameTele const* tele = &itr.second;

        if(tele->wnameLow.find(wnamepart) == std::wstring::npos)
            continue;

        if (m_session)
            reply << "  |cffffffff|Htele:" << itr.first << "|h[" << tele->name << "]|h|r\n";
        else
            reply << "  " << itr.first << " " << tele->name << "\n";
    }

    if(reply.str().empty())
        SendSysMessage(LANG_COMMAND_TELE_NOLOCATION);
    else
        PSendSysMessage(LANG_COMMAND_TELE_LOCATION,reply.str().c_str());

    return true;
}


bool ChatHandler::HandleLookupCreatureCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    wstrToLower (wnamepart);

    uint32 counter = 0;

    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
    for (const auto & itr : *ctc)
    {
        uint32 id = itr.second.Entry;
        CreatureTemplate const* cInfo = &(itr.second);
        if(!cInfo)
            continue;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if (loc_idx >= 0)
        {
            CreatureLocale const *cl = sObjectMgr->GetCreatureLocale (id);
            if (cl)
            {
                if (cl->Name.size() > loc_idx && !cl->Name[loc_idx].empty ())
                {
                    std::string name = cl->Name[loc_idx];

                    if (Utf8FitTo (name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str ());
                        else
                            PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str ());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = cInfo->Name;
        if (name.empty ())
            continue;

        if (Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name.c_str ());
            else
                PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name.c_str ());
            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage (LANG_COMMAND_NOCREATUREFOUND);

    return true;
}

bool ChatHandler::HandleLookupObjectCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;
    
    
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplateStore();
    for (const auto & itr : *gotc)
    {
        uint32 id = itr.first;
        GameObjectTemplate const* gInfo = &(itr.second);
        if(!gInfo)
            continue;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if ( loc_idx >= 0 )
        {
            GameObjectLocale const *gl = sObjectMgr->GetGameObjectLocale(id);
            if (gl)
            {
                if (gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                {
                    std::string name = gl->Name[loc_idx];

                    if (Utf8FitTo(name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, id, id, name.c_str());
                        else
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, id, name.c_str());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = gInfo->name;
        if(name.empty())
            continue;

        if(Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, id, id, name.c_str());
            else
                PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, id, name.c_str());
            ++counter;
        }
    }

    if(counter==0)
        SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);

    return true;
}

bool ChatHandler::HandleLookupFactionCommand(const char* args)
{
    ARGS_CHECK

    // Can be NULL at console call
    Player *target = GetSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower (wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    for (uint32 id = 0; id < sFactionStore.GetNumRows(); ++id)
    {
        FactionEntry const *factionEntry = sFactionStore.LookupEntry (id);
        if (factionEntry)
        {
            FactionState const* repState = nullptr;
            if(target)
            {
                FactionStateList::const_iterator repItr = target->m_factions.find (factionEntry->reputationListID);
                if(repItr != target->m_factions.end())
                    repState = &repItr->second;
            }

            int loc = GetSessionDbcLocale();
            std::string name = factionEntry->name[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    if(GetSessionDbcLocale())
                        continue;

                    name = factionEntry->name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < TOTAL_LOCALES)
            {
                // send faction in "id - [faction] rank reputation [visible] [at war] [own team] [unknown] [invisible] [inactive]" format
                // or              "id - [faction] [no reputation]" format
                std::ostringstream ss;
                if (m_session)
                    ss << id << " - |cffffffff|Hfaction:" << id << "|h[" << name << " " << localeNames[loc] << "]|h|r";
                else
                    ss << id << " - " << name << " " << localeNames[loc];

                if (repState)                               // and then target!=NULL also
                {
                    ReputationRank rank = target->GetReputationRank(factionEntry);
                    std::string rankName = GetTrinityString(ReputationRankStrIndex[rank]);

                    ss << " " << rankName << "|h|r (" << target->GetReputation(factionEntry) << ")";

                    if(repState->Flags & FACTION_FLAG_VISIBLE)
                        ss << GetTrinityString(LANG_FACTION_VISIBLE);
                    if(repState->Flags & FACTION_FLAG_AT_WAR)
                        ss << GetTrinityString(LANG_FACTION_ATWAR);
                    if(repState->Flags & FACTION_FLAG_PEACE_FORCED)
                        ss << GetTrinityString(LANG_FACTION_PEACE_FORCED);
                    if(repState->Flags & FACTION_FLAG_HIDDEN)
                        ss << GetTrinityString(LANG_FACTION_HIDDEN);
                    if(repState->Flags & FACTION_FLAG_INVISIBLE_FORCED)
                        ss << GetTrinityString(LANG_FACTION_INVISIBLE_FORCED);
                    if(repState->Flags & FACTION_FLAG_INACTIVE)
                        ss << GetTrinityString(LANG_FACTION_INACTIVE);
                }
                else
                    ss << GetTrinityString(LANG_FACTION_NOREPUTATION);

                SendSysMessage(ss.str().c_str());
                counter++;
            }
        }
    }

    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_FACTION_NOTFOUND);
    return true;
}

bool ChatHandler::HandleLookupItemCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    // Search in `item_template`
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (const auto & it : *its)
    {
        uint32 id = it.first;
        ItemTemplate const *pProto = &(it.second);
        if(!pProto)
            continue;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if ( loc_idx >= 0 )
        {
            ItemLocale const *il = sObjectMgr->GetItemLocale(pProto->ItemId);
            if (il)
            {
                if (il->Name.size() > loc_idx && !il->Name[loc_idx].empty())
                {
                    std::string name = il->Name[loc_idx];

                    if (Utf8FitTo(name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage(LANG_ITEM_LIST_CHAT, id, id, name.c_str());
                        else
                            PSendSysMessage(LANG_ITEM_LIST_CONSOLE, id, name.c_str());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = pProto->Name1;
        if(name.empty())
            continue;

        if (Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage(LANG_ITEM_LIST_CHAT, id, id, name.c_str());
            else
                PSendSysMessage(LANG_ITEM_LIST_CONSOLE, id, name.c_str());
            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);

    return true;
}

bool ChatHandler::HandleLookupItemSetCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in ItemSet.dbc
    for (uint32 id = 0; id < sItemSetStore.GetNumRows(); id++)
    {
        ItemSetEntry const *set = sItemSetStore.LookupEntry(id);
        if(set)
        {
            int loc = GetSessionDbcLocale();
            std::string name = set->name[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    if(GetSessionDbcLocale())
                        continue;

                    name = set->name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < TOTAL_LOCALES)
            {
                // send item set in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_ITEMSET_LIST_CHAT,id,id,name.c_str(),localeNames[loc]);
                else
                    PSendSysMessage(LANG_ITEMSET_LIST_CONSOLE,id,name.c_str(),localeNames[loc]);
                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOITEMSETFOUND);
    return true;
}

bool ChatHandler::HandleLookupSkillCommand(const char* args)
{
    ARGS_CHECK

    // can be NULL in console call
    Player* target = GetSelectedPlayerOrSelf();

    std::string namepart = args;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in SkillLine.dbc
    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); id++)
    {
        SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(id);
        if(skillInfo)
        {
            int loc = GetSessionDbcLocale();
            std::string name = skillInfo->name[loc];
            if(name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    if(GetSessionDbcLocale())
                        continue;

                    name = skillInfo->name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < TOTAL_LOCALES)
            {
                char const* knownStr = "";
                if(target && target->HasSkill(id))
                    knownStr = GetTrinityString(LANG_KNOWN);

                // send skill in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_SKILL_LIST_CHAT,id,id,name.c_str(),localeNames[loc],knownStr);
                else
                    PSendSysMessage(LANG_SKILL_LIST_CONSOLE,id,name.c_str(),localeNames[loc],knownStr);

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSKILLFOUND);
    return true;
}

bool ChatHandler::HandleLookupQuestCommand(const char* args)
{
    ARGS_CHECK

    // can be NULL at console call
    Player* target = GetSelectedPlayerOrSelf();

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0 ;

    ObjectMgr::QuestMap const& qTemplates = sObjectMgr->GetQuestTemplates();
    for (const auto & qTemplate : qTemplates)
    {
        Quest * qinfo = qTemplate.second;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if ( loc_idx >= 0 )
        {
            QuestLocale const *il = sObjectMgr->GetQuestLocale(qinfo->GetQuestId());
            if (il)
            {
                if (il->Title.size() > loc_idx && !il->Title[loc_idx].empty())
                {
                    std::string title = il->Title[loc_idx];

                    if (Utf8FitTo(title, wnamepart))
                    {
                        char const* statusStr = "";

                        if(target)
                        {
                            QuestStatus status = target->GetQuestStatus(qinfo->GetQuestId());

                            if(status == QUEST_STATUS_COMPLETE)
                            {
                                if(target->GetQuestRewardStatus(qinfo->GetQuestId()))
                                    statusStr = GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                                else
                                    statusStr = GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                            }
                            else if(status == QUEST_STATUS_INCOMPLETE)
                                statusStr = GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
                        }

                        if (m_session)
                            PSendSysMessage(LANG_QUEST_LIST_CHAT,qinfo->GetQuestId(),qinfo->GetQuestId(),title.c_str(),statusStr);
                        else
                            PSendSysMessage(LANG_QUEST_LIST_CONSOLE,qinfo->GetQuestId(),title.c_str(),statusStr);

                        QueryResult result = WorldDatabase.PQuery("SELECT bugged, comment FROM quest_bugs WHERE entry = %u", qinfo->GetQuestId());
                        if (result) 
                        {
                            Field* fields = result->Fetch();
                            if (fields[0].GetUInt8())
                                //PSendSysMessage(" -> L'autovalidation est activée pour cette quête.");
                                PSendSysMessage(" -> Auto validation is activated for this quest.");

                            std::string x = "";
                            if(fields[1].GetString() != x)
                                PSendSysMessage(" -> BUG: %s", fields[1].GetString().c_str());
                        }
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string title = qinfo->GetTitle();
        if(title.empty())
            continue;

        if (Utf8FitTo(title, wnamepart))
        {
            char const* statusStr = "";

            if(target)
            {
                QuestStatus status = target->GetQuestStatus(qinfo->GetQuestId());

                if(status == QUEST_STATUS_COMPLETE)
                {
                    if(target->GetQuestRewardStatus(qinfo->GetQuestId()))
                        statusStr = GetTrinityString(LANG_COMMAND_QUEST_REWARDED);
                    else
                        statusStr = GetTrinityString(LANG_COMMAND_QUEST_COMPLETE);
                }
                else if(status == QUEST_STATUS_INCOMPLETE)
                    statusStr = GetTrinityString(LANG_COMMAND_QUEST_ACTIVE);
            }

            if (m_session)
                PSendSysMessage(LANG_QUEST_LIST_CHAT,qinfo->GetQuestId(),qinfo->GetQuestId(),title.c_str(),statusStr);
            else
                PSendSysMessage(LANG_QUEST_LIST_CONSOLE,qinfo->GetQuestId(),title.c_str(),statusStr);

            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_COMMAND_NOQUESTFOUND);

    return true;
}

bool ChatHandler::HandleGetSpellInfoCommand(const char* args)
{
    ARGS_CHECK

    // can be NULL at console call
    Player* target = GetSelectedPlayerOrSelf();

    std::string namepart = args;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    for (uint32 id = 0; id < sObjectMgr->GetSpellStore().size(); id++)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if(spellInfo)
        {
            int loc = GetSessionDbcLocale();
            std::string name = spellInfo->SpellName[loc];
            if(name.empty())
                continue;

            //if name is not valid, try every locales until we find one valid
            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < TOTAL_LOCALES; ++loc)
                {
                    name = spellInfo->SpellName[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < TOTAL_LOCALES)
            {
                bool known = target && target->HasSpell(id);
                bool learn = (spellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL);

                uint32 talentCost = GetTalentSpellCost(id);

                bool talent = (talentCost > 0);
                bool passive = spellInfo->IsPassive();
                bool active = target && (target->HasAuraEffect(id,0) || target->HasAuraEffect(id,1) || target->HasAuraEffect(id,2));

                // unit32 used to prevent interpreting uint8 as char at output
                // find rank of learned spell for learning spell, or talent rank
                uint32 rank = talentCost ? talentCost : sSpellMgr->GetSpellRank(learn ? spellInfo->Effects[0].TriggerSpell : id);

                // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
                std::ostringstream ss;
                if (m_session)
                    ss << id << " - |cffffffff|Hspell:" << id << "|h[" << name;
                else
                    ss << id << " - " << name;

                // include rank in link name
                if(rank)
                    ss << GetTrinityString(LANG_SPELL_RANK) << rank;

                if (m_session)
                    ss << " " << localeNames[loc] << "]|h|r";
                else
                    ss << " " << localeNames[loc];

                if(talent)
                    ss << GetTrinityString(LANG_TALENT);
                if(passive)
                    ss << GetTrinityString(LANG_PASSIVE);
                if(learn)
                    ss << GetTrinityString(LANG_LEARN);
                if(known)
                    ss << GetTrinityString(LANG_KNOWN);
                if(active)
                    ss << GetTrinityString(LANG_ACTIVE);

                SendSysMessage(ss.str().c_str());

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSPELLFOUND);
    return true;
}

bool ChatHandler::HandleLookupPlayerIpCommand(const char* args)
{
    ARGS_CHECK

    std::string ip = strtok ((char*)args, " ");
    char* limit_str = strtok (nullptr, " ");
    int32 limit = limit_str ? atoi (limit_str) : -1;

    LoginDatabase.EscapeString(ip);

    QueryResult result = LoginDatabase.PQuery ("SELECT id,username FROM account WHERE last_ip = '%s'", ip.c_str ());

    return LookupPlayerSearchCommand (result,limit);
}

bool ChatHandler::HandleLookupPlayerAccountCommand(const char* args)
{
    ARGS_CHECK

    std::string account = strtok ((char*)args, " ");
    char* limit_str = strtok (nullptr, " ");
    int32 limit = limit_str ? atoi (limit_str) : -1;

    if (!AccountMgr::normalizeString (account))
        return false;

    LoginDatabase.EscapeString(account);

    QueryResult result = LoginDatabase.PQuery ("SELECT id,username FROM account WHERE username = '%s'", account.c_str ());

    return LookupPlayerSearchCommand (result,limit);
}

bool ChatHandler::HandleLookupPlayerEmailCommand(const char* args)
{
    ARGS_CHECK

    std::string email = strtok ((char*)args, " ");
    char* limit_str = strtok (nullptr, " ");
    int32 limit = limit_str ? atoi (limit_str) : -1;

    LoginDatabase.EscapeString(email);

    QueryResult result = LoginDatabase.PQuery ("SELECT id,username FROM account WHERE email = '%s'", email.c_str ());

    return LookupPlayerSearchCommand (result,limit);
}

bool ChatHandler::HandleLookupEventCommand(const char* args)
{
    ARGS_CHECK

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();
    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr->GetActiveEventList();

    for(uint32 id = 0; id < events.size(); ++id )
    {
        GameEventData const& eventData = events[id];

        std::string descr = eventData.description;
        if(descr.empty())
            continue;

        if (Utf8FitTo(descr, wnamepart))
        {
            char const* active = activeEvents.find(id) != activeEvents.end() ? GetTrinityString(LANG_ACTIVE) : "";

            if(m_session)
                PSendSysMessage(LANG_EVENT_ENTRY_LIST_CHAT,id,id,eventData.description.c_str(),active );
            else
                PSendSysMessage(LANG_EVENT_ENTRY_LIST_CONSOLE,id,eventData.description.c_str(),active );

            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_NOEVENTFOUND);

    return true;
}

bool ChatHandler::LookupPlayerSearchCommand(QueryResult result, int32 limit)
{
    if(!result)
    {
        PSendSysMessage(LANG_NO_PLAYERS_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    int i =0;
    do
    {
        Field* fields = result->Fetch();
        uint32 acc_id = fields[0].GetUInt32();
        std::string acc_name = fields[1].GetString();

        QueryResult chars = CharacterDatabase.PQuery("SELECT guid,name FROM characters WHERE account = '%u'", acc_id);
        if(chars)
        {
            PSendSysMessage(LANG_LOOKUP_PLAYER_ACCOUNT,acc_name.c_str(),acc_id);

            ObjectGuid guid = ObjectGuid::Empty;
            std::string name;

            do
            {
                Field* charfields = chars->Fetch();
                guid = ObjectGuid(charfields[0].GetUInt64());
                name = charfields[1].GetString();

                PSendSysMessage(LANG_LOOKUP_PLAYER_CHARACTER,name.c_str(),guid);
                ++i;

            } while( chars->NextRow() && ( limit == -1 || i < limit ) );
        }
    } while(result->NextRow());

    return true;
}
