#include "Chat.h"
#include "WorldSession.h"
#include "ReplayPlayer.h"
#include "ReplayRecorder.h"

class replay_commandscript : public CommandScript
{
public:
    replay_commandscript() : CommandScript("replay_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> replayCommandTable =
        {
            { "play",           SEC_ADMINISTRATOR,  false, &HandleReplayPlayCommand,           "" },
            { "forward",        SEC_ADMINISTRATOR,  false, &HandleReplayForwardCommand,        "" },
            { "stop",           SEC_ADMINISTRATOR,  false, &HandleReplayStopCommand,           "" },
            { "record",         SEC_ADMINISTRATOR,  false, &HandleReplayRecordCommand,         "" },
            { "speed",          SEC_ADMINISTRATOR,  false, &HandleReplaySpeedCommand,          "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "replay",         SEC_ADMINISTRATOR,false, nullptr,                              "", replayCommandTable },
        };
        return commandTable;
    }

    static bool HandleReplayPlayCommand(ChatHandler* handler, char const* c)
    {
        if (!c || !*c || strchr(c, '/') != NULL || strchr(c, '.') != NULL)
            return false;

        WorldSession* sess = handler->GetSession();
        if (Player* player = handler->GetSelectedPlayer())
            sess = player->GetSession();

        std::string filename = "replays/";
        filename += c;
        bool result = sess->StartReplaying(filename);
        if (result)
            handler->PSendSysMessage("Starting replay %s for %s", c, handler->playerLink(sess->GetPlayerName()).c_str());
        else
            handler->PSendSysMessage("Could not start replay %s", c);
        return true;
    }

    static bool HandleReplayForwardCommand(ChatHandler* handler, char const* args)
    {
        int32 secsToSkip = (int32)atoi((char*)args);

        auto player = handler->GetSession()->GetReplayPlayer();
        if (player == nullptr)
        {
            handler->SendSysMessage("Not replaying currently");
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->SkipTime(secsToSkip);
        handler->PSendSysMessage("Skipping %i ms", secsToSkip);
        return true;
    }

    static bool HandleReplaySpeedCommand(ChatHandler* handler, char const* args)
    {
        float newRate = (float)atof((char*)args);

        auto player = handler->GetSession()->GetReplayPlayer();
        if (player == nullptr)
        {
            handler->SendSysMessage("Not replaying currently");
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->SetSpeedRate(newRate);
        handler->PSendSysMessage("Read speed rate changed to %f", newRate);
        return true;
    }


    static bool HandleReplayStopCommand(ChatHandler* handler, char const* /*args*/)
    {
        bool result1 = handler->GetSession()->StopRecording();
        if (result1)
            handler->SendSysMessage("Stopped recording");

        bool result2 = handler->GetSession()->StopReplaying();
        //if stopped, output handled in StopReplaying

        if (!result1 && !result2)
        {
            handler->SendSysMessage("No recording to stop");
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleReplayRecordCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* sess = handler->GetSession();
        /*if (Player* player = handler->GetSelectedPlayer())
            sess = player->GetSession();*/

        std::string recordName(args);
        if (strcmp(args, "") == 0)
            return false;

        if (sess->StartRecording(recordName))
            handler->PSendSysMessage("Starting replay recording for %s with name %s", handler->playerLink(sess->GetPlayerName()).c_str(), args);
        else
            handler->PSendSysMessage("Could not start recording. (Maybe you're already recording?)");
        return true;
    }
};

void AddSC_replay_commandscript()
{
    new replay_commandscript();
}
