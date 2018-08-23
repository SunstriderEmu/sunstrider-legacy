
#ifndef _PLAYER_DUMP_H
#define _PLAYER_DUMP_H
#include <string>
#include <map>
#include <set>

enum DumpTableType
{
    DTT_CHARACTER,      //                                  // characters

    DTT_CHAR_TABLE,     //                                  // character_action, character_aura, character_homebind,
                                                            // character_queststatus, character_reputation,
                                                            // character_spell, character_spell_cooldown, character_ticket,
                                                            // character_tutorial

    DTT_INVENTORY,      //    -> item guids collection      // character_inventory

    DTT_MAIL,           //    -> mail ids collection        // mail
                        //    -> item_text

    DTT_MAIL_ITEM,      // <- mail ids                      // mail_items
                        //    -> item guids collection

    DTT_ITEM,           // <- item guids                    // item_instance
                        //    -> item_text

    DTT_ITEM_GIFT,      // <- item guids                    // character_gifts

    DTT_PET,            //    -> pet guids collection       // character_pet
    DTT_PET_TABLE,      // <- pet guids                     // pet_aura, pet_spell, pet_spell_cooldown
    DTT_ITEM_TEXT,      // <- item_text                     // item_text
};

enum DumpReturn
{
    DUMP_SUCCESS,
    DUMP_FILE_OPEN_ERROR,
    DUMP_TOO_MANY_CHARS,
    DUMP_UNEXPECTED_END,
    DUMP_FILE_BROKEN,
};

class PlayerDump
{
    protected:
        PlayerDump() {}
};

class TC_GAME_API PlayerDumpWriter : public PlayerDump
{
    public:
        PlayerDumpWriter() {}

        std::string GetDump(uint32 guid);
        DumpReturn WriteDump(const std::string& file, uint32 guid);
    private:
        typedef std::set<uint32> GUIDs;

        void DumpTable(std::string& dump, uint32 guid, char const*tableFrom, char const*tableTo, DumpTableType type);
        std::string GenerateWhereStr(char const* field, GUIDs const& guids, GUIDs::const_iterator& itr);
        std::string GenerateWhereStr(char const* field, uint32 guid);

        GUIDs pets;
        GUIDs mails;
        GUIDs items;
        GUIDs texts;
};

class TC_GAME_API PlayerDumpReader : public PlayerDump
{
    public:
        PlayerDumpReader() {}

        DumpReturn LoadDump(const std::string& file, uint32 account, std::string name, uint32 guid);
};

#endif

