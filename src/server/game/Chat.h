

#ifndef TRINITYCORE_CHAT_H
#define TRINITYCORE_CHAT_H

#include "SharedDefines.h"

class ChatHandler;
class WorldSession;
class Creature;
class Player;
class Unit;
struct GameTele;
struct GM_Ticket;
class GameObject;

class ChatCommand
{
    typedef bool(*pHandler)(ChatHandler*, char const*);

    public:
        ChatCommand(char const* name, uint32 security, bool allowConsole, pHandler handler, std::string help, std::vector<ChatCommand> childCommands = std::vector<ChatCommand>());

        const char *                  Name;
        uint32                        SecurityLevel;                   // function pointer required correct align (use uint32)
        bool                          AllowConsole;
        pHandler Handler;
        std::string                   Help;
        std::vector<ChatCommand>      ChildCommands;
};

class TC_GAME_API ChatHandler
{
    public:
		explicit ChatHandler(WorldSession* session);
		explicit ChatHandler(Player* player);
         ~ChatHandler() = default;

         // Builds chat packet and returns receiver guid position in the packet to substitute in whisper builders
         static size_t BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, ObjectGuid senderGUID, ObjectGuid receiverGUID, std::string const& message, uint8 chatTag = 0,
                                    std::string const& senderName = "", std::string const& receiverName = "",
                                    uint32 achievementId = 0, bool gmMessage = false, std::string const& channelName = "");
         // Builds chat packet and returns receiver guid position in the packet to substitute in whisper builders
        static size_t BuildChatPacket(WorldPacket& data, ChatMsg chatType, Language language, WorldObject const* sender, WorldObject const* receiver, std::string const& message, uint32 achievementId = 0, std::string const& channelName = "", LocaleConstant locale = DEFAULT_LOCALE);

        static char* LineFromMessage(char*& pos) { char* start = strtok(pos,"\n"); pos = nullptr; return start; }
        
        // function with different implementation for chat/console
        virtual bool HasPermission(uint32 permission) const;
        virtual std::string GetNameLink() const;
        virtual LocaleConstant GetSessionDbcLocale() const;
        virtual bool isAvailable(ChatCommand const& cmd) const;
        virtual bool needReportToTarget(Player* chr) const;

        virtual const char* GetTrinityString(int32 entry) const;
        std::string GetTrinityStringVA(int32 entry, ...) const;

        virtual void SendSysMessage(char const* str, bool escapeCharacters = false);
        void SendSysMessage(int32 entry);
        
        template<typename... Args>
        void PSendSysMessage(const char* fmt, Args&&... args)
        {
            SendSysMessage(Trinity::StringFormat(fmt, std::forward<Args>(args)...).c_str());
        }
        template<typename... Args>
        void PSendSysMessage(uint32 entry, Args&&... args)
        {
            SendSysMessage(PGetParseString(entry, std::forward<Args>(args)...).c_str());
        }
        std::string PGetParseString(int32 entry, ...);

        int ParseCommands(const char* text);

        virtual std::string const GetName() const;
        static std::vector<ChatCommand> const& getCommandTable();
        static void invalidateCommandTable();
        
        WorldSession* GetSession() { return m_session; }

        bool HasLowerSecurity(Player* target, ObjectGuid guid, bool strong = false);
        bool HasLowerSecurityAccount(WorldSession* target, uint32 account, bool strong = false);

        static void SendMessageWithoutAuthor(char const* channel, const char* msg);
        char*     extractKeyFromLink(char* text, char const* linkType, char** something1 = nullptr);

        std::string playerLink(std::string const& name) const { return m_session ? "|cffffffff|Hplayer:"+name+"|h["+name+"]|h|r" : name; }
		std::string GetNameLink(Player* chr) const;

        std::string extractPlayerNameFromLink(char* text);
        // select by arg (name/link) or in-game selection online/offline player
        bool extractPlayerTarget(char* args, Player** player, ObjectGuid* player_guid =nullptr, std::string* player_name = nullptr);
        char*     extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1 = nullptr);
        uint32    extractSpellIdFromLink(char* text);
        GameTele const* extractGameTeleFromLink(char* text);

        // if args have single value then it return in arg2 and arg1 == nullptr
        void      extractOptFirstArg(char* args, char** arg1, char** arg2);
        char*     extractQuotedArg(char* args);

        Player*   GetSelectedPlayer() const;
        Creature* GetSelectedCreature() const;
        Unit*     GetSelectedUnit() const;
        Player*   GetSelectedPlayerOrSelf() const;

        bool HasSentErrorMessage() const { return sentErrorMessage; }
        void SetSentErrorMessage(bool val){ sentErrorMessage = val;};

        static void SendGlobalSysMessage(const char *str);
        static void SendGlobalGMSysMessage(const char *str);

        GameObject* GetNearbyGameObject();
        GameObject* GetObjectFromPlayerMapByDbGuid(ObjectGuid::LowType lowguid);
        Creature* GetCreatureFromPlayerMapByDbGuid(ObjectGuid::LowType lowguid);

        bool GetPlayerGroupAndGUIDByName(const char* cname, Player* &plr, Group* &group, ObjectGuid &guid, bool offline = false);

        static uint32 ReputationRankStrIndex[MAX_REPUTATION_RANK];

        bool ShowHelpForCommand(std::vector<ChatCommand> const& table, const char* cmd);
        bool ShowHelpForSubCommands(std::vector<ChatCommand> const& table, char const* cmd, char const* subcmd);

    protected:
        explicit ChatHandler() : m_session(nullptr) {}      // for CLI subclass
        static bool SetDataForCommandInTable(std::vector<ChatCommand>& table, const char* text, uint32 securityLevel, std::string const& help, std::string const& fullcommand);

        bool hasStringAbbr(const char* name, const char* part) const;

        bool ExecuteCommandInTable(std::vector<ChatCommand> const& table, const char* text, const std::string& fullcommand);

        // Utility methods for commands
        bool LookupPlayerSearchCommand(QueryResult result, int32 limit);
        bool HandleBanListHelper(QueryResult result);
        bool HandleBanHelper(SanctionType mode,char const* args);
        bool HandleBanInfoHelper(uint32 accountid, char const* accountname);
        bool HandleUnBanHelper(SanctionType mode, char const* args);


    private:
        WorldSession * m_session;                           // != NULL for chat command call and NULL for CLI command

        // common global flag
        static bool load_command_table;
        bool sentErrorMessage;
};

class CliHandler : public ChatHandler
{
    public:
        typedef void Print(void*, char const*);
        explicit CliHandler(void* callbackArg, Print* zprint) : m_callbackArg(callbackArg), m_print(zprint) { }

        // overwrite functions
        const char *GetTrinityString(int32 entry) const override;
        bool isAvailable(ChatCommand const& cmd) const override;
        bool HasPermission(uint32 /*permission*/) const override { return true; }
        void SendSysMessage(const char *str, bool escapeCharacters = false) override;
        std::string GetNameLink() const override;
        bool needReportToTarget(Player* chr) const override;
        LocaleConstant GetSessionDbcLocale() const override;

    private:
        void* m_callbackArg;
        Print* m_print;
};

char const *fmtstring(char const *format, ...);

//return false if args char* is empty
#define ARGS_CHECK if(!*args)return false;

#endif