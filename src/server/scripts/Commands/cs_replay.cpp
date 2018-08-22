#include "Chat.h"
#include "WorldSession.h"
#include "ReplayPlayer.h"
#include "ReplayRecorder.h"

static bool HandleReplayPlayCommand(const char* c)
{
    if (!c || !*c || strchr(c, '/') != NULL || strchr(c, '.') != NULL)
        return false;

    WorldSession* sess = m_session;
    if (Player* player = GetSelectedPlayer())
        sess = player->GetSession();

    std::string filename = "replays/";
    filename += c;
    bool result = sess->StartReplaying(filename);
    if (result)
        handler->PSendSysMessage("Starting replay %s for %s", c, playerLink(sess->GetPlayerName()).c_str());
    else
        handler->PSendSysMessage("Could not start replay %s", c);
    return true;
}

static bool HandleReplayForwardCommand(ChatHandler* handler, char const* args)
{
    int32 secsToSkip = (int32)atoi((char*)args);

    auto player = m_session->GetReplayPlayer();
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

    auto player = m_session->GetReplayPlayer();
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


static bool HandleReplayStopCommand(const char* /* args */)
{
    bool result1 = m_session->StopRecording();
    if (result1)
        handler->SendSysMessage("Stopped recording");

    bool result2 = m_session->StopReplaying();
    //if stopped, output handled in StopReplaying

    if(!result1 && !result2)
    {
        handler->SendSysMessage("No recording to stop");
        handler->SetSentErrorMessage(true);
        return false;
    }

    return true;
}

static bool HandleReplayRecordCommand(ChatHandler* handler, char const* args)
{
    WorldSession* sess = m_session;
    /*if (Player* player = GetSelectedPlayer())
        sess = player->GetSession();*/

    std::string recordName(args);
    if (strcmp(args, "") == 0)
        return false;

    if (sess->StartRecording(recordName))
        handler->PSendSysMessage("Starting replay recording for %s with name %s", playerLink(sess->GetPlayerName()).c_str(), args);
    else
        handler->PSendSysMessage("Could not start recording. (Maybe you're already recording?)");
    return true;
}
