#pragma once

class Player;

using namespace ai;

class AiFactory
{
public:
    static AiObjectContext* createAiObjectContext(Player* player, PlayerbotAI* ai);
    static Engine* createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext);
    static Engine* createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext);
    static Engine* createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext);
    static void AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine);
    static void AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine);
    static void AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* engine);

public:
    static void CountTalentsPerTab(Player* player, int& tab1, int& tab2, int& tab3);
    static int GetPlayerSpecTab(Player* player);
};
