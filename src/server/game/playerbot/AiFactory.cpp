
#include "playerbot.h"
#include "AiFactory.h"
#include "strategy/Engine.h"

#include "strategy/priest/PriestAiObjectContext.h"
#include "strategy/mage/MageAiObjectContext.h"
#include "strategy/warlock/WarlockAiObjectContext.h"
#include "strategy/warrior/WarriorAiObjectContext.h"
#include "strategy/shaman/ShamanAiObjectContext.h"
#include "strategy/paladin/PaladinAiObjectContext.h"
#include "strategy/druid/DruidAiObjectContext.h"
#include "strategy/hunter/HunterAiObjectContext.h"
#include "strategy/rogue/RogueAiObjectContext.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"


AiObjectContext* AiFactory::createAiObjectContext(Player* player, PlayerbotAI* ai)
{
    switch (player->GetClass())
    {
    case CLASS_PRIEST:
        return new PriestAiObjectContext(ai);
        break;
    case CLASS_MAGE:
        return new MageAiObjectContext(ai);
        break;
    case CLASS_WARLOCK:
        return new WarlockAiObjectContext(ai);
        break;
    case CLASS_WARRIOR:
        return new WarriorAiObjectContext(ai);
        break;
    case CLASS_SHAMAN:
        return new ShamanAiObjectContext(ai);
        break;
    case CLASS_PALADIN:
        return new PaladinAiObjectContext(ai);
        break;
    case CLASS_DRUID:
        return new DruidAiObjectContext(ai);
        break;
    case CLASS_HUNTER:
        return new HunterAiObjectContext(ai);
        break;
    case CLASS_ROGUE:
        return new RogueAiObjectContext(ai);
        break;
    }
    return new AiObjectContext(ai);
}

int AiFactory::GetPlayerSpecTab(Player* player)
{
    /*  TODO PLAYERBOT
    int c0 = 0, c1 = 0, c2 = 0;
    PlayerTalentMap& talentMap = player->GetTalentMap(0);
    for (PlayerTalentMap::iterator i = talentMap.begin(); i != talentMap.end(); ++i)
    {
        uint32 spellId = i->first;
        TalentSpellPos const* talentPos = GetTalentSpellPos(spellId);
        if(!talentPos)
            continue;

        TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id);
        if (!talentInfo)
            continue;

        uint32 const* talentTabIds = GetTalentTabPages(player->GetClass());
        if (talentInfo->TalentTab == talentTabIds[0]) c0++;
        if (talentInfo->TalentTab == talentTabIds[1]) c1++;
        if (talentInfo->TalentTab == talentTabIds[2]) c2++;
    }

    if (c0 >= c1 && c0 >= c2)
        return 0;

    if (c1 >= c0 && c1 >= c2)
        return 1;
        */
    return 2;
}

void AiFactory::AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* engine)
{
    int tab = GetPlayerSpecTab(player);

    engine->addStrategies({ "attack weak", "racials", "chat", "default", "aoe", "potions", "cast time", "conserve mana", "duel", "pvp" });

    switch (player->GetClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
            {
                engine->addStrategies({ "dps", "threat" });
                if (player->GetLevel() > 19)
                    engine->addStrategy("dps debuff");
            }
            else
                engine->addStrategy("heal");

            engine->addStrategy("flee");
            break;
        case CLASS_MAGE:
            if (tab == 0)
                engine->addStrategies({ "arcane", "threat" });
            else if (tab == 1)
                engine->addStrategies({ "fire", "fire aoe", "threat" });
            else
                engine->addStrategies({ "frost", "frost aoe", "threat" });

            engine->addStrategy("flee");
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                engine->addStrategies({ "tank", "tank aoe" });
            else
                engine->addStrategies({"dps", "threat"});
            break;
        case CLASS_SHAMAN:
            if (tab == 0)
                engine->addStrategies({ "caster", "caster aoe", "bmana", "threat", "flee" });
            else if (tab == 2)
                engine->addStrategies({ "heal", "bmana", "flee" });
            else
                engine->addStrategies({ "dps", "melee aoe", "bdps", "threat" });
            break;
        case CLASS_PALADIN:
            if (tab == 1)
                engine->addStrategies({ "tank", "tank aoe", "barmor" });
            else
                engine->addStrategies({ "dps", "bdps", "threat" });
            break;
        case CLASS_DRUID:
            if (tab == 0)
            {
                engine->addStrategies({ "caster", "caster aoe", "threat", "flee" });
                if (player->GetLevel() > 19)
                    engine->addStrategy("caster debuff");
            }
            else if (tab == 2)
                engine->addStrategies({ "heal", "flee" });
            else
                engine->addStrategies({ "bear", "tank aoe", "threat", "flee" });
            break;
        case CLASS_HUNTER:
            engine->addStrategies({ "dps", "bdps", "threat" });
            if (player->GetLevel() > 19)
                engine->addStrategy("dps debuff");
            break;
        case CLASS_ROGUE:
            engine->addStrategies({ "dps", "threat" });
            break;
        case CLASS_WARLOCK:
            if (tab == 1)
                engine->addStrategies({"tank", "threat"});
            else
                engine->addStrategies({ "dps", "threat" });

            if (player->GetLevel() > 19)
                engine->addStrategy("dps debuff");

            engine->addStrategy("flee");
            break;
    }

    if (sRandomPlayerbotMgr.IsRandomBot(player) && !player->GetGroup())
    {
        engine->ChangeStrategy(sPlayerbotAIConfig.randomBotCombatStrategies);
        if (player->GetClass() == CLASS_DRUID && player->GetLevel() < 20)
            engine->addStrategies({ "bear", "threat" });
    }
}

Engine* AiFactory::createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext) {
    Engine* engine = new Engine(facade, AiObjectContext);
    AddDefaultCombatStrategies(player, facade, engine);
    return engine;
}

void AiFactory::AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine)
{
    int tab = GetPlayerSpecTab(player);

    switch (player->GetClass()){
        case CLASS_PALADIN:
        case CLASS_HUNTER:
        case CLASS_SHAMAN:
            nonCombatEngine->addStrategy("bmana");
            break;
        case CLASS_MAGE:
            if (tab == 1)
                nonCombatEngine->addStrategy("bdps");
            else
                nonCombatEngine->addStrategy("bmana");
            break;
    }
    nonCombatEngine->addStrategies({ "nc", "attack weak", "food", "stay", "chat",
            "default", "quest", "loot", "gather", "duel", "emote", "lfg" });

    if (sRandomPlayerbotMgr.IsRandomBot(player) && !player->GetGroup())
    {
        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotNonCombatStrategies);
    }

}

Engine* AiFactory::createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext) {
    Engine* nonCombatEngine = new Engine(facade, AiObjectContext);

    AddDefaultNonCombatStrategies(player, facade, nonCombatEngine);
    return nonCombatEngine;
}

void AiFactory::AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine)
{
    deadEngine->addStrategies({ "dead", "stay", "chat", "default", "follow" });
    if (sRandomPlayerbotMgr.IsRandomBot(player) && !player->GetGroup())
    {
        deadEngine->removeStrategy("follow");
    }
}

Engine* AiFactory::createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext) {
    Engine* deadEngine = new Engine(facade, AiObjectContext);
    AddDefaultDeadStrategies(player, facade, deadEngine);
    return deadEngine;
}
