#pragma once

#include "ListQuestsActions.h"
#include "StatsAction.h"
#include "LeaveGroupAction.h"
#include "TellReputationAction.h"
#include "LogLevelAction.h"
#include "TellLosAction.h"
#include "DropQuestAction.h"
#include "QueryQuestAction.h"
#include "QueryItemUsageAction.h"
#include "LootStrategyAction.h"
#include "AddLootAction.h"
#include "ReleaseSpiritAction.h"
#include "TeleportAction.h"
#include "TaxiAction.h"
#include "RepairAllAction.h"
#include "UseItemAction.h"
#include "TellItemCountAction.h"
#include "RewardAction.h"
#include "BuyAction.h"
#include "SellAction.h"
#include "UnequipAction.h"
#include "EquipAction.h"
#include "TradeAction.h"
#include "ChangeTalentsAction.h"
#include "ListSpellsAction.h"
#include "ChangeStrategyAction.h"
#include "TrainerAction.h"
#include "ChangeChatAction.h"
#include "SetHomeAction.h"
#include "ResetAiAction.h"
#include "DestroyItemAction.h"
#include "BuffAction.h"
#include "AttackAction.h"
#include "HelpAction.h"
#include "GuildBankAction.h"
#include "ChatShortcutActions.h"
#include "GossipHelloAction.h"
#include "CastCustomSpellAction.h"
#include "InviteToGroupAction.h"
#include "TellCastFailedAction.h"
#include "RtiAction.h"
#include "ReviveFromCorpseAction.h"
#include "BankAction.h"
#include "PositionAction.h"
#include "TellTargetAction.h"
#include "UseMeetingStoneAction.h"
#include "WhoAction.h"
#include "SaveManaAction.h"
#include "../values/Formations.h"

namespace ai
{
    class ChatActionContext : public NamedObjectContext<Action>
    {
    public:
        ChatActionContext()
        {
            creators["stats"] = &ChatActionContext::stats;
            creators["quests"] = &ChatActionContext::quests;
            creators["leave"] = &ChatActionContext::leave;
            creators["reputation"] = &ChatActionContext::reputation;
            creators["log"] = &ChatActionContext::log;
            creators["los"] = &ChatActionContext::los;
            creators["drop"] = &ChatActionContext::drop;
            creators["query quest"] = &ChatActionContext::query_quest;
            creators["query item usage"] = &ChatActionContext::query_item_usage;
            creators["ll"] = &ChatActionContext::ll;
            creators["add all loot"] = &ChatActionContext::add_all_loot;
            creators["release"] = &ChatActionContext::release;
            creators["teleport"] = &ChatActionContext::teleport;
            creators["taxi"] = &ChatActionContext::taxi;
            creators["repair"] = &ChatActionContext::repair;
            creators["use"] = &ChatActionContext::use;
            creators["item count"] = &ChatActionContext::item_count;
            creators["equip"] = &ChatActionContext::equip;
            creators["unequip"] = &ChatActionContext::unequip;
            creators["sell"] = &ChatActionContext::sell;
            creators["buy"] = &ChatActionContext::buy;
            creators["reward"] = &ChatActionContext::reward;
            creators["trade"] = &ChatActionContext::trade;
            creators["talents"] = &ChatActionContext::talents;
            creators["spells"] = &ChatActionContext::spells;
            creators["co"] = &ChatActionContext::co;
            creators["nc"] = &ChatActionContext::nc;
            creators["dead"] = &ChatActionContext::dead;
            creators["trainer"] = &ChatActionContext::trainer;
            creators["attack my target"] = &ChatActionContext::attack_my_target;
            creators["chat"] = &ChatActionContext::chat;
            creators["home"] = &ChatActionContext::home;
            creators["destroy"] = &ChatActionContext::destroy;
            creators["reset ai"] = &ChatActionContext::reset_ai;
            creators["buff"] = &ChatActionContext::buff;
            creators["help"] = &ChatActionContext::help;
            creators["gb"] = &ChatActionContext::gb;
            creators["bank"] = &ChatActionContext::bank;
            creators["follow chat shortcut"] = &ChatActionContext::follow_chat_shortcut;
            creators["stay chat shortcut"] = &ChatActionContext::stay_chat_shortcut;
            creators["flee chat shortcut"] = &ChatActionContext::flee_chat_shortcut;
            creators["runaway chat shortcut"] = &ChatActionContext::runaway_chat_shortcut;
            creators["grind chat shortcut"] = &ChatActionContext::grind_chat_shortcut;
            creators["tank attack chat shortcut"] = &ChatActionContext::tank_attack_chat_shortcut;
            creators["gossip hello"] = &ChatActionContext::gossip_hello;
            creators["cast custom spell"] = &ChatActionContext::cast_custom_spell;
            creators["invite"] = &ChatActionContext::invite;
            creators["spell"] = &ChatActionContext::spell;
            creators["rti"] = &ChatActionContext::rti;
            creators["spirit healer"] = &ChatActionContext::spirit_healer;
            creators["position"] = &ChatActionContext::position;
            creators["tell target"] = &ChatActionContext::tell_target;
            creators["summon"] = &ChatActionContext::summon;
            creators["who"] = &ChatActionContext::who;
            creators["save mana"] = &ChatActionContext::save_mana;
            creators["max dps chat shortcut"] = &ChatActionContext::max_dps_chat_shortcut;
            creators["tell attackers"] = &ChatActionContext::tell_attackers;
            creators["formation"] = &ChatActionContext::formation;
        }

    private:
        static std::shared_ptr<Action> formation(PlayerbotAI* ai) { return std::make_shared<SetFormationAction>(ai); }
        static std::shared_ptr<Action> tell_attackers(PlayerbotAI* ai) { return std::make_shared<TellAttackersAction>(ai); }
        static std::shared_ptr<Action> max_dps_chat_shortcut(PlayerbotAI* ai) { return std::make_shared<MaxDpsChatShortcutAction>(ai); }
        static std::shared_ptr<Action> save_mana(PlayerbotAI* ai) { return std::make_shared<SaveManaAction>(ai); }
        static std::shared_ptr<Action> who(PlayerbotAI* ai) { return std::make_shared<WhoAction>(ai); }
        static std::shared_ptr<Action> summon(PlayerbotAI* ai) { return std::make_shared<SummonAction>(ai); }
        static std::shared_ptr<Action> tell_target(PlayerbotAI* ai) { return std::make_shared<TellTargetAction>(ai); }
        static std::shared_ptr<Action> position(PlayerbotAI* ai) { return std::make_shared<PositionAction>(ai); }
        static std::shared_ptr<Action> spirit_healer(PlayerbotAI* ai) { return std::make_shared<SpiritHealerAction>(ai); }
        static std::shared_ptr<Action> rti(PlayerbotAI* ai) { return std::make_shared<RtiAction>(ai); }
        static std::shared_ptr<Action> invite(PlayerbotAI* ai) { return std::make_shared<InviteToGroupAction>(ai); }
        static std::shared_ptr<Action> spell(PlayerbotAI* ai) { return std::make_shared<TellSpellAction>(ai); }
        static std::shared_ptr<Action> cast_custom_spell(PlayerbotAI* ai) { return std::make_shared<CastCustomSpellAction>(ai); }
        static std::shared_ptr<Action> tank_attack_chat_shortcut(PlayerbotAI* ai) { return std::make_shared<TankAttackChatShortcutAction>(ai); }
        static std::shared_ptr<Action> grind_chat_shortcut(PlayerbotAI* ai) { return std::make_shared<GrindChatShortcutAction>(ai); }
        static std::shared_ptr<Action> flee_chat_shortcut(PlayerbotAI* ai) { return std::make_shared<FleeChatShortcutAction>(ai); }
        static std::shared_ptr<Action> runaway_chat_shortcut(PlayerbotAI* ai) { return std::make_shared<GoawayChatShortcutAction>(ai); }
        static std::shared_ptr<Action> stay_chat_shortcut(PlayerbotAI* ai) { return std::make_shared<StayChatShortcutAction>(ai); }
        static std::shared_ptr<Action> follow_chat_shortcut(PlayerbotAI* ai) { return std::make_shared<FollowChatShortcutAction>(ai); }
        static std::shared_ptr<Action> gb(PlayerbotAI* ai) { return std::make_shared<GuildBankAction>(ai); }
        static std::shared_ptr<Action> bank(PlayerbotAI* ai) { return std::make_shared<BankAction>(ai); }
        static std::shared_ptr<Action> help(PlayerbotAI* ai) { return std::make_shared<HelpAction>(ai); }
        static std::shared_ptr<Action> buff(PlayerbotAI* ai) { return std::make_shared<BuffAction>(ai); }
        static std::shared_ptr<Action> destroy(PlayerbotAI* ai) { return std::make_shared<DestroyItemAction>(ai); }
        static std::shared_ptr<Action> home(PlayerbotAI* ai) { return std::make_shared<SetHomeAction>(ai); }
        static std::shared_ptr<Action> chat(PlayerbotAI* ai) { return std::make_shared<ChangeChatAction>(ai); }
        static std::shared_ptr<Action> attack_my_target(PlayerbotAI* ai) { return std::make_shared<AttackMyTargetAction>(ai); }
        static std::shared_ptr<Action> trainer(PlayerbotAI* ai) { return std::make_shared<TrainerAction>(ai); }
        static std::shared_ptr<Action> co(PlayerbotAI* ai) { return std::make_shared<ChangeCombatStrategyAction>(ai); }
        static std::shared_ptr<Action> nc(PlayerbotAI* ai) { return std::make_shared<ChangeNonCombatStrategyAction>(ai); }
        static std::shared_ptr<Action> dead(PlayerbotAI* ai) { return std::make_shared<ChangeDeadStrategyAction>(ai); }
        static std::shared_ptr<Action> spells(PlayerbotAI* ai) { return std::make_shared<ListSpellsAction>(ai); }
        static std::shared_ptr<Action> talents(PlayerbotAI* ai) { return std::make_shared<ChangeTalentsAction>(ai); }

        static std::shared_ptr<Action> equip(PlayerbotAI* ai) { return std::make_shared<EquipAction>(ai); }
        static std::shared_ptr<Action> unequip(PlayerbotAI* ai) { return std::make_shared<UnequipAction>(ai); }
        static std::shared_ptr<Action> sell(PlayerbotAI* ai) { return std::make_shared<SellAction>(ai); }
        static std::shared_ptr<Action> buy(PlayerbotAI* ai) { return std::make_shared<BuyAction>(ai); }
        static std::shared_ptr<Action> reward(PlayerbotAI* ai) { return std::make_shared<RewardAction>(ai); }
        static std::shared_ptr<Action> trade(PlayerbotAI* ai) { return std::make_shared<TradeAction>(ai); }

        static std::shared_ptr<Action> item_count(PlayerbotAI* ai) { return std::make_shared<TellItemCountAction>(ai); }
        static std::shared_ptr<Action> use(PlayerbotAI* ai) { return std::make_shared<UseItemAction>(ai); }
        static std::shared_ptr<Action> repair(PlayerbotAI* ai) { return std::make_shared<RepairAllAction>(ai); }
        static std::shared_ptr<Action> taxi(PlayerbotAI* ai) { return std::make_shared<TaxiAction>(ai); }
        static std::shared_ptr<Action> teleport(PlayerbotAI* ai) { return std::make_shared<TeleportAction>(ai); }
        static std::shared_ptr<Action> release(PlayerbotAI* ai) { return std::make_shared<ReleaseSpiritAction>(ai); }
        static std::shared_ptr<Action> query_item_usage(PlayerbotAI* ai) { return std::make_shared<QueryItemUsageAction>(ai); }
        static std::shared_ptr<Action> query_quest(PlayerbotAI* ai) { return std::make_shared<QueryQuestAction>(ai); }
        static std::shared_ptr<Action> drop(PlayerbotAI* ai) { return std::make_shared<DropQuestAction>(ai); }
        static std::shared_ptr<Action> stats(PlayerbotAI* ai) { return std::make_shared<StatsAction>(ai); }
        static std::shared_ptr<Action> quests(PlayerbotAI* ai) { return std::make_shared<ListQuestsAction>(ai); }
        static std::shared_ptr<Action> leave(PlayerbotAI* ai) { return std::make_shared<LeaveGroupAction>(ai); }
        static std::shared_ptr<Action> reputation(PlayerbotAI* ai) { return std::make_shared<TellReputationAction>(ai); }
        static std::shared_ptr<Action> log(PlayerbotAI* ai) { return std::make_shared<LogLevelAction>(ai); }
        static std::shared_ptr<Action> los(PlayerbotAI* ai) { return std::make_shared<TellLosAction>(ai); }
        static std::shared_ptr<Action> ll(PlayerbotAI* ai) { return std::make_shared<LootStrategyAction>(ai); }
        static std::shared_ptr<Action> add_all_loot(PlayerbotAI* ai) { return std::make_shared<AddAllLootAction>(ai); }
        static std::shared_ptr<Action> reset_ai(PlayerbotAI* ai) { return std::make_shared<ResetAiAction>(ai); }
        static std::shared_ptr<Action> gossip_hello(PlayerbotAI* ai) { return std::make_shared<GossipHelloAction>(ai); }
    };


};
