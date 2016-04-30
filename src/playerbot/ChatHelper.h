#pragma once

using namespace std;

typedef set<uint32> ItemIds;
typedef set<uint32> SpellIds;



namespace ai
{
    class ChatHelper : public PlayerbotAIAware
    {
    public:
        ChatHelper(PlayerbotAI* ai);

    public:
        static std::string formatMoney(uint32 copper);
        static uint32 parseMoney(std::string& text);
        static ItemIds parseItems(std::string& text);
        uint32 parseSpell(string& text);
        static std::string formatQuest(Quest const* quest);
        static std::string formatItem(ItemTemplate const * proto, int count = 0);
        static std::string formatSpell(SpellInfo const *sInfo);
        static std::string formatGameobject(GameObject* go);
        static std::string formatQuestObjective(std::string name, int available, int required);
        static list<ObjectGuid> parseGameobjects(std::string& text);

        static ChatMsg parseChat(std::string& text);
        static std::string formatChat(ChatMsg chat);

        static std::string formatClass(Player* player, int spec);
        static std::string formatClass(uint8 cls);
        static std::string formatRace(uint8 race);

        static uint32 parseItemQuality(std::string text);
        static bool parseItemClass(std::string text, uint32 *itemClass, uint32 *itemSubClass);
        static uint32 parseSlot(std::string text);

        static bool parseable(std::string text);

    private:
        static map<std::string, uint32> consumableSubClasses;
        static map<std::string, uint32> tradeSubClasses;
        static map<std::string, uint32> itemQualities;
        static map<std::string, uint32> slots;
        static map<std::string, ChatMsg> chats;
        static map<uint8, std::string> classes;
        static map<uint8, std::string> races;
        static map<uint8, map<uint8, std::string> > specs;
    };
};
