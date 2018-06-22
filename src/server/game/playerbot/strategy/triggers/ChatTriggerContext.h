#pragma once

#include "ChatCommandTrigger.h"

namespace ai
{
    class ChatTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        ChatTriggerContext()
        {
            creators["quests"] = &ChatTriggerContext::quests;
            creators["stats"] = &ChatTriggerContext::stats;
            creators["leave"] = &ChatTriggerContext::leave;
            creators["rep"] = &ChatTriggerContext::reputation;
            creators["reputation"] = &ChatTriggerContext::reputation;
            creators["log"] = &ChatTriggerContext::log;
            creators["los"] = &ChatTriggerContext::los;
            creators["drop"] = &ChatTriggerContext::drop;
            creators["q"] = &ChatTriggerContext::q;
            creators["ll"] = &ChatTriggerContext::ll;
            creators["loot all"] = &ChatTriggerContext::loot_all;
            creators["add all loot"] = &ChatTriggerContext::loot_all;
            creators["release"] = &ChatTriggerContext::release;
            creators["teleport"] = &ChatTriggerContext::teleport;
            creators["taxi"] = &ChatTriggerContext::taxi;
            creators["repair"] = &ChatTriggerContext::repair;
            creators["u"] = &ChatTriggerContext::use;
            creators["use"] = &ChatTriggerContext::use;
            creators["c"] = &ChatTriggerContext::item_count;
            creators["e"] = &ChatTriggerContext::equip;
            creators["ue"] = &ChatTriggerContext::uneqip;
            creators["s"] = &ChatTriggerContext::sell;
            creators["b"] = &ChatTriggerContext::buy;
            creators["r"] = &ChatTriggerContext::reward;
            creators["t"] = &ChatTriggerContext::trade;
            creators["nt"] = &ChatTriggerContext::nontrade;
            creators["talents"] = &ChatTriggerContext::talents;
            creators["spells"] = &ChatTriggerContext::spells;
            creators["co"] = &ChatTriggerContext::co;
            creators["nc"] = &ChatTriggerContext::nc;
            creators["dead"] = &ChatTriggerContext::dead;
            creators["trainer"] = &ChatTriggerContext::trainer;
            creators["attack"] = &ChatTriggerContext::attack;
            creators["chat"] = &ChatTriggerContext::chat;
            creators["accept"] = &ChatTriggerContext::accept;
            creators["home"] = &ChatTriggerContext::home;
            creators["reset ai"] = &ChatTriggerContext::reset_ai;
            creators["destroy"] = &ChatTriggerContext::destroy;
            creators["emote"] = &ChatTriggerContext::emote;
            creators["buff"] = &ChatTriggerContext::buff;
            creators["help"] = &ChatTriggerContext::help;
            creators["gb"] = &ChatTriggerContext::gb;
            creators["bank"] = &ChatTriggerContext::bank;
            creators["follow"] = &ChatTriggerContext::follow;
            creators["stay"] = &ChatTriggerContext::stay;
            creators["flee"] = &ChatTriggerContext::flee;
            creators["grind"] = &ChatTriggerContext::grind;
            creators["tank attack"] = &ChatTriggerContext::tank_attack;
            creators["talk"] = &ChatTriggerContext::talk;
            creators["cast"] = &ChatTriggerContext::talk;
            creators["invite"] = &ChatTriggerContext::invite;
            creators["spell"] = &ChatTriggerContext::spell;
            creators["rti"] = &ChatTriggerContext::rti;
            creators["revive"] = &ChatTriggerContext::revive;
            creators["runaway"] = &ChatTriggerContext::runaway;
            creators["warning"] = &ChatTriggerContext::warning;
            creators["position"] = &ChatTriggerContext::position;
            creators["summon"] = &ChatTriggerContext::summon;
            creators["who"] = &ChatTriggerContext::who;
            creators["save mana"] = &ChatTriggerContext::save_mana;
            creators["max dps"] = &ChatTriggerContext::max_dps;
            creators["attackers"] = &ChatTriggerContext::attackers;
            creators["formation"] = &ChatTriggerContext::formation;
        }

    private:
        static std::shared_ptr<Trigger> formation(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "formation"); }
        static std::shared_ptr<Trigger> attackers(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "attackers"); }
        static std::shared_ptr<Trigger> max_dps(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "max dps"); }
        static std::shared_ptr<Trigger> save_mana(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "save mana"); }
        static std::shared_ptr<Trigger> who(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "who"); }
        static std::shared_ptr<Trigger> summon(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "summon"); }
        static std::shared_ptr<Trigger> position(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "position"); }
        static std::shared_ptr<Trigger> runaway(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "runaway"); }
        static std::shared_ptr<Trigger> warning(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "warning"); }
        static std::shared_ptr<Trigger> revive(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "revive"); }
        static std::shared_ptr<Trigger> rti(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "rti"); }
        static std::shared_ptr<Trigger> invite(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "invite"); }
        static std::shared_ptr<Trigger> cast(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "cast"); }
        static std::shared_ptr<Trigger> talk(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "talk"); }
        static std::shared_ptr<Trigger> flee(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "flee"); }
        static std::shared_ptr<Trigger> grind(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "grind"); }
        static std::shared_ptr<Trigger> tank_attack(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "tank attack"); }
        static std::shared_ptr<Trigger> stay(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "stay"); }
        static std::shared_ptr<Trigger> follow(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "follow"); }
        static std::shared_ptr<Trigger> gb(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "gb"); }
        static std::shared_ptr<Trigger> bank(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "bank"); }
        static std::shared_ptr<Trigger> help(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "help"); }
        static std::shared_ptr<Trigger> buff(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "buff"); }
        static std::shared_ptr<Trigger> emote(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "emote"); }
        static std::shared_ptr<Trigger> destroy(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "destroy"); }
        static std::shared_ptr<Trigger> home(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "home"); }
        static std::shared_ptr<Trigger> accept(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "accept"); }
        static std::shared_ptr<Trigger> chat(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "chat"); }
        static std::shared_ptr<Trigger> attack(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "attack"); }
        static std::shared_ptr<Trigger> trainer(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "trainer"); }
        static std::shared_ptr<Trigger> co(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "co"); }
        static std::shared_ptr<Trigger> nc(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "nc"); }
        static std::shared_ptr<Trigger> dead(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "dead"); }
        static std::shared_ptr<Trigger> spells(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "spells"); }
        static std::shared_ptr<Trigger> talents(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "talents"); }
        static std::shared_ptr<Trigger> equip(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "e"); }
        static std::shared_ptr<Trigger> uneqip(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "ue"); }
        static std::shared_ptr<Trigger> sell(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "s"); }
        static std::shared_ptr<Trigger> buy(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "b"); }
        static std::shared_ptr<Trigger> reward(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "r"); }
        static std::shared_ptr<Trigger> trade(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "t"); }
        static std::shared_ptr<Trigger> nontrade(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "nt"); }

        static std::shared_ptr<Trigger> item_count(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "c"); }
        static std::shared_ptr<Trigger> use(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "use"); }
        static std::shared_ptr<Trigger> repair(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "repair"); }
        static std::shared_ptr<Trigger> taxi(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "taxi"); }
        static std::shared_ptr<Trigger> teleport(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "teleport"); }
        static std::shared_ptr<Trigger> q(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "q"); }
        static std::shared_ptr<Trigger> ll(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "ll"); }
        static std::shared_ptr<Trigger> drop(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "drop"); }
        static std::shared_ptr<Trigger> quests(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "quests"); }
        static std::shared_ptr<Trigger> stats(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "stats"); }
        static std::shared_ptr<Trigger> leave(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "leave"); }
        static std::shared_ptr<Trigger> reputation(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "reputation"); }
        static std::shared_ptr<Trigger> log(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "log"); }
        static std::shared_ptr<Trigger> los(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "los"); }
        static std::shared_ptr<Trigger> loot_all(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "add all loot"); }
        static std::shared_ptr<Trigger> release(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "release"); }
        static std::shared_ptr<Trigger> reset_ai(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "reset ai"); }
        static std::shared_ptr<Trigger> spell(PlayerbotAI* ai) { return std::make_shared<ChatCommandTrigger>(ai, "spell"); }
    };
};
