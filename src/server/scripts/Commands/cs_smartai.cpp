#include "Chat.h"
#include "Language.h"
#include "SmartScriptMgr.h"
#include "SmartScript.h"
#include "SmartAI.h"

/** Syntax: .smartai errors [entryOrGuid] */
static bool HandleSmartAIShowErrorsCommand(ChatHandler* handler, char const* args)
{
    uint32 entry = 0;
    uint32 guid = 0;
    Creature* target = nullptr;

    if (!*args)
    {
        //if no arguments given, try getting selected creature
        target = GetSelectedCreature();

        if (!target)
        {
            handler->SendSysMessage("Select a creature or give an entry or a guid (as a negative value).");
            return true;
        }

        guid = target->GetGUID().GetCounter();
        entry = target->GetEntry();
    } else {
        //arguments given, check if guid or entry
        int entryOrGuid = atoi(args);
        if(entryOrGuid > 0)
            entry = entryOrGuid;
        else
            guid = -entryOrGuid;
    }

    handler->SendSysMessage("SmartAI errors :");
    auto errorList = sSmartScriptMgr->GetErrorList(-int32(guid)); //negative guid in argument
    for(auto itr : errorList)
        handler->PSendSysMessage("%s", itr.c_str());

    errorList = sSmartScriptMgr->GetErrorList(entry);
    for(auto itr : errorList)
        handler->PSendSysMessage("%s", itr.c_str());

    return true;
}

/** Syntax: .smartai debug [entryOrGuid] */
static bool HandleSmartAIDebugCommand(ChatHandler* handler, char const* args)
{
    uint32 entry = 0;
    uint32 guid = 0;
    Creature* target = nullptr;

    if (!*args)
    {
        //if no arguments given, try getting selected creature
        target = GetSelectedCreature();

        if (!target)
        {
            handler->SendSysMessage("Select a creature or give an entry or a guid (as a negative value).");
            return true;
        }

        guid = target->GetGUID().GetCounter();
        entry = target->GetEntry();
    } else {
        //arguments given, check if guid or entry
        int entryOrGuid = atoi(args);
        if(entryOrGuid > 0)
            entry = entryOrGuid;
        else
            guid = -entryOrGuid;
    }

    handler->SendSysMessage("SmartAI infos :");
    if(target)
    {
        if(target->GetAIName() == SMARTAI_AI_NAME)
        {
            if(SmartScript* smartScript = dynamic_cast<SmartAI*>(target->AI())->GetScript())
            {
                uint32 phase = smartScript->GetPhase();
                handler->PSendSysMessage("Current phase: %u", phase);

                uint32 lastProcessedActionId = smartScript->GetLastProcessedActionId();
                handler->PSendSysMessage("Last processed action: %u", lastProcessedActionId);
            }
        } else {
            handler->SendSysMessage("Not SmartAI creature.");
        }
    } else {
        handler->SendSysMessage("No target selected.");
        //TODO: try getting AI with args
    }

    return true;
}