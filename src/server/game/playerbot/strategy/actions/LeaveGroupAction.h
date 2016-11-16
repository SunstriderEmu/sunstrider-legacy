#pragma once

#include "../Action.h"
#include "../../RandomPlayerbotMgr.h"

namespace ai
{
    class LeaveGroupAction : public Action {
    public:
        LeaveGroupAction(PlayerbotAI* ai, std::string name = "leave") : Action(ai, name) {}

        virtual bool Execute(Event event)
        {
            if (!bot->GetGroup())
                return false;

            ai->TellMaster("Goodbye!", PLAYERBOT_SECURITY_TALK);

            //SMSG_PARTY_COMMAND_RESULT
            WorldPacket p;
            std::string member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(PARTY_RESULT_OK);
            bot->GetSession()->HandleGroupDisbandOpcode(p);

            if (sRandomPlayerbotMgr.IsRandomBot(bot))
            {
                bot->GetPlayerbotAI()->SetMaster(NULL);
                sRandomPlayerbotMgr.ScheduleTeleport(bot->GetGUIDLow());
                sRandomPlayerbotMgr.SetLootAmount(bot, 0);
            }

            ai->ResetStrategies();
            return true;
        }
    };

    class PartyCommandAction : public LeaveGroupAction {
    public:
        PartyCommandAction(PlayerbotAI* ai) : LeaveGroupAction(ai, "party command") {}

        virtual bool Execute(Event event)
        {
            WorldPacket& p = event.getPacket();
            p.rpos(0);
            uint32 operation;
            std::string member;

            p >> operation >> member;

            if (operation != PARTY_OP_LEAVE)
                return false;

            Player* master = GetMaster();
            if (master && member == master->GetName())
                return LeaveGroupAction::Execute(event);

            return false;
        }
    };

    class UninviteAction : public LeaveGroupAction {
    public:
        UninviteAction(PlayerbotAI* ai) : LeaveGroupAction(ai, "party command") {}

        virtual bool Execute(Event event)
        {
            WorldPacket& p = event.getPacket();
            p.rpos(0);
            ObjectGuid guid;

            p >> guid;

            if (bot->GetGUID() == guid)
                return LeaveGroupAction::Execute(event);

            return false;
        }
    };

}
