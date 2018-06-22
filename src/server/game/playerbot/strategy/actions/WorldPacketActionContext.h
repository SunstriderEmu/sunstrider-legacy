#pragma once

#include "AcceptInvitationAction.h"
#include "PassLeadershipToMasterAction.h"
#include "TellMasterAction.h"
#include "TalkToQuestGiverAction.h"
#include "AcceptQuestAction.h"
#include "LootRollAction.h"
#include "ReviveFromCorpseAction.h"
#include "AcceptResurrectAction.h"
#include "UseMeetingStoneAction.h"
#include "AreaTriggerAction.h"
#include "CheckMountStateAction.h"
#include "RememberTaxiAction.h"
#include "TradeStatusAction.h"
#include "InventoryChangeFailureAction.h"
#include "LootAction.h"
#include "QuestAction.h"
#include "LeaveGroupAction.h"
#include "TellCastFailedAction.h"
#include "AcceptDuelAction.h"
#include "ReadyCheckAction.h"
#include "LfgActions.h"
#include "SecurityCheckAction.h"
#include "GuildAcceptAction.h"

namespace ai
{
    class WorldPacketActionContext : public NamedObjectContext<Action>
    {
    public:
        WorldPacketActionContext()
        {
            creators["accept invitation"] = &WorldPacketActionContext::accept_invitation;
            creators["leader"] = &WorldPacketActionContext::pass_leadership_to_master;
            creators["tell not enough money"] = &WorldPacketActionContext::tell_not_enough_money;
            creators["tell not enough reputation"] = &WorldPacketActionContext::tell_not_enough_reputation;
            creators["tell cannot equip"] = &WorldPacketActionContext::tell_cannot_equip;
            creators["talk to quest giver"] = &WorldPacketActionContext::turn_in_quest;
            creators["accept quest"] = &WorldPacketActionContext::accept_quest;
            creators["accept all quests"] = &WorldPacketActionContext::accept_all_quests;
            creators["accept quest share"] = &WorldPacketActionContext::accept_quest_share;
            creators["loot roll"] = &WorldPacketActionContext::loot_roll;
            creators["revive from corpse"] = &WorldPacketActionContext::revive_from_corpse;
            creators["accept resurrect"] = &WorldPacketActionContext::accept_resurrect;
            creators["use meeting stone"] = &WorldPacketActionContext::use_meeting_stone;
            creators["area trigger"] = &WorldPacketActionContext::area_trigger;
            creators["reach area trigger"] = &WorldPacketActionContext::reach_area_trigger;
            creators["check mount state"] = &WorldPacketActionContext::check_mount_state;
            creators["remember taxi"] = &WorldPacketActionContext::remember_taxi;
            creators["accept trade"] = &WorldPacketActionContext::accept_trade;
            creators["store loot"] = &WorldPacketActionContext::store_loot;
            creators["tell out of react range"] = &WorldPacketActionContext::tell_out_of_react_range;
            creators["quest objective completed"] = &WorldPacketActionContext::quest_objective_completed;
            creators["party command"] = &WorldPacketActionContext::party_command;
            creators["tell cast failed"] = &WorldPacketActionContext::tell_cast_failed;
            creators["accept duel"] = &WorldPacketActionContext::accept_duel;
            creators["ready check"] = &WorldPacketActionContext::ready_check;
            creators["ready check finished"] = &WorldPacketActionContext::ready_check_finished;
            creators["uninvite"] = &WorldPacketActionContext::uninvite;
            creators["lfg join"] = &WorldPacketActionContext::lfg_join;
            creators["lfg accept"] = &WorldPacketActionContext::lfg_accept;
            creators["lfg role check"] = &WorldPacketActionContext::lfg_role_check;
            creators["lfg leave"] = &WorldPacketActionContext::lfg_leave;
            creators["lfg teleport"] = &WorldPacketActionContext::lfg_teleport;
            creators["security check"] = &WorldPacketActionContext::security_check;
            creators["guild accept"] = &WorldPacketActionContext::guild_accept;
        }

    private:
        static std::shared_ptr<Action> guild_accept(PlayerbotAI* ai) { return std::make_shared<GuildAcceptAction>(ai); }
        static std::shared_ptr<Action> security_check(PlayerbotAI* ai) { return std::make_shared<SecurityCheckAction>(ai); }
        static std::shared_ptr<Action> lfg_teleport(PlayerbotAI* ai) { return std::make_shared<LfgTeleportAction>(ai); }
        static std::shared_ptr<Action> lfg_leave(PlayerbotAI* ai) { return std::make_shared<LfgLeaveAction>(ai); }
        static std::shared_ptr<Action> lfg_accept(PlayerbotAI* ai) { return std::make_shared<LfgAcceptAction>(ai); }
        static std::shared_ptr<Action> lfg_role_check(PlayerbotAI* ai) { return std::make_shared<LfgRoleCheckAction>(ai); }
        static std::shared_ptr<Action> lfg_join(PlayerbotAI* ai) { return std::make_shared<LfgJoinAction>(ai); }
        static std::shared_ptr<Action> uninvite(PlayerbotAI* ai) { return std::make_shared<UninviteAction>(ai); }
        static std::shared_ptr<Action> ready_check_finished(PlayerbotAI* ai) { return std::make_shared<FinishReadyCheckAction>(ai); }
        static std::shared_ptr<Action> ready_check(PlayerbotAI* ai) { return std::make_shared<ReadyCheckAction>(ai); }
        static std::shared_ptr<Action> accept_duel(PlayerbotAI* ai) { return std::make_shared<AcceptDuelAction>(ai); }
        static std::shared_ptr<Action> tell_cast_failed(PlayerbotAI* ai) { return std::make_shared<TellCastFailedAction>(ai); }
        static std::shared_ptr<Action> party_command(PlayerbotAI* ai) { return std::make_shared<PartyCommandAction>(ai); }
        static std::shared_ptr<Action> quest_objective_completed(PlayerbotAI* ai) { return std::make_shared<QuestObjectiveCompletedAction>(ai); }
        static std::shared_ptr<Action> store_loot(PlayerbotAI* ai) { return std::make_shared<StoreLootAction>(ai); }
        static std::shared_ptr<Action> tell_out_of_react_range(PlayerbotAI* ai) { return std::make_shared<OutOfReactRangeAction>(ai); }
        static std::shared_ptr<Action> accept_trade(PlayerbotAI* ai) { return std::make_shared<TradeStatusAction>(ai); }
        static std::shared_ptr<Action> remember_taxi(PlayerbotAI* ai) { return std::make_shared<RememberTaxiAction>(ai); }
        static std::shared_ptr<Action> check_mount_state(PlayerbotAI* ai) { return std::make_shared<CheckMountStateAction>(ai); }
        static std::shared_ptr<Action> area_trigger(PlayerbotAI* ai) { return std::make_shared<AreaTriggerAction>(ai); }
        static std::shared_ptr<Action> reach_area_trigger(PlayerbotAI* ai) { return std::make_shared<ReachAreaTriggerAction>(ai); }
        static std::shared_ptr<Action> use_meeting_stone(PlayerbotAI* ai) { return std::make_shared<UseMeetingStoneAction>(ai); }
        static std::shared_ptr<Action> accept_resurrect(PlayerbotAI* ai) { return std::make_shared<AcceptResurrectAction>(ai); }
        static std::shared_ptr<Action> revive_from_corpse(PlayerbotAI* ai) { return std::make_shared<ReviveFromCorpseAction>(ai); }
        static std::shared_ptr<Action> accept_invitation(PlayerbotAI* ai) { return std::make_shared<AcceptInvitationAction>(ai); }
        static std::shared_ptr<Action> pass_leadership_to_master(PlayerbotAI* ai) { return std::make_shared<PassLeadershipToMasterAction>(ai); }
        static std::shared_ptr<Action> tell_not_enough_money(PlayerbotAI* ai) { return std::make_shared<TellMasterAction>(ai, "Not enough money"); }
        static std::shared_ptr<Action> tell_not_enough_reputation(PlayerbotAI* ai) { return std::make_shared<TellMasterAction>(ai, "Not enough reputation"); }
        static std::shared_ptr<Action> tell_cannot_equip(PlayerbotAI* ai) { return std::make_shared<InventoryChangeFailureAction>(ai); }
        static std::shared_ptr<Action> turn_in_quest(PlayerbotAI* ai) { return std::make_shared<TalkToQuestGiverAction>(ai); }
        static std::shared_ptr<Action> accept_quest(PlayerbotAI* ai) { return std::make_shared<AcceptQuestAction>(ai); }
        static std::shared_ptr<Action> accept_all_quests(PlayerbotAI* ai) { return std::make_shared<AcceptAllQuestsAction>(ai); }
        static std::shared_ptr<Action> accept_quest_share(PlayerbotAI* ai) { return std::make_shared<AcceptQuestShareAction>(ai); }
        static std::shared_ptr<Action> loot_roll(PlayerbotAI* ai) { return std::static_pointer_cast<QueryItemUsageAction>(std::make_shared<LootRollAction>(ai)); }
    };


};
