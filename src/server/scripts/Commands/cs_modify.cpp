#include "Chat.h"
#include "Language.h"
#include "ReplayPlayer.h"
#include "ReputationMgr.h"

class modify_commandscript : public CommandScript
{
public:
    modify_commandscript() : CommandScript("modify_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> modifyspeedCommandTable =
        {
            { "all",            SEC_GAMEMASTER1,      false, &HandleModifyASpeedCommand,        "" },
            { "backwalk",       SEC_GAMEMASTER1,      false, &HandleModifyBWalkCommand,         "" },
            { "fly",            SEC_GAMEMASTER1,      false, &HandleModifyFlyCommand,           "" },
            { "walk",           SEC_GAMEMASTER1,      false, &HandleModifySpeedCommand,         "" },
            { "swim",           SEC_GAMEMASTER1,      false, &HandleModifySwimCommand,          "" },
            { "",               SEC_GAMEMASTER1,      false, &HandleModifyASpeedCommand,        "" },
        };
        static std::vector<ChatCommand> modifyCommandTable =
        {
            { "hp",             SEC_GAMEMASTER1,      false, &HandleModifyHPCommand,            "" },
            { "mana",           SEC_GAMEMASTER1,      false, &HandleModifyManaCommand,          "" },
            { "rage",           SEC_GAMEMASTER1,      false, &HandleModifyRageCommand,          "" },
            { "energy",         SEC_GAMEMASTER1,      false, &HandleModifyEnergyCommand,        "" },
            { "money",          SEC_GAMEMASTER1,      false, &HandleModifyMoneyCommand,         "" },
            { "speed",          SEC_GAMEMASTER1,      false, nullptr,                           "", modifyspeedCommandTable },
            { "scale",          SEC_GAMEMASTER1,      false, &HandleModifyScaleCommand,         "" },
            { "bit",            SEC_GAMEMASTER1,      false, &HandleModifyBitCommand,           "" },
            { "faction",        SEC_GAMEMASTER1,      false, &HandleModifyFactionCommand,       "" },
            { "spell",          SEC_GAMEMASTER2,      false, &HandleModifySpellCommand,         "" },
            { "talentpoints",   SEC_GAMEMASTER1,      false, &HandleModifyTalentCommand,        "" },
            { "titles",         SEC_GAMEMASTER2,      false, &HandleModifyKnownTitlesCommand,   "" },
            { "mount",          SEC_GAMEMASTER1,      false, &HandleModifyMountCommand,         "" },
            { "phase",          SEC_GAMEMASTER1,      false, &HandleModifyPhaseCommand,         "" },
            { "honor",          SEC_GAMEMASTER2,      false, &HandleModifyHonorCommand,         "" },
            { "rep",            SEC_GAMEMASTER2,      false, &HandleModifyRepCommand,           "" },
            { "arena",          SEC_GAMEMASTER2,      false, &HandleModifyArenaCommand,         "" },
            { "drunk",          SEC_GAMEMASTER1,      false, &HandleDrunkCommand,               "" },
            { "gender",         SEC_GAMEMASTER3,      false, &HandleModifyGenderCommand,        "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "morph",          SEC_GAMEMASTER2,      false, &HandleMorphCommand,               "" },
            { "demorph",        SEC_GAMEMASTER2,      false, &HandleDeMorphCommand,             "" },
            { "modify",         SEC_GAMEMASTER1,      false, nullptr,                           "", modifyCommandTable },
        };
        return commandTable;
    }

    template<typename... Args>
    static void NotifyModification(ChatHandler* handler, Unit* target, TrinityStrings resourceMessage, TrinityStrings resourceReportMessage, Args&&... args)
    {
        if (Player* player = target->ToPlayer())
        {
            handler->PSendSysMessage(resourceMessage, handler->GetNameLink(player).c_str(), args...);
            if (handler->needReportToTarget(player))
                ChatHandler(player->GetSession()).PSendSysMessage(resourceReportMessage, handler->GetNameLink().c_str(), std::forward<Args>(args)...);
        }
    }

    //morph creature or player
    static bool HandleMorphCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint16 display_id = 0;

        if (strcmp("random", args) == 0)
        {
            display_id = urand(4, 25958);
            handler->PSendSysMessage("displayid: %u", display_id);
        }
        else
            display_id = (uint16)atoi((char*)args);

        if (!display_id)
            return false;

        Unit *target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        target->SetDisplayId(display_id);

        return true;
    }

    //demorph player or unit
    static bool HandleDeMorphCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit *target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        target->DeMorph();

        return true;
    }

    static bool HandleDrunkCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 drunklevel = (uint32)atoi(args);
        if(drunklevel > 100)
            drunklevel = 100;

        uint16 drunkMod = drunklevel * 0xFFFF / 100;

        //handler->GetSession()->GetPlayer()->SetDrunkValue(drunkMod);
        Unit *pUnit = handler->GetSelectedUnit();
        Player *plr;
        if (pUnit && pUnit->GetTypeId() == TYPEID_PLAYER)
            plr = reinterpret_cast<Player*>(pUnit);
        else
            plr = handler->GetSession()->GetPlayer();
        
        plr->SetDrunkValue(drunkMod);

        return true;
    }

    //Edit Player HP
    static bool HandleModifyHPCommand(ChatHandler* handler, char const* args)
    {
        if(!*args)
            return false;

        //    char* pHp = strtok((char*)args, " ");
        //    if (!pHp)
        //        return false;

        //    char* pHpMax = strtok(NULL, " ");
        //    if (!pHpMax)
        //        return false;

        //    int32 hpm = atoi(pHpMax);
        //    int32 hp = atoi(pHp);

        int32 hp = atoi((char*)args);
        int32 hpm = atoi((char*)args);

        if (hp <= 0 || hpm <= 0 || hpm < hp)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // maxing values since being at the max of an int32 can cause problem when healing
        if(hp > 500000000)
            hp = 500000000;
        if(hpm > 500000000)
            hpm = 500000000;            

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_YOU_CHANGE_HP, chr->GetName().c_str(), hp, hpm);
        if (handler->needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_HP_CHANGED, handler->GetName().c_str(), hp, hpm);

        chr->SetMaxHealth( hpm );
        chr->SetHealth( hp );

        return true;
    }

    //Edit Player Mana
    static bool HandleModifyManaCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // char* pmana = strtok((char*)args, " ");
        // if (!pmana)
        //     return false;

        // char* pmanaMax = strtok(NULL, " ");
        // if (!pmanaMax)
        //     return false;

        // int32 manam = atoi(pmanaMax);
        // int32 mana = atoi(pmana);
        int32 mana = atoi((char*)args);
        int32 manam = atoi((char*)args);

        if (mana <= 0 || manam <= 0 || manam < mana)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Unit* unit = handler->GetSelectedUnit();
        if (unit == nullptr)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_YOU_CHANGE_MANA, unit->GetName().c_str(), mana, manam);
        if(Player* p = unit->ToPlayer())
            if (handler->needReportToTarget(p))
                ChatHandler(p).PSendSysMessage(LANG_YOURS_MANA_CHANGED, handler->GetName().c_str(), mana, manam);

        unit->SetMaxPower(POWER_MANA,manam );
        unit->SetPower(POWER_MANA, mana );

        return true;
    }

    //Edit Player Energy
    static bool HandleModifyEnergyCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // char* pmana = strtok((char*)args, " ");
        // if (!pmana)
        //     return false;

        // char* pmanaMax = strtok(NULL, " ");
        // if (!pmanaMax)
        //     return false;

        // int32 manam = atoi(pmanaMax);
        // int32 mana = atoi(pmana);

        int32 energy = atoi((char*)args)*10;
        int32 energym = atoi((char*)args)*10;

        if (energy <= 0 || energym <= 0 || energym < energy)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (!chr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_YOU_CHANGE_ENERGY, chr->GetName().c_str(), energy/10, energym/10);
        if (handler->needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_ENERGY_CHANGED, handler->GetName().c_str(), energy/10, energym/10);

        chr->SetMaxPower(POWER_ENERGY,energym );
        chr->SetPower(POWER_ENERGY, energy );

        TC_LOG_DEBUG("command",handler->GetTrinityString(LANG_CURRENT_ENERGY),chr->GetMaxPower(POWER_ENERGY));

        return true;
    }

    //Edit Player Rage
    static bool HandleModifyRageCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // char* pmana = strtok((char*)args, " ");
        // if (!pmana)
        //     return false;

        // char* pmanaMax = strtok(NULL, " ");
        // if (!pmanaMax)
        //     return false;

        // int32 manam = atoi(pmanaMax);
        // int32 mana = atoi(pmana);

        int32 rage = atoi((char*)args)*10;
        int32 ragem = atoi((char*)args)*10;

        if (rage <= 0 || ragem <= 0 || ragem < rage)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_YOU_CHANGE_RAGE, chr->GetName().c_str(), rage/10, ragem/10);
        if (handler->needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_RAGE_CHANGED, handler->GetName().c_str(), rage/10, ragem/10);

        chr->SetMaxPower(POWER_RAGE,ragem );
        chr->SetPower(POWER_RAGE, rage );

        return true;
    }

    /* Edit unit Faction 
    .modify faction #factionid [#UNIT_FIELD_FLAGS] [#UNIT_NPC_FLAGS] [#UNIT_DYNAMIC_FLAGS]
    */
    static bool HandleModifyFactionCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Unit* u = handler->GetSelectedUnit();
        if(!u)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* pfactionid = handler->extractKeyFromLink((char*)args,"Hfaction");

        if(!pfactionid) // just show info
        {
            uint32 factionid = u->GetFaction();
            uint32 flag      = u->GetUInt32Value(UNIT_FIELD_FLAGS);
            uint32 npcflag   = u->GetUInt32Value(UNIT_NPC_FLAGS);
            uint32 dyflag    = u->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
            handler->PSendSysMessage(LANG_CURRENT_FACTION,u->GetGUID().GetCounter(),factionid,flag,npcflag,dyflag);
            return true;
        }

        uint32 factionid = atoi(pfactionid);
        if(!factionid)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return true;
        }

        if(!sFactionTemplateStore.LookupEntry(factionid))
        {
            handler->PSendSysMessage(LANG_WRONG_FACTION, factionid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        //player case only
        if (handler->GetSelectedPlayer()) 
        {
            u->SetFaction(factionid);
            handler->PSendSysMessage("You changed %s    's faction to %i", u->GetName().c_str(),factionid);
            return true;
        } 

        // else, creature case :
        uint32 flag;
        char *pflag = strtok(nullptr, " ");
        if (!pflag)
            flag = u->GetUInt32Value(UNIT_FIELD_FLAGS);
        else
            flag = atoi(pflag);

        char* pnpcflag = strtok(nullptr, " ");

        uint32 npcflag;
        if(!pnpcflag)
            npcflag   = u->GetUInt32Value(UNIT_NPC_FLAGS);
        else
            npcflag = atoi(pnpcflag);

        char* pdyflag = strtok(nullptr, " ");

        uint32  dyflag;
        if(!pdyflag)
            dyflag   = u->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
        else
            dyflag = atoi(pdyflag);

        handler->PSendSysMessage(LANG_YOU_CHANGE_FACTION, u->GetGUID().GetCounter(),factionid,flag,npcflag,dyflag);

        u->SetFaction(factionid);
        u->SetUInt32Value(UNIT_FIELD_FLAGS,flag);
        u->SetUInt32Value(UNIT_NPC_FLAGS,npcflag);
        u->SetUInt32Value(UNIT_DYNAMIC_FLAGS,dyflag);

        return true;
    }

    //Edit Player Spell
    static bool HandleModifySpellCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
        char* pspellflatid = strtok((char*)args, " ");
        if (!pspellflatid)
            return false;

        char* pop = strtok(nullptr, " ");
        if (!pop)
            return false;

        char* pval = strtok(nullptr, " ");
        if (!pval)
            return false;

        uint16 mark;

        char* pmark = strtok(nullptr, " ");

        uint8 spellflatid = atoi(pspellflatid);
        uint8 op   = atoi(pop);
        uint16 val = atoi(pval);
        if(!pmark)
            mark = 65535;
        else
            mark = atoi(pmark);

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_YOU_CHANGE_SPELLFLATID, spellflatid, val, mark, chr->GetName().c_str());
        if (handler->needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_SPELLFLATID_CHANGED, handler->GetName().c_str(), spellflatid, val, mark);

        WorldPacket data(SMSG_SET_FLAT_SPELL_MODIFIER, (1+1+2+2));
        data << uint8(spellflatid);
        data << uint8(op);
        data << uint16(val);
        data << uint16(mark);
        chr->SendDirectMessage(&data);

        return true;
    }

    //Edit Player TP
    static bool HandleModifyTalentCommand (ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        int tp = atoi((char*)args);
        if (tp>0)
        {
            Player* player = handler->GetSelectedPlayerOrSelf();
            if(!player)
            {
                handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
                handler->SetSentErrorMessage(true);
                return false;
            }
            player->SetFreeTalentPoints(tp);
            return true;
        }
        return false;
    }

    //Edit Player Aspeed
    static bool HandleModifyASpeedCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        float ASpeed = (float)atof((char*)args);

        if (ASpeed > 30.f || ASpeed < 0.1f)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Unit* u = handler->GetSelectedUnit();
        if (u == nullptr)
        {
            handler->SendSysMessage(LANG_NO_SELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* chr = u->ToPlayer();
        if(chr && chr->IsInFlight())
        {
            handler->PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_YOU_CHANGE_ASPEED, ASpeed, u->GetName().c_str());
        if (chr && handler->needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_ASPEED_CHANGED, handler->GetName().c_str(), ASpeed);

        if (chr)
        {
            chr->SetSpeedRate(MOVE_WALK, ASpeed);
            chr->SetSpeedRate(MOVE_RUN, ASpeed);
            chr->SetSpeedRate(MOVE_SWIM, ASpeed);
            chr->SetSpeedRate(MOVE_FLIGHT, ASpeed);
        }
        else { //target is a creature
            if (Creature* c = u->ToCreature())
            {
                c->SetSpeedRate(MOVE_WALK, ASpeed, false);
                c->SetSpeedRate(MOVE_RUN, ASpeed, false);
                c->SetSpeedRate(MOVE_SWIM, ASpeed, false);
                c->SetSpeedRate(MOVE_FLIGHT, ASpeed, false);
            }
        }
        return true;
    }

    static bool CheckModifySpeed(ChatHandler* handler, char const* args, Unit* target, float& speed, float minimumBound, float maximumBound, bool checkInFlight = true)
    {
        if (!*args)
            return false;

        speed = (float)atof((char*)args);

        if (speed > maximumBound || speed < minimumBound)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (Player* player = target->ToPlayer())
        {
            // check online security
            if (handler->HasLowerSecurity(player, ObjectGuid::Empty))
                return false;

            if (player->IsInFlight() && checkInFlight)
            {
                handler->PSendSysMessage(LANG_CHAR_IN_FLIGHT, handler->GetNameLink(player).c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        return true;
    }

    //Edit Player Speed
    static bool HandleModifySpeedCommand(ChatHandler* handler, char const* args)
    {
        float Speed;
        Player* target = handler->GetSelectedPlayerOrSelf();
        if (CheckModifySpeed(handler, args, target, Speed, 0.1f, 50.0f))
        {
            if (auto replayPlayer = handler->GetSession()->GetReplayPlayer())
            {
                WorldPacket dataForMe(SMSG_FORCE_RUN_SPEED_CHANGE, 18);
                dataForMe << ObjectGuid(HighGuid::Player, replayPlayer->GetRecorderGuid()).WriteAsPacked();
                dataForMe << uint32(0);
                dataForMe << float(baseMoveSpeed[MOVE_RUN] * Speed);
                handler->GetSession()->SendPacket(&dataForMe);
                return true;
            }

            NotifyModification(handler, target, LANG_YOU_CHANGE_SPEED, LANG_YOURS_SPEED_CHANGED, Speed);
            target->SetSpeedRate(MOVE_RUN, Speed);
            return true;
        }
        return false;
    }

    //Edit Player Swim Speed
    static bool HandleModifySwimCommand(ChatHandler* handler, char const* args)
    {
        float swimSpeed;
        Player* target = handler->GetSelectedPlayerOrSelf();
        if (CheckModifySpeed(handler, args, target, swimSpeed, 0.1f, 50.0f))
        {
            if (auto replayPlayer = handler->GetSession()->GetReplayPlayer())
            {
                WorldPacket dataForMe(SMSG_FORCE_SWIM_SPEED_CHANGE, 18);
                dataForMe << ObjectGuid(HighGuid::Player, replayPlayer->GetRecorderGuid()).WriteAsPacked();
                dataForMe << uint32(0);
                dataForMe << float(baseMoveSpeed[MOVE_SWIM] * swimSpeed);
                handler->GetSession()->SendPacket(&dataForMe);
                return true;
            }

            NotifyModification(handler, target, LANG_YOU_CHANGE_SWIM_SPEED, LANG_YOURS_SWIM_SPEED_CHANGED, swimSpeed);
            target->SetSpeedRate(MOVE_SWIM, swimSpeed);
            return true;
        }
        return false;
    }

    //Edit Player Walk Speed
    static bool HandleModifyBWalkCommand(ChatHandler* handler, char const* args)
    {
        float backSpeed;
        Player* target = handler->GetSelectedPlayerOrSelf();
        if (CheckModifySpeed(handler, args, target, backSpeed, 0.1f, 50.0f))
        {
            NotifyModification(handler, target, LANG_YOU_CHANGE_BACK_SPEED, LANG_YOURS_BACK_SPEED_CHANGED, backSpeed);
            target->SetSpeedRate(MOVE_RUN_BACK, backSpeed);
            return true;
        }
        return false;
    }

    //Edit Player Fly speed
    static bool HandleModifyFlyCommand(ChatHandler* handler, char const* args)
    {
        float flySpeed;
        Player* target = handler->GetSelectedPlayerOrSelf();
        if (CheckModifySpeed(handler, args, target, flySpeed, 0.1f, 50.0f, false))
        {
            NotifyModification(handler, target, LANG_YOU_CHANGE_FLY_SPEED, LANG_YOURS_FLY_SPEED_CHANGED, flySpeed);
            target->SetSpeedRate(MOVE_FLIGHT, flySpeed);
            return true;
        }
        return false;
    }

    //Edit Player Scale
    static bool HandleModifyScaleCommand(ChatHandler* handler, char const* args)
    {
        float Scale;
        Unit* target = handler->GetSelectedUnit();
        if (CheckModifySpeed(handler, args, target, Scale, 0.1f, 10.0f, false))
        {
            NotifyModification(handler, target, LANG_YOU_CHANGE_SIZE, LANG_YOURS_SIZE_CHANGED, Scale);
            target->SetObjectScale(Scale);
            return true;
        }
        return false;
    }

    static bool HandleModifyPhaseCommand(ChatHandler* handler, char const* args)
    {
        uint32 phasemask = (uint32)atoi((char*)args);
        if (phasemask < PHASEMASK_NORMAL)
            return false;

        Unit* target = handler->GetSelectedUnit();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        // check online security
        else if (target->GetTypeId() == TYPEID_PLAYER && handler->HasLowerSecurity(target->ToPlayer(), ObjectGuid::Empty))
            return false;

        target->SetPhaseMask(phasemask, true);
        return true;
    }

    //Enable Player mount
    static bool HandleModifyMountCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char const* mount_cstr = strtok(const_cast<char*>(args), " ");
        char const* speed_cstr = strtok(nullptr, " ");

        if (!mount_cstr || !speed_cstr)
            return false;

        uint32 mount = atoul(mount_cstr);
        if (!sCreatureDisplayInfoStore.LookupEntry(mount))
        {
            handler->SendSysMessage(LANG_NO_MOUNT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* target = handler->GetSelectedPlayerOrSelf();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check online security
        if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
            return false;

        float speed;
        if (!CheckModifySpeed(handler, speed_cstr, target, speed, 0.1f, 50.0f))
            return false;

        NotifyModification(handler, target, LANG_YOU_GIVE_MOUNT, LANG_MOUNT_GIVED);
        target->Mount(mount);
        target->SetSpeedRate(MOVE_RUN, speed);
        target->SetSpeedRate(MOVE_FLIGHT, speed);
        return true;
    }

    //Edit Player money
    static bool HandleModifyMoneyCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 moneyToAdd = 0;
            if (strchr(args, 'g') || strchr(args, 's') || strchr(args, 'c'))
                moneyToAdd = MoneyStringToMoney(std::string(args));
            else
                moneyToAdd = atoi(args);

        uint32 targetMoney = chr->GetMoney();

        if(moneyToAdd < 0)
        {
            int32 newmoney = targetMoney + moneyToAdd;

            TC_LOG_DEBUG("command",handler->GetTrinityString(LANG_CURRENT_MONEY), targetMoney, moneyToAdd, newmoney);
            if(newmoney <= 0 )
            {
                handler->PSendSysMessage(LANG_YOU_TAKE_ALL_MONEY, chr->GetName().c_str());
                if (handler->needReportToTarget(chr))
                    ChatHandler(chr).PSendSysMessage(LANG_YOURS_ALL_MONEY_GONE, handler->GetName().c_str());

                chr->SetMoney(0);
            }
            else
            {
                handler->PSendSysMessage(LANG_YOU_TAKE_MONEY, abs(moneyToAdd), chr->GetName().c_str());
                if (handler->needReportToTarget(chr))
                    ChatHandler(chr).PSendSysMessage(LANG_YOURS_MONEY_TAKEN, handler->GetName().c_str(), abs(moneyToAdd));
                chr->SetMoney( newmoney );
            }
        }
        else
        {
            handler->PSendSysMessage(LANG_YOU_GIVE_MONEY, moneyToAdd, chr->GetName().c_str());
            if (handler->needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_MONEY_GIVEN, handler->GetName().c_str(), moneyToAdd);
            chr->ModifyMoney( moneyToAdd );
        }

        TC_LOG_DEBUG("command",handler->GetTrinityString(LANG_NEW_MONEY), targetMoney, moneyToAdd, chr->GetMoney() );

        return true;
    }

    //Edit Player field
    static bool HandleModifyBitCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* pField = strtok((char*)args, " ");
        if (!pField)
            return false;

        char* pBit = strtok(nullptr, " ");
        if (!pBit)
            return false;

        uint16 field = atoi(pField);
        uint32 bit   = atoi(pBit);

        if (field < 1 || field >= PLAYER_END)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (bit < 1 || bit > 32)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if ( chr->HasFlag( field, (1<<(bit-1)) ) )
        {
            chr->RemoveFlag( field, (1<<(bit-1)) );
            handler->PSendSysMessage(LANG_REMOVE_BIT, bit, field);
        }
        else
        {
            chr->SetFlag( field, (1<<(bit-1)) );
            handler->PSendSysMessage(LANG_SET_BIT, bit, field);
        }

        return true;
    }

    static bool HandleModifyHonorCommand (ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player *target = handler->GetSelectedPlayerOrSelf();
        if(!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 amount = (uint32)atoi(args);

        target->ModifyHonorPoints(amount);

        handler->PSendSysMessage(LANG_COMMAND_MODIFY_HONOR, target->GetName().c_str(), target->GetHonorPoints());

        return true;
    }

    //Edit Player KnownTitles
    static bool HandleModifyKnownTitlesCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint64 titles = 0;

        sscanf((char*)args, UI64FMTD, &titles);

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (!chr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint64 titles2 = titles;

        for(int i=1; i < sCharTitlesStore.GetNumRows(); ++i)
            if(CharTitlesEntry const* tEntry = sCharTitlesStore.LookupEntry(i))
                titles2 &= ~(uint64(1) << tEntry->bit_index);

        titles &= ~titles2;                                     // remove not existed titles

        chr->SetUInt64Value(PLAYER_FIELD_KNOWN_TITLES, titles);
        handler->SendSysMessage(LANG_DONE);

        return true;
    }

    static bool HandleModifyRepCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player* target = handler->GetSelectedPlayerOrSelf();
        if(!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* factionTxt = handler->extractKeyFromLink((char*)args,"Hfaction");
        if(!factionTxt)
            return false;

        uint32 factionId = atoi(factionTxt);

        int32 amount = 0;
        char *rankTxt = strtok(nullptr, " ");
        if (!factionTxt || !rankTxt)
            return false;

        amount = atoi(rankTxt);
        if ((amount == 0) && (rankTxt[0] != '-') && !isdigit(rankTxt[0]))
        {
            std::string rankStr = rankTxt;
            std::wstring wrankStr;
            if(!Utf8toWStr(rankStr,wrankStr))
                return false;
            wstrToLower( wrankStr );

            int r = 0;
            amount = -42000;
            for (; r < MAX_REPUTATION_RANK; ++r)
            {
                std::string rank = handler->GetTrinityString(ReputationRankStrIndex[r]);
                if(rank.empty())
                    continue;

                std::wstring wrank;
                if(!Utf8toWStr(rank,wrank))
                    continue;

                wstrToLower(wrank);

                if(wrank.substr(0,wrankStr.size())==wrankStr)
                {
                    char *deltaTxt = strtok(nullptr, " ");
                    if (deltaTxt)
                    {
                        int32 delta = atoi(deltaTxt);
                        if ((delta < 0) || (delta > ReputationMgr::PointsInRank[r] -1))
                        {
                            handler->PSendSysMessage(LANG_COMMAND_FACTION_DELTA, (ReputationMgr::PointsInRank[r]-1));
                            handler->SetSentErrorMessage(true);
                            return false;
                        }
                        amount += delta;
                    }
                    break;
                }
                amount += ReputationMgr::PointsInRank[r];
            }
            if (r >= MAX_REPUTATION_RANK)
            {
                handler->PSendSysMessage(LANG_COMMAND_FACTION_INVPARAM, rankTxt);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionId);

        if (!factionEntry)
        {
            handler->PSendSysMessage(LANG_COMMAND_FACTION_UNKNOWN, factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (factionEntry->reputationListID < 0)
        {
            handler->PSendSysMessage(LANG_COMMAND_FACTION_NOREP_ERROR, factionEntry->name[handler->GetSessionDbcLocale()], factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->GetReputationMgr().SetOneFactionReputation(factionEntry, amount, false);
        target->GetReputationMgr().SendState(target->GetReputationMgr().GetState(factionEntry));
        handler->PSendSysMessage(LANG_COMMAND_MODIFY_REP, factionEntry->name[handler->GetSessionDbcLocale()], factionId, target->GetName().c_str(), target->GetReputation(factionId));
        return true;
    }

    static bool HandleModifyArenaCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player *target = handler->GetSelectedPlayerOrSelf();
        if(!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 amount = (uint32)atoi(args);

        target->ModifyArenaPoints(amount);

        handler->PSendSysMessage(LANG_COMMAND_MODIFY_ARENA, target->GetName().c_str(), target->GetArenaPoints());

        return true;
    }

    static bool HandleModifyGenderCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player *player = handler->GetSelectedPlayerOrSelf();

        if(!player)
        {
            handler->PSendSysMessage(LANG_NO_PLAYER);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char const* gender_str = (char*)args;
        int gender_len = strlen(gender_str);

        uint32 displayId = player->GetNativeDisplayId();
        char const* gender_full = nullptr;
        uint32 new_displayId = displayId;
        Gender gender;

        if(!strncmp(gender_str,"male",gender_len))              // MALE
        {
            if(player->GetGender() == GENDER_MALE)
                return true;

            gender_full = "male";
            new_displayId = player->GetRace() == RACE_BLOODELF ? displayId+1 : displayId-1;
            gender = GENDER_MALE;
        }
        else if (!strncmp(gender_str,"female",gender_len))      // FEMALE
        {
            if(player->GetGender() == GENDER_FEMALE)
                return true;

            gender_full = "female";
            new_displayId = player->GetRace() == RACE_BLOODELF ? displayId-1 : displayId+1;
            gender = GENDER_FEMALE;
        }
        else
        {
            handler->SendSysMessage(LANG_MUST_MALE_OR_FEMALE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Set gender
        player->SetByteValue(UNIT_FIELD_BYTES_0, 2, gender);
        player->SetByteValue(PLAYER_BYTES_3, 0, gender);

        // Change display ID
        player->SetDisplayId(new_displayId);
        player->SetNativeDisplayId(new_displayId);

        handler->PSendSysMessage(LANG_YOU_CHANGE_GENDER, player->GetName().c_str(),gender_full);
        if (handler->needReportToTarget(player))
            ChatHandler(player).PSendSysMessage(LANG_YOUR_GENDER_CHANGED, gender_full, handler->GetName().c_str());
        return true;
    }
};

void AddSC_modify_commandscript()
{
    new modify_commandscript();
}
