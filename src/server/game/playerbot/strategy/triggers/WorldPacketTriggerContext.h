#pragma once

#include "WorldPacketTrigger.h"
#include "WithinAreaTrigger.h"

namespace ai
{
    class WorldPacketTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        WorldPacketTriggerContext()
        {
            creators["gossip hello"] = &WorldPacketTriggerContext::gossip_hello;
            creators["group invite"] = &WorldPacketTriggerContext::group_invite;
            creators["group set leader"] = &WorldPacketTriggerContext::group_set_leader;
            creators["not enough money"] = &WorldPacketTriggerContext::no_money;
            creators["not enough reputation"] = &WorldPacketTriggerContext::no_reputation;
            creators["cannot equip"] = &WorldPacketTriggerContext::cannot_equip;
            creators["use game object"] = &WorldPacketTriggerContext::use_game_object;
            creators["complete quest"] = &WorldPacketTriggerContext::complete_quest;
            creators["accept quest"] = &WorldPacketTriggerContext::accept_quest;
            creators["quest share"] = &WorldPacketTriggerContext::quest_share;
            creators["loot roll"] = &WorldPacketTriggerContext::loot_roll;
            creators["resurrect request"] = &WorldPacketTriggerContext::resurrect_request;
            creators["area trigger"] = &WorldPacketTriggerContext::area_trigger;
            creators["within area trigger"] = &WorldPacketTriggerContext::within_area_trigger;
            creators["check mount state"] = &WorldPacketTriggerContext::check_mount_state;
            creators["activate taxi"] = &WorldPacketTriggerContext::taxi;
            creators["trade status"] = &WorldPacketTriggerContext::trade_status;
            creators["loot response"] = &WorldPacketTriggerContext::loot_response;
            creators["out of react range"] = &WorldPacketTriggerContext::out_of_react_range;
            creators["quest objective completed"] = &WorldPacketTriggerContext::quest_objective_completed;
            creators["item push result"] = &WorldPacketTriggerContext::item_push_result;
            creators["party command"] = &WorldPacketTriggerContext::party_command;
            creators["taxi done"] = &WorldPacketTriggerContext::taxi_done;
            creators["cast failed"] = &WorldPacketTriggerContext::cast_failed;
            creators["duel requested"] = &WorldPacketTriggerContext::duel_requested;
            creators["ready check"] = &WorldPacketTriggerContext::ready_check;
            creators["ready check finished"] = &WorldPacketTriggerContext::ready_check_finished;
            creators["uninvite"] = &WorldPacketTriggerContext::uninvite;
            creators["lfg join"] = &WorldPacketTriggerContext::lfg_update;
            creators["lfg proposal"] = &WorldPacketTriggerContext::lfg_proposal;
            creators["lfg role check"] = &WorldPacketTriggerContext::lfg_role_check;
            creators["lfg leave"] = &WorldPacketTriggerContext::lfg_leave;
            creators["guild invite"] = &WorldPacketTriggerContext::guild_invite;
            creators["lfg teleport"] = &WorldPacketTriggerContext::lfg_teleport;
        }

    private:
        static std::shared_ptr<Trigger> guild_invite(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "guild invite"); }
        static std::shared_ptr<Trigger> lfg_teleport(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "lfg teleport"); }
        static std::shared_ptr<Trigger> lfg_leave(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "lfg leave"); }
        static std::shared_ptr<Trigger> lfg_proposal(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "lfg proposal"); }
        static std::shared_ptr<Trigger> lfg_role_check(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "lfg role check"); }
        static std::shared_ptr<Trigger> lfg_update(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "lfg join"); }
        static std::shared_ptr<Trigger> uninvite(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "uninvite"); }
        static std::shared_ptr<Trigger> ready_check_finished(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "ready check finished"); }
        static std::shared_ptr<Trigger> ready_check(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "ready check"); }
        static std::shared_ptr<Trigger> duel_requested(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "duel requested"); }
        static std::shared_ptr<Trigger> cast_failed(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "cast failed"); }
        static std::shared_ptr<Trigger> taxi_done(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "taxi done"); }
        static std::shared_ptr<Trigger> party_command(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "party command"); }
        static std::shared_ptr<Trigger> item_push_result(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "item push result"); }
        static std::shared_ptr<Trigger> quest_objective_completed(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "quest objective completed"); }
        static std::shared_ptr<Trigger> out_of_react_range(PlayerbotAI* ai) { return std::make_shared<OutOfReactRangeTrigger>(ai); }
        static std::shared_ptr<Trigger> loot_response(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "loot response"); }
        static std::shared_ptr<Trigger> trade_status(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "trade status"); }
        static std::shared_ptr<Trigger> cannot_equip(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "cannot equip"); }
        static std::shared_ptr<Trigger> check_mount_state(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "check mount state"); }
        static std::shared_ptr<Trigger> area_trigger(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "area trigger"); }
        static std::shared_ptr<Trigger> within_area_trigger(PlayerbotAI* ai) { return std::make_shared<WithinAreaTrigger>(ai); }
        static std::shared_ptr<Trigger> resurrect_request(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "resurrect request"); }
        static std::shared_ptr<Trigger> gossip_hello(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "gossip hello"); }
        static std::shared_ptr<Trigger> group_invite(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "group invite"); }
        static std::shared_ptr<Trigger> group_set_leader(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "group set leader"); }
        static std::shared_ptr<Trigger> no_money(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "not enough money"); }
        static std::shared_ptr<Trigger> no_reputation(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "not enough reputation"); }
        static std::shared_ptr<Trigger> use_game_object(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "use game object"); }
        static std::shared_ptr<Trigger> complete_quest(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "complete quest"); }
        static std::shared_ptr<Trigger> accept_quest(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "accept quest"); }
        static std::shared_ptr<Trigger> quest_share(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "quest share"); }
        static std::shared_ptr<Trigger> loot_roll(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "loot roll"); }
        static std::shared_ptr<Trigger> taxi(PlayerbotAI* ai) { return std::make_shared<WorldPacketTrigger>(ai, "activate taxi"); }
    };
};
